#pragma once
#include "core.h"
// Module Home: the tiles on the home screen, plus navigation and the remote's own settings screen.
namespace noirapp {

struct Home : Module {
  const char *name() const override { return "home"; }
  // fn of a tile -> screen action
  static int fn_action(const std::string &fn) {
    if (fn == "tv") return noir::TV;
    if (fn == "lights") return noir::LIGHTS;
    if (fn == "scenes") return noir::SCENES;
    if (fn == "settings") return noir::SETTINGS;
    if (fn == "alloff") return noir::SCENE_ALL_OFF;
    if (fn == "tv_power") return noir::TV_POWER;
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
    if (app().built) { noir::home_rebuild(list_of("home")); noir::airco::show(); }   // airco tile: fill it right away
  }
  bool action(int a) override {
    if (a >= noir::ITEM) {
      JsonArrayConst h = list_of("home");
      const int i = a - noir::ITEM;
      if (i < (int) h.size()) run_item(h[i]);
      return true;
    }
    switch (a) {
      case noir::HOME: noir::show_screen(noir::S_HOME); return true;
      case noir::BACK:   // Back closes an open pop-up first (skin list, lamp dimmer)
        if (noir::choice().over != nullptr) { noir::choice_close(); return true; }
        if (noir::dimmer().over != nullptr) { noir::dimmer_close(); return true; }
        if (noir::st().cur_screen != noir::S_HOME) noir::show_screen(noir::S_HOME);
        return true;
      case noir::SETTINGS: noir::show_screen(noir::S_SETTINGS); return true;
      case noir::SET_FULL: case noir::SET_ECO: {   // mode chosen on the remote: apply now, tell the NOIR Builder
        const bool eco = a == noir::SET_ECO;
        set_mode(eco);
        ha("noir_remote.set_mode", {{"mode", eco ? "eco" : "full"}});
        noir::rebuild_all();
        noir::toast(eco ? "Eco" : "Full");
        return true;
      }
      case noir::SET_WIFI:
        noir::toast("Reconnecting Wi-Fi\xE2\x80\xA6", 2500);
        esphome::wifi::global_wifi_component->retry_connect();
        return true;
      case noir::KEYTEST: noir::keytest::reset(); noir::show_screen(noir::S_TEST); return true;
      case noir::TEST_RESET: noir::keytest::reset(); return true;
      default: return false;
    }
  }
  bool key(int k, bool pressed) override {
    if (!pressed) return false;
    switch (k) {
      case 13: action(noir::BACK); return true;        // back
      case 41: action(noir::HOME); return true;        // return key: home
      case 3: action(noir::SETTINGS); return true;  // menu: settings
      default: return false;
    }
  }
};

}  // namespace noirapp
