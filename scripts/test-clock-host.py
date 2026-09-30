#!/usr/bin/env python3
"""Compile and run clock logic checks; this does not qualify hardware."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
main = root / "firmware/clock/main"
tests = root / "firmware/clock/tests"
suites = {
    "audio_control": ["audio_control"],
    "display_policy": ["display_policy"],
    "alarm_output": ["alarm_output"],
    "rtc_codec": ["rtc_codec"],
    "local_time": ["local_time"],
    "alarm_engine": ["alarm_engine"],
    "settings_codec": ["alarm_engine", "settings_codec", "display_policy"],
    "alarm_restart": ["alarm_engine", "settings_codec", "alarm_recovery", "display_policy"],
    "alarm_recovery": ["alarm_engine", "settings_codec", "alarm_recovery", "display_policy"],
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
    binary = str(Path(directory) / "radio_preferences")
    subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-I" + str(tests / "storage_stubs"), "-I" + str(main),
        str(main / "radio_preferences.c"), str(tests / "radio_preferences_test.c"),
        "-o", binary], check=True, timeout=60)
    subprocess.run([binary], check=True, timeout=10)
    binary = str(Path(directory) / "settings_store")
    subprocess.run([
        "cc", "-std=c11", "-D_POSIX_C_SOURCE=200809L", "-Wall", "-Wextra", "-Werror",
        "-I" + str(tests / "storage_stubs"), "-I" + str(main),
        *[str(main / (m + ".c")) for m in ("settings_store", "settings_codec", "alarm_engine", "display_policy")],
        str(tests / "settings_store_test.c"), "-o", binary,
    ], check=True, timeout=60)
    for scenario in ("init", "open", "read", "corrupt", "write", "commit", "reload"):
        subprocess.run([binary, scenario], check=True, timeout=10)
    binary = str(Path(directory) / "media_backend")
    subprocess.run([
        "cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-I" + str(tests / "ha_stubs"), "-I" + str(main),
        str(main / "media_model.c"), str(main / "media_service.c"), str(main / "alarm_output.c"),
        str(tests / "media_backend_test.c"), "-o", binary,
    ], check=True, timeout=60)
    subprocess.run([binary], check=True, timeout=10)
    binary = str(Path(directory) / "alarm_service")
    subprocess.run([
        "cc", "-std=c11", "-D_POSIX_C_SOURCE=200809L", "-Wall", "-Wextra", "-Werror",
        "-I" + str(tests / "alarm_stubs"), "-I" + str(tests / "ha_stubs"), "-I" + str(main),
        *[str(main / (m + ".c")) for m in ("alarm_service", "alarm_output", "alarm_engine", "alarm_recovery", "settings_codec", "display_policy")],
        str(tests / "alarm_service_test.c"), "-o", binary,
    ], check=True, timeout=60)
    for scenario in ("checkpoint", "edit", "invalid-brightness", "invalid-dismiss", "invalid-edit", "conditional-save", "conditional-conflict", "conditional-storage", "conditional-session", "conditional-runtime"):
        subprocess.run([binary, scenario], check=True, timeout=10)
    binary = str(Path(directory) / "remote_alarm")
    subprocess.run([
        "cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-I" + str(tests / "ha_stubs"), "-I" + str(main),
        *[str(main / (m + ".c")) for m in ("remote_alarm", "alarm_output", "media_model")],
        str(tests / "remote_alarm_test.c"), "-o", binary,
    ], check=True, timeout=60)
    subprocess.run([binary], check=True, timeout=10)
    cjson = root / "firmware/clock/managed_components/espressif__cjson/cJSON"
    binary = str(Path(directory) / "weather_model")
    subprocess.run([
        "cc", "-std=c11", "-D_POSIX_C_SOURCE=200809L", "-DCJSON_NESTING_LIMIT=16", "-Wall", "-Wextra", "-Werror",
        "-I" + str(main), "-I" + str(cjson), str(cjson / "cJSON.c"),
        str(main / "weather_model.c"), str(main / "timezone_rules.c"), str(tests / "weather_model_test.c"), "-lm", "-o", binary,
    ], check=True, timeout=60)
    subprocess.run([binary, str(tests / "fixtures/weather-location.json"), str(tests / "fixtures/weather-forecast.json")], check=True, timeout=10)
    binary = str(Path(directory) / "ha_model")
    subprocess.run([
        "cc", "-std=c11", "-DCJSON_NESTING_LIMIT=16", "-Wall", "-Wextra", "-Werror",
        "-I" + str(main), "-I" + str(cjson), str(cjson / "cJSON.c"),
        str(main / "ha_model.c"), str(tests / "ha_model_test.c"), "-o", binary,
    ], check=True, timeout=60)
    subprocess.run([binary], check=True, timeout=10)
    binary = str(Path(directory) / "ha_service")
    subprocess.run([
        "cc", "-std=c11", "-DCJSON_NESTING_LIMIT=16", "-Wall", "-Wextra", "-Werror",
        "-I" + str(tests / "ha_stubs"), "-I" + str(main), "-I" + str(cjson), str(cjson / "cJSON.c"),
        str(main / "ha_model.c"), str(main / "ha_service.c"), str(tests / "ha_service_test.c"), "-o", binary,
    ], check=True, timeout=60)
    subprocess.run([binary], check=True, timeout=10)
    for fault in ("mutex-failure", "queue-failure"):
        subprocess.run([binary, fault], check=True, timeout=10)
    for name, modules in {"setup_model": ["setup_model", "ha_model"], "media_model": ["media_model", "media_ha", "ha_model"], "media_service": ["media_model", "media_service", "media_ha", "ha_model", "alarm_output"]}.items():
        binary = str(Path(directory) / name)
        subprocess.run([
            "cc", "-std=c11", "-DCJSON_NESTING_LIMIT=16", "-Wall", "-Wextra", "-Werror",
            "-I" + str(tests / "ha_stubs"), "-I" + str(main), "-I" + str(cjson), str(cjson / "cJSON.c"),
            *[str(main / (module + ".c")) for module in modules], str(tests / (name + "_test.c")), "-lm", "-o", binary,
        ], check=True, timeout=60)
        subprocess.run([binary], check=True, timeout=10)
        if name == "media_service":
            for fault in ("mutex-failure", "queue-failure", "legacy", "selection-reload"):
                subprocess.run([binary, fault], check=True, timeout=10)
print("PASS host logic only; display, touch, audible sound and power loss require device checks")
