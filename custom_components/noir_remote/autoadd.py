"""Add the remote to the ESPHome integration by itself - nobody has to type the encryption key.

The remote asks Home Assistant for its screens (ConfigView) with the shared key. That proves it was flashed with the
secrets.yaml of this integration, so its ESPHome encryption key is ours too. We then run the normal ESPHome
"add device" flow for its IP address and answer the key question ourselves.
"""
from __future__ import annotations

import logging
import time

from homeassistant.core import HomeAssistant

_LOGGER = logging.getLogger(__name__)
ESPHOME = "esphome"
RETRY_S = 600          # don't try the same remote more often than this
_last_try: dict[str, float] = {}


def _known(hass: HomeAssistant, device: str, host: str) -> bool:
    """True when ESPHome already has this remote (by device name or address)."""
    for entry in hass.config_entries.async_entries(ESPHOME, include_ignore=False):
        if entry.data.get("device_name") == device or entry.data.get("host") == host:
            return True
    return False


async def maybe_add(hass: HomeAssistant, device: str, host: str, api_key: str) -> None:
    """Add the remote at `host` to ESPHome with `api_key`, unless it is there already."""
    if not api_key or not host or _known(hass, device, host):
        return
    now = time.monotonic()
    if now - _last_try.get(host, -RETRY_S) < RETRY_S:
        return
    _last_try[host] = now

    flow = hass.config_entries.flow
    try:
        result = await flow.async_init(ESPHOME, context={"source": "user"}, data={"host": host, "port": 6053})
        if result.get("type") == "form" and result.get("step_id") == "encryption_key":
            result = await flow.async_configure(result["flow_id"], {"noise_psk": api_key})
    except Exception as err:  # noqa: BLE001 - never break the remote's config download
        _LOGGER.warning("Could not add the NOIR remote to ESPHome by itself: %s", err)
        return

    if result.get("type") == "create_entry":
        _LOGGER.info("NOIR remote %s (%s) added to ESPHome", device, host)
        return
    # Anything else (wrong key, name conflict, ...): leave it to the person, don't keep a half-done flow around.
    _LOGGER.warning("NOIR remote %s (%s) could not be added to ESPHome by itself (%s %s) - add it by hand",
                    device, host, result.get("type"), result.get("step_id") or result.get("reason") or "")
    if result.get("flow_id") and result.get("type") == "form":
        try:
            flow.async_abort(result["flow_id"])
        except Exception:  # noqa: BLE001
            pass
