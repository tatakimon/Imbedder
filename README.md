# STEVAL-STWINBX1 Reinforcement Learning Environment

A practical implementation of Reinforcement Learning on embedded hardware. This project transforms the STEVAL-STWINBX1 evaluation board into an RL-enabled device capable of learning from sensor feedback.

---

## The Goal

```
┌─────────────────────────────────────────────────────────────────┐
│                      WHAT WE'RE BUILDING                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│    ┌─────────────┐         ┌─────────────┐         ┌─────────┐│
│    │   Sensor    │────────▶│    RL       │◀────────│  Act    ││
│    │   Data      │         │   Brain     │         │(board)  ││
│    └─────────────┘         └─────────────┘         └─────────┘│
│         │                        │                        │      │
│         │                        │                        │      │
│         ▼                        ▼                        ▼      │
│    AccZ, AccY,            Threshold            LED toggle,     │
│    AccX, Temp,            or Q-learning        motor ctrl,     │
│    Gyro, etc.             policy                etc.           │
│                                                                  │
│              LEARN → ACT → SENSE → LEARN → ACT ...             │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

Build generic RL environments that can activate any sensor on demand, enabling the board to learn optimal policies through trial and error.

---

## Architecture: The Tree

```
┌─────────────────────────────────────────────────────────────────┐
│                        PROJECT TREE                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   Generic_Base/                                                  │
│   ├── Core/Src/main.c         # Hardware-agnostic template      │
│   └── Dell_2_Steval.ioc       # CubeMX base configuration       │
│                                                                  │
│   Environments/                     # Completed branches          │
│   │                                                         │
│   └── Accelerometer_Env/          # Node 1: ISM330DHCX tilt     │
│       ├── Core/Src/main.c         #   Phase 5 working code       │
│       ├── RL_Brain/agent.py       #   Python threshold agent     │
│       ├── test_tilt.py            #   Tilt detection test        │
│       └── README.md               #   Environment docs            │
│                                                                  │
│   Active_Trial/                      # Current development        │
│                                                                  │
│   CLAUDE.md                          # Development rules          │
│   LESSONS_LEARNED.md               # Permanent lessons           │
│   SUMMARY.md                        # Changelog                  │
│   TRIAL_LOG.md                      # Detailed log               │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## How It Works

### 1. Board Sends State (every 100ms)

```
STATE,<Tick>,<LastAction>,<AccX>,<AccY>,<AccZ>

Example:
STATE,22600,N,-224,69,8401
```

### 2. Python Agent Receives & Decides

```python
def decide_action(acc_z):
    if acc_z < 5000:        # Tilted/picked up
        return 'A'
    else:                   # Flat on desk
        return 'B'
```

### 3. Board Receives Action & Acts

- `'A'` → Toggle Green LED
- `'B'` → No LED change

### 4. Loop Repeats ~10x per second

The agent learns which actions work best for which states.

---

## Closed Loop System

```
┌──────────────────────────────────────────────────────────────────┐
│                     CLOSED LOOP DATA FLOW                        │
├──────────────────────────────────────────────────────────────────┤
│                                                                   │
│    ┌───────────┐    115200    ┌───────────┐    Action    ┌──────┴──┐
│    │  STM32    │◄────────────│  Python   │─────────────▶│ STEVAL  │
│    │  UART2    │    USB       │  Agent    │    'A'/'B'   │ STWINBX1│
│    └─────┬─────┘             └─────────────┘              └─────────┘│
│          │                                                         │
│          │ STATE CSV                                              │
│          │ "STATE,123,A,0,0,8400\r\n"                             │
│          │                                                         │
│          └─────────────────────────────────────────────────────────┘
│                                                                   │
└───────────────────────────────────────────────────────────────────┘
```

---

## Quick Start

### Hardware Requirements

- STEVAL-STWINBX1 evaluation board
- USB-C cable for power + UART
- Linux PC with `/dev/ttyACM0` available

### Flash Firmware

```bash
# 1. Setup Active_Trial from Accelerometer_Env
rm -rf Active_Trial
cp -r Environments/Accelerometer_Env Active_Trial

# 2. Compile
make -C Active_Trial/Debug all

# 3. Flash using STM32CubeProgrammer or similar
```

### Run Python Agent

```bash
# Connect board, then:
python3 Environments/Accelerometer_Env/test_tilt.py
```

Watch for the `>>> TILT NOW! <<<` prompt and physically tilt the board.

---

## Development Phases

| Phase | Description | Status |
|-------|-------------|--------|
| 0 | Base LED + UART | ✓ Complete |
| 1 | UART RX command handling | ✓ Complete |
| 2 | Interrupt-based UART | ✓ Complete |
| 3 | RL State Vector (dummy) | ✓ Complete |
| 4 | Generic sensor abstraction | ✓ Complete |
| 5 | Real ISM330DHCX accelerometer | ✓ Complete |
| 6 | Tilt detection + closed loop | ✓ Complete |

---

## Key Files

| File | Purpose |
|------|---------|
| [CLAUDE.md](CLAUDE.md) | Development rules and workflow |
| [LESSONS_LEARNED.md](LESSONS_LEARNED.md) | Permanent rules from failures |
| [SUMMARY.md](SUMMARY.md) | Version history and changelog |
| [TRIAL_LOG.md](TRIAL_LOG.md) | Detailed development log |
| [Environments/README.md](Environments/README.md) | Environment tree overview |

---

## Next Steps

- Add more sensors (gyroscope, magnetometer, microphone)
- Implement Q-learning instead of threshold policy
- Add reward signal calculation
- Create new environment nodes (Motor_Env, Audio_Env, etc.)

---

**Board:** STEVAL-STWINBX1 (STM32U585AI)
**Sensor:** ISM330DHCX 3-axis accelerometer
**UART:** 115200 baud, 8N1, USB-C
**Telemetry:** CSV at 100ms intervals
**Policy:** Threshold-based (expandable to full RL)