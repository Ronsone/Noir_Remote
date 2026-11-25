#pragma once
#include "core.h"
// Module Home: the tiles on the home screen, plus navigation and the remote's own settings screen.
namespace noirapp {

struct Home : Module {
  const char *name() const override { return "home"; }
  // fn of a tile -> screen action
  static int fn_action(const std::string &fn) {
    if (fn == "tv") return noir::TV;
    if (fn == "lights") return noir::LICHT;
    if (fn == "scenes") return noir::SCENES;
    if (fn == "settings") return noir::INSTELLINGEN;
    if (fn == "alloff") return noir::SCENE_ALLES_UIT;
    if (fn == "tv_power") return noir::TV_AANUIT;
    if (fn == "audio") return noir::AUDIO;
    if (fn == "airco") return noir::AIRCO;
    return -1;
  }
  static void run_item(JsonObjectConst it) {
    const std::string fn = it["fn"] | "";
    const std::string e = it["entity"] | "";
    if (!fn.empty()) {
      const int a = fn_action(fn);
      if (a >= 0) noirapp::action(a); else noir::toast("Coming in a later version");
      return;
    }
    if (e.empty()) return;
    if (e.rfind("scene.", 0) == 0) ha("scene.turn_on", {{"entity_id", e}});
    else if (e.rfind("script.", 0) == 0) ha("script.turn_on", {{"entity_id", e}});
    else if (e.rfind("light.", 0) == 0) ha("light.toggle", {{"entity_id", e}});
    else ha("homeassistant.toggle", {{"entity_id", e}});
  }
  void configure(JsonObjectConst) override {
    if (app().built) noir::thuis_herbouwen(list_of("home"));
  }
  bool action(int a) override {
    if (a >= noir::ITEM) {
      JsonArrayConst h = list_of("home");
      const int i = a - noir::ITEM;
      if (i < (int) h.size()) run_item(h[i]);
      return true;
    }
    switch (a) {
      case noir::THUIS: noir::zeigen_seite(noir::S_THUIS); return true;
      case noir::TERUG: if (noir::st().seite != noir::S_THUIS) noir::zeigen_seite(noir::S_THUIS); return true;
      case noir::INSTELLINGEN: noir::zeigen_seite(noir::S_INST); return true;
      case noir::INST_FULL: case noir::INST_ECO: {   // mode chosen on the remote: apply now, tell the NOIR Builder
        const bool eco = a == noir::INST_ECO;
        set_mode(eco);
        ha("noir_remote.set_mode", {{"mode", eco ? "eco" : "full"}});
        noir::alles_herbouwen();
        noir::toast(eco ? "Eco" : "Full");
        return true;
      }
      case noir::INST_WIFI:
        noir::toast("Reconnecting Wi-Fi\xE2\x80\xA6", 2500);
        esphome::wifi::global_wifi_component->retry_connect();
        return true;
      case noir::KNOPTEST: noir::knoptest::reset(); noir::zeigen_seite(noir::S_TEST); return true;
      case noir::TEST_RESET: noir::knoptest::reset(); return true;
      case noir::NETFLIX: case noir::INST_REMOTE:
        noir::toast("Coming in a later version");
        return true;
      default: return false;
    }
  }
  bool key(int k, bool pressed) override {
    if (!pressed) return false;
    switch (k) {
      case 13: action(noir::TERUG); return true;        // back
      case 41: action(noir::THUIS); return true;        // return key: home
      case 3: action(noir::INSTELLINGEN); return true;  // menu: settings
      default: return false;
    }
  }
};

}  // namespace noirapp
