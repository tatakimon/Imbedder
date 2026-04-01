# Latency Test Environment (Latency_Test_Env)

Hardware-in-the-loop (HIL) testing environment for measuring RL loop latency.

---

## Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                   LATENCY_TEST_ENV ARCHITECTURE                  │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌──────────────┐         ┌──────────────┐         ┌─────────┐ │
│   │  STEVAL-     │   UART  │  sigrok-cli  │  GPIO   │ Logic   │ │
│   │  STWINBX1    │◄────────│  (24MHz,     │◄────────│Analyzer │ │
│   │  (STM32U585) │  USB    │   240k samp) │  Probe  │(HIL)    │ │
│   └──────────────┘         └──────────────┘         └─────────┘ │
│         │                        │                        │      │
│         │                        │                        │      │
│         │ 'A' action              │ CSV with timing        │      │
│         │                        │ data                   │      │
│         ▼                        ▼                        ▼      │
│   LED toggle              Parse GPIO                Measure      │
│   response                transition                latency      │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## What We Measure

```
┌────────────────────────────────────────────────────────────────┐
│                     LATENCY MEASUREMENT                         │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│   UART TX ──────────────────────────────────────────────────   │
│   (Action 'A')                                                 │
│   ───┐                                                         │
│       │ 87µs (10 bits @ 115200 baud)                          │
│   ┌───┘                                                        │
│   │                           ┌────────────────────────────    │
│   │  GPIO Response Latency    │ (measured from UART frame     │
│   │  (this is what we want)  │  end to GPIO high)            │
│   ▼                           └────────────────────────────    │
│   GPIO High ────────────────────────────────────────────────   │
│                                                                 │
│   Total Loop Latency ≈ 87µs + GPIO Response + sigrok capture  │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

---

## Configuration

| Parameter | Value | Description |
|-----------|-------|-------------|
| Sample Rate | 24 MHz | sigrok-cli sample rate |
| Samples | 240,000 | Total capture depth |
| Trigger | Ch 0, Falling Edge | GPIO trigger point |
| UART | 115200 baud, 8N1 | Serial communication |
| Frame Duration | ~87 µs | 10 bits @ 115200 baud |

---

## verify_latency.py

Hardware latency profiler script using sigrok-cli.

### Features

- **Automated sigrok-cli control** - Starts capture, sends UART action, parses results
- **CSV parsing** - Extracts timing from sigrok-cli output
- **Graceful failure** - Exits cleanly if hardware not detected
- **HIL ready** - Designed for hardware-in-the-loop validation

### Usage

```bash
# Connect logic analyzer to GPIO pin (channel 0)
# Connect USB to STEVAL-STWINBX1
# Run:
python3 verify_latency.py
```

### Expected Output (when hardware available)

```
============================================================
   HARDWARE LATENCY PROFILER - sigrok-cli HIL Test
============================================================

Configuration:
  Sample Rate:  24MHz
  Samples:      240,000
  Trigger:      Channel 0 (f edge)
  UART:         /dev/ttyACM0 @ 115200 baud

Starting sigrok-cli capture...
Sending action 'A' over UART...
Waiting for capture to complete...
Parsing capture data...

============================================================
   LATENCY MEASUREMENT: XX.XX µs
============================================================
```

### Graceful Failure (current state)

```
[!] sigrok-cli not found in PATH
[!] Hardware analyzer not detected. Test staged and ready.
```

---

## File Structure

```
Latency_Test_Env/
├── Core/Src/main.c              # Generic_Base firmware (unchanged)
├── Drivers/                     # STM32U5 HAL drivers
├── RL_Brain/                    # (placeholder for future agents)
├── verify_latency.py            # HIL latency profiler script
├── Dell_2_Steval.ioc            # CubeMX configuration
├── STM32U585AIIXQ_FLASH.ld      # Linker script
└── README.md                    # This file
```

---

## Status

**Staged** - Awaiting physical logic analyzer hardware.

Once hardware arrives:
1. Connect logic analyzer channel 0 to GPIO pin (e.g., LED GREEN PH12)
2. Connect USB cable to STEVAL-STWINBX1
3. Run `python3 verify_latency.py`
4. Measure actual loop latency

---

## Next Phase

After Latency_Test_Env validation:
- Integrate next sensor (Gyroscope or Magnetometer) into new environment branch
- Use latency measurements to validate RL loop timing requirements