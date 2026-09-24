# Verification

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
- Test repeated boots and power loss during settings updates. For long-lived task,
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
alone is not proof of radio state. This diagnostic is volatile: restart restores
normal networking. Resume after any offline test. It does not simulate power loss.

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
