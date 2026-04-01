#!/usr/bin/env python3
"""
RL Brain - Python Agent for STEVAL-STWINBX1 RL Environment
Connects to the board via UART and implements a threshold policy based on AccZ.
"""

import serial
import time
from datetime import datetime

# Configuration
SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200
ACCZ_THRESHOLD = 5000  # AccZ < 5000 means board tilted/picked up, AccZ >= 5000 means flat

def parse_state(line: str):
    """Parse STATE CSV line: STATE,<TickCount>,<LastAction>,<AccX>,<AccY>,<AccZ>"""
    parts = line.strip().split(',')
    if len(parts) == 6 and parts[0] == 'STATE':
        try:
            tick = int(parts[1])
            last_action = parts[2]
            acc_x = int(parts[3])
            acc_y = int(parts[4])
            acc_z = int(parts[5])
            return tick, last_action, acc_x, acc_y, acc_z
        except ValueError:
            return None, None, None, None, None
    return None, None, None, None, None

def decide_action(acc_z: int) -> str:
    """Threshold policy: if AccZ < 5000 (tilted/picked up) send 'A', else (flat) send 'B'"""
    if acc_z < ACCZ_THRESHOLD:
        return 'A'
    else:
        return 'B'

def main():
    print("=== RL Brain Agent - AccZ Threshold Policy ===")
    print(f"Connecting to {SERIAL_PORT} at {BAUD_RATE} baud...")
    print(f"Threshold policy: AccZ < {ACCZ_THRESHOLD} -> 'A' (tilted/picked up)")
    print(f"                  AccZ >= {ACCZ_THRESHOLD} -> 'B' (flat on desk)")
    print("Press Ctrl+C to stop\n")

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(0.5)  # Wait for connection to stabilize

        # Clear any stale data
        ser.read(1000)

        print("[Time]        AccZ: <val> | Action Sent: <A/B>")
        print("-" * 50)

        # Count actions for summary
        count_a = 0
        count_b = 0

        while True:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='replace')
                tick, last_action, acc_x, acc_y, acc_z = parse_state(line)

                if acc_z is not None:
                    # Decide action based on AccZ threshold policy
                    action = decide_action(acc_z)

                    if action == 'A':
                        count_a += 1
                    else:
                        count_b += 1

                    # Log the decision
                    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                    print(f"[{timestamp}] AccZ: {acc_z:5d} | Action Sent: {action}")

                    # Send action to board
                    ser.write(action.encode('utf-8'))

            time.sleep(0.01)  # Small delay to prevent busy loop

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\n\nAgent stopped by user.")
        print(f"\n=== Summary ===")
        print(f"Actions sent: A={count_a}, B={count_b}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial connection closed.")

if __name__ == "__main__":
    main()
