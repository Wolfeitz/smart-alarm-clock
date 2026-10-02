# Direct Sonos integration

Status2026-10-02: implementation pending. Existing media backend is HA only;
this document defines the next accepted scope, not a working feature claim.

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
