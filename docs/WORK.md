# Work, decisions and evidence

Owner: Rob. Updated: 2026-09-23. Approved bootstrap complete. Hardware readiness active: USB connected; silicon/flash identified; factory backup verified; exact-board source/schematic needed.
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
