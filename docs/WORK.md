# Work, decisions and evidence

Owner: Rob. Updated: 2026-09-23. Approved bootstrap complete. Hardware readiness: receipt identification recorded; technical discovery pending.
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
| Firmware build / device tests | Not run; no firmware/toolchain established |

Discovery evidence and input SHA-256 values are in the approved proposal. Current
shell sandbox previously failed to launch with `mountinfo path is not absolute`;
reviewed escalation worked. Do not disable global sandbox controls to fix this task.

## Next bounded task: hardware readiness (receipt identification recorded)

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
