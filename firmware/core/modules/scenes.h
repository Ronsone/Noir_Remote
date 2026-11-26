#pragma once
#include "core.h"
#include "home.h"   // Home::run_item
// Module Scenes: tiles that run a scene or script, plus the built-in "All off".
namespace noirapp {

struct Scenes : Module {
  const char *name() const override { return "scenes"; }
  void configure(JsonObjectConst) override {
    if (app().built) noir::scenes_rebuild();
  }
  bool action(int a) override {
    if (a >= noir::SCENE && a < noir::SCENE + 40) {
      JsonArrayConst s = list_of("scenes");
      const int i = a - noir::SCENE;
      if (i < (int) s.size()) {
        Home::run_item(s[i]);
        const char *n = s[i]["label"] | "";
        if (*n) noir::toast(n);
      }
      return true;
    }
    if (a == noir::SCENES) { noir::show_screen(noir::S_SCENES); return true; }
    if (a == noir::SCENE_ALL_OFF) { noirapp::all_off(); return true; }
    return false;
  }
};

}  // namespace noirapp
