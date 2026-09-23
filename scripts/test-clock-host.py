#!/usr/bin/env python3
"""Compile and run clock logic checks; this does not qualify hardware."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
main = root / "firmware/clock/main"
tests = root / "firmware/clock/tests"
suites = {
    "rtc_codec": ["rtc_codec"],
    "local_time": ["local_time"],
    "alarm_engine": ["alarm_engine"],
    "settings_codec": ["alarm_engine", "settings_codec"],
    "alarm_recovery": ["alarm_engine", "settings_codec", "alarm_recovery"],
}
with tempfile.TemporaryDirectory(prefix="esp-link-host-") as directory:
    for name, modules in suites.items():
        binary = str(Path(directory) / name)
        subprocess.run([
            "cc", "-std=c11", "-D_POSIX_C_SOURCE=200809L", "-Wall", "-Wextra", "-Werror",
            "-I" + str(main), *[str(main / (m + ".c")) for m in modules],
            str(tests / (name + "_test.c")), "-o", binary,
        ], check=True, timeout=60)
        subprocess.run([binary], check=True, timeout=10)
print("PASS host logic only; display, touch, audible sound and power loss require device checks")
