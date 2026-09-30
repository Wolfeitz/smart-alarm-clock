# Family companion interaction prototype

Hearth is a working name. This is a local design/interaction prototype, not the
running clock's control panel. All devices, weather, readings and acknowledgments
are simulated. Refresh resets the in-memory demo. No provider requests, credentials,
cookies, analytics or external assets are used.

Run from the repository root:

    python -m http.server 8767 --bind 127.0.0.1 --directory companion

Open http://127.0.0.1:8767. If the port is occupied, choose another unused local
port; do not stop an unrelated service. Binding to loopback intentionally keeps
this unauthenticated prototype off the household network.

Try Edit alarm, then simulate acknowledgment. Under Prototype controls, take the
device offline before editing: applied settings stay unchanged. Bring it online
to acknowledge, or simulate a local edit before acknowledgment to exercise the
conflict path. Switch demo profiles to see separate alarms and sharing choices.
Libre, ChatGPT/Dot and music screens describe their unconnected state. The CSS
character is an illustrative mascot, not imported OpenAI Pet artwork.

Checks:

    node --test companion/model.test.mjs
    node --check companion/app.mjs

With Playwright available and the server running:

    node companion/browser.test.mjs

If Playwright is supplied externally, set PLAYWRIGHT_MODULE to its index.mjs
absolute path. CHROMIUM overrides /usr/bin/chromium; PREVIEW_URL overrides the
loopback URL; PREVIEW_OUTPUT overrides /tmp/esp-link-companion. The browser test
checks desktop/mobile routes, dialog cancel, pending/applied/conflict semantics,
profile isolation and private-by-default synthetic readings, and captures PNGs.

There is no real pairing, authentication, durable server storage, device transport,
speaker playback, assistant session or glucose connection. The state machine is
an executable interaction contract; it is not a production sync protocol or
security boundary. Next is the authenticated single-device configuration path,
with device-side validation and persisted revision acknowledgment. Installed
firmware and physical-acceptance requirements are unchanged by this prototype.
