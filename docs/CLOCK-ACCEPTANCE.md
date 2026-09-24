# Offline clock acceptance

**Current board:** clock with Wi-Fi picker, manually selected Summerfield weather,
current/day forecast, network time/RTC sync and dark home/weather UI. Owner accepts
the clear local tone provisionally; desired alarm loudness may need later hardware
or external playback improvements.

Owner expanded scope to weather/location; Home Assistant and Spotify remain future
integrations. Last audit2026-09-23. Installed application SHA256:
`23d59810b8b3c866533caf1d3f80de7c339b37d672658ce9e966cbbb56787e87`.

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
| Independence from Wi-Fi | RTC, engine, NVS, UI and audio remain local; HTTPS in separate lower-priority worker | Offline local example demonstrated before provisioning; online UI/RTC responsiveness verified |
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
