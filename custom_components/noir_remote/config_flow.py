"""Setup: Settings > Devices & services > Add integration > NOIR Remote.

Generates the shared key the remote uses for its screens; the remote gets it by itself (link.py).
"""
from __future__ import annotations

import base64
import secrets

import voluptuous as vol

from homeassistant import config_entries

from .const import DOMAIN


def make_keys() -> dict:
    return {
        "key": secrets.token_urlsafe(18),                                   # remote <-> NOIR Builder
        "api_key": base64.b64encode(secrets.token_bytes(32)).decode(),     # ESPHome API encryption
        "ota": secrets.token_urlsafe(12),                                   # wireless updates
    }


def ha_host(hass) -> str:
    """IP:port of Home Assistant as the remote must use it (the remote can't resolve names like .local)."""
    api = getattr(hass.config, "api", None)
    if api is not None and getattr(api, "local_ip", None):
        return f"{api.local_ip}:{api.port}"
    return "192.168.x.x:8123"


class NoirFlow(config_entries.ConfigFlow, domain=DOMAIN):
    VERSION = 1

    async def async_step_user(self, user_input=None):
        if self._async_current_entries():
            return self.async_abort(reason="single_instance_allowed")
        if user_input is not None:
            device = user_input["device"].strip().lower().replace(" ", "-")
            return self.async_create_entry(title="NOIR Remote", data={"device": device, **make_keys()})
        return self.async_show_form(step_id="user", data_schema=vol.Schema({
            vol.Required("device", default="noir-remote"): str,
        }))
