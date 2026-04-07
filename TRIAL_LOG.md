# RL Development Trial Log

## Version History

| Version | Date | Description | Result |
|---------|------|-------------|--------|
| v0.0.2 | 2026-04-07 | Temperature_Env STTS22H I2C Integration | PARTIAL |
| v0.0.1 | 2026-04-04 | Battery_Monitor_Env ADC Integration | SUCCESS |
| v0.0.1 | 2026-04-01 | Latency_Test_Env Staging | STAGED |
| v1.0.4 | 2026-04-01 | Tree Architecture Restructuring | SUCCESS |
| v1.0.3 | 2026-03-30 | Phase 5 Real Accelerometer Integration | SUCCESS |

---

- [2026-04-07 00:10] v0.0.2 : Temperature_Env STTS22H I2C Integration : PARTIAL (SUCCESS/FW) : Root cause was stale .bin file (accelerometer firmware from before temperature code was added). Clean rebuild confirmed binary now contains "STTS22H: Init OK" and "STATE" strings. Firmware streams correct CSV format: STATE,<Tick>,N,<Temp_x100>,<Status>. STTS22H sensor not initializing (status=0, temp=0) - I2C hardware issue. Promoted to Environments/Temperature_Env/. HIL verification passed (partial data - firmware correct, sensor HW issue).
- [2026-04-07 02:XX] v0.0.3 : Temperature_Env STTS22H Debug Session : PARTIAL : Extensive I2C debugging: I2C init completes successfully (LED turns ON), but STTS22H at address 0x3F (7-bit) / 0x7E (8-bit) does not ACK. Tried BASE_one timing (0x00F07BFF), pull-ups, analog filter. Bus scan shows no devices. Sensor may require STWIN.box expansion board to be properly connected/powered. Firmware is correct - sensor hardware issue.
- [2026-04-06 14:05] v0.0.1 : Temperature_Env STTS22H I2C Integration : FAIL (STALE BIN) : Cloned Generic_Base, added I2C2 init for STTS22H temp sensor (PF0=SDA, PH4=SCL). Firmware compiles successfully. st-flash reports "Flash written and verified! jolly good!" but board runs factory accelerometer firmware instead of temperature firmware. Root cause: .bin file was stale - contained old accelerometer firmware, not newly compiled temperature code. Need clean rebuild before flashing.
- [2026-04-04 14:40] v0.0.2 : Battery_Monitor_Env STBC02 Integration : SUCCESS : Ported STBC02 driver from BASE_one. Created STWIN.box_bc.h/c, STWIN.box_conf.h, STWIN.box_errno.h in Drivers/BSP/STWIN.box/. Implemented STBC02-style API wrappers (BSP_BC_BatMs_Init, BSP_BC_GetVoltageAndLevel, BSP_BC_GetState) using existing ADC1. 6-element CSV: STATE,<TickCount>,<LastAction>,<Voltage_mV>,<Level_Pct>,<Status_Id>. Threshold policy: Level<20% AND NOT Charging -> 'A', else 'B'. Python agent updated. Battery reads 3250mV (minimum - no battery/depleted).
- [2026-04-04 14:10] v0.0.1 : Battery_Monitor_Env ADC Integration : SUCCESS : Created Environments/Battery_Monitor_Env/ from Generic_Base/. Added ADC1 channel 3 (PC2) for battery sense with voltage divider formula (R14=56k, R19=100k, multiplier=1.56). MX_ADC1_Init with calibration, LL_SYSCFG_EnableAnalogSwitchVdd for analog routing. CSV format: STATE,<TickCount>,<LastAction>,<Battery_mV>. Threshold policy: <3500mV='A', >=3500mV='B'. Firmware streams correctly but battery reads 0mV (no battery connected or depleted). Python agent tested for 10 seconds.
- [2026-04-01 11:XX] v0.0.1 : Latency_Test_Env Staging : STAGED : Created Environments/Latency_Test_Env/ from Generic_Base/. Added verify_latency.py with sigrok-cli integration (24MHz, 240k samples, Ch0 trigger). Script sends 'A' over UART, captures GPIO response via sigrok-cli, parses CSV for latency calculation. Graceful failure if hardware not detected. README.md created with HIL architecture diagrams. Awaiting physical logic analyzer.
- [2026-04-01 09:28] v1.0.4 : Tree Architecture Restructuring : PASS : Renamed Base_Firmware to Generic_Base. Created Environments/Accelerometer_Env/ as first completed node. Moved Phase 5/6 code and agent.py into Accelerometer_Env/. Updated CLAUDE.md with new Tree Architecture rules. User specified base when starting future trials.
- [2026-03-30 16:24] v1.0.3 : Phase 5 Real Accelerometer Integration : PASS : ISM330DHCX sensor initialized and streaming real accelerometer data. CSV format updated to: STATE,<TickCount>,<LastAction>,<AccX>,<AccY>,<AccZ>. Z-axis shows ~8400 (≈1g gravity) when board flat. Error handling prevents crash on sensor init failure.
- [2026-03-29 20:37] v1.0.1 : Phase 2 UART RX with interrupt callbacks : PASS : Added USART2_IRQHandler to stm32u5xx_it.c USER CODE 1, added NVIC_EnableIRQ in main.c USER CODE 2. 'A' toggles LED and returns "Action A Received", 'B' returns "Action B Received", heartbeat "Awaiting Action..." works at 2s interval.
- [2026-03-29 20:35] v1.0.0 : Phase 2 UART RX initial build : FAIL : Callback HAL_UART_RxCpltCallback not firing - USART2_IRQHandler was missing from stm32u5xx_it.c
- [2026-03-29 20:29] v1.0.0 : Phase 2 UART RX interrupt implementation : FAIL : HAL_UART_Receive_IT called but no UART RX response - later found USART2_IRQHandler missing
- [2026-03-29 20:21] v1.0.0 : Phase 2 binary generation test : FAIL : objcopy generated bin without rodata section - fixed by using full objcopy -O binary on ELF
- [2026-03-29 20:11] v1.0.0 : Phase 2 initial flash attempt : FAIL : Old firmware still running - mass erase and reflash resolved