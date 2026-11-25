// NOIR Builder - sidebar panel of the NOIR Remote integration.
// One tab per screen, live preview on the left, simple rows on the right. Save = the remote updates in seconds.

const TABS = [["home", "Home", "mdi:home-outline"], ["lights", "Lights", "mdi:lightbulb-outline"],
  ["scenes", "Scenes", "mdi:layers-outline"], ["tv", "TV", "mdi:television"], ["remote", "Remote", "mdi:remote"]];
const OPENS = [["tv", "TV screen"], ["lights", "Lights screen"], ["scenes", "Scenes screen"], ["alloff", "All off"],
  ["tv_power", "TV on/off"], ["settings", "Remote settings"], ["toggle", "Toggle an entity"], ["run", "Run a scene or script"],
  ["audio", "Audio (coming soon)"], ["netflix", "Netflix (coming soon)"], ["airco", "Airco (coming soon)"]];
const OPENS_ICON = { tv: "noir:tv", lights: "noir:licht", scenes: "noir:scenes", alloff: "mdi:power", tv_power: "mdi:power",
  settings: "mdi:cog", toggle: "mdi:lightbulb", run: "mdi:play", audio: "noir:audio", netflix: "mdi:netflix", airco: "mdi:air-conditioner" };
const OPENS_LABEL = { tv: "TV", lights: "Lights", scenes: "Scenes", alloff: "All off", tv_power: "TV", settings: "Settings",
  audio: "Audio", netflix: "Netflix", airco: "Airco" };
const ICONS = ["noir:tv", "noir:audio", "noir:licht", "noir:scenes", "mdi:power", "mdi:lightbulb", "mdi:floor-lamp", "mdi:ceiling-light",
  "mdi:lamp", "mdi:led-strip", "mdi:string-lights", "mdi:sofa", "mdi:bed", "mdi:movie-open", "mdi:music", "mdi:television",
  "mdi:sony-playstation", "mdi:nintendo-switch", "mdi:netflix", "mdi:youtube", "mdi:play", "mdi:weather-night", "mdi:white-balance-sunny",
  "mdi:silverware-fork-knife", "mdi:coffee", "mdi:fan", "mdi:air-conditioner", "mdi:thermometer", "mdi:blinds", "mdi:window-shutter",
  "mdi:door", "mdi:lock", "mdi:home", "mdi:cog", "mdi:heart", "mdi:star", "mdi:leaf", "mdi:flash", "mdi:speaker"];
const INSTALL_URL = "https://ronsone.github.io/Noir_Remote/";
const LOGO_URL = (k) => `/noir_remote_static/logos/${k}.png`;
const NL = {"Home": "Home", "Lights": "Verlichting", "Scenes": "Scènes", "TV": "TV", "Remote": "Remote", "TV screen": "TV", "Lights screen": "Verlichting", "Scenes screen": "Scènes", "All off": "Alles uit", "TV on/off": "TV aan/uit", "Remote settings": "Instellingen remote", "Toggle an entity": "Iets aan/uit zetten", "Run a scene or script": "Scène of script starten", "Audio (coming soon)": "Audio (binnenkort)", "Netflix (coming soon)": "Netflix (binnenkort)", "Airco (coming soon)": "Airco (binnenkort)", "Tiles on your home screen": "Tegels op je startscherm", "Add tile": "Tegel toevoegen", "Add light": "Lamp toevoegen", "Add scene": "Scène toevoegen", "Name": "Naam", "Opens": "Opent", "Entity": "Entiteit", "Size": "Maat", "Half": "Half", "Full": "Vol", "Dimmable": "Dimbaar", "On/off": "Aan/uit", "Scene or script": "Scène of script", "Also used by": "Ook gebruikt door", "App tiles": "App-tegels", "Your TV": "Je tv", "Speaker (on with the TV, volume keys)": "Speaker (aan met de tv, volumetoetsen)", "Advanced · infrared codes (LG is filled in)": "Geavanceerd · infraroodcodes (LG staat ingevuld)", "Address": "Adres", "Command": "Commando", "Power on": "Aan", "Power off": "Uit", "Power toggle": "Aan/uit", "Mode": "Modus", "Language": "Taal", "Always ready": "Altijd klaar", "Longer battery": "Langer op de accu", "First-time setup": "Eerste keer instellen", "Copy": "Kopiëren", "Copied": "Gekopieerd", "Save": "Opslaan", "Unsaved changes": "Niet opgeslagen", "Saved – the remote updates in a few seconds.": "Opgeslagen – de remote past zich zo aan.", "What you see left is what your remote shows.": "Links zie je wat je remote laat zien.", "Tap a tile to edit it": "Tik op een tegel om hem aan te passen", "Tap = on/off · hold = dim": "Tikken = aan/uit · vasthouden = dimmen", "New tile": "Nieuwe tegel", "Up": "Omhoog", "Down": "Omlaag", "Remove": "Verwijderen", "Icon": "Icoon", "TV input / app": "Tv-ingang / app", "Settings": "Instellingen", "Loading…": "Laden…", "Copy into your ESPHome": "Kopieer naar je ESPHome", "Your remote YAML:": "YAML van je remote:", "Flash via USB, then add it in Home Assistant (ESPHome) – it asks for": "Flash via USB en voeg hem toe in Home Assistant (ESPHome) – die vraagt om",
  "Your TV in Home Assistant": "Je tv in Home Assistant", "Switching on and keys": "Aanzetten en toetsen", "No infrared": "Geen infrarood", "Everything via Home Assistant": "Alles via Home Assistant", "Test": "Testen", "On": "Aan", "Off": "Uit", "Sent – did your TV react?": "Verstuurd – reageerde je tv?", "Brand not listed? See docs/tv.md – one small file adds it.": "Merk staat er niet bij? Zie docs/tv.md – één klein bestand voegt het toe.", "Speaker (optional)": "Speaker (optioneel)", "Switches on with the TV, gets the volume keys.": "Gaat aan met de tv en krijgt de volumetoetsen.", "Sources on your remote": "Bronnen op je remote", "Tap one below to add it. Up to 8.": "Tik hieronder op een bron om hem toe te voegen. Maximaal 8.", "Add": "Toevoegen", "Choose your TV first – its inputs and apps appear here.": "Kies eerst je tv – zijn ingangen en apps verschijnen hier.", "Add by hand": "Zelf toevoegen", "Logo": "Logo", "No logo": "Geen logo", "Brands": "Merken", "General": "Algemeen", "Source": "Bron", "No sources yet": "Nog geen bronnen", "tested": "getest", "— none —": "— geen —", "Save as": "Bewaar als", "and fill in your Wi-Fi": "en vul je wifi in", "Connect the remote by USB and run in the firmware folder:": "Sluit de remote aan via USB en voer uit in de map firmware:", "Home Assistant finds the remote (ESPHome) – it asks for": "Home Assistant vindt de remote (ESPHome) – die vraagt om", "Your Wi-Fi (only used for this file, not stored):": "Je wifi (alleen voor dit bestand, wordt niet opgeslagen):", "Wi-Fi name": "Wifi-naam", "Wi-Fi password": "Wifi-wachtwoord", "Download secrets.yaml": "Download secrets.yaml", "Put the file in the folder": "Zet het bestand in de map", "Show the text": "Toon de tekst", "Install the firmware from your browser (Chrome or Edge): connect the remote by USB and press Install.": "Installeer de firmware vanuit je browser (Chrome of Edge): sluit de remote aan via USB en druk op Installeren.", "Open the installer": "Open de installer", "Connect your phone to the Wi-Fi network “noir-remote setup” and choose your own Wi-Fi.": "Verbind je telefoon met het wifi-netwerk “noir-remote setup” en kies je eigen wifi.", "Done: Home Assistant adds the remote by itself – no keys, no files.": "Klaar: Home Assistant voegt de remote vanzelf toe – geen sleutels, geen bestanden.", "Firmware": "Firmware", "No update found on your remote yet. Remotes with firmware 0.2 install the new firmware once via the installer (USB); after that, updates come here with one button.": "Nog geen update gevonden op je remote. Een remote met firmware 0.2 installeert de nieuwe firmware één keer via de installer (USB); daarna komen updates hier, met één knop.", "Updating…": "Bijwerken…", "keep the remote close to your Wi-Fi.": "houd de remote in de buurt van je wifi.", "New firmware": "Nieuwe firmware", "now": "nu", "Up to date": "Up-to-date", "Update firmware": "Firmware bijwerken", "Started – the remote restarts when it is done.": "Gestart – de remote herstart als hij klaar is.", "Switch your TV on – its inputs and apps then appear here.": "Zet je tv aan – dan verschijnen zijn ingangen en apps hier.", "Done: the remote is added to Home Assistant (ESPHome) by itself, about a minute after it starts.": "Klaar: de remote wordt vanzelf aan Home Assistant (ESPHome) toegevoegd, ongeveer een minuut nadat hij start."};
let LANG = "nl";
const L = (en) => (LANG === "nl" && NL[en]) || en;
const esc = (s) => String(s ?? "").replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
const haIcon = (i) => ({ "noir:tv": "mdi:television", "noir:audio": "mdi:waveform", "noir:licht": "mdi:floor-lamp-outline", "noir:scenes": "mdi:layers-outline" }[i] || i || "mdi:square-rounded-outline");

const CSS = `
:host { display:block; min-height:100vh; background:var(--primary-background-color); color:var(--primary-text-color); font-family:var(--paper-font-body1_-_font-family, Roboto, sans-serif); }
.wrap { max-width:1200px; padding:20px 24px; }
h1 { margin:0 0 2px; font-size:24px; font-weight:600; } .sub { margin:0 0 16px; color:var(--secondary-text-color); }
.tabs { display:flex; gap:6px; flex-wrap:wrap; margin-bottom:16px; }
.tab { display:inline-flex; align-items:center; gap:7px; padding:8px 15px; border-radius:999px; border:1px solid var(--divider-color); background:var(--card-background-color); color:var(--primary-text-color); font:inherit; cursor:pointer; }
.tab ha-icon { --mdc-icon-size:18px; opacity:.8; } .tab.on { background:var(--primary-text-color); color:var(--card-background-color); border-color:var(--primary-text-color); font-weight:600; }
.main { display:flex; gap:26px; align-items:flex-start; flex-wrap:wrap; } .phone { flex:none; display:flex; flex-direction:column; align-items:center; position:sticky; top:8px; }
.hint { color:var(--secondary-text-color); margin-top:8px; font-size:12px; } .ed { flex:1 1 520px; min-width:0; }
.card { border:1px solid var(--divider-color); border-radius:14px; padding:14px 16px; margin-bottom:12px; background:var(--card-background-color); }
.card h3 { margin:0 0 4px; font-size:15px; } .card > p { margin:0 0 10px; color:var(--secondary-text-color); font-size:13px; }
.row { display:grid; gap:8px; align-items:center; padding:6px 0; border-top:1px solid var(--divider-color); }
.row.home { grid-template-columns:52px 1fr 1.3fr 1.3fr 84px 100px; } .row.lights { grid-template-columns:1fr 1.6fr 120px 100px; }
.row.scenes { grid-template-columns:52px 1fr 1.6fr 100px; } .row.apps { grid-template-columns:60px 1fr 1fr 100px; }
.chips { display:flex; flex-wrap:wrap; gap:6px; margin-top:10px; } .chip { display:inline-flex; align-items:center; gap:4px; padding:6px 11px; border-radius:999px; border:1px dashed var(--divider-color); background:transparent; color:var(--primary-text-color); font:inherit; font-size:13px; cursor:pointer; } .chip:disabled { opacity:.4; } .chip ha-icon { --mdc-icon-size:16px; } 
.row.head { border-top:0; font-size:11px; text-transform:uppercase; letter-spacing:.05em; color:var(--secondary-text-color); padding-bottom:0; }
.row.sel { background:rgba(59,130,246,.10); border-radius:10px; box-shadow:inset 3px 0 0 #3b82f6; }
input, select { width:100%; box-sizing:border-box; min-width:0; padding:7px 9px; border-radius:9px; border:1px solid var(--divider-color); background:var(--secondary-background-color, #111); color:var(--primary-text-color); font:inherit; }
input[type=checkbox] { width:auto; }
.wifi { display:grid; grid-template-columns:1fr 1fr; gap:8px; }
.ico { position:relative; display:inline-flex; align-items:center; justify-content:center; width:44px; height:34px; border-radius:9px; border:1px solid var(--divider-color); }
.ico select { position:absolute; inset:0; opacity:0; cursor:pointer; } .ico ha-icon { --mdc-icon-size:20px; }
.acts { display:flex; gap:2px; justify-content:flex-end; } .muted { color:var(--secondary-text-color); }
button.i { width:30px; height:30px; border-radius:8px; border:0; background:transparent; color:var(--primary-text-color); cursor:pointer; display:inline-flex; align-items:center; justify-content:center; }
button.i:disabled { opacity:.3; cursor:default; } button.i ha-icon { --mdc-icon-size:18px; }
.btn { display:inline-flex; align-items:center; gap:6px; padding:8px 14px; border-radius:10px; border:1px solid var(--divider-color); background:transparent; color:var(--primary-text-color); font:inherit; cursor:pointer; }
.btn:disabled { opacity:.45; cursor:default; }
.btn.primary { background:var(--primary-color); border-color:var(--primary-color); color:#fff; font-weight:600; } .btn ha-icon { --mdc-icon-size:18px; }
.bar { display:flex; gap:10px; align-items:center; margin-top:4px; } .note { color:var(--secondary-text-color); font-size:12px; margin:8px 0 0; }
.dirty { color:#f59e0b; font-size:13px; } .ok { color:#22c55e; font-size:13px; }
code { background:rgba(127,127,127,.15); padding:1px 5px; border-radius:5px; }
pre { background:rgba(127,127,127,.12); padding:10px 12px; border-radius:10px; overflow:auto; font-size:12px; }
.noirv { position:relative; width:240px; height:320px; background:radial-gradient(120% 90% at 85% 15%, #2a2c31 0%, #0b0c0f 45%, #07080a 100%); border-radius:10px; overflow:hidden; color:#f2f3f5; font-family:Inter,Roboto,sans-serif; box-shadow:0 0 0 8px #15171b, 0 6px 24px rgba(0,0,0,.5); margin:8px; }
.nvbar { position:absolute; left:12px; right:12px; top:5px; display:flex; font-size:12px; color:#b6bac3; } .nvbar span { flex:1; }
.nvbody { position:absolute; left:12px; top:30px; width:216px; height:236px; overflow:hidden; }
.nvt { position:absolute; left:0; top:5px; font-size:16px; font-weight:500; } .nvs { position:absolute; left:0; top:30px; font-size:12px; color:#969aa3; }
.nvk { position:absolute; box-sizing:border-box; background:linear-gradient(#1a1c20,#101114); border:1px solid rgba(56,60,67,.45); border-radius:13px; display:flex; align-items:center; gap:7px; padding:0 10px; font-size:12px; color:#e4e7ec; overflow:hidden; cursor:pointer; }
.nvk.big { flex-direction:column; align-items:flex-start; justify-content:space-between; padding:8px 12px 7px; border-radius:16px; font-size:14px; }
.nvk b { font-weight:400; white-space:nowrap; overflow:hidden; text-overflow:ellipsis; max-width:100%; } .nvk ha-icon { --mdc-icon-size:20px; opacity:.82; }
.nvk.sel { outline:2px solid #6aa8ff; outline-offset:1px; } .nvk.off { color:#9aa0a8; }
.nvfoot { position:absolute; left:12px; right:12px; top:270px; height:42px; display:flex; gap:8px; }
.nvfoot .nvk { position:relative; flex:1; justify-content:center; background:rgba(16,18,22,.7); border-radius:14px; cursor:default; }
.modes { display:grid; grid-template-columns:1fr 1fr; gap:12px; }
.mode { border:2px solid var(--divider-color); border-radius:16px; padding:18px; cursor:pointer; background:var(--card-background-color); }
.mode.on { border-color:var(--primary-color); background:color-mix(in srgb, var(--primary-color) 10%, var(--card-background-color)); }
.mode ha-icon { --mdc-icon-size:32px; } .mode b { display:block; font-size:17px; margin:8px 0 2px; } .mode span { color:var(--secondary-text-color); font-size:13px; }
details summary { cursor:pointer; color:var(--secondary-text-color); font-size:13px; padding:4px 0; }
@media (max-width:900px) { .row.home { grid-template-columns:48px 1fr 1fr; } .phone { position:static; } }
`;

class NoirBuilder extends HTMLElement {
  constructor() {
    super();
    this.attachShadow({ mode: "open" });
    this.s = { tab: "home", sel: null, model: null, dirty: false, msg: "", device: "" };
    this.shadowRoot.addEventListener("click", (e) => this._click(e));
    this.shadowRoot.addEventListener("change", (e) => this._change(e));
  }
  set hass(h) {
    const first = !this._hass;
    this._hass = h;
    if (first) { this._load(); return; }
    if (!this._painted) { this._render(); return; }
    // TV tab: redraw when the TV switches on/off, so its inputs appear (not while you are typing)
    const e = this.s.tab === "tv" ? this.s.model?.tv?.entity : this.s.tab === "remote" ? this._fwEntity() : "", st = e && h.states[e];
    const at = st?.attributes || {};
    const sig = st ? `${e}|${st.state}|${(at.source_list || []).length}|${at.source || ""}|${at.in_progress}|${at.update_percentage}|${at.installed_version}` : "";
    if (sig !== this._tvSig) {
      this._tvSig = sig;
      if (e && !this.shadowRoot.activeElement) this._render();
    }
  }
  set panel(p) { this._panel = p; }
  set narrow(n) { this._narrow = n; }

  async _load() {
    try {
      const r = await this._hass.callWS({ type: "noir_remote/get" });
      this.s.model = r.model; this.s.device = r.device; this.s.profiles = r.profiles || []; this.s.logos = r.logos || [];
    } catch (e) { this.s.msg = `Could not load: ${e.message || e}`; }
    this._render();
  }
  async _save() {
    try {
      await this._hass.callWS({ type: "noir_remote/save", model: this.s.model });
      this.s.dirty = false; this.s.msg = L("Saved – the remote updates in a few seconds.");
    } catch (e) { this.s.msg = `Save failed: ${e.message || e}`; }
    this._render();
  }
  _touch() { this.s.dirty = true; this.s.msg = ""; }
  async _test(key) {   // the remote sends one code of the chosen profile
    this.s.testErr = false; this.s.testMsg = "…"; this._render();
    try {
      await this._hass.callWS({ type: "noir_remote/test", profile: this.s.model.tv.profile, key });
      this.s.testMsg = L("Sent – did your TV react?");
    } catch (e) { this.s.testErr = true; this.s.testMsg = e.message || String(e); }
    this._render();
  }
  async _addSource(source) {   // from the TV's source list, logo guessed by NOIR Remote
    const apps = this.s.model.tv.apps = this.s.model.tv.apps || [];
    if (apps.length >= 8) return;
    let logo = "";
    if (source) { try { logo = (await this._hass.callWS({ type: "noir_remote/logo", names: [source] })).logos[0] || ""; } catch (e) { /* no logo */ } }
    apps.push({ name: (source || L("Name")).slice(0, 20), source, logo });
    this._touch(); this._render();
  }

  // ---- helpers ----
  _ents(doms) {
    const st = this._hass?.states || {};
    return Object.keys(st).filter((e) => doms.includes(e.split(".")[0])).sort();
  }
  _list(id, doms) {
    const st = this._hass?.states || {};
    return `<datalist id="${id}">${this._ents(doms).map((e) => `<option value="${esc(e)}">${esc(st[e]?.attributes?.friendly_name || e)}</option>`).join("")}</datalist>`;
  }
  _sel(f, val, doms, attrs, empty = L("— none —")) {
    const st = this._hass?.states || {};
    const all = this._ents(doms);
    if (val && !all.includes(val)) all.unshift(val);
    return `<select ${attrs} data-f="${f}"><option value="">${empty}</option>${all.map((e) => `<option value="${esc(e)}" ${e === val ? "selected" : ""}>${esc(st[e]?.attributes?.friendly_name || e)} · ${esc(e)}</option>`).join("")}</select>`;
  }
  _icon(f, val, attrs) {
    const list = ICONS.includes(val) || !val ? ICONS : [val, ...ICONS];
    return `<span class="ico"><ha-icon icon="${esc(haIcon(val))}"></ha-icon><select ${attrs} data-f="${f}" title="${L("Icon")}">${list.map((x) => `<option value="${esc(x)}" ${x === val ? "selected" : ""}>${esc(x.replace(/^mdi:/, ""))}</option>`).join("")}</select></span>`;
  }
  _acts(list, i, len) {
    return `<span class="acts">
      <button class="i" data-a="up" data-l="${list}" data-i="${i}" ${i === 0 ? "disabled" : ""} title="${L("Up")}"><ha-icon icon="mdi:arrow-up"></ha-icon></button>
      <button class="i" data-a="down" data-l="${list}" data-i="${i}" ${i === len - 1 ? "disabled" : ""} title="${L("Down")}"><ha-icon icon="mdi:arrow-down"></ha-icon></button>
      <button class="i" data-a="del" data-l="${list}" data-i="${i}" title="${L("Remove")}"><ha-icon icon="mdi:close"></ha-icon></button></span>`;
  }
  _layout(tiles) {   // same grid as the remote: half tiles pair up, full tiles take a row
    let x = 0, y = 0;
    return tiles.map((t) => {
      const full = t.size === "full";
      if (full && x) { x = 0; y += 2; }
      const p = { x: full ? 0 : x, y, w: full ? 6 : 3, h: 2 };
      if (full) y += 2; else if (x === 0) x = 3; else { x = 0; y += 2; }
      return p;
    });
  }
  _phone(title, inner) {
    const now = new Date();
    return `<div class="noirv"><div class="nvbar"><span>${String(now.getHours()).padStart(2, "0")}:${String(now.getMinutes()).padStart(2, "0")}</span>82%</div>
      <div class="nvbody">${title}${inner}</div>
      <div class="nvfoot"><span class="nvk"><ha-icon icon="mdi:arrow-left"></ha-icon>${LANG === "en" ? "Back" : "Terug"}</span><span class="nvk"><ha-icon icon="mdi:home"></ha-icon>${LANG === "en" ? "Home" : "Thuis"}</span></div></div>`;
  }

  // ---- tabs ----
  _home() {
    const m = this.s.model, sel = this.s.sel;
    const pos = this._layout(m.tiles);
    const tiles = m.tiles.map((t, i) => {
      const p = pos[i];
      const px = Math.floor(p.x * 224 / 6), pw = Math.floor((p.x + p.w) * 224 / 6) - px - 8, py = 52 + p.y * 31, ph = p.h * 31 - 6;
      return `<span class="nvk big ${sel === i ? "sel" : ""}" data-a="pick" data-i="${i}" style="left:${px}px;top:${py}px;width:${pw}px;height:${ph}px"><ha-icon icon="${esc(haIcon(t.icon))}"></ha-icon><b>${esc(t.label)}</b></span>`;
    }).join("");
    const preview = this._phone(`<b class="nvt">NOIR / SIGNATURE</b><small class="nvs">Home Control</small>`, tiles);
    const at = (i) => `data-l="tiles" data-i="${i}"`;
    const rows = m.tiles.map((t, i) => {
      const ent = t.opens === "toggle" ? `<input list="dl-toggle" ${at(i)} data-f="entity" value="${esc(t.entity)}" placeholder="light.… / switch.…">`
        : t.opens === "run" ? `<input list="dl-run" ${at(i)} data-f="entity" value="${esc(t.entity)}" placeholder="scene.… / script.…">`
        : `<span class="muted">—</span>`;
      return `<div class="row home ${sel === i ? "sel" : ""}">${this._icon("icon", t.icon, at(i))}
        <input ${at(i)} data-f="label" value="${esc(t.label)}" maxlength="24" placeholder="${L("Name")}">
        <select ${at(i)} data-f="opens">${OPENS.map(([k, l]) => `<option value="${k}" ${k === t.opens ? "selected" : ""}>${L(l)}</option>`).join("")}</select>
        ${ent}
        <select ${at(i)} data-f="size"><option value="half" ${t.size !== "full" ? "selected" : ""}>${L("Half")}</option><option value="full" ${t.size === "full" ? "selected" : ""}>${L("Full")}</option></select>
        ${this._acts("tiles", i, m.tiles.length)}</div>`;
    }).join("");
    return { preview, hint: L("Tap a tile to edit it"), body: `<div class="card"><h3>${L("Tiles on your home screen")}</h3>
      
      <div class="row home head"><span></span><span>${L("Name")}</span><span>${L("Opens")}</span><span>${L("Entity")}</span><span>${L("Size")}</span><span></span></div>
      ${rows}<button class="btn" data-a="add" data-l="tiles"><ha-icon icon="mdi:plus"></ha-icon>${L("Add tile")}</button></div>` };
  }
  _lights() {
    const m = this.s.model, st = this._hass?.states || {};
    const shown = m.lights.filter((l) => l.entity).map((l, i) => {
      const z = st[l.entity], on = z?.state === "on", b = z?.attributes?.brightness;
      const txt = !z ? "?" : !on ? (LANG === "en" ? "Off" : "Uit") : b != null ? `${Math.round(b / 2.55)}%` : (LANG === "en" ? "On" : "Aan");
      return `<span class="nvk ${on ? "" : "off"}" style="left:0;top:${40 + i * 46}px;width:216px;height:38px;justify-content:center">${esc(l.label || l.entity)} · ${txt}</span>`;
    }).join("");
    const at = (i) => `data-l="lights" data-i="${i}"`;
    const rows = m.lights.map((l, i) => `<div class="row lights">
      <input ${at(i)} data-f="label" value="${esc(l.label)}" maxlength="24" placeholder="${L("Name")}">
      <input list="dl-toggle" ${at(i)} data-f="entity" value="${esc(l.entity)}" placeholder="light.… / switch.…">
      ${(l.entity || "").startsWith("light.") ? `<label><input type="checkbox" ${at(i)} data-f="dim" ${l.dim !== false ? "checked" : ""}> ${L("Dimmable")}</label>` : `<span class="muted">${L("On/off")}</span>`}
      ${this._acts("lights", i, m.lights.length)}</div>`).join("");
    return { preview: this._phone(`<b class="nvt">${LANG === "en" ? "Lights" : "Verlichting"}</b>`, shown), hint: L("Tap = on/off · hold = dim"), body: `<div class="card"><h3>${L("Lights")}</h3>
      <p>${L("Also used by")} <b>${L("All off")}</b>.</p>
      <div class="row lights head"><span>${L("Name")}</span><span>${L("Entity")}</span><span>${L("Dimmable")}</span><span></span></div>
      ${rows}<button class="btn" data-a="add" data-l="lights"><ha-icon icon="mdi:plus"></ha-icon>${L("Add light")}</button></div>` };
  }
  _scenes() {
    const m = this.s.model;
    const all = [{ label: LANG === "en" ? "All off" : "Alles uit", icon: "mdi:power" }, ...m.scenes.filter((s) => s.label)];
    const tiles = all.map((s, n) => `<span class="nvk big" style="left:${(n % 2) * 112}px;top:${36 + Math.floor(n / 2) * 64}px;width:104px;height:56px"><ha-icon icon="${esc(haIcon(s.icon))}"></ha-icon><b>${esc(s.label)}</b></span>`).join("");
    const at = (i) => `data-l="scenes" data-i="${i}"`;
    const rows = m.scenes.map((s, i) => `<div class="row scenes">${this._icon("icon", s.icon, at(i))}
      <input ${at(i)} data-f="label" value="${esc(s.label)}" maxlength="24" placeholder="${L("Name")}">
      <input list="dl-run" ${at(i)} data-f="entity" value="${esc(s.entity)}" placeholder="scene.… / script.…">
      ${this._acts("scenes", i, m.scenes.length)}</div>`).join("");
    return { preview: this._phone(`<b class="nvt">${LANG === "en" ? "Scenes" : "Scènes"}</b>`, tiles), hint: "“All off” is always there", body: `<div class="card"><h3>${L("Scenes")}</h3>
      
      <div class="row scenes head"><span></span><span>${L("Name")}</span><span>${L("Scene or script")}</span><span></span></div>
      ${rows}<button class="btn" data-a="add" data-l="scenes"><ha-icon icon="mdi:plus"></ha-icon>${L("Add scene")}</button></div>` };
  }
  _tv() {
    const tv = this.s.model.tv, at = `data-l="tv" data-i="0"`, st = this._hass?.states || {};
    const apps = tv.apps || [];
    // preview: same grid as the remote (2 columns, logo + name)
    const tiles = apps.slice(0, 6).map((a, n) => `<span class="nvk big" style="left:${(n % 2) * 112}px;top:${52 + Math.floor(n / 2) * 80}px;width:104px;height:76px;align-items:center;justify-content:flex-end;gap:4px">
      ${a.logo ? `<img src="${LOGO_URL(esc(a.logo))}" width="34" height="34" style="margin-bottom:auto;margin-top:6px">` : ""}<b style="font-size:12px;color:#bfc2c9">${esc(a.name)}</b></span>`).join("")
      || `<small class="nvs" style="top:80px">${L("No sources yet")}</small>`;
    const tvs = st[tv.entity];
    const on = tvs && !["off", "unavailable", "unknown"].includes(tvs.state);
    const preview = this._phone(`<b class="nvt">TV</b><small class="nvs">${on ? "● " + (LANG === "en" ? "On" : "Aan") + (tvs.attributes.source ? " · " + esc(tvs.attributes.source) : "") : (LANG === "en" ? "Off" : "Uit")}</small>`, tiles);
    // 1 · TV
    const c1 = `<div class="card"><h3>1 · ${L("Your TV in Home Assistant")}</h3>${this._sel("entity", tv.entity, ["media_player"], at)}</div>`;
    // 2 · infrared profile
    const prof = (this.s.profiles || []).map((p) => `<div class="mode ${tv.profile === p.key ? "on" : ""}" data-a="profile" data-v="${esc(p.key)}"><b>${esc(p.name)}</b><span>${esc(p.tested)}</span></div>`).join("")
      + `<div class="mode ${!tv.profile ? "on" : ""}" data-a="profile" data-v=""><b>${L("No infrared")}</b><span>${L("Everything via Home Assistant")}</span></div>`;
    const test = tv.profile ? `<div class="bar" style="margin-top:12px"><b>${L("Test")}:</b>
        <button class="btn" data-a="test" data-k="power_on"><ha-icon icon="mdi:power"></ha-icon>${L("On")}</button>
        <button class="btn" data-a="test" data-k="power_off"><ha-icon icon="mdi:power-off"></ha-icon>${L("Off")}</button>
        <span class="${this.s.testErr ? "dirty" : "ok"}">${esc(this.s.testMsg || "")}</span></div>` : "";
    const c2 = `<div class="card"><h3>2 · ${L("Switching on and keys")}</h3><div class="modes">${prof}</div>${test}
      <p class="note">${L("Brand not listed? See docs/tv.md – one small file adds it.")}</p></div>`;
    // 3 · speaker
    const c3 = `<div class="card"><h3>3 · ${L("Speaker (optional)")}</h3><p>${L("Switches on with the TV, gets the volume keys.")}</p>${this._sel("speaker", tv.speaker, ["media_player"], at)}</div>`;
    // 4 · sources
    const logoSel = (i, val) => {
      const opt = (g) => (this.s.logos || []).filter((x) => x.group === g).map((x) => `<option value="${esc(x.key)}" ${x.key === val ? "selected" : ""}>${esc(x.name)}</option>`).join("");
      return `<span class="ico" style="width:52px;height:40px;background:#15171b;color:#e4e7ec">${val ? `<img src="${LOGO_URL(esc(val))}" width="28" height="28">` : `<ha-icon icon="mdi:image-off-outline"></ha-icon>`}
        <select data-l="apps" data-i="${i}" data-f="logo" title="${L("Logo")}"><option value="">${L("No logo")}</option>
        <optgroup label="${L("Brands")}">${opt("brands")}</optgroup><optgroup label="${L("General")}">${opt("generic")}</optgroup></select></span>`;
    };
    const rows = apps.map((a, i) => `<div class="row apps">${logoSel(i, a.logo)}
      <input data-l="apps" data-i="${i}" data-f="name" value="${esc(a.name)}" maxlength="20" placeholder="${L("Name")}">
      <span class="muted" title="${L("Source")}">${esc(a.source)}</span>${this._acts("apps", i, apps.length)}</div>`).join("");
    const used = new Set(apps.map((a) => a.source));
    // Many TVs (LG, Samsung) only report their inputs and apps while they are on: remember the last list we saw.
    const ck = `noir_sources_${tv.entity}`;
    let list = tvs?.attributes?.source_list || [];
    try { if (list.length) localStorage.setItem(ck, JSON.stringify(list)); else list = JSON.parse(localStorage.getItem(ck) || "[]"); } catch (e) { /* storage blocked */ }
    const avail = list.filter((x) => !used.has(x));
    const chips = !tv.entity ? `<p class="note">${L("Choose your TV first – its inputs and apps appear here.")}</p>`
      : `${!list.length ? `<p class="note">${L("Switch your TV on – its inputs and apps then appear here.")}</p>` : ""}<div class="chips">${avail.map((x) => `<button class="chip" data-a="addsrc" data-v="${esc(x)}" ${apps.length >= 8 ? "disabled" : ""}><ha-icon icon="mdi:plus"></ha-icon>${esc(x)}</button>`).join("")}
         <button class="chip" data-a="addsrc" data-v="" ${apps.length >= 8 ? "disabled" : ""}><ha-icon icon="mdi:pencil-plus-outline"></ha-icon>${L("Add by hand")}</button></div>`;
    const c4 = `<div class="card"><h3>4 · ${L("Sources on your remote")}</h3><p>${L("Tap one below to add it. Up to 8.")}</p>${rows}${chips}</div>`;
    return { preview, hint: "", body: c1 + c2 + c3 + c4 };
  }
  _remote() {
    const eco = this.s.model.mode === "eco";
    const card = (k, ic, t, sub) => `<div class="mode ${(k === "eco") === eco ? "on" : ""}" data-a="mode" data-m="${k}"><ha-icon icon="${ic}"></ha-icon><b>${t}</b><span>${sub}</span></div>`;
    return { preview: this._phone(`<b class="nvt">NOIR / SIGNATURE</b><small class="nvs">Home Control</small>`, ""), hint: "", body: `
      <div class="card"><h3>${L("Mode")}</h3><div class="modes">
        ${card("full", "mdi:lightning-bolt", "Full", L("Always ready"))}
        ${card("eco", "mdi:leaf", "Eco", L("Longer battery"))}</div></div>
      <div class="card"><h3>${L("Language")}</h3><div class="modes">
        <div class="mode ${LANG === "nl" ? "on" : ""}" data-a="lang" data-v="nl"><b>Nederlands</b><span>Remote + Builder</span></div>
        <div class="mode ${LANG === "en" ? "on" : ""}" data-a="lang" data-v="en"><b>English</b><span>Remote + Builder</span></div></div></div>
      ${this._firmware()}
      <div class="card"><details><summary>${L("First-time setup")}</summary>
        <p><b>1</b> · ${L("Install the firmware from your browser (Chrome or Edge): connect the remote by USB and press Install.")}</p>
        <p><a class="btn primary" href="${INSTALL_URL}" target="_blank" rel="noopener"><ha-icon icon="mdi:usb"></ha-icon>${L("Open the installer")}</a></p>
        <p><b>2</b> · ${L("Connect your phone to the Wi-Fi network “noir-remote setup” and choose your own Wi-Fi.")}</p>
        <p><b>3</b> · ${L("Done: Home Assistant adds the remote by itself – no keys, no files.")}</p></details></div>` };
  }

  _render() {
    if (!this._hass) return;
    this._painted = true;
    const m = this.s.model;
    if (!m) { this.shadowRoot.innerHTML = `<style>${CSS}</style><div class="wrap"><h1>NOIR Builder</h1><p class="sub">${esc(this.s.msg || "Loading…")}</p></div>`; return; }
    LANG = m.lang === "en" ? "en" : "nl";
    const t = { home: this._home, lights: this._lights, scenes: this._scenes, tv: this._tv, remote: this._remote }[this.s.tab].call(this);
    const y = window.scrollY;
    this.shadowRoot.innerHTML = `<style>${CSS}</style><div class="wrap">
      <h1>NOIR Builder</h1><p class="sub">${L("What you see left is what your remote shows.")}</p>
      <div class="tabs">${TABS.map(([k, l, ic]) => `<button class="tab ${k === this.s.tab ? "on" : ""}" data-a="tab" data-t="${k}"><ha-icon icon="${ic}"></ha-icon>${L(l)}</button>`).join("")}</div>
      <div class="main"><div class="phone">${t.preview}<small class="hint">${t.hint}</small></div>
        <div class="ed">${t.body}
          <div class="bar"><button class="btn primary" data-a="save" ${this.s.dirty ? "" : "disabled"}><ha-icon icon="mdi:content-save"></ha-icon>${L("Save")}</button>
            ${this.s.dirty ? `<span class="dirty">${L("Unsaved changes")}</span>` : this.s.msg ? `<span class="ok">${esc(this.s.msg)}</span>` : ""}</div></div></div>
      ${this._list("dl-toggle", ["light", "switch", "input_boolean", "fan"])}${this._list("dl-run", ["scene", "script"])}</div>`;
    window.scrollTo(0, y);
  }

  _click(e) {
    const b = e.target.closest("[data-a]");
    if (!b) return;
    const a = b.dataset.a, m = this.s.model, l = b.dataset.l, i = Number(b.dataset.i);
    if (a === "tab") { this.s.tab = b.dataset.t; this.s.sel = null; }
    else if (a === "pick") { this.s.sel = i; }
    else if (a === "mode") { m.mode = b.dataset.m; this._touch(); }
    else if (a === "lang") {   // switch language; standard tile names follow
      const NAMES = [["Verlichting", "Lights"], ["Scènes", "Scenes"], ["Alles uit", "All off"], ["Instellingen", "Settings"]];
      const en = b.dataset.v === "en";
      m.lang = b.dataset.v;
      for (const t of m.tiles) for (const [nl, e] of NAMES) if (t.label === (en ? nl : e)) t.label = en ? e : nl;
      this._touch();
    }
    else if (a === "profile") { m.tv.profile = b.dataset.v; this.s.testMsg = ""; this._touch(); }
    else if (a === "test") { this._test(b.dataset.k); return; }
    else if (a === "addsrc") { this._addSource(b.dataset.v); return; }
    else if (a === "fw_install") { this._fwInstall(); return; }
    else if (a === "save") { this._save(); return; }
    else if (a === "add") {
      if (l === "tiles") { m.tiles.push({ icon: "mdi:lightbulb", label: L("New tile"), opens: "toggle", entity: "", size: "half" }); this.s.sel = m.tiles.length - 1; }
      if (l === "lights") m.lights.push({ label: "", entity: "", dim: true });
      if (l === "scenes") m.scenes.push({ icon: "mdi:play", label: "", entity: "" });
      this._touch();
    } else if (a === "del") { (l === "apps" ? m.tv.apps : m[l]).splice(i, 1); this.s.sel = null; this._touch(); }
    else if (a === "up" || a === "down") {
      const j = a === "up" ? i - 1 : i + 1, arr = l === "apps" ? m.tv.apps : m[l];
      if (j < 0 || j >= arr.length) return;
      [arr[i], arr[j]] = [arr[j], arr[i]];
      if (this.s.sel === i) this.s.sel = j;
      this._touch();
    } else return;
    this._render();
  }
  // The remote's firmware update (ESPHome update entity, e.g. update.noir_remote_firmware)
  _fwEntity() {
    // ESPHome's update entity of the remote, found through its device (the entity id can differ: _2, renamed, ...)
    const h = this._hass || {}, st = h.states || {}, ents = h.entities || {}, devs = h.devices || {};
    const norm = (x) => String(x || "").toLowerCase().replace(/[^a-z0-9]/g, "");
    const want = norm(this.s.device || "noir-remote");
    const ours = (e) => {
      const r = ents[e];
      if (!r || r.platform !== "esphome") return false;
      const d = devs[r.device_id];
      return d ? [d.name, d.name_by_user].some((n) => norm(n).includes(want)) : norm(e).includes(want);
    };
    return Object.keys(st).find((e) => e.startsWith("update.") && ours(e)) || "";
  }
  _firmware() {
    const e = this._fwEntity(), st = e && this._hass.states[e];
    if (!st) return `<div class="card"><h3>${L("Firmware")}</h3>
      <p>${L("No update found on your remote yet. Remotes with firmware 0.2 install the new firmware once via the installer (USB); after that, updates come here with one button.")}</p>
      <p><a class="btn" href="${INSTALL_URL}" target="_blank" rel="noopener"><ha-icon icon="mdi:usb"></ha-icon>${L("Open the installer")}</a></p></div>`;
    const a = st.attributes, busy = a.in_progress === true || typeof a.in_progress === "number" || a.update_percentage != null;
    const pct = typeof a.update_percentage === "number" ? a.update_percentage : (typeof a.in_progress === "number" ? a.in_progress : null);
    const avail = st.state === "on";
    const line = busy ? `${L("Updating…")}${pct != null ? " " + pct + "%" : ""} – ${L("keep the remote close to your Wi-Fi.")}`
      : avail ? `${L("New firmware")}: <b>${esc(a.latest_version || "")}</b> (${L("now")} ${esc(a.installed_version || "?")})`
      : `${L("Up to date")} · ${esc(a.installed_version || "?")}`;
    return `<div class="card"><h3>${L("Firmware")}</h3><p>${line}</p>
      <div class="bar"><button class="btn ${avail ? "primary" : ""}" data-a="fw_install" ${avail && !busy ? "" : "disabled"}><ha-icon icon="mdi:update"></ha-icon>${L("Update firmware")}</button>
      ${this.s.fwMsg ? `<span class="${this.s.fwErr ? "dirty" : "ok"}">${esc(this.s.fwMsg)}</span>` : ""}</div></div>`;
  }
  async _fwInstall() {
    const e = this._fwEntity();
    if (!e) return;
    this.s.fwErr = false; this.s.fwMsg = L("Started – the remote restarts when it is done.");
    this._render();
    try { await this._hass.callService("update", "install", { entity_id: e }); }
    catch (err) { this.s.fwErr = true; this.s.fwMsg = String(err?.message || err); }
    this._render();
  }

  _change(e) {
    const t = e.target, l = t.dataset.l, f = t.dataset.f;
    if (!l || !f) return;
    const m = this.s.model, i = Number(t.dataset.i);
    const v = t.type === "checkbox" ? t.checked : String(t.value).trim();
    const st = this._hass?.states || {};
    if (l === "tv") m.tv[f] = v;
    else if (l === "apps") {
      const it = m.tv.apps[i];
      if (!it) return;
      it[f] = f === "name" ? v.slice(0, 20) : v;
      if (f === "name" && !it.source) it.source = v;
    } else {
      const it = m[l][i];
      if (!it) return;
      if (f === "opens") {
        it.opens = v;
        if (!["toggle", "run"].includes(v)) { it.entity = ""; it.icon = OPENS_ICON[v] || it.icon; if (!it.label || it.label === L("New tile")) it.label = L(OPENS_LABEL[v] || it.label); }
      } else if (f === "entity") {
        it.entity = v;
        if (!it.label && st[v]) it.label = (st[v].attributes.friendly_name || "").slice(0, 24);
      } else it[f] = f === "label" ? v.slice(0, 24) : v;
    }
    this._touch();
    this._render();
  }
}
if (!customElements.get("noir-builder")) customElements.define("noir-builder", NoirBuilder);
