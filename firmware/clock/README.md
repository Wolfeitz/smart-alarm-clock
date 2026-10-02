# Clock firmware

Current application for Waveshare ESP32-C5-Touch-LCD-3.5-C, using ESP-IDF6.1 and
LVGL9.4.0. See the root README for features and docs/USER-GUIDE.md for operation.

## Build

Activate your ESP-IDF6.1 environment using its installation-specific activation
script (a conventional source checkout supplies export.sh), then from the repo root:

```sh
idf.py -C firmware/clock -B "$PWD/local-config/clock/build" \
  -D SDKCONFIG="$PWD/local-config/clock/sdkconfig" set-target esp32c5
idf.py -C firmware/clock -B "$PWD/local-config/clock/build" \
  -D SDKCONFIG="$PWD/local-config/clock/sdkconfig" build
```

Only run set-target when creating/reconfiguring a build. The manifest and lockfile
pin managed components; ESP-IDF downloads them. ST7796 is included from
../display-touch/components/st7796. Outputs and local sdkconfig live in ignored
local-config/clock. Do not commit downloaded dependencies or generated firmware.

## First installation

Identify the correct USB port and board, close serial monitors, and preserve a full
factory flash backup before replacing vendor firmware. Backup/recovery details in
../../docs/SETUP.md are a historical record from the development board: substitute
your own serial device and never assume its serial number or host paths apply.

This firmware uses an app at0x20000 and a dedicated clockcfg NVS partition at
0xa00000. The build uses16MB flash addressing on the32MB board. A stock board needs
this project's bootloader/partition table/application together; an application-only
write is appropriate only after the matching layout is installed.

For an intentional first installation on the matching board, with backup complete:

```sh
idf.py -C firmware/clock -B "$PWD/local-config/clock/build" \
  -D SDKCONFIG="$PWD/local-config/clock/sdkconfig" -p /dev/ttyACM0 flash
```

Replace /dev/ttyACM0 with your actual port. Flashing replaces factory application
behavior. Do not use erase-flash as a routine setup step. Existing device credentials
and factory backups must remain private.

## Updating this project's installed firmware

With the matching project partition layout already installed, preserve bootloader,
partition table and settings using the app-only path from the activated environment:

```sh
python -m esptool --chip esp32c5 --port /dev/ttyACM0 --baud 460800 \
  --after hard-reset write-flash --flash-mode keep --flash-size keep \
  --flash-freq keep 0x20000 local-config/clock/build/esp_link_clock.bin
```

Check the written hash verification. Serial permissions can disappear on USB
re-enumeration; use your OS's normal serial access mechanism.

## Linux USB permissions: finding your user and port

If flashing or monitoring reports **Permission denied**, grant your login user
read/write access to the board's serial device. On the original development machine:

```sh
sudo setfacl -m u:rob:rw /dev/ttyACM0
```

`rob` is the Linux username; `/dev/ttyACM0` is the device node. Neither is universal.
Find yours with:

```sh
whoami
ls -l /dev/serial/by-id/
ls -l /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
```

Look for the **Espressif USB JTAG/serial debug unit** in the by-id list. Its symlink
points to the current device node (for example, `../../ttyACM0`). With more than
one board connected, use the serial number to distinguish them. Resolve the exact
entry you selected, replacing the example name with your actual entry:

```sh
readlink -f /dev/serial/by-id/usb-Espressif_YOUR_DEVICE_SERIAL-if00
```

Then grant access to that device. For example, if your board is `/dev/ttyACM0`:

```sh
sudo setfacl -m "u:$(id -un):rw" /dev/ttyACM0
getfacl /dev/ttyACM0
```

The ACL should show your username with `rw-` access. Use that same device path (or
its stable by-id link) for `--port` / `-p` in the flash and provisioning commands.
Only grant access to the board you identified. If no serial device appears, check
that the USB cable supports data and the board is connected; changing permissions
cannot create a missing port. `setfacl` and `getfacl` are supplied by the Linux
`acl` package if they are missing from your system.

**This permission is temporary.** USB disconnect/reconnect or re-enumeration after
a reset can recreate the device node and remove the ACL. Recheck the port and rerun
the command when needed. This is a Linux host-access step, not a firmware setting;
it is unnecessary if your account already has read/write access. It is not a
request to power-cycle the clock as a development test.

## Time and settings

Configure Wi-Fi and US ZIP location on the device. Resolved location selects a
persisted timezone rule with DST; America/New_York is the initial fallback.
Manual time entry is available in Settings. For USB provisioning from a trusted
host clock, run `python scripts/set-board-time.py --port /dev/ttyACM0` from the root.
It requires pyserial and validates the acknowledgement.

PCF85063 stores UTC with a validated format marker. Invalid RTC data is not treated
as valid time; scheduling waits for a valid clock. NVS stores alarm/configuration
state separately. No power-off test is required for development or contribution.

## Ownership

Alarm scheduling/settings, audio, sensors and network operations have separate
service ownership. LVGL observes snapshots and sends commands. Network HTTP does
not run in UI callbacks. Snooze/dismiss and local fallback do not require a server.
See ../../docs/PROJECT.md and ../../docs/SONOS.md for contracts and known limits.

## Tests and rendering

After configure/build has fetched managed dependencies:

```sh
python scripts/test-clock-host.py
python scripts/test-sonos-protocol.py
cmake -S firmware/clock/preview -B local-config/clock/preview-build
cmake --build local-config/clock/preview-build -j4
local-config/clock/preview-build/clock_preview /tmp/clock.ppm weather
```

Host checks need a C compiler and Expat development headers/library. The preview
renders actual LVGL code with synthetic service data into a PPM image. It is not a
hardware emulator. See ../../docs/TESTING.md for focused scenarios and limitations.
