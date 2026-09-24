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

This is manual player control only. Alarms still use the independent local speaker;
remote alarm playback, playlist/context selection and fallback integration are not
yet implemented. An actual token and entity are needed for live authenticated use.
No Pi configuration, speaker grouping or media playback was changed during build.

API references: [media player actions](https://www.home-assistant.io/integrations/media_player/)
and [pinned2026.9.3 feature flags](https://github.com/home-assistant/core/blob/2026.9.3/homeassistant/components/media_player/const.py).
