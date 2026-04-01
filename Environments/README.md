# Environments

This directory contains completed RL environment nodes as part of the Tree Architecture.

---

## Tree Structure

```
Generic_Base/                    # Hardware-agnostic starting point
    │
    └── Environments/             # Completed environment branches
            │
            └── Accelerometer_Env/    # ISM330DHCX accelerometer RL env
                    ├── Core/        # STM32 firmware
                    ├── RL_Brain/    # Python agents
                    ├── test_tilt.py # Test scripts
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

---

## Creating New Environments

When the user specifies a new environment is needed:

1. Clone from `Generic_Base/` or an existing `Environments/` node
2. Develop in `Active_Trial/`
3. On success, create new node: `Environments/<EnvName>/`

See [CLAUDE.md](../CLAUDE.md) for full development workflow.