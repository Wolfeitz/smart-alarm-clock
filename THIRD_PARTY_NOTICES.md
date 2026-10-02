# Third-party notices and asset provenance

The root MIT license covers original project work. It does not replace licenses
on third-party code, fonts, services or downloaded media.

## Source included in this repository

- **Espressif ST7796 LCD component** under
  `firmware/display-touch/components/st7796/`: Apache-2.0. Original Espressif
  copyright/SPDX headers and the full `license.txt` are preserved in that folder.
- **Blue-hour wallpaper**: original AI-generated landscape, generated for this
  project. Source and conversion details are in `firmware/clock/assets/README.md`.
  It is not a Wallhaven download or a copy of the watermarked design reference.
  It is distributed with this project's original assets under the root license
  to the extent rights apply.

## Dependencies obtained during build

Pinned dependencies are specified in `firmware/clock/main/idf_component.yml` and
`firmware/clock/dependencies.lock`; managed sources are not committed here.

- ESP-IDF: Espressif framework; Apache-2.0 with component-specific notices.
- LVGL 9.4.0: MIT, with its bundled third-party notices. Montserrat font and symbol
  assets, and the TJpgDec implementation, retain their upstream notices/licenses.
- esp_codec_dev 1.6.2: Espressif audio component, Apache-2.0.
- cJSON: MIT.
- Expat: MIT.

The private JPEG wrapper includes the pinned LVGL TJpgDec source without stripping
its original notice. Firmware redistributors must also preserve relevant notices
from the resolved dependency tree; this inventory is not a replacement for them.

## Reference implementations and external services

SoCo informed the local Sonos protocol/favorites implementation; see docs/SONOS.md
for the inspected revision. SoCo is a separate project, not a bundled dependency.
The native C controller and its simulator are maintained here.

Weather, geocoding, IP-based location estimation, Wallhaven and optional media
services have their own availability, API terms and content rights. Runtime
Wallhaven images are not bundled or relicensed by this repository. The bundled
wallpaper remains available without those services.

No affiliation or endorsement by Espressif, Waveshare, Sonos, Spotify or Wallhaven
is implied.
