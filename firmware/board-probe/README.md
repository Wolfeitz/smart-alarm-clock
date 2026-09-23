# Board bus probe

IDF v6.1 USB-serial diagnostic. Uses factory-derived I2C0 SDA27/SCL26 and sends
address-only probes to nonreserved 7-bit addresses 0x08–0x77 every five seconds.
No peripheral register payloads, PMU configuration or GPIO sweep. ACKs establish
address presence, not device identity. See docs/hardware/RECOVERED-MAP.md.

Build from repository root:

```sh
source /home/rob/.espressif/tools/activate_idf_v6.1.sh
export IDF_PY_BUILD_JOBS=4
idf.py -C firmware/board-probe -B "$PWD/local-config/board-probe/build" -D SDKCONFIG="$PWD/local-config/board-probe/sdkconfig" set-target esp32c5 build
```

This replaces the factory GUI when flashed. Leave a successful diagnostic installed
for continuing bring-up unless the owner requests restoration. The full factory
backup and verified recovery process remain available.
