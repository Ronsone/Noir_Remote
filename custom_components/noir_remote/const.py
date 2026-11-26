"""NOIR Remote - constants and the default screen setup."""
import json
import pathlib

DOMAIN = "noir_remote"
STORE_VERSION = 1
_HERE = pathlib.Path(__file__).parent

# TV infrared profiles (profiles.json) and source logos (logos.json) - data, not code
PROFILES = {k: v for k, v in json.loads((_HERE / "profiles.json").read_text()).items() if not k.startswith("_")}
LOGOS = json.loads((_HERE / "logos.json").read_text())

# Rev6 hardware keys (keycode) for the profile key names
KEYCODES = {"up": 14, "down": 35, "left": 15, "right": 32, "ok": 34, "back": 13, "menu": 3, "info": 31,
            "play": 2, "stop": 5, "rewind": 4, "ff": 11, "ch_up": 42, "ch_down": 22,
            "red": 23, "green": 25, "yellow": 24, "blue": 21, "vol_up": 33, "vol_down": 43, "mute": 44}
# Boards NOIR knows: name for the Builder + its key codes (the TV keys by infrared map onto these).
# A device reports its board, screen size and capabilities when it fetches its screens (firmware 0.7.6+).
BOARDS = {
    "rev6": {"name": "OpenRemote Rev6", "keys": KEYCODES},
    "phone": {"name": "Phone (NOIR panel)", "keys": {}},   # sidebar "NOIR" in Home Assistant, the same screens in the browser
}
CAPS = ("touch", "keys", "ir", "battery")
LEGACY_DEVICE = {"board": "rev6", "w": 240, "h": 320, "caps": list(CAPS)}   # firmware before 0.7.6 = a Rev6 remote
MAX_APPS = 8
MAX_AUDIO_SOURCES = 6
MAX_AIRCO_UNITS = 4
AIRCO_IR = {"mhi": "Mitsubishi Heavy (SRK…ZS)"}   # infrared profiles in the firmware

DEFAULT_MODEL = {
    "mode": "full",   # full = always ready, eco = longer battery
    "channel": "stable",   # firmware updates: stable (releases) or test (newest build)
    "lang": "nl",     # nl | en - remote and NOIR Builder
    "tiles": [
        {"icon": "noir:tv", "label": "TV", "opens": "tv", "entity": "", "size": "half"},
        {"icon": "noir:licht", "label": "Verlichting", "opens": "lights", "entity": "", "size": "half"},
        {"icon": "noir:scenes", "label": "Scènes", "opens": "scenes", "entity": "", "size": "half"},
        {"icon": "mdi:power", "label": "Alles uit", "opens": "alloff", "entity": "", "size": "half"},
    ],
    "lights": [],
    "scenes": [],
    # profile: key in profiles.json, "" = no infrared (Home Assistant only)
    # apps: [{"name": "PS5", "source": "PS5 Gameconsole", "logo": "playstation"}]
    "tv": {"entity": "", "speaker": "", "speaker_source": "", "profile": "", "apps": []},
    # audio: one speaker - sources [{"name": "TV", "source": "tv"}] (up to 6), music = media_player for "now playing"
    # (empty = the speaker), sub = number entity of the subwoofer level in dB
    "audio": {"speaker": "", "sources": [], "music": "", "sub": ""},
    # airco: up to 4 units - control = "climate.x" (Home Assistant) or "ir:mhi" (the remote's infrared, Mitsubishi Heavy)
    "airco": {"units": []},
    # each other NOIR device its own home screen: {"noir-wall": {"tiles": [...]}}; the first remote uses "tiles" above
    "devices": {},
}

# Where the remote gets its firmware updates (NOIR Builder > Remote > Firmware)
UPDATE_URLS = {
    "stable": "",   # "" = the URL built into the firmware (releases)
    "test": "https://ronsone.github.io/Noir_Remote/firmware/beta/manifest.json",
}

# Languages: nl and en are built in; the others are packs in languages/<code>.json (NOIR Builder + remote texts)
LANGUAGES = {"nl": "Nederlands", "en": "English"}
LANG_PACKS: dict[str, dict] = {}
for _f in sorted((_HERE / "languages").glob("*.json")):
    _p = json.loads(_f.read_text(encoding="utf-8"))
    LANG_PACKS[_f.stem] = {"builder": _p.get("builder", {}), "remote": _p.get("remote", {})}
    LANGUAGES[_f.stem] = _p.get("name", _f.stem)
