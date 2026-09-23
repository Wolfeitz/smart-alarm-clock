# RTC clock foundation

ESP-IDF 6.1 / ESP32-C5 clock with LVGL 9.4.0 pinned by manifest and lockfile.
Uses the proven ST7796 component from ../display-touch/components/st7796.
480×320 dark clock screen, date, seconds/timezone, source state, and a touch
brightness toggle. Alarm scheduling, audio, Wi-Fi/NTP, weather and persistent
user settings are not implemented in this milestone.

## Build and provision

From the repository root:

```sh
source /home/rob/.espressif/tools/activate_idf_v6.1.sh
export IDF_PY_BUILD_JOBS=4
idf.py -C firmware/clock -B "$PWD/local-config/clock/build" -D SDKCONFIG="$PWD/local-config/clock/sdkconfig" build
python scripts/set-board-time.py
```

For a fresh build directory, add `set-target esp32c5` before `build`.
The time script requires pyserial (available in the activated IDF environment),
serial access, and a trustworthy host clock. It writes current UTC through a
bounded `TIME <Unix seconds>` command and requires a successful acknowledgement.
Board serial identity and app-only flashing/recovery are in docs/SETUP.md.
Clock uses the existing diagnostic partition table; no NVS or factory data writes.

## Ownership and RTC format

One task owns LVGL and hardware I/O. board.c contains transport, reset startup,
fixed touch transformation and brightness. clock_service.c has no LVGL dependency;
it owns RTC validation/provisioning and system time. rtc_codec.c independently
validates BCD dates including leap years. No network calls exist in the UI loop.

PCF85063 at address0x51 stores UTC, 24-hour mode, years2000–2099. RTC RAM
register03=a7 marks the UTC convention. It is cleared before time writes and
set after successful time readback. An oscillator-stop flag, stopped/test/12-hour
mode, missing marker, or invalid calendar causes the UI to show unsynchronized
state. Crystal-load and correction settings are preserved; no PMU writes.
Register source: [NXP PCF85063A Rev7.3](https://www.nxp.com/docs/en/data-sheet/PCF85063A.pdf).

Display timezone defaults to America/New_York, matching the verified host;
POSIX EST5EDT,M3.2.0/2,M11.1.0/2 rules handle DST. This is not yet configurable.
Brightness is a session-only toggle. With the battery disconnected, RTC retention
through complete power loss is not assured: re-provision over USB if invalid.
RTC restore after MCU reset is checked separately from battery-backed retention.

## Host checks

```sh
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/clock/main firmware/clock/main/rtc_codec.c firmware/clock/tests/rtc_codec_test.c -o /tmp/esp-link-rtc-test
/tmp/esp-link-rtc-test
```

Hardware acceptance: visible readable clock, advancing seconds, correctly aligned
Dim/Brighten button, RTC readback agrees with system time, and an MCU reset restores
time without another TIME command. Full battery/USB-loss behavior remains a
separate acceptance gate. Logs and binaries remain private under local-config/clock.
