<p align="center"><img src="docs/hero.png" width="100%" alt="NOIR – a calm, dark remote for Home Assistant"></p>

<p align="center">
  <b>NOIR turns the OpenRemote Rev6 into a calm, dark remote for your home.</b><br>
  Big tiles, few words, one tap. You set it up in Home Assistant – no code, no YAML editing.
</p>

<p align="center">
  <a href="#setup--about-30-minutes-most-of-it-waiting-for-the-first-build"><b>Get started</b></a> ·
  <a href="docs/tv.md">TV</a> ·
  <a href="#inside">How it works</a>
</p>

<br>

| ✦ **Simple** | ✦ **Yours** | ✦ **Open** |
|---|---|---|
| One screen per task. Text only where it helps. Nederlands or English. | Build your screens in the **NOIR Builder** – what you see is what the remote shows. | Any TV in Home Assistant. Free, MIT licensed, every feature a small module. |

<p align="center"><img src="docs/screens.png" width="100%" alt="Home, TV, Lights, Scenes, Settings"></p>

## What it does (version 0.3)

| | |
|---|---|
| **Home** | Big tiles. You choose what each one opens. |
| **TV** | Any TV in Home Assistant. Your inputs and apps as tiles with their logo. Keys by infrared. |
| **Lights** | Tap = on/off, hold = dim. |
| **Scenes** | Your scenes and scripts, plus *All off*. |
| **Modes** | **Full** – always ready. **Eco** – longer battery. |

Audio, Netflix and Airco come next – each as its own module.

## Setup – about 15 minutes

**You need:** an OpenRemote Rev6 with a USB-C cable that carries data (not a charge-only cable) · Home Assistant with [HACS](https://hacs.xyz) · a PC or Mac with Chrome or Edge.
No programming, no files, no keys to copy.

### 1 · Home Assistant
1. Click this button – it opens HACS in your own Home Assistant with NOIR Remote filled in:

   [![Open NOIR Remote in HACS](https://my.home-assistant.io/badges/hacs_repository.svg)](https://my.home-assistant.io/redirect/hacs_repository/?owner=ronsone&repository=Noir_Remote&category=integration)

   Press **Download** (bottom right) → **Download**. Then restart Home Assistant: *Settings → System → ⏻ Restart*.
2. Click this button and press **Submit**:

   [![Add NOIR Remote](https://my.home-assistant.io/badges/config_flow_start.svg)](https://my.home-assistant.io/redirect/config_flow_start/?domain=noir_remote)

   *(Or: Settings → Devices & services → Add integration → NOIR Remote.)*

<details><summary>Without HACS</summary>

Install the **Samba share** add-on. The config folder of Home Assistant then opens on your PC (Windows Explorer: `\\homeassistant.local\config`).
Download this page (**Code → Download ZIP**), unzip it and drag the folder `custom_components/noir_remote` into `config/custom_components`. Restart Home Assistant.
</details>

### 2 · Firmware – from your browser
1. Connect the remote to your PC with the USB-C cable.
2. Open the **[NOIR installer](https://ronsone.github.io/Noir_Remote/)** in Chrome or Edge, press **Installeren** and choose the remote's port.
   It takes about two minutes.

### 3 · Wi-Fi
The remote starts its own Wi-Fi network. Connect your phone to **noir-remote setup**, a page opens: choose your Wi-Fi and fill in the password (2.4 GHz Wi-Fi).

### 4 · Done
Within a minute Home Assistant adds the remote by itself – the encryption key, the actions and the connection to NOIR Remote are all set up for you.
You find it under *Settings → Devices & services → ESPHome*.

### 5 · Your screens
Open **NOIR Builder** in the sidebar. Each tab is one screen, the preview shows your remote. Press **Save** – the remote updates in seconds.

**TV in four steps:** choose your TV → choose its brand and press **Test** → (optional) your speaker → tap the inputs and apps you want. Logos are picked for you. More in [docs/tv.md](docs/tv.md).

## Updating

Your screens and settings stay.

- **Remote:** *NOIR Builder → Remote → Firmware* → **Update firmware**. The remote downloads the new firmware itself and restarts. (Home Assistant also shows the update under *Settings → Updates*.)
- **Home Assistant:** HACS shows the update – press **Update**, then restart Home Assistant.

**Coming from version 0.2** (with a `secrets.yaml`)? Install the new firmware once with the [installer](https://ronsone.github.io/Noir_Remote/) via USB and set up Wi-Fi (steps 2 and 3).
In Home Assistant, delete the old *noir-remote* under ESPHome first; the new one is added by itself.

<details><summary>Build the firmware yourself (developers)</summary>

With Python 3: `pip install esphome`, then in the folder `firmware`: `esphome run noir-remote.yaml`.
GitHub checks that the firmware builds on every change in `firmware/` (`.github/workflows/firmware.yml`). Remotes only get it when you **release** it:
raise `substitutions: version` in `noir-remote.yaml`, then *GitHub → Releases → Draft a new release*, tag `v` + that version (e.g. `v0.3.1`) → **Publish**.
Only then is it published for the installer and the update button.
</details>

## Inside

```
firmware/
  noir-remote.yaml    hardware + ESPHome setup
  noir-ui.h           the screens (shared with the simulator)
  noir-app.h          starts the modules
  modules/            one file per feature: core, home, lights, scenes, tv
  logos/              logos for TV sources (made by tools/make_logos.py)
custom_components/noir_remote/
  NOIR Builder panel, config + state endpoints for the remote
  link.py             links the remote by itself (confirm, actions, host + key)
  profiles.json       infrared codes per TV brand
  logos.json          which source name gets which logo
site/                 the installer page (GitHub Pages)
simulator/            all screens on your PC (SDL2 + LVGL)
```

**New feature = new module.** Copy `firmware/modules/scenes.h`, give it a name, handle its actions/keys, add it in `noir-app.h`.

## Credits

Hardware: [OpenRemote Rev6](https://github.com/LORDSn1per/OpenRemote-Hardware). Display, touch, key and battery drivers come from [ESPHomeRemote by Aggro600](https://github.com/Aggro600/ESPHomeRemote). Font: Inter (OFL). Icons: Material Design Icons, Lucide (ISC). Source logos: [Simple Icons](https://simpleicons.org) (CC0) – trademarks belong to their owners and only show which source a tile opens.

License: MIT
