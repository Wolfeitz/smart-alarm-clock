#!/usr/bin/env python3
"""Exercise production JPEG adapter + pinned LVGL decoder, without a device."""
from pathlib import Path
import os
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
main = root / 'firmware/clock/main'
lvgl = root / 'firmware/clock/managed_components/lvgl__lvgl'
with tempfile.TemporaryDirectory(prefix='esp-link-background-') as d:
    binary = str(Path(d) / 'decoder-test')
    subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
        '-fsanitize=address,undefined', '-g',
        '-DLV_CONF_PATH="' + str(root / 'firmware/clock/preview/lv_conf.h') + '"',
        '-I' + str(main), '-I' + str(lvgl),
        str(root / 'firmware/clock/tests/background_decode_test.c'),
        str(main / 'background_decode.c'), str(main / 'background_jpeg.c'),
        '-o', binary], check=True, timeout=60)
    # LeakSanitizer cannot run under the sandbox's ptrace; ASan/UBSan stay enabled.
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
    for shape in ('wide', 'tall', 'tiny'):
        subprocess.run([binary, str(root / ('firmware/clock/tests/fixtures/backgrounds/red-' + shape + '.jpg'))],
                       check=True, timeout=10, env=env)
