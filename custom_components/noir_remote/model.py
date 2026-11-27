"""Builder model (what you edit in the NOIR Builder) -> payload for the remote (NOIR tile format)."""
from __future__ import annotations

import copy

from .const import AIRCO_IR, BOARDS, DEFAULT_MODEL, LANG_PACKS, LANGUAGES, LEGACY_DEVICE, MAX_AIRCO_UNITS, MAX_AUDIO_SOURCES, PROFILES
from .tv import clean_apps, ir_block

FN = {"tv", "lights", "scenes", "settings", "alloff", "tv_power", "audio", "airco", "mouse"}
INFO = {"weather", "calendar", "value"}   # tiles that show information (info.py) instead of opening something


def normalize(model: dict | None) -> dict:
    """Fill in missing parts with the defaults (old or partial stores keep working)."""
    m = copy.deepcopy(DEFAULT_MODEL)
    for k, v in (model or {}).items():
        if k == "tv" and isinstance(v, dict):
            m["tv"].update(v)
            if "kef" in v and not v.get("speaker"):        # before 0.2
                m["tv"]["speaker"] = v["kef"]
            if "on" in v and "profile" not in v:           # before 0.2: LG codes were filled in
                m["tv"]["profile"] = "lg"
        elif k == "airco" and isinstance(v, dict):
            m["airco"].update(v)
        elif k == "audio" and isinstance(v, dict):
            m["audio"].update(v)
        elif k in m:
            m[k] = v
    m["tv"] = {k: m["tv"].get(k, d) for k, d in DEFAULT_MODEL["tv"].items()}   # drop old keys
    m["tv"]["apps"] = clean_apps(m["tv"]["apps"])
    m["audio"] = {k: m["audio"].get(k, d) for k, d in DEFAULT_MODEL["audio"].items()}
    m["audio"]["sources"] = clean_sources(m["audio"]["sources"])
    m["airco"] = {"units": clean_units(m["airco"].get("units"))}
    devs = m.get("devices") if isinstance(m.get("devices"), dict) else {}
    m["devices"] = {str(k)[:40]: {"tiles": [t for t in v.get("tiles", []) if isinstance(t, dict)][:24]}
                    for k, v in devs.items() if isinstance(v, dict) and isinstance(v.get("tiles"), list)}
    if m.get("lang") not in LANGUAGES:
        m["lang"] = "nl"
    if m.get("channel") not in ("stable", "test"):
        m["channel"] = "stable"
    return m


def clean_units(units) -> list:
    """Airco units: [{"name", "control"}], control = "climate.x" or "ir:<profile>"."""
    out = []
    for u in units if isinstance(units, list) else []:
        if not isinstance(u, dict):
            continue
        c = str(u.get("control", "")).strip()
        if not (c.startswith("climate.") or (c.startswith("ir:") and c[3:] in AIRCO_IR)):
            continue
        out.append({"name": str(u.get("name", "")).strip()[:16], "control": c})
    return out[:MAX_AIRCO_UNITS]


def clean_sources(sources) -> list:
    """Audio sources: [{"name", "source"}], name defaults to the source, at most MAX_AUDIO_SOURCES."""
    out = []
    for s in sources if isinstance(sources, list) else []:
        if not isinstance(s, dict) or not str(s.get("source", "")).strip():
            continue
        src = str(s["source"]).strip()[:40]
        out.append({"name": (str(s.get("name", "")).strip() or src)[:14], "source": src})
    return out[:MAX_AUDIO_SOURCES]


def tiles_of(m: dict, device: str, primary: str) -> list:
    """The home tiles of a device: its own, else (first remote, or not set up yet) the main list."""
    own = m.get("devices", {}).get(device)
    return own["tiles"] if device != primary and own else m["tiles"]


SIZES = {"half": "S", "full": "W", "large": "L"}


def payload(model: dict, version: str, device: str = "", primary: str = "", info: dict | None = None) -> dict:
    """What one device gets: its own home screen; infrared, keys and IR aircos only when it has them."""
    m = normalize(model)
    info = info or LEGACY_DEVICE
    caps = set(info.get("caps", []))
    keycodes = BOARDS.get(info.get("board", ""), {}).get("keys", {}) if "keys" in caps else {}
    home, x, y = [], 0, 0
    for t in tiles_of(m, device, primary)[:24]:
        size = SIZES.get(t.get("size"), "S")
        full = size != "S"
        if full and x:
            x, y = 0, y + 2
        # size for the grid (0.7.4+); x/y/w/h for older firmware
        it = {"t": "action", "label": str(t.get("label", ""))[:24], "icon": t.get("icon", ""), "size": size,
              "x": 0 if full else x, "y": y, "w": 6 if full else 3, "h": 4 if size == "L" else 2}
        opens = t.get("opens", "")
        if opens in INFO and t.get("entity"):   # weather / calendar / value tile
            it["t"], it["entity"] = opens, t["entity"]
        elif opens in FN:
            it["fn"] = opens
        elif t.get("entity"):
            it["entity"] = t["entity"]
            if opens == "toggle":
                it["t"] = "light" if t["entity"].startswith("light.") else "toggle"
        home.append(it)
        if full:
            y += it["h"]
        elif x == 0:
            x = 3
        else:
            x, y = 0, y + 2
    lights = [{"t": "light" if str(l.get("entity", "")).startswith("light.") else "toggle",
               "label": str(l.get("label", ""))[:24], "entity": l["entity"], "dim": bool(l.get("dim", True))}
              for l in m["lights"][:12] if l.get("entity")]
    scenes = [{"t": "action", "label": str(s.get("label", ""))[:24], "icon": s.get("icon", "mdi:play"),
               "entity": s.get("entity", "")} for s in m["scenes"][:30] if s.get("label")]
    tv = m["tv"]
    return {
        "version": version,
        "mode": "eco" if m.get("mode") == "eco" else "full",
        "lang": m["lang"],
        "texts": LANG_PACKS.get(m["lang"], {}).get("remote", {}),   # de/fr/es: the remote's texts (nl/en are built in)
        "home": home,
        "lights": lights,
        "scenes": scenes,
        "tv": {"entity": tv.get("entity", ""), "speaker": tv.get("speaker", ""),
               "speaker_source": str(tv.get("speaker_source", ""))[:40],   # some speakers (KEF LSX II) only wake up by choosing a source
               "ir": ir_block(tv.get("profile", ""), keycodes) if tv.get("profile") in PROFILES and "ir" in caps else {},
               "apps": clean_apps(tv.get("apps"))},
        "audio": audio_block(m["audio"]),
        "airco": {"units": [{"name": u["name"], **({"ir": u["control"][3:]} if u["control"].startswith("ir:") else {"entity": u["control"]})}
                            for u in m["airco"]["units"] if "ir" in caps or not u["control"].startswith("ir:")]},
    }


def audio_block(a: dict) -> dict:
    """What the remote's Audio module needs; the name on the screen comes from the speaker in Home Assistant."""
    return {"speaker": a.get("speaker", ""), "name": a.get("name", ""), "sources": a.get("sources", []),
            "music": a.get("music", ""), "sub": a.get("sub", "")}
