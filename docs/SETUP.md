# Connection, inspection and recovery

## Connect the existing factory firmware

Use a data-capable USB-C cable. On this host, the running kernel and its modules
must match. The initial mismatch was fixed by rebooting into 7.2.6-arch2-1; cdc_acm
then created /dev/ttyACM0. Inspect live state rather than assuming this path persists:

```sh
uname -r
lsusb -d 303a:1001
ls -l /dev/serial/by-id/
```

Observed stable device link:
`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_10:BD:A3:E3:52:74-if00`.
Use that identity to avoid opening another serial device. On this host, rob was
not in uucp. The owner granted temporary access with:

```sh
sudo setfacl -m u:rob:rw /dev/ttyACM0
```

That grant may disappear on unplug/re-enumeration. Do not change global device
permissions or account groups as an incidental troubleshooting step.

## Diagnostic tool used

The following pinned tool runs through uv with a project-specific temporary cache:

```sh
env UV_CACHE_DIR=/tmp/esp-link-uv-cache uv tool run --from esptool==5.4.0 esptool --help
```

Esptool diagnostic commands enter ROM/stub mode and may reset the board; they are
not a way to control the factory GUI or set Wi-Fi unless the firmware explicitly
provides such an interface. Read commands can load a helper into RAM without
writing flash. Avoid simultaneous serial monitors. Never issue write/erase/eFuse
commands during identification.

## Backup already performed

The backup and full-device digest comparison succeeded; see HARDWARE for hashes
and limits. These are the actual commands used from the repository root:

```sh
env UV_CACHE_DIR=/tmp/esp-link-uv-cache uv tool run --from esptool==5.4.0 esptool --port /dev/ttyACM0 --baud 460800 --after no-reset read-flash 0 ALL local-config/hardware-backups/2026-09-23/factory-flash.bin
env UV_CACHE_DIR=/tmp/esp-link-uv-cache uv tool run --from esptool==5.4.0 esptool --port /dev/ttyACM0 --baud 460800 --before no-reset verify-flash 0 local-config/hardware-backups/2026-09-23/factory-flash.bin
```

Keep the original backup immutable. Future captures must use a new path. The first
command keeps the application stopped so it cannot change NVS before the second
command verifies it; the second returns the device to its existing firmware.

## Recovery procedure — verified 2026-09-23

Only after an explicitly authorized firmware replacement/recovery, re-identify the
same device and verify the backup's SHA-256 against HARDWARE. Confirm security
settings remain compatible and close competing serial readers. The full dump is
mapped from address 0 and preserves the original 16 MB boot-image configuration
within the 32 MB physical flash. Do not force a new flash-size header setting.

The authorized restoration used `write-flash 0 <verified-full-backup>` with the
same pinned tool and matching device, preserving header parameters. Its automatic
full-image digest check verified all 32MB while the application was stopped, then
the board was reset. This restores NVS/settings and assets as well as firmware.
Do not perform a separate full erase, overwrite the backup, or infer peripheral
health solely from a successful image verification. Never restore this image to
another board model.

## Firmware development

ESP-IDF **v6.1** is installed and the upstream hello_world example builds for
**esp32c5**. This verifies the compiler environment, not Waveshare peripherals.
Factory firmware uses v5.5.4; compatibility of the exact-board BSP with v6.1
remains to be verified when sources become available.

Installation locations:

- EIM v0.19.0: `local-config/toolchains/eim/eim`.
- SDK: `local-config/toolchains/esp-idf/v6.1/esp-idf`.
- Compiler tools, Python environment and activation: `/home/rob/.espressif/tools`.
- EIM registry/configuration/logs: `local-config/toolchains/`.
- Local dfu-util prerequisite: `local-config/toolchains/prerequisites/bin`.
- Upstream example copy and output: `local-config/toolchain-smoke/hello_world`.

From the repository root, activate and rebuild the existing smoke example:

```sh
source /home/rob/.espressif/tools/activate_idf_v6.1.sh
idf.py --version
export IDF_PY_BUILD_JOBS=4
idf.py -C local-config/toolchain-smoke/hello_world build
```

Initial setup copied `examples/get-started/hello_world` from the pinned SDK and
ran `idf.py -C local-config/toolchain-smoke/hello_world set-target esp32c5 build`.
These commands compile only. The example uses generic defaults including a 2MB
flash header; it is not a prepared board diagnostics image. Ignore the automatic
flash suggestions printed at the end of a successful IDF build.

No system packages changed. EIM's attempt to copy an OpenOCD udev rule to `/etc`
failed for lack of permission; this does not affect compilation. Revisit device
permissions only if USB debugging is needed. Existing serial access has its own
per-device ACL described above.

No working build/flash command for our clock exists yet. Exact-board source,
schematic, PCB identity and LVGL/BSP dependencies remain missing. The separately authorized flash/recovery test used firmware/serial-diagnostic
and passed deployment, USB heartbeat and full-image restoration checks. The
factory application is running again; screen/touch confirmation is pending.
Future firmware writes require their own task scope.


## Verified deployment diagnostic

Source and build command: `firmware/serial-diagnostic/README.md`. Its USB heartbeat
reported ESP-IDF v6.1 and ESP32-C5 on the physical board. Factory restoration then
verified the entire preserved image and returned to app_main/PMU startup.
A passive pyserial reader should set DTR and RTS false **before opening** the port.
The first direct-reset capture received no bytes; esptool `run` followed by that
passive-open sequence produced factory startup logs. Do not equate a silent
monitor with failed firmware without checking reset/serial control state.
