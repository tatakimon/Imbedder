# STEVAL-STWINBX1 RL Environment Firmware Summary

## Version: v1.0.0 (Base_Firmware)

---

## Phase 1: Base Hardware Control (v1.0.0)
**Date:** 2026-03-29

Basic LED and UART initialization on STEVAL-STWINBX1 (STM32U585AI):
- Green LED (GPIOH Pin 12) control
- Orange LED (GPIOH Pin 10) control
- UART2 at 115200 baud on PD5/PD6
- HAL_Delay-based timing

---

## Phase 2: UART RX Command Interface (v1.0.0)
**Date:** 2026-03-29

Two-way UART communication with command handling:
- 'A' command: Toggles Green LED, returns "Action A Received"
- 'B' command: Returns "Action B Received"
- Interrupt-based UART RX (USART2_IRQHandler required in stm32u5xx_it.c)
- **Critical Learning:** USART2 global interrupts disabled in CubeMX profile - use zero-timeout polling instead

---

## Phase 3: RL State Vector (v1.0.2)
**Date:** 2026-03-30

Machine-readable state telemetry for RL environment:
- **CSV Format:** `STATE,<TickCount>,<LastActionReceived>,<DummySensorValue>\r\n`
- **Transmission Rate:** 100ms intervals (non-blocking)
- **State Fields:**
  - `TickCount`: HAL_GetTick() value
  - `LastActionReceived`: 'A', 'B', or 'N' (None)
  - `DummySensorValue`: 0-99, auto-incrementing, wraps at 100
- **Command Handling:**
  - 'A': Toggle Green LED, update last_action to 'A'
  - 'B': Update last_action to 'B'
- **UART:** Non-blocking polling method per LESSONS_LEARNED

### Key Implementation Details
- No HAL_Delay() used - all timing via non-blocking HAL_GetTick() delta comparisons
- UART RX via `HAL_UART_Receive(&huart2, &rx_byte, 1, 0)` (zero-timeout polling)
- Dummy sensor serves as placeholder for future vibration sensor integration

---

## Hardware Notes
- **Board:** STEVAL-STWINBX1
- **MCU:** STM32U585AI
- **UART:** USART2 on PD5 (TX), PD6 (RX) at 115200 baud
- **LEDs:** Green (PH12), Orange (PH10)

## File Structure
```
Base_Firmware/
├── Core/Src/main.c          # Main application (USER CODE regions only)
├── Drivers/                  # STM32U5 HAL drivers
├── Dell_2_Steval.ioc        # CubeMX configuration
└── STM32U585AIIXQ_FLASH.ld  # Linker script
```
