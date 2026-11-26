# Roadmap

What comes next, roughly in this order. Every item is a module or data, so the firmware stays the same for everyone.

## Before sharing widely
1. **Update locally via Home Assistant** – Home Assistant fetches the firmware once and serves it on your own network, so a remote needs no internet (VLANs). Also: install your own `.bin` from NOIR Builder.
2. **More remotes** – every remote its own name (`noir-remote-a1b2c3`), all linked by themselves. Per remote: the same screens as another one, or its own. *Update all* in one go.

## Wider than the remote
3. **One codebase for more devices** – boards with capabilities (screen size, touch, keys, IR, battery); a layout that follows the screen size; the Waveshare ESP32-P4-NANO 10.1" as wall panel; the same screens as a panel in Home Assistant (phone).

## Languages
4. **Italiano** and more language packs – see [languages.md](languages.md). (Deutsch, Français and Español are in since 0.7.)

## Next
5. **Heating (Toon, Nest, tado …)** – the Airco card adapts to a thermostat: orange dial, 0.5° steps, room temperature, presets (Comfort / Home / Sleep / Away) instead of airco modes, burner on/off when the thermostat reports it. Waits for a real thermostat entity to test with.
6. **Smart heating: airco or gas, whichever is cheaper** – an airco is a heat pump (1 kWh power gives 3–4 kWh heat). NOIR Remote gets a sensor *Cheapest heat* (airco / gas, with the price per kWh of heat for both) from your electricity price (dynamic: Tibber, Frank, ENTSO-e …), gas price, the outdoor temperature and a COP curve you can tune; a switch *Smart heating* lets Home Assistant heat with the cheaper one (airco on heat and the thermostat lower, or the other way round). The climate card shows it: *Cheaper now: airco · 9 ct vs gas 15 ct*. **With solar panels:** during a surplus the airco's power costs only the missed feed-in price (often near zero midday), so the airco wins; later also *pre-heat on sun* (a degree warmer in the afternoon, less gas in the evening). Matters more once net metering (saldering) ends in 2027. Extra inputs: grid export or solar production (P1 meter / inverter) and the feed-in price. Price source tested first: Zonneplan (official Home Assistant integration). Needs the thermostat and the aircos as climate entities (ESP32 modules), to test with real numbers.
7. **Activities** – one tile runs several steps (TV on, source, speaker, lights, airco).
8. **Status on Home** – Audio shows what is playing, Lights how many lamps are on (the Airco tile already shows temperature and mode).
9. **Skins** – own colours and wallpaper, own logos for sources without one.
10. **Colour keys as shortcuts** – choose in NOIR Builder what red, green, yellow and blue do.
11. **Notifications** – Home Assistant shows a short message on the remote (doorbell, washing machine).
12. **Clean-up** – English names in `noir-ui.h` (the old Aggro600 parts are gone since 0.7.2).

Done: browser installer, updates over Wi-Fi with a Stable and a Test channel (0.7), language packs de/fr/es (0.7), Audio (0.4), Airco (0.5), D-pad (0.6).

Ideas and requests: open an issue on GitHub.
