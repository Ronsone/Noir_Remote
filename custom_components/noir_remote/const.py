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
MAX_APPS = 8
MAX_AUDIO_SOURCES = 6

DEFAULT_MODEL = {
    "mode": "full",   # full = always ready, eco = longer battery
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
}
