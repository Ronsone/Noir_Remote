#pragma once
#include "core.h"
// Module Airco: up to 4 units (rooms), each one either
//   - a climate entity in Home Assistant (any brand) - modes, fan and swing come from that entity, state from /states;
//   - the remote's own infrared with a brand profile ("mhi" = Mitsubishi Heavy) - the remote remembers what it sent.
// Configuration (NOIR Builder > Airco):  airco.units = [{"name": "Woonkamer", "entity": "climate.x"} | {"name", "ir": "mhi"}]
// Keys on the Airco screen: up/down = temperature, OK = on/off.
namespace noirapp {

struct Airco : Module {
  struct Link {                        // per unit: how it is driven + what was sent last (HA)
    std::string entity, ir;
    std::string hvac[noir::airco::MODI];   // HA hvac_mode name per mode index ("" = not offered)
    std::string sent_fan, sent_swing;
    uint32_t changed{0};               // last change on the remote (ms) - poll waits a moment before overwriting
  };
  std::vector<Link> links;
  std::string sig;

  const char *name() const override { return "airco"; }
  static bool on_screen() { return noir::st().seite == noir::S_AIRCO; }

  static void nvs_put(int i, int v) {
    nvs_handle_t h;
    if (nvs_open("noir", NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_i32(h, ("ac" + std::to_string(i)).c_str(), v);
    nvs_commit(h);
    nvs_close(h);
  }
  static int nvs_get(int i) {
    nvs_handle_t h;
    int32_t v = 0;
    if (nvs_open("noir", NVS_READONLY, &h) != ESP_OK) return 0;
    nvs_get_i32(h, ("ac" + std::to_string(i)).c_str(), &v);
    nvs_close(h);
    return v;
  }

  void configure(JsonObjectConst cfg) override {
    namespace ac = noir::airco;
    std::string s;
    for (JsonObjectConst o : cfg["airco"]["units"].as<JsonArrayConst>())
      s += std::string(o["name"] | "") + "|" + std::string(o["entity"] | "") + "|" + std::string(o["ir"] | "") + ";";
    if (s == sig) return;
    sig = s;
    ac::Data &d = ac::d();
    d.units.clear();
    links.clear();
    for (JsonObjectConst o : cfg["airco"]["units"].as<JsonArrayConst>()) {
      if ((int) d.units.size() >= ac::MAX_UNITS) break;
      ac::Unit n;
      Link l;
      n.naam = o["name"] | "";
      l.entity = o["entity"] | "";
      l.ir = o["ir"] | "";
      if (l.entity.empty() && l.ir.empty()) continue;
      if (!l.ir.empty()) {                       // infrared profile: everything the protocol has
        n.fans = ac::mhi::fans();
        n.swings = ac::mhi::swings();
        n.tmin = 18; n.tmax = 30;
        ac::mhi::uitpakken(n, nvs_get((int) d.units.size()));
      } else {
        n.bekend = false;                        // until the first poll
        n.fans = {}; n.swings = {};
      }
      d.units.push_back(n);
      links.push_back(l);
    }
    if (d.units.empty()) d.units.push_back(ac::Unit{});
    d.cur = 0;
    ac::wijziging() = [this](int i) { send(i); };
    poll_set("airco", {[this]() { return on_screen() && screen_on() && !entities().empty(); },
                       [this]() { return entities(); },
                       [this](JsonDocument &doc) { read(doc); }});
    ac::tonen();
  }

  std::string entities() const {
    std::string e;
    for (const Link &l : links) if (!l.entity.empty()) e += l.entity + ",";
    return e;
  }

  static std::vector<std::string> split(const std::string &s) {   // "a|b|c"
    std::vector<std::string> v;
    size_t p = 0;
    while (p <= s.size() && !s.empty()) {
      size_t q = s.find('|', p);
      if (q == std::string::npos) q = s.size();
      if (q > p) v.push_back(s.substr(p, q - p));
      p = q + 1;
    }
    return v;
  }

  // /states row of a climate: [state, -1, -1, unit, target, current, fan_mode, swing_mode, min, max, hvac_modes, fan_modes, swing_modes]
  void read(JsonDocument &doc) {
    namespace ac = noir::airco;
    static const char *const NAMES[ac::MODI][2] = {{"auto", "heat_cool"}, {"cool", ""}, {"dry", ""}, {"fan_only", ""}, {"heat", ""}};
    for (size_t i = 0; i < links.size() && i < ac::d().units.size(); i++) {
      Link &l = links[i];
      if (l.entity.empty()) continue;
      JsonArrayConst v = doc[l.entity];
      if (v.isNull()) continue;
      if (millis() - l.changed < 4000) continue;   // you are just changing it: don't jump back
      ac::Unit &n = ac::d().units[i];
      const std::string state = v[0] | "";
      n.bekend = !state.empty() && state != "unavailable" && state != "unknown";
      n.aan = n.bekend && state != "off";
      const std::vector<std::string> modes = split(v[10] | "");
      for (int m = 0; m < ac::MODI; m++) {
        l.hvac[m] = "";
        for (const char *nm : NAMES[m]) if (*nm && std::find(modes.begin(), modes.end(), nm) != modes.end()) { l.hvac[m] = nm; break; }
        n.modi[m] = !l.hvac[m].empty() || modes.empty();
        if (state == NAMES[m][0] || (*NAMES[m][1] && state == NAMES[m][1])) n.modus = m;
      }
      const float target = v[4] | -1000.0f, current = v[5] | -1000.0f;
      if (target > -999) n.temp = (int) lroundf(target);
      n.kamer = current > -999 ? (int) lroundf(current) : -1000;
      const float lo = v[8] | 16.0f, hi = v[9] | 30.0f;
      n.tmin = (int) ceilf(lo); n.tmax = (int) floorf(hi);
      n.fans = split(v[11] | "");
      n.swings = split(v[12] | "");
      const std::string fm = v[6] | "", sm = v[7] | "";
      for (size_t k = 0; k < n.fans.size(); k++) if (n.fans[k] == fm) n.fan = k;
      for (size_t k = 0; k < n.swings.size(); k++) if (n.swings[k] == sm) n.swing = k;
      l.sent_fan = fm; l.sent_swing = sm;
    }
    ac::tonen();
  }

  void send(int i) {
    namespace ac = noir::airco;
    if (i < 0 || i >= (int) links.size() || i >= (int) ac::d().units.size()) return;
    Link &l = links[i];
    const ac::Unit &n = ac::d().units[i];
    l.changed = millis();
    if (!l.ir.empty()) {                         // infrared: one frame with the whole state
      if (l.ir == "mhi") ir_raw(ac::mhi::timings(n));
      nvs_put(i, ac::mhi::inpakken(n));
      return;
    }
    if (l.entity.empty()) return;
    const std::map<std::string, std::string> e{{"entity_id", l.entity}};
    if (!n.aan) { ha("climate.turn_off", e); return; }
    if (!l.hvac[n.modus].empty()) ha("climate.set_hvac_mode", {{"entity_id", l.entity}, {"hvac_mode", l.hvac[n.modus]}});
    ha("climate.set_temperature", {{"entity_id", l.entity}, {"temperature", std::to_string(n.temp)}});
    if (n.fan < (int) n.fans.size() && n.fans[n.fan] != l.sent_fan) {
      l.sent_fan = n.fans[n.fan];
      ha("climate.set_fan_mode", {{"entity_id", l.entity}, {"fan_mode", l.sent_fan}});
    }
    if (n.swing < (int) n.swings.size() && n.swings[n.swing] != l.sent_swing) {
      l.sent_swing = n.swings[n.swing];
      ha("climate.set_swing_mode", {{"entity_id", l.entity}, {"swing_mode", l.sent_swing}});
    }
  }

  bool action(int a) override {
    namespace ac = noir::airco;
    if (a == noir::AIRCO) {
      if (links.empty()) { noir::toast("Choose your airco in NOIR Builder > Airco"); return true; }
      noir::zeigen_seite(noir::S_AIRCO);
      return true;
    }
    if (a >= noir::AIRCO_MODUS && a < noir::AIRCO_MODUS + ac::MODI) { ac::modus(a - noir::AIRCO_MODUS); return true; }
    switch (a) {
      case noir::AIRCO_AANUIT: ac::aanuit(); return true;
      case noir::AIRCO_MIN: ac::temp_stap(-1); return true;
      case noir::AIRCO_PLUS: ac::temp_stap(1); return true;
      case noir::AIRCO_FAN: ac::fan_volgende(); return true;
      case noir::AIRCO_SWING: ac::swing_volgende(); return true;
      case noir::AIRCO_UNIT: ac::unit_volgende(); return true;
      default: return false;
    }
  }

  // up/down = temperature, left/right = mode (only the ones the unit has), OK = on/off
  bool key(int k, bool pressed) override {
    namespace ac = noir::airco;
    if (!on_screen() || (k != 14 && k != 35 && k != 15 && k != 32 && k != 34)) return false;
    if (!pressed) return true;
    if (k == 14) ac::temp_stap(1);
    else if (k == 35) ac::temp_stap(-1);
    else if (k == 34) ac::aanuit();
    else {
      const ac::Unit &n = ac::u();
      for (int step = 1; step < ac::MODI; step++) {
        const int m = (n.modus + (k == 32 ? step : ac::MODI - step)) % ac::MODI;
        if (n.modi[m]) { ac::modus(m); break; }
      }
    }
    return true;
  }

  void all_off() override {
    namespace ac = noir::airco;
    for (size_t i = 0; i < links.size() && i < ac::d().units.size(); i++) {
      ac::d().units[i].aan = false;
      send((int) i);
    }
    ac::tonen();
  }
};

}  // namespace noirapp
