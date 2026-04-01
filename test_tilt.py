#!/usr/bin/env python3
import serial
import time
from datetime import datetime

SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200
ACCZ_THRESHOLD = 5000

def parse_state(line):
    parts = line.strip().split(',')
    if len(parts) == 6 and parts[0] == 'STATE':
        try:
            return int(parts[1]), parts[2], int(parts[3]), int(parts[4]), int(parts[5])
        except:
            pass
    return None, None, None, None, None

def decide_action(acc_z):
    return 'A' if acc_z < ACCZ_THRESHOLD else 'B'

print('========================================')
print('   RL BRAIN - ACCZ TILT DETECTION')
print('========================================')
print()
print('INSTRUCTIONS:')
print('  Watch for the >>> TILT NOW! <<< message')
print('  Then pick up the board and tilt it!')
print()
print(f'Policy: AccZ < {ACCZ_THRESHOLD} = A (tilted)')
print(f'        AccZ >= {ACCZ_THRESHOLD} = B (flat)')
print()
print('[Time]       AccZ   Action')
print('-' * 35)

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(0.5)
ser.read(1000)

count_a = 0
count_b = 0
tilt_printed = False
countdown_printed = False
start = time.time()

while time.time() - start < 20:
    if ser.in_waiting:
        line = ser.readline().decode('utf-8', errors='replace')
        tick, last_action, acc_x, acc_y, acc_z = parse_state(line)

        if acc_z is not None:
            action = decide_action(acc_z)
            if action == 'A':
                count_a += 1
            else:
                count_b += 1

            ts = datetime.now().strftime('%H:%M:%S.%f')[:-3]
            print(f'[{ts}] {acc_z:6d}   {action}')
            ser.write(action.encode('utf-8'))

    elapsed = int(time.time() - start)

    # Countdown at 4 seconds (print 3, 2, 1 before tilt message)
    if elapsed == 4 and not countdown_printed:
        print()
        print('>>> GET READY! TILT IN 3... 2... 1... <<<')
        print()
        countdown_printed = True

    # Print TILT message at second 5
    if elapsed == 5 and not tilt_printed:
        print()
        print('>>> TILT NOW! PICK UP THE BOARD! <<<')
        print()
        tilt_printed = True

    time.sleep(0.01)

ser.close()
print()
print('========================================')
print(f'RESULT: A={count_a} (tilted), B={count_b} (flat)')
print('========================================')