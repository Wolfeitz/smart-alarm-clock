#!/usr/bin/env python3
"""Bounded hardware navigation soak. Never saves settings or triggers sound."""
import argparse
from datetime import datetime
import os
from pathlib import Path
import re
import time
import serial

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--port', required=True)
parser.add_argument('--cycles', type=int, default=20)
args = parser.parse_args()
if not 1 <= args.cycles <= 100:
    parser.error('cycles must be between 1 and 100')
root = Path(__file__).resolve().parents[1]
output = root / 'local-config/clock' / ('navigation-soak-' + datetime.now().strftime('%Y%m%d-%H%M%S') + '.log')
output.parent.mkdir(parents=True, exist_ok=True)
log = os.fdopen(os.open(output, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600), 'wb')
connection = serial.Serial()
connection.port = args.port
connection.baudrate = 115200
connection.timeout = .05
connection.dtr = connection.rts = False
heaps = []
remainder = ''

def collect(duration):
    global remainder
    end = time.monotonic() + duration
    lines = []
    while time.monotonic() < end:
        data = connection.read(4096)
        if not data:
            continue
        log.write(data)
        text = remainder + data.decode(errors='replace')
        parts = text.split('\n')
        remainder = parts.pop()
        for line in parts:
            line = line.strip()
            if 'panic' in line.lower() or 'assert failed' in line.lower():
                raise RuntimeError('Device crash reported; inspect private receipt')
            match = re.search(r'CLOCK_ALIVE valid=1 epoch=(\d+) rtc=(\d+) rtc_status=ESP_OK heap=(\d+)', line)
            if match:
                epoch, rtc, heap = map(int, match.groups())
                if abs(epoch - rtc) > 2:
                    raise RuntimeError('System/RTC diverged')
                heaps.append(heap)
            lines.append(line)
    log.flush()
    return '\n'.join(lines)

def command(text, duration=.5):
    connection.write((text + '\n').encode())
    return collect(duration)

def screen(expected):
    text = command('UI', 1.5)
    if f'UI_SCREEN name={expected} overlay=0' not in text:
        raise RuntimeError(f'Expected {expected} without alarm overlay; stopped without altering settings')

def tap(x, y, expected):
    text = command(f'TAP {x} {y}', .4)
    if 'UI_TAP accepted=1' not in text:
        raise RuntimeError('Tap not accepted')
    screen(expected)

def alarms():
    text = command('STATE', 1)
    if 'ringing=0 snoozed=0' not in text or 'storage=ESP_OK' not in text:
        raise RuntimeError('Active alarm or unhealthy storage; do not run navigation soak')
    rows = re.findall(r'^ALARM_SLOT .*$', text, re.M)
    if len(rows) != 8:
        raise RuntimeError('Incomplete alarm snapshot')
    return rows

try:
    connection.open()
    collect(12)
    screen('home')
    original = alarms()
    # Keep this bounded test away from armed or running user alarms.
    if any('enabled=1 ' in row for row in original):
        raise RuntimeError('An alarm is enabled; leaving it untouched')
    for cycle in range(args.cycles):
        tap(410, 288, 'settings')
        tap(356, 87, 'display')
        tap(130, 282, 'settings')  # Cancel
        tap(120, 203, 'ha')
        tap(405, 28, 'ha_setup')
        tap(120, 282, 'ha')       # Cancel; never save credentials
        tap(80, 282, 'settings')
        tap(66, 288, 'home')
        tap(180, 288, 'alarms')
        tap(220, 85, 'alarm')
        tap(130, 282, 'alarms')   # Cancel; never save an alarm
        tap(66, 288, 'home')
        if alarms() != original:
            raise RuntimeError('Alarm records changed unexpectedly')
        print(f'Cycle {cycle + 1}/{args.cycles}: home restored, alarm records unchanged', flush=True)
    collect(10)
    if len(heaps) < 2:
        raise RuntimeError('Insufficient clock heartbeat evidence')
    # A sustained loss larger than8KiB is not normal screen allocation variance.
    early = max(heaps[:min(3, len(heaps))])
    late = max(heaps[-min(3, len(heaps)):])
    if early - late > 8192:
        raise RuntimeError('Possible persistent heap loss; inspect receipt')
    print(f'PASS {args.cycles} cycles; {len(heaps)} RTC heartbeats; heap range {min(heaps)}..{max(heaps)} bytes; early/late {early}/{late}', flush=True)
    print(f'Receipt: {output}', flush=True)
finally:
    if connection.is_open:
        # Avoid an extra USB reset on port close.
        os.close(connection.fileno())
        connection.is_open = False
    log.close()
