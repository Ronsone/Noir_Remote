"""Mouse on the TV: the remote's touchpad screen moves the TV's cursor (as LG's Magic Remote does).

The remote sends noir_remote.pointer (dx, dy, scroll, click) a few dozen times a second while you drag. NOIR Remote
passes it to the TV through Home Assistant's own TV integration - no pairing, no Bluetooth. Supported now: LG webOS
(webostv: aiowebostv's pointer input). Another brand = one more branch in send().
"""
from __future__ import annotations

import logging

from homeassistant.core import HomeAssistant
from homeassistant.helpers import entity_registry as er

_LOGGER = logging.getLogger(__name__)
_warned: set[str] = set()


def _entry(hass: HomeAssistant, entity: str):
    """The config entry of the TV's media_player, or None."""
    ent = er.async_get(hass).async_get(entity) if entity else None
    return hass.config_entries.async_get_entry(ent.config_entry_id) if ent and ent.config_entry_id else None


def supported(hass: HomeAssistant, entity: str) -> bool:
    """True when this TV can show a mouse cursor from NOIR (the remote then offers its touchpad screen)."""
    e = _entry(hass, entity)
    return e is not None and e.domain == "webostv"


async def send(hass: HomeAssistant, entity: str, dx: int = 0, dy: int = 0, scroll: int = 0, click: bool = False) -> None:
    e = _entry(hass, entity)
    if e is None or e.domain != "webostv":
        if entity not in _warned:
            _warned.add(entity)
            _LOGGER.warning("NOIR mouse: %s is not an LG webOS TV - the mouse works with LG webOS for now", entity or "no TV chosen")
        return
    client = getattr(e, "runtime_data", None)
    if client is None:
        return
    try:
        if dx or dy:
            await client.move(int(dx), int(dy))
        if scroll:
            await client.scroll(0, int(scroll))
        if click:
            await client.click()
    except Exception as err:  # noqa: BLE001 - TV off or not connected: the next move tries again
        _LOGGER.debug("NOIR mouse: %s", err)
