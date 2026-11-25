"""Builder model (what you edit in the NOIR Builder) -> payload for the remote (NOIR tile format)."""
from __future__ import annotations

import copy

from .const import DEFAULT_MODEL, MAX_AUDIO_SOURCES, PROFILES
from .tv import clean_apps, ir_block

FN = {"tv", "lights", "scenes", "settings", "alloff", "tv_power", "audio", "netflix", "airco"}


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
        elif k == "audio" and isinstance(v, dict):
            m["audio"].update(v)
        elif k in m:
            m[k] = v
    m["tv"] = {k: m["tv"].get(k, d) for k, d in DEFAULT_MODEL["tv"].items()}   # drop old keys
    m["tv"]["apps"] = clean_apps(m["tv"]["apps"])
    m["audio"] = {k: m["audio"].get(k, d) for k, d in DEFAULT_MODEL["audio"].items()}
    m["audio"]["sources"] = clean_sources(m["audio"]["sources"])
    return m


def clean_sources(sources) -> list:
    """Audio sources: [{"name", "source"}], name defaults to the source, at most MAX_AUDIO_SOURCES."""
    out = []
    for s in sources if isinstance(sources, list) else []:
        if not isinstance(s, dict) or not str(s.get("source", "")).strip():
            continue
        src = str(s["source"]).strip()[:40]
        out.append({"name": (str(s.get("name", "")).strip() or src)[:14], "source": src})
    return out[:MAX_AUDIO_SOURCES]


def payload(model: dict, version: str) -> dict:
    m = normalize(model)
    home, x, y = [], 0, 0
    for t in m["tiles"][:24]:
        full = t.get("size") == "full"
        if full and x:
            x, y = 0, y + 2
        it = {"t": "action", "label": str(t.get("label", ""))[:24], "icon": t.get("icon", ""),
              "x": 0 if full else x, "y": y, "w": 6 if full else 3, "h": 2}
        opens = t.get("opens", "")
        if opens in FN:
            it["fn"] = opens
        elif t.get("entity"):
            it["entity"] = t["entity"]
            if opens == "toggle":
                it["t"] = "light" if t["entity"].startswith("light.") else "toggle"
        home.append(it)
        if full:
            y += 2
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
        "lang": "en" if m.get("lang") == "en" else "nl",
        "home": home,
        "lights": lights,
        "scenes": scenes,
        "tv": {"entity": tv.get("entity", ""), "speaker": tv.get("speaker", ""),
               "speaker_source": str(tv.get("speaker_source", ""))[:40],   # some speakers (KEF LSX II) only wake up by choosing a source
               "ir": ir_block(tv.get("profile", "")) if tv.get("profile") in PROFILES else {},
               "apps": clean_apps(tv.get("apps"))},
        "audio": audio_block(m["audio"]),
    }


def audio_block(a: dict) -> dict:
    """What the remote's Audio module needs; the name on the screen comes from the speaker in Home Assistant."""
    return {"speaker": a.get("speaker", ""), "name": a.get("name", ""), "sources": a.get("sources", []),
            "music": a.get("music", ""), "sub": a.get("sub", "")}
