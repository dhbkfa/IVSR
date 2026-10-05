/**
 * @file    task_control.h
 * @brief   Fixed-period control task (5 ms). Owns the wheel(s).
 *          Other tasks talk to it ONLY through ctl_post() / ctl_get_telemetry().
 */
#ifndef TASK_CONTROL_H
#define TASK_CONTROL_H

#include <stdbool.h>
#include <stdint.h>
#include "status.h"

typedef enum
{
    CTL_CMD_DUTY = 0,       /* i = duty permille (-1000..1000)          */
    CTL_CMD_ARM,
    CTL_CMD_DISARM,
    CTL_CMD_CLEAR,
    CTL_CMD_RESET_ENC,
    CTL_CMD_SET_CPR,        /* f = counts per wheel rev                 */
    CTL_CMD_SET_DEADBAND,   /* i = permille                             */
    CTL_CMD_SET_LIMIT,      /* i = permille                             */
    CTL_CMD_SET_TIMEOUT     /* i = milliseconds                         */
} ctl_cmd_type_t;

typedef struct
{
    ctl_cmd_type_t type;
    int32_t        i;
    float          f;
} ctl_cmd_t;

typedef struct
{
    uint32_t tick_ms;
    uint32_t faults;
    uint8_t  state;             /* veh_state_t                              */
    int32_t  counts;
    int32_t  delta;             /* counts in the last control tick          */
    int32_t  angle_x100;        /* degrees  * 100                           */
    int32_t  rpm_x100;          /* filtered * 100                           */
    int32_t  cpr_x100;          /* counts per rev * 100                     */
    int16_t  duty_permille;     /* motor output (after ramp)                */
    uint32_t exec_us_max;       /* worst execution time of one tick         */
    uint32_t jitter_us_max;     /* worst |period - nominal|                 */
    uint32_t warn_count;        /* number of overspeed warnings             */
    uint32_t stack_free_words;  /* minimum free stack of this task          */
} ctl_telemetry_t;

status_t ctl_init(void);        /* creates queues */
status_t ctl_start(void);       /* creates the task */
bool     ctl_post(const ctl_cmd_t *cmd);           /* non-blocking          */
bool     ctl_get_telemetry(ctl_telemetry_t *out);  /* latest snapshot       */

#endif /* TASK_CONTROL_H */
