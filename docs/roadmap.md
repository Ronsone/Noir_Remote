# Roadmap

What comes next, roughly in this order. Every item is a module or data, so the firmware stays the same for everyone.

## Before sharing widely
1. **Test channel for updates** – *Stable* for everyone (releases only), *Test* for testers. Chosen per remote in NOIR Builder.
2. **Update locally via Home Assistant** – Home Assistant fetches the firmware once and serves it on your own network, so a remote needs no internet (VLANs). Also: install your own `.bin` from NOIR Builder.
3. **More remotes** – every remote its own name (`noir-remote-a1b2c3`), all linked by themselves. Per remote: the same screens as another one, or its own. *Update all* in one go.

## Languages
4. **Language packs** – one file per language (`languages/xx.json`) with all texts of the remote and NOIR Builder; the remote gets its texts from Home Assistant, so a new language needs no new firmware. Missing texts fall back to English.
   First: **Deutsch, Français, Español, Italiano** (their characters are already in the font).

## Next
5. **Activities** – one tile runs several steps (TV on, source, speaker, lights, airco).
6. **Status on Home** – Audio shows what is playing, Lights how many lamps are on (the Airco tile already shows temperature and mode).
7. **Skins** – own colours and wallpaper, own logos for sources without one.
8. **Colour keys as shortcuts** – choose in NOIR Builder what red, green, yellow and blue do.
9. **Notifications** – Home Assistant shows a short message on the remote (doorbell, washing machine).
10. **Netflix module.**
11. **Clean-up** – English names in `noir-ui.h`, remove what is left from the time NOIR was a layer on another firmware.

Ideas and requests: open an issue on GitHub.
