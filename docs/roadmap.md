# Roadmap

What comes next, roughly in this order. Every item is a module or data, so the firmware stays the same for everyone.

## Before sharing widely
1. **Update locally via Home Assistant** – Home Assistant fetches the firmware once and serves it on your own network, so a remote needs no internet (VLANs). Also: install your own `.bin` from NOIR Builder.
2. **More remotes** – every remote its own name (`noir-remote-a1b2c3`), all linked by themselves. Per remote: the same screens as another one, or its own. *Update all* in one go.

## Languages
3. **Italiano** and more language packs – see [languages.md](languages.md). (Deutsch, Français and Español are in since 0.7.)

## Next
- **Heating (Toon, Nest, tado …)** – the Airco card adapts to a thermostat: orange dial, 0.5° steps, room temperature, presets (Comfort / Home / Sleep / Away) instead of airco modes, burner on/off when the thermostat reports it. Waits for a real thermostat entity to test with.
5. **Activities** – one tile runs several steps (TV on, source, speaker, lights, airco).
6. **Status on Home** – Audio shows what is playing, Lights how many lamps are on (the Airco tile already shows temperature and mode).
7. **Skins** – own colours and wallpaper, own logos for sources without one.
8. **Colour keys as shortcuts** – choose in NOIR Builder what red, green, yellow and blue do.
9. **Notifications** – Home Assistant shows a short message on the remote (doorbell, washing machine).
10. **Netflix module.**
11. **Clean-up** – English names in `noir-ui.h`, remove what is left from the time NOIR was a layer on another firmware.

Ideas and requests: open an issue on GitHub.
