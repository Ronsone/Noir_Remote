# TV

The TV module knows no brands. Everything it needs comes from the NOIR Builder:

| What | From |
|---|---|
| On/off, state, choosing an input or app | your TV in Home Assistant (`media_player`) |
| Arrow keys, OK, back, media and colour keys | infrared, codes from the brand profile |
| Volume | your speaker in Home Assistant, or the TV by infrared when there is no speaker |

## Setting it up

In **NOIR Builder → TV**:

1. **Your TV in Home Assistant** – pick it from the list.
2. **Switching on and keys** – pick your brand and press **Test · On**. Did the TV switch on? Done.
   No brand that fits? Choose **No infrared**: on/off and inputs still work through Home Assistant, the arrow keys don't.
3. **Speaker** (optional) – switches on with the TV and gets the volume keys.
4. **Sources** – your TV's inputs and apps appear as buttons. Tap the ones you want (up to 8), give them a short name.
   The logo is picked from the name; change it if you like.

Press **Save**. On the remote: tap a source and the TV switches on (if needed) and goes there.

## Keys on the remote

| Where | What goes to the TV |
|---|---|
| TV screen | every key – arrows and OK too (hold an arrow to repeat) |
| Other screens | media, channel, colour and volume keys; arrows and OK move around the remote |
| Everywhere | **Power** switches the TV (and speaker) on or off |

## Logos

A tile shows a logo when the source name matches one we ship, for example *PS5 Gameconsole* → PlayStation, *HDMI 2* → a screen icon.
Brands that do not allow their logo (like Xbox, Nintendo, Disney+, Prime Video) get a clean general icon: game console, streaming, live TV, HDMI, music or cast.

Your own logos come with the Skins module.

## Adding a brand

All brand knowledge is one entry in `custom_components/noir_remote/profiles.json`:

```json
"mybrand": {
  "name": "My brand",
  "protocol": "nec",
  "address": 0,
  "tested": "Model and year you tested",
  "codes": { "power": 0, "up": 0, "down": 0, "left": 0, "right": 0, "ok": 0, "back": 0 }
}
```
(The zeros are placeholders – fill in your TV's real codes. The LG entry in the same file is a complete, tested example.)

- Only add codes you have **tested on a real TV**.
- Leave out keys you don't know – the remote simply skips them.
- Key names: `power_on power_off power up down left right ok back menu info play stop rewind ff ch_up ch_down red green yellow blue vol_up vol_down mute`.
- Supported protocol now: `nec` (LG and many others). Other protocols are one small addition in `firmware/core/modules/core.h` (`Ir::send`).

The firmware does not change for a new brand: update the integration, press **Save** in NOIR Builder.
