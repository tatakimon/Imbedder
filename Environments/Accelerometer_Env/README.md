# Accelerometer Environment (Accelerometer_Env)

First completed RL environment node using the ISM330DHCX 3-axis accelerometer.

---

## Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    ACCELEROMETER_ENV ARCHITECTURE                │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌──────────────┐         ┌──────────────┐         ┌─────────┐ │
│   │  STEVAL-     │   UART  │   Python     │  UART   │ Python  │ │
│   │  STWINBX1    │◄───────►│   Agent      │◄───────►│ RL      │ │
│   │  (STM32U585) │  USB    │   (agent.py)  │  USB    │ Brain   │ │
│   └──────────────┘         └──────────────┘         └─────────┘ │
│         │                        │                        │      │
│         │                        │                        │      │
│   ┌─────▼─────┐           ┌──────▼──────┐          ┌──────▼────┐ │
│   │ ISM330DHCX │           │  Threshold  │          │  Action   │ │
│   │ Accel API │           │  Policy     │          │  Feedback │ │
│   └───────────┘           │  AccZ<5000  │          │  Loop     │ │
│                           │    = 'A'    │          │          │ │
│                           │  AccZ>=5000 │          └──────────┘ │
│                           │    = 'B'    │                           │
│                           └─────────────┘                           │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Hardware Setup

```
┌────────────────────────────────────────────────────────────────┐
│                    STEVAL-STWINBX1 PINOUT                       │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│    ISM330DHCX (SPI2)              USART2 (USB)                 │
│    ────────────────               ────────────────             │
│    SCK   ──► PB13                 TX   ──► PD5                │
│    MOSI  ──► PB15                 RX   ──► PD6                │
│    MISO  ──► PB14                 (115200 baud)               │
│    CS    ──► PE11                                             │
│    INT1  ──► PB10                                             │
│    INT2  ──► PC05                                             │
│                                                                 │
│    LEDs                         Power                          │
│    ──────                       ──────                         │
│    GREEN ──► PH12               5V via USB                     │
│    ORANGE ──► PH10                                              │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

---

## CSV Telemetry Format

```
STATE,<TickCount>,<LastAction>,<AccX>,<AccY>,<AccZ>

Example: STATE,22600,N,-224,69,8401

Field       Type    Description
───────────────────────────────
TickCount   uint32  HAL_GetTick() milliseconds
LastAction  char    'A', 'B', or 'N' (none received yet)
AccX        int16   X-axis acceleration (Lsb ≈ 0.061 mg/LSB)
AccY        int16   Y-axis acceleration
AccZ        int16   Z-axis acceleration
```

---

## Threshold Policy

```
┌────────────────────────────────────────────────────────────────┐
│                     ACCZ THRESHOLD DECISION                    │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│                    ┌──────────────────┐                       │
│                    │   Read AccZ       │                       │
│                    └────────┬─────────┘                       │
│                             │                                  │
│                             ▼                                  │
│                    ┌──────────────────┐                       │
│                    │  AccZ < 5000 ?    │                       │
│                    └────────┬─────────┘                       │
│                      YES ◄─┼──► NO                            │
│                       │    │     │                            │
│                       ▼    │     ▼                            │
│                 ┌────────┐ │ ┌────────┐                       │
│                 │ ACTION │ │ │ ACTION │                       │
│                 │   'A'  │ │ │   'B'  │                       │
│                 │(tilted)│ │ │ (flat) │                       │
│                 └────────┘ │ └────────┘                       │
│                              │                                  │
└──────────────────────────────┴──────────────────────────────────┘
```

---

## Typical Accelerometer Values

```
Board Position    AccX       AccY       AccZ
──────────────────────────────────────────────
Flat on desk      ~0         ~0         ~8400  (≈1g gravity)
Tilted 45°        varies     varies     ~5900
Picked up         varies     varies     <5000   → triggers 'A'
Upside down       ~0         ~0         ~-8400
```

---

## File Structure

```
Accelerometer_Env/
├── Core/
│   └── Src/main.c              # Phase 5/6 firmware (USER CODE regions)
├── Drivers/
│   └── ISM330DHCX/             # Sensor driver
├── RL_Brain/
│   └── agent.py                # Python threshold policy agent
├── test_tilt.py                # Interactive tilt detection test
├── Dell_2_Steval.ioc           # CubeMX configuration
├── STM32U585AIIXQ_FLASH.ld      # Linker script
└── README.md                    # This file
```

---

## Running the Environment

### 1. Flash Firmware

```bash
cd Active_Trial/Debug
make -C Active_Trial/Debug all
# Flash using your preferred tool (STM32CubeProgrammer, etc.)
```

### 2. Run Python Agent

```bash
# Option A: Using the RL Brain agent
python3 RL_Brain/agent.py

# Option B: Using the tilt test script (with countdown)
python3 test_tilt.py
```

### 3. Expected Output (agent.py)

```
=== RL Brain Agent - AccZ Threshold Policy ===
Connecting to /dev/ttyACM0 at 115200 baud...
Threshold policy: AccZ < 5000 -> 'A' (tilted/picked up)
                  AccZ >= 5000 -> 'B' (flat on desk)
Press Ctrl+C to stop

[Time]        AccZ: <val> | Action Sent: <A/B>
--------------------------------------------------
[14:32:01.123] AccZ:  8401 | Action Sent: B
[14:32:01.223] AccZ:  8398 | Action Sent: B
[14:32:01.323] AccZ:  8402 | Action Sent: B
...
```

### 4. Expected Output (test_tilt.py)

```
========================================
   RL BRAIN - ACCZ TILT DETECTION
========================================

INSTRUCTIONS:
  Watch for the >>> TILT NOW! <<< message
  Then pick up the board and tilt it!

Policy: AccZ < 5000 = A (tilted)
        AccZ >= 5000 = B (flat)

[Time]       AccZ   Action
-----------------------------------

>>> GET READY! TILT IN 3... 2... 1... <<<

>>> TILT NOW! PICK UP THE BOARD! <<<

[14:32:06.101]  3456   A
[14:32:06.201]  2341   A
[14:32:06.301]  1234   A

========================================
RESULT: A=45 (tilted), B=155 (flat)
========================================
```

---

## Version History

| Version | Date | Description |
|---------|------|-------------|
| v1.0.1 | 2026-04-01 | Restructured into Tree Architecture |
| v1.0.3 | 2026-03-30 | Phase 5: Real ISM330DHCX integration |
| v1.0.0 | 2026-03-30 | Phase 4: RL State Vector with dummy sensor |

---

## References

- **ISM330DHCX Datasheet**: STMicroelectronics
- **STEVAL-STWINBX1**: STMicroelectronics evaluation board
- **STM32U585AI**: ARM Cortex-M33 32-bit MCU