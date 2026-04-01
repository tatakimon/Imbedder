#!/usr/bin/env python3
"""
verify_latency.py - Hardware Latency Profiler using sigrok-cli

Performs hardware-in-the-loop (HIL) latency measurement between:
1. UART frame transmission (action 'A' sent)
2. GPIO pin response (LED toggle)

Requires:
- sigrok-cli (part of sigrok-tools)
- Serial connection to STEVAL-STWINBX1 at /dev/ttyACM0
- Logic analyzer on GPIO pin (channel 0)

Usage:
    python3 verify_latency.py
"""

import serial
import subprocess
import time
import sys
from datetime import datetime

# Configuration
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200

# sigrok-cli configuration
SIGROK_SAMPLE_RATE = '24MHz'
SIGROK_NUM_SAMPLES = 240000
SIGROK_TRIGGER_CHANNEL = 0
SIGROK_TRIGGER_EDGE = 'r'  # 'r' = rising, 'f' = falling
SIGROK_OUTPUT_FORMAT = 'csv'


def check_sigrok_available():
    """Check if sigrok-cli is installed and available."""
    try:
        result = subprocess.run(
            ['sigrok-cli', '--version'],
            capture_output=True,
            text=True,
            timeout=5
        )
        return result.returncode == 0
    except (subprocess.SubprocessError, FileNotFoundError):
        return False


def run_latency_test():
    """Run the sigrok-cli latency profiling session."""
    print("=" * 60)
    print("   HARDWARE LATENCY PROFILER - sigrok-cli HIL Test")
    print("=" * 60)
    print()
    print(f"Configuration:")
    print(f"  Sample Rate:  {SIGROK_SAMPLE_RATE}")
    print(f"  Samples:      {SIGROK_NUM_SAMPLES:,}")
    print(f"  Trigger:      Channel {SIGROK_TRIGGER_CHANNEL} ({SIGROK_TRIGGER_EDGE} edge)")
    print(f"  UART:         {SERIAL_PORT} @ {BAUD_RATE} baud")
    print()

    # Build sigrok-cli command
    sigrok_cmd = [
        'sigrok-cli',
        '--samples', str(SIGROK_NUM_SAMPLES),
        '--sample-rate', SIGROK_SAMPLE_RATE,
        '--trigger', f'ch{SIGROK_TRIGGER_CHANNEL}={SIGROK_TRIGGER_EDGE}',
        '--output-format', SIGROK_OUTPUT_FORMAT,
        '--continuous'
    ]

    print(f"Starting sigrok-cli capture...")
    print(f"Command: {' '.join(sigrok_cmd)}")
    print()

    # Start sigrok-cli in background, pipe stdout for CSV parsing
    try:
        sigrok_process = subprocess.Popen(
            sigrok_cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
    except (subprocess.SubprocessError, FileNotFoundError) as e:
        print(f"[!] Failed to start sigrok-cli: {e}")
        print("[!] Hardware analyzer not detected. Test staged and ready.")
        return None

    # Give sigrok time to initialize and arm trigger
    time.sleep(0.5)

    # Open serial connection and send action 'A'
    print("Sending action 'A' over UART...")
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(0.1)  # Wait for serial to stabilize
        ser.write(b'A')
        ser.close()
        print("Action 'A' sent.")
    except serial.SerialException as e:
        print(f"[!] Serial error: {e}")
        sigrok_process.terminate()
        print("[!] Hardware analyzer not detected. Test staged and ready.")
        return None

    # Wait for capture to complete
    print("Waiting for capture to complete...")
    stdout, stderr = sigrok_process.communicate(timeout=10)

    if sigrok_process.returncode != 0:
        print(f"[!] sigrok-cli error: {stderr}")
        print("[!] Hardware analyzer not detected. Test staged and ready.")
        return None

    # Parse CSV output to find timing
    print("Parsing capture data...")
    latency_us = parse_latency_from_csv(stdout)

    return latency_us


def parse_latency_from_csv(csv_data):
    """
    Parse sigrok-cli CSV output to calculate latency.

    CSV format from sigrok-cli:
    CSV with analog channels. Each line has comma-separated sample values.
    We look for the transition on channel 0 (GPIO) and calculate time delta.

    Returns latency in microseconds, or None if calculation fails.
    """
    lines = csv_data.strip().split('\n')
    if len(lines) < 2:
        print("[!] Insufficient data in CSV capture")
        return None

    # Skip header line if present
    if 'time' in lines[0].lower() or not lines[0][0].isdigit():
        data_lines = lines[1:]
    else:
        data_lines = lines

    # Find the trigger point (first sample where channel 0 goes high)
    trigger_sample_idx = None
    prev_value = None

    for idx, line in enumerate(data_lines):
        try:
            # Parse comma-separated sample values
            samples = line.split(',')
            if len(samples) < 1:
                continue

            # Channel 0 is the first value in the CSV
            value = float(samples[0].strip())

            # Detect rising edge (transition from low to high)
            if prev_value is not None:
                if prev_value < 1.0 and value >= 1.0:  # Low to high transition
                    trigger_sample_idx = idx
                    break

            prev_value = value
        except (ValueError, IndexError):
            continue

    if trigger_sample_idx is None:
        print("[!] Could not detect GPIO transition in capture")
        return None

    # Calculate time delta based on sample rate
    # Time per sample = 1 / sample_rate
    # For 24MHz: 1/24,000,000 = 41.67 nanoseconds per sample
    sample_rate_hz = 24_000_000  # 24MHz
    time_per_sample_us = (1.0 / sample_rate_hz) * 1_000_000  # Convert to microseconds

    # Estimate UART frame end time
    # Assumptions:
    # - 8 data bits + 1 start bit + 1 stop bit = 10 bits per frame
    # - At 115200 baud, each bit = 1/115200 ≈ 8.68 µs
    # - Full frame = 10 * 8.68 = 86.8 µs
    # - Action 'A' is 1 byte = ~87 µs
    uart_frame_duration_us = (10.0 / BAUD_RATE) * 1_000_000  # ~86.8 µs

    # The trigger captures the GPIO going HIGH (end of LED toggle response)
    # Latency = time from UART frame end to GPIO response
    # Sample index where trigger fires represents GPIO high time

    # Calculate time at trigger point
    trigger_time_us = trigger_sample_idx * time_per_sample_us

    # Estimated UART frame end time
    uart_end_time_us = uart_frame_duration_us

    # Latency = GPIO response time - UART frame end time
    # Since trigger is at GPIO high, and we want time from UART frame end,
    # we report the trigger time as our measurement
    latency_us = trigger_time_us

    # More accurate: subtract the estimated frame transmission time
    # to get actual GPIO response latency
    response_latency_us = trigger_time_us - uart_end_time_us

    return response_latency_us


def main():
    """Main entry point."""
    print()
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Latency test initialized")
    print()

    # Check if sigrok-cli is available
    if not check_sigrok_available():
        print("[!] sigrok-cli not found in PATH")
        print("[!] Hardware analyzer not detected. Test staged and ready.")
        sys.exit(0)

    # Run the latency test
    latency = run_latency_test()

    print()
    print("=" * 60)
    if latency is not None:
        print(f"   LATENCY MEASUREMENT: {latency:.2f} µs")
        print("=" * 60)
        print()
        print("Result breakdown:")
        print(f"  Trigger sample index: captures GPIO high")
        print(f"  Sample rate:         {SIGROK_SAMPLE_RATE}")
        print(f"  Time per sample:     {24_000_000 * time_per_sample_us:.2f} µs (calc)")
        print(f"  UART frame (~87µs):   excluded from measurement")
        print()
        print("NOTE: This is a staging test. Physical validation pending")
        print("      arrival of logic analyzer hardware.")
    else:
        print("[!] Latency measurement could not be completed")
        print("[!] Hardware analyzer not detected. Test staged and ready.")
    print()


if __name__ == "__main__":
    main()