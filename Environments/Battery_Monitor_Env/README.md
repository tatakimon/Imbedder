# Battery Monitor Environment (Battery_Monitor_Env)

STBC02 Battery Charger driver integration for battery monitoring with voltage, level, and charging state.

---

## Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                 BATTERY_MONITOR_ENV ARCHITECTURE                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌──────────────┐         ┌──────────────┐         ┌─────────┐ │
│   │  STEVAL-     │   UART  │   Python     │  UART   │ Python  │ │
│   │  STWINBX1    │◄───────│   Agent      │◄───────►│ RL      │ │
│   │  (STM32U585) │  USB    │  (agent.py)  │  USB    │ Brain   │ │
│   └──────────────┘         └──────────────┘         └─────────┘ │
│         │                        │                        │      │
│         │                        │                        │      │
│   ┌─────▼─────┐           ┌─────▼──────┐          ┌─────▼────┐ │
│   │  STBC02    │           │ Threshold  │          │ Action   │ │
│   │  Battery   │           │ Policy     │          │ Feedback │ │
│   │  Charger   │           │ Level<20%  │          └──────────┘ │
│   └────────────┘           │ && NOT Chrg │                      │
│                             └─────────────┘                      │
└─────────────────────────────────────────────────────────────────┘
```

---

## Hardware Configuration

```
┌────────────────────────────────────────────────────────────────┐
│                    BATTERY SENSE CIRCUIT                        │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│    Battery (VBAT) ──► [R14=56k] ──► [R19=100k] ──► GND     │
│                               │                                 │
│                               ├──► PC2 (ADC4_IN3)             │
│                               │                                 │
│                         Voltage Divider                         │
│                         Ratio: (56+100)/100 = 1.56            │
│                                                                 │
│    STBC02 Formula: voltage_mv = (adc_voltage * 156) / 100    │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

---

## STBC02 Driver

The environment uses the official STMicroelectronics STBC02 Battery Charger driver from `BASE_one`:

- **Driver Location**: `Drivers/BSP/STWIN.box/STWIN.box_bc.c`
- **API**: STBC02-style functions (BSP_BC_BatMs_Init, BSP_BC_GetVoltageAndLevel, BSP_BC_GetState)
- **Implementation**: Uses existing ADC1 channel 3 (PC2) with voltage divider correction

---

## CSV Telemetry Format

```
STATE,<TickCount>,<LastAction>,<Voltage_mV>,<Level_Pct>,<Status_Id>

Example: STATE,22600,N,3250,0,7

Field       Type    Description
─────────────────────────────────────────────────
TickCount   uint32  HAL_GetTick() milliseconds
LastAction  char    'A', 'B', or 'N' (none received yet)
Voltage_mV  uint32  Battery voltage in millivolts
Level_Pct   uint32  Battery level percentage (0-100)
Status_Id   int     STBC02 charging state (see below)
```

### Status States (STBC02)

| ID  | State                      | Description                          |
|-----|----------------------------|--------------------------------------|
| 0   | NotValidInput              | No valid power source detected       |
| 1   | ValidInput                 | Valid power, battery not charging    |
| 2   | VbatLow                    | Battery voltage low                  |
| 3   | EndOfCharge                | Charging complete                    |
| 4   | ChargingPhase              | Actively charging                   |
| 5   | OverchargeFault            | Overcharge fault condition            |
| 6   | ChargingTimeout            | Charging timeout fault               |
| 7   | BatteryVoltageBelowVpre    | Battery below precondition voltage    |
| 8   | ChargingThermalLimitation  | Thermal limitation active            |
| 9   | BatteryTemperatureFault    | Temperature fault                    |

---

## Threshold Policy

```
┌────────────────────────────────────────────────────────────────┐
│                 BATTERY THRESHOLD DECISION                       │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│              ┌──────────────────────────────┐                │
│              │ Read Level_Pct and Status_Id │                │
│              └──────────────┬───────────────┘                │
│                             │                                  │
│                             ▼                                  │
│              ┌──────────────────────────────┐                │
│              │ Level < 20% AND Status NOT   │                │
│              │ (ChargingPhase OR EndOfCharge)│                │
│              └──────────────┬───────────────┘                │
│                    YES ◄─────┼──────► NO                     │
│                       │      │      │                         │
│                       ▼      │      ▼                         │
│                 ┌────────┐  │  ┌────────┐                  │
│                 │ ACTION │  │  │ ACTION │                  │
│                 │   'A'  │  │  │   'B'  │                  │
│                 │(Power  │  │  │(Normal │                  │
│                 │ Save)  │  │  │   Op)  │                  │
│                 └────────┘  │  └────────┘                  │
│                              │                                │
└──────────────────────────────┴────────────────────────────────┘
```

---

## Python Agent (agent.py)

```python
# Threshold policy
LOW_BATTERY_THRESHOLD_PCT = 20

def decide_action(level_pct: int, status_id: int) -> str:
    # Power Save if battery low and not charging
    if level_pct < LOW_BATTERY_THRESHOLD_PCT and status_id not in (3, 4):
        return 'A'  # Power Save
    else:
        return 'B'  # Normal Operation
```

---

## Known Issues

- **Battery reading 3250mV (minimum)**: The ADC reads the minimum voltage, indicating either:
  - No battery connected to the board
  - Battery is fully depleted
  - Hardware configuration issue with voltage divider

---

## File Structure

```
Battery_Monitor_Env/
├── Core/Src/main.c              # STBC02 API wrappers in USER CODE
├── Drivers/
│   └── BSP/STWIN.box/         # STBC02 driver files
│       ├── STWIN.box_bc.c      # Battery charger driver
│       ├── STWIN.box_bc.h      # Driver header
│       ├── STWIN.box_conf.h    # BSP configuration
│       └── STWIN.box_errno.h   # Error codes
├── RL_Brain/
│   └── agent.py               # STBC02-aware threshold policy agent
├── Dell_2_Steval.ioc           # CubeMX configuration
├── STM32U585AIIXQ_FLASH.ld     # Linker script
└── README.md                   # This file
```

---

## Version History

| Version | Date | Description |
|---------|------|-------------|
| v0.0.2 | 2026-04-04 | STBC02 driver integration with 6-element CSV |
| v0.0.1 | 2026-04-04 | Initial ADC battery monitoring integration |

---

## References

- **STEVAL-STWINBX1**: STMicroelectronics evaluation board
- **STM32U585AI**: ARM Cortex-M33 32-bit MCU
- **STBC02**: STMicroelectronics battery charger IC
- **ADC4_IN3**: PC2 pin - Battery sense via voltage divider
- **Voltage Divider**: R14=56k, R19=100k, multiplier=1.56
- **Voltage Range**: 3250mV (min) to 4225mV (max)