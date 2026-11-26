"""NOIR Remote - Home Assistant side of the NOIR firmware.

- NOIR Builder (sidebar panel): build the screens of the remote
- GET /api/noir_remote/config?k=<key>   configuration for the remote (JSON)
- GET /api/noir_remote/states?k=<key>&e=a,b   states of lamps / TV for the remote
Removing the integration never deletes your screens: they stay in .storage/noir_remote until you delete that file.
"""
from __future__ import annotations

import hmac
import json
import logging
import os

import voluptuous as vol
from aiohttp import web

from homeassistant.components import frontend, panel_custom, websocket_api
from homeassistant.components.http import HomeAssistantView, StaticPathConfig
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.storage import Store
from homeassistant.util import dt as dt_util

from . import link
from .autoadd import maybe_add
from .const import AIRCO_IR, DOMAIN, LANG_PACKS, LANGUAGES, PROFILES, STORE_VERSION, UPDATE_URLS
from .model import normalize, payload
from .tv import guess_logo, logo_list

_LOGGER = logging.getLogger(__name__)
PANEL = "noir-builder"
STATIC = "/noir_remote_static"


class NoirData:
    def __init__(self, hass: HomeAssistant, entry: ConfigEntry) -> None:
        self.hass = hass
        self.entry = entry
        self.store = Store(hass, STORE_VERSION, DOMAIN)
        self.model: dict = {}
        self.version = ""

    @property
    def key(self) -> str:
        return self.entry.data.get("key", "")

    @property
    def device(self) -> str:
        return self.entry.data.get("device", "noir-remote")

    async def load(self) -> None:
        data = await self.store.async_load() or {}
        self.model = normalize(data.get("model"))
        self.version = data.get("version") or dt_util.now().isoformat(timespec="seconds")

    def update_url(self) -> str:
        return UPDATE_URLS.get(self.model.get("channel", "stable"), "")

    async def save(self, model: dict) -> None:
        old_url = self.update_url()
        self.model = normalize(model)
        if self.update_url() != old_url:   # other update channel: tell the remote right away
            svc = f"{self.device.replace('-', '_')}_noir_channel"
            if self.hass.services.has_service("esphome", svc):
                try:
                    await self.hass.services.async_call("esphome", svc, {"url": self.update_url()}, blocking=False)
                except Exception as err:  # noqa: BLE001
                    _LOGGER.warning("Could not set the update channel on the remote: %s", err)
        self.version = dt_util.now().isoformat(timespec="seconds")
        await self.store.async_save({"model": self.model, "version": self.version})
        # Tell the remote: ESPHome action noir_reload (esphome.<device>_noir_reload)
        svc = f"{self.device.replace('-', '_')}_noir_reload"
        if self.hass.services.has_service("esphome", svc):
            try:
                await self.hass.services.async_call("esphome", svc, {}, blocking=False)
            except Exception as err:  # noqa: BLE001
                _LOGGER.warning("Could not tell the remote to reload: %s", err)
        else:
            _LOGGER.info("esphome.%s not found - the remote picks up the new screens within 15 min or after a restart", svc)

    def payload(self) -> dict:
        p = payload(self.model, self.version)
        sp = self.hass.states.get(p["audio"]["speaker"]) if p["audio"]["speaker"] else None
        if sp is not None:   # name on the Audio screen: the speaker as Home Assistant calls it
            p["audio"]["name"] = str(sp.attributes.get("friendly_name") or "")[:24]
        return p


def _key_ok(request: web.Request, key: str) -> bool:
    return bool(key) and hmac.compare_digest(request.query.get("k", ""), key)


class ConfigView(HomeAssistantView):
    url = "/api/noir_remote/config"
    name = "api:noir_remote:config"
    requires_auth = False   # the remote has no HA token; it uses the shared key

    async def get(self, request: web.Request) -> web.Response:
        nd: NoirData | None = request.app["hass"].data.get(DOMAIN)
        if nd is None or not _key_ok(request, nd.key):
            return web.Response(status=403)
        if request.remote:   # the key proves this remote has our secrets.yaml: add it to ESPHome by itself
            hass = request.app["hass"]
            hass.async_create_task(maybe_add(hass, nd.device, request.remote, nd.entry.data.get("api_key", "")))
        return web.Response(body=json.dumps(nd.payload(), separators=(",", ":"), ensure_ascii=False).encode(),
                            content_type="application/json", headers={"Cache-Control": "no-store"})


class StatesView(HomeAssistantView):
    """{"entity": [state, brightness 0-255|-1, volume 0-100|-1, unit, title, artist, muted, shuffle, repeat, app, source]}"""
    url = "/api/noir_remote/states"
    name = "api:noir_remote:states"
    requires_auth = False

    async def get(self, request: web.Request) -> web.Response:
        hass = request.app["hass"]
        nd: NoirData | None = hass.data.get(DOMAIN)
        if nd is None or not _key_ok(request, nd.key):
            return web.Response(status=403)
        out = {}
        for e in request.query.get("e", "").split(",")[:40]:
            st = hass.states.get(e.strip()) if e.strip() else None
            if st is None:
                continue
            a = st.attributes
            bri = a.get("brightness")
            vol_ = a.get("volume_level")
            row = [st.state, int(bri) if isinstance(bri, (int, float)) else -1,
                   round(vol_ * 100) if isinstance(vol_, (int, float)) else -1, str(a.get("unit_of_measurement") or "")]
            if st.entity_id.startswith("media_player."):
                row += [str(a.get("media_title") or "")[:80], str(a.get("media_artist") or a.get("app_name") or "")[:60],
                        bool(a.get("is_volume_muted")), bool(a.get("shuffle")), str(a.get("repeat") or ""),
                        str(a.get("app_name") or "")[:40], str(a.get("source") or "")[:40], -1.0, 0.0, 0.0]
            elif st.entity_id.startswith("climate."):   # NOIR Airco: everything the unit offers
                num = lambda x: float(x) if isinstance(x, (int, float)) else -1000.0
                row += [num(a.get("temperature")), num(a.get("current_temperature")),
                        str(a.get("fan_mode") or ""), str(a.get("swing_mode") or ""),
                        num(a.get("min_temp")), num(a.get("max_temp")),
                        "|".join(str(x) for x in (a.get("hvac_modes") or []))[:120],
                        "|".join(str(x) for x in (a.get("fan_modes") or [])[:8])[:120],
                        "|".join(str(x) for x in (a.get("swing_modes") or [])[:6])[:120]]
            out[st.entity_id] = row
        return web.Response(body=json.dumps(out, separators=(",", ":"), ensure_ascii=False).encode(),
                            content_type="application/json", headers={"Cache-Control": "no-store"})


@websocket_api.websocket_command({vol.Required("type"): "noir_remote/get"})
@websocket_api.require_admin
@websocket_api.async_response
async def ws_get(hass, connection, msg):
    nd: NoirData = hass.data[DOMAIN]
    profiles = [{"key": k, "name": v["name"], "tested": v.get("tested", "")} for k, v in PROFILES.items()]
    connection.send_result(msg["id"], {"model": nd.model, "version": nd.version, "device": nd.device,
                                       "profiles": profiles, "logos": logo_list(),
                                       "airco_ir": [{"key": k, "name": v} for k, v in AIRCO_IR.items()],
                                       "languages": [{"code": k, "name": v} for k, v in LANGUAGES.items()],
                                       "packs": LANG_PACKS})


@websocket_api.websocket_command({vol.Required("type"): "noir_remote/logo", vol.Required("names"): [str]})
@websocket_api.require_admin
@websocket_api.callback
def ws_logo(hass, connection, msg):
    """Best logo for each source name (NOIR Builder fills new sources with it)."""
    connection.send_result(msg["id"], {"logos": [guess_logo(n) for n in msg["names"][:40]]})


@websocket_api.websocket_command({vol.Required("type"): "noir_remote/test", vol.Required("profile"): str,
                                  vol.Required("key"): str})
@websocket_api.require_admin
@websocket_api.async_response
async def ws_test(hass, connection, msg):
    """NOIR Builder "Test": the remote sends one infrared code of a profile (e.g. power_on)."""
    nd: NoirData = hass.data[DOMAIN]
    p = PROFILES.get(msg["profile"])
    code = (p or {}).get("codes", {}).get(msg["key"])
    svc = f"{nd.device.replace('-', '_')}_noir_ir"
    if p is None or code is None:
        connection.send_error(msg["id"], "unknown", "Unknown profile or key")
    elif not hass.services.has_service("esphome", svc):
        connection.send_error(msg["id"], "no_remote", "Remote not found in ESPHome")
    else:
        await hass.services.async_call("esphome", svc, {"protocol": p["protocol"], "address": int(p["address"]),
                                                        "command": int(code)}, blocking=True)
        connection.send_result(msg["id"], {"sent": True})


@websocket_api.websocket_command({vol.Required("type"): "noir_remote/save", vol.Required("model"): dict})
@websocket_api.require_admin
@websocket_api.async_response
async def ws_save(hass, connection, msg):
    nd: NoirData = hass.data[DOMAIN]
    await nd.save(msg["model"])
    connection.send_result(msg["id"], {"version": nd.version})


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    nd = NoirData(hass, entry)
    await nd.load()
    first = DOMAIN not in hass.data
    hass.data[DOMAIN] = nd

    async def set_mode(call) -> None:   # the remote's own settings screen: Full / Eco
        cur: NoirData = hass.data[DOMAIN]
        await cur.save({**cur.model, "mode": "eco" if call.data.get("mode") == "eco" else "full"})

    hass.services.async_register(DOMAIN, "set_mode", set_mode, schema=vol.Schema({vol.Required("mode"): vol.In(["full", "eco"])}))
    if first:   # views and paths can't be unregistered - only once per HA start
        hass.http.register_view(ConfigView())
        hass.http.register_view(StatesView())
        websocket_api.async_register_command(hass, ws_get)
        websocket_api.async_register_command(hass, ws_save)
        websocket_api.async_register_command(hass, ws_logo)
        websocket_api.async_register_command(hass, ws_test)
        await hass.http.async_register_static_paths(
            [StaticPathConfig(STATIC, os.path.join(os.path.dirname(__file__), "frontend"), cache_headers=False)])
    from .config_flow import ha_host
    entry.async_on_unload(link.start(hass, nd.device, ha_host, nd.key, lambda: hass.data[DOMAIN].update_url()))   # the remote links itself
    if PANEL not in hass.data.get("frontend_panels", {}):
        # cache-buster follows the file itself, so a new Builder always loads after an update
        js = os.path.join(os.path.dirname(__file__), "frontend", "noir-builder.js")
        stamp = int(await hass.async_add_executor_job(os.path.getmtime, js))
        await panel_custom.async_register_panel(
            hass, webcomponent_name="noir-builder", frontend_url_path=PANEL,
            module_url=f"{STATIC}/noir-builder.js?v={stamp}", sidebar_title="NOIR Builder",
            sidebar_icon="mdi:view-grid-outline", require_admin=True)
    return True


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    frontend.async_remove_panel(hass, PANEL)
    return True
