#pragma once
#include "core.h"
// Module Mouse: the touchpad screen for the TV's cursor (noir-ui.h mouse_build; NOIR Remote passes it to the TV).
// Opens from a Home tile (fn "mouse") or the mouse button on the TV screen - only when the TV can show a cursor
// (config tv.mouse, LG webOS for now). On the screen the D-pad moves in small steps and OK clicks.
namespace noirapp {

struct Mouse : Module {
  const char *name() const override { return "mouse"; }
  static bool on_screen() { return noir::st().cur_screen == noir::S_MOUSE; }

  void configure(JsonObjectConst cfg) override {
    const bool m = cfg["tv"]["mouse"] | false;
    if (m != noir::tv_mouse()) { noir::tv_mouse() = m; if (app().built) noir::rebuild_all(); }   // the TV screen's button
  }
  bool action(int a) override {
    if (a != noir::MOUSE) return false;
    if (!noir::tv_mouse()) { noir::toast("De muis werkt met een LG webOS-tv"); return true; }
    noir::show_screen(noir::S_MOUSE);
    return true;
  }
  bool key(int k, bool pressed) override {
    if (!on_screen() || !pressed) return on_screen() && (k == 14 || k == 35 || k == 15 || k == 32 || k == 34);
    switch (k) {
      case 14: noir::mouse::nudge(0, -24); return true;
      case 35: noir::mouse::nudge(0, 24); return true;
      case 15: noir::mouse::nudge(-24, 0); return true;
      case 32: noir::mouse::nudge(24, 0); return true;
      case 34: noir::mouse::click(); return true;
      default: return false;
    }
  }
};

}  // namespace noirapp
