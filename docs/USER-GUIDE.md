# Using the clock

## Navigation

The bottom navigation icons are house (Clock), bell (Alarms), sun/cloud (Weather),
and gear (Settings); the current page is highlighted. The home weather card opens the forecast, and the next-alarm
row opens alarms (or active snooze controls). The half-lit lightbulb controls brightness.
Editors keep explicit Save/Cancel actions instead of the navigation bar.

## Alarms

Tap **Alarms** on the home screen. The list shows eight slots with their time,
ON/OFF state and schedule. Scroll to see the remaining slots; tap a row to edit it.

Select the hour and minute (24-hour time), then choose **Every day**, **Weekdays**,
**Weekends**, **Custom** days, or **Once** and a date. Tapping individual days
updates the repeat choice to match your selection. Turn the top-right switch on to enable the alarm.
Tap **Save**. The overview returns after the settings write succeeds. **Cancel**
leaves the saved alarm unchanged. You can also select another slot from the editor's
Alarm dropdown; that discards edits that have not been saved.

When an alarm rings, **Snooze 5 min** silences it for five minutes. **Dismiss** ends
the active alarm, including a snoozed alarm. These controls apply to all currently
active alarms. A once alarm disables itself after firing; recurring alarms remain
enabled for their next scheduled day. Unattended ringing ends after ten minutes.

During snooze, **Show clock** returns to normal clock navigation without changing
the countdown. The home alarm card shows the remaining time; tap it to reopen
**Dismiss**. Ringing automatically brings the alarm controls back to the front.

The display and touch mapping are rotated 180 degrees from the earlier prototype
to match the owner-selected enclosure orientation.

## Time and screen brightness

**Settings → Time & date** sets local time and writes the RTC. Network time also
updates the RTC when available. Local time and alarm execution continue without
Wi-Fi. Complete loss of device power is different from loss of Wi-Fi; RTC retention
across that event still depends on the board's backup-power arrangement.

The home **half-lit lightbulb icon** changes manual brightness. Under
**Settings → Display & night mode**, choose a local-time dim interval and enable
the schedule. Touch temporarily brightens the screen for thirty seconds. Ringing
alarms brighten the screen. When scheduling is enabled, the home shortcut opens
these display settings instead of toggling manual brightness.

## Weather and connection

Tap the top-right Wi-Fi indicator to open Wi-Fi settings; Back returns to Clock.
Three Wi-Fi arcs show strong signal, two good, one weak (associated AP RSSI, sampled
about every five seconds when the network worker is free). No lit arcs means signal
is unavailable or Wi-Fi is disconnected. The Internet status reports the age of
a successful HTTPS weather/location request; it is not a continuous Internet
probe. Failed requests or disconnection clear verification, and old results
expire after31minutes. Audio, saved-settings and invalid-time warnings take
priority over connectivity status.


Tap **Settings → Wi-Fi** to scan nearby networks and connect. Weather retains
a Wi-Fi shortcut. Wi-Fi and Location return to the page you opened them from. **Turn off**
keeps Wi-Fi off across restarts, preserving your saved network and password.
**Turn on** reconnects to that saved network; tap Scan to look for others. Saving
credentials while off does not silently turn Wi-Fi back on. Local time, alarms
and brightness remain available offline. Location is
separate: **Location** selects the ZIP used for weather/timezone. A manually selected
ZIP takes precedence over network-based estimation. The current manual location is
27358. A timezone change may require the restart offered on the weather screen.

## Home Assistant

**Settings → Home Assistant** opens the light control. **Setup** uses the existing
Pi server address. See [connection setup](HOME-ASSISTANT.md) for token and entity
configuration. An unavailable or stale connection disables the light control; it
does not disable local alarms. No Home Assistant server is installed by the clock.

## External media

**Settings → Media → Setup** selects a player already available in Home Assistant.
It uses the saved HA server/token and shows track, artist, state and volume. Only
supported controls are enabled. The controls operate existing playback. A saved media ID/type can also start a
selection; optional alarm use is described below. There is no Spotify playlist
browser or direct Spotify streaming on the clock.

## Status messages

**Local audio unavailable** means the firmware reported an audio initialization or
output error. A ringing alarm then shows **Audio error**, while Snooze/Dismiss remain
available. **Settings storage error** means persistence failed; do not interpret a
queued save as a successful save. **Saved alarms unavailable** means loading the
stored configuration failed. Automatic writes are suppressed to preserve the
stored data. The alarm editor warns before an explicit Save replaces the previous
configuration. The current speaker is audible but has not met the owner's wake-up
volume requirement. External alarm playback is optional and requires setup.

For Home Assistant setup without typing a long token on the touchscreen, use the
[computer USB setup helper](HOME-ASSISTANT.md#usb-setup-from-the-computer). It offers
light/player entity discovery and hidden token entry in your terminal.

Media Setup can also save a media ID and type. **Start saved** starts that selection
on the configured external player; **Play** resumes its existing media. Saving
settings does not start playback. Support depends on the selected HA integration.

Media Setup also offers **Use for alarms (local fallback)**, off by default. With
a saved player/selection, alarms can request that external player; missing or
unconfirmed playback falls back to the board speaker after eight seconds. See
[remote alarm behavior and limits](HOME-ASSISTANT.md#optional-external-alarm-with-local-fallback).

Tap the date on the clock to open an offline month calendar. Arrows browse months;
Today returns to the current month. This shows dates only; external calendar events
are not connected. Alarms still appear over this view.

The home weather tile now includes condition artwork. Faded artwork with
“Outdated” means cached weather, not a fresh observation. Wi-Fi settings show
**Connected** signal strength and channel above the list; **Best nearby** is the
strongest scanned access point for each network name, which may be a different
radio or access point. Both use Strong at -60 dBm or better, Good from -75 dBm,
and Weak below -75 dBm. Connection/reconnection now scans all channels and sorts
compatible access points by signal. This does not implement continuous roaming.

## Background sources

Under **Settings → Backgrounds**, choose the bundled **Local: Blue hour** image,
**Selected image links** (one HTTPS JPEG URL or Wallhaven wallpaper-page link per
line, up to eight), or **Wallhaven search**. Leave search blank for images matching
just your Advanced filters; enter a search to narrow the selection.

**Advanced** exposes General/Anime/People and SFW/Sketchy/NSFW checkboxes,
sort/order and toplist period, source resolution and aspect ratios, color and random
seed, fill/crop versus fit/borders or stretch, custom rotation seconds, retry delay
and count, and refresh/error notifications. Scroll the panel for additional controls.
Existing General/SFW choices are preserved on upgrade and can now be changed.
Wallhaven requires an API key for NSFW; SFW and Sketchy searches need none. Enter
an optional key in the masked field; blank retains it, Remove saved key clears it.
The key is stored separately from image settings and sent only to the Wallhaven
API, never image hosts. It is not encrypted at rest by this feature.
Resolution filters select source images; output remains 480×320.
Choose Keep fixed, 5 minutes, 15 minutes, or an hour; tap Save. The screen shows
Saving, then returns to the clock after settings are stored. A failed save stays
on the settings screen with your entries intact. Image downloading happens in the
background and does not hold you on that screen. Next image requests
another image using the saved settings. A failed download keeps the visible image.
Wallhaven thumbnails use your selected positioning; some images suit this
small screen better than others. Direct links currently require baseline JPEG,
up to2MiB and4096pixels per axis; unsupported images leave the previous one visible.

Source settings survive restart. Downloaded image pixels are currently cached in
RAM only: during an outage the current image stays, but after a restart the bundled
local image appears until a download succeeds. Importing personal local files and
persistent downloaded-image caching are still being built; the current Local
choice is the bundled image only. Background rotation never requires HA/a server.

## Indoor readings and shake-to-snooze

The weather card includes the onboard temperature and relative humidity. These
come from the SHTC3 inside the case, not the weather provider, and work offline.
The electronics can warm the sensor considerably; this is not yet a calibrated
room-temperature measurement. Readings older than30seconds are marked stale.

**Settings → Sensors & gestures** shows the readings and motion-sensor status.
**Shake to snooze** is off by default. Enable it and tap Save to persist the choice.
When an alarm rings, a deliberate series of shakes sends the same five-minute
snooze request as the touchscreen. It does not dismiss alarms. Gestures are ignored
when disabled or no alarm is ringing, with a startup delay and cooldown to avoid
single bumps/repeated triggers. Detection thresholds are host-tested and live
accelerometer sampling is verified; real hand-motion sensitivity is not yet tuned.
Automatic screen rotation is separate and is not yet installed.
