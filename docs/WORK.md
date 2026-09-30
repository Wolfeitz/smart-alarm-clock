# Work, decisions and evidence

Owner: Rob. Updated: 2026-09-23. Working clock/weather example installed.
Current evidence: local alarm UI/scheduler/persistence tested, quiet sound accepted,
Wi-Fi scan and saved reconnection, live HTTPS weather and NTP/RTC sync verified.
Manual ZIP27358 retained. Owner confirmed power-return startup; battery-backed retention and full-power-loss
alarm recovery remain unqualified. HA, external playback and spoken briefings are future work.
See final dated entries below and CLOCK-ACCEPTANCE.md; earlier sections are history.
Approval: user replied “Approved” to docs/bootstrap/PROPOSAL.md in this task.

## Bootstrap checklist

- [x] Discover original directory and runtime; preserve unrelated installer.
- [x] Owner approves Standard, moderate-cost bounded setup.
- [x] Install guidance, operating index, requirements, test plan and source copy.
- [x] Initialize local Git on main, without remote/publication.
- [x] Run documentation verifier and confirm preserved installer hash.
- [x] Fresh Codex session discovers guidance and executes canonical verification.
- [x] Record proof dimensions, review scope and close bootstrap.

After this kernel is installed, execute the remaining checklist from this file.
The long AAE source remains reference-only outside repository discovery.

## Runtime evidence

| Dimension | Status / evidence |
|---|---|
| Projection exists | AGENTS.md installed from approved draft |
| Loading documented | Official Codex guide checked during discovery; link in TESTING |
| Fresh session discovery | Passed in fresh ephemeral CLI session; see bootstrap/PROOF.md; internal loader trace not captured |
| Fresh session verification | Canonical command exit 0 in fresh session; see bootstrap/PROOF.md |
| Firmware build / device tests | ESP-IDF v6.1 upstream ESP32-C5 hello_world build passed; board peripheral tests still pending |

Discovery evidence and input SHA-256 values are in the approved proposal. Current
shell sandbox previously failed to launch with `mountinfo path is not absolute`;
reviewed escalation worked. Do not disable global sandbox controls to fix this task.

## Historical initial bounded task: hardware readiness

2026-09-23: Rob supplied the precise purchased-board description from the receipt;
recorded verbatim in PROJECT. It identifies the ESP32-C5 rounded-corner 3.5-inch IPS
320×480, 262K-color board with 2.4/5GHz Wi-Fi and Bluetooth 5 LE. SKU/PCB revision,
peripheral controllers, memory configuration and physical device identity remain
unverified. This clarification updates the target evidence; it does not establish
SDK/driver compatibility or authorize flashing or installation.

Goal: establish a reproducible, board-specific basis for diagnostics before writing
clock features. Read current vendor documentation/example sources; identify exact
board/revision, attached device/interface, speaker/power/RTC requirements, available
SDK installations, compatible pinned toolchain and recovery procedure. Avoid dumping
credentials or full process environments. Ask only for physical facts that cannot
be safely discovered. No flashing, erase or host install is authorized by bootstrap.

Completion: record dated vendor URLs/revisions, actual local tool availability,
board evidence, dependency compatibility and a proposed diagnostics/build/flash
procedure with observable acceptance. Missing physical facts remain explicit.
Then implement diagnostics within the next authorized scope and demonstrate HW.

## Deferred implementation order

Preserve the brief's section 25 dependency order:

1. Bootstrap (complete).
2. Hardware diagnostics infrastructure.
3. Display, touch and LVGL.
4. RTC/time.
5. Wi-Fi/NTP.
6. Persistent settings.
7. Alarm engine.
8. Local audio alarm.
9. Basic home screen.
10. Alarm UI.
11. Night/brightness behavior.
12. Weather.
13. HA light toggle.
14. Spotify abstraction.
15. Spotify control.
16. Spotify alarm.
17. Polish.
18. Reliability/failure validation.
19. Complete delivery documentation.

Section 6's broad hardware baseline spans steps 2–6 plus early audio/SD diagnostics:
prove those peripheral paths before product clock/alarm integration. Reliability
tests and documentation accompany every phase; steps 18–19 are final reconciliation,
not permission to postpone testing. No major integration starts before its required
foundation is demonstrated. Each future task records scope, acceptance, interface
choices, evidence, limitations and next action here; split out a design only when
complexity warrants it.

## Decisions and maintenance

- 2026-09-23: Standard justified by local/remote reliability boundaries; no
  High-Assurance posture or medical capability implied.
- Native guidance + Markdown + small verifier; no hooks, plugins, permanent agents,
  CI provider or infrastructure. No recurring defect warrants extra machinery.
- Keep current runtime/model settings. Use focused source reads and deterministic
  checks for mechanical claims; independent review only when consequence warrants.
- Rob owns scope/authority decisions. On recurring failures, identify the earliest
  reliable prevention point and improve an existing check before adding machinery.
  Durable workflow changes require approval unless already delegated. Remove stale
  controls when their benefit disappears. Revalidate index on file moves or runtime,
  architecture, toolchain or bootstrap changes. Resume here after context loss.
- Source references and planned behavior cannot grant themselves action authority.
- Rollback: remove bootstrap guidance/verifier/index entries if no longer needed,
  preserving useful requirements/evidence and any established Git history.

## Verification receipts

2026-09-23: bootstrap complete. [Proof receipt](bootstrap/PROOF.md) records fresh
CLI discovery, tool commands, exact verification output and limitations. Local
verifier and three temporary-copy negative checks passed. AGENTS.md matches the
approved draft; source copy and original installer hashes match. Git initialized
on main; no remote configured. Only approved bootstrap files are included in the
local baseline; the unrelated installer remains unchanged and untracked.

Documentation verification is complete; no firmware build, flashing, peripheral,
service integration or physical wake-up claim is made. The observed CLI read-only
probe succeeded despite the separate desktop shell sandbox launch issue seen
during discovery. No global runtime or host configuration was changed.

2026-09-23: recorded receipt description in PROJECT and updated hardware-readiness
status. Documentation verifier and whitespace check passed; no hardware operations.

2026-09-23: after user reboot, running kernel is 7.2.6-arch2-1; cdc_acm
is loaded and Espressif USB 303a:1001 enumerates as /dev/ttyACM0. Serial open
as rob failed with EACCES: node is root:uucp mode 0660, and rob is not in uucp.
One-time sudo passive-read attempt could not run because authentication is required.
No serial bytes received, reset sent, firmware written or permissions changed.
Next connection step: owner grants temporary per-device access, then retry passive
serial capture. User reports factory display and touch UI respond; WLAN factory
page is documented as scan/display by Waveshare, not verified provisioning.

2026-09-23: owner granted rob per-device serial access. Opening /dev/ttyACM0
succeeded; a ten-second passive read received 616 bytes of factory diagnostic
output. Example: `I (4640385) qmi8658: Now_time is  2050.1.1  6 1:18:18`.
Successive records advanced by one second. This proves readable firmware logs,
not an interactive command interface or correct RTC time; the reported date is
2050-01-01. The reader briefly waited in tty_wait_until_sent while closing, then
exited successfully on its own. No application commands, reset or flash operation
was sent. Next: identify firmware/version and supported provisioning or diagnostic
interfaces before attempting Wi-Fi configuration.

2026-09-23: continued hardware readiness after owner requested active progress.
Esptool 5.4.0, obtained through a temporary uv environment (no system package or
SDK installation), identifies ESP32-C5 silicon v1.2, 48MHz crystal and 32MB flash.
An exact-board vendor source/recovery gap was confirmed on the public Resources,
ESP-IDF and Firmware-Flashing pages; the expected exact GitHub repository returns
404. A private full-flash read is underway before any replacement firmware.
Draft vendor request: hardware/VENDOR-REQUEST.md (not sent).

2026-09-23: full 32MB factory read and esptool verify-flash digest comparison
completed successfully. Application checksum/hash and partition MD5 validate.
Factory metadata establishes ESP-IDF v5.5.4 (upstream commit resolved), project
blink/version 1, compiled Sep 11 2026; image header is 16MB despite 32MB chip.
ROM security query reports Secure Boot and flash encryption disabled. Device reset
back to existing firmware; no flash writes/erases/eFuse changes. See HARDWARE.md
and SETUP.md for evidence and procedures. The raw image is private and Git-ignored.

Readiness remains incomplete: missing vendor BSP/example, schematic/pin map, PCB
revision and LVGL/component versions prevent a responsible board-specific build.
The source availability assumption in the broad plan needs revision. Vendor request
is drafted, not sent; owner was asked for any seller-provided download/QR URL.
Continue immediately if matching resources arrive; do not manufacture pin mappings.

Post-diagnostic runtime check: bounded pyserial capture received 6,313 bytes of
factory startup output, including app_main() and AXP2101 `Init PMU SUCCESS!`.
Documentation verification and whitespace checks passed. Hardware GUI/peripheral
acceptance remains incomplete; no replacement firmware has been installed.

2026-09-23: owner supplied storefront URL with SKU 35419 plus official overview,
flashing and resource links. Overview SKU table resolves the purchased variant to
ESP32-C5-Touch-LCD-3.5-C. The flashing page still says test firmware is in preparation.
Storefront and direct resource-page fetches returned 403 during recheck. Following
the Resources link from the overview succeeded through the web reader and again
showed examples/hardware resources as still being prepared. No board changes.

## Authorized local toolchain setup

2026-09-23: user selected ESP-IDF 6.1 and asked Codex to handle setup.
Scope: install official EIM and SDK locally, then compile the upstream hello_world
example for esp32c5. Acceptance: pinned SDK/compiler identity and a successful
build with recorded repeatable commands. No board-specific driver integration,
firmware replacement, flash erase, or host package changes are included.
Factory v5.5.4 remains recovery/reference evidence; new development starts on v6.1,
subject to later exact-board BSP compatibility verification.

EIM v0.19.0 official Linux x64 archive verified against GitHub release asset SHA-256
`f020f19afa9153417394fd6579d13d684d85df5ee5e1d00ad5fbf7de909a631d`.
Its prerequisite check found only dfu-util missing. Built upstream dfu-util 0.11
from https://dfu-util.sourceforge.net/releases/dfu-util-0.11.tar.gz using existing
libusb 1.0.30 and installed it under local-config/toolchains/prerequisites.
No system package installation was needed. EIM installation completed;
logs and generated configuration are kept under ignored local-config/toolchains.
EIM resolves compiler/download paths separately from its SDK base path: compiler
tools and archives use /home/rob/.espressif/{tools,dist}, while SDK and manager
registry use the project. These are user installations, not system package changes.
The downloaded dfu-util source archive SHA-256 is
`b4b53ba21a82ef7e3d4c47df2952adf5fa494f499b6b0b57c58c5d04ae8ff19e`
(recorded fingerprint, not a separate publisher checksum verification).


Toolchain acceptance passed (2026-09-23):

- EIM v0.19.0 completed SDK/tool/Python installation, exit 0.
- SDK tag v6.1 resolves to `fff9895c82d744c7237be8847347bdd1b07c6643`;
  SDK `git status --short` was empty.
- `idf.py --version`: ESP-IDF v6.1; compiler: riscv32-esp-elf-gcc
  (crosstool-NG esp-15.2.0_20251204) 15.2.0; Python 3.14.7.
- Activated `/home/rob/.espressif/tools/activate_idf_v6.1.sh`, copied the SDK's
  hello_world example under local-config/toolchain-smoke, and ran
  `IDF_PY_BUILD_JOBS=4 idf.py -C local-config/toolchain-smoke/hello_world set-target esp32c5 build`.
  Exit 0, project build complete. Log: local-config/toolchain-smoke/build.log.
- `python -m esptool image-info` confirms ESP32-C5, ESP-IDF v6.1, valid
  checksum and validation hash. App binary: 142704 bytes; SHA-256
  `3e6aea7c1cbb8d2c4ab5684f476a927fe1179ac6b68613b6c42e4efc29c3304c`.
  App version reflects the containing project Git state (`6a3b108-dirty`);
  generic example flash header is 2MB, not the board's measured 32MB.
- EIM could not copy its OpenOCD udev rule into /etc (permission denied).
  Compilation passed without it; no system packages or rules changed.
- Tools occupy approximately 7.1GB in ~/.espressif; project SDK/tools 778MB.
  No board reset, flash, erase or serial command was needed for this setup.

Installer invocation used project-local EIM with `--do-not-track true`, local
`--log-file` and `--esp-idf-json-path`, then `install --path` pointing at
local-config/toolchains/esp-idf, `--idf-versions v6.1 --target esp32c5
--non-interactive true --skip-components-download true`, and local
`--config-file-save-path`. The local prerequisites/bin directory was prepended
to PATH; UV_CACHE_DIR was /tmp/esp-link-uv-cache. EIM generated the full resolved
configuration at local-config/toolchains/eim-install-config.toml.

Next: obtain exact-board source/schematic and reconcile peripheral dependencies
against v6.1 before preparing board diagnostics. Toolchain setup is complete;
board support and hardware acceptance remain outstanding.

Documentation verifier and git diff --check passed after the setup documentation
update. Tools/build artifacts remain Git-ignored. The unrelated install-arch.sh
SHA-256 still matches the preserved baseline.

## Authorized flash and recovery test

2026-09-23: owner said "go go go" to flash a minimal IDF 6.1 diagnostic,
verify boot/serial output, then restore the preserved factory firmware.
Acceptance: image writes verify, repeated diagnostic heartbeats report v6.1 and
ESP32-C5, full factory restore verifies before reset, factory startup logs return;
owner confirms the physical GUI. No standalone full erase or eFuse operations.
Diagnostic interface: USB Serial/JTAG text only, no GPIO/peripheral/NVS/network
initialization. Use DIO/80MHz/16MB matching the factory header, auto-detect crystal,
factory app at 0x20000; temporary partition table contains only that application.
Backup hash rechecked and ROM identity/security match prior evidence.

Diagnostic build and execution passed: app 1.0, 166352 bytes, SHA-256
`9565b9ed1fa880a57f9bc5319dd1549ce86458038c72b00b6c0c86ec8a98fe91`.
Bootloader at 0x2000, partition table at 0x8000 and app at 0x20000 were written
with esptool 5.4.0, DIO/80MHz/16MB; all write hashes verified, exit 0.
Twelve-second serial capture received 1144 bytes, including uninterrupted ticks
31 through 42: `ESP_LINK_DIAGNOSTIC_OK idf=v6.1 target=esp32c5 revision=102
flash_bytes=16777216`. The 16MB value reflects configured accessible flash, not
a new physical-size measurement. Serial startup began with a partial line; the
following complete heartbeats were valid. This proves flashing and execution,
not screen/touch or peripheral behavior. Logs are in local-config/flash-test.

Full factory restoration uses `write-flash --flash-mode keep --flash-freq keep
--flash-size keep 0 <factory-flash.bin>` with `--after no-reset` before digest
verification, preserving factory headers. Restoration completed: 33554432 bytes written,
full-image hash verified while still in the loader, exit 0. A second redundant
full comparison was unnecessary after the successful write-time verification.

Factory restart: the first direct reset/capture produced zero bytes and failed its
startup assertion. Using esptool `--after hard-reset run` and opening pyserial with
DTR/RTS false before open resolved the capture. Received 7014 bytes, including
`Calling app_main()` and `Init PMU SUCCESS!`, exit 0. Evidence:
local-config/flash-test/restore-write.log and factory-serial-retry.log.
Physical screen/touch confirmation was requested from the owner and is pending.
No standalone erase or eFuse changes were performed. Factory backup unchanged.

Post-test backup SHA-256 still matches the preserved original. Documentation
verification and git diff --check passed. Diagnostic source/configuration and
receipts are retained in local Git; private images and logs stay ignored.

## Expanded source discovery — 2026-09-23

Owner requested autonomous follow-through on supplied vendor links. Rechecked
English and Chinese Waveshare resource pages; both explicitly mark exact-board
hardware resources and examples as still being prepared. Storefront HTML now
retrieved successfully; its development-resource link leads to the same docs.
Inspected embedded C5-3.5-Interfaces image: external connector/control locations,
not a schematic or complete internal peripheral GPIO assignment.

GitHub API repository search for ESP32-C5 under waveshareteam returns seven
repositories (Zero, Pico, MINI-KIT, LCD-1.47, LCD-2.73, Touch-LCD-1.69 and
Touch-LCD-2.8); none for 3.5. All-public repository-name/description search for
ESP32-C5 plus 3.5 returned zero. The 2.8 repository recursive tree contained no
C5 3.5 path. These bounded searches do not prove no source exists anywhere.

Owner also supplied Espressif DevKitC-1 hardware reference, esp-idf repository,
and C5 technical reference manual. DevKitC-1 is a different carrier PCB; its
header assignments cannot establish Waveshare display/audio wiring. ESP-IDF
is the installed SDK. The chip reference manual documents internal peripheral
operation; it cannot by itself establish the custom PCB's connected nets.
The web reader rejected the manual PDF because it exceeds its size limit; no
claim to have read the entire manual is made.

Checked sources:
- https://docs.waveshare.com/ESP32-C5-Touch-LCD-3.5/Resources-And-Documents
- https://docs.waveshare.net/ESP32-C5-Touch-LCD-3.5/Resources-And-Documents/
- https://www.waveshare.com/esp32-c5-touch-lcd-3.5.htm?sku=35419
- https://github.com/waveshareteam
- https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c5/esp32-c5-devkitc-1/user_guide.html
- https://github.com/espressif/esp-idf
- https://documentation.espressif.com/esp32-c5_technical_reference_manual_en.pdf

No firmware changes during this research. Exact-board GPIO routing remains
unresolved. Vendor support request remains drafted and unsent; contacting others
requires explicit user instruction. Board-independent clock/alarm logic can be
developed without this hardware information; screen/audio bring-up requires
verified wiring or a separately scoped hardware/firmware reconstruction effort.

## Factory reconstruction and bounded bus test

Owner asked Codex to figure out missing board details and run tests. Scope: offline
factory application analysis, recover BSP constants against SDK 5.5.4 layouts,
then flash an IDF 6.1 USB/I2C diagnostic on evidenced SDA27/SCL26 and check ACKs.
No speculative GPIO sweep, PMU configuration writes, peripheral register writes,
erase-all or eFuse writes. Acceptance: board heartbeat plus repeated address ACKs;
address presence does not alone establish device identity. Leave the diagnostic
installed if it passes, retaining verified factory recovery.

Factory BSP source string identifies waveshare__esp32_c5_touch_lcd_3_5. Public
registry lookup for this exact component is 404. Offline RISC-V disassembly
places bsp_i2c_init at 0x4201bade: stores 27 at config offset4 and 26 at offset8,
then calls i2c_new_master_bus; checked against IDF v5.5.4 i2c_master.h layout.
LCD bus setup at 0x4201ba5c stores MOSI7/MISO2/SCLK6. Display constructor at
0x4201c07a stores CS8/DC5 and 60MHz SPI clock. These are reconstructed factory
configuration values; hardware validation remains separate.


Owner photo IMG_2840.jpeg confirms CH32V006 expander labeling and E8–E15
header labels. Factory driver symbol retains the older ch32v003 name; exact
protocol/part distinction is recorded in hardware/RECOVERED-MAP.md.

USB re-enumeration removed the prior ACL. sudo -n required a password; owner
restored temporary rob access and replied done. No account/group or global
permission change was made by Codex.

Bus-test acceptance passed: IDF v6.1 board-probe build and flash exit0; image
checksum/hash and write hashes valid. Binary SHA-256:
`d58cc2c11caeb39edc874f3a998dd0a8d18470e956afc873ac98f2ee93a9a472`.
A 16-second passive serial capture shows rounds0–3, each with eight responders:
0x18,0x24,0x34,0x38,0x51,0x6b,0x6e,0x70; every round reports errors=0.
Source: firmware/board-probe. Logs/build: local-config/board-probe. Individual
chip IDs and functions are not inferred from ACKs alone. The successful probe
is LEFT INSTALLED, as communicated; factory demo is not currently running.

Next bounded hardware work: read-only device identification and RTC/touch reads;
then validate display initialization using recovered SPI/expander configuration.
PMU/expander configuration writes need understood register/bit behavior. The
recovered map replaces the earlier assumption that absence of public schematics
prevents all hardware progress. LCD/audio pins remain firmware-derived candidates
until tested. No destructive full erase, eFuse changes or network provisioning.

Documentation verifier and whitespace checks passed after the recovered-map
and hardware-test updates. Local Git retains probe source and evidence notes;
private factory binaries/disassembly remain ignored.

## Authorized display/touch bring-up

Owner requested working display/touch. Acceptance: IDF6.1 diagnostic initializes
ST7796 on recovered SPI pins, draws a visibly new test screen with heartbeat,
logs FT6336 coordinates and draws corresponding marks; owner checks alignment.
Single UI loop owns SPI and I2C reads. No PMU writes, audio or network changes.
Use software LCD reset and existing powered rails first; cold-power initialization
remains a separate verification. Backlight writes use recovered expander command5.
Touch factory routine reads count from register2 and six bytes per point at3,
12-bit X/Y high nibble + low byte; 0x42022bc4–0x42022d06.

Display/touch build: vendored official esp_lcd_st7796 1.4.0, upstream commit
fd0098aaa277c5b35cc54779ee7bfbda72e8db1e. Five source/header/license files
verified against published CHECKSUMS.json. Local CMake uses IDF6.1 GPIO dependency
and version defines; C5 absent in upstream manifest, hence explicitly a local
SPI port. Initial compiler indentation warning corrected; build then passed.

First on-device run: DISPLAY_READY under v6.1; repeated DISPLAY_ALIVE with zero
I2C errors. Owner confirmed visible test and drawing but reported reflection.
Raw touch logs included Y>320, proving portrait touch axes despite landscape
display. Corrected display mirrorY and transformed x=raw_y, y=319-raw_x.
App-only update at0x20000 verified successfully. Physical alignment check pending.
Evidence: local-config/display-touch/{build.log,write.log,serial.log,
write-orientation.log,orientation-serial.log}.


Final touch correction: owner reported visible CLEAR at upper-right activated by
lower-left touch. Together with raw portrait coordinates, this establishes the
two-axis reversal in the first transform. Final mapping x=479-raw_y, y=raw_x;
LCD swapXY/mirrorX/mirrorY remain true. App-only reflash exit0, hash verified.
Owner confirmed "Perfect!" after the aligned drawing/CLEAR check. Firmware is
LEFT INSTALLED. Serial monitor records DISPLAY_READY/v6.1 and DISPLAY_ALIVE;
no PMU register changes or factory restoration occurred.

Next: qualify cold-power/reset behavior and power initialization, then integrate
LVGL on this verified display/touch transport. Other hardware gates remain open.

Final installed app SHA-256:
`f28413ca4fd60b5fa9929ee27f74e7b4008090bba6f4f4d850223668aeb8e252`.
Documentation verifier and whitespace checks passed. Vendor source hashes checked;
private binary/log artifacts remain under ignored local-config.

## Cold-start failure investigation

Owner confirms battery is disconnected: USB removal fully removes power. On
reconnect the display is blank; temporary serial ACL restored. Capture
local-config/display-touch/cold-start-serial.log shows IDF6.1 boots and
DISPLAY_READY, but every touch read fails (501/501). Warm-start acceptance
does not establish cold-start initialization.

Bounded acceptance: identify missing peripheral initialization, implement only
verified board power/reset settings, then confirm drawing/CLEAR after actual USB
power removal. First diagnostic adds read-only PMU and I2C presence reporting;
no regulator writes until register semantics and factory intent are established.

Read-only probe: PMU ID03=4a, enable90=ff/91=01, ALDO2 voltage93=1c
(3.3V); touch38 absent while expander24 responds. Registers checked against
Waveshare-hosted X-Powers AXP2101 SWcharge V1.0 pp42–43. Do not change
voltage rails: enabled ALDO2 rules out the initial missing-enable hypothesis.
Next controlled test reproduces this exact board's factory expander startup,
not inferred wiring: command2 directionff, command3 output00; directionbf
(all factory outputs except input bit6); output03, delay500ms, output00,
delay500ms, output23, settle200ms. Factory direction flag polarity verified
at 0x42022606–42022624 (uninverted means output=1); constructor flags zero
and initial directionff/output00. Individual net labels remain unproven;
factory mask23 usage and absent touch after complete power loss motivate this
reset-path test. No PMU voltage or charging changes.

Reset startup build and app-only flash passed; esptool write hash verified.
Installed image SHA-256:
`5c1f70bdf1350a2ece2b3b097e01f6f14df7b536eb8fcb0d08276f1b6e96afd6`.
Serial: BOARD_RESET_DONE touch_probe=ESP_OK, DISPLAY_READY, 251 reads with
zero errors. Logs build-cold-fix.log/write-cold-fix.log/cold-fix-serial.log
in local-config/display-touch. No PMU writes. Asked owner to verify visible
screen and a fresh ten-second USB power removal/reconnect; acceptance pending.
Documentation verifier and git diff --check passed before this evidence update.

## Clock screen foundation

Owner confirms screen/drawing/CLEAR work and requests the actual clock.
Acceptance: LVGL time/date screen, advancing seconds, touch brightness toggle,
RTC read/write/readback, host-time USB provisioning, reboot restore from RTC,
and explicit unsynchronized state on invalid/lost RTC. No alarm/weather/music
placeholders implying working features. Initial timezone America/New_York from
owner environment, POSIX DST rules; RTC stores UTC. No network dependency.

Interfaces: board adapter owns SPI/I2C and fixed orientation/reset startup;
clock service (no LVGL) owns RTC validity and system time; UI owns all LVGL
operations in one task. USB accepts bounded TIME <Unix seconds> lines, no
credentials. RTC RAM byte03=a7 marks this application's UTC convention; clear
marker before writes and set after readback. OS/STOP/12h/calendar validation
prevents trusting arbitrary factory dates. Battery is disconnected, so full
power loss may require time provisioning again. NXP PCF85063A Rev7.3 register
map consulted; dependency LVGL pinned9.4.0, compatibility to be established by
IDF6.1 build and physical rendering. Preserve working diagnostic separately.

LVGL9.4.0 resolved with component hash
17e68bfd21f0edf4c3ee838e2273da840bf3930e5dbc3bfa6c1190c3aed41f9f;
manifest and dependencies.lock tracked. IDF6.1 clock build passed. RTC codec
host checks passed (invalid BCD/OS/date, leap day, encoding round trip, DST
spring-forward). Host America/New_York and NTPSynchronized=yes verified.

App-only clock flash verified. Binary SHA-256:
`cff2c335fb50d1007811e5db8e9d6e9e780e8a451bfc6eb31ab6719cfc7b2017`.
First boot correctly rejected unprovisioned RTC; USB TIME command succeeded.
CLOCK_ALIVE reports system and RTC epoch1790201398, rtc_status=ESP_OK,
heap224652. No network configuration or credentials. Owner visual clock and
Dim/Brighten check requested; MCU-reset RTC restore check running.
Previous owner reply confirms diagnostic display/drawing/CLEAR functional after
startup patch; does not explicitly distinguish a second full power cycle.

Clock acceptance: owner replied everything works as advertised (time/date,
seconds, Dim/Brighten). MCU reset restored RTC time without a TIME command:
CLOCK_INIT rtc=ESP_OK source=RTC / offline. Three successive ten-second reports
advanced system/RTC time within one second; free heap224652 unchanged.
CLOCK_BRIGHTNESS dim=1 then0 captured. Evidence reset.log/reset-serial.log.
Working clock is LEFT INSTALLED. README/SETUP updated to this current state.

Next bounded milestone: qualify local audio codec/amplifier and audible output
before implementing alarm delivery; battery-backed retention, NVS settings,
Wi-Fi/NTP and full power-cycle clock qualification remain open. No alarm is armed
or implied by this clock milestone. Documentation/whitespace verification passed.

## Continued delivery: audio then local alarm core

Owner explicitly asks to keep working across milestones. Continue authorized
implementation without treating each successful hardware check as a stopping
point. Next acceptance: short low-volume tone, clock remains responsive,
codec/I2S startup and shutdown checked, owner confirms audible output.
Interfaces: audio task owns I2S output and bounded tone requests; codec uses
existing I2C bus only at initialization (later register changes stay serialized).
No remote services, PMU voltage changes or host packages. Add pinned Espressif
codec component; verify its config layout against factory initialization.

Audio test firmware built and app-only flashed with verified hash. Image SHA256
4dff04f22ff77c156548535da0716119e6ef0f1526afcd52f01e2181d139417c.
Runtime: ES8311 slave mode, 22050Hz/16-bit stereo, codec open OK, AUDIO_READY
volume25, CLOCK_READY and RTC restore OK. Test sound button requests four
quiet pulses via one-item queue to independent audio task. DMA buffers auto-clear
to silence after transmission. Evidence audio-build.log/audio-write.log/
audio-serial.log under local-config/clock. Owner audible check pending; successful
codec initialization is not proof of speaker output.

Owner reports no audible sound and suspects no speaker. Two test requests each
completed ESP_OK with177152 transmitted bytes; clock continued with RTC agreeing.
This is transport evidence ONLY. Physical SPK connection check requested.
Factory audio playback/resume calls at0x4201b948/0x4201b9ca set expander
mask0x20 high; our startup output0x23 already does so. Default volume25 maps
to -37.5dB before hardware-gain adjustment; quietness remains another possible
reason for silence. Do not claim working audio or assume speaker is installed.

## Active goal: offline alarm clock

Previous work made progress: clock deployed/verified; audio diagnostic deployed,
with transmission proven and audible output unresolved. Goal remains active.
Continue independent scheduler/settings work while physical speaker check is pending.

Alarm-core acceptance/interfaces before implementation: up to8 alarms, local
hour/minute plus weekday mask or explicit once-only date; one independent engine
owns scheduling, active/snoozed/dismissed state. Repeated fall-back hour rings
once per local calendar date. A nonexistent spring-forward time is skipped.
A trigger may be caught up within120 seconds; larger forward jumps do not ring
old alarms. Backward corrections do not re-ring a consumed date. Snooze is five
minutes of monotonic time (wall-clock correction cannot shorten it). Maximum
ring duration10 minutes; dismiss consumes that occurrence. Simultaneous alarms
remain independently represented. Invalid time cannot trigger a scheduled alarm.
Persist occurrence consumption before audible start in the integration layer;
failed persistence must surface rather than falsely report durable settings.

Pure C engine/date validation gets host tests for normal/once/weekday triggers,
DST transitions, time jumps, duplicates, simultaneous alarms and snooze/dismiss.
UI, persistence adapter, and independent firmware task follow these interfaces.
New project-owned NVS must occupy verified unused flash, preserving all factory
partitions/backup; do not initialize factory NVS or erase on generic init errors.

Owner cannot inspect inside sealed enclosure. No dismantling requested.
Next audio test raises codec volume25 to55 (-37.5dB to-22.5dB in default
curve), keeping PCM amplitude5000/32768 and two-second duration. Read-only
codec ID/clock/mute/volume dump added. Expander playback helper0x4201b6fc
clears bit5 before transitions; playback/resume then sets it high, consistent
with our high bit5 during playback. Hardware speaker remains unconfirmed.

Alarm-engine host tests passed for invalid time, daily/weekday/once schedules,
120-second grace boundary, spring gap, fall repeated hour, duplicate suppression,
monotonic snooze despite wall-time jumps, dismiss, ten-minute timeout, simultaneous
alarms and invalid dates. Engine not yet wired to firmware task/UI.

Versioned settings codec tests passed for disabled defaults, once/consumed dates,
every-byte single-bit corruption and every truncated length. Proposed dedicated
clockcfg NVS region0xa00000/0x6000 is all0xff in preserved factory backup and
beyond every factory partition. Still need live confirmation before table update.
NVS adapter uses explicit partition/namespace, atomic blob commit, no automatic
erase-on-error. Not yet called or installed; current board retains tone-test app.

Adjusted audio runtime: ID fd=83/fe=11/revisionff=01, reset00=80,
clock01=bf, interface09=0c, system12=00, DAC31=00, volume32=99;
all reads success. AUDIO_READY volume55, RTC and clock continue. This confirms
ES8311 identity and configured unmuted DAC, not actual speaker output.
Owner louder-tone observation pending; no need to open sealed case.

IDF6.1 build including alarm engine/settings codec/NVS adapter passed;
bootstrap documentation and whitespace checks passed. Current source adds tested
foundations; alarm task/editor and NVS partition deployment remain next work.
Goal not complete: audible output, editable deployed alarms, persistence and
power-loss acceptance still outstanding.

## Alarm runtime/editor integration

Previous goal turn was progress: tested scheduling/settings modules committed.
This turn connects an independent100ms alarm-owner task, bounded UI command queue,
mutex-protected snapshots, persistent saves and looping/stoppable local tone.
UI commands change a candidate setting and commit it before activation; failed
save retains prior config and exposes error. Occurrence consumption is committed
before sound, with persistence failure visibly reported while local ringing still
runs (storage failure must not silently suppress a wake-up). Runtime restart
recovery will be audited separately before completion. Eight editable alarm slots,
weekday toggles, once date, enable switch and saved brightness are required.
Current source uses dedicated clockcfg partition at0xa00000/0x6000 only after live
read confirms blank; factory partitions/recovery image stay intact.

Alarm editor/runtime deployed. Live esptool read of0xa00000/0x6000 returned
exactly24576 bytes of0xff, matching factory backup. Added clockcfg NVS partition
there; flashed partition table0x8000 and app0x20000 with verified hashes. No
factory NVS/data erased. Current app SHA256:
5f5f0f19f7ea6288d957eca25445bb5eb87c48d691ac3d7ef40276d0034b5655.

Settings load/save ESP_OK. Disabled slot7 06:43 weekday mask62 survived MCU
reset exactly. Scheduled once alarm7 triggered at18:41 on2026-09-23 with
consumption persistence ESP_OK. First harness queried a queued snooze too early
and timed out, so not counted as snooze proof. Corrected test observed owner-task
ALARM_PHASE ringing64/snoozed0 -> ringing0/snoozed64 -> ringing0/snoozed0.
Test slots6/7 restored disabled. Runtime logs alarm-startup.log/alarm-runtime.log/
alarm-transitions.log under local-config/clock. Clock continued, heap204096
before alarm overlay. Owner editor and louder-tone feedback requested.

Host next-alarm tests also verify spring-gap skipping and both fall-hour candidates
with consumed-date suppression. Home now shows next occurrence rather than count.
All relevant host tests and IDF build pass; documentation/whitespace pass.
Next: persist/recover ringing and snooze across restart (currently runtime-only),
verify brightness persistence and physical editor, resolve speaker silence, then
power-loss checks. Do not call goal complete on current partial acceptance.

## Ring/snooze restart recovery

Previous goal turn made verified progress: deployed editor, NVS save/reset proof,
real scheduled trigger and owner-task snooze/dismiss transitions. Recovery acceptance:
save active phase and UTC deadline atomically with consumed occurrence; restore
unexpired ringing or snooze using valid RTC. If snooze expired <=120 seconds ago,
ring on recovery; older stale events do not ring. Do not restore active events
until time is valid. Runtime snooze remains monotonic; clock corrections rebase the
persisted UTC deadline without changing the remaining duration. Dismiss clears the
saved active event; migration from settings schema1 preserves existing alarms.
Host tests cover schema migration and recovery timing; hardware resets will test
ringing, snooze, dismiss, and brightness. Actual USB power loss remains a separate
physical check, especially with the disconnected battery.

Repeated reset tests proved ringing restoration and saved brightness, but snooze
checks timed out twice with delayed USB output. Treat this as unresolved runtime
behavior, not merely a harness issue. SDK source confirms default stdout uses
direct FIFO polling while our receive path installs the interrupt driver.
Correction acceptance: one driver owns USB, runtime diagnostic producers enqueue
without waiting; a separate low-priority task drains bounded records with finite
write timeout. Disconnected/slow USB may lose diagnostics, never block scheduler
or UI. Repeat ringing/snooze/dismiss reset proof after installation.

Driver-backed queued diagnostics alone did not resolve the repeat timeout.
Audio generation was also doing two seconds of per-sample software floating-point
sine at priority4, above the UI task. Replace runtime synthesis with an integer
lookup waveform and explicitly yield each DMA block. Verify command responsiveness
while looping and repeat the same recovery test; do not claim the cause resolved
before that evidence.

Recovery/audio-yield build and app-only flash passed; image SHA256 a7c00983f9cdc63960c6da9b58a9e0b8b78a0ea0479b0044d0f7871edec6f21c.
Fourth hardware test passed ringing reset, brightness reset, snooze reset, and
dismissal reset. SNOOZE now produces accepted/checkpoint/phase transition without
the earlier timeout. Evidence: local-config/clock/recovery-fourth-summary.log and
recovery-runtime-fourth.log. Earlier attempts failed and remain preserved.
All four host suites (alarm engine, settings codec, recovery, RTC) pass with
-Wall -Wextra -Werror. Test alarm2 restored disabled07:00daily, brightness160;
STATE confirms every slot disabled and ringing/snoozed zero. Schema2 migration
preserved prior disabled slot7 06:43 weekdays62.

Updated audio uses a256-sample lookup table and one-tick yield per256-frame
block; preserves660Hz,22050Hz stereo, four pulses and PCM peak5000. A bounded
32-record diagnostic queue drops when full; USB drain task uses20ms timeout.
The runtime alarm/UI producers never wait for a serial reader. This is scheduler
and recovery proof, not audible speaker or physical UI proof. Owner has been
asked to tap Test sound with the sealed case intact; feedback pending.
Remaining: actual sound, physical editor and ringing controls, full power loss
with disconnected battery, and end-to-end five-minute snooze expiry. Goal active.

## Save acknowledgment and complete snooze interval

Previous turn was progress: deployed and proved reset recovery plus audio/UI
scheduling correction. A live five-minute snooze test now runs on existing image,
with test slot1 automatically returned disabled. No flash while it is running.
Editor correctness acceptance: close only after acknowledgment of its own save,
not an unrelated checkpoint/revision increment. Failed save retains the editor
and reports the exact operation failure. Ignore repeated Save taps while pending.
Use a request ticket carried with the owner command and a separately published
completion ticket/status; untracked serial and brightness commands cannot overwrite
the editor completion. This is independent of the running hardware test.

Offline time-entry acceptance before coding: visible Set time control with local
calendar date and24-hour hour/minute. Validate calendar and reject DST gaps;
ambiguous fall-back hour selects the earlier occurrence. Save writes/read-verifies
RTC through the existing clock service; failures keep editor open. Do not claim
physical UI qualification from the build. Host-test calendar/DST conversion.

Live five-minute test passed: snooze returned to ringing after300.01 seconds,
then DISMISS cleared it. Slot1 restored disabled07:00daily. Evidence:
local-config/clock/snooze-duration-summary.log and snooze-duration.log. RTC/system
heartbeats continued throughout with stable free heap193916. This proves timed
state transition on hardware, not sound or physical button operation.

Per-save acknowledgment and local time editor build passed and are installed;
app-only write hash verified, SHA256 095ee2246533a79c88550e54d2a9eb5c3a3bc7cccfcd013170c14c0574559c1f.
New image boots CLOCK_READY, RTC/offline time valid, brightness160 loaded,
ringing/snoozed zero, storageESP_OK, RTC heartbeat within one second. Evidence
local-time-runtime.log and local-time-flash.log. All five consolidated host suites
pass via python scripts/test-clock-host.py; docs and whitespace checks pass.
Asked owner to inspect Set time and save/reopen a disabled alarm. Audible speaker
response remains pending from earlier question. No battery/full-power-loss proof.
Goal remains active; physical acceptance is still required.

## Completion audit after ab10c17

Previous turn was progress: completed the real five-minute snooze proof and
installed offline time entry/exact save acknowledgments. Reviewed current source
and receipts against every active-goal requirement; recorded the distinction
between host/serial evidence and physical evidence in CLOCK-ACCEPTANCE.md.
Audible output, new screen/buttons and complete power loss remain unproven.
Requested10-second USB unplug/replug with battery left disconnected; no firmware
reset or automatic TIME provisioning will substitute for that physical test.
No test process was left running by the previous turn. Goal is not complete.

Pending-acceptance recheck: serial device accessible and CLOCK_READY/RTC valid,
ringing0/snoozed0, brightness160, storageESP_OK. Log acceptance-pending-runtime.log
includes a USB_UART_HPSYS reset (rst0x15), not power-on proof. Opening serial with
DTR/RTS preset false did not establish a reset-free observation; avoid further
serial reconnections while waiting for the owner's physical check. No deliberate
esptool reset or TIME command was sent. This does not pass full-power-loss or
audible output gates. Remaining progress now depends on the requested physical
observations; no test process remains live.

## Speaker confirmed, output level adjustment

Owner now hears Test sound but reports it barely audible. This establishes actual
speaker output; loudness sufficient for an alarm remains unproven. Raise codec
volume55 to85 and PCM peak5000 to10000 (below16-bit clipping), retaining the
two-second four-pulse envelope,660Hz tone and task yielding. No automatic test
sound on boot. Build/flash/readback must pass, then request owner loudness and
distortion feedback. Physical screen/power-cycle gates remain unchanged.

Louder-tone IDF build and application flash passed. Runtime reports volume85,
peak10000, DAC volume register32=b7, CLOCK_READY, RTC/system agreement,
storageESP_OK and retained brightness160. No automatic sound command sent.
Image SHA256 2605da61813ff336b16687a361d0a37a2935f3143a409a442312471ed124b0d5. Evidence: louder-tone-build.log,
louder-tone-flash.log, louder-tone-runtime.log under local-config/clock.
Physical loudness/clarity feedback required; prior owner report proves speaker
existence/output but not alarm-appropriate level.

Owner confirms volume85/peak10000 is louder and clear, but still very quiet.
Next bounded step: volume100 (default curve0dB) and PCM peak20000. Pinned codec
source applies approximately+3.61dB voltage compensation; this peak retains
headroom after that compensation (about0.925 full scale). Relative programmed
level increases approximately13.5dB; acoustic loudness remains a physical check.
Keep two-second user-triggered test and existing envelope/yielding. If still too
quiet, investigate amplifier/power/routing rather than exceeding this gain blindly.

Full-level build and app-only flash verified. Boot confirms volume100/peak20000,
DAC register32=c6, retained brightness160, CLOCK_READY and RTC within one second.
No automatic test tone sent. Image SHA256 c1f518f8d57dc25e8b5493f9a49af8b6cff7ca9bba7ae1487a47efc92e3182f1.
Evidence: full-level-build.log/full-level-flash.log/full-level-runtime.log.
Await owner loudness/distortion check before claiming usable alarm volume.

## Amplifier/power diagnosis after audible but inadequate tone

Owner confirms full-normal-volume tone is easy to hear but far below alarm loudness.
Compare recovered factory audio controls (bit5 low before stop, high on playback)
and PMU configuration. Current startup already sets bit5 high and uses the same
expander direction polarity; no missing high-level enable identified yet.
Add read-only diagnostic evidence at boot: expander command4 physical pin sample,
AXP2101 ID03 and rail configuration90..99, codec output/system registers.
No PMU writes, gain increases or factory rollback. Build/flash acceptance: display,
RTC and current settings retained; report exact bytes without equating configured
voltage with measured supply voltage or loudness.

Audio-power diagnostic runtime: expander physical inputs23 (bit5 high),
AXP ID03=4a,90=ff,91=01,92=1c,93=1c,94=19,95=0d,96=07,97=17,98=0e,99=00.
Codec00=80,01=bf,09=0c,0d=01,0e=02,12=00,13=10,14=1a,31=00,32=c6,37=08.
All reads success. ALDO2 configuration agrees with factory3300mV setting;
actual amplifier VCC/net topology cannot be established without schematic or
measurement. No PMU changes made and no absent enable found.
Next bounded acoustic test: Test sound plays660,1320,2200,3300Hz in ascending
order at the SAME amplitude/codec level and envelope, still two seconds total.
Alarm playback remains660Hz until owner comparison. This tests speaker frequency
response; it is a hypothesis, not a claim that the speaker is defective/limited.

Owner proposes checking original demo music; proceeding with reversible acoustic
comparison. Vendor Instructions-For-Use confirms built-in SPIFFS MP3 and Settings
Sound volume control (no SD required for that track). Capture live0..0xa06000
before switch; extract exact current boot/table and entire application partition
for restoration. Factory image SHA256 reverified against original receipt.
Temporary factory write only0x2000..0x8fff and0x20000..0x501fff; clockcfg at
0xa00000..0xa05fff stays untouched. No whole-flash erase/restore. Restore exact
current clock snapshot after owner compares music at increased demo volume.
The ascending-pitch build completed but was NOT installed; factory comparison
takes precedence. Current saved clock is the audio-power diagnostic build.

Factory comparison installed successfully: hashes verified for boot/table slice
and factory app partition. Live backup size10510336 bytes; original factory NVS,
assets and storage byte-match immutable factory backup. Clock recovery slices
and hashes are in private local-config/clock/factory-comparison/snapshot-receipt.json;
restore-clock.py verifies hashes before writing only saved boot/table+app.
Factory runtime reached app_main, Init PMU SUCCESS and ALDO2 enabled3300mV.
Owner asked to raise Settings→Sound gradually then play Music and compare volume.
CURRENT BOARD: temporary factory demo, NOT bedside clock. Clockcfg remains
untouched. Restore the exact saved clock after this physical comparison; do not
leave the factory demo as the final goal delivery. Original clock RTC may be
changed by factory code; validate/provision time if necessary after restoration.
No assumption of factory loudness until owner reports it.

## Factory comparison result and accepted interim volume

Owner reports factory music is quite a bit louder but distorts. This demonstrates
additional output capability with different content/configuration, not a proven
missing amplifier switch or a hardware fault. Owner explicitly accepts our clear
current level for now; keep codec100/PCMpeak20000 and do not keep chasing gain.
Potential later options are speaker replacement or external Sonos/Spotify playback.
These are future directions, not authorization to remove independent local sound
or start remote integrations now. Restore exact pre-demo clock and verify saved
settings and RTC. Full-power-loss and new physical controls remain open gates.

Clock restored: settings-after-demo.bin byte-matches clock-settings.bin; both
saved boot/table and app writes hash-verified. Restored runtime reports RTC/offline
valid/current time, codec100/peak20000, brightness160, storageESP_OK, idle alarm
state and CLOCK_READY. No TIME correction was necessary. Evidence restore.log
and restored-runtime.log under local-config/clock/factory-comparison.
CURRENT BOARD: our bedside clock again, with the accepted interim local volume.

## Complete working local example: UI integration verification

Owner requests continued implementation to a working example. Keep accepted audio
level and original local-first scope. Next acceptance: exercise actual LVGL input,
editor callbacks, persistent saves, local time save, and alarm overlay controls on
the board through bounded USB test taps, rather than bypassing UI with ALARM/SNOOZE
commands. A diagnostic UI snapshot reports screen/control state; this is evidence
of widget/callback behavior, not a substitute for physical touchscreen or visual
readability. Synthetic input stays on the existing UI owner task, validates display
bounds, uses press/release events, and rejects overlapping taps. No network control.
Preserve disabled/user alarm settings and restore current time after tests.

USB-driven LVGL test passed Set time navigation, alarm-minute dropdown selection,
on-screen Save→home transition and reopen/persisted value26. Prior slot0 disabled
20:25daily configuration restored. Found consumed-date editing bug: a newly chosen
time after today's trigger remained suppressed. Fix contract: changed time or
recurrence/date defines a new schedule and clears consumption; unchanged schedule
(including simple disable/enable) retains duplicate protection. Ignore obsolete
once_date when both schedules repeat. Regression must demonstrate new same-day
trigger and unchanged re-enable suppression, not just field assignment.

Complete on-device UI workflow passed (ui-flow-summary.log/ui-flow.log): synthetic
press/release through LVGL opened the actual alarm editor, selected a new minute,
enabled the slot, pressed Save and verified NVS-owned settings. Real scheduled
minute triggered ringing; visible Snooze then Dismiss buttons changed owner-task
state and removed overlay. Original disabled slot0 20:25daily restored.
Earlier UI console also exercised Set time→Save time→home and RTC reflected the
selected minute. Time subsequently synchronized through TIME for exact test timing.
This verifies real UI event routing, persistence, scheduling and controls on the
board; physical touch alignment was confirmed earlier, while current full-power
loss remains a separate uncompleted check.

Remaining usability addition for the local example: show time remaining while
snoozed and identify active alarm number(s); keep large existing controls and
100ms independent scheduler. Countdown is derived from saved UTC deadline, which
the alarm service rebases on wall-clock correction; it does not drive scheduling.

Final local-example test passed after countdown addition: UI_OVERLAY Alarm1,
snoozed countdown04:59, Snooze/Dismiss transitions and original alarm restoration.
Evidence ui-example-summary.log/ui-example.log; build/flash passed. Installed image
SHA256 3abb1c7ec149d0b95e8a2bdda7926bb0f24660c4178004dd0a08cb6b67aeb81a. Host suites pass, including rescheduling regression.
Read-only USB topology inspection found board at1-1.2 but no uhubctl and no writable
hub power control; no USB power state changed. Physical power-loss remains pending.

## Expanded owner scope: location, weather, polish and extensibility

Owner explicitly requests local weather and today's forecast, location setup
(prefer ZIP) that also selects timezone, polished appearance after functionality,
and clean addition points for Home Assistant and later spoken weather/schedules.
Weather is now authorized next work, despite earlier phase deferral. Preserve
local alarm behavior and treat external playback/voice as later integrations.
First location/forecast provider: Open-Meteo official geocoding supports postal
codes and country filtering and returns IANA timezone. Forecast supports current
temperature/feels-like/condition and daily high/low/precipitation probability.
ZIP requested from owner; no credentials requested in chat. Wi-Fi will be entered
on-device, stored separately from non-secret settings, never logged.
Interfaces before code: provider adapter→validated weather snapshot (timestamp,
units, location, condition, daily forecast, freshness); UI observes snapshots;
background network task owns bounded HTTPS requests/retry, never calls LVGL/RTC.
Persist location and POSIX DST rules corresponding to returned IANA timezone so
timezone remains correct offline; do not store only today's UTC offset.
Unknown/multiple locations require selection/confirmation rather than silently
using an arbitrary result. Unknown timezone leaves previous time zone unchanged.
Network credentials use separate project-owned namespace, with Wi-Fi RAM storage
and nvs_enable=false to preserve factory NVS. NTP updates are handed to the
clock/UI owner for RTC writes. No shared backend needed for direct provider API.

## Weather implementation and setup corrections (2026-09-23)

Owner supplied ZIP27358; Open-Meteo returned Summerfield, North Carolina,
America/New_York. Current weather and daily forecast host request also succeeded.
Added provider-neutral validated weather model, pinned cJSON1.7.19~2 (nest limit16),
IANA2026d future POSIX rule table, background HTTPS/Wi-Fi/NTP service and LVGL
weather/settings screens. Host parser tests cover ambiguous ZIP, invalid/missing
values, wrong units, stale/future responses, truncation and timezone/DST.
First build/flash passed; app-only1745424bytes verified, clock RTC/settings boot
and new screens observed. Factory NVS and PHY calibration writes explicitly
turned off; private clockcfg/network namespace stores network configuration.

Owner correctly rejected typed SSID and combined Wi-Fi/ZIP setup. Revised contract:
scan nearby networks, select SSID, enter password; separate weather/location page.
Manual ZIP always takes priority over inference. An unset location may suggest a
ZIP from public-IP geolocation, clearly marked approximate; never send SSID/BSSID
or Wi-Fi password to geolocation services. Existing supplied27358 is manual.
No inference should run for this configured device. Network scan belongs to the
background worker. Tests must verify responsive clock/alarms during scans and
HTTP, successful network selection, persisted manual location and live weather.

Owner then requested automatic ZIP suggestion only when no manual choice exists.
Implemented strict manual precedence, separate Location page, explicit Use network
choice, public-IP postal parser with country/format/error validation; IP lookup
is skipped for manual27358. Approximate suggestions remain editable and Save
location makes the choice manual. Unknown zones preserve the prior timezone.

Broader GitHub/Reddit/specialist search completed and pinned sources inspected:
REUSE-REVIEW.md records applicable examples, framework/hardware differences and
per-file license caveats. Exact-board Waveshare examples still unavailable; nearby
C5 2.8 weather UI entry points include an empty custom_init, so not a complete
networking replacement. ESP-IDF station example supplied explicit WPA3 SAE mode.

Board scan succeeded with6 AP records. Initial saved authentication failed202;
owner re-entered password, then on-device snapshot became online/valid/fresh.
Firmware restart automatically reconnected; HTTPS200 with732bytes and
NETWORK_TIME rtc=ESP_OK observed. Owner identified password-form navigation bug;
fixed automatic exit on successful selected-network connection and added local
Show/Hide password. Actual clock_ui.c host regression verifies password textarea
is removed after Connect success. No password was read back or printed.

Added actual-LVGL windowless preview, inspected home/weather/network renders,
dark theme and home weather card. Host preview is synthetic data, not hardware
visual proof. Radio/TLS heap budget increased by disabling Wi-Fi IRAM throughput
optimizations and using dynamic TLS buffers. Post-fetch heap ~91KiB stable;
mid-request heartbeat observed66KiB, returned91KiB. Generic SDK logging now uses
nonblocking diagnostic queue too, so unattended USB cannot stall radio/UI logs.
Observed a burst of rejected synthetic taps during TLS; raised UI owner priority3
above network2 while alarm owner remains5. Final app built/flashed/hash verified:
1759952bytes, SHA25623d59810b8b3c866533caf1d3f80de7c339b37d672658ce9e966cbbb56787e87.
Final restart again fetched731-byte forecastHTTP200, RTCvalid and advancing.

Final priority-adjusted verification: all synthetic taps accepted during HTTPS;
UI returned home while request ran. Second test navigated away during AP scan,
ALARM_STATE replied with storageESP_OK and no active alarms, scan returned7 AP
records and heap returned91128bytes. Repeated10-second heartbeats remained live.
No new audible alarm test was run while owner was heading to bed; prior audio and
real5-minute snooze evidence remains recorded separately. Current image's full
power-loss reliability and physical GUI review are still unproven.

## Owner-confirmed power return

Owner reports "powered back on - sudo command given - things look good to me".
Accept this as physical confirmation that the current application/display starts
after the owner's power cycle; prior blank-screen startup fault did not recur in
this check. This does not establish battery-backed RTC retention (battery was
previously disconnected), a powered-off alarm firing, or every alarm-editor control.
Serial access was checked separately; opening USB can itself reset this board, so
subsequent serial boot evidence must not be mislabeled as the original cold boot.

Serial follow-up passed: access works, opening produced USB reset0x15 (not a second
full power cycle), CLOCK_INIT rtc=ESP_OK, SETTINGS_LOAD/ALARM_STATE storageESP_OK,
brightness160 and home screen. Saved Wi-Fi reconnected and weatherHTTPS200 returned
732bytes. Two10-second heartbeats showed system/RTC equal, heap91340bytes stable.
Receipt: local-config/clock/power-return-check.log. No firmware or settings changed.

## Current work: scheduled night brightness (2026-09-24)

Owner defers all further full-power-off tests until functionality is complete and
regards the current visual design as a proof of concept. Next functional slice:
editable local-time night interval (defaults22:00–07:00), persisted enable/settings,
30-second touch wake, bright screen for ringing alarms, and manual brightness when
schedule is off. Existing installations migrate with schedule disabled. Equal
start/end is invalid; overnight/daytime intervals are supported. Invalid clock
uses manual brightness. UI owns backlight; pure policy has no LVGL/network calls.
Store version3 in unused header bytes, preserving version1/2 alarm/recovery data.
Use same save-ticket acknowledgement as alarm editor. No full-power tests or
visual redesign in this slice; build/host/migration and ordinary device checks only.

Night slice completed: pure policy, v3 migration, actual-UI preview tests and IDF6.1
build passed. Inspected480x320 settings render. Backed up clockcfg privately before
migration (before-night-settings.bin,0600; contains secrets, never publish). App-only
flash hash verified: ec31e7d815448459c8e74d2c553fc383d2c341759f8b4fab9e31a2224a166128.
On-device UI enable/save/disable/save passed; all8 alarm records unchanged and
settings storageESP_OK; schedule restored disabled with22:00–07:00 defaults.
First UI assertion sampled before the one-second UI refresh; corrected test waits
for save acknowledgement/home transition. Intermediate ordinary USB reset also
loaded enabled schedule from NVS successfully. WeatherHTTPS200 remained functional.
No full power-off tests were performed. Physical night brightness perception and
long unattended behavior remain unverified. Test receipt: night-ui-check.log;
build/flash receipts: night-build.log and night-flash.log.

## Next functional slice: Home Assistant light adapter (2026-09-24)

Continuing regular functionality without a milestone stop. Read-only discovery
found no HA-named container or8123 listener on this workstation; asked for existing
HA URL, not credentials. No shared stack changes/installations authorized or made.
Official REST/light docs checked. Contract before implementation: configure endpoint,
secret token and one light entity; GET actual state, explicit turn_on/turn_off POST,
then re-read state. No optimistic success or HTTP-only confirmation. Reject invalid
entity/endpoint/header input; bounded response/timeouts; no token/body logging.
Serialize integration HTTP with weather on the same network worker to preserve
heap and alarm/UI priority. Integration unavailable must leave local clock intact.
Setup requires device-side secret entry; no token in chat/source/examples.

Owner clarified HA is already running on a dedicated Pi and explicitly prohibits
installing Home Assistant. Use existing origin http://192.168.1.232:8123; supplied
/homestead-command/home is its dashboard route, not an API prefix. Read-only
unauthenticated GET /api/ returned HTTP401 on2026-09-24, establishing reachability
and required authentication, not authenticated integration success. Do not install
HA or modify the Pi configuration. Token must stay out of chat and logs.

## Offline goal continuation audit (2026-09-24)

Previous turn made progress by verifying the existing HA endpoint without modifying
it. Renewed goal directs attention to offline acceptance; unfinished HA edits remain
uninstalled. Owner prohibition on further full power-off tests remains in force.
Audit found separate engine/codec/recovery tests, but no complete scheduled-trigger
to encoded-settings to restarted-engine test. Add that integration regression using
production modules: once-alarm consumption, active ring recovery, persisted snooze,
exact120-second expiry grace, stale/future rejection and durable dismissal. This
is simulated storage/restart evidence, not NVS atomicity or hardware power proof.
Physical alarm-editor/Snooze/Dismiss check requested without unplugging.

Audit result: all8 host suites pass, including new encoded restart integration.
Re-read retained reset/snooze/night receipts and verified current local binary
SHA256 matches night installation receipt. No board writes, resets, new firmware
installation or full-power tests performed. Updated CLOCK-ACCEPTANCE/TESTING with
precise evidence limits. Physical controls and deferred power-loss checks prevent
claiming full goal completion; software-driven checks do not substitute for them.

Recovery preservation revalidated2026-09-24: full factory-flash.bin SHA256 remains
a963c18040b476d61ec4dc8f630f1b1e38c5b61cc69f6c530a8a7c6676d302ab,
matching the original recovery receipt. No device operation performed. Previous
goal turn made progress through encoded restart regression and acceptance audit.
Physical-control confirmation is still unanswered and full-power testing remains
owner-deferred; this is the second consecutive resumed goal turn encountering
that completion blocker. Offline regression/evidence work is saved separately
from unfinished integration source. No Git remote exists for publication.

Third consecutive resumed goal audit2026-09-24: same remaining blocker confirmed.
Prior turn made progress by revalidating recovery bytes and committing tests; no
physical-control answer or revised power-test authorization has arrived. Current
acceptance record does not prove full power-loss behavior or final physical alarm
controls. Available software checks cannot supply that evidence. Mark goal blocked
on physical confirmation and deferred power tests, not complete. Installed image
is unchanged; no additional reset/flash or HA installation performed.

## Owner correction: continue development, batch physical acceptance (2026-09-24)

Owner explicitly rejects physical testing as a gate between partial features.
Continue authorized application development, including existing-Pi Home Assistant
integration; batch extensive physical acceptance after functional integration.
The older goal's integration deferral no longer governs this development sequence.
Do not repeatedly request partial-feature checks. No HA installation, Pi changes,
or full power-off testing. Finish device setup and observed light-state controls,
host/service failure checks, preview and firmware build before identifying genuine
external credential needs. Existing independent local alarm behavior is retained.

HA light adapter delivered2026-09-24: Settings → Home Assistant → Setup/control,
owner-provided Pi address prefilled, immediate masked token, private NVS, actual
entity GET and explicit on/off POST followed by confirmation. Shared bounded HTTP
worker keeps network out of UI/alarm owners. Host service tests cover queued work,
HTTP200 with unchanged state, confirmation timeout even when later reads fail,
auth/offline/stale state, blank-token endpoint binding and failed config retention.
All10 host suites pass. Actual LVGL ha-test passes; setup render inspected.
IDF6.1 build passes, app1769664bytes; app-only flash verified. Installed SHA256:
04e749a08a00cb0809a73fe1f13de353a1ad2dbb14f5692917fbd35757ce9a0b.
Automated on-device navigation passed home/settings/ha/ha_setup/back/home. RTCvalid,
clock/RTC within1second, alarm storageESP_OK, weatherHTTP200731bytes, heap88924
stable across observed heartbeats. No credentials entered, no light command sent,
no Pi configuration changes and no full power-off testing. Receipts: ha-build.log,
ha-flash.log, ha-runtime.log under local-config/clock. Real authenticated HA needs
an actual token/entity; this does not block continuing unrelated application work.
Do not request owner testing after this partial feature. Batch physical acceptance
when functionality is integrated, per explicit owner correction.

## Audio failure visibility (2026-09-24)

Continuing development without physical acceptance gates. Inspection found codec
initialization errors return silently to the clock UI, I2S setup errors abort the
whole clock, and Test sound ignores queue failure. Acceptance for this fix: publish
atomic audio status; failed audio initialization leaves time/UI/alarms operational
and visibly reports unavailable audio; runtime I2S errors remain visible until a
successful playback attempt, with bounded retries rather than a tight failure loop.
Preserve existing waveform/volume and local snooze/dismiss. Preview injects fault
state to verify visible warning plus operable alarm controls. Physical fault/audio
checks remain deferred; normal build/startup is checked automatically.

Audio failure visibility completed: I2S init and task/queue allocation failures
return an unavailable-audio state instead of aborting clock startup; codec failures
are also visible through the default failed status. Playback errors publish atomic
status and delay1second before retry; successful playback clears the runtime error.
Silence-buffer short writes are checked too. Home and ringing overlay expose audio
failure; Test sound reports queue acceptance/rejection. Tone/volume unchanged.
Actual-UI fault preview passed warning/title/Snooze/Dismiss assertions. IDF6.1 build
passed; app-only flash verified1769888bytes, SHA256:
54fc5e949db4c57716f1efb92609293b7519d5e629204d9ca9e05596cfeeb36c.
Automatic USB startup check: AUDIO_READY at prior volume100/peak20000, AUDIO_STATE0,
RTCvalid and within1second, alarm storageESP_OK, weatherHTTP200730bytes, heap88924.
No physical test requested, no power-off performed, no new acoustic claim. Injected
UI error is not a real codec/I2S failure test. Receipts: audio-status-build.log,
audio-status-flash.log, audio-status-runtime.log. Development continues with final
physical acceptance batched per owner direction.

## Network startup failure isolation (2026-09-24)

Inspection of pinned IDF6.1 wifi_default.c showed the convenience station factory
asserts on netif allocation and ESP_ERROR_CHECKs attach/handler setup. Application
weather/HA queue allocations also abort. Replace those optional-network fatal
paths with checked failures and immutable unavailable snapshots when resources
cannot be created. Reject commands when no worker can consume them, including HA
when radio/task startup fails. Preserve persisted timezone loading before alarms.
Acceptance: build and normal startup; host injection of HA mutex/queue failure and
worker disable proves safe snapshots/rejected commands; no physical test required.

Network isolation fix installed: checked netif allocation/attach/default handlers
replace the aborting SDK convenience factory. Weather and HA optional allocation
failures now expose safe unavailable snapshots, reject commands and leave local
startup intact. Network worker startup/radio failures also disable HA submissions;
queue submission rechecks availability under the state lock to avoid stranded busy
state during startup failure. Saved timezone still loads before the alarm owner.
All10 host suites plus mutex/queue-failure scenarios pass; IDF6.1 build passes.
App-only flash verified1769664bytes; SHA256:
787d304942a3005102f75276436854e2e4fc177055157547ad4fc55598c3b322.
Normal hardware boot: audio ready/status0, RTCvalid within1second, clock home,
alarm storageESP_OK, weatherHTTP200731bytes, heap88936. No power-off or physical
interaction requested. Host failures cover HA resource/worker behavior, not real
radio or whole-device OOM injection. Receipts: network-isolation-build.log,
network-isolation-flash.log, network-isolation-runtime.log.

## Alarm overview (2026-09-24)

Next regular offline workflow: replace home Alarms opening slot1 directly with a
scrollable eight-alarm overview. Show time, enabled state and recurrence/date;
select a row to edit that slot. Save returns only after the existing persistence
acknowledgment; Cancel returns without saving. Keep current editor controls and
local alarm overlay. Acceptance: actual LVGL preview selects/edits slot8, verifies
other slots unchanged and return-to-list on Save/Cancel; render check, build and
automatic device navigation. No new settings schema or physical acceptance gate.

Alarm overview installed: eight scrollable rows show24-hour time, ON/OFF and
weekday/once-date summary; each opens its corresponding editor. Acknowledged Save
and Cancel return to the overview. Existing dropdown remains available within the
editor. Host actual-UI slot8 edit/save, seven other slots preserved, Cancel return
passed;480x320 render inspected. IDF6.1 build and app-only flash verified1770704
bytes, SHA25699d609f26851af785f7604452932c8a11e7408b77b37fea6f5fc8b7648d57225.
On-device automated home/alarms/editor/cancel/alarms/home passed. All8 saved alarm
records compared identical before/after; storageESP_OK, RTC/systemequal, heap88892
stable, weatherHTTP200731bytes. No physical interaction or full-power test needed.
Receipts: alarm-list-preview-build.log, alarm-list-build.log, alarm-list-flash.log,
alarm-list-runtime.log. This adds normal alarm-management functionality without
changing persistence schema or requiring another owner acceptance interruption.

## Installed offline-operation check (2026-09-24)

Add bounded serial diagnostics NETWORK OFF / NETWORK ON, queued to the existing
network owner. OFF stops the board radio and suppresses reconnects; ON starts it
and reconnects using untouched stored credentials. Mode is volatile and ordinary
restart restores normal connection. No router/HA/shared infrastructure change.
Use this to verify RTC/alarm scheduling, snooze/dismiss and UI while networking is
actually off. Preserve all8 alarm records, choosing only an unused unconsumed slot
for a temporary once alarm and restoring it. No power-off or owner interaction.

Installed offline check passed: app-only flash1771536bytes verified, SHA256
 dc90d92d80bd6126448bdb5432a89e844974067602202791e681d43e8f7250d6.
NETWORK OFF acknowledged by owner with ESP_OK; stopped-radio interval showed
RTC/system within1second, heap92564stable and no weather HTTP. All alarms were
initially disabled; selected unused slot0 with consumed_date0, saved a temporary
once alarm for next bounded RTC deadline. It triggered mask1 with persistenceESP_OK,
entered ringing, and produced repeated AUDIO_TEST_DONE ESP_OK177152-byte blocks.
SNOOZE produced ringing0/snoozed1; DISMISS cleared both. Restored slot0 and compared
all8 alarm records exactly against the pre-test snapshot. NETWORK ON acknowledged
ESP_OK; weatherHTTP200730bytes confirmed reconnection. No Wi-Fi credentials changed.
No full-power-off or physical interaction used. Audio transmission is established,
not a new human acoustic observation. Private receipts: offline-check-build.log,
offline-check-flash.log, offline-alarm-runtime.log, offline-alarm-original.json.
This strengthens current-image offline independence evidence; deferred final
physical/power acceptance is not a gate to further development.

## Combined screen stability check (2026-09-24)

Previous turn progressed current-image offline proof and exact restoration. Next
bounded check exercises12 navigation transitions per cycle across home/settings/
night settings/HA/setup/alarm overview/editor, using only Cancel/back and no saves.
Record20cycles, heartbeat/RTC agreement, heap range and exact8-alarm preservation.
Abort on active/enabled alarms, crash, wrong screen or changed settings. This is
an automated combined-feature check, not another owner physical acceptance request.
Add reproducible script with private timestamped receipts and bounded iterations.

Combined navigation soak passed20cycles/240screen transitions, with exact8-alarm
records unchanged after every cycle and final home screen restored.25valid RTC
heartbeats stayed within2seconds of system time. Heap88728..89028bytes;
early/late maxima88768/89028, no net loss observed in this bounded run. Receipt:
local-config/clock/navigation-soak-20260924-104659.log. No firmware update, settings
save, audio request, full-power-off or physical interaction performed. Added the
reproducible navigation script and concise USER-GUIDE linked from README; Python
compile and documentation verifier pass. This is combined UI stability evidence,
not unlimited-uptime or full-power-loss proof. Deferred physical checks remain
batched; no new request to the owner.

## Common alarm recurrence presets (2026-09-24)

Reuse follow-through: inspected cached flight-radar alarm.h (ESPHome/C++ packed
weekly-mask model) and pinned LVGL9.4 dropdown example. Existing scheduler already
uses the compatible Sunday-first seven-bit model; replacing it adds no capability.
Use the existing LVGL dropdown pattern for Every day / Weekdays / Weekends / Custom /
Once, synchronizing seven day buttons to the selected mask. Manual day edits update
the displayed preset; empty custom selection stays invalid on Save. No persistence
schema change or third-party application code copied. Acceptance: actual UI preset
save masks127/62/65, custom adjustment, once-mode visibility and original slot
isolation; firmware build and automated navigation, no owner testing gate.

Repeat presets installed. Actual-UI test passed127/62/65 saved masks, weekend+Monday
custom67, once date-field visibility, empty repeating mask rejected without save,
and unchanged remaining7slots. Build and app-only flash verified1771776bytes:
66f1e5d709419fee8f4027b6da6c8c0a935d9758a10ac0298056bd83f440c601.
Automated on-device overview/editor/Cancel/home passed; all8 saved records exactly
unchanged, storageESP_OK, RTC/system within1second, heap88760stable, weatherHTTP200.
No physical test request. Updated user/firmware guides and preview instructions.
Receipts: repeat-preview-build.log, repeat-build.log, repeat-flash.log,
repeat-runtime.log. This adapts standard widget/state patterns; it does not claim
that any third-party application was transplanted or replace working scheduler code.

## External media controls (2026-09-24)

Following owner's explicit correction to continue integration development and batch
physical acceptance, proceed beyond offline core while retaining its independent
alarm execution. Use existing HA REST media_player actions/state, not direct MCU
Spotify streaming. Reuse the credential owner and bounded HTTP worker; separate
player entity preferences from secrets. Implement state/metadata and supported
play/pause/previous/next/volume controls. HTTP acceptance is not playback proof;
show observed state only. Do not route alarms to remote playback in this slice.
No HA/Pi changes; authenticated device operation awaits user-owned credentials.

Media controls delivered: player preferences in clockcfg/media (no token), bound to
HA endpoint; credential owner supplies authenticated requests only on existing
network worker. HA light entity is optional for a media-only connection, preserving
existing credential layout. UI under Settings/Media shows bounded name/title/artist,
state/volume and six controls gated by actual supported_features and fresh state.
Uses HA2026.9.3 feature flags from official const.py (PLAY16384, not PLAY_MEDIA512).
Play/pause confirmation reads observed state with10-second confirmation/backoff;
previous/next/volume report actual state without claiming an HTTP acknowledgment
proves physical playback. Server changes cancel queued actions targeting old server.
No remote alarm routing, playlist/context selection, streaming or Pi config changes.

All12 host suites plus HA/media allocation-failure variants pass; actual-LVGL media
play/pause/setup/cancel/stale controls pass. Inspected480x320 synthetic render.
Initial firmware build rejected two misleading-indentation lines; corrected both,
then successful IDF6.1 build and app-only flash verified1779456bytes. SHA256:
059cf5aa7bbe8c99cbd653f13fefc0fbf7fae0c9ec8f3df65a273f47b7f60db6.
On-device home/settings/media/setup/cancel/media/home passed. All8 alarm records
unchanged, storageESP_OK, RTC/system within1second, heap87284stable, weatherHTTP200.
No real token/entity configured and no media commands sent to the Pi. Live external
playback remains unverified; local alarms stay independent. Receipts: media-build,
media-preview-build, media-flash, media-runtime logs under local-config/clock.
Physical acceptance remains batched per owner's correction, not a development gate.

## Computer-side credential provisioning (2026-09-24)

Make existing HA/media setup usable without typing a long token on the touchscreen.
Add bounded USB setup messages with request tags and owner-task persistence
acknowledgments. Never echo JSON/token; clear serial command buffers after handling.
Local Python helper prompts with getpass only in a terminal, accepts no token CLI
argument, writes no credential file and prints only known acknowledgment fields.
Handshake before requesting credentials, chunked writes, finite acknowledgment
waits; queue acceptance alone is not reported as saved. Device setup changes only
clock-owned preferences. Host parser/failure tests and device read-only handshake
check precede delivery; no real token requested during implementation.

USB setup delivered with hidden terminal token entry, read-only HA entity discovery,
request-tagged queue/persistence acknowledgments and separate player selection.
No real credentials supplied and no HA configuration or playback requests made.
All13 host suites plus allocation-failure variants and6 helper tests pass.
First device overflow test at32bytes/20ms failed to receive its expected reply;
slower80ms transmission passed. The SDK default256-byte RX buffer was too small
for reliable long-message handling across UI work. Increased it to4096bytes to
hold a complete bounded setup frame, rebuilt and repeated at the normal20ms rate.
Final device check passed handshake, malformed JSON rejection,2100-byte overflow
rejection, subsequent handshake recovery, and exact preservation of all8 alarm
records. RTC/system within1second, storageESP_OK, heap81400..81412 in the bounded
check; weatherHTTP200. No power-off or owner physical check required.

Final app-only installation1780848bytes verified by esptool; SHA256:
956d2cd39bb555c3891e6dfa205046ac2dd7263e277301e1c47290d795857e21.
Private receipts: usb-setup-build.log, usb-setup-flash.log,
usb-setup-runtime.log (initial failed check), usb-setup-runtime-retry.log (slow),
usb-setup-runtime-final.log (normal rate, pass). Computer setup instructions are
in HOME-ASSISTANT.md. Persisted acknowledgment tests are synthetic; actual token
acceptance/external playback remain unverified. Offline goal remains active with
batched physical alarm-control and full-power-loss checks outstanding, as directed.

## Settings storage boundary verification (2026-09-24)

Previous turn delivered installed USB provisioning. Next offline-core gap: existing
codec/recovery tests bypass settings_store.c. Exercise the production adapter with
injected NVS failures, missing/corrupt data, and successful save/reload. Require
safe disabled defaults on failed load, propagation of write/commit errors, no commit
after a failed write, and exact encoded alarm/active-phase reload. These synthetic
checks cannot establish physical NVS power-loss atomicity; no power interruption.

Storage boundary verification passed all seven isolated scenarios against production
settings_store.c. Failed init/open do not permit saves; failed/corrupt reads leave
disabled defaults and return the error. Failed writes do not invoke commit; commit
errors propagate. Successful save/reload preserves encoded alarm, consumed date,
snoozed phase/deadline and brightness exactly. Invalid settings are rejected before
another NVS write. All existing host suites also pass. No production firmware
change, flash, board restart or physical test was performed; installed27c3bad image
remains. This adds adapter-level evidence, not simulated physical flash atomicity.

## Monotonic snooze countdown (2026-09-24)

Storage-failure messages already exist in editor/home. Review found snooze UI derives
remaining seconds from persisted wall-clock deadlines while execution uses monotonic
time. Publish remaining seconds from the alarm owner's runtime instead; keep wall
clock deadlines solely for restart recovery. Acceptance: ceiling at partial seconds,
earliest simultaneous snooze, zero after expiry/dismissal, countdown independent of
wall-clock changes, actual UI renders supplied remaining time. No settings migration.

Monotonic countdown installed: host engine boundaries and actual-LVGL Snooze05:00
assertion pass; all host suites pass. Initial preview build exposed a nonexistent
test helper; replaced with actual overlay-label assertion and rebuild passed.
IDF6.1 build/application-only flash1780800bytes verified; SHA256:
0506972d800f56648ce0bb1a789e0ca57548b8b9c6698bbba85aa6fdd893eed8.
On-device startup uses RTC, system/RTC within1second, storageESP_OK, heap81512stable;
automated settings/media/setup/cancel/home navigation preserves all8 alarm records.
No power-off or physical test request. Receipts: snooze-preview-build.log,
snooze-build.log, snooze-flash.log and snooze-runtime.log under local-config/clock.
The reused navigation helper also overwrote media-runtime.log with this run; the
older media-runtime receipt is no longer independently available at that path.
New countdown behavior is verified in host/runtime-model and actual-LVGL preview,
not a new five-minute physical snooze test. Full-power-loss checks remain deferred.

## Offline completion-evidence audit (2026-09-24)

Previous turn made concrete progress by installing the monotonic snooze correction.
Reviewed local audio cancellation and acceptance scope. Alarm playback polls its
active flag each256-frame block; SDK default DMA capacity is6x240frames at22050Hz.
These are code-derived bounds on generated/buffered content, not acoustic timing
proof; blocking writes have1000ms timeout. No speculative audio modification made.
Corrected CLOCK-ACCEPTANCE historical results mislabeled as current-image tests,
and identified exactly what the c782e14 installation receipts establish. Physical
alarm controls and full-power-loss behavior remain unproved and owner-deferred;
this audit does not satisfy the full goal or justify marking it complete. No board
restart, flash, physical test request, or HA/Pi change in this audit.

## Installed-image offline verification (2026-09-24)

c782e14 tested with existing bounded offline test in a separately named private
copy, preserving earlier receipts. All8 alarms initially disabled; unused slot0
consumed_date0 selected. NETWORK OFF returned ESP_OK. RTC/system stayed within
1second, offline heap85164stable, and no weatherHTTP appeared during stopped-radio
interval. Temporary once alarm triggered mask1 with persistenceESP_OK; I2S wrote
177152bytes successfully. SNOOZE changed ringing1 to snoozed1; DISMISS cleared both.
Restored original slot0, compared all8 records exactly, NETWORK ON returned ESP_OK,
and weatherHTTP200731bytes verified reconnection. Test exited0. No power-off,
credential changes, physical input or new acoustic claim. Current-image offline
independence is now directly evidenced; five-minute snooze duration and full-power
recovery remain covered only by their separately scoped historical/pending evidence.
Receipts: offline-snooze-c782e14-runtime.log and original.json under local-config/clock.

## Bedside home hierarchy (2026-09-24)

Owner requested substantial presentation improvement after core functionality and
continued work while physical tests remain batched. Begin with home hierarchy:
larger time, a distinct tappable next-alarm card, quieter date/time-source details,
and secondary weather. Preserve existing top/bottom navigation hit areas and all
alarm behavior. Render populated/offline home and run actual-UI navigation/night
checks before build/install. This is an incremental home improvement, not a claim
that complete interface redesign or final physical acceptance is finished.

Owner reconfirmed during this work: complete regular functionality first, then
continue substantially improving appearance. This home layout is only a first
pass. Final visual work must cover typography, weather graphics, spacing, colors,
and consistent alarm/settings screens; do not treat it as completed design polish.

First home-layout pass installed: larger time, quieter date/detail, tappable next
alarm card and secondary weather; bottom/top navigation positions preserved.
Actual LVGL populated/offline renders inspected; night behavior and alarm editor
preview checks pass. IDF6.1 build/app-only flash verified. SHA256:
e30394faa7f34a651c869636793fc75af2d93d731e700749a4b86507ce55a98b.
Device test opens next-alarm card, returns home and traverses existing settings/
media/setup/cancel paths; all8 alarm records unchanged. StorageESP_OK, RTC/system
agree, heap81328..81364. No power-off or physical input. Receipts: home-preview-build,
home-build, home-flash, home-layout-runtime logs. Screenshots inspected from /tmp.

## Remaining functional scope audit (2026-09-24)

Previous turn installed a first-pass home layout and recorded owner's explicit
functionality-first/final-polish-later preference. Inspected current HA/media APIs
and guide: manual player transport/volume exists, but selecting/starting a media
context and remote-alarm fallback do not. Those remain broader application work;
no claim of a completed Spotify/external-speaker alarm is justified. Live HA access
also remains unverified without user-provided credentials through the private setup
path. Existing Pi configuration remains outside authorization.

The narrower persisted offline-clock goal still lacks batched physical control
and full-power-loss acceptance, as recorded in CLOCK-ACCEPTANCE. These are owner-
deferred; neither extra documentation nor another identical build proves them.
No firmware/board changes in this audit, no new physical test request, and goal
not marked complete. Continue broader authorized functionality independently of
that acceptance gap; final visual polish remains after regular functionality.

## Saved external media selection (2026-09-24)

Continue owner's broader functional scope while physical acceptance stays batched.
Implement an optional bounded media ID/type saved with the player and bound to its
HA server. Preserve version1 player preferences on migration. Add explicit Start
saved action, gated by fresh PLAY_MEDIA capability512 (distinct from resume PLAY).
Encode JSON safely; reject incomplete/oversized selections. HTTP200 or a reported
playing state must not claim the requested playlist is verified. Local alarms stay
independent; no remote alarm routing or Pi changes. Verify migration, persistence
failure, escaped IDs, capability gating, request body, and real-LVGL setup/control
states before installation. Do not send real playback requests during verification.

Saved external selection delivered and installed. Version2 player preferences add
bounded content ID/type; legacy version1 loads with empty selection. Explicit Start
saved uses PLAY_MEDIA512 and safe cJSON encoding; Play remains resume. Shared
network worker owns requests; request storage allocated with response on heap to
avoid growing its stack by1152bytes. All host suites pass, including legacy/new
selection reload, invalid/escaped input, action/service body, failed-save retention,
and honest playing-state reporting. Real-LVGL media-test passes setup/save/start,
play/pause/cancel and all7 stale controls disabled; two renders inspected.
IDF6.1 build and app-only flash verified; SHA256:
e566e433e168e42f7ecdfe231ae6ad0234dfdec59aac1e7da86727f309c83a80.
On-device navigation/setup/cancel/home passed, exact8alarms unchanged, storageESP_OK,
RTC/system within1second, heap80168..80200. No token supplied, no real media request,
no Pi modification or power-off. Receipts: selection-host.log, selection-preview-build,
selection-build, selection-flash and selection-runtime logs under local-config/clock.
Live authentication/player/content behavior remains unverified; remote-alarm fallback
is still not implemented. Owner raised open-source portability; recorded standalone
base/optional HA boundary and current direct-HA media limitation in PROJECT.md.

## Optional media adapter boundary (2026-09-24)

Replace the media owner's direct HA dependency with a small backend interface for
configuration identity, target validation, state reads and actions. Keep queue,
persistence, freshness and confirmation in the owner; HA adapter owns JSON/REST,
feature-number translation and credential-owner access. Publish generic action
capabilities to UI. Retain existing player preferences and actual behavior, with
one HA implementation today; do not claim a non-HA player backend exists yet.
Acceptance: existing production service tests through the HA adapter, generic
capability tests, actual-LVGL preview and firmware build; local alarm modules unchanged.

Media adapter separation installed. Generic media_model/service contain no HA
headers, REST paths, JSON parsing or HA feature constants. media_ha.c owns those
and accesses credentials only through ha_service. Owner retains queue/NVS/freshness/
confirmation; existing preference layouts preserved. An alternate backend test
links the real owner/model without HA/HTTP/JSON and exercises configure/read/play/
pause/offline using non-HA identity/target strings. All host suites and real-LVGL
media-test pass. HA remains the only shipping backend; no runtime plugin selector
or direct Spotify/Sonos backend is claimed. Offline alarm modules unchanged.
IDF6.1 build/app-only flash1785328bytes verified; SHA256:
6fe15e5c75758f327d9c8b07448a7be21e3705ec704056cae17c0572c96b2d96.
On-device navigation passed, all8 alarms unchanged, RTC/system agree, storageESP_OK,
heap80148..80160. No playback or Pi changes. Receipts: backend-host.log,
backend-preview-build.log, backend-build.log, backend-flash.log, backend-runtime.log.

## Alarm-owner persistence fault test (2026-09-24)

Close a specific offline-core test gap by running production alarm_service.c's
owner task under a deterministic fake RTOS/storage/audio boundary. Verify failed
trigger checkpoints do not suppress local sound, Snooze/Dismiss stay responsive,
failed persistence retries recover without retriggering, and failed tracked edits
retain prior alarms and report their exact ticket/error. No board changes or
physical power-fault simulation; codec/engine tests alone did not cover this owner.

Production alarm-owner fault tests passed both scenarios. With checkpoint writes
returning93, the RTC-due alarm still activates audio, Snooze publishes300seconds
and silences audio, and Dismiss clears active phases. Once storage recovers, the
bounded five-second retry saves consumed date/dismissal without another ring;
three failed state-transition checkpoints plus one retry were observed. A tracked
edit failure preserves the original07:00 alarm and matching failure ticket; a
subsequent08:00 edit succeeds and is persisted. All existing host suites pass.
Only test/docs changed; no flash, reset, credentials or user settings touched.
This closes owner-loop fault coverage, not physical power-loss/acoustic acceptance.

## Offline-goal completion audit and dependency (2026-09-24)

Rechecked repository state, installed-image hash against installation receipt, and
backend-runtime evidence. Offline clock features are implemented; sound/display/
touch have owner observations, while scheduling, persistence, MCU recovery and
network-independent execution have the scoped device/host receipts above. Latest
owner-task fault tests add storage-failure coverage without a firmware change.
The physical-control/full-power-loss acceptance gap has remained across successive
goal continuations and is explicitly owner-deferred until combined functionality
is ready. No new independent software defect or required offline-core implementation
step is identified by this audit. Another duplicate test or audit cannot prove
those physical outcomes. The narrow offline goal is therefore blocked on deferred
acceptance, not complete. Preserve the installed application and no-power-off
instruction. Broader project work (remote-alarm behavior and final visual polish)
is still unfinished and is not claimed complete by this goal audit.

## Remote alarm implementation resumed (2026-09-24)

Owner explicitly rejects deferred physical acceptance as a development blocker:
physical tests occur at the end unless necessary to prove feasibility. Prior goal
blocking decision does not gate implementation. Add opt-in saved-player alarm use,
with an independent8second local deadline, exact-media/fresh-session confirmation,
and local fallback latched on timeout/lost confirmation. HTTP runs only on network
worker; Snooze/Dismiss invalidate the session immediately and queue best-effort
remote pause. No real remote playback during development. Test policy, cancellation
races, unsupported/offline/auth failures, migration and UI; preserve local defaults.

Remote alarm implementation installed. Defaults remain local-only. Version3 media
preferences migrate older layouts with remote use disabled. The network worker
starts the saved selection once per alarm session; local scheduling owns the
eight-second deadline and three-second confirmation lease. Only exact content,
playing state, known nonzero volume and known unmuted status count as proof.
Fallback latches for that ringing phase; late responses cannot silence it.
Snooze/Dismiss invalidate the session immediately; remote pause is best-effort
with bounded retries and cannot block local controls. Server identity and target
are pinned for cleanup. This is state confirmation, not acoustic proof.

Host suites and real-LVGL media-test passed, including v1/v2 migration, failed
saves, offline/start timeout, cancellation during request, wrong/muted/zero-volume
media and cleanup exhaustion. A test exposed normal polling delaying the first
pause attempt; cleanup now clears that polling deadline and the regression passes.
IDF6.1 build and application-only flash1787888bytes passed, SHA256:
6014036b05befa473bd2ea846967f40407b602f4c31a4eff2406b20f5179c01f.
USB media/setup/cancel/home navigation passed; all eight alarms unchanged,
storageESP_OK, RTC/system agree, heap78268..78304, weatherHTTP200. No HA credentials,
remote playback, Pi changes or physical power-off tests. Logs under local-config/
clock: remote-alarm-host.log, remote-alarm-build.log, remote-alarm-flash.log and
remote-alarm-runtime.log.

## Resume and installation checkpoint (2026-09-30)

Previous status-only turn classified as no progress. Revalidated the worktree and
completed September24 device-test receipt rather than repeating physical tests.
The offline-remote-default-runtime.log records SDK radio-off, durable RTC alarm
trigger, successful I2S transmission, Snooze/Dismiss, radio-on and weatherHTTP200.
Its final eight ALARM_SLOT records exactly match the saved original JSON. This
verifies the installed remote-alarm image retains default local-only operation;
it is not acoustic or full power-loss proof. No claim of six-day uptime is made.

Physical acceptance remains deferred by explicit owner instruction and is not a
blocker for authorized implementation. The previous blocked-goal audit is historical
and superseded for development sequencing. Final physical verification remains
required before declaring the offline goal achieved.

## Local sound cancellation (2026-09-30)

Previous turn made progress: installed remote-alarm changes committed6912d55.
Code review now finds a local-control defect: audio.c checks cancellation only
when a sequence started as an alarm. A test already playing, or queued before an
alarm, can therefore remain audible after Snooze/Dismiss. Acceptance: each alarm
transition invalidates older sound requests, including a complete on/off cycle
between audio blocks; reject test requests while local alarm sound is active.
Keep tone, codec and DMA configuration unchanged. Verify the shared atomic policy
with host tests, compile firmware and retain the acoustic-latency limitation.

Cancellation fix installed: audio_control tags queued/running sound contexts with
an atomic generation plus alarm flag. Every activity transition invalidates older
contexts, even if both start and dismissal occur between audio blocks. Unchanged
owner updates do not cancel playback; new test requests during local alarms are
rejected. All host suites passed, IDF6.1 build passed, app-only flash1788016bytes
hash verified. Installed SHA256:
229e1167f30362df71e6891a1d069bc4db77ece1b04ce74250906218c91b2b94.
Device boot reported RTC/system within1second, storageESP_OK, weatherHTTP200 and
heap78380. Navigation preserved all eight alarms; SOUND was accepted and I2S
transmission completed ESP_OK. Receipts: audio-cancel-host.log,
audio-cancel-build.log, audio-cancel-flash.log, audio-cancel-runtime.log under
local-config/clock. This verifies policy and transport, not acoustic stop latency.
No physical power interruption or HA changes.

## Invalid-time startup recovery coverage (2026-09-30)

Previous turn progressed the goal with installed sound-cancellation fix0699193.
The owner loop waits for valid RTC time before restoring persisted active phases;
existing owner tests always supplied valid time. Add production-owner scenarios
for delayed validity: brightness edits must preserve the pending snooze, while
Dismiss or editing that alarm must cancel it before recovery. Verify durable
state, no premature sound, and no revival after time becomes valid. This covers
software behavior following an invalid-time boot, not RTC backup-power retention.

All three production-owner scenarios passed with the full host suite. During
invalid time there is no sound, no restored runtime phase and no unsolicited
checkpoint. Brightness25 saves without discarding the durable snooze; valid time
then restores its remaining interval. Dismiss and tracked alarm edits clear the
durable phase/deadline before valid time, so recovery does not revive them.
Receipt: local-config/clock/invalid-time-owner-host.log. No firmware change or
flash was needed; installed0699193 remains in place. These deterministic tests
do not substitute for physical RTC retention or abrupt power-loss acceptance.

## Final acceptance preparation (2026-09-30)

Previous turn progressed restart coverage with8c46a29. Current audit preserves the
remaining physical gaps rather than claiming software tests close them. Added a
single end-stage procedure to CLOCK-ACCEPTANCE: preserve settings, physical edit/
brightness, offline audible alarm, full Snooze/Dismiss cycle, reset, documented
power topology, full interruption and active-phase/save interruption. It explicitly
distinguishes absent RTC backup from firmware recovery and restores all settings.
No physical test or power-off request is being made now. Updated contradictory
USER-GUIDE claims about saved/external playback; no firmware/runtime change.
Final visual polish remains authorized unfinished work before the combined round.

Procedure review exposed a concrete remaining setup gap: NETWORK OFF is RAM-only.
A reset restarts connection attempts, so it cannot isolate offline cold-boot
acceptance. Next bounded implementation: persisted user Wi-Fi off/on control,
preserving credentials and default behavior, with startup/re-enable tests. This
is an offline-operating control, not a new external integration.

## Persistent offline mode (2026-09-30)

Acceptance before coding: save a separate radio-enabled flag without altering
credentials/location or alarm settings. Missing flag preserves legacy enabled
behavior; unreadable/corrupt flag keeps radio off and reports error. A saved off
flag must prevent esp_wifi_start and connection attempts at boot. Failed saves
must not change current radio state. UI provides explicit on/off; saving network
credentials while off preserves off. NETWORK OFF/ON adopts the same persisted
semantics. Verify host persistence faults and actual MCU restart offline, then
restore on and reconnect. No full power-off or shared network changes.

## Updated speaker and UX priority (2026-09-30)

Owner now says onboard volume is insufficient to wake anyone and wants external
sound for alarms and music; earlier provisional tone acceptance establishes
audibility only, not acceptable wake-up loudness. Speaker model question is
pending. Existing Sonos through the existing HA server was recommended, with
Music Assistant optional later; no service installation or HA modification was
authorized/performed. External playback remains unverified and cannot establish
a loud offline fallback. Owner also requests greater UI/UX priority. Final
acceptance must retain the volume limitation explicitly, not recycle the earlier
provisional acceptance as proof of bedside reliability.

Persistent offline mode installed and verified. Host suites cover legacy missing
key, off/on reload, malformed/read errors (off), write/commit errors; actual LVGL
radio-test passes both controls and labels, rendered Wi-Fi screen inspected.
IDF6.1 build and app-only flash1788864bytes hash verified. Installed SHA256:
1f36803fb0eda121b7c88b21a2641afe8dee29214186ac33c074aea70ba9a89f.
On-device NETWORK OFF succeeded, an esptool chip-id/reset cycle was performed,
then boot reported NETWORK_SETTINGS enabled=0 ESP_OK and NETWORK_BOOT enabled=0.
RTC-source time agreed exactly during offline observation; no weather request or
IP connection was observed. All eight alarm records matched before/after.
NETWORK ON succeeded, weatherHTTP200 and network time resumed using the existing
credentials; final heap78304. No full power interruption. Receipts under
local-config/clock: radio-persist-host.log, radio-persist-preview-build.log,
radio-persist-build.log, radio-persist-flash.log, radio-persist-runtime.log.

## Product direction reset (2026-09-30)

Owner rejects the narrow, kludgy demo trajectory and expands direction to a family
website, personalized devices, ChatGPT/Dot/Pet and Libre. Stopped the not-yet-edited
alarm-only cosmetic pass to establish coherent product boundaries. Added
FAMILY-COMPANION.md and PROJECT scope override: offline execution remains local;
website configuration requires durable device acknowledgments and conflict handling;
HA optional; person-specific sharing; authenticated assistant tools; read-only
freshness-aware Libre presentation. Official Dot/Pet/voice and Abbott sharing docs
were read. No supported direct Dot embedding or Libre application API was verified.
No firmware change, service installation, health-data access or publication.
Next concrete slice is the companion/device interaction design, then one paired
web-to-device alarm configuration path. Questions sent for intended Dot/Pet,
Libre sharing scope and speaker model remain pending; design need not wait.

## Companion interaction prototype (2026-09-30)

Acceptance: local responsive prototype with family/device selection, matching
bedside preview, alarm editing, and explicit simulated pending/applied/conflict
states. Preserve applied settings until acknowledgment; offline changes remain
pending. Label synthetic data and disconnected integrations. Include private
health presentation and assistant/music empty states. No accounts, publication
or firmware changes. Verify desktop/mobile rendering and synchronization logic.

Companion prototype implemented under companion/ with no frontend dependencies
or provider calls. Includes seven destinations, two explicitly demo profiles,
matching bedside concept, editable alarm, manual simulated acknowledgment,
offline pending state, local-revision conflict, separate sharing toggles and
unconnected speaker/assistant/Libre states. Hearth is a working name. Real
pairing and account/backend/device transport are not implemented or implied.

Model tests passed: pending changes preserve applied settings, offline cannot
acknowledge, local edits reject stale changes, profiles isolated, invalid/duplicate
proposals rejected. Chromium tests passed all seven routes at1440 and390px, dialog
Escape, actual form and acknowledgment/conflict flows, profile/private-health
presentation and no JS exceptions/viewport overflow. First browser check used
getByRole to count a closed dialog; corrected the test to inspect the DOM node,
then full run passed. Desktop/mobile/editor screenshots visually inspected,
retained under local-config/companion-preview/. No real health information used.

Preview served only on127.0.0.1:8767. Port8765 was occupied; left its process alone.
No runtime infrastructure, HA Pi or firmware changes. Next is a real paired
website-to-device alarm path; design prototype is not offline-goal completion.

## Conditional device alarm writes (2026-09-30)

Next companion prerequisite: an owner-task compare-and-save API using the snapshot
boot-session token and revision. Reject queued requests after a local edit,
runtime transition or reboot; check inside the owner, not in a web/UI thread.
Successful acknowledgement still follows NVS save. A conflict is distinct from a
storage failure and must not modify settings/audio. Session0 means startup is not
ready. No network endpoint or authentication is implied by this internal API.

Conditional-save owner API implemented and all host suites passed, including
matching durable success, stale revision after queued local edit, failed NVS
write, old boot token, and runtime command ahead of conditional save. A conflict
has a separate save_conflict flag and does not masquerade as storage damage.
The owner publishes session0 until ready; session/revision checks occur where
settings are mutated. Tokens are concurrency markers, not credentials.
IDF6.1 build passed. Receipts: conditional-owner-host.log and
conditional-owner-build.log under local-config/clock. No flash: this internal API
has no transport caller yet; installed d10c3cf remains the last qualified image.
Next transport work must add authenticated pairing and durable request/result
correlation; the existing latest-ticket snapshot alone is not sufficient for
concurrent web transactions. No website-to-device capability is claimed yet.

## Per-request save completion (2026-09-30)

Acceptance: retain a bounded set of completed save tickets so a later command
cannot hide an earlier failure/success. Store results after the owner decides
conflict or storage outcome. UI consumes its exact ticket and times out honestly
if a result is no longer available. This is an in-memory receipt cache, not a
durable replay journal or authentication. Test mixed failed/successful saves in
one owner cycle plus bounded eviction; preserve offline execution.

Implemented a16-result RAM cache, exact-ticket alarm/display UI reads, and a
10-second unavailable-result message without automatic retry or false success.
Host owner tests pass mixed failure/success completions and bounded eviction;
actual LVGL alarms-test, repeat-test, night-test and save-timeout-test pass.
IDF6.1 build passed (save-results-build.log). New preview timeout test verifies
both editors stay on their page and report unknown completion honestly.

Application-only flash attempt failed before opening USB: the stable by-id device
and /dev/serial/by-id are absent. Nothing was written. Current installed hash
remains the previously recorded d10c3cf image; conditional-save and receipt-cache
changes are built but not installed. No physical test requested. Receipts are
save-results-{host,preview-build,build,flash}.log under local-config/clock.
RAM receipts are not durable transport acknowledgments across reboot; real
pairing, authentication and website/device transport remain outstanding.
