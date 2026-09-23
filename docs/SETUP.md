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

## Recovery procedure — documented, not executed

Only after an explicitly authorized firmware replacement/recovery, re-identify the
same device and verify the backup's SHA-256 against HARDWARE. Confirm security
settings remain compatible and close competing serial readers. The full dump is
mapped from address 0 and preserves the original 16 MB boot-image configuration
within the 32 MB physical flash. Do not force a new flash-size header setting.

A reviewed restoration would use `write-flash 0 <verified-full-backup>` with the
same pinned tool and matching device, preserving header parameters, followed by
`verify-flash` while the application is stopped, and then a normal reset. This
restores NVS/settings and assets as well as firmware. Do not perform a separate
full erase, overwrite the backup, or treat this unexecuted restoration procedure
as proof of recovery. Never restore this image to another board model.

## Firmware development

No working build/flash command for our clock exists yet. Exact-board source,
schematic, PCB identity and LVGL/BSP dependencies are missing. Candidate SDK
v5.5.4 comes from factory metadata, not from the generic documentation screenshot.
Do not flash hello-world merely to prove a compiler works while it would replace
the only working board-specific demonstration without advancing peripheral proof.
