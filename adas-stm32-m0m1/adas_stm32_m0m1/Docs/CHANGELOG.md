# Changelog
## M0-M1 (2026-10-04)
- Layered skeleton (Config / BSP / Lib / Components / Services / App / Test / Tools)
- BSP: PWM, encoder (TIM3), GPIO bridge gates, UART console, DWT timing
- Components: encoder (pure C, host-tested), BTS7960 motor driver (host-tested)
- Services: wheel (encoder + motor); App: vehicle state machine, control task 5 ms, console task
- Failsafe: command timeout, bridge disabled outside RUN, duty limit 30 % by default
