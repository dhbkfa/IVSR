#include "task_comm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_tasks.h"
#include "bsp_uart.h"
#include "task_control.h"
#include "vehicle_state.h"

#define COMM_STACK_WORDS     (768U)    /* snprintf needs a generous stack */
#define COMM_RX_QUEUE_LEN    (64U)
#define COMM_LINE_MAX        (48U)
#define COMM_TX_MAX          (160U)
#define COMM_TEL_PERIOD_MS   (100U)
#define COMM_POLL_MS         (20U)

static QueueHandle_t s_rx_q;
static bool          s_tel_on;

/* ---- ISR context: only *FromISR calls ---- */
static void rx_isr_cb(uint8_t byte)
{
    BaseType_t woken = pdFALSE;

    (void)xQueueSendFromISR(s_rx_q, &byte, &woken);
    portYIELD_FROM_ISR(woken);
}

static void say(const char *s)
{
    (void)bsp_uart_write((const uint8_t *)s, (uint16_t)strlen(s));
}

static void print_help(void)
{
    say("cmds: arm | disarm | clear | d <-1000..1000> | rst | cpr <x> | db <permille>\r\n"
        "      lim <permille> | tmo <ms> | tel <0|1> | info | help\r\n");
}

static void print_info(void)
{
    char buf[COMM_TX_MAX];
    ctl_telemetry_t t;

    if (ctl_get_telemetry(&t))
    {
        (void)snprintf(buf, sizeof(buf),
                       "state=%u faults=0x%lX ctl_stack_free=%lu comm_stack_free=%lu heap_free=%lu\r\n",
                       (unsigned int)t.state, (unsigned long)t.faults,
                       (unsigned long)t.stack_free_words,
                       (unsigned long)uxTaskGetStackHighWaterMark(NULL),
                       (unsigned long)xPortGetFreeHeapSize());
        say(buf);
    }
    else
    {
        say("no telemetry yet\r\n");
    }
}

static void print_telemetry(void)
{
    char buf[COMM_TX_MAX];
    ctl_telemetry_t t;

    if (ctl_get_telemetry(&t))
    {
        (void)snprintf(buf, sizeof(buf),
                       "T,%lu,%u,%lu,%ld,%ld,%ld,%ld,%d,%lu,%lu,%lu,%ld\r\n",
                       (unsigned long)t.tick_ms, (unsigned int)t.state, (unsigned long)t.faults,
                       (long)t.counts, (long)t.delta, (long)t.angle_x100, (long)t.rpm_x100,
                       (int)t.duty_permille, (unsigned long)t.exec_us_max,
                       (unsigned long)t.jitter_us_max, (unsigned long)t.warn_count,
                       (long)t.cpr_x100);
        say(buf);
    }
}

static bool parse_i32(const char *s, int32_t *out)
{
    char *end = NULL;
    long  v;
    bool  ok = false;

    if ((s != NULL) && (*s != '\0'))
    {
        v = strtol(s, &end, 10);
        if ((end != NULL) && (*end == '\0'))
        {
            *out = (int32_t)v;
            ok = true;
        }
    }
    return ok;
}

static bool parse_f32(const char *s, float *out)
{
    char *end = NULL;
    float v;
    bool  ok = false;

    if ((s != NULL) && (*s != '\0'))
    {
        v = strtof(s, &end);
        if ((end != NULL) && (*end == '\0'))
        {
            *out = v;
            ok = true;
        }
    }
    return ok;
}

static void reply(bool ok)
{
    say(ok ? "OK\r\n" : "ERR\r\n");
}

static void handle_line(char *line)
{
    char     *arg = strchr(line, ' ');
    ctl_cmd_t c;
    int32_t   iv = 0;
    float     fv = 0.0f;

    if (arg != NULL)
    {
        *arg = '\0';
        arg++;
    }
    c.type = CTL_CMD_DUTY;
    c.i    = 0;
    c.f    = 0.0f;

    if (strcmp(line, "help") == 0)
    {
        print_help();
    }
    else if (strcmp(line, "info") == 0)
    {
        print_info();
    }
    else if (strcmp(line, "arm") == 0)
    {
        c.type = CTL_CMD_ARM;
        reply(ctl_post(&c));
    }
    else if (strcmp(line, "disarm") == 0)
    {
        c.type = CTL_CMD_DISARM;
        reply(ctl_post(&c));
    }
    else if (strcmp(line, "clear") == 0)
    {
        c.type = CTL_CMD_CLEAR;
        reply(ctl_post(&c));
    }
    else if (strcmp(line, "rst") == 0)
    {
        c.type = CTL_CMD_RESET_ENC;
        reply(ctl_post(&c));
    }
    else if ((strcmp(line, "d") == 0) && parse_i32(arg, &iv) && (iv >= -1000) && (iv <= 1000))
    {
        c.type = CTL_CMD_DUTY;
        c.i    = iv;
        reply(ctl_post(&c));
    }
    else if ((strcmp(line, "cpr") == 0) && parse_f32(arg, &fv) && (fv > 0.0f))
    {
        c.type = CTL_CMD_SET_CPR;
        c.f    = fv;
        reply(ctl_post(&c));
    }
    else if ((strcmp(line, "db") == 0) && parse_i32(arg, &iv))
    {
        c.type = CTL_CMD_SET_DEADBAND;
        c.i    = iv;
        reply(ctl_post(&c));
    }
    else if ((strcmp(line, "lim") == 0) && parse_i32(arg, &iv))
    {
        c.type = CTL_CMD_SET_LIMIT;
        c.i    = iv;
        reply(ctl_post(&c));
    }
    else if ((strcmp(line, "tmo") == 0) && parse_i32(arg, &iv))
    {
        c.type = CTL_CMD_SET_TIMEOUT;
        c.i    = iv;
        reply(ctl_post(&c));
    }
    else if ((strcmp(line, "tel") == 0) && parse_i32(arg, &iv))
    {
        s_tel_on = (iv != 0);
        if (s_tel_on)
        {
            say("H,tick_ms,state,faults,counts,delta,angle_x100,rpm_x100,duty,exec_us,jit_us,warn,cpr_x100\r\n");
        }
        reply(true);
    }
    else
    {
        say("ERR unknown (type help)\r\n");
    }
}

static void comm_task(void *arg)
{
    char       line[COMM_LINE_MAX];
    uint8_t    len = 0U;
    TickType_t last_tel = xTaskGetTickCount();

    (void)arg;
    print_help();

    for (;;)
    {
        uint8_t ch;

        if (xQueueReceive(s_rx_q, &ch, pdMS_TO_TICKS(COMM_POLL_MS)) == pdTRUE)
        {
            if ((ch == (uint8_t)'\r') || (ch == (uint8_t)'\n'))
            {
                if (len > 0U)
                {
                    line[len] = '\0';
                    handle_line(line);
                    len = 0U;
                }
            }
            else if ((ch == 0x08U) || (ch == 0x7FU))
            {
                if (len > 0U)
                {
                    len--;
                }
            }
            else if (len < (uint8_t)(COMM_LINE_MAX - 1U))
            {
                line[len] = (char)ch;
                len++;
            }
            else
            {
                len = 0U;               /* line too long: discard */
            }
        }

        if (s_tel_on && ((xTaskGetTickCount() - last_tel) >= pdMS_TO_TICKS(COMM_TEL_PERIOD_MS)))
        {
            last_tel = xTaskGetTickCount();
            print_telemetry();
        }
    }
}

status_t comm_init(void)
{
    s_rx_q = xQueueCreate(COMM_RX_QUEUE_LEN, sizeof(uint8_t));
    return (s_rx_q != NULL) ? ST_OK : ST_ERR_HW;
}

status_t comm_start(void)
{
    status_t st = bsp_uart_start(rx_isr_cb);

    if (st == ST_OK)
    {
        st = (xTaskCreate(comm_task, "comm", COMM_STACK_WORDS, NULL, TASK_PRIO_COMM, NULL) == pdPASS)
                 ? ST_OK : ST_ERR_HW;
    }
    return st;
}
