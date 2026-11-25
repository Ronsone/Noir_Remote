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
#include "modules/audio.h"
#include "modules/airco.h"
#include "modules/nav.h"

namespace noirapp {

inline void start(const std::string &host, const std::string &key) {   // host + key: kept in flash (noir_setup)
  static Airco airco;
  static Nav nav;
  static Audio audio;
  static Tv tv;
  static Lights lights;
  static Scenes scenes;
  static Home home;
  modules() = {&airco, &audio, &tv, &lights, &scenes, &home, &nav};   // screen modules first (their keys on their screen), audio before tv

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
  if (a.host.empty()) ESP_LOGI("noir", "Not set up yet - waiting for NOIR Remote in Home Assistant (action noir_setup)");
}
// From NOIR Remote (action noir_setup): where Home Assistant is + the shared key. Fetch the screens right away.
inline void setup(const std::string &host, const std::string &key) {
  App &a = app();
  const bool changed = host != a.host || key != a.key;
  a.host = host;
  a.key = key;
  ESP_LOGI("noir", "Set up by Home Assistant (%s)", host.c_str());
  if (changed && a.built) fetch_config();
}

struct Images { esphome::image::Image *wall, *tv, *audio, *licht, *scenes; };
// Logos for the TV source tiles (registered by logos.yaml, before build)
inline void logo(const char *key, esphome::image::Image *img) { noir::logos()[key] = img->get_lv_image_dsc(); }
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
  noir::airco::tonen();
  fetch_config();
}

inline void touched() { app().last_input = millis(); app().wake = true; }

// ---- key filter: the Rev6 matrix sometimes reports a key you did not press ----------------------------------
// Seen in the logs: Home (41) "pressed" for seconds on its own, and a neighbour key coming along with a real press.
//  - Home, Back, Menu change the screen: they act on release, only after a normal short press (80-800 ms; a real press takes about 200 ms) during
//    which no other key went down. A key that hangs or flickers does nothing.
//  - Other keys act right away (arrows and volume repeat while held), but a second key that goes down within 80 ms of
//    the first is taken as a ghost of the first and ignored - with its release.
//  - The same key again within 120 ms of its release is contact bounce: ignored.
struct KeyFilter {
  uint32_t down_ms[128]{};
  uint32_t up_ms[128]{};   // last release (bounce: the same key again within 120 ms)
  bool passed[128]{};      // press was passed on, so pass its release too
  bool spoiled[128]{};     // a slow key during which another key went down
  int last_key{0};
  uint32_t last_ms{0};
};
inline KeyFilter &kf() { static KeyFilter f; return f; }
inline bool slow_key(int k) { return k == 41 || k == 13 || k == 3; }

inline void key_to_modules_(int k, bool pressed) {
  for (Module *m : modules()) if (m->key(k, pressed)) return;
}

inline void key(int k, bool pressed, bool held_long) {
  (void) held_long;
  App &a = app();
  const bool was_dark = !screen_on();
  a.last_input = millis();
  a.wake = true;
  // Button test (Settings): every key shows its name and number unfiltered, so ghosts are visible; Terug leaves it
  if (noir::st().seite == noir::S_TEST && k != 102) { noir::knoptest::taste(k, pressed); return; }
  if (k < 1 || k > 127) { key_to_modules_(k, pressed); return; }   // power key (GPI) etc.
  KeyFilter &f = kf();
  const uint32_t now = millis();
  if (pressed) {
    for (int i = 1; i < 128; i++) if (i != k && slow_key(i) && f.down_ms[i] != 0) f.spoiled[i] = true;   // another key during a slow key
    if (f.up_ms[k] != 0 && now - f.up_ms[k] < 120) {   // nobody presses that fast: contact bounce
      ESP_LOGD("noir", "Key %d ignored: bounce", k);
      f.passed[k] = false;
      f.down_ms[k] = 0;
      return;
    }
    f.down_ms[k] = now;
    f.passed[k] = false;
    f.spoiled[k] = false;
    if (f.last_key != 0 && f.last_key != k && now - f.last_ms < 80) {
      ESP_LOGD("noir", "Key %d ignored: within 80 ms of key %d", k, f.last_key);
      return;
    }
    f.last_key = k;
    f.last_ms = now;
    if (was_dark && k != 102 && k != 33 && k != 43 && k != 44) return;   // first press only wakes the display (power and volume work dark)
    if (slow_key(k)) return;                                 // acts on release
    f.passed[k] = true;
    key_to_modules_(k, true);
    return;
  }
  // released
  f.up_ms[k] = now;
  const uint32_t held = f.down_ms[k] != 0 ? now - f.down_ms[k] : 0;
  f.down_ms[k] = 0;
  if (slow_key(k) && f.last_key == k) {
    if (held >= 80 && held <= 800 && !f.spoiled[k] && !was_dark) { key_to_modules_(k, true); key_to_modules_(k, false); }
    else ESP_LOGD("noir", "Key %d ignored: held %u ms%s", k, (unsigned) held, f.spoiled[k] ? ", other key during it" : "");
    return;
  }
  if (f.passed[k]) { f.passed[k] = false; key_to_modules_(k, false); }
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
