#include "vehicle_state.h"
#include "FreeRTOS.h"
#include "task.h"

static veh_state_t s_state  = VEH_STATE_INIT;
static uint32_t    s_faults = 0U;

void veh_init(void)
{
    taskENTER_CRITICAL();
    s_state  = VEH_STATE_INIT;
    s_faults = 0U;
    taskEXIT_CRITICAL();
}

void veh_init_done(void)
{
    taskENTER_CRITICAL();
    if ((s_state == VEH_STATE_INIT) && (s_faults == 0U))
    {
        s_state = VEH_STATE_READY;
    }
    taskEXIT_CRITICAL();
}

veh_state_t veh_get_state(void)
{
    return s_state;     /* 32-bit aligned read is atomic on Cortex-M */
}

uint32_t veh_get_faults(void)
{
    return s_faults;
}

bool veh_request_arm(void)
{
    bool ok = false;

    taskENTER_CRITICAL();
    if ((s_state == VEH_STATE_READY) && (s_faults == 0U))
    {
        s_state = VEH_STATE_RUN;
        ok = true;
    }
    taskEXIT_CRITICAL();
    return ok;
}

void veh_request_disarm(void)
{
    taskENTER_CRITICAL();
    if (s_state == VEH_STATE_RUN)
    {
        s_state = VEH_STATE_READY;
    }
    taskEXIT_CRITICAL();
}

void veh_raise_fault(uint32_t mask)
{
    taskENTER_CRITICAL();
    s_faults |= mask;
    s_state   = VEH_STATE_SAFE_STOP;
    taskEXIT_CRITICAL();
}

bool veh_clear_faults(void)
{
    bool ok = false;

    taskENTER_CRITICAL();
    if (s_state == VEH_STATE_SAFE_STOP)
    {
        s_faults = 0U;
        s_state  = VEH_STATE_READY;
        ok = true;
    }
    taskEXIT_CRITICAL();
    return ok;
}
