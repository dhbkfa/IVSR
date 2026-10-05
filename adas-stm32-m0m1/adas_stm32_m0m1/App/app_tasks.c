#include "app_tasks.h"
#include "task.h"

#include "board_config.h"
#include "bsp_encoder.h"
#include "bsp_gpio.h"
#include "bsp_pwm.h"
#include "bsp_time.h"
#include "task_comm.h"
#include "task_control.h"
#include "vehicle_state.h"

#define HEARTBEAT_STACK_WORDS   (128U)
#define HEARTBEAT_PERIOD_MS     (500U)

/* Unrecoverable start-up error: leave the bridge off and stop. */
static void app_fatal(void)
{
    bsp_bridge_enable(false);
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
        /* wait for watchdog/reset */
    }
}

static void heartbeat_task(void *arg)
{
    (void)arg;
    for (;;)
    {
        bsp_led_toggle();
        vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_PERIOD_MS));
    }
}

void app_init(void)
{
    uint8_t i;

    bsp_time_init();
    bsp_gpio_init();                 /* bridge disabled, gates in BRAKE state */
    veh_init();

    for (i = 0U; i < BOARD_NUM_WHEELS; i++)
    {
        if ((bsp_pwm_start(i) != ST_OK) || (bsp_encoder_start(i) != ST_OK))
        {
            app_fatal();
        }
    }

    if ((ctl_init() != ST_OK) || (comm_init() != ST_OK))
    {
        app_fatal();
    }
    if ((ctl_start() != ST_OK) || (comm_start() != ST_OK))
    {
        app_fatal();
    }
    if (xTaskCreate(heartbeat_task, "heartbeat", HEARTBEAT_STACK_WORDS, NULL,
                    TASK_PRIO_HEARTBEAT, NULL) != pdPASS)
    {
        app_fatal();
    }

    veh_init_done();                 /* INIT -> READY */
}
