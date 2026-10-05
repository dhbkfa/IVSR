/**
 * @file    vehicle_state.h
 * @brief   Vehicle state machine. Single owner of "is it allowed to move".
 *
 *   INIT --init_done--> READY --arm--> RUN --disarm--> READY
 *   (any state) --fault--> SAFE_STOP --clear--> READY
 *
 * Thread-safe: short critical sections. Do not call from an ISR.
 */
#ifndef VEHICLE_STATE_H
#define VEHICLE_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    VEH_STATE_INIT = 0,
    VEH_STATE_READY,
    VEH_STATE_RUN,
    VEH_STATE_SAFE_STOP
} veh_state_t;

#define VEH_FAULT_CMD_TIMEOUT   (1UL << 0U)
#define VEH_FAULT_ENCODER       (1UL << 1U)
#define VEH_FAULT_INIT          (1UL << 2U)

void        veh_init(void);
void        veh_init_done(void);          /* INIT -> READY            */
veh_state_t veh_get_state(void);
uint32_t    veh_get_faults(void);
bool        veh_request_arm(void);        /* READY -> RUN             */
void        veh_request_disarm(void);     /* RUN -> READY             */
void        veh_raise_fault(uint32_t mask);   /* any -> SAFE_STOP     */
bool        veh_clear_faults(void);       /* SAFE_STOP -> READY       */

#endif /* VEHICLE_STATE_H */
