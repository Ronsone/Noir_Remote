#pragma once
#include "core.h"
// Module Lights: the Lights screen (tap = on/off, hold = dim). States come from HA every few seconds.
namespace noirapp {

struct Lights : Module {
  const char *name() const override { return "lights"; }
  void configure(JsonObjectConst) override {
    noir::lamps_from_items(list_of("lights"));
    if (app().built) noir::lights_rebuild();
  }
  bool action(int a) override {
    if (a >= noir::LAMP && a < noir::LAMP + 12) { noir::lamp_toggle(a - noir::LAMP, millis()); return true; }
    if (a == noir::LIGHTS) { noir::show_screen(noir::S_LIGHTS); return true; }
    if (a == noir::DIM_DONE) { noir::dimmer_close(); return true; }
    return false;
  }
  void tick(uint32_t now) override { noir::lights_http::poll_tick(now, noir::at_lights() && screen_on()); }
  void all_off() override {
    std::string all;
    for (const auto &l : noir::lamps()) { if (l.entity_ref.empty()) continue; if (!all.empty()) all += ","; all += l.entity_ref; }
    if (!all.empty()) ha("homeassistant.turn_off", {{"entity_id", all}});
    for (auto &l : noir::lamps()) if (!l.state.empty()) l.state = "off";
    noir::lamps_text();
  }
};

}  // namespace noirapp
