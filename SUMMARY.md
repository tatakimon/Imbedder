# STEVAL-STWINBX1 RL Environment Firmware Summary

## Architecture: Tree System

The project uses a **Tree Architecture** for generic, reusable RL environments:

- **Generic_Base/**: Hardware-agnostic Phase 4 code (v1.0.0)
- **Environments/**: Completed environment branches (nodes)
  - **Accelerometer_Env/**: ISM330DHCX accelerometer RL environment

---

## Environments

### Accelerometer_Env (v1.0.1)

**First completed environment node - Real Accelerometer Integration**

**Date:** 2026-03-30 to 2026-04-01

Real ISM330DHCX 3-axis accelerometer integration for tilt detection RL:

#### Phase 5: Real Accelerometer Integration (v1.0.1 → v1.0.3)
- ISM330DHCX sensor initialized via SPI2 with error handling
- 6-element CSV telemetry: `STATE,<TickCount>,<LastAction>,<AccX>,<AccY>,<AccZ>`
- AccZ ≈ 8400 (≈1g) when board flat, drops when tilted
- Threshold policy: AccZ < 5000 = 'A' (tilted), AccZ >= 5000 = 'B' (flat)

#### Phase 4: RL State Vector (v1.0.0)
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

---

## Generic_Base (v1.0.0)

### Phase 2: UART RX Command Interface (v1.0.0)
**Date:** 2026-03-29

Two-way UART communication with command handling:
- 'A' command: Toggles Green LED, returns "Action A Received"
- 'B' command: Returns "Action B Received"
- Interrupt-based UART RX (USART2_IRQHandler required in stm32u5xx_it.c)
- **Critical Learning:** USART2 global interrupts disabled in CubeMX profile - use zero-timeout polling instead

### Phase 1: Base Hardware Control (v1.0.0)
**Date:** 2026-03-29

Basic LED and UART initialization on STEVAL-STWINBX1 (STM32U585AI):
- Green LED (GPIOH Pin 12) control
- Orange LED (GPIOH Pin 10) control
- UART2 at 115200 baud on PD5/PD6
- HAL_Delay-based timing

---

## Hardware Notes
- **Board:** STEVAL-STWINBX1
- **MCU:** STM32U585AI
- **UART:** USART2 on PD5 (TX), PD6 (RX) at 115200 baud
- **LEDs:** Green (PH12), Orange (PH10)
- **Accelerometer:** ISM330DHCX via SPI2

## File Structure
```
Generic_Base/                    # Hardware-agnostic base (v1.0.0)
├── Core/Src/main.c
├── Drivers/
├── Dell_2_Steval.ioc
└── STM32U585AIIXQ_FLASH.ld

Environments/
└── Accelerometer_Env/          # First completed node
    ├── Core/Src/main.c          # Phase 5/6 working code
    ├── RL_Brain/agent.py        # Python threshold policy agent
    ├── test_tilt.py             # Tilt detection test script
    ├── Dell_2_Steval.ioc        # CubeMX configuration
    └── STM32U585AIIXQ_FLASH.ld  # Linker script
```