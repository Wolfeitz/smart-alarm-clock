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
