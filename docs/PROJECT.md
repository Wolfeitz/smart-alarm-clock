# Product requirements and architecture

Status: approved bootstrap distillation of the supplied brief, not authorization
to implement all V1 features. Owner: Rob. Date: 2026-09-23.
[Original brief](reference/PRODUCT-BRIEF.md) is reference-only. Current user scope
and the machine contract govern actions; vendor material provides technical facts.
Actual code, tests and physical measurements establish implementation evidence.

## Product and boundaries

Create a bedside clock that remains useful and sounds a local alarm when Wi-Fi,
Internet, Home Assistant, Spotify, weather and every external service are absent.
The target is the ESP32-C5 3.5-inch rounded-corner LCD development board identified
by Rob's receipt. The earlier brief identifies the vendor as Waveshare. The exact
receipt wording supplied by Rob on 2026-09-23 is:

> ESP32-C5 3.5inch Rounded Corner LCD Development Board, IPS  Panel, 320 × 480 Resolution, 262K Color, Supports 2.4GHz/5GHz Dual-band  Wi-Fi And Bluetooth 5 (LE)

This establishes the purchased product description: ESP32-C5, rounded-corner
3.5-inch IPS display, 320×480 resolution, 262K color, dual-band 2.4/5GHz Wi-Fi and
Bluetooth 5 LE. It is user-provided receipt evidence, not a physical board test.
The receipt excerpt does not identify a SKU, PCB revision, display/touch controller,
RTC, flash/PSRAM configuration or pin map. The PCF85063 RTC remains a claim from
the earlier brief pending vendor/board verification. Driver and SDK compatibility
must be established from the matching vendor example and actual board revision.

Subsequent user-provided product link identifies SKU **35419**, which Waveshare's
official documentation maps to **ESP32-C5-Touch-LCD-3.5-C**. See HARDWARE.md for
verified silicon/flash identity, factory backup and remaining board-support gaps.

Use ESP-IDF and C/C++, LVGL for local GUI, FreeRTOS tasks/events/queues, NVS for
configuration, vendor-supported drivers, SD/FAT where useful, ESP-IDF HTTP(S) and
SNTP. Exact SDK/BSP/LVGL versions, dependency locks and layout follow the verified
vendor integration point. No browser UI or direct Spotify streaming on the MCU.

Architecture: LVGL screens issue commands and observe application state. Clock,
alarms and settings are independent of LVGL. Hardware adapters own RTC, display,
touch, backlight, audio and storage. Network adapters own connectivity and service
integration. Each mutable state has an explicit owner; queues/events cross task
boundaries. Only the designated UI context performs LVGL operations. HTTP cannot
run in UI callbacks; integration work cannot starve alarm scheduling or local audio.
A central state model exposes clock, alarms, connectivity, weather, HA, music and
display state without exposing provider JSON to screens.

HA is the preferred weather/light/music boundary. A backend requires demonstrated
need and evaluation of existing infrastructure; availability alone is not a reason
to use a shared service. Spotify targets an external Connect-capable device. Verify
current integration capability and vendor terms when that work starts.

## Observable acceptance contracts

- **HW:** Repeated boots without crash/reset loops; independently demonstrate LCD,
  touch coordinates, LVGL rendering, RTC read/write, backlight, Wi-Fi, SD mount,
  audio initialization and external-speaker output, reset behavior and NVS read/write.
  Diagnostics distinguish not tested, unavailable, failed and demonstrated OK.
- **CLOCK:** Show useful valid RTC time before network; otherwise show unsynchronized
  state. NTP corrects system time and RTC; expose source, sync quality and last sync.
  Offline reboot retains reasonable time. Timezone handling includes DST, without
  hardcoded UTC offsets. Network failure cannot block UI.
- **ALARM:** Scheduling is UI-independent; persist multiple alarms including once,
  daily, weekdays, weekends and selected weekdays. Support enabled/disabled,
  armed/triggering/ringing/snoozed/dismissed states. Snooze and dismiss always work
  locally. Resolve time correction and recurrence edge cases before implementation.
- **FALLBACK:** Arm local fallback independently of remote playback requests. HTTP
  success alone never cancels fallback. Confirm actual playback within a short
  configured window (brief suggests 5–10 seconds); failure or uncertainty sounds
  local audio. Test Wi-Fi, HA, Spotify and playback-target outages individually.
- **AUDIO:** Local looping sounds, volume and graceful stop; selected-media failure
  uses a firmware-resident fallback. Removing SD cannot silence an alarm. Choose
  supported codecs from working vendor examples, not a speculative codec list.
- **UI:** Bedside-first home prioritizes time, next alarm and weather. Large touch
  targets and clear feedback; home, alarm list/editor, weather, music and settings
  screens. No dense dashboard or IMU gesture requirement.
- **NIGHT:** Dark theme, normal/night/manual brightness, scheduled night mode,
  temporary touch wake and persisted settings. Screen timeout is optional.
- **WEATHER:** Provider-neutral state with temperature, feels-like, condition, daily
  high/low, precipitation probability, freshness and availability. Cache last valid
  data; outages become unobtrusive stale state, never modal clock-blocking errors.
- **HA:** Read and toggle one configurable light, reflect observed state, degrade
  cleanly on failure. Read weather through the adapter. Protect HA credentials.
- **MUSIC:** External target/context playback, pause, previous/next, volume where
  supported, metadata/state; artwork only if resource behavior is acceptable.
  No Spotify video/Canvas playback. Alarm use inherits FALLBACK acceptance.
- **CONFIG:** Separate secret and non-secret configuration. Document provisioning;
  no credentials in source, samples or logs. Cover defaults, reload, corrupt/missing
  values, version migration and reset/power loss during configuration writes.
- **RELIABILITY:** Bounded retries/backoff; tolerate DNS/NTP/network/auth failures,
  missing/corrupt SD media, invalid RTC, malformed responses and reboots. Redacted
  subsystem logs include firmware version, reset reason and peripheral/time/alarm
  status. Application logging is separate from engineering evidence in WORK.

V1 requires all applicable contracts demonstrated on the physical device, reproducible
build/flash/provisioning instructions and no committed secrets. Host tests and builds
are necessary evidence for their claims but cannot prove audible device behavior.

## Non-goals and future compatibility

Defer AI/LLMs, TTS/morning briefings, calendars, Dexcom/LibreLinkUp/health data, voice,
camera, Zigbee/Thread, multiple-node synchronization, IMU snooze, elaborate HA
dashboards and enclosure functionality. Retain clean adapter boundaries; do not
implement placeholder services or speculative health interfaces. External services
are conveniences; clock, alarms, local audio and local controls have priority.

## Open decisions before affected implementation

1. Board revision, speaker/power requirements, RTC validity/backup behavior, USB
   interface, vendor example provenance, SDK/LVGL compatibility and recovery method.
2. Once-only alarm date semantics; skipped/repeated DST times; forward/backward clock
   corrections; duplicate prevention and behavior after reboot/missed alarm time.
3. Playback-confirmation freshness, correct target and context; cancellation races,
   fallback timer ownership and local snooze/dismiss behavior for remote playback.
4. Provisioning, secret storage, persistence migration, audio formats and memory
   budgets. Record choices only after evidence; do not invent a build/flash command.

## Source coverage

Every brief section has a disposition. Deferred means outside current execution,
not deleted from V1. Detailed task ordering lives in [WORK](WORK.md).

| Sections | Disposition | Owner / contract |
|---|---|---|
| 1-5 | adopted | Product, technology and architecture above; hardware facts unverified |
| 6 | next | HW readiness and physical diagnostics |
| 7-9 | deferred | CLOCK, ALARM, FALLBACK, AUDIO |
| 10-13 | deferred | UI, NIGHT, WEATHER, HA |
| 14-15 | deferred | MUSIC and FALLBACK |
| 16-21 | adopted | CONFIG, state ownership, RELIABILITY and TESTING gates |
| 22 | deferred | Build/flash/setup/hardware documentation grows with verified implementation |
| 23-24 | excluded-v1 | Future compatibility and non-goals above |
| 25-27 | adopted | WORK ordering, physical V1 acceptance and local reliability priority |

## Owner decision: interim audio level (2026-09-23)

After comparing factory music (louder but distorted), owner accepts the clock's
current clear local tone as an interim alarm level. It is audible but not yet at
the desired bedside alarm loudness. Retain local sound at this level; possible
future upgrades include a replacement speaker or external Sonos/Spotify playback.
External playback remains subject to independent local fallback and later
integration scope; this decision does not claim reliable remote playback.

## Expanded working-example scope (2026-09-23)

Owner authorizes location setup (ZIP/postal code), current weather and today's
forecast, with timezone derived from the selected location. After functionality,
polish the bedside interface. Keep provider adapters and presentation separate so
Home Assistant entities and later spoken weather/schedule summaries can be added
without coupling them to alarm execution. Weather/location work may proceed while
the remaining physical power-cycle check is pending; core offline guarantees remain.


## Portability clarification (2026-09-24)

Owner raised possible open-source distribution and users without Home Assistant.
Standalone clock/alarm functionality remains the base product; HA is optional.
Current weather uses its own provider and does not require HA. The external-player owner now uses a backend interface, with HA as the only
shipping implementation. Additional media adapters and removal of the personal
server-address default remain public-release work.
Do not advertise non-HA external playback as implemented. Voice/activation/morning
briefings are possible later capabilities, not a reason to install HA or make
local time/alarm execution depend on it. No publication or license choice requested.
