# Windowless UI preview

This builds the real `main/clock_ui.c` against pinned LVGL with synthetic service
snapshots and a memory framebuffer. It needs the managed components from the IDF
build, CMake, a C/C++ compiler, and no display server. It does not emulate radio,
NVS, touch hardware, RTC, audio or task scheduling. The displayed weather and
network names are deliberately fake.

From the repository root:

```sh
cmake -S firmware/clock/preview -B local-config/clock/preview-build
cmake --build local-config/clock/preview-build -j4
local-config/clock/preview-build/clock_preview /tmp/clock.ppm weather
local-config/clock/preview-build/clock_preview /tmp/weather.ppm weather detail
local-config/clock/preview-build/clock_preview /tmp/wifi.ppm weather wifi
local-config/clock/preview-build/clock_preview /tmp/location.ppm weather location
```

Omit `weather` for an offline home screen. PPM output is 480x320 RGB. Open with an
image viewer, or convert using Pillow when available. Review long labels, empty
states and keyboard screens as well as populated data before installing UI edits.

Night settings and actual-UI backlight policy check:

```sh
local-config/clock/preview-build/clock_preview /tmp/display.ppm weather display
local-config/clock/preview-build/clock_preview /tmp/night-check.ppm weather night-test
```

The latter asserts immediate touch wake,30-second expiry and ringing priority
through actual clock_ui callbacks with a stubbed backlight; it is not a physical
brightness measurement.

Alarm overview and edit/cancel navigation check:

```sh
local-config/clock/preview-build/clock_preview /tmp/alarms.ppm weather alarms
local-config/clock/preview-build/clock_preview /tmp/alarms-test.ppm weather alarms-test
```

The latter selects Alarm8, changes its hour through the actual editor, saves and
returns to the overview, verifies the other seven slots remain unchanged, and
checks Cancel navigation. Preview persistence is synthetic, not an NVS test.

`repeat-test` exercises daily/weekday/weekend presets, custom day changes, once
date-field visibility and rejection of an empty recurring schedule.

`media-test` checks play/pause, setup/cancel and disabling all playback controls
when the reported player state is stale. `media` renders synthetic player data.

Main navigation regression: `clock_preview /tmp/navigation.ppm weather navigation-test`
checks 25 cycles across all primary destinations, Settings/Weather setup-return
behavior, and unchanged alarm records. `settings` renders the settings hub.
