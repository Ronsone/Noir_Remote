// NOIR on your phone (proof of concept) - sidebar "NOIR".
// The remote's own screen code (simulator/, built to WebAssembly in web/) runs here in a canvas. Home Assistant gives it
// its configuration like a remote gets it (websocket noir_remote/phone: own home screen "noir-phone" in the NOIR Builder),
// the states of its lamps and TV, and runs the actions it sends with your own rights.
const BASE = "/noir_remote_static/web/";

class NoirPhone extends HTMLElement {
  constructor() {
    super();
    this._sig = "";   // no shadow DOM: Emscripten's SDL finds the canvas and its touch events in the page
  }
  set narrow(n) { this._narrow = n; }
  set panel(p) { this._panel = p; }
  set hass(h) {
    this._hass = h;
    if (!this._started) { this._started = true; this._start(); return; }
    this._states();
  }

  async _start() {
    this.innerHTML = `<style>
      noir-phone { display:block; height:100%; min-height:100vh; background:#07080a; position:relative; overflow:hidden; }
      noir-phone .top
      { position:absolute; left:4px; top:4px; z-index:2; color:#e4e7ec; }
      noir-phone canvas { display:block; margin:0 auto; touch-action:none; outline:none; }
      noir-phone .msg { color:#969aa3; font:14px Inter, Roboto, sans-serif; padding:24px; }
    </style><div class="top">${this._narrow ? "<ha-menu-button></ha-menu-button>" : ""}</div>
      <canvas id="canvas" tabindex="-1"></canvas><div class="msg"></div>`;
    const mb = this.querySelector("ha-menu-button");
    if (mb) { mb.hass = this._hass; mb.narrow = true; }
    const canvas = this.querySelector("canvas"), msg = this.querySelector(".msg");
    // the screen: the panel's own size in CSS pixels (the layout picks columns and scale from it, like on a wall panel).
    // Home Assistant can give us hass before the panel is laid out - wait until it has a size (at most 2 s).
    for (let i = 0; i < 40 && !(this.clientWidth > 0 && this.clientHeight > 0); i++) await new Promise((r) => setTimeout(r, 50));
    const w = Math.max(240, Math.min(this.clientWidth || window.innerWidth, 1280));
    const h = Math.max(320, Math.min(this.clientHeight || window.innerHeight, 1280));
    canvas.style.width = `${w}px`;
    canvas.style.height = `${h}px`;
    console.info(`NOIR: panel ${this.clientWidth}x${this.clientHeight}, screen ${w}x${h}, pixel ratio ${window.devicePixelRatio}`);
    try {
      if (!window.NoirSim) await new Promise((ok, bad) => {
        const s = document.createElement("script");
        s.src = BASE + "noir-simulator.js"; s.onload = ok; s.onerror = bad;
        document.head.appendChild(s);
      });
      this._m = await window.NoirSim({
        canvas, arguments: ["--size", `${w}x${h}`],
        locateFile: (f) => BASE + f,
        noirHa: (svc, data) => {   // an action from the screens -> Home Assistant
          const [domain, service] = svc.split(".");
          this._hass.callService(domain, service, data).catch((e) => console.warn("NOIR:", svc, e));
          setTimeout(() => this._states(true), 600);
        },
      });
    } catch (e) {
      msg.textContent = `NOIR could not start: ${e?.message || e}`;
      return;
    }
    await this._config(w, h);
    this._timer = setInterval(() => this._config(w, h), 60000);   // new screens, weather, calendar, values
    // the browser can wipe the canvas when the panel is shown, resized or comes back: draw everything again then
    const redraw = () => { try { this._m.ccall("noir_web_redraw", null, [], []); } catch (e) { /* not started */ } };
    this._ro = new ResizeObserver(redraw);
    this._ro.observe(canvas);
    this._vis = () => { if (!document.hidden) redraw(); };
    document.addEventListener("visibilitychange", this._vis);
    for (const t of [100, 500, 1500]) setTimeout(redraw, t);
  }

  async _config(w, h) {
    try {
      this._cfg = await this._hass.callWS({ type: "noir_remote/phone", w, h });
      this._m.ccall("noir_web_config", null, ["string"], [JSON.stringify(this._cfg)]);
      this._sig = "";
      this._states(true);
    } catch (e) {
      this.querySelector(".msg").textContent = `NOIR: ${e?.message || e}`;
    }
  }

  // lamps and the TV: their state as the remote's /states gives it ([state, brightness, source])
  _states(force) {
    if (!this._m || !this._cfg) return;
    const st = this._hass.states, out = {};
    const ents = [...(this._cfg.lights || []).map((l) => l.entity), this._cfg.tv?.entity].filter(Boolean);
    for (const e of ents) {
      const s = st[e];
      if (s) out[e] = [s.state, typeof s.attributes.brightness === "number" ? s.attributes.brightness : -1, s.attributes.source || ""];
    }
    const sig = JSON.stringify(out);
    if (!force && sig === this._sig) return;
    this._sig = sig;
    this._m.ccall("noir_web_states", null, ["string"], [sig]);
  }

  connectedCallback() { if (this._m) setTimeout(() => { try { this._m.ccall("noir_web_redraw", null, [], []); } catch (e) { /* */ } }, 50); }
  disconnectedCallback() {
    clearInterval(this._timer);
    this._ro?.disconnect();
    if (this._vis) document.removeEventListener("visibilitychange", this._vis);
  }
}
if (!customElements.get("noir-phone")) customElements.define("noir-phone", NoirPhone);
