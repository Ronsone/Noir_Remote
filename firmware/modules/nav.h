#pragma once
#include "core.h"
// Module Nav: move around the screen with the D-pad. A light focus ring sits on a button or slider; the arrows move it
// to the nearest one in that direction. OK presses a button (hold OK = long press, e.g. dim a lamp); on a slider
// left/right change its value.
// Rule: the screen you see decides - on the TV screen the D-pad goes to the TV (module Tv), on the Airco screen it sets
// temperature and mode (module Airco); everywhere else it moves around the remote. Modules before this one get the
// keys first, so this one only sees the arrows and OK that nobody else wanted.
// The ring is its own object on the top layer: screens that refresh their buttons (Audio every 2 s) can't wipe it.
namespace noirapp {

struct Nav : Module {
  lv_obj_t *focus{nullptr}, *ring{nullptr};
  int page{-1};
  uint32_t last{0}, ok_down{0}, slide_release{0};
  const char *name() const override { return "nav"; }

  static bool is_dir(int k) { return k == 14 || k == 35 || k == 15 || k == 32; }
  static bool is_slider(lv_obj_t *o) { return o != nullptr && lv_obj_check_type(o, &lv_slider_class); }

  static bool shown(lv_obj_t *o) {   // not hidden, and none of its parents
    for (lv_obj_t *p = o; p != nullptr; p = lv_obj_get_parent(p)) if (lv_obj_has_flag(p, LV_OBJ_FLAG_HIDDEN)) return false;
    return true;
  }
  static void collect(lv_obj_t *o, std::vector<lv_obj_t *> &out) {
    const uint32_t n = lv_obj_get_child_count(o);
    for (uint32_t i = 0; i < n; i++) {
      lv_obj_t *c = lv_obj_get_child(o, i);
      if (lv_obj_has_flag(c, LV_OBJ_FLAG_HIDDEN)) continue;
      if ((lv_obj_check_type(c, &lv_button_class) && lv_obj_has_flag(c, LV_OBJ_FLAG_CLICKABLE)) || is_slider(c)) out.push_back(c);
      else collect(c, out);
    }
  }
  static lv_point_t centre(lv_obj_t *o) {
    lv_area_t a;
    lv_obj_get_coords(o, &a);
    return {(a.x1 + a.x2) / 2, (a.y1 + a.y2) / 2};
  }
  bool valid() const { return focus != nullptr && lv_obj_is_valid(focus) && shown(focus); }

  // ---- the ring: follows the focused object (also when it scrolls) ----
  void place_ring() {
    if (!valid()) { if (ring != nullptr) lv_obj_add_flag(ring, LV_OBJ_FLAG_HIDDEN); return; }
    if (ring == nullptr) {
      ring = lv_obj_create(lv_layer_top());
      lv_obj_remove_style_all(ring);
      lv_obj_remove_flag(ring, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_remove_flag(ring, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_style_border_color(ring, lv_color_hex(0xE8EEF3), 0);
      lv_obj_set_style_border_opa(ring, 235, 0);
      lv_obj_set_style_border_width(ring, 2, 0);
    }
    lv_area_t a;
    lv_obj_get_coords(focus, &a);
    const int pad = is_slider(focus) ? 9 : 0;   // a slider is thin: ring around it with room
    lv_obj_set_pos(ring, a.x1 - pad, a.y1 - pad);
    lv_obj_set_size(ring, a.x2 - a.x1 + 1 + 2 * pad, a.y2 - a.y1 + 1 + 2 * pad);
    lv_obj_set_style_radius(ring, is_slider(focus) ? 12 : lv_obj_get_style_radius(focus, LV_PART_MAIN), 0);
    lv_obj_remove_flag(ring, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(ring);
  }
  void set_focus(lv_obj_t *o) {
    focus = o;
    if (focus != nullptr) lv_obj_scroll_to_view_recursive(focus, LV_ANIM_OFF);
    place_ring();
  }
  void clear() { focus = nullptr; place_ring(); }

  void move(int k) {
    lv_obj_t *layer = noir::laag_van(noir::st().seite);
    if (layer == nullptr) return;
    std::vector<lv_obj_t *> all;
    collect(layer, all);
    if (all.empty()) return;
    if (!valid()) {   // first press: show the ring top left
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

  // left/right on a slider: change it like a finger would (label right away, the value is sent when you stop)
  void slide(int dir) {
    const int lo = lv_slider_get_min_value(focus), hi = lv_slider_get_max_value(focus);
    const int step = std::max(1, (hi - lo) / 20);
    const int v = std::max(lo, std::min(hi, (int) lv_slider_get_value(focus) + dir * step));
    lv_obj_add_state(focus, LV_STATE_PRESSED);   // the screen's refresh leaves a pressed slider alone
    lv_slider_set_value(focus, v, LV_ANIM_OFF);
    lv_obj_send_event(focus, LV_EVENT_VALUE_CHANGED, nullptr);
    slide_release = millis() + 500;
  }

  void press(bool longp) {
    if (!valid() || is_slider(focus)) return;
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
        if (!valid()) { move(14); ok_down = 0; return true; }   // first OK only shows the ring
        ok_down = millis();
        return true;
      }
      if (ok_down != 0) press(millis() - ok_down >= 500);
      ok_down = 0;
      return true;
    }
    if (!pressed) return true;
    if (valid() && is_slider(focus) && (k == 15 || k == 32)) { slide(k == 32 ? 1 : -1); return true; }
    move(k);
    return true;
  }

  void tick(uint32_t now) override {
    if (slide_release != 0 && (int32_t) (now - slide_release) >= 0) {   // stopped sliding: send the value
      slide_release = 0;
      if (valid() && is_slider(focus)) {
        lv_obj_remove_state(focus, LV_STATE_PRESSED);
        lv_obj_send_event(focus, LV_EVENT_RELEASED, nullptr);
      }
    }
    if (focus != nullptr && (noir::st().seite != page || now - last > 8000 || !noir::st().zichtbaar)) { clear(); return; }
    if (focus != nullptr) place_ring();   // follow scrolling and rebuilt screens
  }
};

}  // namespace noirapp
