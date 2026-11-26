"""Link the remote to Home Assistant by itself - no keys, no files.

Since firmware 0.3 the remote holds no secrets. Every 20 seconds this module:
1. confirms ESPHome's "New device found" for the remote (Home Assistant then gives it an encryption key itself);
2. lets the remote run Home Assistant actions (ESPHome switches that off for new devices);
3. tells the remote where Home Assistant is and the shared key (action noir_setup);
4. tells the remote where its updates come from (action noir_channel: stable or test).
3 and 4 go once per Home Assistant start, and again every time the remote comes back online (after a reset, a flash
via USB or empty batteries it may have lost them). Sending the same again changes nothing on the remote.
"""
from __future__ import annotations

from datetime import timedelta
import logging

from homeassistant.core import HomeAssistant, callback
from homeassistant.helpers import device_registry as dr, entity_registry as er
from homeassistant.helpers.event import async_track_time_interval

from .autoadd import ESPHOME, allow_actions

_LOGGER = logging.getLogger(__name__)
INTERVAL = timedelta(seconds=20)


def _norm(s: str) -> str:
    return "".join(c for c in (s or "").lower() if c.isalnum())


def start(hass: HomeAssistant, device: str, host_of, key: str, update_url=lambda: ""):
    """Start the link loop. Returns the function that stops it."""
    sent: set[str] = set()
    busy: set[str] = set()
    was_online = [False]

    def online() -> bool:
        """True while ESPHome has the remote connected (one of its entities is not unavailable)."""
        devs, ents = dr.async_get(hass), er.async_get(hass)
        for d in devs.devices.values():
            if not any(_norm(device) in _norm(n) for n in (d.name, d.name_by_user) if n):
                continue
            for e in er.async_entries_for_device(ents, d.id):
                if e.platform == ESPHOME and (st := hass.states.get(e.entity_id)) and st.state != "unavailable":
                    return True
        return False

    async def confirm(flow_id: str) -> None:
        try:
            result = await hass.config_entries.flow.async_configure(flow_id, {})
            if result.get("type") == "create_entry":
                _LOGGER.info("NOIR remote added to ESPHome")
        except Exception as err:  # noqa: BLE001 - the person can still confirm it by hand
            _LOGGER.debug("Could not confirm the ESPHome flow for the remote: %s", err)
        finally:
            busy.discard(flow_id)

    async def setup_remote(svc: str) -> None:
        host = host_of(hass)   # asked late: early in the start Home Assistant may not know its own address yet
        if "x" in host:
            return
        try:
            await hass.services.async_call(ESPHOME, svc, {"host": host, "key": key}, blocking=True)
            sent.add(svc)
            _LOGGER.info("NOIR remote set up (Home Assistant at %s)", host)
        except Exception as err:  # noqa: BLE001 - try again next round
            _LOGGER.debug("noir_setup not sent yet: %s", err)

    @callback
    def tick(_now=None) -> None:
        # back online (reset, flash, batteries)? then host, key and channel go again
        now_online = online()
        if now_online and not was_online[0]:
            sent.difference_update({s for s in sent if s.endswith(("_noir_setup", "_noir_channel"))})
        was_online[0] = now_online
        # 1. "New device found: noir-remote" -> confirm it
        for flow in hass.config_entries.flow.async_progress_by_handler(ESPHOME):
            name = (flow.get("context", {}).get("title_placeholders") or {}).get("name", "")
            if flow.get("step_id") == "discovery_confirm" and _norm(device) in _norm(name) and flow["flow_id"] not in busy:
                busy.add(flow["flow_id"])
                hass.async_create_task(confirm(flow["flow_id"]))
        # 2. actions allowed (once per start, so you can still switch it off yourself)
        if "allowed" not in sent and allow_actions(hass, None, device):
            sent.add("allowed")
        # 3. host + key to the remote (firmware 0.3+ has the action)
        svc = f"{device.replace('-', '_')}_noir_setup"
        if svc not in sent and hass.services.has_service(ESPHOME, svc):
            hass.async_create_task(setup_remote(svc))

        # 4. update channel (firmware 0.7+ has the action)
        chan = f"{device.replace('-', '_')}_noir_channel"
        if chan not in sent and hass.services.has_service(ESPHOME, chan):
            sent.add(chan)
            hass.async_create_task(hass.services.async_call(ESPHOME, chan, {"url": update_url()}, blocking=False))

    tick()
    return async_track_time_interval(hass, tick, INTERVAL)
