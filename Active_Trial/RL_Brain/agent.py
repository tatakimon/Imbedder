#!/usr/bin/env python3
"""
agent.py - Battery Monitor RL Brain

Threshold policy agent for battery voltage monitoring:
- Parses 4-element CSV: STATE,<TickCount>,<LastAction>,<Battery_mV>
- Threshold: Battery < 3500mV -> 'A' (Low Battery / Power Save)
- Threshold: Battery >= 3500mV -> 'B' (Normal Operation)

Usage:
    python3 agent.py
"""

import serial
import time
from datetime import datetime

# Configuration
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200
TEST_DURATION_SEC = 10

# Threshold policy
LOW_BATTERY_THRESHOLD_MV = 3500


def parse_state(line: str):
    """
    Parse STATE CSV line.

    Format: STATE,<TickCount>,<LastAction>,<Battery_mV>

    Returns:
        tuple: (tick, last_action, battery_mv) or None if invalid
    """
    parts = line.strip().split(',')
    if len(parts) == 4 and parts[0] == 'STATE':
        try:
            tick = int(parts[1])
            last_action = parts[2]
            battery_mv = int(parts[3])
            return tick, last_action, battery_mv
        except ValueError:
            return None
    return None


def decide_action(battery_mv: int) -> str:
    """
    Threshold policy for battery monitoring.

    Args:
        battery_mv: Battery voltage in millivolts

    Returns:
        str: 'A' for Low Battery (Power Save), 'B' for Normal
    """
    if battery_mv < LOW_BATTERY_THRESHOLD_MV:
        return 'A'  # Low Battery - Power Save action
    else:
        return 'B'  # Normal Operation


def main():
    """Main entry point."""
    print("=" * 60)
    print("   BATTERY MONITOR RL BRAIN - Threshold Policy Agent")
    print("=" * 60)
    print()
    print(f"Configuration:")
    print(f"  Serial Port:  {SERIAL_PORT}")
    print(f"  Baud Rate:    {BAUD_RATE}")
    print(f"  Test Duration: {TEST_DURATION_SEC} seconds")
    print(f"  Low Battery Threshold: {LOW_BATTERY_THRESHOLD_MV} mV")
    print()
    print(f"Policy:")
    print(f"  Battery < {LOW_BATTERY_THRESHOLD_MV} mV -> 'A' (Power Save)")
    print(f"  Battery >= {LOW_BATTERY_THRESHOLD_MV} mV -> 'B' (Normal)")
    print()
    print("-" * 60)
    print(f"[{'Time':<12}] {'Battery':>8} {'Action':<6} {'Sent':<4}")
    print("-" * 60)

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(0.5)  # Wait for serial to stabilize

        start_time = time.time()
        count_a = 0
        count_b = 0
        last_action = 'N'

        while (time.time() - start_time) < TEST_DURATION_SEC:
            # Read line from serial
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
            except UnicodeDecodeError:
                continue

            if not line:
                continue

            # Parse STATE line
            state = parse_state(line)
            if state is None:
                continue

            tick, last_action_received, battery_mv = state

            # Decide action based on threshold policy
            action = decide_action(battery_mv)

            # Send action if changed
            if action != last_action:
                ser.write(action.encode('utf-8'))
                last_action = action
                if action == 'A':
                    count_a += 1
                else:
                    count_b += 1

            # Print status
            elapsed = int(time.time() - start_time)
            timestamp = f"{elapsed:02d}s"
            print(f"[{timestamp:<12}] {battery_mv:>6} mV   {action:<6}    {action}")

        # Close serial
        ser.close()

        print("-" * 60)
        print()
        print("=" * 60)
        print(f"   TEST COMPLETE - {TEST_DURATION_SEC} seconds")
        print("=" * 60)
        print()
        print(f"Results:")
        print(f"  Action 'A' (Low Battery / Power Save): {count_a}")
        print(f"  Action 'B' (Normal Operation):        {count_b}")
        print()
        print(f"NOTE: Board battery voltage depends on power source.")
        print(f"      USB power may show different voltage than battery.")
        print()

    except serial.SerialException as e:
        print(f"[!] Serial error: {e}")
        print(f"[!] Could not connect to {SERIAL_PORT}")
        return


if __name__ == "__main__":
    main()