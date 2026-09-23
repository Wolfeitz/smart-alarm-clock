# Display and touch bring-up

ESP-IDF v6.1 hardware diagnostic for the factory-reconstructed ESP32-C5
Touch LCD3.5 wiring. Shows a landscape 480x320 drawing surface, blinking square,
yellow touch marks and a CLEAR button. Uses no LVGL or product alarm logic yet.

- ST7796: SPI2 MOSI7/MISO2/SCLK6, CS8/DC5, 20MHz (factory used60MHz).
- FT6336: I2C0 SDA27/SCL26, address0x38, 100kHz polling; count register2 and
  coordinate bytes from3, verified against factory code.
- Backlight: expander0x24, command5, PWM160/255. No expander direction writes,
  reset-output toggles or PMU changes.
- Panel software reset; BGR565, inverted colors, swapXY and mirrorX/mirrorY enabled. Orientation adjusted from visual feedback;
  raw portrait touch maps to landscape x=479-raw_y, y=raw_x. Owner confirmed aligned drawing and CLEAR.
- DMA buffer is reused only after the SPI completion semaphore; one task owns
  display rendering and touch reads. Finite transaction timeouts.

Build from repository root:

```sh
source /home/rob/.espressif/tools/activate_idf_v6.1.sh
export IDF_PY_BUILD_JOBS=4
idf.py -C firmware/display-touch -B "$PWD/local-config/display-touch/build" -D SDKCONFIG="$PWD/local-config/display-touch/sdkconfig" set-target esp32c5 build
```

Pinned vendor code/provenance is in components/st7796. Five upstream source,
header and license files match Espressif's published SHA-256 checksums; only
local build integration differs. This C5 port is hardware-tested separately
from the upstream manifest's target list.

Acceptance: new screen visible, blinking square, marks track finger at all four
corners, CLEAR removes marks, repeated touch with zero I2C errors. Cold-power
startup is not proven by warm-reset success; board power initialization remains
for the next hardware qualification. Firmware remains installed after testing.
