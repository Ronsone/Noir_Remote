#pragma once
#include "core.h"
// Module Info: the weather, calendar and value tiles on Home get their data from Home Assistant.
// The tiles are items with t = "weather" | "calendar" | "value" and an entity (NOIR Builder > Home). While Home is on
// screen their entities ride along in the states poll as "i:<entity>"; NOIR Remote (info.py) answers with a small
// ready-made object, at most once a minute fresh. Home is only rebuilt when something changed.
namespace noirapp {

struct Info : Module {
  std::vector<std::string> ents;
  std::map<std::string, std::string> seen;   // entity -> last answer (to see changes)
  const char *name() const override { return "info"; }

  void configure(JsonObjectConst) override {
    ents.clear();
    for (JsonObjectConst it : list_of("home")) {
      const std::string t = it["t"] | "", e = it["entity"] | "";
      if ((t == "weather" || t == "calendar" || t == "value") && !e.empty()) ents.push_back(e);
    }
    poll_set("info", {[this]() { return !ents.empty() && noir::st().cur_screen == noir::S_HOME && screen_on(); },
                      [this]() { std::string s; for (const std::string &e : ents) s += "i:" + e + ","; return s; },
                      [this](JsonDocument &doc) { read(doc); }});
  }

  void read(JsonDocument &doc) {
    bool changed = false;
    for (const std::string &e : ents) {
      JsonObjectConst o = doc["i:" + e];
      if (o.isNull()) continue;
      std::string raw;
      serializeJson(o, raw);
      if (seen[e] == raw) continue;
      seen[e] = raw;
      changed = true;
      const std::string d = e.substr(0, e.find('.'));
      if (d == "weather") weather(e, o);
      else if (d == "calendar") calendar(e, o);
      else value(e, o);
    }
    if (changed && app().built) { noir::home_rebuild(list_of("home")); noir::airco::show(); }
  }

  static void weather(const std::string &e, JsonObjectConst o) {
    noir::info::Weather w;
    w.known = true;
    w.cond = o["c"] | "";
    w.place = o["p"] | "";
    w.temp = o["t"] | 0; w.lo = o["lo"] | 0; w.hi = o["hi"] | 0;
    for (JsonArrayConst h : o["h"].as<JsonArrayConst>()) w.hours.push_back({h[0] | "", h[1] | 0});
    for (JsonArrayConst x : o["d"].as<JsonArrayConst>()) w.days.push_back({x[0] | "", x[1] | 0, x[2] | 0});
    noir::info::weather()[e] = w;
  }
  static void calendar(const std::string &e, JsonObjectConst o) {
    std::vector<noir::info::Event> ev;
    for (JsonArrayConst x : o["ev"].as<JsonArrayConst>()) ev.push_back({x[0] | "", x[1] | "", x[2] | "", x[3] | ""});
    noir::info::calendar()[e] = ev;
  }
  static void value(const std::string &e, JsonObjectConst o) {
    noir::info::Value v;
    v.known = true;
    v.name = o["n"] | ""; v.text = o["v"] | ""; v.change = o["ch"] | ""; v.trend = o["tr"] | 0;
    for (float f : o["hs"].as<JsonArrayConst>()) v.history.push_back(f);
    noir::info::values()[e] = v;
  }
};

}  // namespace noirapp
