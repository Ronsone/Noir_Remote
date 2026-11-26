#pragma once
// Icons that NOIR Builder offers for tiles and scenes: mdi name -> code point in the icon font.
// Keep in sync with ICONS / OPENS_ICON in custom_components/noir_remote/frontend/noir-builder.js;
// tools/check_icons.py checks that every icon here is also in the font (firmware/noir-remote.yaml, ico20).
#include <cstdint>
namespace noir {
struct IconEntry { const char *name; uint32_t cp; };
static const IconEntry MDI_ICONS[] = {
  {"air-conditioner", 0xF001B},
  {"bed", 0xF02E3},
  {"blinds", 0xF00AC},
  {"ceiling-light", 0xF0769},
  {"coffee", 0xF0176},
  {"cog", 0xF0493},
  {"door", 0xF081A},
  {"fan", 0xF0210},
  {"flash", 0xF0241},
  {"floor-lamp", 0xF08DD},
  {"heart", 0xF08D0},
  {"home", 0xF02DC},
  {"lamp", 0xF06B5},
  {"leaf", 0xF032A},
  {"led-strip", 0xF07D6},
  {"lightbulb", 0xF0335},
  {"lock", 0xF033E},
  {"movie-open", 0xF0FCE},
  {"music", 0xF075A},
  {"netflix", 0xF0746},
  {"nintendo-switch", 0xF07E1},
  {"play", 0xF040A},
  {"power", 0xF0425},
  {"silverware-fork-knife", 0xF0A70},
  {"sofa", 0xF04B9},
  {"sony-playstation", 0xF0414},
  {"speaker", 0xF04C3},
  {"star", 0xF04CE},
  {"string-lights", 0xF12BA},
  {"television", 0xF0502},
  {"thermometer", 0xF050F},
  {"weather-night", 0xF0594},
  {"white-balance-sunny", 0xF05A8},
  {"window-shutter", 0xF111C},
  {"youtube", 0xF05C3},
};
}  // namespace noir
