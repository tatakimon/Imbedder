# RL Development Trial Log

## Version History

| Version | Date | Description | Result |
|---------|------|-------------|--------|
| v1.0.2 | 2026-03-30 | Phase 3 RL State Vector with CSV telemetry | SUCCESS |

---

- [2026-03-30 14:29] v1.0.2 : Phase 3 RL State Vector with CSV telemetry : PASS : STATE CSV every 100ms with tick count, last_action (A/B/N), dummy sensor 0-99. 'A' toggles LED and updates last_action to 'A', 'B' updates to 'B'. Non-blocking UART polling per LESSONS_LEARNED. Dummy sensor replaces future vibration sensor.
- [2026-03-29 20:37] v1.0.1 : Phase 2 UART RX with interrupt callbacks : PASS : Added USART2_IRQHandler to stm32u5xx_it.c USER CODE 1, added NVIC_EnableIRQ in main.c USER CODE 2. 'A' toggles LED and returns "Action A Received", 'B' returns "Action B Received", heartbeat "Awaiting Action..." works at 2s interval.
- [2026-03-29 20:35] v1.0.0 : Phase 2 UART RX initial build : FAIL : Callback HAL_UART_RxCpltCallback not firing - USART2_IRQHandler was missing from stm32u5xx_it.c
- [2026-03-29 20:29] v1.0.0 : Phase 2 UART RX interrupt implementation : FAIL : HAL_UART_Receive_IT called but no UART RX response - later found USART2_IRQHandler missing
- [2026-03-29 20:21] v1.0.0 : Phase 2 binary generation test : FAIL : objcopy generated bin without rodata section - fixed by using full objcopy -O binary on ELF
- [2026-03-29 20:11] v1.0.0 : Phase 2 initial flash attempt : FAIL : Old firmware still running - mass erase and reflash resolved
