# Work, decisions and evidence

Owner: Rob. Updated: 2026-09-23. Approved bootstrap complete. Hardware readiness active: IDF6.1 display-touch installed; I2C, screen rendering and aligned touch/CLEAR verified; cold-start power/audio still outstanding.
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

## Active bounded task: hardware readiness

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
