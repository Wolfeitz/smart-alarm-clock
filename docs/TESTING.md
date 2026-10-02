# Verification

## Current owner direction — power-off requirement removed (2026-10-01)

Power-off, full power-loss, battery/RTC-retention and interrupted-power testing
are not required for this project milestone, goal completion, or continued
development. Do not request these tests, defer them as future gates, or mark work
blocked because they are absent. This supersedes historical checklists, blocked
audits and the original goal wording. Do not claim those unperformed tests passed.
Keep offline operation, normal processor-restart recovery and settings integrity
requirements. Continue authorized development independently of optional physical
qualification; ask for physical action only when necessary to establish feasibility.

## Available now

From the project root:

```sh
python scripts/verify-bootstrap.py
```

This standard-library check verifies required bootstrap artifacts, README's local
index links, archived product-brief byte identity and complete/dispositioned source
coverage. It does not validate the semantic correctness of requirements, scan all
secrets, compile firmware or test hardware. Review the change against accepted scope
as well. The generic ESP32-C5 compiler smoke build is documented in SETUP and
passed with ESP-IDF v6.1. It is not a product-firmware or hardware test.

The original unrelated installer must retain SHA-256
`8033f0bc484e863ad274645db2e90e3ac7dd17c35c53de4589653850140344bf`.
Do not execute it as a test. Build outputs and local secrets are ignored; ignore
rules do not substitute for reviewing staged content before any commit.

## Fresh-session bootstrap proof

Launch a fresh bounded Codex session in the project, without prior conversation or
the canonical AAE source in context. It should identify loaded guidance, discover
the operating index/current task and run the documented verification command.
Capture tool output/exit status, loaded instruction evidence and the final report.
Do not supply the canonical command in the probe prompt: discovery is being tested.
Do not allow the probe to read the full AAE source or run the unrelated installer.

[Official Codex instruction discovery](https://learn.chatgpt.com/docs/agent-configuration/agents-md)
describes global then project guidance and deeper directory precedence, with
AGENTS.override.md preferred per directory. This is documented loading behavior;
only an actual fresh-session result proves this project's setup. No override or
runtime permission change is installed. Record projection presence, documented
loading, observed discovery and command execution separately in WORK. A sandbox
error or timeout is unproven execution, not a passing result.

## Product firmware verification gates (not run)

- Verify exact board/BSP/SDK/LVGL compatibility and pin dependencies before defining
  reproducible build commands. Preserve required component lockfiles in Git.
- Add host tests for UI-independent logic: recurrence, once/date semantics, midnight,
  DST gaps/repeats, forward/backward time correction, invalid RTC, snooze/dismiss,
  duplicate prevention, persistence defaults/corruption/migration and reboot behavior.
- Test remote success, timeout, unavailable target and HTTP success without actual
  playback; test stale/incorrect confirmation, local timer independence and races.
- Test weather freshness, malformed responses and service outage; bounded retries
  and clean recovery without UI stalls. Never put real tokens into fixtures/logs.
- Compile for the verified target; record tool versions and clean-build commands.
- Physically demonstrate each HW contract in PROJECT with firmware revision, board
  identity, configuration (redacted), procedure, observed result and date.
- Demonstrate audible local fallback with Wi-Fi, HA, Spotify and target unavailable;
  remove SD and corrupt selected media; exercise local snooze/dismiss and reboot.
- Test processor restarts and settings reload/error handling. For long-lived task,
  timer, queue, network and audio resources, verify cleanup and bounded recovery;
  record bounded soak duration, memory behavior and failures when relevant.

A host test, successful build or service acknowledgement is insufficient to claim
physical wake-up reliability. V1 needs the full physical acceptance in PROJECT plus
reproducible build/flash/provisioning documentation and reviewed secret hygiene.


## Deployment/recovery check (2026-09-23)

The tracked serial-diagnostic built under IDF v6.1, flashed with verified hashes,
and emitted repeated matching target/version heartbeats on the physical board.
Full factory restoration passed the whole-image hash check and returned to
factory app_main/PMU startup. This is deployment/recovery evidence, not completion
of the product firmware or peripheral gates above. Physical GUI check is pending.

## Recovered-bus test (2026-09-23)

firmware/board-probe builds under v6.1 and runs on the board. Four complete
address-only scans on SDA27/SCL26 returned the same eight ACKs with zero errors.
See hardware/RECOVERED-MAP.md for addresses and provenance. This validates the
bus mapping but not device identity, register semantics, or display/audio paths.

## Display/touch test (2026-09-23)

IDF6.1 build and flash passed. Panel initialization/rendering stayed responsive;
serial logs recorded touch coordinates with zero I2C errors. Owner observed
initial reflection, then confirmed the final corrected drawing/CLEAR behavior.
Final transform: x=479-raw_y, y=raw_x; swapXY/mirrorX/mirrorY enabled for LCD.
This qualifies warm-reset diagnostic display/touch, not cold-power startup,
LVGL integration, audio, RTC, or alarm reliability.

## Clock foundation acceptance (2026-09-23)

firmware/clock builds under IDF6.1 with pinned LVGL9.4.0. RTC codec/date and
host timezone-transition checks pass (command in its README). Application-only
flash hash verified; USB TIME command received ESP_OK. MCU reset then reported
CLOCK_INIT rtc=ESP_OK source=RTC / offline without another TIME command.
Three ten-second reports showed advancing system/RTC time within one second,
rtc_status=ESP_OK, stable free heap224652. Serial recorded both brightness
events, and owner confirmed readable clock/date, advancing seconds and both
buttons work. This is not battery-retention, alarm or audio acceptance.

## Alarm recovery (2026-09-23)

Host suites in firmware/clock/tests cover engine recurrence/DST/grace/monotonic
snooze, settings corruption/truncation/schema1 migration, active-phase recovery,
and RTC encoding. Compile each with its matching main modules, C11,
-D_POSIX_C_SOURCE=200809L, -Wall -Wextra -Werror.
Hardware reset proof: ringing resumes, snooze resumes, dismissal persists,
brightness persists; serial snapshots confirm restored disabled test slots.
See WORK for private logs, failed intermediate attempts and installed hash.
Audible sound, physical alarm editor/controls and battery/full-power-loss behavior
remain unverified. No build or serial success establishes those physical results.

Run the consolidated host suites: `python scripts/test-clock-host.py`. This includes
local time-entry validation for leap dates, invalid dates, spring-DST gaps and the
earlier fall-back occurrence. Physical Set time/Save time interaction remains a
separate acceptance check.

## Weather and UI previews

`python scripts/test-clock-host.py` also compiles the weather parser against the
pinned managed cJSON source, with nesting limit16. Fixtures are public weather/
geocoding responses, not credentials. Tests reject missing/null data, wrong units,
wrong timezone, stale/future values, ambiguous ZIPs and trailing/truncated JSON;
manual ZIP precedence and DST rules are checked independently of LVGL.

The actual LVGL UI can be rendered without hardware using the commands in
`firmware/clock/preview/README.md`. Synthetic preview data does not prove network
integration. On-device acceptance must include scan, connection, TLS forecast,
time sync/RTC handoff, retry after loss, cached-data freshness and local alarm
responsiveness while network work is active.

Night-mode checks: the consolidated host runner covers interval boundaries,
overnight/daytime schedules, invalid-time/manual fallback and wake expiry. Settings
fixtures verify version1/2 migration to version3, preserving active snooze deadline,
consumed date and manual brightness. The windowless `night-test` exercises the real
UI's touch-wake and ringing-brightness path. Full-power-off checks are deferred by
owner until regular functionality is complete; do not request or run them meanwhile.

The host runner also tests a complete encoded-settings alarm restart path in
`alarm_restart_test.c`, including once consumption, snooze expiry boundaries and
dismissal. Simulated byte persistence does not prove NVS power-failure atomicity.

HA host checks compile the production service against synthetic queue/NVS/HTTP
boundaries: rejected duplicate requests, state confirmation, HTTP200 without a
change, failure expiry, authentication failure, stale/offline state, and failed
configuration retaining the prior endpoint/entity. No real HA credentials needed.

The windowless UI preview `audio-error` injects an unavailable-audio snapshot,
checks the home warning and ringing error title, then exercises Snooze and Dismiss.
This tests UI behavior under reported failure, not a physical codec failure.

HA service checks also inject mutex/queue allocation failures and a disabled
network worker; safe snapshots and rejected commands are required. Network-startup
error handling uses checked SDK netif steps instead of the default factory, whose
internal assertions were verified in the pinned IDF source. No physical OOM or
radio-failure injection is claimed by these host checks.

Serial `NETWORK OFF` and `NETWORK ON` commands pause/resume the board radio through
the network worker without changing saved credentials. Require both the queued
request acknowledgment and `NETWORK_RADIO ... status=ESP_OK`; a queued request
alone is not proof of radio state. The setting now persists across restart in a
separate radio_enabled key; legacy missing keys default to enabled. Corrupt or
unreadable keys leave the radio off. Resume after an offline test if originally
on. This does not simulate power loss. Host tests cover reload and storage errors;
actual-LVGL radio-test covers both buttons/labels.

Combined navigation stability check (board attached, no alarms enabled):

```sh
/home/rob/.espressif/tools/python/v6.1/venv/bin/python scripts/check-clock-navigation.py \
  --port /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_10:BD:A3:E3:52:74-if00 \
  --cycles 20
```

This opens USB (which can reset this board), then repeatedly visits settings,
night settings, HA/setup, alarm overview/editor and home. It uses Cancel/back only,
never Save, sound, erase or power-off. It checks all eight alarm records, idle alarm
state, RTC agreement, crash output and heap samples. More than8KiB early-to-late
heap loss fails as suspicious; a pass is a bounded stability observation, not proof
of unlimited uptime or absence of all leaks. Timestamped receipts are private under
`local-config/clock`. Do not run while a user alarm is enabled or active.

Media parser/service host suites cover reported capability flags, bounded parsing,
explicit play/pause/track/volume actions, HTTP200 without changed playback, failed
confirmation expiry, authentication errors, server-bound targets, failed saves and
allocation failures. The UI preview covers stale-state control disabling. These
checks use synthetic HA responses; they are not real external playback evidence.

USB provisioning checks: `python scripts/test-clock-setup.py` covers tagged
acceptance/persistence acknowledgments, missing or failed saves, reordered replies,
input bounds, URL/entity validation and redirect refusal. The consolidated host
runner also tests the firmware setup JSON parser, including decoded NULs and invalid
tags. Tests use synthetic data and never provision a real credential. A device
handshake and malformed/oversized message test confirms USB framing/recovery only;
it does not establish successful authenticated Home Assistant operation.

The host runner compiles production `settings_store.c` with a synthetic NVS boundary
and runs seven isolated scenarios: initialization failure, namespace-open failure,
read failure, corrupt stored bytes, write failure, commit failure, and save/reload.
It checks clockcfg/clock ownership, disabled defaults after failed reads, error
propagation, no commit after write failure, rejection of invalid settings before
writing, and encoded equality after reloading an alarm with active snooze metadata.
The fake NVS is not a flash emulator: these checks do not prove atomicity, wear
behavior, or recovery from physical power loss during a write.

Snooze countdown uses the alarm owner's monotonic runtime snapshot, separate from
persisted wall-clock deadlines used for recovery. Engine tests cover rounding,
earliest simultaneous snooze, expiry and dismissal. The `audio-error` LVGL preview
also asserts that Snooze shows `Rings again in 05:00` from the supplied snapshot.

Saved-media checks cover version1 player migration, incomplete/oversized input,
JSON escaping, PLAY_MEDIA capability gating, exact service body, failed-save
retention, and distinguishing playing state from requested-selection proof.
`media-test` now tests selection fields/save/start and seven stale controls.
`media-setup` renders the three-field setup. No real playback is requested.

`media_backend_test.c` links the production media owner and generic model without
HA, HTTP or JSON, using a synthetic backend with `test:local` connection identity
and `speaker/bedroom` target. It verifies configure/read/play/pause confirmation and
offline rejection. HA-specific parser/service tests still run through the real HA
adapter, including persisted selection migration and failures. This proves a code
boundary, not an implemented non-HA device integration.

`alarm_service_test.c` executes the production alarm-owner loop with fake RTOS,
wall/monotonic clocks, storage and audio. A trigger checkpoint failure must still
activate local sound; Snooze and Dismiss must work while writes fail, and the
five-second retry must persist dismissal/consumption without retriggering. A failed
tracked edit must preserve the old alarm and report its ticket/error; a later
successful edit must persist. These deterministic owner tests do not simulate
thread races, physical flash atomicity or audible output.

`alarm_output_test.c` checks the independent eight-second local deadline, wrong or
stale session proofs, lease expiry, fallback latching, disable/cancel and monotonic
timer wrap. `remote_alarm_test.c` runs the production network coordinator against
a simulated backend: exact/mismatched media, lost confirmation, muted player,
offline no-late-start, cancellation during POST, ambiguous-start cleanup and bounded
failed-stop retries. Media service/UI tests cover persisted opt-in and rejecting
an enabled remote alarm without a selection. No physical player is contacted.

## Sound-test cancellation policy

`python scripts/test-clock-host.py` includes the production audio_control atomic
policy: alarm start invalidates running/queued tests; Snooze/Dismiss invalidates
alarm playback; an on/off cycle between audio blocks cannot revive an old test.
Repeated unchanged owner updates preserve a valid context. audio.c checks the
token before each generated block and queues the token with each sound test.
These checks do not measure physical sound-stop latency or DMA drain time.

## Invalid-time owner startup

The production alarm-owner host harness also runs invalid-brightness,
invalid-dismiss and invalid-edit scenarios. A saved snooze must remain dormant
until time is valid. Brightness changes preserve that checkpoint; Dismiss and
alarm edits cancel it durably before recovery. Tests check persisted state and
audio state across the validity transition, without emulating battery hardware.

## Companion conditional-save boundary

Host owner tests exercise conditional-save, conditional-conflict, conditional-storage,
conditional-session and conditional-runtime: compare the published session/revision
inside the owner task, reject newer local edits and queued runtime changes, retain
old settings on storage failure and reject obsolete boot tokens. Conflict is a
separate snapshot flag. This internal API is not network authentication. A future
transport still needs pairing, request-result correlation, replay handling and
authorization; the prototype website is not connected by these tests.

The result-cache owner scenario preserves separate failed/successful completion
receipts when commands drain together and checks bounded eviction. The actual
LVGL preview `save-timeout-test` withholds receipts and verifies both alarm and
display editors report an unavailable result after ten seconds without claiming
success. This RAM cache does not persist across reboot or authenticate callers.

The `load-failure` owner scenario rejects automatic checkpoints and brightness/
display writes after a failed startup read, preserving the original stored data.
A failed explicit alarm save leaves protection active; a successful explicit save
establishes a new configuration. The actual-LVGL `load-failure-test` checks the
home warning and pre-save replacement notice. These inject storage errors in host
code; no physical flash corruption or interrupted-power behavior is claimed.

Actual-LVGL `snooze-clock-test` exercises Snooze → Show clock, countdown updates,
Settings/home navigation while snoozed, reopening the alarm card, another ringing
alarm taking foreground priority and brightness, and Dismiss restoring the normal
home card. Persistence and timers still belong to the unchanged alarm owner.

Actual-LVGL `calendar-test` checks December/January rollover, Today, and alarm
foreground priority. `signal-test` checks exact -60/-75 dBm boundaries, separate
connected/nearby readings and live connected-signal updates without rescanning.
These use synthetic snapshots; installed `WIFI_SCAN_LINK` records associated RSSI,
channel, matching-BSSID scan RSSI and best same-SSID RSSI without network identifiers.

`python scripts/test-background-decode.py` compiles the production JPEG adapter and
pinned LVGL TJpgDec with ASan/UBSan; fixtures exercise landscape/portrait/tiny
coverage, RGB565 color, guard words, cancellation, truncated and invalid data.
LeakSanitizer is disabled because it cannot run under the sandbox's ptrace.
The consolidated host runner includes background-source/query validation and
Wallhaven array/single-image response parsing. Actual-LVGL `background-test`
checks source/list/interval/Next controls, delayed save receipts, duplicate-tap
suppression, return to home only after confirmed persistence, failure/retry with
retained entries, and preserving configuration after an invalid URL. These synthetic checks do not prove real Wallhaven availability.

The JPEG host check compiles the private background_jpeg wrapper used on target.
After a firmware build, confirm background_jd_prepare/background_jd_decomp resolve
to flash implementations in esp_link_clock.map; generic jd_prepare/jd_decomp are
ESP32-C5 ROM symbols with an incompatible JDEC layout. Live acceptance also checks
weather plus wallpaper startup/reconnection for TLS allocation failures.
