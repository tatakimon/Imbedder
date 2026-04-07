# WiFi Environment (Wifi_Env)

Hello World over WiFi using ESP32 (MXCHIP) on STEVAL-STWINBX1.

---

## Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                      WIFI_ENV ARCHITECTURE                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌──────────────┐         WiFi         ┌──────────────┐       │
│   │  STEVAL-     │◄─────────────────────►│   PC /       │       │
│   │  STWINBX1    │    (ESP32/MXCHIP)     │   Server     │       │
│   │  (STM32U585) │                       └──────────────┘       │
│   └──────────────┘                                          │    │
│         │                                                     │    │
│   ┌─────▼─────┐                                               │    │
│   │  ESP32    │   "Hello World" UDP packet                   │    │
│   │  MXCHIP   │                                               │    │
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
│    ESP32/MXCHIP (SPI1)           USART2 (USB)                   │
│    ─────────────────────         ────────────────              │
│    SCK   ──► PG8                 TX   ──► PD5                   │
│    MISO  ──► PB4                 RX   ──► PD6                   │
│    MOSI  ──► PB5                 (115200 baud)                   │
│    NSS   ──► PH7                                            │
│    FLOW  ──► PG15                                           │
│    NOTIFY ──► PE7                                            │
│    RESET ──► PE12                                            │
│                                                                 │
│    LEDs                       Power                            │
│    ──────                     ──────                           │
│    GREEN ──► PH12              5V via USB                       │
│    ORANGE ──► PH10                                             │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

---

## WiFi Configuration

```
SSID: <configured at runtime>
Password: <configured at runtime>
Protocol: UDP
Port: 12345
Data: "Hello World"
```

---

## File Structure

```
Wifi_Env/
├── Core/
│   ├── Src/main.c              # Firmware (USER CODE regions)
│   └── Inc/main.h
├── Drivers/
├── RL_Brain/
│   └── agent.py                # Python UDP listener
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

- **ESP32/MXCHIP**: WiFi module on STEVAL-STWINBX1
- **STEVAL-STWINBX1**: STMicroelectronics evaluation board
- **STM32U585AI**: ARM Cortex-M33 32-bit MCU