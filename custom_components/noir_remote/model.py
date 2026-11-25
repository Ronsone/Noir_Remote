"""Builder model (what you edit in the NOIR Builder) -> payload for the remote (NOIR tile format)."""
from __future__ import annotations

import copy

from .const import DEFAULT_MODEL, PROFILES
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
        elif k in m:
            m[k] = v
    m["tv"] = {k: m["tv"].get(k, d) for k, d in DEFAULT_MODEL["tv"].items()}   # drop old keys
    m["tv"]["apps"] = clean_apps(m["tv"]["apps"])
    return m


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
               "ir": ir_block(tv.get("profile", "")) if tv.get("profile") in PROFILES else {},
               "apps": clean_apps(tv.get("apps"))},
    }
