# Languages

NOIR speaks **Nederlands** and **English** (built in) and **Deutsch**, **Français** and **Español** (language packs).
Choose in *NOIR Builder → Remote → Language* – the remote follows after **Save**. A new language needs **no new firmware**.

## Add or improve a language

All texts are in **one file**: [`languages/en.json`](../custom_components/noir_remote/languages/en.json) – the English reference.

1. Copy `en.json` to `<code>.json`, e.g. `it.json`, and set `"name"` to the language's own name (`"Italiano"`).
2. Translate the **values** – never the keys:
   - `builder`: the texts of NOIR Builder (the key is the English text);
   - `remote`: the texts on the remote's screen (the key is how the firmware writes it, mostly Dutch – the English value tells you what it means).
3. Keep the `remote` texts short – the screen is 240 pixels wide.
4. Check it: `python3 tools/check_languages.py it` – it lists what is missing, left over or maybe too long.
5. Send it in:
   - **No git?** Open an issue with the **Translation** form and paste or attach your file: [new translation](https://github.com/ronsone/Noir_Remote/issues/new?template=translation.yml).
   - **With git:** open a pull request with your file.

Missing texts simply show in English. Improving an existing language works the same way – change its file.

## Letters

The remote's font has the letters of Western European languages (é, ü, ß, ñ, å, ç …). Languages with other letters
(Polish, Czech, Greek, Russian …) also need those letters added to the font in `firmware/noir-remote.yaml`.
