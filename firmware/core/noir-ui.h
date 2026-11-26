#pragma once
// NOIR - the screens of the remote (LVGL 9). Shared by the firmware and the simulator (simulator/).
// One layer per screen over page_home; the status bar moves to the visible one. The firmware modules (modules/*.h)
// fill the data and handle the actions; this file only builds and shows.
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cmath>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif
#include "mdi_icons.h"   // mdi:name -> glyph (next to this header)

namespace noir {

// Kleuren van de simulator (skin NOIR / Obsidian)
static const uint32_t BG = 0x07080A, SURF = 0x1A1C20, SURF2 = 0x101114, SURF_DIM = 0x101216, TEXT = 0xF2F3F5,
                      TEXT2 = 0xE4E7EC, MUTED = 0x969AA3, STAT = 0xB6BAC3, BORDER = 0x383C43, BORDER_P = 0xAEB4BF,
                      PRESSED_BG = 0x363A43, PRESSED_BG2 = 0x252931, LINE = 0xC9D0DC, BATT = 0xA4AAB5, BATT_F = 0xD3D8E0,
                      DOT = 0xD7DCE5;

// Acties van de tegels (zelfde betekenis als action() in de simulator: 101-104 tegels, 113 instellingen, 1 Terug,
// 100 Thuis)
// ---- NOIR-instellingen (2026-11-22, simulator build(13) "UI-instellingen") --------------------------------------
// Skin = vier grondkleuren + afronding; alle vaste kleuren hierboven worden bij het tekenen omgezet zoals c() in de
// simulator (k()). Skin 0 Obsidian geeft precies de kleuren van de simulator.
struct Theme {
  int skin{0};        // 0 Obsidian, 1 Graphite, 2 Pure Black, 3 Nautisch
  int radius{16};     // afronding van knoppen en tegels (8-20 px)
  uint32_t bg{BG}, surf{SURF}, text{TEXT}, accent{0xBFC2C9};
};
inline Theme &theme() { static Theme t; return t; }
static const int SKINS = 4;
static const char *SKIN_NAMES[SKINS] = {"NOIR / Obsidian", "SIGNATURE / Graphite", "PURE / Black", "Nautisch / Navy"};
// bg, surf, text, accent - radius
static const uint32_t SKIN_COLOR[4][4] = {{0x07080A, 0x1A1C20, 0xF2F3F5, 0xBFC2C9}, {0x0C0E12, 0x20242B, 0xF2F3F5, 0xD4D9E2},
                                          {0x000000, 0x111214, 0xF5F5F5, 0xCCCCCC}, {0x071322, 0x14283A, 0xF5EEDC, 0xD6AB61}};
static const int SKIN_RADIUS[4] = {16, 14, 12, 16};

inline uint32_t mix(uint32_t a, uint32_t b, int pct) {
  uint32_t v = 0;
  for (int s = 0; s < 24; s += 8) v |= ((((a >> s) & 255) * (100 - pct) + ((b >> s) & 255) * pct) / 100) << s;
  return v;
}
// Vaste simulatorkleur -> kleur van de skin (simulator: c())
inline uint32_t tint(uint32_t x) {
  const Theme &t = theme();
  switch (x) {
    case 0x07080A: case 0x08090B: return t.bg;
    case 0x101216: return mix(t.bg, t.surf, 35);
    case 0x1A1C20: case 0x14161A: case 0x22252A: return t.surf;
    case 0x101114: return mix(t.surf, t.bg, 55);
    case 0xF2F3F5: case 0xE4E7EC: case 0xEFF1F5: return t.text;
    case 0x969AA3: case 0xB6BAC3: return mix(t.text, t.bg, 37);
    case 0xBFC2C9: case 0xAEB4BF: case 0xD3D8E0: return t.accent;
    case 0x383C43: case 0x36393F: return mix(t.surf, t.text, 14);
    default: return x;
  }
}
inline lv_color_t k(uint32_t c) { return lv_color_hex(tint(c)); }
// simulator box(): afrondingen vanaf 10 px volgen de skin
inline int rnd(int r) { return r >= 10 && r < LV_RADIUS_CIRCLE ? theme().radius : r; }

enum Action : int { BACK = 1, HOME = 100, TV = 101, AUDIO = 102, LIGHTS = 103, SCENES = 104, AIRCO = 105, SETTINGS = 113,
                   SET_WIFI = 131, SET_FULL = 132, SET_ECO = 133, DIM_DONE = 128,
                   AIRCO_POWER = 140, AIRCO_MIN = 141, AIRCO_PLUS = 142, AIRCO_FAN = 143, AIRCO_SWING = 144, AIRCO_UNIT = 147, KEYTEST = 160, TEST_RESET = 161, SCENE_ALL_OFF = 170,
                   AUDIO_NOW = 107 /* tap "now playing": play/pause */, AUDIO_POWER = 189,
                   AUDIO_SOURCE = 4600 /* 4600+i: source i of the speaker */,
                   SCENE = 3000 /* 3000+i: scene i (NOIR Builder) */,
                   AIRCO_MODE = 150 /* 150-154 */,
                   LAMP = 400 /* 400+i: lamp i op NOIR Verlichting */,
                   TV_POWER = 120, TV_APP = 200 /* 200-207: source tiles */,
                   ITEM = 1000 /* 1000+i: tegel i van de configuratorpagina "noir" */ };

struct Fonts { const lv_font_t *f12{nullptr}, *f14{nullptr}, *f16{nullptr}, *ico{nullptr}, *f20{nullptr}, *large{nullptr}; };

// TV sources (NOIR Builder): name on the tile, source for HA, logo key (logos.yaml). Up to 8.
struct TvApp { std::string name, source, logo; };
inline std::vector<TvApp> &tv_apps() { static std::vector<TvApp> a; return a; }
inline std::map<std::string, const void *> &logos() { static std::map<std::string, const void *> m; return m; }
inline const void *logo_src(const std::string &k) { auto it = logos().find(k); return it == logos().end() ? nullptr : it->second; }
struct ImageSet {
  const void *wall{nullptr}, *tv{nullptr}, *audio{nullptr}, *light{nullptr}, *scenes{nullptr};
};

enum Screen : int { S_HOME = 0, S_TV = 1, S_LIGHTS = 2, S_SETTINGS = 3, S_AIRCO = 4, S_TEST = 5, S_SCENES = 6, S_AUDIO = 7, S_COUNT = 8 };

struct UiState {
  lv_obj_t *layer_obj{nullptr};                  // NOIR Thuis
  lv_obj_t *tv{nullptr};                    // NOIR TV (2026-11-21)
  lv_obj_t *topbar{nullptr};                   // statusbalk, hangt aan de zichtbare laag
  lv_obj_t *clock{nullptr}, *batt_txt{nullptr}, *batt_fill{nullptr}, *ha_dot{nullptr};
  lv_obj_t *wifi[4]{};                      // wifi-streepjes in de statusbalk (2026-11-22)
  lv_obj_t *tv_status{nullptr};
  lv_obj_t *home_body{nullptr};            // tegels van NOIR Thuis (opnieuw opgebouwd bij nieuwe configuratie)
  Fonts fonts;                              // voor het herbouwen
  ImageSet images;
  int cur_screen{S_HOME};                       // laatst getoonde NOIR-pagina
  std::string tv_state;                     // "on", "off", "unavailable" ... (uit HA)
  bool visible{false};
  std::function<void(int)> actie;   // filled in by the firmware (noirapp::action)
  const lv_font_t *toast_font{nullptr};
  // 2026-11-22: Verlichting en UI-instellingen, herbouwen na skinwissel
  lv_obj_t *light{nullptr}, *settings{nullptr}, *lights_body{nullptr}, *airco{nullptr}, *test{nullptr}, *scenes{nullptr}, *scenes_body{nullptr};
  std::function<JsonArrayConst()> scenes_items;   // configuratorpagina "noir_scenes"
  lv_obj_t *audio{nullptr}, *audio_body{nullptr};
  lv_obj_t *home{nullptr};
  std::function<JsonArrayConst()> items;    // tegels van NOIR Thuis (configurator), voor het herbouwen
};
inline UiState &st() { static UiState s; return s; }


// Language (NOIR Remote): the screens are written in Dutch; tr() gives the English text when another language is chosen.
// Text that is not in the table (names from the configuration) stays as it is.
inline bool &english() { static bool e = false; return e; }
inline bool &eco_mode() { static bool e = false; return e; }   // shown on the settings screen
// Other languages (de, fr, es, ...): a language pack from NOIR Remote (cfg.texts, text -> translation); what is missing
// there falls back to English.
inline std::map<std::string, std::string> &texts() { static std::map<std::string, std::string> t; return t; }
inline const char *tr(const char *nl) {
  if (!english() || nl == nullptr) return nl;
  if (!texts().empty()) { auto it = texts().find(nl); if (it != texts().end()) return it->second.c_str(); }
  static const char *const T[][2] = {
    {"Terug", "Back"}, {"Thuis", "Home"}, {"Verlichting", "Lights"}, {"Sc\xC3\xA8nes", "Scenes"}, {"Alles uit", "All off"},
    {"Instellingen", "Settings"}, {"Skin", "Skin"}, {"Modus", "Mode"}, {"Wi-Fi herstellen", "Restore Wi-Fi"},
    {"Kies je bronnen in de NOIR Builder", "Choose your sources in NOIR Builder"}, {"TV aan", "TV on"}, {"TV uit", "TV off"},
    {"Gedempt", "Muted"}, {"Geluid aan", "Sound on"},
    {"Uit", "Off"}, {"Aan", "On"}, {"Niet bereikbaar", "Unreachable"}, {"Gereed", "Done"},
    {"Lang drukken = dimmen", "Hold = dim"}, {"Knoppentest", "Button test"},
    {"Nog geen lampen. Configurator \xE2\x86\x92 NOIR \xE2\x86\x92 Verlichting.", "No lights yet. Add them in the NOIR Builder."},
    {"Eigen sc\xC3\xA8nes: configurator \xE2\x86\x92 NOIR \xE2\x86\x92 Sc\xC3\xA8nes.", "Add scenes in the NOIR Builder."},
    {"Apparaten en apps", "Devices and apps"}, {"Wordt geladen\xE2\x80\xA6", "Loading\xE2\x80\xA6"},
    {"TV aan", "TV on"}, {"TV uit", "TV off"}, {"Volle helderheid", "Full brightness"},
    {"Ingang", "Input"}, {"Niveau", "Level"}, {"Niets aan het afspelen", "Nothing playing"}, {"MUZIEK", "MUSIC"},
    {"Volumetoetsen op de remote regelen deze speaker.", "The volume keys control this speaker."},
    {"uit", "off"}, {"aan", "on"}, {"niet bereikbaar", "unreachable"},
    {"Automatisch", "Auto"}, {"Koelen", "Cooling"}, {"Ontvochtigen", "Drying"}, {"Ventileren", "Fan"}, {"Verwarmen", "Heating"},
    {"kamer", "room"},
  };
  for (const auto &r : T) if (strcmp(r[0], nl) == 0) return r[1];
  return nl;
}

inline lv_obj_t *label(lv_obj_t *p, const char *t, int x, int y, int w, const lv_font_t *f, uint32_t tint) {
  lv_obj_t *l = lv_label_create(p);
  lv_label_set_text(l, tr(t));
  lv_obj_set_pos(l, x, y);
  lv_obj_set_width(l, w);
  if (f != nullptr) lv_obj_set_style_text_font(l, f, 0);
  lv_obj_set_style_text_color(l, k(tint), 0);
  lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_DOTS);
  lv_obj_remove_flag(l, LV_OBJ_FLAG_CLICKABLE);
  return l;
}

inline lv_obj_t *detail(lv_obj_t *p, int x, int y, int w, int h, uint32_t tint, int opa, int radius) {
  lv_obj_t *o = lv_obj_create(p);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_color(o, k(tint), 0);
  lv_obj_set_style_bg_opa(o, (lv_opa_t) opa, 0);
  lv_obj_set_style_radius(o, rnd(radius), 0);
  lv_obj_remove_flag(o, (lv_obj_flag_t) (LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE));
  return o;
}

inline void click_(lv_event_t *e) {
  const int a = (int) (intptr_t) lv_event_get_user_data(e);
  if (st().actie) st().actie(a);
}

// Knop zoals button() in de simulator (grafietvlak, verloop, fijne bovenrand, aparte ingedrukte toestand)
inline lv_obj_t *btn(lv_obj_t *p, int x, int y, int w, int h, int actie, lv_event_code_t code = LV_EVENT_CLICKED) {
  lv_obj_t *b = lv_button_create(p);
  lv_obj_set_pos(b, x, y);
  lv_obj_set_size(b, w, h);
  lv_obj_set_style_bg_color(b, k(SURF), 0);
  lv_obj_set_style_bg_opa(b, 232, 0);
  lv_obj_set_style_radius(b, rnd(h >= 60 ? 16 : h <= 34 ? 11 : 13), 0);
  lv_obj_set_style_border_width(b, 1, 0);
  lv_obj_set_style_border_color(b, k(BORDER), 0);
  lv_obj_set_style_border_opa(b, 115, 0);
  lv_obj_set_style_pad_all(b, 0, 0);
  lv_obj_set_style_shadow_width(b, 0, 0);
  lv_obj_set_style_outline_width(b, 0, LV_STATE_FOCUSED);
  lv_obj_set_style_bg_grad_color(b, k(SURF2), 0);
  lv_obj_set_style_bg_grad_dir(b, LV_GRAD_DIR_VER, 0);
  lv_obj_set_style_bg_color(b, k(PRESSED_BG), LV_STATE_PRESSED);
  lv_obj_set_style_bg_grad_color(b, k(PRESSED_BG2), LV_STATE_PRESSED);
  lv_obj_set_style_border_color(b, k(BORDER_P), LV_STATE_PRESSED);
  lv_obj_set_style_border_opa(b, 210, LV_STATE_PRESSED);
  lv_obj_set_style_border_color(b, k(BORDER), LV_STATE_FOCUSED);   // theme-focusrand niet blauw
  lv_obj_add_event_cb(b, click_, code, (void *) (intptr_t) actie);
  if (h >= 40) detail(b, 13, 0, w - 26, 1, LINE, 42, 0);
  return b;
}

// Tegel op NOIR Thuis: icoon linksboven (afbeelding of MDI-teken), naam linksonder. h = hoogte, w = breedte.
inline lv_obj_t *tile(lv_obj_t *body, const char *t, int x, int y, int actie, const void *icon_src, const Fonts &f,
                       int w = 104, int h = 66, const char *mdi = nullptr) {
  lv_obj_t *b = btn(body, x, y, w, h, actie);
  const int iy = h >= 60 ? 10 : 8;
  if (icon_src != nullptr) {
    lv_obj_t *im = lv_image_create(b);
    lv_image_set_src(im, icon_src);
    lv_obj_set_pos(im, 13, iy);
    lv_obj_set_style_image_opa(im, 210, 0);
    lv_obj_remove_flag(im, LV_OBJ_FLAG_CLICKABLE);
  } else if (mdi != nullptr && f.ico != nullptr) {
    lv_obj_t *i = lv_label_create(b);
    lv_label_set_text(i, mdi);
    lv_obj_set_style_text_font(i, f.ico, 0);
    lv_obj_set_style_text_color(i, k(TEXT2), 0);
    lv_obj_set_style_text_opa(i, 210, 0);
    lv_obj_set_pos(i, 12, iy - 1);
    lv_obj_remove_flag(i, LV_OBJ_FLAG_CLICKABLE);
  }
  label(b, t, 12, h - 25, w - 24, f.f14, TEXT2);
  return b;
}
static const char *ICO_SNOW = "\U000F0717";   // mdi:snowflake (airco)

// Terug / Thuis onderaan: donkerder vlak zonder verloop (simulator: p == scr && a == 1 || a == 100)
inline void bottom_button(lv_obj_t *p, const char *ico, const char *t, int x, int actie, const Fonts &f) {
  lv_obj_t *b = btn(p, x, 270, 104, 42, actie);
  lv_obj_set_style_bg_opa(b, 180, 0);
  lv_obj_set_style_bg_color(b, k(SURF_DIM), 0);
  lv_obj_set_style_bg_grad_dir(b, LV_GRAD_DIR_NONE, 0);
  lv_obj_set_style_radius(b, rnd(14), 0);
  lv_obj_set_style_border_opa(b, 65, 0);
  lv_obj_t *row_obj = lv_obj_create(b);
  lv_obj_remove_style_all(row_obj);
  lv_obj_set_size(row_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row_obj, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row_obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row_obj, 7, 0);
  lv_obj_remove_flag(row_obj, (lv_obj_flag_t) (LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE));
  lv_obj_center(row_obj);
  if (f.ico != nullptr && ico != nullptr) {
    lv_obj_t *i = lv_label_create(row_obj);
    lv_label_set_text(i, ico);
    lv_obj_set_style_text_font(i, f.ico, 0);
    lv_obj_set_style_text_color(i, k(TEXT2), 0);
  }
  lv_obj_t *l = lv_label_create(row_obj);
  lv_label_set_text(l, tr(t));
  if (f.f12 != nullptr) lv_obj_set_style_text_font(l, f.f12, 0);
  lv_obj_set_style_text_color(l, k(TEXT2), 0);
}

// MDI-codes in fonts/mdi-subset.ttf (font mdi_menu bevat ze)
static const char *ICO_COG = "\U000F0493";
static const char *ICO_BACK = "\U000F004D";
static const char *ICO_HOME = "\U000F02DC";

// Lege NOIR-laag over page_home (volle 240 x 320, achtergrond + Obsidian-wallpaper)
inline lv_obj_t *layer_make(lv_obj_t *home, const ImageSet &bl) {
  lv_obj_t *layer_obj = lv_obj_create(home);
  lv_obj_remove_style_all(layer_obj);
  lv_obj_add_flag(layer_obj, LV_OBJ_FLAG_IGNORE_LAYOUT);   // page_home heeft padding (st_screen): de laag dekt alles
  lv_obj_add_flag(layer_obj, LV_OBJ_FLAG_FLOATING);
  // Posities van kinderen tellen vanaf de binnenrand (padding) van page_home: terugschuiven naar de schermhoek
  lv_obj_set_pos(layer_obj, -lv_obj_get_style_pad_left(home, LV_PART_MAIN), -lv_obj_get_style_pad_top(home, LV_PART_MAIN));
  lv_obj_set_size(layer_obj, 240, 320);
  lv_obj_set_style_bg_color(layer_obj, k(BG), 0);
  lv_obj_set_style_bg_opa(layer_obj, LV_OPA_COVER, 0);
  lv_obj_remove_flag(layer_obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(layer_obj, LV_OBJ_FLAG_CLICKABLE);   // a tap on the background stays on this layer
  if (bl.wall != nullptr) {   // wallpaper
    lv_obj_t *bg = lv_image_create(layer_obj);
    lv_image_set_src(bg, bl.wall);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_set_style_image_opa(bg, 210, 0);
    lv_obj_remove_flag(bg, LV_OBJ_FLAG_CLICKABLE);
  }
  lv_obj_add_flag(layer_obj, LV_OBJ_FLAG_HIDDEN);
  return layer_obj;
}

// Statusbalk (klok links, HA-punt, accu-% en accusymbool rechts) - een keer, hangt aan de zichtbare laag
inline void topbar_make(lv_obj_t *parent_obj, const Fonts &f) {
  UiState &s = st();
  lv_obj_t *topbar = s.topbar = lv_obj_create(parent_obj);
  lv_obj_remove_style_all(topbar);
  lv_obj_set_pos(topbar, 0, 0);
  lv_obj_set_size(topbar, 240, 26);
  lv_obj_remove_flag(topbar, (lv_obj_flag_t) (LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE));
  s.clock = label(topbar, "--:--", 12, 7, 60, f.f12, STAT);
  s.batt_txt = label(topbar, "", 151, 7, 31, f.f12, STAT);
  lv_obj_set_style_text_align(s.batt_txt, LV_TEXT_ALIGN_RIGHT, 0);
  s.ha_dot = detail(topbar, 143, 11, 3, 3, DOT, 45, LV_RADIUS_CIRCLE);
  for (int i = 0; i < 4; i++) s.wifi[i] = detail(topbar, 122 + i * 4, 16 - i * 2, 2, 4 + i * 2, STAT, 45, 0);   // 4 streepjes, onder gelijk
  lv_obj_t *bat = detail(topbar, 192, 9, 25, 11, 0x08090B, 100, 3);
  lv_obj_set_style_border_width(bat, 1, 0);
  lv_obj_set_style_border_color(bat, k(BATT), 0);
  s.batt_fill = detail(bat, 3, 3, 15, 5, BATT_F, 220, 1);
  detail(topbar, 218, 12, 2, 5, BATT, 180, 1);
}

inline lv_obj_t *body_make(lv_obj_t *layer_obj, bool scroll) {
  lv_obj_t *body = lv_obj_create(layer_obj);
  lv_obj_remove_style_all(body);
  lv_obj_set_pos(body, 12, 30);
  lv_obj_set_size(body, 216, 230);
  lv_obj_set_style_pad_bottom(body, 6, 0);
  if (scroll) {
    lv_obj_set_scroll_dir(body, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(body, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(body, LV_OBJ_FLAG_SCROLL_ELASTIC);
  } else {
    lv_obj_remove_flag(body, LV_OBJ_FLAG_SCROLLABLE);
  }
  return body;
}

// ---- NOIR-builder (2026-11-22): tegels van NOIR Thuis uit de configurator --------------------------------------
// Pagina met id "noir" in het menu van de configurator (cfg.menu.pages). Elke tegel = een menu-item met x/y/w/h op
// het raster van de configurator (6 kolommen). NOIR: kolom = (216 + 8) / 6 px, rij = 31 px (h 2 = 56 px hoog).
// icon: "mdi:naam" (fonts/mdi-subset.ttf, zie components/menu/mdi_icons.h) of "noir:tv|audio|licht|scenes" (beelden).
// Wat een tik doet, regelt de firmware (tasten::noir_item_actie): page, special, action (steps), toggle/light/cover.
static const int ROW = 31, ROW_GAP = 6, COL_GAP = 8, TILES_Y = 52;

inline std::string utf8_(uint32_t cp) {
  std::string o;
  if (cp < 0x80) o += (char) cp;
  else if (cp < 0x800) { o += (char) (0xC0 | (cp >> 6)); o += (char) (0x80 | (cp & 0x3F)); }
  else if (cp < 0x10000) { o += (char) (0xE0 | (cp >> 12)); o += (char) (0x80 | ((cp >> 6) & 0x3F)); o += (char) (0x80 | (cp & 0x3F)); }
  else { o += (char) (0xF0 | (cp >> 18)); o += (char) (0x80 | ((cp >> 12) & 0x3F)); o += (char) (0x80 | ((cp >> 6) & 0x3F)); o += (char) (0x80 | (cp & 0x3F)); }
  return o;
}
// "mdi:air-conditioner" -> UTF-8 van het teken in mdi_menu, "" = onbekend
inline std::string mdi_teken(const char *icon) {
  if (icon == nullptr || strncmp(icon, "mdi:", 4) != 0) return "";
  for (const auto &e : MDI_ICONS)
    if (strcmp(e.name, icon + 4) == 0) return utf8_(e.cp);
  return "";
}
inline const void *noir_beeld(const char *icon, const ImageSet &bl) {
  if (icon == nullptr || strncmp(icon, "noir:", 5) != 0) return nullptr;
  const char *n = icon + 5;
  if (!strcmp(n, "tv")) return bl.tv;
  if (!strcmp(n, "audio")) return bl.audio;
  if (!strcmp(n, "licht")) return bl.light;
  if (!strcmp(n, "scenes")) return bl.scenes;
  return nullptr;
}

// Een tegel uit een item (JSON uit de configurator) op de NOIR-maat
// Airco tile on Home: mini dial with the temperature + the mode (design B). Filled by airco::tonen().
struct AircoTile { lv_obj_t *arc_obj{nullptr}, *temp{nullptr}, *mode_lbl{nullptr}; };
inline AircoTile &airco_tile() { static AircoTile t; return t; }
inline void airco_tile_make(lv_obj_t *b, int w, const Fonts &f) {
  AircoTile &t = airco_tile();
  lv_obj_t *name0 = lv_obj_get_child(b, -1);   // the name label tegel() made last
  lv_obj_t *icon0 = lv_obj_get_child(b, -2);  // the icon before it (child 0 is the thin top line)
  t.arc_obj = lv_arc_create(b);
  lv_obj_set_size(t.arc_obj, 36, 36);
  lv_obj_set_pos(t.arc_obj, w - 44, 6);
  lv_arc_set_bg_angles(t.arc_obj, 135, 45);
  lv_arc_set_range(t.arc_obj, 0, 100);
  lv_obj_remove_style(t.arc_obj, nullptr, LV_PART_KNOB);
  lv_obj_remove_flag(t.arc_obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(t.arc_obj, 3, LV_PART_MAIN);
  lv_obj_set_style_arc_color(t.arc_obj, k(BORDER), LV_PART_MAIN);
  lv_obj_set_style_arc_width(t.arc_obj, 3, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(t.arc_obj, k(0x9FD8F0), LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(t.arc_obj, true, LV_PART_INDICATOR);
  t.temp = label(b, "", w - 44, 15, 36, f.f14 != nullptr ? f.f14 : f.f12, TEXT);
  lv_obj_set_style_text_align(t.temp, LV_TEXT_ALIGN_CENTER, 0);
  // name a bit higher (tegel() put it at the bottom), the mode underneath: icon / name / mode
  lv_obj_t *name_s = name0;
  lv_obj_update_layout(b);
  const int h = lv_obj_get_height(b);
  if (icon0 != nullptr && icon0 != name_s) lv_obj_set_y(icon0, 4);   // three lines in 56 px: icon 4-24, name 22-39, mode 40-55
  if (name_s != nullptr && lv_obj_check_type(name_s, &lv_label_class)) lv_obj_set_y(name_s, h - 34);
  t.mode_lbl = label(b, "", 12, h - 17, w - 56, f.f12, MUTED);
  lv_label_set_long_mode(t.mode_lbl, LV_LABEL_LONG_MODE_CLIP);
}
inline void item_tile(lv_obj_t *body, JsonObjectConst it, int index, const Fonts &f, const ImageSet &bl) {
  const int x = std::max(0, std::min(5, (int) (it["x"] | 0))), w = std::max(1, std::min(6 - x, (int) (it["w"] | 6)));
  const int y = std::max(0, (int) (it["y"] | 0)), h = std::max(1, std::min(6, (int) (it["h"] | 2)));
  const int px = x * (216 + COL_GAP) / 6, pw = (x + w) * (216 + COL_GAP) / 6 - px - COL_GAP;
  const int py = y * ROW, ph = h * ROW - ROW_GAP;
  const char *label_t = it["label"] | "";
  const char *icon = it["icon"] | "";
  const std::string t = it["t"] | "";
  if (t == "text") {   // alleen tekst (kop/uitleg)
    lv_obj_t *l = label(body, label_t, px, py + 4, pw, h >= 2 ? f.f16 : f.f12, h >= 2 ? TEXT : MUTED);
    lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_WRAP);
    return;
  }
  const void *img_src = noir_beeld(icon, bl);
  const std::string glyph = img_src == nullptr ? mdi_teken(icon) : std::string();
  if (ph >= 46) {   // groot: icoon linksboven, naam linksonder (zoals de simulator)
    lv_obj_t *b = tile(body, label_t, px, py, ITEM + index, img_src, f, pw, ph, glyph.empty() ? nullptr : glyph.c_str());
    if (std::string(it["fn"] | "") == "airco") airco_tile_make(b, pw, f);
    return;
  }
  // klein (1 rij): icoon en naam naast elkaar
  lv_obj_t *b = btn(body, px, py, pw, ph, ITEM + index);
  int lx = 10;
  if (img_src != nullptr || !glyph.empty()) {
    lv_obj_t *i;
    if (img_src != nullptr) { i = lv_image_create(b); lv_image_set_src(i, img_src); lv_image_set_scale(i, 180); }
    else { i = lv_label_create(b); lv_label_set_text(i, glyph.c_str()); if (f.ico != nullptr) lv_obj_set_style_text_font(i, f.ico, 0);
           lv_obj_set_style_text_color(i, k(TEXT2), 0); }
    lv_obj_align(i, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_remove_flag(i, LV_OBJ_FLAG_CLICKABLE);
    lx = 34;
  }
  lv_obj_t *l = label(b, label_t, lx, 0, pw - lx - 6, f.f12, TEXT2);
  lv_obj_align(l, LV_ALIGN_LEFT_MID, lx, 0);
}

// NOIR Thuis (simulator build(0))
// items = tegels van de configuratorpagina "noir" (leeg = de ingebouwde tegels)
inline void home_build(lv_obj_t *layer_obj, const Fonts &f, const ImageSet &bl, JsonArrayConst items = JsonArrayConst()) {
  UiState &s = st();
  airco_tile() = AircoTile();   // the home tiles are rebuilt: the old airco tile is gone
  lv_obj_t *body = s.home_body = body_make(layer_obj, true);
  lv_obj_set_height(body, 236);
  label(body, "NOIR / SIGNATURE", 0, 5, 180, f.f16, TEXT);
  label(body, "Home Control", 0, 30, 216, f.f12, MUTED);
  lv_obj_t *tw = btn(body, 184, 0, 32, 32, SETTINGS);
  if (f.ico != nullptr) {
    lv_obj_t *i = lv_label_create(tw);
    lv_label_set_text(i, ICO_COG);
    lv_obj_set_style_text_font(i, f.ico, 0);
    lv_obj_set_style_text_color(i, k(TEXT2), 0);
    lv_obj_center(i);
  }
  if (!items.isNull() && items.size() > 0) {   // NOIR-builder: tegels uit de configurator
    lv_obj_t *grid_obj = lv_obj_create(body);
    lv_obj_remove_style_all(grid_obj);
    lv_obj_set_pos(grid_obj, 0, TILES_Y);
    lv_obj_set_size(grid_obj, 216, LV_SIZE_CONTENT);
    lv_obj_remove_flag(grid_obj, (lv_obj_flag_t) (LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));
    int i = 0;
    for (JsonObjectConst it : items) {
      if (i >= 40) break;
      item_tile(grid_obj, it, i, f, bl);
      i++;
    }
    return;
  }
  // 2026-11-22: Airco erbij - drie rijen van 56 hoog (simulator: twee van 66), Airco over de volle breedte
  tile(body, "TV", 0, 52, TV, bl.tv, f, 104, 56);
  tile(body, "Audio", 112, 52, AUDIO, bl.audio, f, 104, 56);
  tile(body, "Verlichting", 0, 114, LIGHTS, bl.light, f, 104, 56);
  tile(body, "Sc\xC3\xA8nes", 112, 114, SCENES, bl.scenes, f, 104, 56);
  tile(body, "Airco", 0, 176, AIRCO, nullptr, f, 216, 50, ICO_SNOW);
}

static const char *ICO_POWER = "\U000F0425";

// NOIR TV (simulator build(1)): titel, status uit HA, aan/uit, 6 app-tegels (scrollt), Terug/Thuis
inline void tv_bouwen(lv_obj_t *layer_obj, const Fonts &f, const ImageSet &bl) {
  UiState &s = st();
  lv_obj_t *body = body_make(layer_obj, true);
  label(body, "TV", 0, 0, 150, f.f20 != nullptr ? f.f20 : f.f16, TEXT);
  s.tv_status = label(body, "Apparaten en apps", 0, 31, 160, f.f12, MUTED);
  lv_obj_t *pw = btn(body, 164, 20, 52, 34, TV_POWER);
  if (f.ico != nullptr) {
    lv_obj_t *i = lv_label_create(pw);
    lv_label_set_text(i, ICO_POWER);
    lv_obj_set_style_text_font(i, f.ico, 0);
    lv_obj_set_style_text_color(i, k(TEXT2), 0);
    lv_obj_center(i);
  }
  const auto &apps = tv_apps();
  if (apps.empty()) {
    label(body, "Kies je bronnen in de NOIR Builder", 0, 76, 216, f.f12, MUTED);
  }
  for (size_t i = 0; i < apps.size() && i < 8; i++) {
    const int x = (i % 2) * 112, y = 60 + (i / 2) * 80;
    lv_obj_t *b = btn(body, x, y, 104, 76, TV_APP + (int) i);
    const void *lg = logo_src(apps[i].logo);
    if (lg != nullptr) {
      lv_obj_t *im = lv_image_create(b);
      lv_image_set_src(im, lg);
      lv_obj_align(im, LV_ALIGN_TOP_MID, 0, 12);
      lv_obj_remove_flag(im, LV_OBJ_FLAG_CLICKABLE);
    }
    // with a logo: small name underneath; without: the name is the tile
    lv_obj_t *l = lg != nullptr ? label(b, apps[i].name.c_str(), 3, 55, 98, f.f12, 0xBFC2C9)
                                : label(b, apps[i].name.c_str(), 3, 28, 98, f.f16, TEXT);
    lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
  }
  bottom_button(layer_obj, ICO_BACK, "Terug", 12, BACK, f);
  bottom_button(layer_obj, ICO_HOME, "Thuis", 124, HOME, f);
}

// ---- NOIR Verlichting (2026-11-22, simulator build(3)) ---------------------------------------------------------
// Per lamp een brede knop "Naam  ·  Aan/Uit" of (dimbaar) "Naam  ·  65%". Tikken = aan/uit, lang drukken (dimbaar)
// = dimvenster met schuifregler (light.turn_on brightness_pct bij loslaten). De echte stand komt elke 3 s uit HA
// (/api/noir_remote/states), only while the screen is open.
using Data = std::map<std::string, std::string>;
inline std::function<void(const std::string &, Data)> &ha();
inline void tv_status(const std::string &state, const std::string &src);
struct Lamp {
  std::string name_s, entity_ref;
  bool dim{false};
  std::string state;   // "" = nog onbekend
  int bri{-1};         // 0-255, -1 = onbekend/uit
  lv_obj_t *lbl{nullptr};
};
inline std::vector<Lamp> &lamps() { static std::vector<Lamp> l; return l; }
inline bool lamp_aan(const Lamp &l) { return l.state == "on"; }
inline int lamp_pct(const Lamp &l) { return l.bri < 0 ? -1 : (l.bri * 100 + 127) / 255; }

inline void lamp_text(int i) {
  if (i < 0 || i >= (int) lamps().size()) return;
  Lamp &l = lamps()[i];
  if (l.lbl == nullptr) return;
  std::string w;
  if (l.state.empty()) w = "\xE2\x80\x94";                                   // —
  else if (l.state == "unavailable" || l.state == "unknown") w = tr("Niet bereikbaar");
  else if (!lamp_aan(l)) w = tr("Uit");
  else if (l.dim && lamp_pct(l) >= 0) w = std::to_string(std::max(1, lamp_pct(l))) + "%";
  else w = tr("Aan");
  const std::string t = l.name_s + "  \xC2\xB7  " + w;
  if (t != lv_label_get_text(l.lbl)) lv_label_set_text(l.lbl, t.c_str());
  lv_obj_set_style_text_color(l.lbl, k(lamp_aan(l) ? 0xF2F3F5 : 0xE4E7EC), 0);
  lv_obj_set_style_text_opa(l.lbl, lamp_aan(l) || l.state.empty() ? 255 : 170, 0);
}
inline void lamps_text() { for (int i = 0; i < (int) lamps().size(); i++) lamp_text(i); }

// "Naam=entity|Naam=entity" (substitution noir_lampen) -> lampen()
inline void lamps_from_text(const char *list_obj) {
  lamps().clear();
  std::string l = list_obj != nullptr ? list_obj : "";
  while (!l.empty()) {
    const size_t p = l.find('|');
    const std::string d = l.substr(0, p);
    const size_t is = d.find('=');
    if (is != std::string::npos && is + 1 < d.size()) {
      Lamp a;
      a.name_s = d.substr(0, is);
      a.entity_ref = d.substr(is + 1);
      a.dim = a.entity_ref.rfind("light.", 0) == 0;
      lamps().push_back(a);
    }
    if (p == std::string::npos) break;
    l = l.substr(p + 1);
  }
}
// Items van de configuratorpagina "noir_licht" (label + entity; dim: false = niet dimbaar) -> lampen()
inline bool lamps_from_items(JsonArrayConst items) {
  if (items.isNull() || items.size() == 0) return false;
  std::vector<Lamp> is_new;
  for (JsonObjectConst it : items) {
    const char *e = it["entity"] | "";
    if (!*e) continue;
    Lamp a;
    a.entity_ref = e;
    a.name_s = it["label"] | e;
    a.dim = a.entity_ref.rfind("light.", 0) == 0 && (it["dim"] | true);
    for (const Lamp &o : lamps()) if (o.entity_ref == a.entity_ref) { a.state = o.state; a.bri = o.bri; }
    is_new.push_back(a);
    if (is_new.size() >= 12) break;
  }
  if (is_new.empty()) return false;
  lamps() = is_new;
  return true;
}

// HTTP request in its own task: url_basis() = "http://host/api/noir_remote/states?k=..."
namespace lights_http {
inline std::function<std::string()> &url_basis() { static std::function<std::string()> f; return f; }
struct Request { volatile bool running{false}, finished{false}; std::string url; char *buf{nullptr}; size_t len{0}; int status{0};
                 void *worker{nullptr}; uint32_t next_at{0}; };
inline Request &a() { static Request x; return x; }
// NOIR Audio (2026-11-22) haakt hier in: extra entiteiten in dezelfde vraag, eigen uitlezing, eigen "open"
inline std::function<bool()> &extra_open() { static std::function<bool()> f; return f; }
inline std::function<std::string()> &extra_e() { static std::function<std::string()> f; return f; }
inline std::function<void(JsonDocument &)> &extra_read() { static std::function<void(JsonDocument &)> f; return f; }
#ifdef ESP_PLATFORM
inline void run_once_() {
  Request &q = a();
  esp_http_client_config_t cfg = {};
  cfg.url = q.url.c_str();
  cfg.timeout_ms = 3000;
  cfg.buffer_size = 512;
  cfg.buffer_size_tx = std::max<int>(512, (int) q.url.size() + 256);
  cfg.keep_alive_enable = false;
  esp_http_client_handle_t c = esp_http_client_init(&cfg);
  int status = -1;
  size_t len = 0;
  char *buf = nullptr;
  if (c != nullptr && esp_http_client_open(c, 0) == ESP_OK) {
    esp_http_client_fetch_headers(c);
    status = esp_http_client_get_status_code(c);
    if (status == 200) {
      const size_t MAX = 4096;
      buf = (char *) heap_caps_malloc(MAX, MALLOC_CAP_SPIRAM);
      while (buf != nullptr && len < MAX) {
        int r = esp_http_client_read(c, buf + len, MAX - len);
        if (r <= 0) break;
        len += r;
      }
    }
  }
  if (c != nullptr) { esp_http_client_close(c); esp_http_client_cleanup(c); }
  q.buf = buf; q.len = len; q.status = status; q.finished = true;
}
inline void worker_(void *) {
  UBaseType_t lowest = ~0u;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    run_once_();
    const UBaseType_t free_ = uxTaskGetStackHighWaterMark(nullptr);   // minst vrije stapel tot nu toe (bytes)
    if (free_ < lowest) { lowest = free_; ESP_LOGD("noir", "%s: stapel minimaal %u bytes vrij", pcTaskGetName(nullptr), (unsigned) free_); }
  }
}
inline void start_() {
  Request &q = a();
  if (q.worker == nullptr) {
    static StaticTask_t tcb;
    const size_t size_px = 8192;   // 2026-11-25: 4096 was te krap voor esp_http_client (DNS/lwIP) - overloop gaf panics
    auto *pile = (StackType_t *) heap_caps_malloc(size_px, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (pile != nullptr) q.worker = xTaskCreateStaticPinnedToCore(worker_, "noir_licht", size_px, nullptr, 1, pile, &tcb, tskNO_AFFINITY);
  }
  if (q.worker == nullptr) return;
  q.running = true;
  q.finished = false;
  xTaskNotifyGive((TaskHandle_t) q.worker);
}
#else
inline void start_() {}
#endif
// Antwort {"light.x": ["on", 128, -1, ""], ...} -> lampen()
inline void evaluate_() {
  Request &q = a();
  if (q.status == 200 && q.buf != nullptr && q.len > 0) {
    JsonDocument doc;
    if (!deserializeJson(doc, q.buf, q.len)) {
      for (Lamp &l : lamps()) {
        JsonArrayConst v = doc[l.entity_ref];
        if (v.isNull()) continue;
        l.state = v[0] | "";
        l.bri = v[1] | -1;
      }
      lamps_text();
      if (extra_read()) extra_read()(doc);
    }
  }
#ifdef ESP_PLATFORM
  if (q.buf != nullptr) heap_caps_free(q.buf);
#endif
  q.buf = nullptr;
  q.len = 0;
}
// Elke 100 ms vanuit de firmware; open = NOIR Verlichting is te zien (scherm aan, startpagina)
inline void poll_tick(uint32_t now_ms, bool open) {
  Request &q = a();
  if (q.finished) { q.finished = false; q.running = false; evaluate_(); }
  const bool extra = extra_open() && extra_open()();
  if ((!open && !extra) || q.running || (lamps().empty() && !extra) || !url_basis() || (int32_t) (now_ms - q.next_at) < 0) return;
  q.next_at = now_ms + (extra ? 2000 : 3000);
  const std::string base = url_basis()();
  if (base.empty()) return;
  std::string list_obj;
  if (open) for (const Lamp &l : lamps()) { list_obj += l.entity_ref; list_obj += ","; }
  if (extra && extra_e()) list_obj += extra_e()();
  q.url = base + "&e=" + list_obj;
  start_();
}
inline void soon(uint32_t now_ms, uint32_t ms) { a().next_at = now_ms + ms; }   // na een tik: stand kort daarna opnieuw
}  // namespace licht_http

inline void lamp_toggle(int i, uint32_t now_ms) {
  if (i < 0 || i >= (int) lamps().size() || !ha()) return;
  Lamp &l = lamps()[i];
  const std::string &e = l.entity_ref;
  const char *svc = e.rfind("light.", 0) == 0 ? "light.toggle" : e.rfind("switch.", 0) == 0 ? "switch.toggle" : "homeassistant.toggle";
  ha()(svc, {{"entity_id", e}});
  if (!l.state.empty()) { l.state = lamp_aan(l) ? "off" : "on"; lamp_text(i); }   // meteen tonen, HA bevestigt
  lights_http::soon(now_ms, 900);
}

// Dimvenster (simulator open_dimmer): over alles op lv_layer_top
struct Dimmer { lv_obj_t *over{nullptr}, *value_lbl{nullptr}; int lamp{-1}; };
inline Dimmer &dimmer() { static Dimmer d; return d; }
inline void dimmer_close() {
  Dimmer &d = dimmer();
  if (d.over != nullptr) { lv_obj_delete_async(d.over); d.over = nullptr; d.value_lbl = nullptr; }
  d.lamp = -1;
}
inline void dimmer_value_(lv_event_t *e) {
  lv_obj_t *sl = (lv_obj_t *) lv_event_get_target(e);
  const int v = lv_slider_get_value(sl);
  Dimmer &d = dimmer();
  if (d.value_lbl != nullptr) lv_label_set_text_fmt(d.value_lbl, "%d%%", v);
  if (lv_event_get_code(e) != LV_EVENT_RELEASED || d.lamp < 0 || d.lamp >= (int) lamps().size() || !ha()) return;
  Lamp &l = lamps()[d.lamp];
  if (v <= 0) { ha()("light.turn_off", {{"entity_id", l.entity_ref}}); l.state = "off"; }
  else { ha()("light.turn_on", {{"entity_id", l.entity_ref}, {"brightness_pct", std::to_string(v)}}); l.state = "on"; l.bri = (v * 255 + 50) / 100; }
  lamp_text(d.lamp);
  lights_http::soon(lv_tick_get(), 1500);
}
inline void dimmer_open(int i) {
  UiState &s = st();
  if (i < 0 || i >= (int) lamps().size() || !lamps()[i].dim || dimmer().over != nullptr) return;
  if (lv_indev_active() != nullptr) lv_indev_wait_release(lv_indev_active());   // loslaten = geen tik op de knop
  Lamp &l = lamps()[i];
  Dimmer &d = dimmer();
  d.lamp = i;
  d.over = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(d.over);
  lv_obj_add_flag(d.over, LV_OBJ_FLAG_USER_4);
  lv_obj_set_size(d.over, 240, 320);
  lv_obj_set_style_bg_color(d.over, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(d.over, 175, 0);
  lv_obj_add_flag(d.over, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(d.over, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t *m = lv_obj_create(d.over);
  lv_obj_set_pos(m, 16, 80);
  lv_obj_set_size(m, 208, 162);
  lv_obj_set_style_bg_color(m, k(SURF), 0);
  lv_obj_set_style_bg_opa(m, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(m, rnd(16), 0);
  lv_obj_set_style_border_width(m, 1, 0);
  lv_obj_set_style_border_color(m, k(BORDER), 0);
  lv_obj_set_style_border_opa(m, LV_OPA_60, 0);
  lv_obj_set_style_pad_all(m, 0, 0);
  lv_obj_set_style_shadow_width(m, 0, 0);
  lv_obj_remove_flag(m, LV_OBJ_FLAG_SCROLLABLE);
  label(m, l.name_s.c_str(), 16, 15, 176, s.fonts.f16, TEXT);
  const int pct = lamp_aan(l) ? std::max(1, lamp_pct(l) < 0 ? 100 : lamp_pct(l)) : 0;
  d.value_lbl = label(m, "", 16, 45, 176, s.fonts.f20 != nullptr ? s.fonts.f20 : s.fonts.f16, TEXT);
  lv_label_set_text_fmt(d.value_lbl, "%d%%", pct);
  lv_obj_t *sl = lv_slider_create(m);
  lv_obj_set_pos(sl, 18, 88);
  lv_obj_set_size(sl, 172, 5);
  lv_slider_set_range(sl, 0, 100);
  lv_slider_set_value(sl, pct, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(sl, k(0x36393F), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(sl, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(sl, k(0xBFC2C9), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(sl, k(0xF2F3F5), LV_PART_KNOB);
  lv_obj_set_style_pad_all(sl, 6, LV_PART_KNOB);
  lv_obj_set_style_outline_width(sl, 0, LV_STATE_FOCUSED);
  lv_obj_set_ext_click_area(sl, 14);
  lv_obj_add_event_cb(sl, dimmer_value_, LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_add_event_cb(sl, dimmer_value_, LV_EVENT_RELEASED, nullptr);
  lv_obj_t *g = btn(m, 12, 116, 184, 32, DIM_DONE);
  lv_obj_t *gl = label(g, "Gereed", 0, 0, 174, s.fonts.f12, 0xE4E7EC);
  lv_obj_set_style_text_align(gl, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(gl);
}
inline void lang_(lv_event_t *e) { dimmer_open((int) (intptr_t) lv_event_get_user_data(e) - LAMP); }

inline void lights_build(lv_obj_t *layer_obj, const Fonts &f) {
  UiState &s = st();
  lv_obj_t *body = s.lights_body = body_make(layer_obj, true);
  label(body, "Verlichting", 0, 0, 216, f.f20 != nullptr ? f.f20 : f.f16, TEXT);
  if (lamps().empty()) {
    lv_obj_t *l = label(body, "Nog geen lampen. Configurator \xE2\x86\x92 NOIR \xE2\x86\x92 Verlichting.", 0, 42, 216, f.f12, MUTED);
    lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_WRAP);
  }
  for (int i = 0; i < (int) lamps().size(); i++) {
    Lamp &l = lamps()[i];
    lv_obj_t *b = btn(body, 0, 42 + i * 55, 216, 47, LAMP + i, LV_EVENT_SHORT_CLICKED);
    l.lbl = label(b, "", 0, 0, 200, f.f12, 0xE4E7EC);
    lv_obj_set_style_text_align(l.lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(l.lbl);
    if (l.dim) {   // dimbaar: lichte rand, lang drukken = dimmer
      lv_obj_add_event_cb(b, lang_, LV_EVENT_LONG_PRESSED, (void *) (intptr_t) (LAMP + i));
      lv_obj_set_style_border_color(b, k(0xBFC2C9), 0);
      lv_obj_set_style_border_opa(b, 85, 0);
    }
  }
  if (!lamps().empty()) {
    lv_obj_t *h = label(body, "Lang drukken = dimmen", 0, 42 + (int) lamps().size() * 55, 216, f.f12, MUTED);
    lv_obj_set_style_text_align(h, LV_TEXT_ALIGN_CENTER, 0);
  }
  lamps_text();
}

// ---- NOIR Airco (2026-11-22): Mitsubishi Heavy Industries via infrarood ----------------------------------------
// Afstandsbediening RLA502A704A (SRK..ZS). De airco krijgt bij elke wijziging de HELE stand in een frame (152 bits):
// kop (5 bytes) | modus+aan | temperatuur | fan | 3D-auto | lamellen | staart, elke databyte als (omgekeerd, echt).
// Bits staan hieronder in zendvolgorde (zoals de decodering van deze afstandsbediening door de community:
// github.com/peterhinch/micropython_ir issue 46). Timing: kop 3200/1600 us, bit = 400 us aan + 1200 (1) of 430 (0) uit.
// De remote kan de stand van de airco niet uitlezen - hij onthoudt zelf wat hij het laatst stuurde (global).
namespace airco {
// NOIR Airco - brand-free. Up to 4 units (rooms). Each unit is driven by its module (modules/airco.h):
//   a climate entity in Home Assistant (any brand: ESPHome module, Daikin, SmartIR, ...) - the screen shows the modes,
//   fan and swing settings that entity has; or the remote's own infrared with a brand profile (now: Mitsubishi Heavy).
// The screen only reads and changes st(); the module sends it (debounced) and fills in what the unit can do.
static const int MODES = 5;   // 0 auto, 1 cool, 2 dry, 3 fan, 4 heat
static const char *MODE_NAME[MODES] = {"Automatisch", "Koelen", "Ontvochtigen", "Ventileren", "Verwarmen"};
static const char *MODE_ICON[MODES] = {"\U000F0BED", "\U000F0717", "\U000F058C", "\U000F0210", "\U000F05A8"};   // alpha-a-circle-outline, snowflake, water, fan, white-balance-sunny
static const int MAX_UNITS = 4;
struct Unit {
  std::string name_s;                 // "Woonkamer"
  bool is_on{false};
  int mode{1}, temp{22};
  int fan{0}, swing{0};             // index in fans / swings
  int room{-1000};                 // room temperature (unknown = -1000)
  bool mode_list[MODES]{true, true, true, true, true};   // what the unit can do
  std::vector<std::string> fans{"Auto"}, swings{"Uit", "Aan"};
  int tmin{16}, tmax{30};
  bool known{true};                // state known (HA: after the first poll)
};
struct Data { std::vector<Unit> units{Unit{}}; int cur{0}; };
inline Data &d() { static Data x; return x; }
inline Unit &u() { Data &x = d(); if (x.units.empty()) x.units.push_back(Unit{}); x.cur = std::max(0, std::min(x.cur, (int) x.units.size() - 1)); return x.units[x.cur]; }
// The module sends unit i (after a change, debounced). Simulator: prints.
inline std::function<void(int)> &on_change() { static std::function<void(int)> f; return f; }
inline std::function<void(const std::string &)> &sender() { static std::function<void(const std::string &)> z; return z; }   // simulator

inline lv_timer_t *&timer_() { static lv_timer_t *t = nullptr; return t; }
inline void send_now() { if (on_change()) on_change()(d().cur); }
inline void schedule_send(uint32_t ms = 600) {   // wait a moment: holding + does not send every step
  if (timer_() != nullptr) { lv_timer_reset(timer_()); return; }
  timer_() = lv_timer_create([](lv_timer_t *) { timer_() = nullptr; send_now(); }, ms, nullptr);
  lv_timer_set_repeat_count(timer_(), 1);
}

struct Ui {
  lv_obj_t *title{nullptr}, *status{nullptr}, *is_on{nullptr}, *on_i{nullptr}, *swap{nullptr}, *arc_obj{nullptr}, *temp{nullptr}, *deg{nullptr};
  lv_obj_t *min{nullptr}, *plus{nullptr}, *fan{nullptr}, *swing{nullptr};
  lv_obj_t *mode[MODES]{};
};
inline Ui &ui() { static Ui x; return x; }
static const uint32_t ICE = 0x9FD8F0;   // icy blue: arc and the active mode

inline void tile_show() {   // the airco tile on Home
  AircoTile &t = airco_tile();
  if (t.arc_obj == nullptr) return;
  Unit &n = u();
  const int span = std::max(1, n.tmax - n.tmin);
  lv_arc_set_value(t.arc_obj, std::max(0, std::min(100, (n.temp - n.tmin) * 100 / span)));
  lv_obj_set_style_arc_opa(t.arc_obj, n.is_on ? 255 : 90, LV_PART_INDICATOR);
  lv_label_set_text(t.temp, std::to_string(n.temp).c_str());
  lv_obj_set_style_text_opa(t.temp, n.is_on ? 255 : 120, 0);
  lv_label_set_text(t.mode_lbl, !n.known ? tr("Niet bereikbaar") : n.is_on ? tr(MODE_NAME[n.mode]) : tr("Uit"));
}
inline void show() {
  tile_show();
  Ui &x = ui();
  if (x.temp == nullptr) return;
  Unit &n = u();
  const bool more = d().units.size() > 1;
  lv_label_set_text(x.title, more && !n.name_s.empty() ? n.name_s.c_str() : "Airco");
  if (x.swap != nullptr) { if (more) lv_obj_remove_flag(x.swap, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(x.swap, LV_OBJ_FLAG_HIDDEN); }
  char t[64];
  lv_label_set_text(x.temp, std::to_string(n.temp).c_str());
  lv_obj_set_style_text_opa(x.temp, n.is_on ? 255 : 110, 0);
  lv_obj_set_style_text_opa(x.deg, n.is_on ? 220 : 90, 0);
  std::string st = !n.known ? std::string(tr("Niet bereikbaar")) : n.is_on ? std::string(tr(MODE_NAME[n.mode])) : std::string(tr("Uit"));
  if (n.room > -1000) { snprintf(t, sizeof(t), " \xC2\xB7 %s %d\xC2\xB0", tr("kamer"), n.room); st += t; }
  lv_label_set_text(x.status, st.c_str());
  // power: light pill when on
  lv_obj_set_style_bg_color(x.is_on, k(n.is_on ? 0xE8EEF3 : SURF), 0);
  lv_obj_set_style_bg_grad_color(x.is_on, k(n.is_on ? 0xD9E2EA : SURF2), 0);
  lv_obj_set_style_text_color(x.on_i, k(n.is_on ? BG : TEXT2), 0);
  // arc: where the set temperature is between min and max
  const int span = std::max(1, n.tmax - n.tmin);
  lv_arc_set_value(x.arc_obj, std::max(0, std::min(100, (n.temp - n.tmin) * 100 / span)));
  lv_obj_set_style_arc_opa(x.arc_obj, n.is_on ? 255 : 90, LV_PART_INDICATOR);
  for (int i = 0; i < MODES; i++) {
    lv_obj_t *b = x.mode[i];
    if (b == nullptr) continue;
    const bool sel = n.is_on && i == n.mode;
    if (n.mode_list[i]) { lv_obj_remove_flag(b, LV_OBJ_FLAG_HIDDEN); } else { lv_obj_add_flag(b, LV_OBJ_FLAG_HIDDEN); continue; }
    lv_obj_set_style_bg_color(b, k(sel ? ICE : SURF), 0);
    lv_obj_set_style_bg_grad_color(b, k(sel ? 0x86C9E6 : SURF2), 0);
    lv_obj_set_style_border_opa(b, sel ? 0 : 115, 0);
    lv_obj_t *l = lv_obj_get_child(b, -1);
    if (l != nullptr) lv_obj_set_style_text_color(l, k(sel ? BG : TEXT2), 0);
  }
  if (!n.fans.empty()) lv_label_set_text(lv_obj_get_child(x.fan, -1), n.fans[std::min<int>(n.fan, n.fans.size() - 1)].c_str());
  if (n.fans.size() > 1) lv_obj_remove_flag(x.fan, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(x.fan, LV_OBJ_FLAG_HIDDEN);
  if (!n.swings.empty()) {
    snprintf(t, sizeof(t), "Swing %s", n.swings[std::min<int>(n.swing, n.swings.size() - 1)].c_str());
    lv_label_set_text(lv_obj_get_child(x.swing, -1), n.swing == 0 ? "Swing" : t);
    lv_obj_set_style_border_color(x.swing, k(n.swing ? 0xBFC2C9 : BORDER), 0);
  }
  if (n.swings.size() > 1) lv_obj_remove_flag(x.swing, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(x.swing, LV_OBJ_FLAG_HIDDEN);
}

// ---- actions (AIRCO_* in Actie) ----
inline void temp_stap(int dt) { Unit &n = u(); n.temp = std::max(n.tmin, std::min(n.tmax, n.temp + dt)); if (!n.is_on) n.is_on = true; show(); schedule_send(); }
inline void toggle_power() { u().is_on = !u().is_on; show(); schedule_send(150); }
inline void uit() { u().is_on = false; show(); schedule_send(150); }   // always send off (state may have changed elsewhere)
inline void mode(int m) { if (m < 0 || m >= MODES || !u().mode_list[m]) return; u().mode = m; u().is_on = true; show(); schedule_send(150); }
inline void fan_next() { Unit &n = u(); if (n.fans.size() < 2) return; n.fan = (n.fan + 1) % n.fans.size(); show(); schedule_send(); }
inline void swing_next() { Unit &n = u(); if (n.swings.size() < 2) return; n.swing = (n.swing + 1) % n.swings.size(); show(); schedule_send(); }
inline void fan_special(int) {}   // before 0.4 (HI/ECO buttons); now part of the fan list of the profile
inline void unit_next() { Data &x = d(); if (x.units.size() < 2) return; x.cur = (x.cur + 1) % x.units.size(); show(); }

// ---- Mitsubishi Heavy (SRK..ZS, remote RLA502A704A) - infrared profile "mhi" -----------------------------------
// One frame carries the whole state. Fan list of this profile: Auto, 1, 2, 3, 4, HI, Eco.
namespace mhi {
struct Install {
  int hi{9}, eco{6};          // fan code of HI POWER and ECONO
  int hor{3}, ver{6};         // louvres without swing (horizontal 0-8, vertical 0-12)
  int hor_sw{0}, ver_sw{0};   // louvres with swing
  int start{3200};            // start pulse in us
};
inline Install &install() { static Install i; return i; }
inline const std::vector<std::string> &fans() { static std::vector<std::string> v{"Auto", "1", "2", "3", "4", "HI", "Eco"}; return v; }
inline const std::vector<std::string> &swings() { static std::vector<std::string> v{"Uit", "Op/neer", "Links/rechts", "Alles"}; return v; }
inline std::string lsb_(int v, int n) { std::string o; for (int i = 0; i < n; i++) o += (v >> i) & 1 ? '1' : '0'; return o; }
inline std::string pair_(const std::string &b) { std::string o; for (char c : b) o += c == '1' ? '0' : '1'; return o + b; }
// mode index (auto, cool, dry, fan, heat) is the protocol's own order
inline std::string bits(const Unit &s) {
  std::string b = "0100101001110101110000110101100010100111";   // header
  b += pair_(lsb_(s.mode, 3) + (s.is_on ? "1" : "0") + "0000");
  b += pair_(lsb_(std::max(17, std::min(31, s.temp)) - 17, 4) + "0000");   // 17 + 0..15
  b += pair_(lsb_(s.fan <= 4 ? s.fan : s.fan == 5 ? install().hi : install().eco, 4) + "0000");
  const int hor = (s.swing & 2) ? install().hor_sw : install().hor, ver = (s.swing & 1) ? install().ver_sw : install().ver;
  const std::string panel = lsb_(hor, 4) + lsb_(ver, 4);
  b += pair_("00000" + panel.substr(5));   // 3D auto off
  b += pair_(panel);
  b += "11111111000000001111111000000001";   // tail
  return b;
}
inline std::vector<int32_t> timings(const Unit &s) {
  std::vector<int32_t> t{install().start, -1600};
  for (char c : bits(s)) { t.push_back(400); t.push_back(c == '1' ? -1200 : -430); }
  t.push_back(400);
  return t;
}
// state <-> int (flash: infrared gets no feedback, so the remote remembers what it sent)
inline int pack(const Unit &s) { return (s.is_on ? 1 : 0) | (s.mode & 7) << 1 | ((s.temp - 16) & 31) << 4 | (s.fan & 7) << 9 | (s.swing & 3) << 12 | 1 << 15; }
inline void unpack(Unit &s, int v) {
  if (!(v & (1 << 15))) return;
  s.is_on = v & 1; s.mode = std::min(4, (v >> 1) & 7); s.temp = std::max(s.tmin, std::min(s.tmax, ((v >> 4) & 31) + 16));
  s.fan = std::min(6, (v >> 9) & 7); s.swing = (v >> 12) & 3;
}
}  // namespace mhi

}  // namespace airco

static const char *ICO_MIN = "\U000F0374", *ICO_PLUS = "\U000F0415";   // mdi:minus, mdi:plus

inline lv_obj_t *icon_button_(lv_obj_t *p, int x, int y, int w, int h, int actie, const char *ico, const Fonts &f) {
  lv_obj_t *b = btn(p, x, y, w, h, actie);
  if (f.ico != nullptr) {
    lv_obj_t *i = lv_label_create(b);
    lv_label_set_text(i, ico);
    lv_obj_set_style_text_font(i, f.ico, 0);
    lv_obj_set_style_text_color(i, k(TEXT2), 0);
    lv_obj_center(i);
  }
  return b;
}
inline lv_obj_t *text_button_(lv_obj_t *p, int x, int y, int w, int h, int actie, const char *t, const Fonts &f) {
  lv_obj_t *b = btn(p, x, y, w, h, actie);
  lv_obj_t *l = label(b, t, 0, 0, w - 2, f.f12, 0xE4E7EC);
  lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_CLIP);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(l);
  return b;
}

// NOIR Airco, design B "Boog": title + power, status, a dial arc around the temperature with - and +,
// the modes as icons, fan and swing. More units: the swap button next to power picks the next room.
static const char *ICO_SWAP = "\U000F04E1", *ICO_SWING = "\U000F1C91", *ICO_FAN = "\U000F0210";   // swap-horizontal, arrow-oscillating, fan
inline lv_obj_t *pill_(lv_obj_t *p, int x, int y, int w, int h, int actie, const char *ico, const Fonts &f) {
  lv_obj_t *b = btn(p, x, y, w, h, actie);
  lv_obj_set_style_radius(b, h / 2, 0);
  if (f.ico != nullptr) {
    lv_obj_t *i = lv_label_create(b);
    lv_label_set_text(i, ico);
    lv_obj_set_style_text_font(i, f.ico, 0);
    lv_obj_set_style_text_color(i, k(TEXT2), 0);
    lv_obj_align(i, LV_ALIGN_LEFT_MID, 14, 0);
  }
  lv_obj_t *l = label(b, "", 0, 0, w - 46, f.f12, TEXT2);
  lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_CLIP);
  lv_obj_align(l, LV_ALIGN_LEFT_MID, 40, 0);
  return b;
}
inline void airco_build(lv_obj_t *layer_obj, const Fonts &f) {
  airco::Ui &u = airco::ui();
  lv_obj_t *body = body_make(layer_obj, false);
  u.title = label(body, "Airco", 0, 0, 120, f.f20 != nullptr ? f.f20 : f.f16, TEXT);
  u.status = label(body, "", 0, 26, 150, f.f12, MUTED);
  u.swap = icon_button_(body, 120, 0, 38, 34, AIRCO_UNIT, ICO_SWAP, f);
  u.is_on = icon_button_(body, 164, 0, 52, 34, AIRCO_POWER, ICO_POWER, f);
  lv_obj_set_style_radius(u.is_on, 17, 0);
  u.on_i = lv_obj_get_child(u.is_on, -1);
  // dial arc (270 degrees) with the temperature in the middle
  u.arc_obj = lv_arc_create(body);
  lv_obj_set_size(u.arc_obj, 104, 104);   // everything fits the 230 px body: arc 40-144, modes 152-184, pills 194-224
  lv_obj_set_pos(u.arc_obj, 56, 40);
  lv_arc_set_bg_angles(u.arc_obj, 135, 45);
  lv_arc_set_range(u.arc_obj, 0, 100);
  lv_obj_remove_style(u.arc_obj, nullptr, LV_PART_KNOB);
  lv_obj_remove_flag(u.arc_obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(u.arc_obj, 4, LV_PART_MAIN);
  lv_obj_set_style_arc_color(u.arc_obj, k(BORDER), LV_PART_MAIN);
  lv_obj_set_style_arc_width(u.arc_obj, 5, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(u.arc_obj, k(airco::ICE), LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(u.arc_obj, true, LV_PART_INDICATOR);
  const lv_font_t *large = f.large != nullptr ? f.large : (f.f20 != nullptr ? f.f20 : f.f16);
  u.temp = label(body, "", 58, 40 + (104 - lv_font_get_line_height(large)) / 2, 100, large, TEXT);
  lv_obj_set_style_text_align(u.temp, LV_TEXT_ALIGN_CENTER, 0);
  u.deg = label(body, "\xC2\xB0", 136, 58, 16, f.f20 != nullptr ? f.f20 : f.f16, TEXT);
  u.min = icon_button_(body, 0, 70, 44, 44, AIRCO_MIN, ICO_MIN, f);
  u.plus = icon_button_(body, 172, 70, 44, 44, AIRCO_PLUS, ICO_PLUS, f);
  lv_obj_set_style_radius(u.min, 22, 0);
  lv_obj_set_style_radius(u.plus, 22, 0);
  // modes as icons
  const int bw = (216 - 4 * 6) / 5;
  for (int i = 0; i < airco::MODES; i++) {
    u.mode[i] = icon_button_(body, i * (bw + 6), 152, bw, 32, AIRCO_MODE + i, airco::MODE_ICON[i], f);
    lv_obj_set_style_radius(u.mode[i], 12, 0);
  }
  u.fan = pill_(body, 0, 194, 104, 30, AIRCO_FAN, ICO_FAN, f);
  u.swing = pill_(body, 112, 194, 104, 30, AIRCO_SWING, ICO_SWING, f);
  airco::show();
}

// ---- NOIR Knoppentest (2026-11-22): D-pad en OK testen zonder tv/HA ---------------------------------------------
// Zelfde weg als bij normaal gebruik (on_key -> driver met entprellen -> hier), maar er gaat niets naar buiten.
// Raster 3 x 3: pijlen verplaatsen de cursor, OK telt op de tegel. "Verdacht" = dezelfde knop opnieuw ingedrukt
// binnen 120 ms na loslaten (zo snel drukt niemand - dat is dender of een dubbel gelezen druk).
namespace keytest {
struct UiState {
  int cur{4};
  int count_[6]{};                 // ^ v < > OK anders
  int per[128]{};               // presses per key code (to see which key ghosts)
  int ok_op[9]{};
  int suspect{0};
  uint32_t release_ms[128]{}, press_ms[128]{};
  std::string latest{"Druk op een pijl of OK"};
  lv_obj_t *tile[9]{}, *info{nullptr}, *counter{nullptr}, *susp_lbl{nullptr}, *large{nullptr};
  lv_timer_t *flash_obj{nullptr};
  int flash_tile{-1};
};
inline UiState &st_() { static UiState s; return s; }
inline const char *name_lbl(int t) {   // names as used by NOIR (const.py KEYCODES, modules/*.h)
  switch (t) { case 14: return "Op"; case 35: return "Neer"; case 15: return "Links"; case 32: return "Rechts";
               case 34: return "OK"; case 13: return "Terug"; case 3: return "Menu"; case 31: return "Info";
               case 41: return "Home"; case 2: return "Play"; case 5: return "Stop"; case 4: return "Terugspoelen";
               case 11: return "Vooruitspoelen"; case 42: return "Kanaal +"; case 22: return "Kanaal -";
               case 23: return "Rood"; case 25: return "Groen"; case 24: return "Geel"; case 21: return "Blauw";
               case 33: return "Volume +"; case 43: return "Volume -"; case 44: return "Mute"; case 102: return "Aan/uit";
               default: return nullptr; }
}
inline void show() {
  UiState &s = st_();
  if (s.info == nullptr) return;
  for (int i = 0; i < 9; i++) {
    if (s.tile[i] == nullptr) continue;
    const bool sel = i == s.cur;
    lv_obj_set_style_border_color(s.tile[i], k(sel ? 0xBFC2C9 : BORDER), 0);
    lv_obj_set_style_border_opa(s.tile[i], sel ? 230 : 90, 0);
    lv_obj_set_style_border_width(s.tile[i], sel ? 2 : 1, 0);
    lv_label_set_text_fmt(lv_obj_get_child(s.tile[i], -1), "%d", s.ok_op[i]);
  }
  lv_label_set_text(s.info, s.latest.c_str());
  // "overige": the four keys pressed most outside the arrows and OK, e.g. "41x18 31x8"
  std::string top;
  bool done[128]{};
  for (int n = 0; n < 4; n++) {
    int best = 0;
    for (int t = 1; t < 128; t++)
      if (!done[t] && t != 14 && t != 35 && t != 15 && t != 32 && t != 34 && s.per[t] > s.per[best]) best = t;
    if (best == 0 || s.per[best] == 0) break;
    done[best] = true;
    top += (top.empty() ? "" : "  ") + std::to_string(best) + "x" + std::to_string(s.per[best]);
  }
  lv_label_set_text_fmt(s.counter, "Op %d   Neer %d   Links %d   Rechts %d\nOK %d   overige %d\n%s",
                        s.count_[0], s.count_[1], s.count_[2], s.count_[3], s.count_[4], s.count_[5], top.c_str());
  lv_label_set_text_fmt(s.susp_lbl, "Verdacht (dubbel < 120 ms): %d", s.suspect);
  lv_obj_set_style_text_color(s.susp_lbl, k(s.suspect ? 0xF2F3F5 : 0x969AA3), 0);
}
// OK: tegel licht 180 ms op
inline void flash_obj(int i) {
  UiState &s = st_();
  if (i < 0 || i > 8 || s.tile[i] == nullptr) return;
  if (s.flash_obj != nullptr) { lv_timer_delete(s.flash_obj); s.flash_obj = nullptr; if (s.flash_tile >= 0 && s.tile[s.flash_tile]) lv_obj_remove_state(s.tile[s.flash_tile], LV_STATE_PRESSED); }
  s.flash_tile = i;
  lv_obj_add_state(s.tile[i], LV_STATE_PRESSED);   // zelfde look als aantikken
  s.flash_obj = lv_timer_create([](lv_timer_t *) {
    UiState &z = st_();
    if (z.flash_tile >= 0 && z.tile[z.flash_tile] != nullptr) lv_obj_remove_state(z.tile[z.flash_tile], LV_STATE_PRESSED);
    z.flash_obj = nullptr;
    z.flash_tile = -1;
  }, 180, nullptr);
  lv_timer_set_repeat_count(s.flash_obj, 1);
}
inline void reset() {
  UiState &s = st_();
  for (int &v : s.count_) v = 0;
  for (int &v : s.ok_op) v = 0;
  for (int &v : s.per) v = 0;
  s.suspect = 0;
  s.cur = 4;
  s.latest = "Teller op nul";
  show();
}
// Uit on_key (via tv::taste). true = afgehandeld; de menutoets (31) blijft vrij om eruit te komen.
inline bool key_in(int t, bool is_down) {
  if (t < 1 || t > 127) return false;
  UiState &s = st_();
  const uint32_t now_ms = lv_tick_get();
  if (!is_down) {
    s.release_ms[t] = now_ms;
    const char *n = name_lbl(t);
    char b[64];
    snprintf(b, sizeof(b), "%s%s losgelaten na %u ms", n ? n : "Toets ", n ? "" : std::to_string(t).c_str(), (unsigned) (now_ms - s.press_ms[t]));
    s.latest = b;
    show();
    return true;
  }
  const bool double_hit = s.release_ms[t] != 0 && now_ms - s.release_ms[t] < 120;
  if (double_hit) s.suspect++;
  s.press_ms[t] = now_ms;
  int r = s.cur / 3, c = s.cur % 3;
  switch (t) {
    case 14: s.count_[0]++; r = (r + 2) % 3; break;
    case 35: s.count_[1]++; r = (r + 1) % 3; break;
    case 15: s.count_[2]++; c = (c + 2) % 3; break;
    case 32: s.count_[3]++; c = (c + 1) % 3; break;
    case 34: s.count_[4]++; s.ok_op[s.cur]++; flash_obj(s.cur); break;
    default: s.count_[5]++; s.per[t]++; break;
  }
  s.cur = r * 3 + c;
  char b[64];
  const char *n = name_lbl(t);
  if (n) snprintf(b, sizeof(b), "%s (%d) ingedrukt%s", n, t, double_hit ? "  \xE2\x80\x94 VERDACHT" : "");
  else snprintf(b, sizeof(b), "Toets %d ingedrukt%s", t, double_hit ? "  \xE2\x80\x94 VERDACHT" : "");
  s.latest = b;
  if (s.large != nullptr) {   // laatste knop groot in beeld (rood-wit bij verdacht)
    lv_label_set_text(s.large, n ? n : std::to_string(t).c_str());
    lv_obj_set_style_text_opa(s.large, 255, 0);
  }
  show();
  return true;
}
}  // namespace knoptest

inline void test_build(lv_obj_t *layer_obj, const Fonts &f) {
  keytest::UiState &s = keytest::st_();
  lv_obj_t *body = body_make(layer_obj, false);
  label(body, "Knoppentest", 0, 0, 150, f.f20 != nullptr ? f.f20 : f.f16, TEXT);
  text_button_(body, 150, 0, 66, 30, TEST_RESET, "Reset", f);
  // raster links (3 x 3, 44 px), laatste knop groot rechts
  for (int i = 0; i < 9; i++) {
    lv_obj_t *b = btn(body, (i % 3) * 46, 36 + (i / 3) * 40, 42, 34, -1);
    lv_obj_remove_flag(b, LV_OBJ_FLAG_CLICKABLE);   // alleen met de knoppen
    lv_obj_t *l = label(b, "0", 0, 0, 38, f.f14, 0xE4E7EC);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(l);
    s.tile[i] = b;
  }
  s.large = label(body, "", 140, 70, 76, f.f20 != nullptr ? f.f20 : f.f16, TEXT);
  lv_obj_set_style_text_align(s.large, LV_TEXT_ALIGN_CENTER, 0);
  label(body, "laatste", 140, 100, 76, f.f12, MUTED);
  lv_obj_set_style_text_align(lv_obj_get_child(body, -1), LV_TEXT_ALIGN_CENTER, 0);
  s.info = label(body, "", 0, 156, 216, f.f12, TEXT);
  s.counter = label(body, "", 0, 174, 216, f.f12, MUTED);
  lv_label_set_long_mode(s.counter, LV_LABEL_LONG_MODE_WRAP);
  s.susp_lbl = label(body, "", 0, 228, 216, f.f12, MUTED);
  keytest::show();
}

// ---- NOIR Scènes (2026-11-22) ---------------------------------------------------------------------------------
// Tegels = items van de configuratorpagina "noir_scenes" (naam, icoon, stappen: HA, IR, wachten ...) - zonder
// compileren aan te passen. Vooraan altijd de ingebouwde scène "Alles uit" (lampen, tv, KEF en airco uit).
static const char *ICO_OFF = "\U000F0425";   // mdi:power
inline void scenes_build(lv_obj_t *layer_obj, const Fonts &f, const ImageSet &bl, JsonArrayConst items) {
  UiState &s = st();
  lv_obj_t *body = s.scenes_body = body_make(layer_obj, true);
  label(body, "Sc\xC3\xA8nes", 0, 0, 216, f.f20 != nullptr ? f.f20 : f.f16, TEXT);
  int n = 0;
  auto slot = [&](int i, int &x, int &y) { x = (i % 2) * 112; y = 36 + (i / 2) * 64; };
  int x, y;
  slot(n++, x, y);
  tile(body, "Alles uit", x, y, SCENE_ALL_OFF, nullptr, f, 104, 56, ICO_OFF);
  int i = 0;
  for (JsonObjectConst it : items) {
    if (i >= 30) break;
    const char *name_s = it["label"] | "";
    if (!*name_s && (it["t"] | std::string()) == "text") { i++; continue; }
    const char *icon = it["icon"] | "";
    const void *img_src = noir_beeld(icon, bl);
    const std::string glyph = img_src == nullptr ? mdi_teken(icon) : std::string();
    slot(n++, x, y);
    tile(body, name_s, x, y, SCENE + i, img_src, f, 104, 56, glyph.empty() ? nullptr : glyph.c_str());
    i++;
  }
  if (items.isNull() || items.size() == 0) {
    slot(n, x, y);
    lv_obj_t *h = label(body, "Eigen sc\xC3\xA8nes: configurator \xE2\x86\x92 NOIR \xE2\x86\x92 Sc\xC3\xA8nes.", 0, y + 8, 216, f.f12, MUTED);
    lv_label_set_long_mode(h, LV_LABEL_LONG_MODE_WRAP);
  }
}

// ---- NOIR Audio: one speaker in Home Assistant (on/off, sources, volume, subwoofer, now playing) -------------------
// The screen reads audio::d(); module Audio (modules/audio.h) fills it from /states and sends the actions via ha().
namespace audio {
static const int MAX_SOURCES = 6;   // at most 6 source buttons on the Audio screen
// Sources of the speaker: names on the buttons + source values of its media_player (NOIR Builder > Audio).
// Default: the KEF LSX II (simulator).
inline std::vector<std::string> &source_name() { static std::vector<std::string> v{"Wi-Fi", "Bluetooth", "TV", "Optical", "Aux", "USB"}; return v; }
inline std::vector<std::string> &source_id() { static std::vector<std::string> v{"wifi", "bluetooth", "tv", "optical", "analog", "usb"}; return v; }
inline int src_list() { return std::min<int>(MAX_SOURCES, std::min(source_name().size(), source_id().size())); }
inline std::string &speaker_name() { static std::string n{"KEF LSX II"}; return n; }
struct Data {
  std::string speaker_e, muziek, sub_e;    // entities: speaker, player for "now playing", subwoofer level (number.*)
  std::string speaker_state;              // on/off/unavailable
  int src{-1};                       // index in bron_id(), -1 = unknown
  int volume{-1};                     // 0-100
  int sub{0};                         // subwoofer in dB (-10..+10)
  bool sub_known{false};
  std::string sp_state, title, artist;
};
inline Data &d() { static Data x; return x; }
struct Ui {
  lv_obj_t *status{nullptr}, *is_on{nullptr}, *np_title{nullptr}, *np_sub{nullptr}, *np_source{nullptr};
  lv_obj_t *src[MAX_SOURCES]{};
  lv_obj_t *vol_sld{nullptr}, *vol_w{nullptr}, *sub_sld{nullptr}, *sub_w{nullptr};
};
inline Ui &ui() { static Ui u; return u; }
inline std::string sub_text_(int db) { char b[12]; snprintf(b, sizeof(b), db > 0 ? "+%d dB" : "%d dB", db); return b; }
inline bool playing() { return d().sp_state == "playing" || d().sp_state == "paused"; }

// Everything visible from d() (after every poll)
inline void show() {
  Ui &u = ui();
  Data &x = d();
  const bool speaker_on = x.speaker_state == "on" || x.speaker_state == "playing" || x.speaker_state == "paused" || x.speaker_state == "idle";
  if (u.status != nullptr) {
    std::string t = speaker_name() + " \xC2\xB7 ";
    if (x.speaker_state.empty() || x.speaker_state == "unavailable" || x.speaker_state == "unknown") t += tr("niet bereikbaar");
    else if (!speaker_on) t += tr("uit");
    else t += x.src >= 0 && x.src < src_list() ? source_name()[x.src] : tr("aan");
    lv_label_set_text(u.status, t.c_str());
    lv_obj_set_style_border_color(u.is_on, k(speaker_on ? 0xBFC2C9 : BORDER), 0);
    lv_obj_set_style_border_opa(u.is_on, speaker_on ? 220 : 115, 0);
  }
  for (int i = 0; i < src_list(); i++) {
    if (u.src[i] == nullptr) continue;
    const bool sel = speaker_on && i == x.src;
    lv_obj_set_style_border_color(u.src[i], k(sel ? 0xBFC2C9 : BORDER), 0);
    lv_obj_set_style_border_opa(u.src[i], sel ? 230 : 115, 0);
    lv_obj_t *l = lv_obj_get_child(u.src[i], -1);
    if (l != nullptr) lv_obj_set_style_text_opa(l, sel ? 255 : 150, 0);
  }
  if (u.vol_sld != nullptr && !lv_obj_has_state(u.vol_sld, LV_STATE_PRESSED)) {
    lv_slider_set_value(u.vol_sld, std::max(0, x.volume), LV_ANIM_OFF);
    lv_label_set_text(u.vol_w, x.volume < 0 ? "\xE2\x80\x94" : std::to_string(x.volume).c_str());
  }
  if (u.sub_sld != nullptr && !lv_obj_has_state(u.sub_sld, LV_STATE_PRESSED)) {
    lv_slider_set_value(u.sub_sld, x.sub, LV_ANIM_OFF);
    lv_label_set_text(u.sub_w, x.sub_known ? sub_text_(x.sub).c_str() : "\xE2\x80\x94");
  }
  if (u.np_title != nullptr) {
    lv_label_set_text(u.np_title, playing() ? x.title.c_str() : tr("Niets aan het afspelen"));
    lv_label_set_text(u.np_sub, playing() ? x.artist.c_str() : "");
  }
}
// /states rows of speaker, subwoofer and player: [state, bri, volume 0-100, unit, title, artist, ..., source]
inline std::string entities_() {
  std::string e;
  for (const std::string *x : {&d().speaker_e, &d().sub_e, &d().muziek}) if (!x->empty()) { e += *x; e += ","; }
  return e;
}
inline void read_(JsonDocument &doc) {
  Data &x = d();
  if (!x.speaker_e.empty()) {
    JsonArrayConst v = doc[x.speaker_e];
    if (!v.isNull()) {
      x.speaker_state = v[0] | "";
      x.volume = v[2] | -1;
      const std::string src = v[10] | "";
      x.src = -1;
      for (int i = 0; i < src_list(); i++) if (src == source_id()[i]) x.src = i;
    }
  }
  if (!x.sub_e.empty()) {
    JsonArrayConst v = doc[x.sub_e];
    if (!v.isNull()) {
      const char *w = v[0] | "";
      if (*w && strcmp(w, "unavailable") != 0 && strcmp(w, "unknown") != 0) { x.sub = (int) lroundf(atof(w)); x.sub_known = true; }
    }
  }
  if (!x.muziek.empty()) {
    JsonArrayConst v = doc[x.muziek];
    if (!v.isNull()) { x.sp_state = v[0] | ""; x.title = v[4] | ""; x.artist = v[5] | ""; }
  }
  show();
}
// ---- actions of the screen ----
inline void source_select(int i) {
  Data &x = d();
  if (i < 0 || i >= src_list() || x.speaker_e.empty() || !ha()) return;
  x.src = i;
  ha()("media_player.select_source", {{"entity_id", x.speaker_e}, {"source", source_id()[i]}});
  show();
}
inline void toggle_power() {
  Data &x = d();
  if (x.speaker_e.empty() || !ha()) return;
  const bool is_on = x.speaker_state == "off" || x.speaker_state.empty() || x.speaker_state == "standby";
  ha()(is_on ? "media_player.turn_on" : "media_player.turn_off", {{"entity_id", x.speaker_e}});
  x.speaker_state = is_on ? "on" : "off";
  show();
}
}  // namespace audio

// Schuifregelaar (volume 0-100, subwoofer -10..+10 dB): waarde live in het label, naar HA bij loslaten
inline lv_obj_t *slider_(lv_obj_t *body, int y, const char *name_s, int min, int max, lv_obj_t *&value_lbl, bool sub, const Fonts &f) {
  label(body, name_s, 0, y, 120, f.f12, TEXT2);
  value_lbl = label(body, "\xE2\x80\x94", 136, y, 80, f.f12, MUTED);
  lv_obj_set_style_text_align(value_lbl, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_t *s = lv_slider_create(body);
  lv_obj_set_pos(s, 6, y + 22);
  lv_obj_set_size(s, 204, 6);
  lv_slider_set_range(s, min, max);
  lv_obj_set_style_bg_color(s, k(SURF), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(s, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_radius(s, 3, LV_PART_MAIN);
  lv_obj_set_style_bg_color(s, k(0xBFC2C9), LV_PART_INDICATOR);
  lv_obj_set_style_radius(s, 3, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(s, k(TEXT), LV_PART_KNOB);
  lv_obj_set_style_pad_all(s, 5, LV_PART_KNOB);
  lv_obj_set_style_shadow_width(s, 0, LV_PART_KNOB);
  lv_obj_set_ext_click_area(s, 14);
  if (sub) lv_slider_set_mode(s, LV_SLIDER_MODE_SYMMETRICAL);
  auto cb = [](lv_event_t *e) {
    lv_obj_t *o = (lv_obj_t *) lv_event_get_target(e);
    const bool is_sub = lv_event_get_user_data(e) != nullptr;
    const int v = lv_slider_get_value(o);
    audio::Ui &u = audio::ui();
    if (is_sub) lv_label_set_text(u.sub_w, audio::sub_text_(v).c_str());
    else lv_label_set_text(u.vol_w, std::to_string(v).c_str());
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    audio::Data &x = audio::d();
    if (is_sub) {
      x.sub = v; x.sub_known = true;
      if (!x.sub_e.empty() && ha()) ha()("number.set_value", {{"entity_id", x.sub_e}, {"value", std::to_string(v)}});
    }
    else {
      x.volume = v;
      char lv[12]; snprintf(lv, sizeof(lv), "%.2f", v / 100.0f);
      if (!x.speaker_e.empty() && ha()) ha()("media_player.volume_set", {{"entity_id", x.speaker_e}, {"volume_level", lv}});
    }
  };
  // alleen deze twee: bij LV_EVENT_ALL zou het label zetten bij elk teken-event opnieuw tekenen (eindeloze lus)
  lv_obj_add_event_cb(s, cb, LV_EVENT_VALUE_CHANGED, sub ? (void *) 1 : nullptr);
  lv_obj_add_event_cb(s, cb, LV_EVENT_RELEASED, sub ? (void *) 1 : nullptr);
  return s;
}
static const char *ICO_NOTE = "\U000F0388";

// NOIR Audio: speaker status + on/off, "now playing" (tap = play/pause), sources, volume and subwoofer
inline void audio_build(lv_obj_t *layer_obj, const Fonts &f) {
  UiState &s = st();
  audio::Ui &u = audio::ui();
  lv_obj_t *body = s.audio_body = body_make(layer_obj, true);
  label(body, "Audio", 0, 0, 150, f.f20 != nullptr ? f.f20 : f.f16, TEXT);
  u.status = label(body, "", 0, 26, 160, f.f12, MUTED);
  u.is_on = icon_button_(body, 164, 0, 52, 34, AUDIO_POWER, ICO_POWER, f);
  // Nu speelt
  lv_obj_t *np = btn(body, 0, 48, 216, 76, AUDIO_NOW);
  if (f.ico != nullptr) {
    lv_obj_t *i = lv_label_create(np);
    lv_label_set_text(i, ICO_NOTE);
    lv_obj_set_style_text_font(i, f.ico, 0);
    lv_obj_set_style_text_color(i, k(TEXT2), 0);
    lv_obj_set_pos(i, 26, 27);
  }
  u.np_source = label(np, "MUZIEK", 76, 9, 80, f.f12, TEXT2);
  u.np_title = label(np, "", 76, 25, 130, f.f14 != nullptr ? f.f14 : f.f12, TEXT);
  u.np_sub = label(np, "", 76, 45, 130, f.f12, MUTED);
  // Ingang
  label(body, "Ingang", 0, 136, 120, f.f14 != nullptr ? f.f14 : f.f12, TEXT);
  const int bw = (216 - 2 * 6) / 3;
  for (int i = 0; i < audio::MAX_SOURCES; i++) u.src[i] = nullptr;
  for (int i = 0; i < audio::src_list(); i++)
    u.src[i] = text_button_(body, (i % 3) * (bw + 6), 158 + (i / 3) * 38, bw, 32, AUDIO_SOURCE + i, audio::source_name()[i].c_str(), f);
  // Niveau
  label(body, "Niveau", 0, 242, 120, f.f14 != nullptr ? f.f14 : f.f12, TEXT);
  u.vol_sld = slider_(body, 266, "Volume", 0, 100, u.vol_w, false, f);
  u.sub_sld = slider_(body, 310, "Subwoofer", -10, 10, u.sub_w, true, f);
  lv_obj_t *h = label(body, "Volumetoetsen op de remote regelen deze speaker.", 0, 352, 216, f.f12, MUTED);
  lv_label_set_long_mode(h, LV_LABEL_LONG_MODE_WRAP);
  audio::show();
}

// ---- Skin (Settings): colours + corner radius. The firmware keeps the choice (opslaan -> flash). ----------------------
inline std::function<void(int)> &save_skin() { static std::function<void(int)> f; return f; }   // skin number
inline void rebuild_all();
inline void theme_colors_() {
  Theme &t = theme();
  const int n = std::max(0, std::min(SKINS - 1, t.skin));
  t.bg = SKIN_COLOR[n][0]; t.surf = SKIN_COLOR[n][1]; t.text = SKIN_COLOR[n][2]; t.accent = SKIN_COLOR[n][3];
}
// At start (saved value) - before bouwen()
inline void theme_load(int skin) {
  Theme &t = theme();
  t.skin = std::max(0, std::min(SKINS - 1, skin));
  t.radius = SKIN_RADIUS[t.skin];
  theme_colors_();
}
inline void skin_choose(int nr) {
  theme_load(nr);
  if (save_skin()) save_skin()(theme().skin);
  rebuild_all();
}

// Choice list (Settings > Skin): LVGL is built without lv_dropdown, so our own list of buttons over the screen
// (lv_layer_top), in NOIR style.
struct Choice { lv_obj_t *over{nullptr}; };
inline Choice &choice() { static Choice k; return k; }
inline void choice_close() {
  Choice &ch = choice();
  if (ch.over != nullptr) { lv_obj_delete_async(ch.over); ch.over = nullptr; }
}
inline void choice_picked_(lv_event_t *e) {
  const int v = (int) (intptr_t) lv_event_get_user_data(e);
  choice_close();
  if (v >= 0 && v != theme().skin) skin_choose(v);   // v < 0: background = cancel
}
inline void choice_open_(lv_event_t *e) {
  (void) e;
  Choice &ch = choice();
  if (ch.over != nullptr) return;
  const Fonts &f = st().fonts;
  const char *const *name_list = SKIN_NAMES;
  const int n = SKINS, chosen = theme().skin;
  ch.over = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(ch.over);
  lv_obj_set_size(ch.over, 240, 320);
  lv_obj_set_style_bg_color(ch.over, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(ch.over, 175, 0);
  lv_obj_add_flag(ch.over, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(ch.over, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(ch.over, choice_picked_, LV_EVENT_CLICKED, (void *) (intptr_t) -1);
  const int h = 16 + 22 + n * 42 + 8;
  lv_obj_t *m = lv_obj_create(ch.over);
  lv_obj_set_size(m, 208, h);
  lv_obj_align(m, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(m, k(SURF), 0);
  lv_obj_set_style_bg_opa(m, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(m, rnd(16), 0);
  lv_obj_set_style_border_width(m, 1, 0);
  lv_obj_set_style_border_color(m, k(BORDER), 0);
  lv_obj_set_style_border_opa(m, LV_OPA_60, 0);
  lv_obj_set_style_pad_all(m, 0, 0);
  lv_obj_set_style_shadow_width(m, 0, 0);
  lv_obj_remove_flag(m, LV_OBJ_FLAG_SCROLLABLE);
  label(m, "Skin", 14, 12, 180, f.f12, MUTED);
  for (int i = 0; i < n; i++) {
    lv_obj_t *b = btn(m, 8, 34 + i * 42, 192, 36, -1);
    lv_obj_add_event_cb(b, choice_picked_, LV_EVENT_CLICKED, (void *) (intptr_t) i);
    lv_obj_t *l = label(b, name_list[i], 12, 0, 168, f.f12, 0xE4E7EC);
    lv_obj_align(l, LV_ALIGN_LEFT_MID, 12, 0);
    if (i == chosen) {   // huidige keuze: lichte rand
      lv_obj_set_style_border_color(b, k(0xBFC2C9), 0);
      lv_obj_set_style_border_opa(b, 200, 0);
    }
  }
}
// Button "Name  v" that opens the choice list (instead of a dropdown)
inline lv_obj_t *choice_list_(lv_obj_t *body, int y, const Fonts &f) {
  lv_obj_t *b = btn(body, 0, y, 216, 34, -1);
  lv_obj_add_event_cb(b, choice_open_, LV_EVENT_CLICKED, nullptr);
  const char *name_s = SKIN_NAMES[theme().skin];
  lv_obj_t *l = label(b, name_s, 12, 0, 170, f.f12, TEXT);
  lv_obj_align(l, LV_ALIGN_LEFT_MID, 12, 0);
  if (f.ico != nullptr) {
    lv_obj_t *p = lv_label_create(b);
    lv_label_set_text(p, "\U000F0140");   // mdi:chevron-down
    lv_obj_set_style_text_font(p, f.ico, 0);
    lv_obj_set_style_text_color(p, k(MUTED), 0);
    lv_obj_align(p, LV_ALIGN_RIGHT_MID, -10, 0);
  }
  return b;
}
inline void btn_text_(lv_obj_t *b, const char *t, const Fonts &f) {
  lv_obj_t *l = label(b, t, 0, 0, lv_obj_get_style_width(b, LV_PART_MAIN) - 10, f.f12, 0xE4E7EC);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(l);
}
inline void settings_build(lv_obj_t *layer_obj, const Fonts &f) {
  // NOIR Remote: only what matters - mode, skin, Wi-Fi
  lv_obj_t *body = body_make(layer_obj, false);
  label(body, "Instellingen", 0, 0, 216, f.f20 != nullptr ? f.f20 : f.f16, TEXT);
  label(body, "Modus", 0, 34, 216, f.f12, MUTED);
  auto mode = [&](int x, int actie, const char *name_s, bool is_on) {
    lv_obj_t *b = btn(body, x, 52, 104, 40, actie);
    btn_text_(b, name_s, f);
    if (is_on) { lv_obj_set_style_border_color(b, k(0xBFC2C9), 0); lv_obj_set_style_border_opa(b, 230, 0); }
  };
  mode(0, SET_FULL, "Full", !eco_mode());
  mode(112, SET_ECO, "Eco", eco_mode());
  label(body, "Skin", 0, 104, 216, f.f12, MUTED);
  choice_list_(body, 122, f);
  btn_text_(btn(body, 0, 170, 104, 40, SET_WIFI), "Wi-Fi", f);
  btn_text_(btn(body, 112, 170, 104, 40, KEYTEST), "Knoppentest", f);
}

// Lagen over page_home bouwen (eenmalig). home = page_home->obj.
inline void build_all(lv_obj_t *home, const Fonts &f, const ImageSet &bl, JsonArrayConst home_items = JsonArrayConst()) {
  UiState &s = st();
  if (s.toast_font == nullptr) s.toast_font = f.f14;
  if (s.layer_obj != nullptr || home == nullptr) return;
  s.home = home;
  s.fonts = f;
  s.images = bl;
  s.layer_obj = layer_make(home, bl);
  home_build(s.layer_obj, f, bl, home_items);
  bottom_button(s.layer_obj, ICO_BACK, "Terug", 12, BACK, f);
  bottom_button(s.layer_obj, ICO_HOME, "Thuis", 124, HOME, f);
  s.tv = layer_make(home, bl);
  tv_bouwen(s.tv, f, bl);
  s.light = layer_make(home, bl);
  lights_build(s.light, f);
  bottom_button(s.light, ICO_BACK, "Terug", 12, BACK, f);
  bottom_button(s.light, ICO_HOME, "Thuis", 124, HOME, f);
  s.airco = layer_make(home, bl);
  airco_build(s.airco, f);
  bottom_button(s.airco, ICO_BACK, "Terug", 12, BACK, f);
  bottom_button(s.airco, ICO_HOME, "Thuis", 124, HOME, f);
  s.scenes = layer_make(home, bl);
  scenes_build(s.scenes, f, bl, s.scenes_items ? s.scenes_items() : JsonArrayConst());
  bottom_button(s.scenes, ICO_BACK, "Terug", 12, BACK, f);
  bottom_button(s.scenes, ICO_HOME, "Thuis", 124, HOME, f);
  s.audio = layer_make(home, bl);
  audio_build(s.audio, f);
  bottom_button(s.audio, ICO_BACK, "Terug", 12, BACK, f);
  bottom_button(s.audio, ICO_HOME, "Thuis", 124, HOME, f);
  s.test = layer_make(home, bl);
  test_build(s.test, f);
  bottom_button(s.test, ICO_BACK, "Terug", 12, BACK, f);
  bottom_button(s.test, ICO_HOME, "Thuis", 124, HOME, f);
  s.settings = layer_make(home, bl);
  settings_build(s.settings, f);
  bottom_button(s.settings, ICO_BACK, "Terug", 12, BACK, f);
  bottom_button(s.settings, ICO_HOME, "Thuis", 124, HOME, f);
  topbar_make(s.layer_obj, f);
}

// Nieuwe configuratie: tegels van NOIR Thuis opnieuw opbouwen (statusbalk en Terug/Thuis blijven)
inline void body_replace_(lv_obj_t *layer_obj, lv_obj_t *&body, const std::function<void()> &make) {
  if (layer_obj == nullptr) return;
  if (body != nullptr) { lv_obj_delete(body); body = nullptr; }
  make();
  lv_obj_move_background(body);
  // wallpaper blijft onderop
  for (uint32_t c = 0; c < lv_obj_get_child_count(layer_obj); c++)
    if (lv_obj_check_type(lv_obj_get_child(layer_obj, c), &lv_image_class)) { lv_obj_move_background(lv_obj_get_child(layer_obj, c)); break; }
}
inline void home_rebuild(JsonArrayConst items) {
  UiState &s = st();
  body_replace_(s.layer_obj, s.home_body, [&]() { home_build(s.layer_obj, s.fonts, s.images, items); });
}
// Nieuwe scènes (configurator): NOIR Scènes opnieuw
inline void scenes_rebuild() {
  UiState &s = st();
  body_replace_(s.scenes, s.scenes_body, [&]() { scenes_build(s.scenes, s.fonts, s.images, s.scenes_items ? s.scenes_items() : JsonArrayConst()); });
}
// Nieuwe lijsten of laatst gestart: NOIR Netflix opnieuw (detail: nf_detail_tonen)
inline void show_screen(int cur_screen);
// Nieuwe lampenlijst (configurator): NOIR Verlichting opnieuw
inline void lights_rebuild() {
  UiState &s = st();
  for (Lamp &l : lamps()) l.lbl = nullptr;
  body_replace_(s.light, s.lights_body, [&]() { lights_build(s.light, s.fonts); });
}

inline lv_obj_t *layer_of(int cur_screen) {
  UiState &s = st();
  return cur_screen == S_TV ? s.tv : cur_screen == S_LIGHTS ? s.light : cur_screen == S_SETTINGS ? s.settings : cur_screen == S_AIRCO ? s.airco : cur_screen == S_TEST ? s.test : cur_screen == S_SCENES ? s.scenes : cur_screen == S_AUDIO ? s.audio : s.layer_obj;
}

// Een NOIR-pagina tonen (de andere verbergen); de statusbalk verhuist mee
inline void show_screen(int cur_screen) {
  UiState &s = st();
  if (s.layer_obj == nullptr) return;
  if (cur_screen != S_LIGHTS) dimmer_close();
  s.cur_screen = cur_screen;
  s.visible = true;
  for (int i = 0; i < S_COUNT; i++) {
    lv_obj_t *l = layer_of(i);
    if (l == nullptr) continue;
    if (i == cur_screen) { lv_obj_remove_flag(l, LV_OBJ_FLAG_HIDDEN); lv_obj_move_foreground(l); }
    else lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
  }
  if (s.topbar != nullptr) { lv_obj_set_parent(s.topbar, layer_of(cur_screen)); lv_obj_move_foreground(s.topbar); }
  if (cur_screen == S_LIGHTS) lights_http::soon(lv_tick_get(), 0);   // meteen de echte stand ophalen
}
inline bool at_tv() { return st().visible && st().cur_screen == S_TV; }
inline bool at_lights() { return st().visible && st().cur_screen == S_LIGHTS; }
inline bool at_airco() { return st().visible && st().cur_screen == S_AIRCO; }
inline bool at_test() { return st().visible && st().cur_screen == S_TEST; }
inline bool at_audio() { return st().visible && st().cur_screen == S_AUDIO; }

// NOIR on (last shown screen) or hidden (display off)
inline void show_page(bool an) {
  UiState &s = st();
  if (s.layer_obj == nullptr) return;
  if (an) { show_screen(s.cur_screen); return; }
  s.visible = false;
  dimmer_close();
  for (int i = 0; i < S_COUNT; i++) if (layer_of(i) != nullptr) lv_obj_add_flag(layer_of(i), LV_OBJ_FLAG_HIDDEN);
}

// Skin or language changed: every layer again (after the running event - the choice list that reported it is
// deleted itself)
inline void rebuild_all() {
  static bool planned = false;
  if (planned || st().layer_obj == nullptr) return;
  planned = true;
  lv_async_call([](void *) {
    planned = false;
    UiState &s = st();
    if (s.layer_obj == nullptr || s.home == nullptr) return;
    theme_colors_();
    dimmer_close();
    choice_close();
    const bool vis = s.visible;
    const int cur_screen = s.cur_screen;
    lv_obj_t *gone[] = {s.layer_obj, s.tv, s.light, s.settings, s.airco, s.test, s.scenes, s.audio};
    for (lv_obj_t *o : gone) if (o != nullptr) lv_obj_delete(o);   // kop hangt aan een ervan
    s.layer_obj = s.tv = s.light = s.settings = s.airco = s.test = s.scenes = s.scenes_body = s.audio = s.audio_body = nullptr;
    audio::ui() = audio::Ui();
    for (auto &t : keytest::st_().tile) t = nullptr;
    keytest::st_().info = nullptr;
    airco::ui() = airco::Ui();
    s.topbar = s.clock = s.batt_txt = s.batt_fill = s.ha_dot = s.tv_status = s.home_body = s.lights_body = nullptr;
    for (Lamp &l : lamps()) l.lbl = nullptr;
    build_all(s.home, s.fonts, s.images, s.items ? s.items() : JsonArrayConst());
    tv_status(s.tv_state, "");
    s.cur_screen = cur_screen;
    if (vis) show_screen(cur_screen);
  }, nullptr);
}

// TV-status uit HA (state van de media_player, attribuut source) op het TV-scherm
inline void tv_status(const std::string &state, const std::string &src) {
  UiState &s = st();
  s.tv_state = state;
  if (s.tv_status == nullptr) return;
  std::string t;
  if (state == "off") t = std::string("\xE2\x97\x8F ") + tr("Uit");
  else if (state.empty() || state == "unknown" || state == "unavailable") t = std::string("\xE2\x97\x8F ") + tr("Niet bereikbaar");
  else { t = std::string("\xE2\x97\x8F ") + tr("Aan"); if (!src.empty()) t += " \xC2\xB7 " + src; }
  if (t != lv_label_get_text(s.tv_status)) lv_label_set_text(s.tv_status, t.c_str());
}
inline bool tv_aan() { return st().tv_state == "on" || st().tv_state == "playing" || st().tv_state == "paused" || st().tv_state == "idle"; }

// Statusbalk bijwerken (elke seconde vanuit de firmware). pct < 0 = onbekend.
// rssi: wifi-signaal in dBm, 0 = geen wifi-verbinding (streepjes dof, eerste rood)
inline void status(int uu, int mm, int pct, bool charging, bool ha, int rssi = -50) {
  UiState &s = st();
  if (s.topbar == nullptr || !s.visible) return;
  char t[16];
  if (uu >= 0) snprintf(t, sizeof(t), "%02d:%02d", uu, mm); else snprintf(t, sizeof(t), "--:--");
  if (strcmp(lv_label_get_text(s.clock), t) != 0) lv_label_set_text(s.clock, t);
  if (pct >= 0) snprintf(t, sizeof(t), "%d%%", pct); else t[0] = 0;
  if (strcmp(lv_label_get_text(s.batt_txt), t) != 0) lv_label_set_text(s.batt_txt, t);
  const int w = pct < 0 ? 0 : 1 + (pct * 18 + 50) / 100;   // 19 px = vol
  lv_obj_set_width(s.batt_fill, w);
  lv_obj_set_style_bg_color(s.batt_fill, k(charging ? 0xF2F3F5 : BATT_F), 0);
  lv_obj_set_style_bg_opa(s.ha_dot, ha ? 220 : 45, 0);
  const int n = rssi >= 0 ? 0 : rssi >= -55 ? 4 : rssi >= -65 ? 3 : rssi >= -75 ? 2 : 1;
  for (int i = 0; i < 4; i++) {
    if (s.wifi[i] == nullptr) continue;
    lv_obj_set_style_bg_color(s.wifi[i], k(n == 0 && i == 0 ? 0xE5484D : STAT), 0);
    lv_obj_set_style_bg_opa(s.wifi[i], n == 0 ? (i == 0 ? 230 : 45) : (i < n ? 220 : 45), 0);
  }
}

// Korte melding bovenaan (verdwijnt vanzelf na ms). Eigen label op lv_layer_top - de pil van de firmware
// (tasten::modus_anzeige) hoort bij de volgmodus en verdwijnt niet vanzelf (2026-11-21: bleef staan na Stumm).
inline void toast(const char *text_s, uint32_t ms = 1500) {
  static lv_obj_t *t = nullptr;
  static lv_timer_t *gone = nullptr;
  if (t == nullptr) {
    t = lv_label_create(lv_layer_top());
    lv_obj_add_flag(t, LV_OBJ_FLAG_USER_4);
    lv_obj_set_style_bg_opa(t, 240, 0);
    if (st().toast_font != nullptr) lv_obj_set_style_text_font(t, st().toast_font, 0);
    lv_obj_set_style_radius(t, 14, 0);
    lv_obj_set_style_border_width(t, 1, 0);
    lv_obj_set_style_border_color(t, k(BORDER_P), 0);
    lv_obj_set_style_pad_hor(t, 16, 0);
    lv_obj_set_style_pad_ver(t, 8, 0);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_remove_flag(t, LV_OBJ_FLAG_CLICKABLE);
  }
  lv_obj_set_style_bg_color(t, k(SURF), 0);   // skin kan intussen gewisseld zijn
  lv_obj_set_style_text_color(t, k(TEXT), 0);
  lv_obj_set_style_border_color(t, k(BORDER_P), 0);
  lv_label_set_text(t, tr(text_s));
  lv_obj_remove_flag(t, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(t);
  if (gone != nullptr) lv_timer_delete(gone);
  gone = lv_timer_create([](lv_timer_t *) { lv_obj_add_flag(t, LV_OBJ_FLAG_HIDDEN); gone = nullptr; }, ms, nullptr);
  lv_timer_set_repeat_count(gone, 1);
}

// ---- HA-aanroep en ingedrukt-houden (gedeeld door KEF en TV) ---------------------------------------------------
// HA-aanroep van de firmware (tk_cfg->ha_aufruf: via wifi, Bluetooth-link of dock, met wachtrij na het wekken)
inline std::function<void(const std::string &, Data)> &ha() {
  static std::function<void(const std::string &, Data)> z;
  return z;
}
struct Hold { int key_in{0}; int n{0}; uint32_t since{0}; std::function<void()> f; };
inline Hold &hold() { static Hold h; return h; }
static const uint32_t REPEAT_MS = 300;
static const int REPEAT_MAX = 30;
inline void hold_start(int key_in, std::function<void()> f) { Hold &h = hold(); h.key_in = key_in; h.n = 0; h.since = 0; h.f = std::move(f); }
inline void hold_release(int key_in) { if (hold().key_in == key_in) hold().key_in = 0; }
// Elke ~100 ms vanuit de firmware (interval): ingedrukt houden herhaalt
inline void repeat_tick(uint32_t now_ms) {
  Hold &h = hold();
  if (h.key_in == 0) { h.since = 0; return; }
  if (h.since == 0) { h.since = now_ms; return; }
  if (now_ms - h.since < REPEAT_MS) return;
  h.since = now_ms;
  if (++h.n > REPEAT_MAX) { h.key_in = 0; return; }   // loslaten kwam nooit aan
  if (h.f) h.f();
}

// ---- KEF LSX II: hardwarevolume op elk scherm ------------------------------------------------------------------
namespace speaker_e {

static const int T_LOUDER = 33, T_QUIETER = 43, T_MUTE = 44;
using noir::Data;

inline std::string &entity_ref() { static std::string e; return e; }
inline std::function<void(const std::string &, Data)> &sender() { return noir::ha(); }
inline std::function<void(const char *)> &notice() { static std::function<void(const char *)> m; return m; }

inline bool is_volume(int t) { return t == T_LOUDER || t == T_QUIETER || t == T_MUTE; }
inline bool actief() { return !entity_ref().empty() && sender() != nullptr; }

inline void send_(int key_in) {
  const char *svc = key_in == T_LOUDER ? "media_player.volume_up" : key_in == T_QUIETER ? "media_player.volume_down" : nullptr;
  if (svc != nullptr) sender()(svc, {{"entity_id", entity_ref()}});
}

// Uit on_key, vóór alles: true = afgehandeld (niets anders krijgt de toets)
inline bool key_in(int t, bool is_down) {
  if (!actief() || !is_volume(t)) return false;
  if (!is_down) { hold_release(t); return true; }
  if (t == T_MUTE) {
    // Dempen omschakelen. De remote kent de werkelijke mute-stand van de KEF niet: hij wisselt zelf tussen
    // is_volume_muted true/false (HA zet de tekst "true"/"false" zelf om).
    static bool muted = false;
    muted = !muted;
    sender()("media_player.volume_mute", {{"entity_id", entity_ref()}, {"is_volume_muted", muted ? "true" : "false"}});
    if (notice()) notice()(muted ? "Gedempt" : "Geluid aan");
    return true;
  }
  send_(t);
  hold_start(t, [t]() { send_(t); });
  return true;
}
// oude naam (interval in de YAML)
inline void repeat_tick(uint32_t now_ms) { noir::repeat_tick(now_ms); }

}  // namespace kef

// ---- TV: the TV's media_player in Home Assistant (module Tv sets it) -----------------------------------------------
namespace tv {
inline std::string &entity_ref() { static std::string e; return e; }
}  // namespace tv
}  // namespace noir
