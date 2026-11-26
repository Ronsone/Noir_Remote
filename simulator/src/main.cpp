// NOIR simulator - the remote's screens (firmware/core/noir-ui.h, the same code as the firmware) in a window on your PC
// (LVGL 9.5 + SDL2). Home Assistant, the TV and infrared are simulated: what the remote would send is printed.
//
// Mouse = finger. Keys: arrows = D-pad, Enter = OK, Backspace/Esc = Back, H = Home, space = play/pause,
// + / - / M = volume/mute, P = screenshot (noir-<n>.bmp), Q = quit.
// --demo walks every screen, saves a screenshot of each and quits. --en = English screens.
// --size WxH = another screen (e.g. 800x480, 1280x800): the layout follows the size (columns, scale).
// --grid = a Home with weather, calendar and value tiles too (sample data). --clock HH:MM = fixed time (screenshots).

#include "lvgl.h"
#include "ArduinoJson.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

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
// --grid: the same tiles plus weather, calendar and values in their sizes (S, W, L)
static const char *GRID_JSON = R"([
 {"t":"weather","entity":"weather.thuis","size":"W"},
 {"t":"calendar","entity":"calendar.gezin","size":"L"},
 {"t":"action","label":"TV","icon":"noir:tv","fn":"tv","size":"S"},
 {"t":"action","label":"Audio","icon":"noir:audio","fn":"audio","size":"S"},
 {"t":"action","label":"Verlichting","icon":"noir:licht","fn":"lights","size":"S"},
 {"t":"action","label":"Scènes","icon":"noir:scenes","fn":"scenes","size":"S"},
 {"t":"action","label":"Airco","icon":"mdi:air-conditioner","fn":"airco","size":"W"},
 {"t":"value","label":"BTC","entity":"sensor.btc_eur","size":"S"},
 {"t":"value","label":"ETH","entity":"sensor.eth_eur","size":"S"},
 {"t":"value","label":"AEX","entity":"sensor.aex","size":"W"},
 {"t":"action","label":"Alles uit","icon":"mdi:power","fn":"alloff","size":"S"}
])";
static const char *GRID_JSON_EN = R"([
 {"t":"weather","entity":"weather.thuis","size":"W"},
 {"t":"calendar","entity":"calendar.gezin","size":"L"},
 {"t":"action","label":"TV","icon":"noir:tv","fn":"tv","size":"S"},
 {"t":"action","label":"Audio","icon":"noir:audio","fn":"audio","size":"S"},
 {"t":"action","label":"Lights","icon":"noir:licht","fn":"lights","size":"S"},
 {"t":"action","label":"Scenes","icon":"noir:scenes","fn":"scenes","size":"S"},
 {"t":"action","label":"Airco","icon":"mdi:air-conditioner","fn":"airco","size":"W"},
 {"t":"value","label":"BTC","entity":"sensor.btc_eur","size":"S"},
 {"t":"value","label":"ETH","entity":"sensor.eth_eur","size":"S"},
 {"t":"value","label":"AEX","entity":"sensor.aex","size":"W"},
 {"t":"action","label":"All off","icon":"mdi:power","fn":"alloff","size":"S"}
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
static bool g_demo = false, g_grid = false;
static int g_w = 240, g_h = 320, g_hh = -1, g_mm = 0;

// ---------------------------------------------------------------- Home Assistant
// In the browser (phone, HA panel /noir) actions go to the real Home Assistant through JavaScript (noir-phone.js);
// on the PC they are printed.
#ifdef __EMSCRIPTEN__
EM_JS(void, js_ha, (const char *svc, const char *json), {
  if (Module.noirHa) Module.noirHa(UTF8ToString(svc), JSON.parse(UTF8ToString(json)));
});
static bool g_web = true;
#else
static void js_ha(const char *, const char *) {}
static bool g_web = false;
#endif
static void ha_send(const std::string &svc, const Data &d) {
  JsonDocument j;
  for (auto &kv : d) j[kv.first] = kv.second;
  std::string out;
  serializeJson(j, out);
  js_ha(svc.c_str(), out.c_str());
}
static void ha_print(const std::string &svc, const Data &d) {
  if (g_web) { ha_send(svc, d); return; }
  std::printf("[HA] %s", svc.c_str());
  for (auto &kv : d) std::printf("  %s=%s", kv.first.c_str(), kv.second.c_str());
  std::printf("\n");
}
static void ha_stub(const std::string &svc, Data d) {
  ha_print(svc, d);
  if (svc == "media_player.turn_off" && d["entity_id"] == tv::entity_ref()) tv_status("off", "");
}

static void sample_audio() {
  audio::Data &x = audio::d();
  x.speaker_e = "media_player.woonkamer"; x.muziek = x.speaker_e; x.sub_e = "number.woonkamer_sw_gain";
  x.speaker_state = "on"; x.src = 2; x.volume = 38; x.sub = 2; x.sub_known = true;
  x.sp_state = "playing"; x.title = "Midnight Drive"; x.artist = "Nova Lane";
}
// sample weather, calendar and values (in the firmware a module fills info:: from Home Assistant - phase 4)
static void sample_info() {
  const bool en = english();
  info::Weather w;
  w.known = true; w.place = "Utrecht"; w.cond = "partlycloudy"; w.temp = 12; w.lo = 9; w.hi = 14;
  w.hours = {{"15:00", 13}, {"16:00", 12}, {"17:00", 11}, {"18:00", 10}};
  w.days = en ? std::vector<info::Day>{{"Fri", 14, 8}, {"Sat", 11, 6}, {"Sun", 9, 4}, {"Mon", 12, 7}, {"Tue", 13, 8}}
              : std::vector<info::Day>{{"vr", 14, 8}, {"za", 11, 6}, {"zo", 9, 4}, {"ma", 12, 7}, {"di", 13, 8}};
  info::weather()["weather.thuis"] = w;
  info::calendar()["calendar.gezin"] = {
      {"Vandaag", "14:00", en ? "Dentist" : "Tandarts", en ? "City centre" : "Centrum"},
      {"Vandaag", "19:30", en ? "Football practice" : "Voetbal Jesse", en ? "Sports park" : "Sportpark"},
      {"Morgen", "09:00", en ? "Team meeting" : "Overleg werk", "Teams"},
      {"Morgen", "12:30", en ? "Lunch with Sanne" : "Lunch met Sanne", ""},
      {en ? "Sat 29 Nov" : "za 29 nov", "10:00", en ? "Market" : "Markt", ""}};
  auto val = [](const char *e, const char *name, const char *text, const char *ch, int trend, std::vector<float> h) {
    info::Value v; v.known = true; v.name = name; v.text = text; v.change = ch; v.trend = trend; v.history = h;
    info::values()[e] = v;
  };
  val("sensor.btc_eur", "BTC", "\xE2\x82\xAC" " 84.210", "+2,4%", 1, {80, 81, 79, 82, 83, 82, 85, 84, 86, 88, 87, 90});
  val("sensor.eth_eur", "ETH", "\xE2\x82\xAC" " 3.122", "+1,1%", 1, {3, 4, 3, 5, 4, 6, 5, 6});
  val("sensor.aex", "AEX", "912,3", "-0,6%", -1, {92, 91, 92, 90, 89, 90, 88, 89, 87, 88, 86, 85});
}

static void sample_airco() {
  airco::Data &d = airco::d();
  airco::Unit u;
  u.name_s = english() ? "Living room" : "Woonkamer";
  u.is_on = true; u.mode = 1; u.temp = 21; u.room = 23;
  u.fans = airco::mhi::fans();
  u.swings = airco::mhi::swings();
  u.tmin = 18; u.tmax = 30;
  d.units = {u};
  d.cur = 0;
  airco::on_change() = [](int i) {
    const airco::Unit &n = airco::d().units[i];
    std::printf("[IR] Mitsubishi Heavy: %s mode %d %d C fan %d swing %d\n", n.is_on ? "on" : "off", n.mode, n.temp, n.fan, n.swing);
  };
}

// Bigger screen: the same fonts at u x their size, rendered from the firmware's TTF files (the firmware gets them
// from its board file instead)
static std::vector<char> read_file(const char *name) {
  std::string p = std::string(NOIR_FONT_DIR) + "/" + name;
  std::vector<char> d;
  FILE *f = std::fopen(p.c_str(), "rb");
  if (f == nullptr) { std::printf("[sim] missing %s\n", p.c_str()); return d; }
  std::fseek(f, 0, SEEK_END); d.resize(std::ftell(f)); std::fseek(f, 0, SEEK_SET);
  if (std::fread(d.data(), 1, d.size(), f) != d.size()) d.clear();
  std::fclose(f);
  return d;
}
static Fonts scaled_fonts() {
  static std::vector<char> inter = read_file("Inter-Regular.ttf"), mdi = read_file("mdi-subset.ttf");
  auto ttf = [](std::vector<char> &d, int size) { return lv_tiny_ttf_create_data(d.data(), d.size(), sc(size)); };
  return Fonts{ttf(inter, 12), ttf(inter, 14), ttf(inter, 16), ttf(mdi, 20), ttf(inter, 20), ttf(inter, 48)};
}

// ---------------------------------------------------------------- the phone: configuration and states from Home Assistant
static JsonDocument g_cfg;
// the device's payload from NOIR Remote (as the remote fetches it) + {"info": {"i:weather.x": {...}}}
extern "C" void noir_web_config(const char *json) {
  if (deserializeJson(g_cfg, json)) return;
  g_home.clear(); g_home.set(g_cfg["home"]);
  g_scenes.clear(); g_scenes.set(g_cfg["scenes"]);
  english() = std::string(g_cfg["lang"] | "nl") != "nl";
  texts().clear();
  for (JsonPairConst kv : g_cfg["texts"].as<JsonObjectConst>()) texts()[kv.key().c_str()] = kv.value().as<std::string>();
  if (!lamps_from_items(g_cfg["lights"].as<JsonArrayConst>())) lamps().clear();
  tv::entity_ref() = g_cfg["tv"]["entity"] | "";
  tv_apps().clear();
  for (JsonObjectConst a : g_cfg["tv"]["apps"].as<JsonArrayConst>()) tv_apps().push_back({a["name"] | "", a["source"] | "", a["logo"] | ""});
  for (JsonPairConst kv : g_cfg["info"].as<JsonObjectConst>()) {   // same objects as the remote's module Info gets
    const std::string e = std::string(kv.key().c_str()).substr(2), d = e.substr(0, e.find('.'));
    JsonObjectConst o = kv.value();
    if (d == "weather") {
      info::Weather w; w.known = true; w.cond = o["c"] | ""; w.place = o["p"] | ""; w.temp = o["t"] | 0; w.lo = o["lo"] | 0; w.hi = o["hi"] | 0;
      for (JsonArrayConst h : o["h"].as<JsonArrayConst>()) w.hours.push_back({h[0] | "", h[1] | 0});
      for (JsonArrayConst x : o["d"].as<JsonArrayConst>()) w.days.push_back({x[0] | "", x[1] | 0, x[2] | 0});
      info::weather()[e] = w;
    } else if (d == "calendar") {
      std::vector<info::Event> ev;
      for (JsonArrayConst x : o["ev"].as<JsonArrayConst>()) ev.push_back({x[0] | "", x[1] | "", x[2] | "", x[3] | ""});
      info::calendar()[e] = ev;
    } else {
      info::Value v; v.known = true; v.name = o["n"] | ""; v.text = o["v"] | ""; v.change = o["ch"] | ""; v.trend = o["tr"] | 0;
      for (float f : o["hs"].as<JsonArrayConst>()) v.history.push_back(f);
      info::values()[e] = v;
    }
  }
  rebuild_all();
}
// The page resized or showed the canvas again (Home Assistant opening the panel, the sidebar, a tab coming back): the
// browser may have wiped the canvas, and LVGL only redraws what changes - so draw everything again.
extern "C" void noir_web_redraw() {
  lv_obj_invalidate(lv_screen_active());
  lv_obj_invalidate(lv_layer_top());
}
// {"light.x": ["on", 128], "media_player.tv": ["on", -1, "HDMI 1"]} - lamps and the TV
extern "C" void noir_web_states(const char *json) {
  JsonDocument d;
  if (deserializeJson(d, json)) return;
  for (Lamp &l : lamps()) {
    JsonArrayConst v = d[l.entity_ref];
    if (!v.isNull()) { l.state = v[0] | ""; l.bri = v[1] | -1; }
  }
  lamps_text();
  JsonArrayConst t = d[tv::entity_ref()];
  if (!t.isNull()) tv_status(t[0] | "", t[2] | "");
}

// ---------------------------------------------------------------- actions (as noirapp::action in the firmware)
static void run_tile(int i) {
  JsonArrayConst items = g_home.as<JsonArrayConst>();
  if (i < 0 || i >= (int) items.size()) return;
  const std::string fn = items[i]["fn"] | "";
  extern void action(int a);
  if (fn == "tv") action(TV);
  else if (fn == "audio") action(AUDIO);
  else if (fn == "lights") action(LIGHTS);
  else if (fn == "scenes") action(SCENES);
  else if (fn == "airco") action(AIRCO);
  else if (fn == "alloff") action(SCENE_ALL_OFF);
  else if (!fn.empty()) std::printf("[remote] tile %d: %s\n", i, fn.c_str());
  else {   // an entity tile (as module Home): scene/script run, light/switch toggle; info tiles only show
    const std::string e = items[i]["entity"] | "", t = items[i]["t"] | "";
    if (e.empty() || t == "weather" || t == "calendar" || t == "value") return;
    if (e.rfind("scene.", 0) == 0) ha_print("scene.turn_on", {{"entity_id", e}});
    else if (e.rfind("script.", 0) == 0) ha_print("script.turn_on", {{"entity_id", e}});
    else if (e.rfind("light.", 0) == 0) ha_print("light.toggle", {{"entity_id", e}});
    else ha_print("homeassistant.toggle", {{"entity_id", e}});
  }
}

void action(int a) {
  if (a >= SCENE && a < SCENE + 40) {
    JsonArrayConst it = g_scenes.as<JsonArrayConst>();
    if (a - SCENE < (int) it.size()) {
      const std::string e = it[a - SCENE]["entity"] | "";
      if (!e.empty()) ha_print(e.rfind("script.", 0) == 0 ? "script.turn_on" : "scene.turn_on", {{"entity_id", e}});
      toast(it[a - SCENE]["label"] | "Scene");
    }
    return;
  }
  if (a >= ITEM) { run_tile(a - ITEM); return; }
  if (a >= AUDIO_SOURCE && a < AUDIO_SOURCE + audio::MAX_SOURCES) { audio::source_select(a - AUDIO_SOURCE); return; }
  if (a >= AIRCO_MODE && a < AIRCO_MODE + airco::MODES) { airco::mode(a - AIRCO_MODE); return; }
  if (a >= LAMP && a < LAMP + 12) { lamp_toggle(a - LAMP, lv_tick_get()); return; }
  if (a >= TV_APP && a < TV_APP + (int) tv_apps().size()) {
    const TvApp &app = tv_apps()[a - TV_APP];
    if (!tv_aan()) tv_status("on", "");
    ha_print("media_player.select_source", {{"entity_id", tv::entity_ref()}, {"source", app.source}});
    tv_status("on", app.source);
    toast(app.name.c_str());
    return;
  }
  switch (a) {
    case TV: show_screen(S_TV); break;
    case TV_POWER:
      if (tv_aan()) { ha_print("media_player.turn_off", {{"entity_id", tv::entity_ref()}}); tv_status("off", ""); toast("TV uit"); }
      else { tv_status("on", "Live TV"); toast("TV aan"); }
      break;
    case AUDIO: if (g_web) { toast("Audio: op de remote"); break; } show_screen(S_AUDIO); break;
    case AUDIO_POWER: audio::toggle_power(); break;
    case AUDIO_NOW: ha_print("media_player.media_play_pause", {{"entity_id", audio::d().muziek}}); break;
    case LIGHTS: show_screen(S_LIGHTS); break;
    case SCENES: show_screen(S_SCENES); break;
    case SCENE_ALL_OFF:
      for (auto &l : lamps()) l.state = "off";
      lamps_text();
      ha_print("light.turn_off", {{"entity_id", "all lamps"}});
      toast("Alles uit");
      break;
    case AIRCO: if (g_web) { toast("Airco: op de remote"); break; } show_screen(S_AIRCO); break;
    case AIRCO_POWER: airco::toggle_power(); break;
    case AIRCO_MIN: airco::temp_stap(-1); break;
    case AIRCO_PLUS: airco::temp_stap(1); break;
    case AIRCO_FAN: airco::fan_next(); break;
    case AIRCO_SWING: airco::swing_next(); break;
    case AIRCO_UNIT: airco::unit_next(); break;
    case KEYTEST: keytest::reset(); show_screen(S_TEST); break;
    case TEST_RESET: keytest::reset(); break;
    case SETTINGS: show_screen(S_SETTINGS); break;
    case SET_FULL: case SET_ECO: eco_mode() = a == SET_ECO; rebuild_all(); break;
    case SET_WIFI: toast("Reconnecting Wi-Fi\xE2\x80\xA6"); break;
    case DIM_DONE: dimmer_close(); break;
    case BACK:
      if (choice().over != nullptr) choice_close();
      else if (st().cur_screen != S_HOME) show_screen(S_HOME);
      break;
    case HOME: show_screen(S_HOME); break;
    default: std::printf("[remote] action %d\n", a); break;
  }
}

// ---------------------------------------------------------------- hardware keys (Rev6 key codes)
static void key(int k) {
  for (bool down : {true, false}) {
    if (speaker_e::key_in(k, down)) continue;   // volume / mute -> speaker
    if (!down) continue;
    if (st().cur_screen == S_TEST) { keytest::key_in(k, true); keytest::key_in(k, false); continue; }
    if (k == 13) action(BACK);
    else if (k == 41 || k == 3) action(HOME);
    else if (st().cur_screen == S_AIRCO && (k == 14 || k == 35)) airco::temp_stap(k == 14 ? 1 : -1);
    else if (k == 2) action(AUDIO_NOW);
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
  if (g_hh >= 0) status(g_hh, g_mm, 78, false, true, -60);
  else status(l->tm_hour, l->tm_min, 78, false, true, -60);
  repeat_tick(lv_tick_get());
}

// --demo: every screen once, a screenshot of each
static void demo_cb(lv_timer_t *tm) {
  static int step = 0;
  struct S { int action; const char *file; };
  static const S steps[] = {{HOME, "demo-01-home.bmp"}, {TV, "demo-02-tv.bmp"}, {LIGHTS, "demo-03-lights.bmp"},
                            {SCENES, "demo-04-scenes.bmp"}, {AUDIO, "demo-05-audio.bmp"}, {-1, "demo-05b-audio-bottom.bmp"},
                            {AIRCO, "demo-06-airco.bmp"}, {SETTINGS, "demo-07-settings.bmp"}, {KEYTEST, "demo-08-buttontest.bmp"}};
  const int n = sizeof(steps) / sizeof(steps[0]);
  if (step > 0) screenshot(steps[step - 1].file);
  if (step >= n) { lv_timer_delete(tm); std::exit(0); }
  if (steps[step].action == -1) lv_obj_scroll_to_y(st().audio_body, 200, LV_ANIM_OFF);
  else action(steps[step].action);
  if (steps[step].action == LIGHTS) action(LAMP + 0);
  if (steps[step].action == KEYTEST) { key(14); key(41); }
  step++;
}

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--demo")) g_demo = true;
    if (!strcmp(argv[i], "--en")) english() = true;   // English screens
    if (!strcmp(argv[i], "--grid")) g_grid = true;
    if (!strcmp(argv[i], "--size") && i + 1 < argc) std::sscanf(argv[++i], "%dx%d", &g_w, &g_h);
    if (!strcmp(argv[i], "--clock") && i + 1 < argc) std::sscanf(argv[++i], "%d:%d", &g_hh, &g_mm);
  }
  layout_init(g_w, g_h);
  lv_init();
  lv_display_t *disp = lv_sdl_window_create(g_w, g_h);
  lv_sdl_window_set_zoom(disp, g_w <= 320 ? 2 : 1);
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
  tv::entity_ref() = "media_player.lg_tv";
  speaker_e::entity_ref() = "media_player.woonkamer";
  speaker_e::notice() = [](const char *t) { toast(t); };
  st().actie = action;
  if (g_web) { deserializeJson(g_home, "[]"); deserializeJson(g_scenes, "[]"); }
  else {
    deserializeJson(g_home, g_grid ? (english() ? GRID_JSON_EN : GRID_JSON) : (english() ? HOME_JSON_EN : HOME_JSON));
    deserializeJson(g_scenes, english() ? SCENES_JSON_EN : SCENES_JSON);
  }
  st().items = []() { return g_home.as<JsonArrayConst>(); };
  st().scenes_items = []() { return g_scenes.as<JsonArrayConst>(); };
  if (!g_web) {   // the browser gets the real configuration from Home Assistant (noir_web_config)
    lamps_from_text(english() ? LAMPS_EN : LAMPS);
    for (auto &l : lamps()) { l.state = "off"; l.bri = -1; }
    sample_audio();
    sample_airco();
    sample_info();
  }

  Fonts f{&inter12, &inter14, &inter16, &mdi20, &inter20, &inter48};
  if (lay().u != 1.0f) f = scaled_fonts();
  ImageSet bl;
  bl.wall = &wall;
  bl.tv = &ic_tv; bl.audio = &ic_audio; bl.light = &ic_light; bl.scenes = &ic_scenes;
  sim_logos();
  if (!g_web) tv_apps() = {{"PS5", "PS5 Gameconsole", "playstation"}, {"Netflix", "Netflix", "netflix"}, {"YouTube", "YouTube", "youtube"},
               {"Xbox", "HDMI 3", "game"}, {"Live TV", "Live TV", "tv"}, {"Spotify", "Spotify", "spotify"}};
  st().toast_font = &inter14;
  build_all(scr, f, bl, g_home.as<JsonArrayConst>());
  tv_status("on", "PS5 Gameconsole");
  audio::show();
  airco::show();
  show_page(true);
  show_screen(S_HOME);

  lv_timer_create(clock_cb, 1000, nullptr);
  clock_cb(nullptr);
  if (g_demo) lv_timer_create(demo_cb, 400, nullptr);
  std::printf("NOIR simulator: mouse = finger, arrows/Enter/Backspace = D-pad/OK/Back, H = Home,\n"
              "space = play/pause, +/-/M = volume, P = screenshot, Q = quit.\n");
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop([]() { lv_timer_handler(); }, 0, false);   // the browser calls it every frame
#else
  for (;;) {
    const uint32_t wait = lv_timer_handler();
    lv_delay_ms(wait < 5 ? wait : 5);
  }
#endif
}
