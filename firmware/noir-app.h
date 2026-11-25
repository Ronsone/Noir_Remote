#pragma once
// NOIR Remote - app layer: connects the NOIR screens (noir-ui.h) to the hardware and Home Assistant.
//
// Configuration comes from the NOIR Builder (integration noir_remote):
//   GET http://<ha>/api/noir_remote/config?k=<key>  ->  {version, mode, home:[...], lights:[...], scenes:[...], tv:{...}}
// The last configuration is kept in NVS, so the remote works without Home Assistant after a restart.
//
// Features are modules (modules/*.h). To add one: write modules/<name>.h with a struct derived from Module,
// include it below and add it to the list in start(). Order matters: the first module that handles an action
// or key wins.

#include "modules/core.h"
#include "modules/home.h"
#include "modules/lights.h"
#include "modules/scenes.h"
#include "modules/tv.h"

namespace noirapp {

inline void start(const char *host, const char *key) {
  static Tv tv;
  static Lights lights;
  static Scenes scenes;
  static Home home;
  modules() = {&tv, &lights, &scenes, &home};   // tv first: on the TV screen its keys win

  App &a = app();
  a.host = host;
  a.key = key;
  a.last_input = millis();
  noir::ha() = [](const std::string &svc, noir::Data d) { ha(svc, d); };
  noir::st().actie = [](int x) { action(x); };
  noir::st().aan = true;
  noir::st().items = []() { return list_of("home"); };
  noir::st().scenes_items = []() { return list_of("scenes"); };
  noir::kef::melding() = [](const char *t) { noir::toast(t); };
  noir::licht_http::url_basis() = []() -> std::string {
    if (app().host.empty() || app().key.empty()) return "";
    return "http://" + app().host + "/api/noir_remote/states?k=" + app().key;
  };
  nvs_load();   // last known screens, so the remote works right away
}

struct Images { const esphome::image::Image *wall, *tv, *audio, *licht, *scenes; };
// Logos for the TV source tiles (registered by logos.yaml, before build)
inline void logo(const char *key, const esphome::image::Image *img) { noir::logos()[key] = img->get_lv_image_dsc(); }
inline void build(lv_obj_t *home, const noir::Fonts &f, const Images &b) {
  noir::Beelden bl;
  bl.wall = b.wall->get_lv_image_dsc();
  bl.tv = b.tv->get_lv_image_dsc(); bl.audio = b.audio->get_lv_image_dsc();
  bl.licht = b.licht->get_lv_image_dsc(); bl.scenes = b.scenes->get_lv_image_dsc();
  noir::st().toast_font = f.f14;
  noir::bouwen(home, f, bl, list_of("home"));
  app().built = true;
  noir::tv_status("", "");
  noir::zeigen(true);
  noir::zeigen_seite(noir::S_THUIS);
  fetch_config();
}

inline void touched() { app().last_input = millis(); app().wake = true; }

inline void key(int k, bool pressed, bool held_long) {
  (void) held_long;
  App &a = app();
  const bool was_dark = !screen_on();
  a.last_input = millis();
  a.wake = true;
  if (pressed && was_dark && k != 102 && k != 33 && k != 43 && k != 44) return;   // first press only wakes the display
  for (Module *m : modules()) if (m->key(k, pressed)) return;
}

// Every 100 ms
inline void tick(uint32_t now, int hh, int mm, int pct, bool ha_ok, int rssi) {
  download_check_();
  for (Module *m : modules()) m->tick(now);
  static uint32_t last = 0, cfg_time = 0;
  static bool api_before = false;
  if (ha_ok && !api_before) fetch_config();   // (re)connected to HA: fetch the screens
  api_before = ha_ok;
  if (now - cfg_time > 15UL * 60 * 1000) { cfg_time = now; fetch_config(); }
  if (now - last < 1000) return;
  last = now;
  noir::status(hh, mm, pct, false, ha_ok, rssi);
}

}  // namespace noirapp
