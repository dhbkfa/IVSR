/**
 * @file    task_comm.h
 * @brief   Text console on the debug UART (bring-up tool, M0-M2).
 *          Replaced by the binary NUC protocol in M5.
 */
#ifndef TASK_COMM_H
#define TASK_COMM_H

#include "status.h"

status_t comm_init(void);
status_t comm_start(void);

#endif /* TASK_COMM_H */
