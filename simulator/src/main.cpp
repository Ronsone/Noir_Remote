// NOIR simulator - the remote's screens (firmware/core/noir-ui.h, the same code as the firmware) in a window on your PC
// (LVGL 9.5 + SDL2). Home Assistant, the TV and infrared are simulated: what the remote would send is printed.
//
// Mouse = finger. Keys: arrows = D-pad, Enter = OK, Backspace/Esc = Back, H = Home, space = play/pause,
// + / - / M = volume/mute, P = screenshot (noir-<n>.bmp), Q = quit.
// --demo walks every screen, saves a screenshot of each and quits. --en = English screens.

#include "lvgl.h"
#include "ArduinoJson.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

extern "C" {
extern const lv_font_t inter12, inter14, inter16, inter20, inter48, mdi20;
}
#include "noir-ui.h"
#include "noir_assets.h"
#include "logos.h"

using namespace noir;

// ---------------------------------------------------------------- sample home (as from NOIR Builder)
static const char *HOME_JSON = R"([
 {"t":"action","label":"TV","icon":"noir:tv","x":0,"y":0,"w":3,"h":2,"fn":"tv"},
 {"t":"action","label":"Audio","icon":"noir:audio","x":3,"y":0,"w":3,"h":2,"fn":"audio"},
 {"t":"action","label":"Verlichting","icon":"noir:licht","x":0,"y":2,"w":3,"h":2,"fn":"lights"},
 {"t":"action","label":"Scènes","icon":"noir:scenes","x":3,"y":2,"w":3,"h":2,"fn":"scenes"},
 {"t":"action","label":"Airco","icon":"mdi:air-conditioner","x":0,"y":4,"w":3,"h":2,"fn":"airco"},
 {"t":"action","label":"Alles uit","icon":"mdi:power","x":3,"y":4,"w":3,"h":2,"fn":"alloff"}
])";
static const char *HOME_JSON_EN = R"([
 {"t":"action","label":"TV","icon":"noir:tv","x":0,"y":0,"w":3,"h":2,"fn":"tv"},
 {"t":"action","label":"Audio","icon":"noir:audio","x":3,"y":0,"w":3,"h":2,"fn":"audio"},
 {"t":"action","label":"Lights","icon":"noir:licht","x":0,"y":2,"w":3,"h":2,"fn":"lights"},
 {"t":"action","label":"Scenes","icon":"noir:scenes","x":3,"y":2,"w":3,"h":2,"fn":"scenes"},
 {"t":"action","label":"Airco","icon":"mdi:air-conditioner","x":0,"y":4,"w":3,"h":2,"fn":"airco"},
 {"t":"action","label":"All off","icon":"mdi:power","x":3,"y":4,"w":3,"h":2,"fn":"alloff"}
])";
static const char *SCENES_JSON = R"([
 {"t":"action","label":"Film","icon":"mdi:movie-open"},
 {"t":"action","label":"Diner","icon":"mdi:silverware-fork-knife"},
 {"t":"action","label":"Avond","icon":"mdi:weather-night"}
])";
static const char *SCENES_JSON_EN = R"([
 {"t":"action","label":"Movie","icon":"mdi:movie-open"},
 {"t":"action","label":"Dinner","icon":"mdi:silverware-fork-knife"},
 {"t":"action","label":"Evening","icon":"mdi:weather-night"}
])";
static const char *LAMPS = "Woonkamer=light.woonkamer|Eettafel=light.eettafel|Keuken=light.keuken|Gang=switch.gang";
static const char *LAMPS_EN = "Living room=light.woonkamer|Dining table=light.eettafel|Kitchen=light.keuken|Hall=switch.gang";

static JsonDocument g_home, g_scenes;
static bool g_demo = false;

// ---------------------------------------------------------------- simulated Home Assistant
static void ha_print(const std::string &svc, const Data &d) {
  std::printf("[HA] %s", svc.c_str());
  for (auto &kv : d) std::printf("  %s=%s", kv.first.c_str(), kv.second.c_str());
  std::printf("\n");
}
static void ha_stub(const std::string &svc, Data d) {
  ha_print(svc, d);
  if (svc == "media_player.turn_off" && d["entity_id"] == tv::entiteit()) tv_status("off", "");
}

static void sample_audio() {
  audio::Data &x = audio::d();
  x.kef = "media_player.woonkamer"; x.muziek = x.kef; x.sub_e = "number.woonkamer_sw_gain";
  x.kef_state = "on"; x.bron = 2; x.volume = 38; x.sub = 2; x.sub_bekend = true;
  x.sp_state = "playing"; x.titel = "Midnight Drive"; x.artiest = "Nova Lane";
}
static void sample_airco() {
  airco::Data &d = airco::d();
  airco::Unit u;
  u.naam = english() ? "Living room" : "Woonkamer";
  u.aan = true; u.modus = 1; u.temp = 21; u.kamer = 23;
  u.fans = airco::mhi::fans();
  u.swings = airco::mhi::swings();
  u.tmin = 18; u.tmax = 30;
  d.units = {u};
  d.cur = 0;
  airco::wijziging() = [](int i) {
    const airco::Unit &n = airco::d().units[i];
    std::printf("[IR] Mitsubishi Heavy: %s mode %d %d C fan %d swing %d\n", n.aan ? "on" : "off", n.modus, n.temp, n.fan, n.swing);
  };
}

// ---------------------------------------------------------------- actions (as noirapp::action in the firmware)
static void run_tile(int i) {
  JsonArrayConst items = g_home.as<JsonArrayConst>();
  if (i < 0 || i >= (int) items.size()) return;
  const std::string fn = items[i]["fn"] | "";
  extern void action(int a);
  if (fn == "tv") action(TV);
  else if (fn == "audio") action(AUDIO);
  else if (fn == "lights") action(LICHT);
  else if (fn == "scenes") action(SCENES);
  else if (fn == "airco") action(AIRCO);
  else if (fn == "alloff") action(SCENE_ALLES_UIT);
  else std::printf("[remote] tile %d: %s\n", i, fn.c_str());
}

void action(int a) {
  if (a >= SCENE && a < SCENE + 40) {
    JsonArrayConst it = g_scenes.as<JsonArrayConst>();
    if (a - SCENE < (int) it.size()) toast(it[a - SCENE]["label"] | "Scene");
    return;
  }
  if (a >= ITEM) { run_tile(a - ITEM); return; }
  if (a >= AUDIO_BRON && a < AUDIO_BRON + audio::BRONNEN) { audio::bron_kiezen(a - AUDIO_BRON); return; }
  if (a >= AIRCO_MODUS && a < AIRCO_MODUS + airco::MODI) { airco::modus(a - AIRCO_MODUS); return; }
  if (a >= LAMP && a < LAMP + 12) { lamp_schakelen(a - LAMP, lv_tick_get()); return; }
  if (a >= TV_APP && a < TV_APP + (int) tv_apps().size()) {
    const TvApp &app = tv_apps()[a - TV_APP];
    if (!tv_aan()) tv_status("on", "");
    ha_print("media_player.select_source", {{"source", app.source}});
    tv_status("on", app.source);
    toast(app.name.c_str());
    return;
  }
  switch (a) {
    case TV: zeigen_seite(S_TV); break;
    case TV_AANUIT:
      if (tv_aan()) { ha_print("media_player.turn_off", {{"entity_id", tv::entiteit()}}); tv_status("off", ""); toast("TV uit"); }
      else { tv_status("on", "Live TV"); toast("TV aan"); }
      break;
    case AUDIO: zeigen_seite(S_AUDIO); break;
    case AUDIO_AANUIT: audio::aanuit(); break;
    case AUDIO_NU: ha_print("media_player.media_play_pause", {{"entity_id", audio::d().muziek}}); break;
    case LICHT: zeigen_seite(S_LICHT); break;
    case SCENES: zeigen_seite(S_SCENES); break;
    case SCENE_ALLES_UIT:
      for (auto &l : lampen()) l.state = "off";
      lampen_tekst();
      ha_print("light.turn_off", {{"entity_id", "all lamps"}});
      toast("Alles uit");
      break;
    case AIRCO: zeigen_seite(S_AIRCO); break;
    case AIRCO_AANUIT: airco::aanuit(); break;
    case AIRCO_MIN: airco::temp_stap(-1); break;
    case AIRCO_PLUS: airco::temp_stap(1); break;
    case AIRCO_FAN: airco::fan_volgende(); break;
    case AIRCO_SWING: airco::swing_volgende(); break;
    case AIRCO_UNIT: airco::unit_volgende(); break;
    case KNOPTEST: knoptest::reset(); zeigen_seite(S_TEST); break;
    case TEST_RESET: knoptest::reset(); break;
    case INSTELLINGEN: zeigen_seite(S_INST); break;
    case INST_FULL: case INST_ECO: eco_mode() = a == INST_ECO; alles_herbouwen(); break;
    case INST_WIFI: toast("Reconnecting Wi-Fi\xE2\x80\xA6"); break;
    case DIM_GEREED: dimmer_dicht(); break;
    case TERUG:
      if (keuze().over != nullptr) keuze_dicht();
      else if (st().seite != S_THUIS) zeigen_seite(S_THUIS);
      break;
    case THUIS: zeigen_seite(S_THUIS); break;
    default: std::printf("[remote] action %d\n", a); break;
  }
}

// ---------------------------------------------------------------- hardware keys (Rev6 key codes)
static void key(int k) {
  for (bool down : {true, false}) {
    if (kef::taste(k, down)) continue;   // volume / mute -> speaker
    if (!down) continue;
    if (st().seite == S_TEST) { knoptest::taste(k, true); knoptest::taste(k, false); continue; }
    if (k == 13) action(TERUG);
    else if (k == 41 || k == 3) action(THUIS);
    else if (st().seite == S_AIRCO && (k == 14 || k == 35)) airco::temp_stap(k == 14 ? 1 : -1);
    else if (k == 2) action(AUDIO_NU);
    else std::printf("[remote] key %d (D-pad: see the firmware's module Nav)\n", k);
  }
}

static int g_shot = 0;
static void screenshot(const char *name = nullptr) {
  lv_draw_buf_t *b = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_ARGB8888);
  if (b == nullptr) { std::printf("[sim] screenshot failed (memory)\n"); return; }
  char fn[64];
  if (name) std::snprintf(fn, sizeof(fn), "%s", name);
  else std::snprintf(fn, sizeof(fn), "noir-%d.bmp", ++g_shot);
  FILE *f = std::fopen(fn, "wb");
  if (f != nullptr) {
    const int w = b->header.w, h = b->header.h, stride = b->header.stride, size = 54 + w * h * 4;
    unsigned char hd[54] = {'B', 'M'};
    auto le = [&](int o, int v) { for (int i = 0; i < 4; i++) hd[o + i] = (v >> (8 * i)) & 255; };
    le(2, size); le(10, 54); le(14, 40); le(18, w); le(22, -h); hd[26] = 1; hd[28] = 32; le(34, w * h * 4);
    std::fwrite(hd, 1, 54, f);
    for (int y = 0; y < h; y++) std::fwrite(b->data + y * stride, 1, w * 4, f);
    std::fclose(f);
    std::printf("[sim] screenshot %s\n", fn);
  }
  lv_draw_buf_destroy(b);
}

static void key_cb(lv_event_t *e) {
  switch (lv_event_get_key(e)) {
    case LV_KEY_UP: key(14); break;
    case LV_KEY_DOWN: key(35); break;
    case LV_KEY_LEFT: key(15); break;
    case LV_KEY_RIGHT: key(32); break;
    case LV_KEY_ENTER: case '\r': key(34); break;
    case LV_KEY_BACKSPACE: case LV_KEY_ESC: key(13); break;
    case 'h': case 'H': key(41); break;
    case ' ': key(2); break;
    case '+': case '=': key(33); break;
    case '-': key(43); break;
    case 'm': case 'M': key(44); break;
    case 'p': case 'P': screenshot(); break;
    case 'q': case 'Q': std::exit(0);
    default: break;
  }
}

static void clock_cb(lv_timer_t *) {
  std::time_t t = std::time(nullptr);
  std::tm *l = std::localtime(&t);
  status(l->tm_hour, l->tm_min, 78, false, true, -60);
  herhaal_tik(lv_tick_get());
}

// --demo: every screen once, a screenshot of each
static void demo_cb(lv_timer_t *tm) {
  static int step = 0;
  struct S { int action; const char *file; };
  static const S steps[] = {{THUIS, "demo-01-home.bmp"}, {TV, "demo-02-tv.bmp"}, {LICHT, "demo-03-lights.bmp"},
                            {SCENES, "demo-04-scenes.bmp"}, {AUDIO, "demo-05-audio.bmp"}, {-1, "demo-05b-audio-bottom.bmp"},
                            {AIRCO, "demo-06-airco.bmp"}, {INSTELLINGEN, "demo-07-settings.bmp"}, {KNOPTEST, "demo-08-buttontest.bmp"}};
  const int n = sizeof(steps) / sizeof(steps[0]);
  if (step > 0) screenshot(steps[step - 1].file);
  if (step >= n) { lv_timer_delete(tm); std::exit(0); }
  if (steps[step].action == -1) lv_obj_scroll_to_y(st().audio_body, 200, LV_ANIM_OFF);
  else action(steps[step].action);
  if (steps[step].action == LICHT) action(LAMP + 0);
  if (steps[step].action == KNOPTEST) { key(14); key(41); }
  step++;
}

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--demo")) g_demo = true;
    if (!strcmp(argv[i], "--en")) english() = true;   // English screens
  }
  lv_init();
  lv_display_t *disp = lv_sdl_window_create(240, 320);
  lv_sdl_window_set_zoom(disp, 2);
  lv_sdl_window_set_title(disp, "NOIR simulator");
  lv_sdl_mouse_create();
  lv_indev_t *kb = lv_sdl_keyboard_create();
  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
  lv_obj_set_style_pad_all(scr, 12, 0);   // like page_home in the firmware
  lv_obj_set_style_pad_top(scr, 10, 0);

  // keys: an invisible object in a group catches them
  lv_group_t *g = lv_group_create();
  lv_obj_t *catcher = lv_obj_create(lv_layer_sys());
  lv_obj_set_size(catcher, 0, 0);
  lv_obj_add_event_cb(catcher, key_cb, LV_EVENT_KEY, nullptr);
  lv_group_add_obj(g, catcher);
  lv_indev_set_group(kb, g);

  // the outside world
  ha() = ha_stub;
  tv::entiteit() = "media_player.lg_tv";
  kef::entiteit() = "media_player.woonkamer";
  kef::melding() = [](const char *t) { toast(t); };
  st().actie = action;
  deserializeJson(g_home, english() ? HOME_JSON_EN : HOME_JSON);
  deserializeJson(g_scenes, english() ? SCENES_JSON_EN : SCENES_JSON);
  st().items = []() { return g_home.as<JsonArrayConst>(); };
  st().scenes_items = []() { return g_scenes.as<JsonArrayConst>(); };
  lampen_uit_tekst(english() ? LAMPS_EN : LAMPS);
  for (auto &l : lampen()) { l.state = "off"; l.bri = -1; }
  sample_audio();
  sample_airco();

  Fonts f{&inter12, &inter14, &inter16, &mdi20, &inter20, &inter48};
  Beelden bl;
  bl.wall = &wall;
  bl.tv = &ic_tv; bl.audio = &ic_audio; bl.licht = &ic_light; bl.scenes = &ic_scenes;
  sim_logos();
  tv_apps() = {{"PS5", "PS5 Gameconsole", "playstation"}, {"Netflix", "Netflix", "netflix"}, {"YouTube", "YouTube", "youtube"},
               {"Xbox", "HDMI 3", "game"}, {"Live TV", "Live TV", "tv"}, {"Spotify", "Spotify", "spotify"}};
  st().toast_font = &inter14;
  bouwen(scr, f, bl, g_home.as<JsonArrayConst>());
  tv_status("on", "PS5 Gameconsole");
  audio::tonen();
  airco::tonen();
  zeigen(true);
  zeigen_seite(S_THUIS);

  lv_timer_create(clock_cb, 1000, nullptr);
  clock_cb(nullptr);
  if (g_demo) lv_timer_create(demo_cb, 400, nullptr);
  std::printf("NOIR simulator: mouse = finger, arrows/Enter/Backspace = D-pad/OK/Back, H = Home,\n"
              "space = play/pause, +/-/M = volume, P = screenshot, Q = quit.\n");
  for (;;) {
    const uint32_t wait = lv_timer_handler();
    lv_delay_ms(wait < 5 ? wait : 5);
  }
}
