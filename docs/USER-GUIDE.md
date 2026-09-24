# Using the clock

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

## Time and screen brightness

**Settings → Time & date** sets local time and writes the RTC. Network time also
updates the RTC when available. Local time and alarm execution continue without
Wi-Fi. Complete loss of device power is different from loss of Wi-Fi; RTC retention
across that event still depends on the board's backup-power arrangement.

The home **Dim / Brighten** button changes manual brightness. Under
**Settings → Display & night mode**, choose a local-time dim interval and enable
the schedule. Touch temporarily brightens the screen for thirty seconds. Ringing
alarms brighten the screen. When scheduling is enabled, the home shortcut opens
these display settings instead of toggling manual brightness.

## Weather and connection

Tap **Weather**, then **Wi-Fi** to scan nearby networks and connect. Location is
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
supported controls are enabled. This controls existing playback; choosing a new
Spotify playlist and using remote playback as an alarm are not implemented yet.

## Status messages

**Local audio unavailable** means the firmware reported an audio initialization or
output error. A ringing alarm then shows **Audio error**, while Snooze/Dismiss remain
available. **Settings storage error** means persistence failed; do not interpret a
queued save as a successful save. The current speaker level is the provisionally
accepted local tone; external playback is not yet an alarm source.

For Home Assistant setup without typing a long token on the touchscreen, use the
[computer USB setup helper](HOME-ASSISTANT.md#usb-setup-from-the-computer). It offers
light/player entity discovery and hidden token entry in your terminal.
