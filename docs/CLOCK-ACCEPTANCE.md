# Offline clock acceptance

**Current board:** clock with Wi-Fi picker, manually selected Summerfield weather,
current/day forecast, network time/RTC sync and dark home/weather UI. Owner accepts
the clear local tone provisionally; desired alarm loudness may need later hardware
or external playback improvements.

Owner expanded scope to weather/location and continued integration development.
Home Assistant light setup/control is installed; authenticated operation is pending. Last audit2026-09-24. Installed application SHA256:
`956d2cd39bb555c3891e6dfa205046ac2dd7263e277301e1c47290d795857e21`.

| Requirement | Evidence | Status |
|---|---|---|
| Installed ESP-IDF clock application | Application-only esptool hash verification; CLOCK_READY and RTC heartbeat in local-time-runtime.log | Demonstrated |
| Readable display and aligned touch | Owner confirmed time/date, drawing/CLEAR and Dim/Brighten | Clock foundation demonstrated; owner reports current screen looks good after power return |
| RTC-backed offline time | RTC write/readback and reset without TIME command; system/RTC agree within one second | Demonstrated across MCU reset |
| Local time/date entry | Set time installed; host calendar/leap/DST tests pass; RTC adapter already hardware-tested | On-device LVGL navigation/save tested; physical editor use pending |
| Editable persistent alarms | Eight-slot editor installed; NVS save/reset readback preserved disabled06:43 weekdays62; exact save acknowledgment implemented | Persistence and on-device LVGL editing/save demonstrated; physical editor pending |
| Local scheduled trigger | Once alarm triggered on physical board; consumption saved before playback | State transition demonstrated |
| Audible local sound | Owner confirms clear audible tone; factory music louder but distorted; accepts current tone for now | Demonstrated and provisionally accepted; desired loudness remains a limitation |
| Snooze and dismiss | Real five-minute snooze re-rang after300.01 seconds; dismissal cleared state | Scheduler and on-device LVGL event paths demonstrated; physical button check pending |
| Restart recovery | Ringing, snooze, dismissal and brightness survive MCU resets | Demonstrated |
| Saved brightness | Owner confirmed Dim/Brighten; brightness25 survived reset, restored160 | Demonstrated |
| Power-return startup | Owner reports powered back on and current application looks good; prior battery state disconnected | Display/application recovery observed by owner; offline RTC retention without backup power is not established |
| Independence from Wi-Fi | RTC, engine, NVS, UI and audio remain local; HTTPS in separate lower-priority worker | Current image: radio stopped via SDK, RTC deadline triggered alarm, audio writes succeeded, snooze/dismiss passed; exact alarm restoration and network reconnection verified |
| Factory recovery preserved | Private32MB factory backup and prior whole-image restoration/hash proof; factory partitions retained | Demonstrated; no further factory rewrite needed |

Build/host tests cannot prove sound, physical controls or a complete power cycle.
Owner has confirmed successful power-return startup. Battery-backed RTC retention,
alarm recovery across full power loss, and physical alarm controls on the final
combined application remain separate acceptance checks.

Evidence and commands are recorded in WORK.md; private runtime receipts are under
local-config/clock. Reproduce pure logic checks with python scripts/test-clock-host.py.
Weather connection, HTTPS success, fresh parsed snapshot and RTC sync were
observed on-device; see WEATHER.md. The remaining human checks have been requested. Do not infer successful checks
from unanswered requests or from this checklist.

Owner defers further full power-off tests until regular functionality is complete.
Scheduled night mode is installed; interval/wake/migration host checks pass.
The current UI remains a proof of concept; visual redesign is deferred.

## Current evidence refresh (2026-09-24)

Rechecked the retained night-mode binary SHA256 against the installed-image receipt;
it matched the night-mode installation receipt (ec31e7d8…). No new flash or board reset was performed during this
audit. Historical recovery-fourth-summary.log explicitly reports ringing, snooze,
brightness and dismissal reset checks; snooze-duration-summary.log covers the real
five-minute interval. These receipts apply to the recorded firmware revisions,
not a new physical test of every path on today's image.

The consolidated host runner now also executes alarm_restart_test.c. It crosses
production engine → capture → encode → decode → restore boundaries, verifying
once-alarm consumption, remaining ring/snooze duration, exact120-second recovery
grace, rejection at121 seconds, implausible future deadlines, invalid wall time
during running snooze, and durable dismissal. All eight host suites pass. This
closes a host integration coverage gap; it does not establish physical power-loss
behavior or NVS writes interrupted by power loss. Physical alarm controls remain
pending; further full-power tests remain deferred by owner. Goal not complete.

Owner correction: physical acceptance is batched after integrated functionality,
not a prerequisite to continue development. HA update runtime/navigation checks
passed without physical input; see WORK for installed image and evidence.

Current-image offline check: NETWORK OFF returned ESP_OK, RTC scheduled the
temporary once alarm with durable consumption; I2S writes, Snooze and Dismiss
passed. All eight alarm records restored exactly; NETWORK ON and weatherHTTP200
confirmed reconnect. This checks disconnected operation, not battery/power loss.

Combined UI stability on current image:20navigation cycles/240transitions across
settings/display/HA/setup/alarms/editor/home passed, preserving all eight alarm
records.25RTC heartbeats remained within2seconds; observed heap88728..89028bytes
with no net early-to-late loss. Timestamped receipt and reproducible command are
in WORK/TESTING. This bounded test does not establish indefinite uptime.
