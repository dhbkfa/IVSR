/**
 * @file    app_tasks.h
 * @brief   Task priorities/stacks and the single entry point called from
 *          CubeMX's freertos.c (USER CODE BEGIN RTOS_THREADS).
 *
 * Priority rule (higher number = higher priority):
 *     control > safety (M4) > comm > heartbeat
 */
#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "FreeRTOS.h"

#define TASK_PRIO_CONTROL    ((UBaseType_t)configMAX_PRIORITIES - 2U)
#define TASK_PRIO_SAFETY     ((UBaseType_t)configMAX_PRIORITIES - 3U)   /* M4 */
#define TASK_PRIO_COMM       (tskIDLE_PRIORITY + 2U)
#define TASK_PRIO_HEARTBEAT  (tskIDLE_PRIORITY + 1U)

/** Initialise BSP, create queues and tasks. Call BEFORE the scheduler starts. */
void app_init(void);

#endif /* APP_TASKS_H */
