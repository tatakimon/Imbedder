# STEVAL-STWINBX1 RL Environment Firmware Summary

## Architecture: Tree System

The project uses a **Tree Architecture** for generic, reusable RL environments:

- **Generic_Base/**: Hardware-agnostic Phase 4 code (v1.0.0)
- **Environments/**: Completed environment branches (nodes)
  - **Accelerometer_Env/**: ISM330DHCX accelerometer RL environment
  - **Latency_Test_Env/**: Hardware-in-the-loop (HIL) latency testing
  - **Battery_Monitor_Env/**: ADC battery voltage monitoring

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

### Battery_Monitor_Env (v0.0.2)

**STBC02 Battery Charger Driver Integration**

**Date:** 2026-04-04

STBC02 battery monitoring with voltage, level percentage, and charging state:

#### Battery Monitoring (v0.0.2)
- **Driver**: STBC02 Battery Charger BSP driver from BASE_one
- **Implementation**: STBC02-style API wrappers using ADC1 channel 3 (PC2)
- **Voltage Range**: 3250mV (min) to 4225mV (max)
- **CSV Format**: `STATE,<TickCount>,<LastAction>,<Voltage_mV>,<Level_Pct>,<Status_Id>`
- **Status States** (via CHG pin frequency detection):
  - 0 = NotValidInput, 1 = ValidInput, 2 = VbatLow, 3 = EndOfCharge
  - 4 = ChargingPhase, 5 = OverchargeFault, 6 = ChargingTimeout
  - 7 = BatteryVoltageBelowVpre, 8 = ChargingThermalLimitation
  - 9 = BatteryTemperatureFault
- **Threshold Policy**: Level < 20% AND NOT Charging -> 'A' (Power Save), Otherwise -> 'B' (Normal)
- **Status**: Firmware working, battery reads 3250mV (minimum - no battery or depleted)

#### Previous Implementation (v0.0.1)
- Raw ADC1 approach with voltage divider formula
- 4-element CSV format: `STATE,<TickCount>,<LastAction>,<Battery_mV>`

---

### Latency_Test_Env (Staged)

**Hardware-in-the-loop (HIL) testing environment**

**Date:** 2026-04-01 (Staged)

sigrok-cli based latency profiler for measuring RL loop timing:

#### Latency Profiling (v0.0.1)
- **verify_latency.py**: Automated sigrok-cli profiler
- **Configuration**: 24MHz sample rate, 240k samples, Ch0 falling edge trigger
- **Measurement**: UART frame end to GPIO response time delta
- **Status**: Staged - awaiting logic analyzer hardware

#### sigrok-cli Parameters
| Parameter | Value |
|-----------|-------|
| Sample Rate | 24 MHz |
| Samples | 240,000 |
| Trigger | Channel 0, Falling Edge |
| Output | CSV |

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
├── Accelerometer_Env/          # Completed: Tilt detection RL
│   ├── Core/Src/main.c          # Phase 5/6 working code
│   ├── RL_Brain/agent.py        # Python threshold policy agent
│   ├── test_tilt.py             # Tilt detection test script
│   └── README.md                # Environment documentation
│
├── Latency_Test_Env/           # Staged: HIL latency testing
│   ├── Core/Src/main.c          # Generic_Base firmware
│   ├── verify_latency.py        # sigrok-cli profiler script
│   └── README.md                # Environment documentation
│
└── Battery_Monitor_Env/       # ADC battery voltage monitoring
    ├── Core/Src/main.c          # ADC battery monitoring code
    ├── RL_Brain/agent.py        # Battery threshold policy agent
    └── README.md                # Environment documentation
```

## Roadmap: Next Phase

After Latency_Test_Env validation:
- Integrate next sensor (Gyroscope or Magnetometer) into new environment branch
- Use latency measurements to validate RL loop timing requirements