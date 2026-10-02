# Offline clock acceptance

## Current owner direction — power-off requirement removed (2026-10-01)

Power-off, full power-loss, battery/RTC-retention and interrupted-power testing
are not required for this project milestone, goal completion, or continued
development. Do not request these tests, defer them as future gates, or mark work
blocked because they are absent. This supersedes historical checklists, blocked
audits and the original goal wording. Do not claim those unperformed tests passed.
Keep offline operation, normal processor-restart recovery and settings integrity
requirements. Continue authorized development independently of optional physical
qualification; ask for physical action only when necessary to establish feasibility.

**Current board:** clock with Wi-Fi picker, manually selected Summerfield weather,
current/day forecast, network time/RTC sync and dark home/weather UI. Owner previously accepted
the clear local tone provisionally, but on September30 explicitly reported it is
not loud enough to wake someone. Audibility is demonstrated; acceptable wake-up
volume is unresolved and requires external playback and/or a louder local output.

Owner expanded scope to weather/location and continued integration development.
Home Assistant light setup/control is installed; authenticated operation is pending. Last audit2026-10-01. Installed application with selectable backgrounds and live Wallhaven rotation, SHA256:
`8247c382d40ce3eee83051c91ecbe7f8534f13600b1ad5cca24899994dd43dd7`.
Application-only flash verification and subsequent navigation receipts
are `background-worker-flash.log` and `background-worker-live.log` under
`local-config/clock`. These do not repeat historical offline alarm recovery tests.

| Requirement | Evidence | Status |
|---|---|---|
| Installed ESP-IDF clock application | Application-only esptool hash verification; CLOCK_READY and RTC heartbeat in local-time-runtime.log | Demonstrated |
| Readable display and aligned touch | Owner confirmed time/date, drawing/CLEAR and Dim/Brighten | Clock foundation demonstrated; owner reports current screen looks good after power return |
| RTC-backed offline time | RTC write/readback and reset without TIME command; system/RTC agree within one second | Demonstrated across MCU reset |
| Local time/date entry | Set time installed; host calendar/leap/DST tests pass; RTC adapter already hardware-tested | On-device LVGL navigation/save tested; further physical UX review optional |
| Editable persistent alarms | Eight-slot editor installed; NVS save/reset readback preserved disabled06:43 weekdays62; exact save acknowledgment implemented | Persistence and on-device LVGL editing/save demonstrated |
| Local scheduled trigger | Once alarm triggered on physical board; consumption saved before playback | State transition demonstrated |
| Audible local sound | Owner confirms clear audible tone; factory music louder but distorted; accepts current tone for now | Audibility demonstrated; owner now reports wake-up loudness insufficient |
| Snooze and dismiss | Real five-minute snooze re-rang after300.01 seconds; dismissal cleared state | Scheduler and on-device LVGL event paths demonstrated; touch alignment previously owner-confirmed |
| Restart recovery | Current1fa9f06 image: offline ringing, snooze and dismissal survived three processor resets; RTC valid, alarm overlay restored and saved slots preserved (offline-recovery-1fa9f06-runtime.log). Brightness persistence was previously demonstrated | Processor reset demonstrated; power-loss testing excluded from requirements |
| Saved brightness | Owner confirmed Dim/Brighten; brightness25 survived reset, restored160 | Demonstrated |
| Independence from Wi-Fi | RTC, engine, NVS, UI and audio remain local; HTTPS in separate lower-priority worker | Current1fa9f06 image: SDK radio-off RTC deadline triggered persisted alarm, local audio writes succeeded, snooze/dismiss passed; all eight alarm records restored exactly and network reconnection verified (offline-background-1fa9f06-runtime.log) |
| Factory recovery preserved | Private32MB factory backup and prior whole-image restoration/hash proof; factory partitions retained | Demonstrated; no further factory rewrite needed |

The functional offline-clock milestone is supported by the evidence above under
the owner's revised requirements. Power-loss testing has been removed entirely.
Additional acoustic/physical UX qualification remains optional and cannot block
feature development. No unperformed test is claimed as passed.

Evidence and commands are recorded in WORK.md; private runtime receipts are under
local-config/clock. Historical entries below retain their original evidence limits;
any old requirement or blocker conflicts are superseded by the owner direction above.

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

Historical offline-check image: NETWORK OFF returned ESP_OK, RTC scheduled the
temporary once alarm with durable consumption; I2S writes, Snooze and Dismiss
passed. All eight alarm records restored exactly; NETWORK ON and weatherHTTP200
confirmed reconnect. This checks disconnected operation, not battery/power loss.

Historical navigation-soak image:20navigation cycles/240transitions across
settings/display/HA/setup/alarms/editor/home passed, preserving all eight alarm
records.25RTC heartbeats remained within2seconds; observed heap88728..89028bytes
with no net early-to-late loss. Timestamped receipt and reproducible command are
in WORK/TESTING. This bounded test does not establish indefinite uptime.


## Latest installed-image scope (2026-09-24, c782e14)

The prior c782e14 image corresponds to the monotonic snooze-countdown fix.
Its receipts are `snooze-build.log`, `snooze-flash.log`, and `snooze-runtime.log`
under local-config/clock. That run proves verified application flash, RTC startup,
RTC/system agreement within one second, storageESP_OK, unchanged eight alarm
records and settings/media/setup/cancel/home navigation. It does not repeat all
historical offline-trigger, five-minute snooze, MCU-recovery or navigation-soak
checks. Host and real-LVGL preview checks cover the new countdown calculation.

Audio cancellation review: the running alarm checks its active flag before each
256-frame block at22050Hz (about11.6ms of generated audio). The pinned SDK defaults
to six240-frame DMA buffers (about65.3ms of capacity). These describe software
buffering, not measured acoustic stop latency; each driver write has a1000ms timeout.
The owner has accepted the current audible tone level, but final physical control
and full-power-loss behavior remain unverified. No further physical test is
requested here: the owner's batching/no-power-off instruction remains in force.

Current-image offline check on c782e14 subsequently passed: SDK acknowledged radio
off, the RTC deadline triggered a temporary once alarm with persistenceESP_OK,
I2S transmission completed, Snooze/Dismiss changed local phases, and all eight
alarm records were restored exactly. Radio-on acknowledgment and weatherHTTP200
confirmed reconnection. This is not a repeat of the five-minute snooze-duration
check or acoustic/power-loss acceptance. Receipt:
`local-config/clock/offline-snooze-c782e14-runtime.log`.


The subsequent home-layout image was installed and verified. Its build/flash
and home-card/navigation checks passed, with unchanged eight alarms, storageESP_OK
and RTC/system agreement. It changes presentation only; the c782e14 offline test
above remains historical evidence for the unchanged alarm implementation.
Receipts: home-build.log, home-flash.log, home-layout-runtime.log. Full visual polish
remains future work after regular functionality, as reconfirmed by the owner.


The saved-selection image added optional external-media selection.
Selection runtime receipt confirms RTC startup/agreement, storageESP_OK, exact eight
alarm preservation and setup/cancel/home navigation. No authenticated external
playback occurred. Offline alarm execution and settings schema are unchanged;
prior offline tests remain scoped to their recorded images.


Latest installed image separates the HA media backend from generic player ownership.
Its backend-runtime.log verifies RTC startup/agreement, storageESP_OK, all eight
alarm records unchanged and media/setup/cancel/home navigation. Backend-host.log
includes a synthetic non-HA backend test, not real non-HA playback. No offline alarm
code or settings layout changed; physical acceptance remains batched.


Latest installed image adds opt-in external alarm playback, disabled by default.
Host tests cover the independent eight-second local deadline, fresh exact-media
confirmation, latched fallback and cancellation/cleanup failures. Application-only
flash was hash-verified; USB navigation preserved all eight alarm records and
reported storageESP_OK, RTC/system agreement and weatherHTTP200. Receipts:
remote-alarm-host.log, remote-alarm-flash.log, remote-alarm-runtime.log.
Authenticated external playback and acoustic fallback remain unverified; these
are included in the combined end-stage acceptance, not a development gate.

The same installed image also passed the automated offline-default alarm check:
radio stopped, RTC alarm triggered with persistenceESP_OK, I2S writes succeeded,
Snooze/Dismiss worked, all eight alarm records restored exactly, then Wi-Fi and
weather recovered. September24 receipt inspected on September30:
`local-config/clock/offline-remote-default-runtime.log`. This is software-driven
device evidence and does not replace the deferred acoustic/power-loss checks.

September30 installed sound-cancellation fix invalidates sound tests across alarm
transitions. Host tests prove stale queued/running contexts cannot resume after
dismissal. New-image boot/navigation/audio-transport checks passed; all eight
alarms remained unchanged. The prior offline scheduler test is historical; this
change does not repeat it or prove acoustic latency. See audio-cancel-runtime.log.

## Optional physical review procedure (not a completion or development gate)

Run this once integrated functionality and interface work are ready, honoring the
owner's instruction to batch physical checks. The operator's observations are
required; USB acknowledgments alone do not count as touch or audible proof.
Record the actual firmware hash, date, power sources, selected alarm slot and
results beside the existing installation receipts. Do not assume the battery is
connected from earlier discussions.

Before testing, capture all eight alarm records and the display settings. Select
an unused slot; do not replace an enabled personal alarm. Keep a private settings
backup if using diagnostic tools, since the partition also contains credentials.
Keep remote alarm use disabled for this offline acceptance. Stop Wi-Fi through the
existing NETWORK OFF diagnostic and verify its acknowledgement; do not erase the
saved network or change the router. Check the RTC agrees with local time.
NETWORK OFF now saves the preference across reset. Require NETWORK_SETTINGS and
NETWORK_BOOT enabled=0 after restart, valid RTC-source startup and no network time
correction during the observation. This isolates offline boot without changing
the router or erasing credentials.

1. **Physical editing and brightness.** Use the touchscreen to save a once alarm
   for at least two minutes ahead, with today's local date. Reopen it and verify
   the saved time/date and enabled state. Exercise Dim/Brighten, or the display
   settings when night scheduling is enabled. Text must be readable, touch must
   hit the visible controls, and each brightness change must be apparent.
2. **Offline sound and controls.** Observe the alarm reaching its scheduled time
   without network. Record whether sound is audible at the provisionally accepted
   level. Tap the visible Snooze button: sound must stop, the countdown must show
   approximately five minutes, and the clock must remain responsive. Wait for
   the repeat, then tap Dismiss. Verify no sound resumes and the once alarm is
   disabled. Capture the device transitions alongside the physical observations.
3. **Restart with retained power.** Save the chosen brightness and a future alarm,
   then perform an MCU reset. Verify those settings persist and time is recovered
   from RTC before any network correction. This is a reset test, not evidence of
   complete power loss. Historical MCU recovery receipts already cover active
   ringing and snooze; repeat on the final image if its recovery path has changed.
Finish by restoring the original alarm/display settings and checking all eight
records exactly, turning Wi-Fi back on if it was originally on, and verifying
that no test alarm remains armed or snoozed. Leave the verified application
installed. Never restore the factory demo merely to end this test.

Further physical editor/control and acoustic observations may improve product
qualification but are not prerequisites for continued development. Power-off,
RTC-backup and interrupted-save tests are not part of this procedure.

September30 persistent-offline image passed a radio-off MCU restart: saved off
flag loaded, radio boot stayed off, RTC/system agreed, no network activity was
observed, and all eight alarms were unchanged. Wi-Fi was restored with existing
credentials and weather/network time recovered. This proves reset isolation, not
full power loss or scheduling on this new image. Receipt: radio-persist-runtime.log.

## Current-image offline restart regression (2026-09-30)

Installed719ffda (SHA559fbcc45ff8e35f3cc65e6311cb3e4a6aeeef6cdce5fd8b712dcbdd55357ba9) passed radio-off RTC alarm trigger, successful audio
transmission, persisted snooze through a processor reset, and persisted dismissal
through a second reset without re-ringing. Radio stayed off across both resets;
RTC/system agreement was within one second. All eight original alarm records and
initial enabled radio were restored, with weather reconnect confirmed. Receipt:
`local-config/clock/offline-recovery-719ffda-runtime.log`. This is hardware evidence
for normal restart recovery after the failed-load protection change; it does not
prove physical audibility, full-power-loss atomicity or battery-backed retention.


## Acceptance audit after calendar/Wi-Fi update (2026-10-01)

Rehashed the built application and checked its successful flash receipt, matching
installed-image identity above. The post-installation receipt shows valid RTC
heartbeats within one second, calendar navigation and no alarm-setting changes.
The September30 offline recovery receipt remains the evidence for radio-off
triggering, persisted snooze/reset and persisted dismissal/reset. The October1
image changes UI and network selection, not alarm execution or persistence; it
must not inherit a claim of final physical acceptance from that earlier receipt.

Outstanding acceptance remains the combined physical procedure above: current
finger-operated alarm editing/snooze/dismiss, acoustic behavior, actual backup
power/RTC retention, complete interruption recovery and interrupted-save behavior.
The owner has deferred full-power tests until integrated functionality is ready.
No physical check is requested by this audit, and no completion claim is made.
