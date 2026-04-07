# Bluetooth Environment (Bluetooth_Env)

Hello World over BLE using BlueNRG-2 on STEVAL-STWINBX1.

---

## Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                   BLUETOOTH_ENV ARCHITECTURE                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌──────────────┐         BLE          ┌──────────────┐       │
│   │  STEVAL-     │◄─────────────────────►│   PC /       │       │
│   │  STWINBX1    │    (BlueNRG-2)         │   Phone      │       │
│   │  (STM32U585) │                       └──────────────┘       │
│   └──────────────┘                                          │    │
│         │                                                     │    │
│   ┌─────▼─────┐                                               │    │
│   │ BlueNRG-2 │   "Hello World" string advertising            │    │
│   │    BLE    │                                               │    │
│   └───────────┘                                               │    │
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
│    BlueNRG-2 (SPI3)              USART2 (USB)                   │
│    ────────────────              ────────────────                │
│    SCK   ──► PG9                 TX   ──► PD5                   │
│    MISO  ──► PB4                 RX   ──► PD6                   │
│    MOSI  ──► PB5                 (115200 baud)                  │
│    CS    ──► PA15                                           │
│    RESET ──► PE8                                             │
│    IRQn  ──► PE7                                             │
│                                                                 │
│    LEDs                       Power                            │
│    ──────                     ──────                           │
│    GREEN ──► PH12              5V via USB                       │
│    ORANGE ──► PH10                                             │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

---

## BLE Service Structure

```
Service UUID: 0x0000xxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
├── TX Characteristic (Notify): "Hello World"
└── RX Characteristic (Write): Command input
```

---

## File Structure

```
Bluetooth_Env/
├── Core/
│   ├── Src/main.c              # Firmware (USER CODE regions)
│   └── Inc/main.h
├── Drivers/
├── RL_Brain/
│   └── agent.py                # Python BLE client
├── Dell_2_Steval.ioc           # CubeMX configuration
├── STM32U585AIIXQ_FLASH.ld      # Linker script
└── README.md                    # This file
```

---

## Version History

| Version | Date | Description |
|---------|------|-------------|
| v0.0.1 | 2026-04-07 | Initial structure |

---

## References

- **BlueNRG-2**: STMicroelectronics BLE 5.0 chip
- **STEVAL-STWINBX1**: STMicroelectronics evaluation board
- **STM32U585AI**: ARM Cortex-M33 32-bit MCU