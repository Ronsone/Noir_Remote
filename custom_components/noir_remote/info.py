"""Info tiles on Home: weather, calendar and a value (stock, crypto, any number sensor).

The device asks for them in its /states request as "i:<entity>"; it gets a small ready-made object back, so the
firmware does not need to know weather, calendar or history formats. Each entity is looked up at most once a minute.

    weather  {"c": condition, "t": temp, "lo": .., "hi": .., "p": place, "h": [["15:00", 13], ..], "d": [["vr", 14, 8], ..]}
    calendar {"ev": [[day, time, title, where], ..]}   day = "Vandaag" / "Morgen" (the remote translates) or "za 29 nov"
    value    {"n": name, "v": "€ 84.210", "ch": "+2,4%", "tr": 1 | 0 | -1, "hs": [24 numbers]}
"""
from __future__ import annotations

from datetime import date, datetime, timedelta
import logging
import time

from homeassistant.core import HomeAssistant
from homeassistant.util import dt as dt_util

_LOGGER = logging.getLogger(__name__)
TTL = 60          # seconds an answer is reused
HOURS, DAYS, EVENTS, POINTS = 4, 5, 6, 24

_cache: dict[str, tuple[float, dict]] = {}

WEEKDAYS = {   # short weekday names, Monday first
    "nl": ["ma", "di", "wo", "do", "vr", "za", "zo"],
    "en": ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"],
    "de": ["Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"],
    "fr": ["lun", "mar", "mer", "jeu", "ven", "sam", "dim"],
    "es": ["lun", "mar", "mié", "jue", "vie", "sáb", "dom"],
}
MONTHS = {
    "nl": ["jan", "feb", "mrt", "apr", "mei", "jun", "jul", "aug", "sep", "okt", "nov", "dec"],
    "en": ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"],
    "de": ["Jan", "Feb", "Mär", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"],
    "fr": ["janv", "févr", "mars", "avr", "mai", "juin", "juil", "août", "sept", "oct", "nov", "déc"],
    "es": ["ene", "feb", "mar", "abr", "may", "jun", "jul", "ago", "sep", "oct", "nov", "dic"],
}


async def info(hass: HomeAssistant, entity: str, lang: str) -> dict:
    """The info object for one entity (cached for a minute). {} when it does not exist or has nothing yet."""
    key = f"{lang}|{entity}"
    hit = _cache.get(key)
    if hit and time.monotonic() - hit[0] < TTL:
        return hit[1]
    domain = entity.split(".", 1)[0]
    try:
        if domain == "weather":
            out = await _weather(hass, entity, lang)
        elif domain == "calendar":
            out = await _calendar(hass, entity, lang)
        else:
            out = await _value(hass, entity, lang)
    except Exception as err:  # noqa: BLE001 - a tile shows "—" rather than breaking the request
        _LOGGER.debug("NOIR info for %s failed: %s", entity, err)
        out = hit[1] if hit else {}
    _cache[key] = (time.monotonic(), out)
    return out


def _num(x, default=0):
    try:
        return round(float(x))
    except (TypeError, ValueError):
        return default


def _local(s) -> datetime | None:
    if isinstance(s, datetime):
        return dt_util.as_local(s)
    d = dt_util.parse_datetime(str(s)) if s else None
    return dt_util.as_local(d) if d else None


async def _forecast(hass: HomeAssistant, entity: str, kind: str) -> list:
    try:
        r = await hass.services.async_call("weather", "get_forecasts", {"entity_id": entity, "type": kind},
                                           blocking=True, return_response=True)
        return (r or {}).get(entity, {}).get("forecast", []) or []
    except Exception:  # noqa: BLE001 - not every weather integration has hourly (or daily) forecasts
        return []


async def _weather(hass: HomeAssistant, entity: str, lang: str) -> dict:
    st = hass.states.get(entity)
    if st is None or st.state in ("unavailable", "unknown"):
        return {}
    a = st.attributes
    out = {"c": st.state, "t": _num(a.get("temperature")), "p": str(a.get("friendly_name") or "")[:20]}
    days = await _forecast(hass, entity, "daily")
    hours = await _forecast(hass, entity, "hourly")
    if days:
        out["hi"], out["lo"] = _num(days[0].get("temperature")), _num(days[0].get("templow"), out["t"])
    else:
        out["hi"] = out["lo"] = out["t"]
    wd = WEEKDAYS.get(lang, WEEKDAYS["en"])
    out["d"] = [[wd[d.weekday()], _num(f.get("temperature")), _num(f.get("templow"))]
                for f in days[1:DAYS + 1] if (d := _local(f.get("datetime")))]
    now = dt_util.now()
    out["h"] = [[f"{d.hour:02d}:00", _num(f.get("temperature"))]
                for f in hours if (d := _local(f.get("datetime"))) and d > now][:HOURS]
    return out


def _day(d: date, today: date, lang: str) -> str:
    if d == today:
        return "Vandaag"   # the remote translates these two (language packs)
    if d == today + timedelta(days=1):
        return "Morgen"
    return f"{WEEKDAYS.get(lang, WEEKDAYS['en'])[d.weekday()]} {d.day} {MONTHS.get(lang, MONTHS['en'])[d.month - 1]}"


async def _calendar(hass: HomeAssistant, entity: str, lang: str) -> dict:
    if hass.states.get(entity) is None:
        return {}
    r = await hass.services.async_call("calendar", "get_events", {"entity_id": entity, "duration": {"days": 7}},
                                       blocking=True, return_response=True)
    events = (r or {}).get(entity, {}).get("events", []) or []
    today, now = dt_util.now().date(), dt_util.now()
    rows = []
    for e in events:
        start, end = e.get("start"), e.get("end")
        all_day = isinstance(start, str) and len(start) == 10
        if all_day:
            d, t = date.fromisoformat(start), "·"
            if date.fromisoformat(str(end)[:10]) <= today:
                continue
            d = max(d, today)
        else:
            s, en = _local(start), _local(end)
            if s is None or (en and en < now):
                continue
            d, t = max(s.date(), today), f"{s.hour:02d}:{s.minute:02d}" if s.date() >= today else "·"
        rows.append((d, t, str(e.get("summary") or "")[:40], str(e.get("location") or "")[:30]))
    rows.sort(key=lambda x: (x[0], x[1] != "·", x[1]))
    return {"ev": [[_day(d, today, lang), t, title, where] for d, t, title, where in rows[:EVENTS]]}


def fmt(v: float, unit: str, lang: str) -> str:
    """84210 EUR -> "€ 84.210" (nl) / "€84,210" (en)."""
    dec = 0 if abs(v) >= 1000 else 1 if abs(v) >= 100 else 2
    s = f"{v:,.{dec}f}"
    if lang != "en":
        s = s.replace(",", "_").replace(".", ",").replace("_", ".")
    u = (unit or "").strip()
    pre = {"EUR": "€", "€": "€", "USD": "$", "$": "$", "GBP": "£", "£": "£"}.get(u)
    if pre:
        return f"{pre} {s}" if lang != "en" else f"{pre}{s}"
    return f"{s}%" if u == "%" else f"{s} {u}".strip()


async def _history(hass: HomeAssistant, entity: str) -> list[float]:
    """The last 24 hours of a sensor from the recorder, as numbers ([] without a recorder or history)."""
    try:
        from homeassistant.components.recorder import get_instance, history
    except ImportError:
        return []
    start = dt_util.utcnow() - timedelta(hours=24)
    states = await get_instance(hass).async_add_executor_job(
        lambda: history.state_changes_during_period(hass, start, None, entity, no_attributes=True,
                                                    include_start_time_state=True))
    out = []
    for s in states.get(entity, []):
        try:
            out.append(float(s.state))
        except (TypeError, ValueError):
            continue
    return out


def _points(vals: list[float], n: int = POINTS) -> list[float]:
    if len(vals) <= n:
        return vals
    return [vals[round(i * (len(vals) - 1) / (n - 1))] for i in range(n)]


async def _value(hass: HomeAssistant, entity: str, lang: str) -> dict:
    st = hass.states.get(entity)
    if st is None or st.state in ("unavailable", "unknown"):
        return {}
    try:
        v = float(st.state)
    except ValueError:   # not a number: show the state as text
        return {"n": str(st.attributes.get("friendly_name") or entity)[:20], "v": st.state[:20], "ch": "", "tr": 0, "hs": []}
    hist = await _history(hass, entity)
    out = {"n": str(st.attributes.get("friendly_name") or entity)[:20],
           "v": fmt(v, str(st.attributes.get("unit_of_measurement") or ""), lang), "ch": "", "tr": 0,
           "hs": [round(x, 4) for x in _points(hist + [v])]}
    if hist and hist[0]:
        pct = (v - hist[0]) / abs(hist[0]) * 100
        s = f"{pct:+.1f}%"
        out["ch"] = s.replace(".", ",") if lang != "en" else s
        out["tr"] = 1 if pct > 0.05 else -1 if pct < -0.05 else 0
    return out
