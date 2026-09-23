# Draft request to Waveshare — not sent

Subject: ESP32-C5-Touch-LCD-3.5 source, schematic and factory recovery image

I own the ESP32-C5 3.5-inch rounded-corner IPS 320×480 board with dual-band Wi-Fi.
The device is running its factory touchscreen demo; display and touch respond,
and the WLAN screen scans networks. USB diagnostics identify ESP32-C5 silicon
revision v1.2 and 32 MB flash. The existing application metadata reports project
`blink`, version `1`, compiled Sep 11 2026 with ESP-IDF v5.5.4. The application
header specifies 16 MB flash; please provide the matching project configuration.

Your exact-board Resources and ESP-IDF pages currently say the examples and
hardware resources are still being prepared. Please provide:

1. The matching factory-demo source and ESP-IDF project, including component
   versions/lockfile, sdkconfig defaults, partition table and build instructions.
2. Board schematic/pin map and a way to identify the matching PCB revision.
3. A factory recovery binary, checksum and complete flash offsets/settings.
4. Whether the factory firmware supports joining Wi-Fi or a serial configuration
   interface; if so, the documented commands/procedure.

Our intended use is a local-first clock/alarm with LVGL, RTC, touch and local audio.
We need verified board support for ST7796, FT6336, PCF85063, ES8311, AXP2101 and SD.
Please confirm these components for the supplied board revision.
