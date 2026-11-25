#pragma once
#include "core.h"
// Module Nav: move around the screen with the D-pad. A light focus ring sits on a button; the arrows move it to the
// nearest button in that direction, OK presses it (hold OK = long press, e.g. dim a lamp).
// Rule: the screen you see decides - on the TV screen the D-pad goes to the TV (module Tv), on the Airco screen
// up/down set the temperature (module Airco); everywhere else it moves around the remote. Modules before this one
// get the keys first, so this one only sees the arrows and OK that nobody else wanted.
namespace noirapp {

struct Nav : Module {
  lv_obj_t *focus{nullptr};
  int page{-1};
  uint32_t last{0}, ok_down{0};
  const char *name() const override { return "nav"; }

  static bool is_dir(int k) { return k == 14 || k == 35 || k == 15 || k == 32; }

  static bool shown(lv_obj_t *o) {   // not hidden, and none of its parents
    for (lv_obj_t *p = o; p != nullptr; p = lv_obj_get_parent(p)) if (lv_obj_has_flag(p, LV_OBJ_FLAG_HIDDEN)) return false;
    return true;
  }
  static void collect(lv_obj_t *o, std::vector<lv_obj_t *> &out) {
    const uint32_t n = lv_obj_get_child_count(o);
    for (uint32_t i = 0; i < n; i++) {
      lv_obj_t *c = lv_obj_get_child(o, i);
      if (lv_obj_has_flag(c, LV_OBJ_FLAG_HIDDEN)) continue;
      if (lv_obj_check_type(c, &lv_button_class) && lv_obj_has_flag(c, LV_OBJ_FLAG_CLICKABLE)) out.push_back(c);
      else collect(c, out);
    }
  }
  static lv_point_t centre(lv_obj_t *o) {
    lv_area_t a;
    lv_obj_get_coords(o, &a);
    return {(a.x1 + a.x2) / 2, (a.y1 + a.y2) / 2};
  }
  void ring(lv_obj_t *o, bool on) {
    if (o == nullptr || !lv_obj_is_valid(o)) return;
    lv_obj_set_style_outline_width(o, on ? 2 : 0, 0);
    lv_obj_set_style_outline_color(o, lv_color_hex(0xE8EEF3), 0);
    lv_obj_set_style_outline_opa(o, on ? 230 : 0, 0);
    lv_obj_set_style_outline_pad(o, 2, 0);
  }
  void set_focus(lv_obj_t *o) {
    if (focus != nullptr && focus != o) ring(focus, false);
    focus = o;
    ring(focus, true);
    if (focus != nullptr) lv_obj_scroll_to_view_recursive(focus, LV_ANIM_ON);
  }
  void clear() { ring(focus, false); focus = nullptr; }

  void move(int k) {
    lv_obj_t *layer = noir::laag_van(noir::st().seite);
    if (layer == nullptr) return;
    std::vector<lv_obj_t *> all;
    collect(layer, all);
    if (all.empty()) return;
    if (focus == nullptr || !lv_obj_is_valid(focus) || !shown(focus)) {   // first press: show the ring top left
      lv_obj_t *best = all[0];
      for (lv_obj_t *o : all) { lv_point_t c = centre(o), b = centre(best); if (c.y < b.y - 8 || (abs(c.y - b.y) <= 8 && c.x < b.x)) best = o; }
      set_focus(best);
      return;
    }
    const lv_point_t f = centre(focus);
    lv_obj_t *best = nullptr;
    long best_score = 0;
    for (lv_obj_t *o : all) {
      if (o == focus) continue;
      const lv_point_t c = centre(o);
      const int dx = c.x - f.x, dy = c.y - f.y;
      int along, across;   // distance in the direction, and sideways
      switch (k) {
        case 14: along = -dy; across = abs(dx); break;   // up
        case 35: along = dy; across = abs(dx); break;    // down
        case 15: along = -dx; across = abs(dy); break;   // left
        default: along = dx; across = abs(dy); break;    // right
      }
      if (along <= 4) continue;
      const long score = (long) along + 2L * across;     // prefer straight ahead
      if (best == nullptr || score < best_score) { best = o; best_score = score; }
    }
    if (best != nullptr) set_focus(best);
  }

  void press(bool longp) {
    if (focus == nullptr || !lv_obj_is_valid(focus) || !shown(focus)) return;
    lv_obj_t *o = focus;
    if (longp) { lv_obj_send_event(o, LV_EVENT_LONG_PRESSED, nullptr); return; }
    lv_obj_send_event(o, LV_EVENT_SHORT_CLICKED, nullptr);   // lamp tiles listen to this one
    if (lv_obj_is_valid(o)) lv_obj_send_event(o, LV_EVENT_CLICKED, nullptr);   // all other buttons
  }

  bool key(int k, bool pressed) override {
    if (!is_dir(k) && k != 34) return false;
    if (noir::st().seite != page) { clear(); page = noir::st().seite; }
    last = millis();
    if (k == 34) {   // OK: short = tap, held 0.5 s = long press
      if (pressed) {
        if (focus == nullptr || !lv_obj_is_valid(focus) || !shown(focus)) { move(14); ok_down = 0; return true; }   // first OK only shows the ring
        ok_down = millis();
        return true;
      }
      if (ok_down != 0) press(millis() - ok_down >= 500);
      ok_down = 0;
      return true;
    }
    if (pressed) move(k);
    return true;
  }

  void tick(uint32_t now) override {
    if (focus != nullptr && (noir::st().seite != page || now - last > 8000)) clear();   // ring goes after 8 s without keys
  }
};

}  // namespace noirapp
