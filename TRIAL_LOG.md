# RL Development Trial Log

## Version History

| Version | Date | Description | Result |
|---------|------|-------------|--------|
| v0.0.1 | 2026-04-01 | Latency_Test_Env Staging | STAGED |
| v1.0.4 | 2026-04-01 | Tree Architecture Restructuring | SUCCESS |
| v1.0.3 | 2026-03-30 | Phase 5 Real Accelerometer Integration | SUCCESS |

---

- [2026-04-01 11:XX] v0.0.1 : Latency_Test_Env Staging : STAGED : Created Environments/Latency_Test_Env/ from Generic_Base/. Added verify_latency.py with sigrok-cli integration (24MHz, 240k samples, Ch0 trigger). Script sends 'A' over UART, captures GPIO response via sigrok-cli, parses CSV for latency calculation. Graceful failure if hardware not detected. README.md created with HIL architecture diagrams. Awaiting physical logic analyzer.
- [2026-04-01 09:28] v1.0.4 : Tree Architecture Restructuring : PASS : Renamed Base_Firmware to Generic_Base. Created Environments/Accelerometer_Env/ as first completed node. Moved Phase 5/6 code and agent.py into Accelerometer_Env/. Updated CLAUDE.md with new Tree Architecture rules. User specified base when starting future trials.
- [2026-03-30 16:24] v1.0.3 : Phase 5 Real Accelerometer Integration : PASS : ISM330DHCX sensor initialized and streaming real accelerometer data. CSV format updated to: STATE,<TickCount>,<LastAction>,<AccX>,<AccY>,<AccZ>. Z-axis shows ~8400 (≈1g gravity) when board flat. Error handling prevents crash on sensor init failure.
- [2026-03-29 20:37] v1.0.1 : Phase 2 UART RX with interrupt callbacks : PASS : Added USART2_IRQHandler to stm32u5xx_it.c USER CODE 1, added NVIC_EnableIRQ in main.c USER CODE 2. 'A' toggles LED and returns "Action A Received", 'B' returns "Action B Received", heartbeat "Awaiting Action..." works at 2s interval.
- [2026-03-29 20:35] v1.0.0 : Phase 2 UART RX initial build : FAIL : Callback HAL_UART_RxCpltCallback not firing - USART2_IRQHandler was missing from stm32u5xx_it.c
- [2026-03-29 20:29] v1.0.0 : Phase 2 UART RX interrupt implementation : FAIL : HAL_UART_Receive_IT called but no UART RX response - later found USART2_IRQHandler missing
- [2026-03-29 20:21] v1.0.0 : Phase 2 binary generation test : FAIL : objcopy generated bin without rodata section - fixed by using full objcopy -O binary on ELF
- [2026-03-29 20:11] v1.0.0 : Phase 2 initial flash attempt : FAIL : Old firmware still running - mass erase and reflash resolved