#include "task_control.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "app_tasks.h"
#include "bsp_gpio.h"
#include "bsp_time.h"
#include "control_config.h"
#include "vehicle_config.h"
#include "vehicle_state.h"
#include "wheel.h"

#define CTL_CMD_QUEUE_LEN     (8U)
#define CTL_STACK_WORDS       (384U)
#define CTL_JITTER_SKIP       (20U)       /* ignore start-up ticks in statistics */
#define CTL_RPM_PER_RAD_S     (9.5492966f)
#define CTL_RPM_LPF_ALPHA     (0.2f)
#define CTL_TIMEOUT_MIN_MS    (100U)
#define CTL_TIMEOUT_MAX_MS    (60000U)

static QueueHandle_t s_cmd_q;
static QueueHandle_t s_tel_q;
static wheel_t       s_wheel;
static veh_state_t   s_prev_state;
static TickType_t    s_last_duty_tick;
static uint32_t      s_timeout_ms = CTL_CMD_TIMEOUT_MS;
static uint32_t      s_warn_count;

static void build_default_cfg(wheel_cfg_t *c)
{
    c->enc.counter_bits    = 16U;                    /* overwritten by wheel_init */
    c->enc.direction       = (int8_t)VEH_ENC_DIRECTION;
    c->enc.counts_per_rev  = VEH_WHEEL_CPR;
    c->mot.wheel_idx       = 0U;
    c->mot.max_duty_permille      = (int16_t)MOT_DEFAULT_MAX_DUTY;
    c->mot.deadband_permille      = (int16_t)MOT_DEFAULT_DEADBAND;
    c->mot.ramp_permille_per_tick = (int16_t)MOT_DEFAULT_RAMP;
    c->mot.deadtime_ticks         = (uint8_t)MOT_DEFAULT_DEADTIME;
}

static void on_state_change(veh_state_t st)
{
    wheel_stop(&s_wheel);                        /* zero + brake first            */
    if (st == VEH_STATE_RUN)
    {
        s_last_duty_tick = xTaskGetTickCount();  /* timeout counts from arming    */
        bsp_bridge_enable(true);
    }
    else
    {
        bsp_bridge_enable(false);                /* coast: bridge fully off       */
    }
}

static void apply_cmd(const ctl_cmd_t *c)
{
    switch (c->type)
    {
        case CTL_CMD_DUTY:
            if (veh_get_state() == VEH_STATE_RUN)
            {
                wheel_set_duty(&s_wheel, (int16_t)c->i);
                s_last_duty_tick = xTaskGetTickCount();
            }
            break;
        case CTL_CMD_ARM:
            (void)veh_request_arm();
            break;
        case CTL_CMD_DISARM:
            veh_request_disarm();
            break;
        case CTL_CMD_CLEAR:
            (void)veh_clear_faults();
            break;
        case CTL_CMD_RESET_ENC:
            (void)wheel_reset_encoder(&s_wheel);
            break;
        case CTL_CMD_SET_CPR:
            (void)enc_set_counts_per_rev(&s_wheel.enc, c->f);
            break;
        case CTL_CMD_SET_DEADBAND:
            (void)mot_set_deadband(&s_wheel.mot, (int16_t)c->i);
            break;
        case CTL_CMD_SET_LIMIT:
            (void)mot_set_limit(&s_wheel.mot, (int16_t)c->i);
            break;
        case CTL_CMD_SET_TIMEOUT:
            if ((c->i >= (int32_t)CTL_TIMEOUT_MIN_MS) && (c->i <= (int32_t)CTL_TIMEOUT_MAX_MS))
            {
                s_timeout_ms = (uint32_t)c->i;
            }
            break;
        default:
            break;
    }
}

static void publish(float rpm_filt, uint32_t exec_us_max, uint32_t jit_us_max)
{
    ctl_telemetry_t t;

    t.tick_ms          = (uint32_t)xTaskGetTickCount() * (uint32_t)portTICK_PERIOD_MS;
    t.faults           = veh_get_faults();
    t.state            = (uint8_t)veh_get_state();
    t.counts           = enc_get_count(&s_wheel.enc);
    t.delta            = enc_get_delta(&s_wheel.enc);
    t.angle_x100       = (int32_t)(enc_get_angle_deg(&s_wheel.enc) * 100.0f);
    t.rpm_x100         = (int32_t)(rpm_filt * 100.0f);
    t.cpr_x100         = (int32_t)(s_wheel.enc.cfg.counts_per_rev * 100.0f);
    t.duty_permille    = mot_get_output(&s_wheel.mot);
    t.exec_us_max      = exec_us_max;
    t.jitter_us_max    = jit_us_max;
    t.warn_count       = s_warn_count;
    t.stack_free_words = (uint32_t)uxTaskGetStackHighWaterMark(NULL);
    (void)xQueueOverwrite(s_tel_q, &t);
}

static void control_task(void *arg)
{
    wheel_cfg_t cfg;
    TickType_t  wake;
    uint32_t    prev_start   = 0U;
    uint32_t    tick_count   = 0U;
    uint32_t    exec_max_us  = 0U;
    uint32_t    jitter_max_us = 0U;
    float       rpm_filt     = 0.0f;
    const float dt_s         = (float)CTL_PERIOD_MS / 1000.0f;

    (void)arg;
    build_default_cfg(&cfg);
    if (wheel_init(&s_wheel, 0U, &cfg) != ST_OK)
    {
        veh_raise_fault(VEH_FAULT_INIT);
    }
    s_prev_state = veh_get_state();
    wake = xTaskGetTickCount();

    for (;;)
    {
        const uint32_t start = bsp_time_cycles();
        ctl_cmd_t      cmd;
        veh_state_t    st;
        status_t       ws;

        /* ---- period jitter (measured between loop starts) ---- */
        if (tick_count > CTL_JITTER_SKIP)
        {
            const uint32_t period_us = bsp_time_cycles_to_us(start - prev_start);
            const uint32_t nominal   = CTL_PERIOD_MS * 1000U;
            const uint32_t dev       = (period_us > nominal) ? (period_us - nominal) : (nominal - period_us);
            if (dev > jitter_max_us)
            {
                jitter_max_us = dev;
            }
        }
        prev_start = start;

        /* ---- commands from other tasks ---- */
        while (xQueueReceive(s_cmd_q, &cmd, 0U) == pdTRUE)
        {
            apply_cmd(&cmd);
        }

        /* ---- state handling ---- */
        st = veh_get_state();
        if (st != s_prev_state)
        {
            on_state_change(st);
            s_prev_state = st;
        }
        if (st == VEH_STATE_RUN)
        {
            const TickType_t age = xTaskGetTickCount() - s_last_duty_tick;
            if (age > pdMS_TO_TICKS(s_timeout_ms))
            {
                veh_raise_fault(VEH_FAULT_CMD_TIMEOUT);   /* next tick enters SAFE_STOP */
                wheel_stop(&s_wheel);
            }
        }
        else
        {
            wheel_set_duty(&s_wheel, 0);
        }

        /* ---- the wheel ---- */
        ws = wheel_step(&s_wheel);
        if (ws == ST_WARN_OVERSPEED)
        {
            s_warn_count++;
        }
        else if (ws != ST_OK)
        {
            veh_raise_fault(VEH_FAULT_ENCODER);
        }
        else
        {
            /* ok */
        }

        {
            const float rpm = enc_get_speed_rad_s(&s_wheel.enc, dt_s) * CTL_RPM_PER_RAD_S;
            rpm_filt += CTL_RPM_LPF_ALPHA * (rpm - rpm_filt);
        }

        /* ---- statistics + telemetry ---- */
        {
            const uint32_t used_us = bsp_time_cycles_to_us(bsp_time_cycles() - start);
            if ((tick_count > CTL_JITTER_SKIP) && (used_us > exec_max_us))
            {
                exec_max_us = used_us;
            }
        }
        tick_count++;
        if ((tick_count % CTL_TEL_DECIMATION) == 0U)
        {
            publish(rpm_filt, exec_max_us, jitter_max_us);
        }

        vTaskDelayUntil(&wake, pdMS_TO_TICKS(CTL_PERIOD_MS));
    }
}

status_t ctl_init(void)
{
    status_t st = ST_OK;

    s_cmd_q = xQueueCreate(CTL_CMD_QUEUE_LEN, sizeof(ctl_cmd_t));
    s_tel_q = xQueueCreate(1U, sizeof(ctl_telemetry_t));
    if ((s_cmd_q == NULL) || (s_tel_q == NULL))
    {
        st = ST_ERR_HW;
    }
    return st;
}

status_t ctl_start(void)
{
    const BaseType_t r = xTaskCreate(control_task, "control", CTL_STACK_WORDS, NULL,
                                     TASK_PRIO_CONTROL, NULL);
    return (r == pdPASS) ? ST_OK : ST_ERR_HW;
}

bool ctl_post(const ctl_cmd_t *cmd)
{
    bool ok = false;

    if ((cmd != NULL) && (s_cmd_q != NULL))
    {
        ok = (xQueueSend(s_cmd_q, cmd, 0U) == pdTRUE);
    }
    return ok;
}

bool ctl_get_telemetry(ctl_telemetry_t *out)
{
    bool ok = false;

    if ((out != NULL) && (s_tel_q != NULL))
    {
        ok = (xQueuePeek(s_tel_q, out, 0U) == pdTRUE);
    }
    return ok;
}
