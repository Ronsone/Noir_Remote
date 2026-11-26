"""TV: profile -> infrared block for the remote, and logo matching for sources. Brand knowledge lives here, not in the firmware."""
from __future__ import annotations

import re

from .const import KEYCODES, LOGOS, MAX_APPS, PROFILES


def ir_block(profile: str, keycodes: dict | None = None) -> dict:
    """{"p": "nec", "a": 4, "on": .., "off": .., "power": .., "keys": {"14": 64, ...}} or {} (no infrared)."""
    p = PROFILES.get(profile)
    if not p:
        return {}
    c = p.get("codes", {})
    keycodes = KEYCODES if keycodes is None else keycodes   # the board's key codes ({} = a device without keys)
    return {"p": p["protocol"], "a": int(p["address"]),
            "on": c.get("power_on", -1), "off": c.get("power_off", -1), "power": c.get("power", -1),
            "keys": {str(keycodes[k]): v for k, v in c.items() if k in keycodes}}


def _words(text: str) -> str:
    return " " + re.sub(r"[^a-z0-9]+", " ", text.lower()).strip() + " "


def guess_logo(name: str) -> str:
    """Brand first, then a generic mark; longest match wins. "" = no logo (name only)."""
    w = _words(name)
    best, size = "", 0
    for group in ("brands", "generic"):
        for key, v in LOGOS[group].items():
            for m in v["match"]:
                if f" {m} " in w and len(m) > size:
                    best, size = key, len(m)
        if best:
            return best
    return ""


def logo_list() -> list[dict]:
    """For the NOIR Builder: [{"key", "name", "group"}]"""
    return [{"key": k, "name": v["name"], "group": g} for g in ("brands", "generic") for k, v in LOGOS[g].items()]


def clean_apps(apps) -> list[dict]:
    out = []
    for a in apps or []:
        if isinstance(a, str):   # before 0.2: just the source name
            a = {"name": a, "source": a}
        if not isinstance(a, dict) or not (a.get("source") or a.get("name")):
            continue
        name = str(a.get("name") or a.get("source"))[:20]
        logo = a.get("logo")
        if logo is None:
            logo = guess_logo(name + " " + str(a.get("source", "")))
        out.append({"name": name, "source": str(a.get("source") or name)[:40], "logo": str(logo)})
    return out[:MAX_APPS]
