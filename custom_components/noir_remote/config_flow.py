"""Setup: Settings > Devices & services > Add integration > NOIR Remote.

Generates all keys the remote needs and shows them as one block for secrets.yaml - nothing to invent yourself.
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


def secrets_block(d: dict, host: str) -> str:
    """The complete secrets.yaml for the remote - only the two Wi-Fi lines are yours to fill in."""
    return (f'wifi_ssid: "your Wi-Fi name"\n'
            f'wifi_password: "your Wi-Fi password"\n'
            f'noir_device: "{d.get("device", "noir-remote")}"\n'
            f'noir_host: "{host}"\n'
            f'noir_key: "{d["key"]}"\n'
            f'noir_api_key: "{d["api_key"]}"\n'
            f'noir_ota: "{d["ota"]}"')


class NoirFlow(config_entries.ConfigFlow, domain=DOMAIN):
    VERSION = 1

    async def async_step_user(self, user_input=None):
        if self._async_current_entries():
            return self.async_abort(reason="single_instance_allowed")
        if user_input is not None:
            self._data = {"device": user_input["device"].strip().lower().replace(" ", "-"), **make_keys()}
            return await self.async_step_keys()
        return self.async_show_form(step_id="user", data_schema=vol.Schema({
            vol.Required("device", default="noir-remote"): str,
        }))

    async def async_step_keys(self, user_input=None):
        if user_input is not None:
            return self.async_create_entry(title="NOIR Remote", data=self._data)
        return self.async_show_form(step_id="keys", data_schema=vol.Schema({}),
                                    description_placeholders={"block": secrets_block(self._data, ha_host(self.hass))})
