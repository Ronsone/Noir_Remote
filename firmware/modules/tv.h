#pragma once
#include "core.h"
// Module TV - knows no brands. Everything comes from the NOIR Builder:
//   tv.entity   media_player of the TV (state, choosing a source)
//   tv.speaker  media_player for on/off together with the TV and the volume keys (optional)
//   tv.ir       {"p":"nec","a":4,"on":196,"off":197,"power":8,"keys":{"14":64,...}}  (keycode -> command)
//               empty = no infrared, everything via Home Assistant
//   tv.apps     [{"name":"PS5","source":"PS5 Gameconsole","logo":"playstation"}, ...]  (up to 8)
namespace noirapp {

struct Tv : Module {
  Ir ir;
  int on{-1}, off{-1}, power{-1};
  std::map<int, int> keys;          // Rev6 keycode -> IR command
  std::string speaker, apps_sig;
  std::string speaker_source;   // some speakers (KEF LSX II) only wake up by choosing a source
  int held{0};
  uint32_t held_next{0};

  const char *name() const override { return "tv"; }

  static bool arrow(int k) { return k == 14 || k == 35 || k == 15 || k == 32; }
  static bool repeats(int k) { return arrow(k) || k == 33 || k == 43; }   // hold = repeat (arrows, volume)
  static bool nav(int k) { return arrow(k) || k == 34 || k == 31 || k == 13 || k == 3; }   // remote navigation elsewhere
  static bool on_screen() { return noir::st().seite == noir::S_TV; }
  static const std::string &entity() { return noir::tv::entiteit(); }
  int code(int k) const { auto it = keys.find(k); return it == keys.end() ? -1 : it->second; }

  void power_on() {
    if (on >= 0) ir.send(on);
    else if (power >= 0 && !noir::tv_aan()) ir.send(power);
    else if (!entity().empty()) ha("media_player.turn_on", {{"entity_id", entity()}});
    if (!speaker.empty()) {
      if (!speaker_source.empty()) ha("media_player.select_source", {{"entity_id", speaker}, {"source", speaker_source}});
      else ha("media_player.turn_on", {{"entity_id", speaker}});
    }
  }
  void power_off() {
    if (off >= 0) ir.send(off);
    else if (power >= 0 && noir::tv_aan()) ir.send(power);
    else if (!entity().empty()) ha("media_player.turn_off", {{"entity_id", entity()}});
    if (!speaker.empty()) ha("media_player.turn_off", {{"entity_id", speaker}});
  }
  static void select(const std::string &source) {
    if (!entity().empty() && !source.empty()) ha("media_player.select_source", {{"entity_id", entity()}, {"source", source}});
  }
  void app_tile(int i) {
    const auto &apps = noir::tv_apps();
    if (i < 0 || i >= (int) apps.size()) return;
    static std::string pending;
    pending = apps[i].source;
    noir::toast(apps[i].name.c_str());
    if (noir::tv_aan()) { select(pending); return; }
    power_on();   // TV off: switch on, choose the source once the TV is up (~9 s)
    lv_timer_t *t = lv_timer_create([](lv_timer_t *) { select(pending); }, 9000, nullptr);
    lv_timer_set_repeat_count(t, 1);
  }

  void configure(JsonObjectConst cfg) override {
    JsonObjectConst tv = cfg["tv"];
    noir::tv::entiteit() = tv["entity"] | "";
    speaker = tv["speaker"] | (tv["kef"] | "");          // "kef" = name before 0.2
    speaker_source = tv["speaker_source"] | "";
    noir::kef::entiteit() = speaker;                       // volume keys -> the speaker
    JsonObjectConst r = tv["ir"];
    ir = Ir{r["p"] | "", r["a"] | -1};
    on = r["on"] | -1; off = r["off"] | -1; power = r["power"] | -1;
    keys.clear();
    for (JsonPairConst kv : r["keys"].as<JsonObjectConst>()) keys[atoi(kv.key().c_str())] = kv.value() | -1;
    // Sources: rebuild the screens only when they changed
    std::vector<noir::TvApp> apps;
    std::string sig;
    for (JsonObjectConst a : tv["apps"].as<JsonArrayConst>()) {
      if (apps.size() >= 8) break;
      apps.push_back({a["name"] | "", a["source"] | "", a["logo"] | ""});
      sig += apps.back().name + "|" + apps.back().source + "|" + apps.back().logo + ";";
    }
    if (sig != apps_sig) {
      apps_sig = sig;
      noir::tv_apps() = apps;
      if (app().built) noir::alles_herbouwen();
    }
    // TV state in the same poll as the lamps (while the display is on)
    noir::licht_http::extra_open() = []() { return !entity().empty() && screen_on(); };
    noir::licht_http::extra_e() = []() { return entity().empty() ? std::string() : entity() + ","; };
    noir::licht_http::extra_lees() = [](JsonDocument &doc) {
      JsonArrayConst v = doc[entity()];
      if (!v.isNull()) noir::tv_status(v[0] | "", v[10] | "");
    };
  }
  bool action(int a) override {
    if (a >= noir::TV_APP && a < noir::TV_APP + 8) { app_tile(a - noir::TV_APP); return true; }
    if (a == noir::TV) { noir::zeigen_seite(noir::S_TV); if (!noir::tv_aan()) power_on(); return true; }
    if (a == noir::TV_AANUIT) {
      if (noir::tv_aan()) { power_off(); noir::toast("TV uit"); } else { power_on(); noir::toast("TV aan"); }
      return true;
    }
    return false;
  }
  bool key(int k, bool pressed) override {
    if (noir::kef::taste(k, pressed)) return true;   // volume / mute -> speaker (with repeat)
    if (!pressed) { held = 0; return false; }
    if (k == 102) { action(noir::TV_AANUIT); return true; }   // power key
    const int c = code(k);
    if (c < 0) return false;
    if (!on_screen() && nav(k)) return false;   // outside the TV screen these navigate the remote
    ir.send(c);                                  // the rest (media, channel, colour, volume) reaches the TV everywhere
    if (repeats(k)) { held = k; held_next = millis() + 450; }
    return true;
  }
  void tick(uint32_t now) override {
    noir::kef::herhaal_tik(now);
    if (held != 0 && (on_screen() || !nav(held)) && (int32_t) (now - held_next) >= 0) {   // hold = repeat
      ir.send(code(held));
      held_next = now + 160;
    }
  }
  void all_off() override { power_off(); }
};

}  // namespace noirapp
