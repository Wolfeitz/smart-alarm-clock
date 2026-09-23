# Hardware readiness — 2026-09-23

Status: USB communication and factory-image preservation verified. Exact-board
source/schematic unavailable in checked public vendor resources. Firmware bring-up
is not complete. Rob owns the physical board; this record separates device evidence,
user observations and vendor specifications.

## Purchased variant

User supplied the product URL `https://www.waveshare.com/esp32-c5-touch-lcd-3.5.htm?sku=35419`.
The official board documentation maps **SKU 35419** to
**ESP32-C5-Touch-LCD-3.5-C**. This resolves the ordered variant, not the PCB revision.
The storefront itself rejected automated retrieval (HTTP 403); the mapping comes
from the successfully retrieved official documentation SKU table.

## Confirmed on the connected device

| Item | Evidence |
|---|---|
| MCU | Esptool 5.4.0 detects ESP32-C5, silicon revision v1.2 (not PCB revision) |
| USB | Espressif 303a:1001, USB Serial/JTAG, /dev/ttyACM0 |
| Flash | 32 MB physically detected, manufacturer 0x20 / device 0x4019 |
| Crystal | 48 MHz in initial identification and subsequent ROM-only security probe |
| Security | ROM reports Secure Boot disabled, flash encryption disabled |
| Application | Factory partition project `blink`, version `1` |
| Build metadata | Sep 11 2026 09:52:58; ESP-IDF `v5.5.4` |
| Image configuration | DIO, 80 MHz flash, **16 MB header setting**, despite 32 MB physical chip |
| Application integrity | Espressif image-info reports valid checksum and appended validation hash |
| Partition table | At 0x8000; its embedded MD5 validates |

The no-reset verification session reported a conflicting crystal estimate while an
existing flasher stub was running. A subsequent ROM-only connection after reset
again reported 48 MHz. Preserve this tool-context caveat; do not configure a 26 MHz
crystal based on the stub-session warning.

User observations: screen cycles colors, tapping opens the factory interface,
and WLAN can be viewed but the user could not join a network. Serial logs were
received, including a ticking `Now_time` value dated 2050-01-01. This does not prove
correct RTC operation, every touch coordinate, audio or any other peripheral.

## Factory backup

Private local folder: `local-config/hardware-backups/2026-09-23/` (Git-ignored,
directory mode 0700; artifacts 0600). The full dump may contain NVS configuration
and must not be published. This is a local recovery copy, not an independent backup.

- File: `factory-flash.bin`, exactly 33,554,432 bytes.
- SHA-256: `a963c18040b476d61ec4dc8f630f1b1e38c5b61cc69f6c530a8a7c6676d302ab`.
- `read-flash 0 ALL`: completed successfully.
- `verify-flash 0 ...`: digest matched the entire device image before resuming it.
- Device reset to existing firmware after verification; no flash write/erase or
  eFuse modification was performed. A subsequent serial capture received 6,313 bytes,
  reached app_main() and reported `Init PMU SUCCESS!`; GUI behavior was not
  physically re-observed by the agent.
- Supporting local files: SHA256SUMS, inspection.json, read-flash.log,
  verify-flash.log and extracted factory-app-partition.bin.

| Partition | Offset | Size |
|---|---|---|
| nvs | 0x9000 | 0x4000 |
| otadata | 0xd000 | 0x2000 |
| phy_init | 0xf000 | 0x1000 |
| factory | 0x20000 | 0x4e2000 |
| ota_0 | 0x510000 | 0x380000 |
| assets | 0x890000 | 0x20000 |
| storage | 0x8b0000 | 0x100000 |

Only the factory partition contained an identified application image in this
inspection. Its project name `blink` does not mean the app merely blinks an LED;
the user-observed touchscreen demo and binary metadata are distinct evidence.

## Vendor specifications and source gap

[Board documentation](https://docs.waveshare.com/ESP32-C5-Touch-LCD-3.5) names
ST7796 LCD, FT6336 touch, PCF85063 RTC, ES8311 audio, AXP2101 power management,
QMI8658 IMU, 8 MB PSRAM and a 320×480 panel. These component/memory claims have
not all been independently probed. PCB revision and verified GPIO wiring remain
unknown; other Waveshare 3.5-inch boards must not supply assumed pin assignments.

[Factory instructions](https://docs.waveshare.com/ESP32-C5-Touch-LCD-3.5/Instructions-For-Use)
document WLAN scan/display, not joining a network. No supported serial command
interface was found in the checked documentation. This is a documentation finding,
not proof that no undocumented functionality exists.

Checked on 2026-09-23:

- [Resources](https://docs.waveshare.com/ESP32-C5-Touch-LCD-3.5/Resources-And-Documents): examples and hardware resources still being prepared.
- [ESP-IDF guide](https://docs.waveshare.com/ESP32-C5-Touch-LCD-3.5/ESP-IDF): example resources still being prepared; screenshot SDK version is illustrative.
- [Recovery firmware](https://docs.waveshare.com/ESP32-C5-Touch-LCD-3.5/Firmware-Flashing): test firmware still being prepared.
- Exact `waveshareteam/ESP32-C5-Touch-LCD-3.5` GitHub API lookup returned 404; bounded repository search found other board sizes, not a matching vendor project.

The broad plan's assumption that a matching vendor example would already be
available is currently unproven. Do not substitute a different board's firmware.
A [vendor request](hardware/VENDOR-REQUEST.md) is drafted but has not been sent.
A seller-provided source/download URL may resolve this gap.

## Toolchain and next execution boundary

No idf.py or esptool was initially on PATH, and no ESP-IDF checkout was found in
the bounded locations searched (Downloads, /opt, Projects3 and ~/Projects).
This does not prove none exists elsewhere. uv, CMake, Ninja and Python 3.12.13
are available. Esptool 5.4.0 was run in a temporary uv environment; no system
package, ESP-IDF SDK or compiler toolchain was installed.

The factory metadata supports ESP-IDF v5.5.4 as the first compatibility candidate.
Upstream tag resolves to commit `735507283d5b2f9fb363a1901172dbd9e847945d`.
LVGL version, board component versions and sdkconfig are unresolved; a generic
IDF compiler installation alone would not establish working board support.

Next: obtain matching board source/schematic, reconcile exact PCB revision and
component pins, then pin dependencies, build diagnostics and verify the resulting
image before a separately scoped flash operation. Recovery commands are documented
in SETUP; an actual restore has not been performed.
