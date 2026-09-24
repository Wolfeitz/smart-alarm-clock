# Weather and Wi-Fi example

The current example uses US ZIP codes and Fahrenheit. The owner's manually
supplied 27358 is seeded as Summerfield, North Carolina, America/New_York.

- **Weather → Wi-Fi** scans nearby access points. Choose a network, enter its
  password on the device, close the keyboard with its checkmark, then Connect.
  Show/Hide changes password visibility only on the device. Passwords are never
  returned through diagnostics. Open and WPA2/WPA3 Personal networks are supported;
  enterprise, WEP and captive portals are not implemented.
- **Weather → Location** edits weather location independently of Wi-Fi. A manual
  ZIP remains in place when networks change. **Use network** explicitly clears
  that manual selection and requests an approximate public-IP ZIP suggestion.
  Check the suggestion and use Save location to turn it into a manual choice.
  Failed/non-US/missing postal results require manual entry; no ZIP is fabricated.
- A resolved location selects an IANA timezone. Its current/future POSIX DST rules
  come from a generated IANA2026d table. When changing to a different zone, Apply
  zone restarts the clock, avoiding concurrent process-wide TZ changes while the
  alarm scheduler is running. An active ringing/snoozed alarm blocks that restart.
  A pending/unresolvable location retains the previous timezone across boots.
- Weather refreshes every30 minutes, retrying failures after60 seconds. Refresh
  requests an earlier update. RAM-cached weather is explicitly marked outdated
  after1 hour (or when its observation exceeds2 hours). Cache is lost at reboot.
  The screen labels forecast dates; old data is not relabeled as today's forecast.
- Network time is handed to the UI/RTC owner for durable RTC updates. Alarms keep
  using local RTC/system time without Internet. Network work owns no LVGL or I2C.

## Ownership and extension points

`weather_model` defines provider-neutral location/current/daily snapshots and
validates external data. `weather_service` owns radio, bounded HTTPS, retries and
its project-owned NVS configuration; `clock_ui` consumes copied snapshots. The
alarm service remains independent. Later Home Assistant or spoken briefings can
consume the snapshot or provide their own service snapshot without taking over
the alarm loop or placing HTTP calls in UI callbacks. No plugin framework or
shared backend is required for this example.

Only the `clockcfg` partition is opened. Network credentials are in its `network`
namespace; alarm settings remain in `clock`. Wi-Fi driver NVS and PHY calibration
persistence are disabled, leaving factory NVS untouched. This development device
does not have encrypted NVS provisioned. Keep physical flash backups private.

HTTPS verifies certificates against Espressif's bundled roots. Response bodies
are capped at12 KiB; JSON nesting16; requests have an8-second I/O timeout and
20-second event deadline. Providers receive the requested location and normal
source public IP. SSIDs, BSSIDs and Wi-Fi passwords are not sent to weather or
geolocation providers. IP lookup runs only for an unset, non-manual location.

References: [Open-Meteo forecast](https://open-meteo.com/en/docs),
[ZIP geocoding](https://open-meteo.com/en/docs/geocoding-api),
[ipapi public-IP location](https://ipapi.co/api/).
Personal weather use uses Open-Meteo's public noncommercial endpoint; screen
attribution is Open-Meteo / CC BY4.0. Recheck terms before distribution/commercial use.

## Evidence and limits

Host tests validate actual retrieved Summerfield fixtures and malformed responses,
units, freshness, ambiguity, manual precedence and DST. IDF6.1 builds and app-only
flashes pass. Device scan found6 access-point records; clock/RTC remained responsive
with stable post-scan heap. An initial connection was rejected (reason202); after the owner re-entered the
password, on-device HTTPS returned200 with732 bytes, the validated snapshot was
fresh, and NTP-to-RTC handoff returned ESP_OK. Saved credentials reconnected after
firmware restart. Host HTTP success alone is not device TLS/weather proof. Full-power-loss and
physical wake-up reliability gates remain open. See WORK for current receipts.
