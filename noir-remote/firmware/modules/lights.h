#pragma once
#include "core.h"
// Module Lights: the Lights screen (tap = on/off, hold = dim). States come from HA every few seconds.
namespace noirapp {

struct Lights : Module {
  const char *name() const override { return "lights"; }
  void configure(JsonObjectConst) override {
    noir::lampen_uit_items(list_of("lights"));
    if (app().built) noir::licht_herbouwen();
  }
  bool action(int a) override {
    if (a >= noir::LAMP && a < noir::LAMP + 12) { noir::lamp_schakelen(a - noir::LAMP, millis()); return true; }
    if (a == noir::LICHT) { noir::zeigen_seite(noir::S_LICHT); return true; }
    if (a == noir::DIM_GEREED) { noir::dimmer_dicht(); return true; }
    return false;
  }
  void tick(uint32_t now) override { noir::licht_http::tik(now, noir::op_licht() && screen_on()); }
  void all_off() override {
    std::string all;
    for (const auto &l : noir::lampen()) { if (l.entiteit.empty()) continue; if (!all.empty()) all += ","; all += l.entiteit; }
    if (!all.empty()) ha("homeassistant.turn_off", {{"entity_id", all}});
    for (auto &l : noir::lampen()) if (!l.state.empty()) l.state = "off";
    noir::lampen_tekst();
  }
};

}  // namespace noirapp
