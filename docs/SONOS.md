# Direct Sonos integration

Status2026-10-02: core, transport and media routing implemented and tested.
Setup/favorites UI is implemented and installed. Actual ESP32-to-simulator
HTTP/SOAP lookup, favorites, group/malformed refusal and recovery pass, alongside
localhost protocol, adapter and combined alarm tests. Real-speaker audio remains
unqualified.

The clock is a local controller. Sonos fetches/plays the selected music; the clock
does not stream Spotify audio, and HA/cloud control is not mandatory. Both devices
must be reachable on the same LAN. Internet-dependent media still needs Internet;
local alarm fallback is independent of both LAN and media-provider availability.

## Implementation contract

Add a selectable Sonos backend alongside existing optional HA. Preserve previously
saved HA/media settings; do not activate or command an unselected target. Use a
bounded local UPnP/SOAP client with validated device identity, current transport
state, volume/mute, play/pause/stop and saved-content triggering. Provide device
selection/setup and useful failure/status feedback. Prefer selecting Sonos favorites
configured through the Sonos app for playlists; avoid inventing Spotify URI-to-DIDL
mappings or requiring account credentials on the clock. Group coordinator handling
must either be implemented and verified or explicitly rejected, never silently
control an unrelated room. Pin device identity so IP reuse cannot retarget alarms.

Existing media-service/remote-alarm boundaries remain: all network operations on a
network owner, independent local fallback deadline, no HTTP acknowledgment treated
as audible proof, stale state expires, cancellation checked around blocking actions,
late playback cleanup bounded and reported when unconfirmed. Stop must be attempted
after timeout because a request may have reached the speaker. Never weaken the
local fallback just to make a simulator show success.

## Verification contract

A protocol-faithful local simulator accepts actual HTTP requests and XML, models
transport/volume/favorites and injects SOAP faults, malformed/oversized responses,
timeouts and delayed playback. Exercise the production C parser/controller rather
than a separate Python reimplementation. Verify discovery/setup identity, command
payloads, escaping, state observation, target/selection matching, cancellation and
local fallback behavior. Run actual device-to-simulator traffic when accessible,
without replacing user preferences permanently or arming ordinary daily alarms.

Simulator results prove the tested protocol/control behavior only. They cannot
prove Era100 compatibility, actual sound, wake-up volume, Sonos account permissions,
Spotify subscription/catalog behavior, group topology variants, or campus LAN
reachability. Those limitations do not block completing the controller and tests.

## Research sources

[SoCo core](https://github.com/SoCo/SoCo/blob/master/soco/core.py) is a maintained
primary implementation of local Sonos UPnP transport, queue and rendering controls.
Its coordinator-only methods highlight why group targeting needs explicit handling.
[Sonos Control API](https://docs.sonos.com/docs/control) is a different cloud API;
do not mistake its HTTP endpoints for the local UPnP protocol or silently require
cloud credentials/HA for this standalone device.

## Core protocol progress

Production `sonos_client.c` now probes/pins UDN, checks standalone topology, reads
transport/volume/mute/current URI and issues play/pause/stop/previous/next/volume
and explicit-URI selection. Calls have a cancellation callback between network
operations. Each control operation rechecks identity/topology. Grouped and bonded
multi-member targets are rejected in this first implementation, not regrouped.
Identity pinning prevents accidental IP reuse; unencrypted local UPnP is not
cryptographic device authentication.

`sonos_xml.c` uses Expat with bounded input/depth/output fields, rejects DTDs,
SOAP faults and duplicate selected fields, and decodes nested/escaped XML safely.
Target dependency pinned to Espressif Expat2.8.1. Host tests currently use the
installed Expat development library (2.8.5 on this host), not the target library
binary. No new host package installed. ESP-IDF target build checks the pinned
component separately. Target parser allocations prefer external RAM.

Run `python scripts/test-sonos-protocol.py` for actual localhost HTTP traffic into
a simulator using the production C shared library. It compiles with the host C
compiler and `-lexpat`; localhost bind may require sandbox approval. The temporary
HTTP server terminates at test exit; no service is installed and no real speaker
or HA endpoint is contacted. Tests cover identity/target formatting, transport,
volume, XML escaping, grouped/changed-device refusal, faults, malformed/oversized
responses, cancelled selection before Play and late playback followed by Stop.

The production ESP-IDF HTTP callback is now implemented in `sonos_network.c`
and `network_http_sonos`, using the existing shared network mutex. It sets SOAP
headers, accepts only private numeric endpoints and fixed Sonos paths, sends no
HA or Wallhaven credentials, and disables redirects. A ten-second operation
deadline is checked before each request and on response events; each socket wait
is capped at two seconds or the remaining budget. This is not a proven strict
wall-clock bound for all ESP-IDF internals (for example, partial-header delivery).
Overflow/expired events explicitly close the socket because ESP-IDF ignores data
callback return values. SDK-boundary tests cover this behavior and lock/cleanup
paths. The large controller remains caller-owned and must live in external RAM.

Media routing now selects HA or Sonos from the saved target. Sonos requires no
HA endpoint or token; its IP/port plus pinned UUID live in the existing target
field, with a separate connection identity. Existing HA schema/migrations remain.
The production Sonos media adapter is exercised over HTTP by the simulator, using
a host transport boundary; the ESP-IDF transport has separate SDK-boundary tests.
Explicit URI selection currently uses type `uri`. Favorites now resolve fresh Sonos metadata at playback time.
Remote alarm starts pass a session-cancellation callback into multi-request Sonos
operations; cleanup uses Sonos Stop and remains bound to the original target.
HA retains its existing pause cleanup. Local fallback confirmation remains strict.

Setup UI and combined production alarm-to-HTTP simulator scenarios now pass.
Actual device-to-simulator lookup/favorites, rejection and recovery checks pass;
see WORK for the approved temporary firewall access and corrected stack/memory
findings. Board checks performed no playback or preference writes. Playback and
alarm control paths have host simulator evidence, not real-speaker audio proof.
SoCo reference revision:
18effdc21312fa6e9a3c87e01741632275c3b481.

## Favorites protocol

Browse reads six favorites at a time from `FV:2`, retaining stable IDs and decoded
titles. Playback browses the selected favorite ID again, validates the returned ID,
resource URI/protocol and nested `resMD`, and adds the resource to that reference
metadata as in pinned SoCo. Service descriptors are preserved. Malformed, duplicate,
missing or oversized fields reject the selection instead of guessing.

Radio schemes use SetAVTransportURI; queueable selections append with AddURIToQueue,
select the device queue, seek to FirstTrackNumberEnqueued, then Play. Existing queue
contents are not cleared. Cancellation can leave appended tracks without playing;
there is no unsafe attempt to delete a potentially changed user queue. HTTP simulator
covers Browse pagination, escaping, resource/service metadata, append/seek, radio,
missing metadata and cancellation. It does not prove any real Spotify/Sonos account.

A queue URI does not prove which favorite is playing. Existing exact-content alarm
confirmation deliberately cannot suppress local fallback for these queue favorites
until matching evidence exists; remote music may accompany the local alarm.
No API acknowledgement is treated as proof that anyone can hear the speaker.

## Device verification

`check-sonos-device.py --bind <host LAN IP> --http-port 18400 --port <USB path>`
starts a temporary simulator and asks the real clock to look up its identity and
favorites, reject grouped/malformed responses and recover. It neither saves a media
target nor plays anything, and verifies alarm records remain unchanged. Host ingress
must permit the connection. Do not modify shared firewall policy without approval.
The server terminates on test exit. A successful localhost test is not equivalent
to this device transport test.
