# Home Assistant connection

Use the existing dedicated Pi at `http://192.168.1.232:8123`. Its
`/homestead-command/home` path is a dashboard, not the API base. Do not install
Home Assistant or change the Pi configuration as part of clock development.

Open Settings → Home Assistant → Setup on the clock. The server address is
pre-filled. Supply an optional light entity such as `light.bedside` (example only) and a
Home Assistant long-lived access token. Keep the token out of chat and logs.
The token field is masked immediately. Close the keyboard with its checkmark,
then Save. An empty token preserves the saved token only for the same server.
Changing servers requires entering a token; credentials are never forwarded to
HTTP redirects. Cancel leaves saved configuration intact. Save errors remain
visible and retain the previous configuration.

Configuration is stored in the clock's private `clockcfg/ha_private` namespace,
separately from local alarms. The token is not included in UI snapshots or serial
diagnostics. Flash encryption is not currently enabled: physical flash access
can expose stored credentials, just as it can expose saved Wi-Fi credentials.

The control reads `/api/states/<entity>`, sends an explicit light `turn_on` or
`turn_off`, then reads the entity again. HTTP200 alone does not confirm a change.
Confirmation polling ends after ten seconds (a request already in progress is
bounded by HTTP timeouts). Normal refresh is ten seconds; state older than thirty
seconds cannot initiate a toggle. Unavailable, unknown, authentication failure,
wrong entity and network failure disable the light control. The local clock and
alarms remain operational. HTTP runs on the existing lower-priority network
worker, serialized with weather to limit memory use.

Verification: `python scripts/test-clock-host.py` includes parser and production
service tests with synthetic network/storage boundaries. The real LVGL preview's
`ha-test` checks masked setup, save navigation, and state-driven controls. These
checks do not claim a real light was operated. Live authenticated operation needs
a token and actual entity; no access token has been supplied or extracted.

References: [REST API](https://developers.home-assistant.io/docs/api/rest/) and
[light actions](https://www.home-assistant.io/integrations/light/).

## External media player

Settings → Media opens a separate player screen. In its Setup, select an existing
`media_player.*` entity. It shares the server/token configured in HA setup; the
light entity may now be blank when only a media player is wanted. No token is
copied into player preferences. A player is bound to the configured server address;
changing servers requires reselecting the player before controls can run there.

The screen shows reported name, track/artist, playback state and volume. It enables
only controls advertised by the player: previous/next, play/pause and volume.
Volume uses relative actions when supported, otherwise5-percentage-point explicit
volume changes if a current volume is known. Missing, stale or unavailable state
disables controls. A successful play/pause request is followed by state checks;
unchanged/failed state remains unconfirmed and polling backs off after the bounded
confirmation window. Track/volume controls display reported state, without claiming
that an accepted HTTP request proves the intended physical effect.

Manual player controls are independent of the optional remote-alarm mode described
below. Alarms use the local speaker by default. An actual token and entity are needed for live authenticated use.
No Pi configuration, speaker grouping or media playback was changed during build.

API references: [media player actions](https://www.home-assistant.io/integrations/media_player/)
and [pinned2026.9.3 feature flags](https://github.com/home-assistant/core/blob/2026.9.3/homeassistant/components/media_player/const.py).

## USB setup from the computer

Run this in an interactive terminal from the project root:

```sh
/home/rob/.espressif/tools/python/v6.1/venv/bin/python scripts/configure-clock-ha.py
```

The helper defaults to the existing server at `http://192.168.1.232:8123` and the
clock's stable USB port. It checks the clock protocol first, then prompts for a
Home Assistant token with input hidden. Never put the token in a command argument
or send it in chat. A read-only entity lookup lists available light and media-player
IDs; select either or both. Optional `--light light.example` and
`--player media_player.example` arguments skip discovery. `--url` accepts the server
root, not a dashboard path. Opening USB can reset the board; it does not power it off.

Only the clock's connection/player preferences are saved. The helper makes no
Home Assistant configuration changes and requests no playback. It confirms both
queue acceptance and persistence for each request using a matching request tag.
If setup fails partway through, an already-confirmed connection save can remain;
player selection is a separate save. Saved configuration does not establish that
a token is authorized or that a selected speaker is playing. Live state is shown
on the clock. Credentials are not written to a computer file or echoed in output.


## Start a saved selection

Media → Setup now has optional **Media ID** and **Type** fields. Supply both or
leave both blank for transport controls only. Use the ID/type supported by your
existing Home Assistant player: for example, an accessible audio stream URL and
`music`, or a provider-specific playlist ID and `playlist`. A Spotify link is not
universally playable by every Sonos/Cast/Spotify entity; use that integration's
supported identifier. The clock stores the selection with the player and HA server.
It does not download or decode the stream itself. Do not put access tokens or
credential-bearing URLs in these non-secret fields.

**Start saved** explicitly invokes `media_player.play_media` and only enables for
a fresh, available player advertising that capability. **Play** retains its resume
meaning. Saving never starts playback. A reported playing state is shown as
“Player playing; selection not verified”: a player already playing something else
is not evidence that the requested selection started. Local alarms are unaffected.
Old player preferences load with an empty selection; saving upgrades the format.

Fields are bounded to383 bytes for the ID and47 for the type. The existing USB
helper selects the player and preserves its saved selection when the same player
is selected; use the on-device Setup fields to edit the media selection itself.

Reference: [Home Assistant play specified media](https://www.home-assistant.io/actions/media_player.play_media/).

## Optional adapter boundary

The media owner now calls `media_backend.h` for connection identity, target
validation, state reads and actions. It owns queues, saved selection, freshness
and confirmation without including HA headers or knowing REST paths/JSON fields.
`media_ha.c` implements that interface, translates HA feature flags into generic
control capabilities, and uses the HA credential owner. The UI sees only generic
capabilities. HTTP work remains on the existing network worker.

HA is currently the only shipping media backend, selected by the build. This is
not a runtime plugin selector, and no direct Sonos/Spotify backend is implemented.
An alternate-backend host test links the production owner without HA/HTTP/JSON,
using a different identity/target format; it demonstrates the interface boundary,
not an actual second integration. Existing HA settings retain their format.


## Optional external alarm with local fallback

In Media → Setup, select a player and media ID/type, then enable **Use for alarms
(local fallback)**. This applies to all enabled alarms. It is off by default and
stays off when older player settings migrate. Saving the option does not play audio.
Use a dedicated alarm player: starting the selection can replace its existing media,
and Snooze/Dismiss requests pause on the player this alarm attempted to start.

At an alarm, the local alarm task starts its own eight-second deadline and exposes a
new session to the network worker. The worker requires Start saved and Pause support
before sending playback. Only fresh state from the requested player, playing the
exact media ID with reported nonzero volume and explicitly unmuted output, grants
a three-second renewable confirmation. Missing metadata, offline/auth failures,
HTTP acknowledgment alone, mismatched media, muted/unknown volume, or expiry leaves
local fallback active. Once fallback starts, it stays on for that ringing phase.
Playlist players often report the current track ID instead of the requested playlist;
those will keep the local fallback. Reported playback is not physical acoustic proof.

Snooze, Dismiss and alarm expiry cancel the session immediately in the local task.
The network worker attempts pause and reads back paused state, with at most three
attempts in a ten-second retry window; an already-running HTTP request may extend
that window. In-flight start requests cannot always be cancelled at the server,
so cleanup also runs after an ambiguous start timeout. Server identity changes
prevent commands being sent to a different server. Network failure can leave the
external player running; local Snooze/Dismiss remains usable. Serial diagnostics
report an unconfirmed remote stop without logging the target/media/token.

This behavior is host-tested with simulated players. No real authenticated alarm
playback or acoustic verification has yet been performed; that belongs in the
batched end-to-end acceptance. Local alarm scheduling never waits for HTTP.
