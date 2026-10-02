# Optional hardware capabilities

Verified2026-10-02. These are feasibility findings, not installed features unless
explicitly identified. The clock, local sound and snooze/dismiss stay independent.

## Voice

The onboard ES8311/microphone provides an input path. Current audio code only
implements output; microphone capture and recognition are not installed.
[Espressif ESP-SR](https://github.com/espressif/esp-sr) lists ESP32-C5 WakeNet support
and specifically WakeNet9s for C5. Its current MultiNet command-recognition support
lists S3/P4/S31, not C5. A small local wake-word-triggered snooze is plausible;
arbitrary configurable spoken commands are not a drop-in supported feature here.

A prototype needs input routing, suitable wake model, bounded CPU/memory use and
false-trigger/echo testing while the alarm or external speaker is sounding.
A remote speech/intent service could support richer commands, but optional voice
must not replace working offline touch controls or make alarms cloud-dependent.
No model, microphone recording, external audio upload or voice service enabled.

## Zigbee and tactile outputs

[ESP Zigbee SDK](https://docs.espressif.com/projects/esp-zigbee-sdk/en/latest/esp32c5/introduction.html)
supports coordinator, router and end-device roles. The clock could act as the
coordinator for a small compatible network without a separate hub or HA. This
requires an implemented stack, commissioning and actual device cluster/protocol
support; the presence of802.15.4 hardware is not an installed Zigbee integration.

[Espressif C5 coexistence](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/api-guides/coexist.html)
documents shared RF resources and unstable performance for Wi-Fi station plus
Thread/Zigbee router operation. Espressif recommends separate SoCs/antennas for
Wi-Fi gateways. Do not rely on a speculative single-radio mesh for alarm delivery.
BLE could directly control a purpose-built haptic puck with documented commands.
A vibration sensor is an input, not a bed shaker; a shaker needs its own actuator,
driver and power. No haptic product, strength or protocol has been qualified.

## Camera and physical accessories

The purchased C variant includes a rear-facing BF3901, as shown in the owner's
photo and [Waveshare board documentation](https://docs.waveshare.com/ESP32-C5-Touch-LCD-3.5).
It faces away from the user in a normal bedside position. No face/gesture feature
is installed; a different enclosure or camera placement would be needed for a
front-facing use. The microphone/IMU avoid that placement issue for alarm controls.

The BAT connector is internal, item7 in the board diagram: two-pin MX1.25 for a
single3.7V lithium battery. The exposed GPIO header is not that battery connector.
Check connector polarity against the board before connecting a battery; matching
plug shape alone does not establish polarity. Battery fit/capacity has not been
selected or tested. Do not change charger limits without battery specifications.

The case speaker outlet is between the microSD slot and BOOT button in the vendor
case illustration. Exact internal mounting has not been inspected. A passive dock
would need its acoustic inlet aligned with this side outlet and clearance for the
thicker case and controls. No measured acoustic gain or compatibility with Amazon
item B0FF473SWY is claimed; that exact listing could not be retrieved. Sonos remains
the selected external speaker direction.

## Sensor and rotation implementation

SHTC3 readings and QMI8658 sampling are installed and measured. The approximately
33.4C reading inside the running case is not representative of the room, per the
owner. Removed it from the home weather card; keep it explicitly labeled Inside
case in diagnostics/settings. No guessed calibration offset.

Shake-to-snooze is installed, optional and off by default. Auto-rotation is also
optional:180degree landscape only, steady upright delay, flat/near-flat hold,
active-touch deferral. See USER-GUIDE and WORK for evidence and physical tuning
limits. No full power-off test is required for development or goal completion.
