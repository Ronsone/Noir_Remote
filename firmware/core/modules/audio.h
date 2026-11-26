#pragma once
#include "core.h"
// Module Audio: one speaker in Home Assistant - on/off, its sources, volume, subwoofer and what is playing.
// Configuration (NOIR Builder > Audio):
//   audio.speaker   media_player of the speaker (on/off, source, volume)
//   audio.name      shown on the screen ("KEF LSX II")
//   audio.sources   [{"name":"TV","source":"tv"}, ...]  (up to 6 buttons)
//   audio.music     media_player for "now playing" and the media keys (empty = the speaker)
//   audio.sub       number entity of the subwoofer level in dB (empty = no subwoofer slider)
// Keys on the Audio screen: play/pause, next, previous -> the music player.
namespace noirapp {

struct Audio : Module {
  std::string sig;
  const char *name() const override { return "audio"; }

  static bool on_screen() { return noir::st().seite == noir::S_AUDIO; }

  void configure(JsonObjectConst cfg) override {
    JsonObjectConst a = cfg["audio"];
    noir::audio::Data &x = noir::audio::d();
    x.kef = a["speaker"] | "";
    x.muziek = a["music"] | "";
    if (x.muziek.empty()) x.muziek = x.kef;
    x.sub_e = a["sub"] | "";
    if (!x.kef.empty()) noir::kef::entiteit() = x.kef;   // the volume keys control this speaker
    // sources and name: rebuild the screens only when they changed
    std::vector<std::string> names, ids;
    std::string s = std::string(a["name"] | "") + "|" + x.sub_e + ";";
    for (JsonObjectConst b : a["sources"].as<JsonArrayConst>()) {
      if ((int) names.size() >= noir::audio::BRONNEN) break;
      names.push_back(b["name"] | "");
      ids.push_back(b["source"] | "");
      s += names.back() + "|" + ids.back() + ";";
    }
    if (s != sig) {
      sig = s;
      noir::audio::bron_naam() = names;
      noir::audio::bron_id() = ids;
      noir::audio::speaker_naam() = a["name"] | "Speaker";
      if (app().built) noir::alles_herbouwen();
    }
    // states of speaker, music player and subwoofer while the Audio screen is open
    poll_set("audio", {[]() { return on_screen() && screen_on() && !noir::audio::d().kef.empty(); },
                       noir::audio::entiteiten_,
                       [](JsonDocument &doc) { noir::audio::lees_(doc); }});
  }

  static void media(const char *svc) {
    const std::string &m = noir::audio::d().muziek;
    if (!m.empty()) ha(svc, {{"entity_id", m}});
  }

  bool action(int a) override {
    if (a == noir::AUDIO) {
      if (noir::audio::d().kef.empty()) { noir::toast("Choose your speaker in NOIR Builder > Audio"); return true; }
      noir::zeigen_seite(noir::S_AUDIO);
      return true;
    }
    if (a >= noir::AUDIO_BRON && a < noir::AUDIO_BRON + noir::audio::BRONNEN) { noir::audio::bron_kiezen(a - noir::AUDIO_BRON); return true; }
    if (a == noir::AUDIO_AANUIT) { noir::audio::aanuit(); return true; }
    if (a == noir::AUDIO_NU) { media("media_player.media_play_pause"); return true; }   // tap "now playing"
    return false;
  }

  bool key(int k, bool pressed) override {
    if (!on_screen()) return false;
    if (!pressed) return k == 2 || k == 11 || k == 4;
    if (k == 2) { media("media_player.media_play_pause"); return true; }
    if (k == 11) { media("media_player.media_next_track"); return true; }
    if (k == 4) { media("media_player.media_previous_track"); return true; }
    return false;
  }

  void all_off() override {
    const std::string &e = noir::audio::d().kef;
    if (!e.empty()) ha("media_player.turn_off", {{"entity_id", e}});
  }
};

}  // namespace noirapp
