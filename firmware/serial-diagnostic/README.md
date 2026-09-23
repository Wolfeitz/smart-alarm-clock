# USB serial diagnostic

Bounded deployment/recovery test for the identified ESP32-C5. Prints IDF version,
target, silicon revision, configured flash size and a monotonic heartbeat counter.
It does not initialize board peripherals, NVS, radio or PSRAM.

Pinned SDK: ESP-IDF v6.1, commit fff9895c82d744c7237be8847347bdd1b07c6643.
DIO/80MHz/16MB matches factory headers; measured physical flash is 32MB.
`esp_flash_get_size` reports the configured 16MB accessible range here.
Partition layout deliberately places the test app at the factory offset 0x20000;
it is temporary and is not the product partition design.

Build from the repository root:

```sh
source /home/rob/.espressif/tools/activate_idf_v6.1.sh
export IDF_PY_BUILD_JOBS=4
idf.py -C firmware/serial-diagnostic -B "$PWD/local-config/flash-test/build" -D SDKCONFIG="$PWD/local-config/flash-test/sdkconfig" set-target esp32c5 build
```

Flashing replaces factory bootloader/table/application. See docs/SETUP.md for
backup and recovery boundaries; do not treat the build command as permission to
repeat a flash. Test receipts and limitations are in docs/WORK.md.
