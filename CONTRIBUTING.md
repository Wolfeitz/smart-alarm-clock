# Contributing

Thanks for helping build a better morning.

Start with the README, firmware/clock/README.md and docs/USER-GUIDE.md. Use ESP-IDF
6.1 and the checked-in component lockfile. The Waveshare ESP32-C5-Touch-LCD-3.5-C
is the current target; other boards need explicit pin, power and driver work.

## Changes

1. Open an issue for substantial architectural or hardware changes.
2. Keep alarm scheduling, local sound, snooze, dismiss and settings independent of
   network services. Network work must never block UI or local alarm execution.
3. Keep logic outside LVGL callbacks. Use service commands and snapshots.
4. Run focused checks and the relevant host suites from docs/TESTING.md. Report
   exactly what was tested: host, UI renderer, simulator or actual hardware.
5. Include screenshots for UI changes, and update the user guide for behavior changes.

A PR should explain the problem, behavior change and validation. Hardware access
is useful but not required for logic, tests and documentation contributions.
Never claim a build proves audible playback or real-speaker compatibility.

## Bug reports

Include board/revision, firmware commit, reproduction steps, expected/actual result,
and sanitized diagnostics. For network failures, describe the topology without
posting Wi-Fi passwords, tokens, private URLs or full flash dumps.

## Licensing

Contribute original work or clearly attributed, license-compatible material.
Contributions to original project files are under the repository's MIT license.
Preserve upstream notices. Do not upload downloaded wallpapers without permission.

## Credentials and local files

Keep secrets, build outputs and hardware backups in ignored local-config/.
The unrelated local install-arch.sh file is not project setup. Do not run it.
