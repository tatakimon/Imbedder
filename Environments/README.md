# Environments

This directory contains completed and staged RL environment nodes as part of the Tree Architecture.

---

## Tree Structure

```
Generic_Base/                    # Hardware-agnostic starting point
    │
    └── Environments/             # Environment branches
            │
            ├── Accelerometer_Env/    # ISM330DHCX accelerometer RL env
            │       ├── Core/        # STM32 firmware
            │       ├── RL_Brain/    # Python agents
            │       ├── test_tilt.py # Test scripts
            │       └── README.md     # Environment documentation
            │
            └── Latency_Test_Env/     # HIL latency testing (staged)
                    ├── Core/        # Generic_Base firmware
                    ├── verify_latency.py  # sigrok-cli profiler
                    └── README.md     # Environment documentation
```

---

## Available Environments

### Accelerometer_Env (v1.0.1)
**Status:** ✓ Complete

Real accelerometer-based RL environment using ISM330DHCX 3-axis sensor:
- Tilt detection via AccZ threshold
- 6-element CSV state telemetry
- Python agent with threshold policy
- See [Accelerometer_Env/README.md](Accelerometer_Env/README.md)

### Latency_Test_Env (Staged)
**Status:** ⏳ Staged (awaiting hardware)

Hardware-in-the-loop (HIL) testing environment for measuring RL loop latency:
- **verify_latency.py**: sigrok-cli based latency profiler
- **Configuration**: 24MHz sample rate, 240k samples, Ch0 trigger
- **Measures**: UART frame end to GPIO response time
- **Hardware required**: Logic analyzer (e.g., sigrok-compatible device)
- See [Latency_Test_Env/README.md](Latency_Test_Env/README.md)

---

## Creating New Environments

When the user specifies a new environment is needed:

1. Clone from `Generic_Base/` or an existing `Environments/` node
2. Develop in `Active_Trial/`
3. On success, create new node: `Environments/<EnvName>/`

See [CLAUDE.md](../CLAUDE.md) for full development workflow.