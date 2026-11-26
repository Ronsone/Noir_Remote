# Languages

NOIR speaks **Nederlands** and **English** (built in) and **Deutsch**, **Français** and **Español** (language packs).
Choose in *NOIR Builder → Remote → Language* – the remote follows after **Save**. A new language needs **no new firmware**.

## Add a language

1. Copy `custom_components/noir_remote/languages/de.json` to `<code>.json`, e.g. `it.json`.
2. Set `"name"` to the language's own name (`"Italiano"`).
3. Translate the values – never the keys:
   - `builder`: the English text of NOIR Builder → your translation;
   - `remote`: the text in the firmware (mostly Dutch) → your translation.
4. Keep texts short – the remote's screen is 240 pixels wide.
5. Open a pull request. Missing texts simply show in English.

The remote's font has the letters of Western European languages (é, ü, ß, ñ, å, ç …). Languages with other letters
(Polish, Czech, Greek, Russian …) also need those letters added to the font in `firmware/noir-remote.yaml`.
