#!/usr/bin/env python3
"""
agent.py - Battery Monitor RL Brain (STBC02)

Threshold policy agent for battery monitoring using STBC02:
- Parses 6-element CSV: STATE,<TickCount>,<LastAction>,<Voltage_mV>,<Level_Pct>,<Status_Id>
- Policy: If <Level_Pct> < 20 AND NOT Charging -> 'A' (Power Save)
- Policy: Otherwise -> 'B' (Normal Operation)

STBC02 Status States:
  0 = NotValidInput (no valid power source)
  1 = ValidInput (USB/power connected, battery not charging)
  2 = VbatLow (battery voltage low)
  3 = EndOfCharge (charging complete)
  4 = ChargingPhase (actively charging)
  5 = OverchargeFault
  6 = ChargingTimeout
  7 = BatteryVoltageBelowVpre
  8 = ChargingThermalLimitation
  9 = BatteryTemperatureFault

Usage:
    python3 agent.py
"""

import serial
import time

# Configuration
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200
TEST_DURATION_SEC = 10

# Threshold policy
LOW_BATTERY_THRESHOLD_PCT = 20


def parse_state(line: str):
    """
    Parse STATE CSV line.

    Format: STATE,<TickCount>,<LastAction>,<Voltage_mV>,<Level_Pct>,<Status_Id>

    Returns:
        tuple: (tick, last_action, voltage_mv, level_pct, status_id) or None if invalid
    """
    parts = line.strip().split(',')
    if len(parts) == 6 and parts[0] == 'STATE':
        try:
            tick = int(parts[1])
            last_action = parts[2]
            voltage_mv = int(parts[3])
            level_pct = int(parts[4])
            status_id = int(parts[5])
            return tick, last_action, voltage_mv, level_pct, status_id
        except ValueError:
            return None
    return None


def is_charging(status_id: int) -> bool:
    """
    Check if the battery is in an active charging state.

    Args:
        status_id: STBC02 status ID

    Returns:
        True if actively charging (ChargingPhase=4 or EndOfCharge=3)
    """
    # ChargingPhase (4) or EndOfCharge (3)
    return status_id in (3, 4)


def decide_action(level_pct: int, status_id: int) -> str:
    """
    Threshold policy for battery monitoring.

    Policy: If Level_Pct < 20 AND NOT Charging -> 'A' (Power Save)
            Otherwise -> 'B' (Normal Operation)

    Args:
        level_pct: Battery level percentage (0-100)
        status_id: STBC02 status ID

    Returns:
        str: 'A' for Power Save, 'B' for Normal
    """
    if level_pct < LOW_BATTERY_THRESHOLD_PCT and not is_charging(status_id):
        return 'A'  # Low battery and not charging - Power Save
    else:
        return 'B'  # Normal operation


def main():
    """Main entry point."""
    print("=" * 60)
    print("   BATTERY MONITOR RL BRAIN - STBC02 Threshold Policy")
    print("=" * 60)
    print()
    print(f"Configuration:")
    print(f"  Serial Port:  {SERIAL_PORT}")
    print(f"  Baud Rate:    {BAUD_RATE}")
    print(f"  Test Duration: {TEST_DURATION_SEC} seconds")
    print(f"  Low Battery Threshold: {LOW_BATTERY_THRESHOLD_PCT}%")
    print()
    print(f"Policy:")
    print(f"  Level < {LOW_BATTERY_THRESHOLD_PCT}% AND NOT Charging -> 'A' (Power Save)")
    print(f"  Otherwise -> 'B' (Normal)")
    print()
    print("-" * 60)
    print(f"[{'Time':<8}] {'Voltage':>7} {'Level':>5} {'Status':>7} {'Action':<6} {'Sent':<4}")
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

            tick, last_action_received, voltage_mv, level_pct, status_id = state

            # Decide action based on threshold policy
            action = decide_action(level_pct, status_id)

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
            status_name = ["NotValid", "ValidInput", "VbatLow", "EndCharge",
                           "Charging", "OverChgFault", "Timeout", "BelowVpre",
                           "Thermal", "TempFault"][status_id] if status_id < 10 else "Unknown"
            print(f"[{timestamp:<8}] {voltage_mv:>6} mV  {level_pct:>4}%  {status_name:>9}  {action:<6}    {action}")

        # Close serial
        ser.close()

        print("-" * 60)
        print()
        print("=" * 60)
        print(f"   TEST COMPLETE - {TEST_DURATION_SEC} seconds")
        print("=" * 60)
        print()
        print(f"Results:")
        print(f"  Action 'A' (Power Save): {count_a}")
        print(f"  Action 'B' (Normal):     {count_b}")
        print()

    except serial.SerialException as e:
        print(f"[!] Serial error: {e}")
        print(f"[!] Could not connect to {SERIAL_PORT}")
        return


if __name__ == "__main__":
    main()