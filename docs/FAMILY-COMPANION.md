# Family companion direction

Owner direction recorded 2026-09-30. Architecture proposal and delivery plan;
not an assertion that these components or integrations are implemented.

## Experience

A household website used comfortably from phone or desktop manages people,
paired devices, alarms, speaker selection, routines and optional connections.
Each bedside device belongs to a person or shared room. Its home screen presents
that person's selected information with a consistent visual design. Routine
configuration moves off the small touchscreen; time, alarm editing, snooze,
dismiss and brightness remain available locally without the website or network.

Suggested main website destinations: Today, People & devices, Alarms & routines,
Music, Connections and Privacy. Device destinations: Home, Alarms, Music and a
small Settings area. Optional glucose and assistant cards appear only when
configured for that person. Device pairing uses a short-lived code displayed on
the actual device, with explicit reassignment/revocation.

## Ownership and synchronization

The device remains the authority for executing persisted alarms. The companion
service manages identities, household membership, optional integrations and
configuration delivery. A website save is first a pending request; it becomes
applied only after the device validates, persists and acknowledges its revision.
Offline devices remain visibly pending. Local edits increment the same device
revision; stale website edits produce a conflict rather than overwriting them.
Retries carry an idempotency key. Reconnecting must not replay expired snooze,
dismiss, sound-test or playback commands. A network request never enters the
alarm owner's timing path.

A household administrator may manage device assignment without automatically
seeing private health readings or assistant conversations. Per-person connections
and explicit sharing grants control what each device and family member receives.
The server holds provider secrets; devices receive scoped credentials and only
the data needed for their assigned profile. Do not bake Rob's accounts or HA
address into the public product defaults.

A companion service is justified by family identity, consent, provider credentials
and integration work beyond the ESP32. Deployment location/storage selection still
requires live infrastructure discovery under the machine contract; no new shared
service or HA Pi installation is part of this design update. HA remains an optional
adapter, not the identity system or a prerequisite for owning a device.

## ChatGPT, Dot and Pet

Verified official documentation distinguishes a Dot (cloud agent with connected
plugins) from a Pet (visual presence and activity controls). Proposed ChatGPT
integration: expose narrow authenticated companion operations through an MCP
plugin, such as list my devices, inspect my alarms, propose/update an alarm, and
retrieve explicitly authorized summaries. All operations pass through the same
permissions, validation and device acknowledgments as the website.

No direct embedded Dot session/memory API has been verified. Do not label a
separate API assistant as the user's existing Dot or assume automatic access to
ChatGPT history. A device avatar can reflect actual companion states, but must
not pretend to show live Dot activity without a supported event source. Existing
pet artwork portability/format and account availability also need validation.

A conversational voice prototype can begin in the companion website using the
supported OpenAI voice APIs, with keys on the server. This is distinct from a
ChatGPT Dot integration and has its own usage/account requirements. ESP32 microphone
capture, streaming, echo handling and interruption behavior remain hardware work;
no working voice pipeline is claimed. Alarm scheduling never depends on a model.

Sources inspected September30:
- https://learn.chatgpt.com/docs/dots
- https://learn.chatgpt.com/docs/pets
- https://developers.openai.com/api/docs/guides/live

## Libre

First proposed slice: read-only glucose value, original units, trend, measurement
time, receipt time, source and explicit fresh/stale/unavailable state. Never replace
missing readings with zero or silently show old readings as current. A household
administrator cannot infer permission to see another member's readings. Sharing
and allowing assistant access are separate choices, off until granted.

Abbott documents LibreLinkUp family sharing, connectivity dependence and regional
variation. The pages reviewed do not establish a public supported API for this
application. Verify sensor/app version, region and the user's existing data path;
then evaluate a supported integration or an explicitly identified community bridge.
Do not embed Libre passwords in firmware, invent an Abbott API contract, or claim
an unofficial connector is Abbott-supported. Display is supplementary; retain the
manufacturer's app/alarms as the primary monitoring path. No dosing, treatment
recommendations or replacement glucose alarms in this initial slice.

Source inspected September30: https://www.librelinkup.com/

## Audio and design

External speakers are now a primary requested alarm/music experience; the onboard
tone is insufficient for the owner's wake-up needs. Speaker model remains pending.
Provide friendly device selection, alarm-specific volume, music favorites and
observed playback feedback. Network-dependent sound does not establish a loud
offline fallback; local hardware output remains a separate unresolved requirement.

Use a shared visual system across website and device: readable hierarchy, clear
primary actions, restrained color, consistent spacing, understandable status and
empty/error states. Keep entity IDs and raw integration fields behind advanced
setup. The existing firmware is a functional foundation; do not keep adding dense
menus and call it the final interface.

## Next delivery slice

1. Build a cohesive interactive companion/device design using clearly labeled
   synthetic data, including one-person and family cases, pairing, alarm editing,
   speaker setup and private glucose presentation. No real account connection.
2. Implement a vertical path: pair one device, edit one alarm on the website,
   acknowledge durable application on the board, show local edits and conflicts,
   and demonstrate continued local operation with the companion unavailable.
3. Add actual speaker selection/playback and the supported ChatGPT tool bridge.
4. Qualify the Libre data path and permission model, then connect the read-only card.
5. Add the verified Dot/Pet interaction and voice path appropriate to the owner's
   intended product, then conduct the batched final physical acceptance.

Pending questions are the intended Dot/Pet experience, Libre display/sharing
scope and existing speaker model. They do not block design or the local alarm
configuration path. No services were installed and no accounts were connected by
this document.
