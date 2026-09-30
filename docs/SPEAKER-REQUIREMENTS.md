# Speaker requirements for Duncan's clock

Owner has no external speaker for this project. Selection remains open. The
onboard speaker is audible but insufficient for the intended wake-up use.

Preferred architecture to investigate: powered desktop speaker(s) with an analog
line input, driven locally by the clock through a verified audio output stage.
This is a direction, not a claim that this board exposes a ready-to-use line-out.
The board has an ES8311 codec and NS4150B speaker amplifier; its speaker connection
must not be treated as a conventional line-level output. Qualify an appropriate
DAC/output adapter or engineered conversion before choosing a wiring solution.
No pins or electrical compatibility are inferred here.

Required behavior:
- Play locally without internet, phone, cloud account, home server or HA.
- Stay ready overnight. Avoid sleep requiring a button, phone or app; if signal
  wake is used, verify low-level chimes and the start of speech are not lost.
- Recover automatically after power restoration with usable input and volume.
- Produce intelligible speech, pleasant music and clearly audible escalating
  alerts at the bed without distortion. Judge this in the room, not by watts.
- Let the clock raise/lower audio within an established physical gain limit;
  support prompt stopping for Snooze/Dismiss. A powered AUX speaker need not
  have a software API if the clock controls its own output level and samples.
- Remain on the alarm input; optional phone Bluetooth must not silently steal
  the only audio path. Network music must yield predictably to the alarm path.
- Fit a dorm desk and run from mains without daily charging. Whole-system backup
  during a mains outage is separate from surviving an internet/router outage.

For a network speaker, additionally require documented local playback/control,
volume/mute/state feedback, bounded start time and local-file playback. Validate
operation without WAN and after overnight idle. Even a local Wi-Fi API depends
on the LAN, so retain an independent adequately audible local output. Streaming
Spotify is optional music, not the only offline alarm source. An AUX input is
valuable but its wake, priority and offline behavior must be tested.

Bluetooth-only is not a direct solution: ESP32-C5 supports Bluetooth LE rather
than Classic A2DP speaker streaming. A transmitter/other hardware would be a
separate integration with its own pairing/reconnection behavior.

College suitability matters. Sonos lists portal-login/guest and WPA/WPA2
Enterprise networks as unsupported and requires app/products on one subnet.
Do not assume dorm Wi-Fi will permit speaker discovery or control. Era100 offers
line-in using its separate adapter and signal Autoplay after initial app setup;
that alone does not qualify unattended alarm behavior for this project.

Primary sources checked2026-09-30:
- https://docs.waveshare.com/ESP32-C5-Touch-LCD-3.5
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/api-guides/ble/overview.html
- https://support.sonos.com/en-gb/article/sonos-system-requirements
- https://support.sonos.com/en-au/article/play-line-in-on-your-era-100

Next selection decisions: willingness to use a short audio cable, footprint and
budget. No model purchase, host service installation or board modification is
requested by this document. Qualify board-to-speaker audio before recommending
an exact purchase as compatible.
