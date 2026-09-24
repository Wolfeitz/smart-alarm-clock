# Reusable project review — 2026-09-23

Owner requested a wider search of GitHub, Reddit and specialist projects. Reddit
was used for discovery, followed by inspection of upstream code and license files.
No third-party application code from this review has been copied into the firmware.

| Candidate | Concrete useful material | Fit / decision |
| --- | --- | --- |
| [ESPHome modular LVGL buttons](https://github.com/agillis/esphome-modular-lvgl-buttons) | `ui/weather/today.yaml`, `forecast.yaml`, theme definitions, entity-specific tiles | Best reference for future HA cards and reusable weather widgets. MIT root license verified. ESPHome YAML needs adaptation to our C/LVGL service snapshots; retain independent local alarms. |
| [Waveshare C5 2.8-inch examples](https://github.com/waveshareteam/ESP32-C5-Touch-LCD-2.8) | ESP-IDF LVGL9 weather layout, sensor/RTC/audio/BSP examples | Useful same-chip reference. Its ST7789/CST3530 display/touch hardware differs from ours; never import its pins or entire image. Inspected weather `main.c`, custom and generated entry points: UI exists, but custom_init is empty and those files contain no HTTP/Wi-Fi implementation despite header claims. Generated files also carry NXP-specific notices; root Apache2 alone is insufficient evidence for copying every asset. |
| [Frixos](https://github.com/ArtLogicIKE/frixos) | ESP-IDF/LVGL clock, substantial `f-wifi.c`, smart-home integration | Useful complete-product reference for connection state and page flow. Its free-use license restricts use to personal/noncommercial purposes. Do not introduce its large application dependencies into the existing small firmware. |
| [ESP32 flight radar](https://github.com/delphicchen/esp32_flight_radar) | Combined clock/weather/HA device and `alarm.h` interface | Useful interaction reference; different board/framework and CC BY-NC-SA4.0 terms. Our already-tested local scheduler need not be replaced. |

Pinned source inspection:

- Waveshare: `4d7a40b75cfe609f87b619296db2d96536fe800e`
- Modular LVGL: `163d92a795f7636cced9f47b24f87e6bb22ddfcc`
- Frixos: `73be6b4ccf92115e5eb75c50145b92c749329a0c`
- Flight radar: `ffb0edc3c32c60592c7893f02b20ba93141757dd`

The exact C5 3.5-inch [Waveshare ESP-IDF page](https://docs.waveshare.com/ESP32-C5-Touch-LCD-3.5/ESP-IDF#example)
still says examples are being prepared. That limits direct factory-app reuse;
it does not justify ignoring mature generic examples. Current firmware already
uses Espressif Wi-Fi/TLS/SNTP/NVS, esp_codec_dev, pinned LVGL and the ST7796 component.
The local IDF6.1 `examples/wifi/getting_started/station` example was checked and
explicit WPA3 SAE derivation configuration incorporated into our adapter. The
nearby-network flow uses IDF's scan API and LVGL's standard list/keyboard widgets.

Discovery pointers: [Reddit HA panel](https://www.reddit.com/r/homeassistant/comments/1rr5g38/built_a_small_home_assistant_control_panel_with_a/),
[Reddit clock/weather/radar](https://www.reddit.com/r/esp32/comments/1uozln9/esp32_flight_radar_with_weather_echo_and_home/),
[Reddit Frixos](https://www.reddit.com/r/esp32/comments/1ut1zua/followup_my_esp32_projection_clock_got_its/),
[Home Assistant community panel](https://community.home-assistant.io/t/esphome-lvgl-cinema-room-touch-panel-waveshare-esp32-s3-4-3/1022834).

Next integration work should start with an existing entity/widget pattern and
adapt it to a small data/control interface, instead of inventing another complete
setup application. Reference appearance can be reused as a design pattern while
checking individual code/font/icon licensing before importing actual files.
