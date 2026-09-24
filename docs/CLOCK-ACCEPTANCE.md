# Offline clock acceptance

**Temporary diagnostic state:** factory demo is installed for the owner-requested
music-volume comparison. Exact clock boot/table/app and settings snapshots are
preserved under local-config/clock/factory-comparison. Restore the clock after
comparison; this temporary state does not fulfill the installed-clock goal.

Scope: active goal only; weather, Home Assistant and Spotify remain deferred.
Last audit: 2026-09-23. Latest runtime change: full-level tone adjustment after ab10c17.
Installed application SHA256: c1f518f8d57dc25e8b5493f9a49af8b6cff7ca9bba7ae1487a47efc92e3182f1.

| Requirement | Evidence | Status |
|---|---|---|
| Installed ESP-IDF clock application | Application-only esptool hash verification; CLOCK_READY and RTC heartbeat in local-time-runtime.log | Demonstrated |
| Readable display and aligned touch | Owner confirmed time/date, drawing/CLEAR and Dim/Brighten | Demonstrated for clock foundation; new screens pending |
| RTC-backed offline time | RTC write/readback and reset without TIME command; system/RTC agree within one second | Demonstrated across MCU reset |
| Local time/date entry | Set time installed; host calendar/leap/DST tests pass; RTC adapter already hardware-tested | Physical editor use pending |
| Editable persistent alarms | Eight-slot editor installed; NVS save/reset readback preserved disabled06:43 weekdays62; exact save acknowledgment implemented | Persistence demonstrated; physical editor pending |
| Local scheduled trigger | Once alarm triggered on physical board; consumption saved before playback | State transition demonstrated |
| Audible local sound | Owner confirms Test sound is audible but barely; ES8311 and I2S checks also pass | Speaker output demonstrated; usable alarm loudness pending |
| Snooze and dismiss | Real five-minute snooze re-rang after300.01 seconds; dismissal cleared state | Scheduler demonstrated; physical buttons pending |
| Restart recovery | Ringing, snooze, dismissal and brightness survive MCU resets | Demonstrated |
| Saved brightness | Owner confirmed Dim/Brighten; brightness25 survived reset, restored160 | Demonstrated |
| Complete power loss | No battery connected; previous blank-screen problem received expander-reset fix | Final unplug/replug and RTC validity behavior pending |
| Independence from Wi-Fi | Application starts no Wi-Fi/network services; RTC, engine, NVS, UI and audio are local | Implemented; current tests run without network provisioning |
| Factory recovery preserved | Private32MB factory backup and prior whole-image restoration/hash proof; factory partitions retained | Demonstrated; no further factory rewrite needed |

Build/host tests cannot prove sound, physical controls or a complete power cycle.
No alarm reliability claim is complete while the audible output gate is open.

Evidence and commands are recorded in WORK.md; private runtime receipts are under
local-config/clock. Reproduce pure logic checks with python scripts/test-clock-host.py.
The remaining human checks have been requested. Do not infer successful checks
from unanswered requests or from this checklist.
