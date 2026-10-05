# ADAS low-level controller: M0 + M1 (one wheel)

STM32F411CEU6 + FreeRTOS. One BTS7960 + JGB37-545 motor + quadrature encoder (TIM3).
Goal of these two milestones: a verified RTOS skeleton and a measured, safely driven wheel.

## 1. What is in the box

```
Config/        board_config.h (pins/timers)  vehicle_config.h (CPR...)  control_config.h (limits)
BSP/           ONLY place that calls HAL: bsp_pwm, bsp_encoder, bsp_gpio, bsp_uart, bsp_time
Lib/           status.h  (common status_t)
Components/    encoder/ (pure C)   motor_driver/ (BTS7960 logic)
Services/      wheel/ (encoder + motor = one reusable wheel)
App/           vehicle_state, task_control (5 ms), task_comm (console), app_tasks (entry)
Test/          host unit tests:  make -C Test      (already passing: encoder, motor)
Tools/         bench.py  (CSV log + dead-band sweep)
Docs/          MISRA_deviations.md, CHANGELOG.md
```

Rule: upper layers call lower layers. Nothing outside `BSP/` includes HAL functions.
To reuse with another motor/encoder, change only `wheel_cfg_t` values (and the BSP tables for a new MCU).

## 2. CubeMX settings (.ioc)

| Item | Setting |
|---|---|
| SYS | Debug = Serial Wire. **Timebase Source = TIM11** (SysTick belongs to FreeRTOS) |
| Clock | Your decision (100 MHz, or 96 MHz if you need USB). The code reads ARR at run time, nothing to recompute |
| TIM1 | Internal clock, **PWM Generation CH1**, pin **PA8**, PSC = 0, ARR = f_tim/20000 - 1 (4999 at 100 MHz, 4799 at 96 MHz), pulse 0, PWM mode 1, auto-reload preload on |
| TIM3 | **Combined Channels = Encoder Mode**, mode **TI1 and TI2**, PSC = 0, Counter Period = **65535**, IC1/IC2 filter 6-10. Pins **PB4 (CH1) / PB5 (CH2)**: click the pins in CubeMX to move them from the default PA6/PA7 |
| USART2 | Asynchronous, 115200 8N1, PA2 TX / PA3 RX, **NVIC global interrupt ON**, priority number >= 5 (not more urgent than `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY`) |
| GPIO outputs | PB12 (ENR), PB13 (ENL), PB14 (bridge EN), PC13 (LED). Push-pull, **initial level LOW**, no labels needed |
| FreeRTOS | Interface **CMSIS_V2**, TICK_RATE_HZ **1000**, heap scheme heap_4, **TOTAL_HEAP_SIZE 16384**, enable `vTaskDelayUntil`, `vTaskDelay`, `uxTaskGetStackHighWaterMark`. Keep the default task (harmless). Recommended: CHECK_FOR_STACK_OVERFLOW = Option 2 |

All pins above are my placeholders: verify them in CubeMX, and mirror any change in `Config/board_config.h`.

## 3. Add to the project (STM32CubeIDE)

1. Copy the folders into the project root. For each of `Config BSP Lib Components Services App` make sure the folder is a source folder (Project > Properties > C/C++ General > Paths and Symbols > **Source Location**).
2. Same dialog > **Includes (GNU C)**: add `Config`, `BSP`, `Lib`, `App`, `Components/encoder`, `Components/motor_driver`, `Services/wheel`.
3. In `Core/Src/freertos.c`:
   ```c
   /* USER CODE BEGIN Includes */
   #include "app_tasks.h"
   /* USER CODE END Includes */
   ...
   void MX_FREERTOS_Init(void) {
     ...
     /* USER CODE BEGIN RTOS_THREADS */
     app_init();
     /* USER CODE END RTOS_THREADS */
   }
   ```
4. Optional but recommended, same file, in `vApplicationStackOverflowHook`: call `bsp_bridge_enable(false);` first.

## 4. Console (115200 8N1, local echo on, Enter = CR or LF)

`arm | disarm | clear | d <-1000..1000> | rst | cpr <x> | db <permille> | lim <permille> | tmo <ms> | tel <0|1> | info | help`

Telemetry line (`tel 1`, every 100 ms), all integers:
`T,tick_ms,state,faults,counts,delta,angle_x100,rpm_x100,duty,exec_us,jit_us,warn,cpr_x100`
State: 0 INIT, 1 READY, 2 RUN, 3 SAFE_STOP. Faults: bit0 command timeout, bit1 encoder, bit2 init.

## 5. M0 acceptance (no motor power connected)

1. LED blinks (500 ms toggle).
2. `info` prints `state=1 faults=0x0` and positive stack/heap numbers: write down `ctl_stack_free` and `comm_stack_free`; if either is below ~50 words, enlarge the stack in the .c file.
3. `tel 1` streams lines. `jit_us` should be far below one tick (1000 us) and `exec_us` far below 5000 us. If not, something is interfering: investigate before going on.

## 6. M1 procedure (motor)

**Safety first:** wheel lifted off the ground, bench supply with current limit, duty limit stays at 30 %, a hand on the supply switch. Bridge starts disabled and is only enabled in state RUN.

1. **Counting direction and CPR (motor power off):** `tel 1`, turn the wheel by hand exactly one turn, read `counts`. Repeat 10 turns and divide by 10. Enter it with `cpr <value>` for the session, then write it into `VEH_WHEEL_CPR` in `Config/vehicle_config.h`. Turn the other way: counts must decrease. If positive duty later moves the wheel the "wrong" way relative to counts, fix with `VEH_ENC_DIRECTION` or by swapping the motor wires.
2. **First spin:** power on, `tmo 10000`, `arm`, `d 50`, watch `rpm_x100` and `counts`; send `d 0`, then `disarm`.
3. **Dead-band:** `python Tools/bench.py COMx --sweep --max-duty 250`. Put the result into `MOT_DEFAULT_DEADBAND`.
4. **Failsafe:** `arm`, `d 100`, stop typing: after the timeout the state must become 3 with `faults=0x1` and the wheel must stop. `clear` returns to READY.
5. **Record:** save the CSV, CPR, dead-band, `exec_us`, `jit_us`, stack numbers in `Docs/` (they are the baseline for M2).

M1 is done when: CPR measured within ~1 %, direction consistent, dead-band recorded, failsafe works, timing numbers logged.

## 7. Wiring notes

- Intended: PWM (PA8) feeds two AND gates (74HC08): `RPWM = PWM & ENR`, `LPWM = PWM & ENL`. ENR/ENL = PB12/PB13. BTS7960 `R_EN`/`L_EN` tied together to PB14, with a pull-down so a reset or hang leaves the bridge off.
- **No gates yet?** Forward-only test: PA8 -> RPWM, LPWM -> GND, R_EN/L_EN -> PB14. Reverse will not move the motor.
- Encoder at 3.3 V (or confirm its output levels), common ground, keep encoder wires away from motor wires.

## 8. Known limits (be aware)

- Tested here: encoder and motor logic on a PC (`make -C Test`, strict warnings) and the HAL/RTOS files for syntax against stubs. **Not compiled against your real HAL and not run on hardware**: expect small integration fixes on first build; send me the compiler output.
- "Driving but not moving" (encoder wire lost) is not detected yet: that is the stall/plausibility check planned for M4.
- Not MISRA-certified: written MISRA-aware, deviations listed in `Docs/MISRA_deviations.md`.
- One wheel only. Going to four wheels (M3): extend the tables in `bsp_pwm.c`, `bsp_encoder.c`, `bsp_gpio.c`, `BOARD_NUM_WHEELS`, and use an array of `wheel_t`.
- The console is a bring-up tool and is replaced by the NUC protocol in M5.

## 9. Next (M2)

PID speed loop on one wheel (`Lib/pid`), step-response logging, then M3: four wheels + mecanum kinematics.
