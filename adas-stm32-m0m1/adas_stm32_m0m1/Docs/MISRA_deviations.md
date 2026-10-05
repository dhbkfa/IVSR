# MISRA C deviation log (project policy: MISRA C:2025 on own code; CubeMX/HAL excluded)

Status: code is written MISRA-aware but NOT formally checked. Next step: cppcheck in CI,
then a commercial checker when the guideline document is available.

| ID    | Where                     | Rule (check numbering in your edition) | Reason | Plan |
|-------|---------------------------|----------------------------------------|--------|------|
| D-001 | BSP/bsp_uart.c            | cast away const                        | HAL prototype takes non-const pointer | keep, confined to BSP |
| D-002 | App/task_comm.c           | stdio (snprintf), stdlib (strtol)      | bring-up console | remove in M5 (binary protocol) |
| D-003 | BSP/*.c                   | HAL/CMSIS macros (register access)     | vendor code | keep, confined to BSP |
| D-004 | App/*.c                   | FreeRTOS macros (taskENTER_CRITICAL...) | vendor code | keep |
