#!/usr/bin/env python3
"""
RL Brain - Python Agent for STEVAL-STWINBX1 RL Environment
Connects to the board via UART and implements a threshold policy.
"""

import serial
import time
from datetime import datetime

# Configuration
SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200
THRESHOLD = 50

def parse_state(line: str):
    """Parse STATE CSV line: STATE,<TickCount>,<LastActionReceived>,<DummySensorValue>"""
    parts = line.strip().split(',')
    if len(parts) == 4 and parts[0] == 'STATE':
        try:
            tick = int(parts[1])
            last_action = parts[2]
            sensor_value = int(parts[3])
            return tick, last_action, sensor_value
        except ValueError:
            return None, None, None
    return None, None, None

def decide_action(sensor_value: int) -> str:
    """Threshold policy: if sensor > 50 send 'A', else send 'B'"""
    if sensor_value > THRESHOLD:
        return 'A'
    else:
        return 'B'

def main():
    print("=== RL Brain Agent ===")
    print(f"Connecting to {SERIAL_PORT} at {BAUD_RATE} baud...")
    print(f"Threshold policy: sensor > {THRESHOLD} -> 'A', else -> 'B'")
    print("Press Ctrl+C to stop\n")

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(0.5)  # Wait for connection to stabilize

        # Clear any stale data
        ser.read(1000)

        print("[Time]            State: <val> | Action Sent: <A/B>")
        print("-" * 55)

        while True:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='replace')
                tick, last_action, sensor_value = parse_state(line)

                if sensor_value is not None:
                    # Decide action based on threshold policy
                    action = decide_action(sensor_value)

                    # Log the decision
                    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                    print(f"[{timestamp}] State: {sensor_value:3d} | Action Sent: {action}")

                    # Send action to board
                    ser.write(action.encode('utf-8'))

            time.sleep(0.01)  # Small delay to prevent busy loop

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\n\nAgent stopped by user.")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial connection closed.")

if __name__ == "__main__":
    main()
