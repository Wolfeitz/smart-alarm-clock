#!/usr/bin/env python3
"""Set the clock RTC from this host's UTC epoch using the USB TIME command."""
import argparse
import os
import time
import serial

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--port', default='/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_10:BD:A3:E3:52:74-if00')
args = parser.parse_args()
s = serial.Serial()
s.port, s.baudrate, s.timeout = args.port, 115200, 0.2
s.dtr = s.rts = False
s.open()
received = bytearray()
sent = False
success = False
expected = None
start = time.monotonic()
try:
    while time.monotonic() - start < 25:
        data = s.read(4096)
        if data:
            received.extend(data)
            print(data.decode(errors='replace'), end='', flush=True)
        if not sent and (b'CLOCK_READY' in received or time.monotonic()-start > 5):
            epoch = int(time.time())
            expected = f'TIME_SET status=ESP_OK epoch={epoch}'.encode()
            received.clear()
            s.write(f'TIME {epoch}\n'.encode())
            sent = True
        if expected is not None and expected in received:
            success = True
        if success and b'CLOCK_ALIVE valid=1' in received:
            break
finally:
    os.close(s.fileno())
    s.is_open = False
if not success:
    raise SystemExit('No successful TIME_SET acknowledgement received')
