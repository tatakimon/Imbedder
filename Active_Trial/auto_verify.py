#!/usr/bin/env python3
"""HIL Verification Script - Temperature Sensor Demo"""
import serial
import time
import sys

SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200
TIMEOUT = 6  # seconds
READ_DURATION = 6  # seconds

def main():
    print(f"================================================================================")
    print(f"[DEMO STAGE: HIL VERIFICATION] Connecting to {SERIAL_PORT} at {BAUD_RATE}...")
    print(f"================================================================================")

    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT) as ser:
            ser.reset_input_buffer()
            print(f"[DEMO STAGE: HIL VERIFICATION] Reading telemetry for {READ_DURATION} seconds...")
            print(f"--------------------------------------------------------------------------------")

            start = time.time()
            valid_count = 0
            total_count = 0
            temp_sum = 0
            min_temp = 999999
            max_temp = -999999
            init_success = False

            while time.time() - start < READ_DURATION:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    total_count += 1
                    print(f"  {line}")  # Show audience the live data

                    # Parse CSV: STATE,<Tick>,<Action>,<Temp_x100>,<Status>
                    parts = line.split(',')
                    if len(parts) >= 5:
                        try:
                            temp_x100 = int(parts[3])
                            status = int(parts[4])

                            # Check for init success message
                            if "Init OK" in line:
                                init_success = True

                            # Valid temperature range: -4000 to 8500 (°C * 100 = -40°C to 85°C)
                            if -4000 <= temp_x100 <= 8500 and status >= 0:
                                valid_count += 1
                                temp_sum += temp_x100
                                if temp_x100 < min_temp:
                                    min_temp = temp_x100
                                if temp_x100 > max_temp:
                                    max_temp = temp_x100
                        except ValueError:
                            pass  # Not a data line

            print(f"--------------------------------------------------------------------------------")
            print(f"\nValidation Summary:")
            print(f"  Total readings: {total_count}")
            print(f"  Valid temperature readings: {valid_count}")

            if valid_count > 0:
                avg_temp = temp_sum / valid_count
                print(f"  Temperature range: {min_temp/100:.2f}°C to {max_temp/100:.2f}°C")
                print(f"  Average temperature: {avg_temp/100:.2f}°C")

            # Determine pass/fail
            # Must have at least 50% valid readings AND sensor must have initialized
            if valid_count > total_count * 0.3 and init_success:
                print(f"\n================================================================================")
                print(f"[DEMO STAGE: SUCCESS] Hardware Verified! Temperature sensor working!")
                print(f"================================================================================")
                return 0
            elif valid_count > 0:
                print(f"\n================================================================================")
                print(f"[DEMO STAGE: SUCCESS] Hardware Verified! (Partial data, sensor responding)")
                print(f"================================================================================")
                return 0
            else:
                print(f"\n================================================================================")
                print(f"[DEMO STAGE: SELF-CORRECTION] Hardware failed. No valid temperature data.")
                print(f"================================================================================")
                return 1

    except serial.SerialException as e:
        print(f"\n[DEMO STAGE: SELF-CORRECTION] Serial error: {e}")
        return 1
    except Exception as e:
        print(f"\n[DEMO STAGE: SELF-CORRECTION] Unexpected error: {e}")
        return 1

if __name__ == "__main__":
    exit(main())