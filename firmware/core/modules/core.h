#pragma once
// NOIR Remote - core: shared state, module registry, Home Assistant calls, infrared, configuration download.
//
// Every feature is a module (modules/*.h). A module gets its part of the configuration, handles its own
// screen actions and keys, and may do something every 100 ms. Adding a feature = one new module file + one
// line in noir-app.h. The screens themselves are drawn by noir-ui.h.

#include <map>
#include <string>
#include <vector>
#include "driver/gpio.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esphome/components/api/custom_api_device.h"
// A board with infrared declares the script ir_protocols (transmit_nec) next to its remote_transmitter; ESPHome then
// defines USE_REMOTE_PROTOCOL_NEC. (It does not define USE_REMOTE_TRANSMITTER - that guard silently removed all
// infrared in 0.7.3/0.7.4.) Boards without infrared build the IR calls as no-ops.
#include "esphome/core/defines.h"
#if defined(USE_REMOTE_PROTOCOL_NEC)
#define NOIR_HAS_IR 1
#include "esphome/components/remote_base/nec_protocol.h"
#endif

namespace noirapp {

// ---- module interface ----------------------------------------------------------------------------------------
struct Module {
  virtual ~Module() = default;
  virtual const char *name() const = 0;
  virtual void configure(JsonObjectConst cfg) {}        // whole configuration, after every (re)load
  virtual bool action(int a) { return false; }          // a screen action (tap); true = handled
  virtual bool key(int k, bool pressed) { return false; }   // a hardware key; true = handled
  virtual void tick(uint32_t now) {}                    // every 100 ms
  virtual void all_off() {}                             // "All off" scene
};
inline std::vector<Module *> &modules() { static std::vector<Module *> m; return m; }

// ---- shared state --------------------------------------------------------------------------------------------
struct App {
  std::string host, key;
  JsonDocument cfg;            // current configuration (screens refer into it - keep it alive)
  uint32_t last_input{0};      // last key/touch (ms)
  bool wake{false};            // display should wake up
  bool built{false};           // screens exist
  bool eco{false};             // mode: false = Full (always ready), true = Eco (longer battery)
};
inline App &app() { static App a; return a; }
inline uint32_t idle_ms() { return millis() - app().last_input; }
inline bool wake_flag() { bool w = app().wake; app().wake = false; return w; }
inline uint32_t dim_s() { return app().eco ? 10 : 30; }   // display timeouts per mode (seconds)
inline uint32_t off_s() { return app().eco ? 20 : 60; }
inline bool screen_on() { return idle_ms() < off_s() * 1000; }
inline JsonArrayConst list_of(const char *name) { return app().cfg[name].as<JsonArrayConst>(); }

// ---- extra states in the lamp poll (/states): every module can add its entities --------------------------------
// The screens have one "extra" slot (licht_http::extra_*); this shares it between modules (TV, Audio, ...).
struct Poll {
  std::function<bool()> open;                    // poll now? (e.g. its screen is visible)
  std::function<std::string()> entities;         // "a,b,"
  std::function<void(JsonDocument &)> read;      // states arrived
};
inline std::map<std::string, Poll> &polls() { static std::map<std::string, Poll> p; return p; }
inline void poll_set(const std::string &name, Poll p) {
  polls()[name] = std::move(p);
  noir::lights_http::extra_open() = []() {
    for (auto &kv : polls()) if (kv.second.open && kv.second.open()) return true;
    return false;
  };
  noir::lights_http::extra_e() = []() {
    std::string e;
    for (auto &kv : polls()) if (kv.second.open && kv.second.open() && kv.second.entities) e += kv.second.entities();
    return e;
  };
  noir::lights_http::extra_read() = [](JsonDocument &doc) {
    for (auto &kv : polls()) if (kv.second.read) kv.second.read(doc);
  };
}

// ---- Home Assistant ------------------------------------------------------------------------------------------
inline void ha(const std::string &svc, const std::map<std::string, std::string> &data) {
  static esphome::api::CustomAPIDevice dev;
  if (esphome::api::global_api_server == nullptr || !esphome::api::global_api_server->is_connected()) {
    ESP_LOGW("noir", "No Home Assistant connection - %s skipped", svc.c_str());
    noir::toast("Home Assistant not connected", 2000);
    return;
  }
  dev.call_homeassistant_service(svc, data);
}

// ---- infrared ------------------------------------------------------------------------------------------------
// Brand-free: protocol + address come from the TV profile (NOIR Builder), the module only sends commands.
// Supported now: "nec" (LG, many others). A new protocol = one more branch here + profiles that use it.
inline void ir_nec(int address, int cmd, int repeats = 0) {
  if (address < 0 || cmd < 0) return;
#ifdef NOIR_HAS_IR
  auto call = ir_tx->transmit();
  esphome::remote_base::NECData d;
  d.address = (uint16_t) ((address & 0xFF) | ((~address & 0xFF) << 8));
  d.command = (uint16_t) ((cmd & 0xFF) | ((~cmd & 0xFF) << 8));
  d.command_repeats = 1;
  esphome::remote_base::NECProtocol().encode(call.get_data(), d);
  call.set_send_times(1 + repeats);
  call.perform();
#endif
}
// Raw timings (us, + = mark, - = space) at a carrier, e.g. airco frames that carry the whole state
inline void ir_raw(const std::vector<int32_t> &timings, uint32_t carrier = 38000) {
  if (timings.empty()) return;
#ifdef NOIR_HAS_IR
  auto call = ir_tx->transmit();
  call.get_data()->set_carrier_frequency(carrier);
  call.get_data()->set_data(timings);
  call.perform();
#endif
}
struct Ir {
  std::string protocol;
  int address{-1};
  bool ok() const { return !protocol.empty() && address >= 0; }
  void send(int cmd) const {
    if (!ok() || cmd < 0) return;
    if (protocol == "nec") ir_nec(address, cmd);
    else ESP_LOGW("noir", "Infrared protocol '%s' not supported yet", protocol.c_str());
  }
};
// NOIR Builder "Test" button (ESPHome action noir_ir)
inline void ir_test(const std::string &protocol, int address, int cmd) {
  ESP_LOGI("noir", "IR test %s 0x%02X 0x%02X", protocol.c_str(), address & 0xFF, cmd & 0xFF);
  Ir{protocol, address}.send(cmd);
}

// ---- dispatch ------------------------------------------------------------------------------------------------
inline void action(int a) {
  for (Module *m : modules()) if (m->action(a)) return;
  ESP_LOGD("noir", "Action %d not handled", a);
}
inline void all_off() {
  for (Module *m : modules()) m->all_off();
  noir::toast("All off");
}

// ---- mode: Full (always ready) or Eco (longer battery) ----------------------------------------------------------
inline void set_mode(bool eco) {
  App &a = app();
  noir::eco_mode() = eco;
  if (eco == a.eco) return;
  a.eco = eco;
  esp_wifi_set_ps(eco ? WIFI_PS_MIN_MODEM : WIFI_PS_NONE);   // Eco: Wi-Fi sleeps between packets
  ESP_LOGI("noir", "Mode: %s", eco ? "Eco" : "Full");
}

// ---- configuration -------------------------------------------------------------------------------------------
inline void apply_config() {
  App &a = app();
  set_mode(strcmp(a.cfg["mode"] | "full", "eco") == 0);
  // language: nl = built in, en = built in, other = pack (cfg.texts) with English as fallback
  static std::string lang = "nl";
  const std::string l = a.cfg["lang"] | "nl";
  if (l != lang) {
    lang = l;
    noir::english() = l != "nl";
    noir::texts().clear();
    for (JsonPairConst kv : a.cfg["texts"].as<JsonObjectConst>()) noir::texts()[kv.key().c_str()] = kv.value() | "";
    if (a.built) noir::rebuild_all();   // every screen in the new language
  }
  JsonObjectConst cfg = a.cfg.as<JsonObjectConst>();
  for (Module *m : modules()) m->configure(cfg);
  noir_config_version->publish_state(a.cfg["version"] | "?");
}
inline bool read_config(const char *data, size_t len) {
  JsonDocument fresh;
  if (deserializeJson(fresh, data, len) || !fresh.is<JsonObject>()) {
    ESP_LOGW("noir", "Configuration not readable (%u bytes)", (unsigned) len);
    return false;
  }
  app().cfg = std::move(fresh);
  apply_config();
  ESP_LOGI("noir", "Configuration loaded (%u bytes)", (unsigned) len);
  return true;
}
inline void nvs_save(const char *data, size_t len) {
  nvs_handle_t h;
  if (nvs_open("noir", NVS_READWRITE, &h) != ESP_OK) return;
  nvs_set_blob(h, "cfg", data, len);
  nvs_commit(h);
  nvs_close(h);
}
inline void nvs_load() {
  nvs_handle_t h;
  if (nvs_open("noir", NVS_READONLY, &h) != ESP_OK) return;
  size_t len = 0;
  if (nvs_get_blob(h, "cfg", nullptr, &len) == ESP_OK && len > 0 && len < 64 * 1024) {
    char *buf = (char *) heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
    if (buf != nullptr && nvs_get_blob(h, "cfg", buf, &len) == ESP_OK) read_config(buf, len);
    if (buf != nullptr) heap_caps_free(buf);
  }
  nvs_close(h);
}

// Download in its own task (8 KB stack - esp_http_client needs it)
struct Download { volatile bool busy{false}, done{false}; char *buf{nullptr}; size_t len{0}; int status{0}; void *task{nullptr}; };
inline Download &dl() { static Download d; return d; }
inline void download_task_(void *) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    Download &d = dl();
    const std::string url = "http://" + app().host + "/api/noir_remote/config?k=" + app().key;
    esp_http_client_config_t c = {};
    c.url = url.c_str();
    c.timeout_ms = 6000;
    c.buffer_size = 1024;
    c.buffer_size_tx = 1024;
    esp_http_client_handle_t h = esp_http_client_init(&c);
    int status = -1;
    size_t len = 0;
    char *buf = nullptr;
    if (h != nullptr && esp_http_client_open(h, 0) == ESP_OK) {
      esp_http_client_fetch_headers(h);
      status = esp_http_client_get_status_code(h);
      if (status == 200) {
        const size_t MAX = 60 * 1024;
        buf = (char *) heap_caps_malloc(MAX, MALLOC_CAP_SPIRAM);
        while (buf != nullptr && len < MAX) {
          const int r = esp_http_client_read(h, buf + len, MAX - len);
          if (r <= 0) break;
          len += r;
        }
      }
    }
    if (h != nullptr) { esp_http_client_close(h); esp_http_client_cleanup(h); }
    d.buf = buf; d.len = len; d.status = status; d.done = true;
  }
}
inline void fetch_config() {
  Download &d = dl();
  if (d.busy || app().host.empty() || app().key.empty()) return;
  if (d.task == nullptr) {
    static StaticTask_t tcb;
    const size_t size = 8192;
    auto *stack = (StackType_t *) heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (stack != nullptr) d.task = xTaskCreateStaticPinnedToCore(download_task_, "noir_cfg", size, nullptr, 1, stack, &tcb, tskNO_AFFINITY);
  }
  if (d.task == nullptr) return;
  d.busy = true;
  d.done = false;
  xTaskNotifyGive((TaskHandle_t) d.task);
}
inline void download_check_() {
  Download &d = dl();
  if (!d.done) return;
  d.done = false;
  d.busy = false;
  if (d.status == 200 && d.buf != nullptr && d.len > 0) {
    if (read_config(d.buf, d.len)) { nvs_save(d.buf, d.len); noir::toast("Screens updated"); }
  } else {
    ESP_LOGW("noir", "Configuration download failed (HTTP %d)", d.status);
  }
  if (d.buf != nullptr) heap_caps_free(d.buf);
  d.buf = nullptr;
}

}  // namespace noirapp
