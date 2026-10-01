# Local wallpaper

`blue-hour-source.png` is an original AI-generated landscape created with the
built-in image-generation tool on2026-10-01. It is not the user's watermarked
reference and is not a Wallhaven download. This is one bundled offline choice,
not a substitute for the requested rotating-source feature.

`python scripts/encode-clock-wallpaper.py` converts the source into the checked-in
480x320 RGB565 `main/home_wallpaper.c`. Pillow is needed only to regenerate it.
The compiled const pixels remain in flash; LVGL uses the existing partial display
buffer. No full-resolution decode or frame allocation happens at runtime.

Wallhaven images will be fetched at runtime into the device-owned cache. They are
not bundled here or assumed to share a redistribution license.
