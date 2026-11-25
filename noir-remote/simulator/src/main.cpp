// NOIR-simulator voor de ESPHomeRemote (OpenRemote Rev6, 240 x 320)
//
// Draait esphomeremote-noir.h - dezelfde schermcode als de firmware - in een venster op de pc (LVGL 9.5 + SDL2).
// Home Assistant, de tv, IR en internet zijn nagebootst: wat de remote naar HA zou sturen verschijnt in de console.
//
// Muis = vinger. Toetsen: pijlen = D-pad, Enter = OK, Backspace/Esc = Terug, H = Home, spatie = play/pauze,
// S = stop, , en . = spoelen, + / - / M = volume/dempen (KEF), P = schermafdruk (noir-<n>.bmp), Q = stoppen.
// Opstarten met --demo loopt alle schermen langs, bewaart van elk een schermafdruk en stopt.

#include "lvgl.h"
#include "ArduinoJson.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

extern "C" {
extern const lv_font_t inter12, inter14, inter16, inter20, inter48, mdi20;
}
#include "noir-ui.h"
#include "noir_assets.h"
#include "logos.h"
#include "posters.h"

using namespace noir;

// ---------------------------------------------------------------- voorbeeldgegevens (zoals uit de configurator)
static const char *THUIS_JSON = R"([
 {"t":"action","label":"TV","icon":"noir:tv","x":0,"y":0,"w":3,"h":2,"fn":"tv"},
 {"t":"action","label":"Verlichting","icon":"noir:licht","x":3,"y":0,"w":3,"h":2,"fn":"lights"},
 {"t":"action","label":"Sc\u00e8nes","icon":"noir:scenes","x":0,"y":2,"w":3,"h":2,"fn":"scenes"},
 {"t":"action","label":"Alles uit","icon":"mdi:power","x":3,"y":2,"w":3,"h":2,"fn":"alloff"}
])";
static const char *SCENES_JSON = R"([
 {"t":"action","label":"Film","icon":"mdi:movie-open"},
 {"t":"action","label":"Diner","icon":"mdi:silverware-fork-knife"},
 {"t":"action","label":"Avond","icon":"mdi:weather-night"}
])";
static const char *LAMPEN = "Woonkamer=light.woonkamer|Eettafel=light.eettafel|Keuken=light.keuken|Gang=switch.gang";
// --en: the same sample home in English
static const char *SCENES_JSON_EN = R"([
 {"t":"action","label":"Movie","icon":"mdi:movie-open"},
 {"t":"action","label":"Dinner","icon":"mdi:silverware-fork-knife"},
 {"t":"action","label":"Evening","icon":"mdi:weather-night"}
])";
static const char *LAMPEN_EN = "Living room=light.woonkamer|Dining table=light.eettafel|Kitchen=light.keuken|Hall=switch.gang";
static const char *NF_TITELS[16] = {"Night Harbor", "The Glass Coast", "Northline", "Echo Park 9", "Velvet Hours",
                                    "Salt & Ember", "The Cartographer", "Low Orbit", "Kingfisher", "Ashfall", "Parallel",
                                    "Quiet Signal", "Red Meridian", "The Long Winter", "Copperhead", "Lantern"};   // verzonnen voorbeeldtitels

static JsonDocument g_thuis, g_scenes;
static bool g_demo = false;

// ---------------------------------------------------------------- nagebootste buitenwereld
static void ha_print(const std::string &svc, const Data &d) {
  std::printf("[HA] %s", svc.c_str());
  for (auto &kv : d) std::printf("  %s=%s", kv.first.c_str(), kv.second.c_str());
  std::printf("\n");
}
static void ha_stub(const std::string &svc, Data d) {
  ha_print(svc, d);
  if (svc == "media_player.turn_off" && d["entity_id"] == tv::entiteit()) tv_status("off", "");
}

static uint8_t *verloop_(int w, int h, uint32_t a, uint32_t b, int soort);
static void netflix_voorbeeld() {
  const bool vrij = getenv("NOIR_README") != nullptr;   // abstracte posters zonder titels (voor de README)
  static const uint32_t pk[8][2] = {{0x0F766E, 0xA7F3D0}, {0x1E3A8A, 0x93C5FD}, {0x7C2D12, 0xFBBF24}, {0x312E81, 0xF0ABFC}, {0x064E3B, 0x84CC16}, {0x7F1D1D, 0xFCA5A5}, {0x0C4A6E, 0xE0F2FE}, {0x111827, 0x9CA3AF}};
  nf::Data &x = nf::d();
  x.week = "9\xE2\x80\x93" "15 nov";
  const char *genre[4] = {"Mysterie, Komedie", "Sci-fi, Drama", "Drama, Thriller", "Misdaad"};
  for (int i = 0; i < 10; i++) {
    nf::Titel t;
    t.id = 100 + i; t.naam = NF_TITELS[i]; t.zoek = t.naam; t.jaar = std::to_string(2016 + i % 8); t.genre = genre[i % 4];
    t.seizoenen = 1 + i % 4; t.leeftijd = i % 2 ? "12+" : "16+"; t.netflix = std::to_string(80000000 + i);
    t.tekst = "Voorbeeldtekst: in de firmware staat hier de Nederlandse beschrijving van TMDB, ingekort tot drie regels.";
    x.top10.push_back(t);
    if (vrij) nf::beeld_vullen_(x.posters[t.id], verloop_(64, 96, pk[i % 8][0], pk[i % 8][1], i % 3), 64, 96);
    else x.posters[t.id].dsc = *POSTERS[i];
    nf::beeld_vullen_(x.klein[t.id], verloop_(48, 72, pk[i % 8][0], pk[i % 8][1], i % 3), 48, 72);   // detailscherm
  }
  for (int i = 0; i < 6; i++) {
    nf::Titel t;
    t.id = 200 + i; t.naam = NF_TITELS[10 + i]; t.zoek = t.naam; t.jaar = "2019"; t.genre = "Drama"; t.seizoenen = 3;
    x.populair.push_back(t);
    if (vrij) nf::beeld_vullen_(x.posters[t.id], verloop_(64, 96, pk[(i + 3) % 8][0], pk[(i + 3) % 8][1], (i + 1) % 3), 64, 96);
    else x.posters[t.id].dsc = *POSTERS[10 + i];
  }
  nf::laatst_laden("101\tThe Glass Coast\t80000001\n106\tThe Cartographer\t80000006\n");
  x.bewaar = [](const std::string &s) { std::printf("[remote] laatst gestart bewaard (%zu tekens)\n", s.size()); };
  x.achter.dsc = backdrop;
  x.geladen = true;
}

// Voorbeeldhoesjes: abstracte verlopen (de firmware haalt de echte hoes bij HA op, RGB565 zoals de Netflix-posters)
static uint8_t *verloop_(int w, int h, uint32_t a, uint32_t b, int soort) {
  uint8_t *px = (uint8_t *) malloc(w * h * 2);
  for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) {
    int t = soort == 0 ? (x + y) * 255 / (w + h) : soort == 1 ? y * 255 / h : (std::abs(x - w / 2) + std::abs(y - h / 2)) * 255 / w;
    if (soort == 3) t = ((x / 8 + y / 8) % 2) ? 70 : 200;
    auto c = [&](int sh) { return (((a >> sh) & 255) * (255 - t) + ((b >> sh) & 255) * t) / 255; };
    const uint16_t v = ((c(16) >> 3) << 11) | ((c(8) >> 2) << 5) | (c(0) >> 3);
    px[(y * w + x) * 2] = v & 255; px[(y * w + x) * 2 + 1] = v >> 8;
  }
  return px;
}
static void audio_voorbeeld() {
  audio::Data &x = audio::d();
  audio::instellen("media_player.woonkamer_2", "number.woonkamer_sw_gain", "media_player.woonkamer");
  x.kef_state = "on"; x.bron = 0; x.volume = 38; x.sub = 2; x.sub_bekend = true;
  x.sp_state = "playing"; x.titel = "Midnight Drive"; x.artiest = "Nova Lane"; x.album = "City Lights";
  x.pos = 83; x.duur = 214; x.shuffle = true; x.repeat = 0; x.cover = 1;
  const uint32_t kl[9][2] = {{0x1E3A8A, 0xF472B6}, {0x0F766E, 0xA7F3D0}, {0x7C2D12, 0xFBBF24}, {0x312E81, 0x22D3EE},
                             {0x111827, 0x9CA3AF}, {0x701A75, 0xF0ABFC}, {0x064E3B, 0x84CC16}, {0x7F1D1D, 0xFCA5A5}, {0x0C4A6E, 0xE0F2FE}};
  for (int i = 0; i < 9; i++) audio::beeld_vullen(x.covers[i + 1], verloop_(96, 96, kl[i][0], kl[i][1], i % 4), 96, 96);
  const char *pl[6][2] = {{"Avond chill", "52 nummers"}, {"Focus", "Instrumentaal"}, {"Roadtrip", "Rons"}, {"Jaren 90", "Rons"},
                          {"Synthwave", "38 nummers"}, {"Zondag", "Akoestisch"}};
  for (int i = 0; i < 6; i++) x.playlists.push_back({pl[i][0], pl[i][1], "library://playlist/" + std::to_string(i), "playlist", 2 + i % 8});
  const char *rc[4][2] = {{"City Lights", "Nova Lane"}, {"Deep Blue", "Kaito"}, {"Low Tide", "Mira Vos"}, {"Signals", "The Arcs"}};
  for (int i = 0; i < 4; i++) x.recent.push_back({rc[i][0], rc[i][1], "library://album/" + std::to_string(i), "album", 1 + (i * 3) % 9});
}

// Een tegel van NOIR Thuis uitvoeren (zoals tasten::noir_item_uitvoeren)
static void item_uitvoeren(int i);

// ---------------------------------------------------------------- acties (zoals tasten::noir_actie in de firmware)
static void actie(int a) {
  if (a >= SCENE && a < SCENE + 40) {
    JsonArrayConst it = g_scenes.as<JsonArrayConst>();
    if (a - SCENE < (int) it.size()) toast(it[a - SCENE]["label"] | "Sc\xC3\xA8ne");
    std::printf("[remote] sc\xC3\xA8ne %d starten (stappen uit de configurator)\n", a - SCENE);
    return;
  }
  if (a >= NF_TITEL && a < NF_TITEL + 300) {
    // de firmware haalt de achtergrond bij HA op; hier hetzelfde voorbeeldbeeld voor elke titel
    nf::Titel t; int plek = 0;
    if (nf::titel_van(a - NF_TITEL, t, plek)) { nf::d().achter_id = t.id; nf::d().achter.px = (uint8_t *) backdrop.data; }
    nf_detail_tonen(a - NF_TITEL);
    return;
  }
  if (a >= AUDIO_BRON && a < AUDIO_BRON + audio::BRONNEN) { audio::bron_kiezen(a - AUDIO_BRON); return; }
  if (a >= ZOEK_TOETS && a < ZOEK_TOETS + 26) { audio::zoek_toets(a - ZOEK_TOETS); return; }
  if (a >= ZOEK_RES && a < ZOEK_RES + 10) { audio::zoek_spelen(a - ZOEK_RES); toast(audio::d().titel.c_str()); return; }
  if (a >= SP_LIJST && a < SP_RECENT + 100) {
    if (audio::lijst_actie(a, SP_LIJST, SP_RECENT)) toast(audio::d().titel.c_str());
    return;
  }
  if (a >= ITEM) { item_uitvoeren(a - ITEM); return; }
  if (a >= AIRCO_MODUS && a < AIRCO_MODUS + 5) { airco::modus(a - AIRCO_MODUS); return; }
  if (a >= LAMP && a < LAMP + 12) { lamp_schakelen(a - LAMP, lv_tick_get()); return; }
  if (a >= TV_APP && a < TV_APP + (int) tv_apps().size()) {
    const TvApp &app = tv_apps()[a - TV_APP];
    if (!tv_aan()) tv_status("on", "");
    ha_print("media_player.select_source", {{"source", app.source}});
    toast(app.name.c_str());
    return;
  }
  switch (a) {
    case TV: tv_status("on", "Live TV"); zeigen_seite(S_TV); break;
    case TV_AANUIT:
      if (tv_aan()) { ha_print("media_player.turn_off", {{"entity_id", tv::entiteit()}}); tv_status("off", ""); toast("TV uit"); }
      else { tv_status("on", "Live TV"); toast("TV aan"); }
      break;
    case AUDIO: zeigen_seite(S_AUDIO); break;
    case AUDIO_MUZIEK: zeigen_seite(S_MUZIEK); break;
    case AUDIO_AANUIT: audio::aanuit(); break;
    case SP_SHUFFLE: audio::shuffle_wissel(); break;
    case SP_REPEAT: audio::repeat_wissel(); break;
    case ZOEK_OPEN: zeigen_seite(S_ZOEK); break;
    case ZOEK_WIS: audio::zoek_wis(); break;
    case ZOEK_SPATIE: audio::zoek_spatie(); break;
    case ZOEK_GA: {   // simulator: HA nagebootst met vaste resultaten
      audio::zoek_ga();
      const char *tsv = "z\tMidnight Drive\tNova Lane\tlibrary://track/1\ttrack\nz\tMidnight Drive (Live)\tNova Lane\tlibrary://track/2\ttrack\nz\tMidnight City Echo\tKaito\tlibrary://track/3\ttrack\nz\tAfter Midnight\tMira Vos\tlibrary://track/4\ttrack\n";
      uint8_t *b = (uint8_t *) malloc(strlen(tsv)); memcpy(b, tsv, strlen(tsv));
      nf::Taak t{"", 0, 'z', 4096};
      nf::d().rij.clear();
      audio::klaar_(t, 200, b, strlen(tsv));
      for (int i = 0; i < 4; i++) audio::beeld_vullen(audio::d().covers[audio::COVER_ZOEK + i], verloop_(64, 64, 0x1E3A8A + i * 0x203010, 0xF472B6 - i * 0x101020, i % 4), 64, 64);
      zoek_herbouwen();
      break;
    }
    case ZOEK_VELD: audio::zoek_veld_tik(); break;
    case ZOEK_INVOER: audio::invoer_wissel(); break;
    case WIEL_GROEP: audio::wiel_groep(-1); break;
    case WIEL_CIJFERS: audio::wiel_groep(2); break;
    case WIEL_TEKENS: audio::wiel_groep(3); break;
    case LICHT: zeigen_seite(S_LICHT); break;
    case SCENES: zeigen_seite(S_SCENES); break;
    case SCENE_ALLES_UIT:
      for (auto &l : lampen()) l.state = "off";
      lampen_tekst();
      ha_print("light.turn_off", {{"entity_id", "alle lampen"}});
      toast("Alles uit");
      break;
    case AIRCO: zeigen_seite(S_AIRCO); break;
    case AIRCO_AANUIT: airco::aanuit(); break;
    case AIRCO_MIN: airco::temp_stap(-1); break;
    case AIRCO_PLUS: airco::temp_stap(1); break;
    case AIRCO_FAN: airco::fan_volgende(); break;
    case AIRCO_SWING: airco::swing_volgende(); break;
    case AIRCO_HI: airco::fan_speciaal(5); break;
    case AIRCO_ECO: airco::fan_speciaal(6); break;
    case KNOPTEST: knoptest::reset(); zeigen_seite(S_TEST); break;
    case TEST_RESET: knoptest::reset(); break;
    case INSTELLINGEN: zeigen_seite(S_INST); break;
    case INST_REMOTE: toast("Aggro600-instellingen (niet in de simulator)"); break;
    case INST_AFRONDING: {
      Thema &t = thema();
      t.radius = t.radius >= 20 ? 8 : t.radius + 2;
      alles_herbouwen();
      break;
    }
    case DIM_GEREED: dimmer_dicht(); break;
    case NETFLIX: zeigen_seite(S_NETFLIX); break;
    case NF_AFSPELEN: {
      const nf::Titel t = nf::d().detail;
      nf::laatst_toevoegen(t);
      netflix_herbouwen();
      if (!t.netflix.empty()) ha_print("esphomeremote.tv_app", {{"app", "netflix"}, {"content", t.netflix}});
      else ha_print("esphomeremote.tv_app", {{"app", "netflix"}});
      toast(t.naam.c_str());
      break;
    }
    case TERUG:
      if (st().seite == S_NF_DETAIL) zeigen_seite(S_NETFLIX);
      else if (st().seite == S_NETFLIX) zeigen_seite(S_TV);
      else if (st().seite == S_MUZIEK) zeigen_seite(S_AUDIO);
      else if (st().seite == S_ZOEK) zeigen_seite(S_MUZIEK);
      else if (st().seite != S_THUIS) zeigen_seite(S_THUIS);
      break;
    case THUIS: zeigen_seite(S_THUIS); break;
    default: std::printf("[remote] actie %d\n", a); break;
  }
}

static void item_uitvoeren(int i) {
  JsonArrayConst items = g_thuis.as<JsonArrayConst>();
  if (i < 0 || i >= (int) items.size()) return;
  JsonObjectConst it = items[i];
  const std::string t = it["t"] | "";
  if (t == "special") { actie(AUDIO); return; }
  const std::string fn = it["steps"][0]["fn"] | "";
  if (fn == "noir_tv") actie(TV);
  else if (fn == "noir_licht") actie(LICHT);
  else if (fn == "noir_scenes") actie(SCENES);
  else if (fn == "noir_airco") actie(AIRCO);
  else if (fn == "noir_netflix") actie(NETFLIX);
  else if (fn == "noir_test") actie(KNOPTEST);
  else if (fn == "noir_inst") actie(INSTELLINGEN);
  else std::printf("[remote] tegel %d: %s\n", i, fn.c_str());
}

// ---------------------------------------------------------------- fysieke toetsen (keycodes van de Rev6)
static void toets(int t) {
  // zoals on_key in de firmware: eerst KEF (volume), dan het NOIR TV-/Netflix-/airco-/testscherm
  for (bool in : {true, false}) {
    if (kef::taste(t, in)) continue;
    if (audio_taste(t, in)) continue;
    if (tv::taste(t, in, true)) continue;
    if (!in) continue;
    if (t == 13) actie(TERUG);
    else if (t == 3) actie(THUIS);
    else std::printf("[remote] toets %d (hier niet gebruikt)\n", t);
  }
}

static int g_shot = 0;
static void schermafdruk(const char *naam = nullptr) {
  lv_draw_buf_t *b = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_ARGB8888);
  if (b == nullptr) { std::printf("[sim] schermafdruk mislukt (geheugen)\n"); return; }
  char fn[64];
  if (naam) std::snprintf(fn, sizeof(fn), "%s", naam);
  else std::snprintf(fn, sizeof(fn), "noir-%d.bmp", ++g_shot);
  FILE *f = std::fopen(fn, "wb");
  if (f != nullptr) {
    const int w = b->header.w, h = b->header.h, stride = b->header.stride, size = 54 + w * h * 4;
    unsigned char hd[54] = {'B', 'M'};
    auto le = [&](int o, int v) { for (int i = 0; i < 4; i++) hd[o + i] = (v >> (8 * i)) & 255; };
    le(2, size); le(10, 54); le(14, 40); le(18, w); le(22, -h); hd[26] = 1; hd[28] = 32; le(34, w * h * 4);
    std::fwrite(hd, 1, 54, f);
    for (int y = 0; y < h; y++) std::fwrite(b->data + y * stride, 1, w * 4, f);
    std::fclose(f);
    std::printf("[sim] schermafdruk %s\n", fn);
  }
  lv_draw_buf_destroy(b);
}

static void toets_cb(lv_event_t *e) {
  const uint32_t k = lv_event_get_key(e);
  switch (k) {
    case LV_KEY_UP: toets(14); break;
    case LV_KEY_DOWN: toets(35); break;
    case LV_KEY_LEFT: toets(15); break;
    case LV_KEY_RIGHT: toets(32); break;
    case LV_KEY_ENTER: case '\r': toets(34); break;
    case LV_KEY_BACKSPACE: case LV_KEY_ESC: toets(13); break;
    case 'h': case 'H': toets(3); break;
    case ' ': toets(2); break;
    case 's': case 'S': toets(5); break;
    case ',': toets(4); break;
    case '.': toets(11); break;
    case '+': case '=': toets(33); break;
    case '-': toets(43); break;
    case 'm': case 'M': toets(44); break;
    case 'p': case 'P': schermafdruk(); break;
    case 'q': case 'Q': std::exit(0);
    default: break;
  }
}

static void klok_cb(lv_timer_t *) {
  std::time_t t = std::time(nullptr);
  std::tm *l = std::localtime(&t);
  status(l->tm_hour, l->tm_min, 78, false, true, getenv("NOIR_GEEN_WIFI") ? 0 : -60);
  herhaal_tik(lv_tick_get());
}

// --demo: alle schermen langs, van elk een schermafdruk
static void demo_cb(lv_timer_t *tm) {
  static int stap = 0;
  struct S { int actie; const char *bestand; };
  static const S stappen[] = {{THUIS, "demo-01-thuis.bmp"}, {TV, "demo-02-tv.bmp"}, {LICHT, "demo-03-verlichting.bmp"},
                              {SCENES, "demo-04-scenes.bmp"}, {AIRCO, "demo-05-airco.bmp"}, {NETFLIX, "demo-06-netflix.bmp"},
                              {NF_TITEL + 0, "demo-07-netflix-detail.bmp"}, {INSTELLINGEN, "demo-08-instellingen.bmp"},
                              {KNOPTEST, "demo-09-knoppentest.bmp"}, {AUDIO, "demo-10-audio.bmp"},
                              {-2, "demo-10b-audio-onder.bmp"}, {AUDIO_MUZIEK, "demo-11-muziek.bmp"},
                              {-1, "demo-11b-muziek-onder.bmp"}, {-4, "demo-12-test.bmp"}, {-5, "demo-13-lees.bmp"}, {-6, "demo-14-letterwiel.bmp"}, {-8, "demo-14b-qwerty.bmp"}, {-7, "demo-15-zoekresultaat.bmp"}};
  const int n = sizeof(stappen) / sizeof(stappen[0]);
  if (stap > 0) schermafdruk(stappen[stap - 1].bestand);
  if (stap >= n) { lv_timer_delete(tm); std::exit(0); }
  if (stappen[stap].actie == -2) lv_obj_scroll_to_y(st().audio_body, 200, LV_ANIM_OFF);
  else if (stappen[stap].actie == -1) lv_obj_scroll_to_y(st().muziek_body, 190, LV_ANIM_OFF);
  else if (stappen[stap].actie == -5) {
    nf::url_basis() = []() { return std::string("http://ha:8123/api/esphomeremote/states?k=K"); };
    JsonDocument doc;
    deserializeJson(doc, R"({"media_player.woonkamer_2":["on",-1,45,"","","",false,false,"","","tv",-1,0,0],
      "number.woonkamer_sw_gain":["-3.0",-1,-1,"dB",-10,10,1],
      "media_player.woonkamer":["playing",-1,50,"","Echte Titel","Echte Artiest",false,true,"one","Music Assistant","",30.0,200.0,4.5]})");
    audio::lees_(doc);
    const char *tsv = "p\tLijst A\tPlaylist\tlibrary://playlist/1\tplaylist\np\tLijst B\t\tspotify--x://playlist/2\tplaylist\nr\tNummer C\tArtiest\tlibrary://track/3\ttrack\n";
    uint8_t *b = (uint8_t *) malloc(strlen(tsv)); memcpy(b, tsv, strlen(tsv));
    nf::Taak t{"", 0, 'a', 8192};
    audio::klaar_(t, 200, b, strlen(tsv));
    audio::Data &x = audio::d();
    std::printf("[test] bron=%d vol=%d sub=%d titel=%s rep=%d shuf=%d pos=%d duur=%d pl=%zu rc=%zu rij=%zu\n", x.bron, x.volume, x.sub,
      x.titel.c_str(), x.repeat, x.shuffle, x.pos, x.duur, x.playlists.size(), x.recent.size(), nf::d().rij.size());
    for (auto &q : nf::d().rij) std::printf("[test] rij %c %s\n", q.soort, q.url.c_str());
    zeigen_seite(S_AUDIO);
  }
  else if (stappen[stap].actie == -6) {   // letterwiel: "mid" met de pijlen en OK
    zeigen_seite(S_ZOEK);
    for (char c : std::string("mid")) { while (audio::wiel_teken_() != c) toets(35); toets(34); }
    toets(35); toets(35);
  }
  else if (stappen[stap].actie == -8) { actie(ZOEK_INVOER); }
  else if (stappen[stap].actie == -7) actie(ZOEK_GA);
  else if (stappen[stap].actie == -4) { actie(SP_LIJST + 1); actie(AUDIO_BRON + 3); actie(SP_SHUFFLE); toets(2); toets(11); }
  else actie(stappen[stap].actie);
  if (stappen[stap].actie == LICHT) actie(LAMP + 0);
  stap++;
}

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--demo")) g_demo = true;
    if (!strcmp(argv[i], "--en")) english() = true;   // English screens
  }
  lv_init();
  lv_display_t *disp = lv_sdl_window_create(240, 320);
  lv_sdl_window_set_zoom(disp, 2);
  lv_sdl_window_set_title(disp, "NOIR-simulator (ESPHomeRemote)");
  lv_indev_t *muis = lv_sdl_mouse_create();
  (void) muis;
  lv_indev_t *kb = lv_sdl_keyboard_create();
  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
  lv_obj_set_style_pad_all(scr, 12, 0);   // page_home van de firmware heeft padding (st_screen)
  lv_obj_set_style_pad_top(scr, 10, 0);

  // toetsen: een onzichtbaar object in een groep vangt ze op
  lv_group_t *g = lv_group_create();
  lv_obj_t *vanger = lv_obj_create(lv_layer_sys());
  lv_obj_set_size(vanger, 0, 0);
  lv_obj_add_event_cb(vanger, toets_cb, LV_EVENT_KEY, nullptr);
  lv_group_add_obj(g, vanger);
  lv_indev_set_group(kb, g);

  // buitenwereld
  ha() = ha_stub;
  tv::entiteit() = "media_player.lg_tv";
  kef::entiteit() = "media_player.kef_lsx_ii";
  kef::melding() = [](const char *t) { toast(t); };
  airco::zender() = [](const std::string &j) { std::printf("[IR] %s\n", j.c_str()); };
  st().actie = actie;
  st().aan = true;
  deserializeJson(g_thuis, THUIS_JSON);
  deserializeJson(g_scenes, english() ? SCENES_JSON_EN : SCENES_JSON);
  st().items = []() { return g_thuis.as<JsonArrayConst>(); };
  st().scenes_items = []() { return g_scenes.as<JsonArrayConst>(); };
  lampen_uit_tekst(english() ? LAMPEN_EN : LAMPEN);
  for (auto &l : lampen()) { l.state = "off"; l.bri = -1; }
  netflix_voorbeeld();
  audio_voorbeeld();

  Fonts f{&inter12, &inter14, &inter16, &mdi20, &inter20, &inter48};
  Beelden bl;
  bl.wall = &wall;
  bl.tv = &ic_tv; bl.audio = &ic_audio; bl.licht = &ic_light; bl.scenes = &ic_scenes;
  sim_logos();
  tv_apps() = {{"PS5", "PS5 Gameconsole", "playstation"}, {"Netflix", "Netflix", "netflix"}, {"YouTube", "YouTube", "youtube"},
               {"Xbox", "HDMI 3", "game"}, {"Live TV", "Live TV", "tv"}, {"Spotify", "Spotify", "spotify"}};
  st().toast_font = &inter14;
  bouwen(scr, f, bl, g_thuis.as<JsonArrayConst>());
  tv_status("off", "");
  zeigen(true);
  zeigen_seite(S_THUIS);

  lv_timer_create(klok_cb, 1000, nullptr);
  klok_cb(nullptr);
  if (g_demo) lv_timer_create(demo_cb, 400, nullptr);
  std::printf("NOIR-simulator: muis = vinger, pijlen/Enter/Backspace = D-pad/OK/Terug, H = Home,\n"
              "spatie/S/,/. = play/stop/spoelen, +/-/M = volume, P = schermafdruk, Q = stoppen.\n");

  for (;;) {
    const uint32_t wacht = lv_timer_handler();
    lv_delay_ms(wacht < 5 ? wacht : 5);
  }
}
