# Factory-derived board map

2026-09-23. Source: private factory application extracted from the verified full
flash backup (full-image SHA-256 recorded in HARDWARE.md). These findings are
reverse-engineered configuration evidence, not a vendor schematic. Hardware
checks must be recorded separately. No binary content or credentials are published.

The owner photo IMG_2840.jpeg identifies ESP32-C5-Touch-LCD-3.5-C, CH32V006
I/O expander, and header labels E8–E15. The factory driver calls itself
custom_io_expander_new_i2c_ch32v003; retain this naming mismatch rather than
assuming the chip is V003. Driver protocol can be compatible with multiple MCUs.

| Interface | Recovered configuration | Evidence in factory runtime address space | Physical validation |
|---|---|---|---|
| I2C0 | SDA27, SCL26, internal pullups, glitch filter7 | bsp_i2c_init 0x4201bade; config stores at 0x4201bafe–0x4201bb1c | Four scans: eight ACKs, zero errors |
| SPI2 bus | MOSI7, MISO2, SCLK6, quad pins unused | bsp_spi_init 0x4201ba5c, stores 0x4201ba7c–0x4201ba92 | Display rendering passed; MISO reads not tested |
| LCD SPI | CS8, DC5, mode0, 60MHz, queue2, 8-bit command/parameter | bsp_display_new 0x4201c07a, 0x4201c0ae–0x4201c0ce | Rendering passed at20MHz |
| LCD panel | ST7796, 16 bits/pixel, native reset GPIO -1 | 0x4201c120–0x4201c15e and driver-name string | Software reset and rendering passed |
| Touch | I2C address0x38, interrupt GPIO3, reset GPIO -1 | config 0x4201c1ee–0x4201c242; FT6336 constructor | Aligned drawing/CLEAR confirmed; interrupt not tested |
| I/O expander | I2C address0x24, 400kHz | bsp init at 0x4201bfe4; constructor 0x42022150 | Pending |
| Backlight | Expander command byte0x05 followed by one PWM byte; 0–100 mapped to 0–255 | bsp brightness 0x4201c25a calls 0x42022308 | PWM160 command succeeded; brightness range untested |
| Audio I2S | MCLK unused(-1), BCLK23, WS10, DOUT25, DIN24 | default 72-byte std config at 0x4227f3a8, GPIO words at 0x4227f3d8 | Pending |

I2C struct layout was checked against official ESP-IDF v5.5.4
components/esp_driver_i2c/include/driver/i2c_master.h. Audio configuration was
checked against the same tag's esp_driver_i2s/include/driver/i2s_std.h; factory
code copies 72 bytes into the std-mode config. SPI fields follow spi_bus_config_t
and esp_lcd_panel_io_spi_config_t; checked against those headers at v5.5.4. Executable LCD testing is still pending.

The expander initialization sets direction mask0x23 to output and mask0x40 to
input, toggles mask0x03 high/low with delays, then sets mask0x23 high. The board
functions of these individual bits are not yet established. This was initially withheld pending a concrete startup failure; see the
cold-start investigation below for the evidence supporting its controlled test.

Offline artifacts: local-config/board-analysis/segments.json, segment-*.bin,
irom.S, and matching IDF header copies. Original backup remains unchanged.

Next: address-only I2C ACK tests on the recovered bus; then chip identification
and read-only RTC/touch tests using verified register definitions. LCD/power
writes follow only after enough of the expander/power path is established.

## Physical bus receipt

IDF v6.1 board-probe 1.0 completed rounds0–3 with identical results:
`0x18, 0x24, 0x34, 0x38, 0x51, 0x6b, 0x6e, 0x70`, eight ACKs, zero errors.
Addresses 0x24 and 0x38 match factory expander/touch configuration. Individual
chip identities have not been established by the address-only test, including
0x6e. Do not label all responders solely from conventional address tables.

Firmware is currently the board-probe, not the factory GUI. Boot log confirms
ESP-IDF v6.1 and the measured 32MB flash with configured 16MB accessible range.
Logs: local-config/board-probe/serial.log and write.log. Physical display/touch,
RTC content, audio and cold-power-start behavior remain untested by this probe.


## Display/touch acceptance

Owner confirmed the final test works correctly on 2026-09-23 ("Perfect!").
ST7796 landscape480x320: BGR565, invert=true, swapXY=true, mirrorX=true,
mirrorY=true. Touch reports portrait coordinates; final mapping is
screen_x=479-raw_y, screen_y=raw_x. Initial reflection and opposite-corner
button activation were corrected using user observations and raw touch logs.

Current firmware is display-touch, superseding board-probe. Uses software LCD
reset and existing power rails, with expander command5 PWM160; no PMU or
expander direction/reset writes. Warm-reset display/touch acceptance is complete.
Cold-power startup, sleep/wake, interrupt input and full brightness range remain
unqualified. The product UI/LVGL and alarm features are not implemented yet.

## Cold-start reset initialization (2026-09-23)

Battery is disconnected, confirmed by owner. USB power loss produced a blank
display and every FT6336 read failed, although the ESP32 application kept running.
Read-only PMU snapshot: ID03=4a, enable90=ff/91=01, ALDO2 voltage93=1c.
All LDO enables were already set; no PMU writes were required for this test.
Register semantics: [X-Powers AXP2101 datasheet, pp42–43](https://files.waveshare.com/wiki/common/X-power-AXP2101_SWcharge_V1.0.pdf).

Factory expander constructor writes command2=ff and command3=00.
Direction flag polarity at 0x42022606–42022624 is output=1, input=0;
BSP initialization therefore writes directionbf (bit6 input), output03,
waits50 ticks, output00, waits50 ticks, output23. Diagnostic uses500ms
between pulses and200ms settling before panel init. Exact net labels remain
unverified; this is the factory sequence for this board, not a guessed pin map.

With the sequence installed, touch_probe=ESP_OK and DISPLAY_ALIVE errors=0
through251 reads. No voltage/charging changes. This establishes communication
recovery after the failure; owner visual and fresh power-cycle checks pending.
It supersedes the earlier no-expander-writes description above.
