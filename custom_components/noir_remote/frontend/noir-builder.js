// NOIR Builder - sidebar panel of the NOIR Remote integration.
// One tab per screen, live preview on the left, simple rows on the right. Save = the remote updates in seconds.

const TABS = [["home", "Home", "mdi:home-outline"], ["lights", "Lights", "mdi:lightbulb-outline"],
  ["scenes", "Scenes", "mdi:layers-outline"], ["tv", "TV", "mdi:television"], ["audio", "Audio", "mdi:speaker"], ["airco", "Airco", "mdi:snowflake"], ["remote", "Remote", "mdi:remote"]];
const OPENS = [["tv", "TV screen"], ["lights", "Lights screen"], ["scenes", "Scenes screen"], ["alloff", "All off"],
  ["tv_power", "TV on/off"], ["settings", "Remote settings"], ["toggle", "Toggle an entity"], ["run", "Run a scene or script"],
  ["audio", "Audio screen"], ["airco", "Airco screen"],
  ["weather", "Weather"], ["calendar", "Calendar"], ["value", "Value (stock, crypto, sensor)"], ["mouse", "TV mouse"]];
const INFO = { weather: ["weather"], calendar: ["calendar"], value: ["sensor", "input_number", "number"] };   // info tiles: which entities
const OPENS_ICON = { tv: "noir:tv", lights: "noir:licht", scenes: "noir:scenes", alloff: "mdi:power", tv_power: "mdi:power",
  settings: "mdi:cog", toggle: "mdi:lightbulb", run: "mdi:play", audio: "noir:audio", airco: "mdi:air-conditioner",
  weather: "mdi:weather-partly-cloudy", calendar: "mdi:calendar", value: "mdi:chart-line", mouse: "mdi:mouse" };
const OPENS_LABEL = { tv: "TV", lights: "Lights", scenes: "Scenes", alloff: "All off", tv_power: "TV", settings: "Settings",
  audio: "Audio", airco: "Airco", weather: "Weather", calendar: "Calendar", mouse: "TV mouse" };
const ICONS = ["noir:tv", "noir:audio", "noir:licht", "noir:scenes", "mdi:power", "mdi:lightbulb", "mdi:floor-lamp", "mdi:ceiling-light",
  "mdi:lamp", "mdi:led-strip", "mdi:string-lights", "mdi:sofa", "mdi:bed", "mdi:movie-open", "mdi:music", "mdi:television",
  "mdi:sony-playstation", "mdi:nintendo-switch", "mdi:netflix", "mdi:youtube", "mdi:play", "mdi:weather-night", "mdi:white-balance-sunny",
  "mdi:silverware-fork-knife", "mdi:coffee", "mdi:fan", "mdi:air-conditioner", "mdi:thermometer", "mdi:blinds", "mdi:window-shutter",
  "mdi:door", "mdi:lock", "mdi:home", "mdi:cog", "mdi:heart", "mdi:star", "mdi:leaf", "mdi:flash", "mdi:speaker"];
const INSTALL_URL = "https://ronsone.github.io/Noir_Remote/";
const LOGO_URL = (k) => `/noir_remote_static/logos/${k}.png`;
const NL = {"Home": "Home", "Lights": "Verlichting", "Scenes": "Scènes", "TV": "TV", "Remote": "Remote", "TV screen": "TV", "Lights screen": "Verlichting", "Scenes screen": "Scènes", "All off": "Alles uit", "TV on/off": "TV aan/uit", "Remote settings": "Instellingen remote", "Toggle an entity": "Iets aan/uit zetten", "Run a scene or script": "Scène of script starten", "Tiles on your home screen": "Tegels op je startscherm", "Add tile": "Tegel toevoegen", "Add light": "Lamp toevoegen", "Add scene": "Scène toevoegen", "Name": "Naam", "Opens": "Opent", "Entity": "Entiteit", "Size": "Maat", "Half": "Half", "Full": "Vol", "Dimmable": "Dimbaar", "On/off": "Aan/uit", "Scene or script": "Scène of script", "Also used by": "Ook gebruikt door", "Mode": "Modus", "Language": "Taal", "Always ready": "Altijd klaar", "Longer battery": "Langer op de accu", "First-time setup": "Eerste keer instellen", "Save": "Opslaan", "Unsaved changes": "Niet opgeslagen", "Saved – the remote updates in a few seconds.": "Opgeslagen – de remote past zich zo aan.", "What you see left is what your remote shows.": "Links zie je wat je remote laat zien.", "Tap a tile to edit it": "Tik op een tegel om hem aan te passen", "Tap = on/off · hold = dim": "Tikken = aan/uit · vasthouden = dimmen", "New tile": "Nieuwe tegel", "Up": "Omhoog", "Down": "Omlaag", "Remove": "Verwijderen", "Icon": "Icoon", "Settings": "Instellingen", "Loading…": "Laden…", "Your TV in Home Assistant": "Je tv in Home Assistant", "Switching on and keys": "Aanzetten en toetsen", "No infrared": "Geen infrarood", "Everything via Home Assistant": "Alles via Home Assistant", "Test": "Testen", "On": "Aan", "Off": "Uit", "Sent – did your TV react?": "Verstuurd – reageerde je tv?", "Brand not listed? See docs/tv.md – one small file adds it.": "Merk staat er niet bij? Zie docs/tv.md – één klein bestand voegt het toe.", "Speaker (optional)": "Speaker (optioneel)", "Switches on with the TV, gets the volume keys.": "Gaat aan met de tv en krijgt de volumetoetsen.", "Sources on your remote": "Bronnen op je remote", "Tap one below to add it. Up to 8.": "Tik hieronder op een bron om hem toe te voegen. Maximaal 8.", "Choose your TV first – its inputs and apps appear here.": "Kies eerst je tv – zijn ingangen en apps verschijnen hier.", "Add by hand": "Zelf toevoegen", "Logo": "Logo", "No logo": "Geen logo", "Brands": "Merken", "General": "Algemeen", "Source": "Bron", "No sources yet": "Nog geen bronnen", "— none —": "— geen —", "Install the firmware from your browser (Chrome or Edge): connect the remote by USB and press Install.": "Installeer de firmware vanuit je browser (Chrome of Edge): sluit de remote aan via USB en druk op Installeren.", "Open the installer": "Open de installer", "Connect your phone to the Wi-Fi network “noir-remote setup” and choose your own Wi-Fi.": "Verbind je telefoon met het wifi-netwerk “noir-remote setup” en kies je eigen wifi.", "Done: Home Assistant adds the remote by itself – no keys, no files.": "Klaar: Home Assistant voegt de remote vanzelf toe – geen sleutels, geen bestanden.", "No update found on your remote yet. Remotes with firmware 0.2 install the new firmware once via the installer (USB); after that, updates come here with one button.": "Nog geen update gevonden op je remote. Een remote met firmware 0.2 installeert de nieuwe firmware één keer via de installer (USB); daarna komen updates hier, met één knop.", "Updating…": "Bijwerken…", "keep the remote close to your Wi-Fi.": "houd de remote in de buurt van je wifi.", "New firmware": "Nieuwe firmware", "now": "nu", "Up to date": "Up-to-date", "Started – the remote restarts when it is done.": "Gestart – de remote herstart als hij klaar is.", "Stable": "Stabiel", "Test channel": "Test", "Released versions": "Vrijgegeven versies", "Newest build – for testers": "Nieuwste versie – voor testers", "Airco screen": "Airco-scherm", "— choose —": "— kies —", "Home Assistant (any brand)": "Home Assistant (elk merk)", "Infrared of the remote": "Infrarood van de remote", "Room, e.g. Living room": "Kamer, bijv. Woonkamer", "Your aircos": "Je airco's", "Per room: a climate entity in Home Assistant (any brand, e.g. an ESP32 module on the airco), or the remote's own infrared.": "Per kamer: een climate-entiteit in Home Assistant (elk merk, bijvoorbeeld een ESP32-module op de airco), of de infrarood van de remote zelf.", "Add airco": "Airco toevoegen", "Via Home Assistant the remote shows the modes, fan and swing settings your airco has. Infrared: the remote remembers what it sent (the airco reports nothing back).": "Via Home Assistant toont de remote de standen, ventilator en swing die jouw airco heeft. Infrarood: de remote onthoudt wat hij stuurde (de airco meldt niets terug).", "Add a tile “Airco screen” on Home to open it.": "Zet op Home een tegel “Airco-scherm” om hem te openen.", "Cooling": "Koelen", "room": "kamer", "Audio screen": "Audio-scherm", "Your speaker in Home Assistant": "Je speaker in Home Assistant", "On/off, source and volume go to this speaker. The volume keys of the remote too.": "Aan/uit, bron en volume gaan naar deze speaker. De volumetoetsen van de remote ook.", "Choose your speaker first – its sources appear here.": "Kies eerst je speaker – zijn bronnen verschijnen hier.", "Switch your speaker on – its sources then appear here.": "Zet je speaker aan – dan verschijnen zijn bronnen hier.", "Tap one below to add it. Up to 6.": "Tik hieronder op een bron om hem toe te voegen. Maximaal 6.", "Now playing (optional)": "Nu speelt (optioneel)", "The player that shows what is playing and gets play/pause and next. Empty = the speaker itself.": "De speler die laat zien wat er speelt en play/pauze en volgende krijgt. Leeg = de speaker zelf.", "— the speaker —": "— de speaker —", "Subwoofer (optional)": "Subwoofer (optioneel)", "A number entity with the subwoofer level, e.g. the KEF's.": "Een number-entiteit met het niveau van de subwoofer, bijvoorbeeld die van de KEF.", "Add a tile “Audio screen” on Home to open it.": "Zet op Home een tegel “Audio-scherm” om hem te openen.", "Switch it on with": "Aanzetten met", "On (standard)": "Aan (standaard)", "Speaker does not switch on? Choose the source it plays the TV on – choosing a source wakes it up.": "Gaat de speaker niet aan? Kies de bron waarop hij de tv speelt – een bron kiezen maakt hem wakker.", "Checking…": "Controleren…", "not checked yet": "nog niet gecontroleerd", "Switch your TV on – its inputs and apps then appear here.": "Zet je tv aan – dan verschijnen zijn ingangen en apps hier.", "NOIR app in Home Assistant": "NOIR-app in Home Assistant", "New version": "Nieuwe versie", "Restart Home Assistant": "Home Assistant herstarten", "Downloading the update…": "Update downloaden…", "Home Assistant is restarting – this page reloads by itself.": "Home Assistant herstart – deze pagina laadt daarna vanzelf opnieuw.", "Home Assistant restarts: lights, automations and the remote are offline for about a minute. Continue?": "Home Assistant herstart: lampen, automatiseringen en de remote zijn ongeveer een minuut offline. Doorgaan?", "Not installed through HACS – update it the way you installed it.": "Niet via HACS geïnstalleerd – werk hem bij zoals je hem installeerde.", "Still waiting for Home Assistant… reload this page when it is back.": "Nog steeds wachten op Home Assistant… ververs deze pagina als hij terug is.", "The Builder, its texts and how Home Assistant talks to your remotes.": "De Builder, zijn teksten en hoe Home Assistant met je remotes praat.", "Updates": "Updates", "Update": "Bijwerken", "Everything is up to date.": "Alles is up-to-date.", "Update checks both and only does what is needed. Home Assistant only restarts when NOIR Remote itself got a new version.": "Bijwerken kijkt bij allebei en doet alleen wat nodig is. Home Assistant herstart alleen als NOIR Remote zelf een nieuwe versie kreeg.", "Press Save after changing the channel.": "Druk na het wisselen op Opslaan.", "Updating the remote…": "Remote bijwerken…", "The remote is up to date.": "De remote is bijgewerkt.", "The remote takes long – NOIR Remote itself was not updated yet. Press Update again when the remote is back.": "De remote doet er lang over – NOIR Remote zelf is nog niet bijgewerkt. Druk opnieuw op Bijwerken als de remote terug is.", "Large": "Groot", "columns": "kolommen", "This device shows the same tiles as your first remote. Change something and it gets its own home screen.": "Dit apparaat toont dezelfde tegels als je eerste remote. Verander iets en het krijgt zijn eigen homescherm.", "This device has no infrared – the TV goes through Home Assistant.": "Dit apparaat heeft geen infrarood – de tv gaat via Home Assistant.", "Weather": "Weer", "Calendar": "Agenda", "Value (stock, crypto, sensor)": "Waarde (aandeel, crypto, sensor)", "TV mouse": "Muis voor de tv"};
let LANG = "nl";
let PACKS = {};   // de/fr/es... from NOIR Remote (languages/*.json): {builder: {English: text}, remote: {Dutch: text}}
const L = (en) => (LANG === "nl" ? NL[en] : PACKS[LANG]?.builder?.[en]) || en;
// texts in the remote preview: written as on the remote (nl, en); other languages from the pack's remote texts
const P = (nl, en) => LANG === "nl" ? nl : LANG === "en" ? en : (PACKS[LANG]?.remote?.[nl] || en);
const esc = (s) => String(s ?? "").replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
const haIcon = (i) => ({ "noir:tv": "mdi:television", "noir:audio": "mdi:waveform", "noir:licht": "mdi:floor-lamp-outline", "noir:scenes": "mdi:layers-outline" }[i] || i || "mdi:square-rounded-outline");

const CSS = `
:host { display:block; height:100%; overflow-y:auto; -webkit-overflow-scrolling:touch; background:var(--primary-background-color); color:var(--primary-text-color); font-family:var(--paper-font-body1_-_font-family, Roboto, sans-serif); }
.wrap { max-width:1200px; padding:20px 24px; }
.top { display:flex; align-items:center; gap:4px; } h1 { margin:0 0 2px; font-size:24px; font-weight:600; } .sub { margin:0 0 16px; color:var(--secondary-text-color); }
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
    const e = this.s.tab === "tv" ? this.s.model?.tv?.entity : this.s.tab === "audio" ? this.s.model?.audio?.speaker : this.s.tab === "remote" ? this._fwEntity() : "", st = e && h.states[e];
    const at = st?.attributes || {};
    const sig = st ? `${e}|${st.state}|${(at.source_list || []).length}|${at.source || ""}|${at.in_progress}|${at.update_percentage}|${at.installed_version}` : "";
    if (sig !== this._tvSig) {
      this._tvSig = sig;
      if (e && !this.shadowRoot.activeElement) this._render();
    }
  }
  set panel(p) { this._panel = p; }
  set narrow(n) { const was = this._narrow; this._narrow = n; if (was !== n && this._painted) this._render(); }

  async _load() {
    try {
      const r = await this._hass.callWS({ type: "noir_remote/get" });
      this.s.model = r.model; this.s.device = r.device;
      this.s.devices = r.devices?.length ? r.devices : [{ name: r.device, primary: true, board: "rev6", w: 240, h: 320, caps: ["touch", "keys", "ir", "battery"] }];
      this.s.boards = r.boards || {};
      if (!this.s.devices.some((d) => d.name === this.s.dev)) this.s.dev = this.s.devices[0].name;
      this._hacs(false).then(() => this._render()); this.s.profiles = r.profiles || []; this.s.aircoIr = r.airco_ir || []; this.s.languages = r.languages || []; PACKS = r.packs || {}; this.s.logos = r.logos || [];
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
      await this._hass.callWS({ type: "noir_remote/test", profile: this.s.model.tv.profile, key, device: this._has("ir") ? this._dev().name : this.s.device });
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

  // ---- devices: every NOIR device its own home screen; the Builder hides what the chosen one can't do ----
  _dev() {
    const ds = this.s.devices || [];
    return ds.find((d) => d.name === this.s.dev) || ds[0] || { name: this.s.device, primary: true, w: 240, h: 320, caps: ["touch", "keys", "ir", "battery"] };
  }
  _has(cap) { return (this._dev().caps || []).includes(cap); }
  // home tiles of the chosen device; make = it gets its own list now (a copy of the first remote's)
  _tiles(make) {
    const m = this.s.model, d = this._dev();
    if (d.primary) return m.tiles;
    const own = m.devices?.[d.name];
    if (own) return own.tiles;
    if (!make) return m.tiles;
    m.devices = m.devices || {};
    m.devices[d.name] = { tiles: JSON.parse(JSON.stringify(m.tiles)) };
    return m.devices[d.name].tiles;
  }
  // the same grid as the firmware (noir-ui.h Layout/Grid): dp, columns and row height from the screen size
  _grid(w, h) {
    const u = Math.max(1, Math.min(2, 1 + (Math.min(w, h) - 240) / 700)), wd = Math.round(w / u), hd = Math.round(h / u);
    const full = wd - 24, cols = Math.max(2, Math.min(6, Math.floor((full + 8) / 104)));
    const cw = Math.floor((full - (cols - 1) * 8) / cols), rh = Math.max(56, Math.floor(cw * (cols >= 3 ? 0.8 : 0.54)));
    return { wd, hd, full, cols, cw, rh, big: wd >= 400 && hd >= 380 };
  }
  _place(tiles, g) {   // left to right, top to bottom, no back-filling (like the firmware)
    const used = [], out = [];
    let r = 0, c = 0;
    const free = (r0, c0, sc, sr) => c0 + sc <= g.cols && [...Array(sr).keys()].every((y) => [...Array(sc).keys()].every((x) => !(used[r0 + y] || [])[c0 + x]));
    for (const t of tiles) {
      const sc = Math.min(t.size === "half" || !t.size ? 1 : 2, g.cols), sr = t.size === "large" ? 2 : 1;
      while (!free(r, c, sc, sr)) { if (++c >= g.cols) { c = 0; r++; } }
      for (let y = r; y < r + sr; y++) { used[y] = used[y] || []; for (let x = c; x < c + sc; x++) used[y][x] = 1; }
      out.push({ x: c * (g.cw + 8), y: r * (g.rh + 6), w: sc * g.cw + (sc - 1) * 8, h: sr * g.rh + (sr - 1) * 6 });
      c += sc; if (c >= g.cols) { c = 0; r++; }
    }
    return out;
  }
  _devBar() {
    const ds = this.s.devices || [];
    if (ds.length < 2) return "";
    return `<div class="tabs devs">${ds.map((d) => `<button class="tab ${d.name === this._dev().name ? "on" : ""}" data-a="dev" data-v="${esc(d.name)}">
      <ha-icon icon="${(d.caps || []).includes("battery") ? "mdi:remote" : "mdi:tablet"}"></ha-icon>${esc(d.name)}
      <small style="opacity:.6;margin-left:4px">${esc(this.s.boards?.[d.board]?.name || d.board || "")} · ${d.w}×${d.h}</small></button>`).join("")}</div>`;
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
  _phone(title, inner, g) {   // g = grid of the chosen device (Home); without it the remote's 240 x 320
    const now = new Date(), clock = `${String(now.getHours()).padStart(2, "0")}:${String(now.getMinutes()).padStart(2, "0")}`;
    const box = g ? `width:${g.wd}px;height:${g.hd}px` : "", body = g ? `width:${g.full}px;height:${g.hd - 84}px` : "";
    const foot = g ? `top:${g.hd - 50}px;left:${(g.wd - 216) / 2}px;right:auto;width:216px` : "";
    const v = `<div class="noirv" style="${box}"><div class="nvbar"><span>${clock}</span>82%</div>
      <div class="nvbody" style="${body}">${title}${inner}</div>
      <div class="nvfoot" style="${foot}"><span class="nvk"><ha-icon icon="mdi:arrow-left"></ha-icon>${P("Terug", "Back")}</span><span class="nvk"><ha-icon icon="mdi:home"></ha-icon>${P("Thuis", "Home")}</span></div></div>`;
    if (!g || g.wd <= 260) return v;
    const k = Math.min(1, 380 / g.wd);   // a big screen, scaled to fit next to the editor
    return `<div style="width:${Math.round((g.wd + 16) * k)}px;height:${Math.round((g.hd + 16) * k)}px"><div style="transform:scale(${k});transform-origin:top left">${v}</div></div>`;
  }

  _list_(l) { const m = this.s.model; return l === "apps" ? m.tv.apps : l === "asrc" ? m.audio.sources : l === "units" ? m.airco.units : l === "tiles" ? this._tiles(true) : m[l]; }

  // ---- tabs ----
  _airco() {
    const units = this.s.model.airco.units, st = this._hass?.states || {};
    const climates = this._ents(["climate"]);
    const ctl = (i, val) => `<select data-l="units" data-i="${i}" data-f="control">
        <option value="">${L("— choose —")}</option>
        <optgroup label="${L("Home Assistant (any brand)")}">${climates.map((e) => `<option value="${esc(e)}" ${e === val ? "selected" : ""}>${esc(st[e]?.attributes?.friendly_name || e)} · ${esc(e)}</option>`).join("")}</optgroup>
        ${this._has("ir") || String(val).startsWith("ir:") ? `<optgroup label="${L("Infrared of the remote")}">${(this.s.aircoIr || []).map((p) => `<option value="ir:${esc(p.key)}" ${"ir:" + p.key === val ? "selected" : ""}>${esc(p.name)}</option>`).join("")}</optgroup>` : ""}</select>`;
    const rows = units.map((x, i) => `<div class="row lights">
      <input data-l="units" data-i="${i}" data-f="name" value="${esc(x.name)}" maxlength="16" placeholder="${L("Room, e.g. Living room")}">
      ${ctl(i, x.control)}<span></span>${this._acts("units", i, units.length)}</div>`).join("");
    // preview: design B - dial arc around the temperature of the first unit
    const u0 = units[0], c0 = u0 && st[u0.control], on = c0 && c0.state !== "off";
    const t = c0?.attributes?.temperature ?? 21, room = c0?.attributes?.current_temperature;
    const pct = Math.max(0, Math.min(1, (t - 16) / 14)), len = 270 * pct;
    const arc = `<svg width="104" height="104" viewBox="0 0 104 104" style="position:absolute;left:56px;top:40px">
      <circle cx="52" cy="52" r="48" fill="none" stroke="#383c43" stroke-width="4" stroke-dasharray="226.2 301.6" transform="rotate(135 52 52)" stroke-linecap="round"/>
      <circle cx="52" cy="52" r="48" fill="none" stroke="#9fd8f0" stroke-width="5" stroke-dasharray="${(226.2 * len / 270).toFixed(1)} 301.6" transform="rotate(135 52 52)" stroke-linecap="round" opacity="${on || !c0 ? 1 : .35}"/></svg>`;
    const modes = ["mdi:alpha-a-circle-outline", "mdi:snowflake", "mdi:water", "mdi:fan", "mdi:white-balance-sunny"].map((ic, n) =>
      `<span class="nvk" style="left:${n * 44}px;top:152px;width:38px;height:32px;justify-content:center;${n === 1 ? "background:#9fd8f0;color:#07080a;border-color:#9fd8f0" : ""}"><ha-icon icon="${ic}" style="--mdc-icon-size:18px"></ha-icon></span>`).join("");
    const preview = this._phone(`<b class="nvt">${esc(units.length > 1 && u0?.name ? u0.name : "Airco")}</b><small class="nvs">${on ? L("Cooling") : L("Off")}${room != null ? ` · ${L("room")} ${Math.round(room)}°` : ""}</small>`,
      `${arc}<b style="position:absolute;left:58px;top:68px;width:100px;text-align:center;font-size:40px;font-weight:300">${Math.round(t)}°</b>
       <span class="nvk" style="left:0;top:70px;width:44px;height:44px;border-radius:22px;justify-content:center"><ha-icon icon="mdi:minus"></ha-icon></span>
       <span class="nvk" style="left:172px;top:70px;width:44px;height:44px;border-radius:22px;justify-content:center"><ha-icon icon="mdi:plus"></ha-icon></span>${modes}
       <span class="nvk" style="left:0;top:194px;width:104px;height:30px;border-radius:15px;justify-content:center;gap:6px"><ha-icon icon="mdi:fan" style="--mdc-icon-size:16px"></ha-icon>Auto</span>
       <span class="nvk" style="left:112px;top:194px;width:104px;height:30px;border-radius:15px;justify-content:center;gap:6px"><ha-icon icon="mdi:arrow-oscillating" style="--mdc-icon-size:16px"></ha-icon>Swing</span>`);
    const c1 = `<div class="card"><h3>1 · ${L("Your aircos")}</h3>
      <p>${L("Per room: a climate entity in Home Assistant (any brand, e.g. an ESP32 module on the airco), or the remote's own infrared.")}</p>
      ${rows}<div class="bar" style="margin-top:8px"><button class="btn" data-a="add" data-l="units" ${units.length >= 4 ? "disabled" : ""}><ha-icon icon="mdi:plus"></ha-icon>${L("Add airco")}</button></div>
      <p class="note">${L("Via Home Assistant the remote shows the modes, fan and swing settings your airco has. Infrared: the remote remembers what it sent (the airco reports nothing back).")}</p></div>`;
    return { preview, hint: L("Add a tile “Airco screen” on Home to open it."), body: c1 };
  }
  _audio() {
    const au = this.s.model.audio, at = `data-l="audio" data-i="0"`, st = this._hass?.states || {};
    const sp = st[au.speaker];
    // like the TV: remember the speaker's source list (some only report it while on)
    const ck = `noir_sources_${au.speaker}`;
    let list = sp?.attributes?.source_list || [];
    try { if (list.length) localStorage.setItem(ck, JSON.stringify(list)); else list = JSON.parse(localStorage.getItem(ck) || "[]"); } catch (e) { /* storage blocked */ }
    const used = new Set(au.sources.map((x) => x.source));
    const on = sp && !["off", "standby", "unavailable", "unknown"].includes(sp.state);
    const name = sp?.attributes?.friendly_name || P("Speaker", "Speaker");
    // preview: status, now playing, source buttons (3 per row)
    const btns = au.sources.map((x, n) => `<span class="nvk" style="left:${(n % 3) * 74}px;top:${150 + Math.floor(n / 3) * 38}px;width:68px;height:32px;justify-content:center;${on && sp.attributes.source === x.source ? "border-color:#bfc2c9" : ""}">${esc(x.name)}</span>`).join("");
    const preview = this._phone(`<b class="nvt">Audio</b><small class="nvs">${esc(name)} · ${on ? esc(sp.attributes.source || P("aan", "on")) : P("uit", "off")}</small>`,
      `<span class="nvk" style="left:0;top:52px;width:216px;height:76px;align-items:flex-start;flex-direction:column;justify-content:center;padding-left:12px;box-sizing:border-box"><b style="font-size:13px">${esc(sp?.attributes?.media_title || P("Niets aan het afspelen", "Nothing playing"))}</b><small style="color:#969aa3">${esc(sp?.attributes?.media_artist || "")}</small></span>${btns}`);
    const c1 = `<div class="card"><h3>1 · ${L("Your speaker in Home Assistant")}</h3>${this._sel("speaker", au.speaker, ["media_player"], at)}
      <p class="note">${L("On/off, source and volume go to this speaker. The volume keys of the remote too.")}</p></div>`;
    const rows = au.sources.map((x, i) => `<div class="row lights">
      <input data-l="asrc" data-i="${i}" data-f="name" value="${esc(x.name)}" maxlength="14" placeholder="${L("Name")}">
      <span class="muted" title="${L("Source")}">${esc(x.source)}</span><span></span>${this._acts("asrc", i, au.sources.length)}</div>`).join("");
    const chips = !au.speaker ? `<p class="note">${L("Choose your speaker first – its sources appear here.")}</p>`
      : !list.length ? `<p class="note">${L("Switch your speaker on – its sources then appear here.")}</p>`
      : `<div class="chips">${list.filter((x) => !used.has(x)).map((x) => `<button class="chip" data-a="addasrc" data-v="${esc(x)}" ${au.sources.length >= 6 ? "disabled" : ""}><ha-icon icon="mdi:plus"></ha-icon>${esc(x)}</button>`).join("")}</div>`;
    const c2 = `<div class="card"><h3>2 · ${L("Sources on your remote")}</h3><p>${L("Tap one below to add it. Up to 6.")}</p>${rows}${chips}</div>`;
    const c3 = `<div class="card"><h3>3 · ${L("Now playing (optional)")}</h3><p>${L("The player that shows what is playing and gets play/pause and next. Empty = the speaker itself.")}</p>
      ${this._sel("music", au.music, ["media_player"], at, L("— the speaker —"))}</div>`;
    const c4 = `<div class="card"><h3>4 · ${L("Subwoofer (optional)")}</h3><p>${L("A number entity with the subwoofer level, e.g. the KEF's.")}</p>
      ${this._sel("sub", au.sub, ["number", "input_number"], at)}</div>`;
    return { preview, hint: L("Add a tile “Audio screen” on Home to open it."), body: c1 + c2 + c3 + c4 };
  }
  _home() {
    const m = this.s.model, sel = this.s.sel, d = this._dev(), list = this._tiles(false), g = this._grid(d.w || 240, d.h || 320);
    const top = g.big ? 70 : 52, pos = this._place(list, g);
    const tiles = list.map((t, i) => {
      const p = pos[i];
      // info tiles: what Home Assistant has now (the remote shows the same, with forecast / events / history)
      const es = INFO[t.opens] && this._hass.states[t.entity], at_ = es?.attributes || {};
      const live = !es ? "" : t.opens === "weather" ? `${Math.round(at_.temperature ?? 0)}°`
        : t.opens === "calendar" ? (at_.message || "") : `${es.state}${at_.unit_of_measurement ? " " + at_.unit_of_measurement : ""}`;
      return `<span class="nvk big ${sel === i ? "sel" : ""}" data-a="pick" data-i="${i}" style="left:${p.x}px;top:${top + p.y}px;width:${p.w}px;height:${p.h}px"><ha-icon icon="${esc(haIcon(t.icon))}"></ha-icon>${live ? `<b style="font-size:18px">${esc(live)}</b>` : ""}<b>${esc(t.label)}</b></span>`;
    }).join("");
    const head = g.big ? `<b class="nvt" style="font-size:44px;font-weight:300">${String(new Date().getHours()).padStart(2, "0")}:${String(new Date().getMinutes()).padStart(2, "0")}</b>`
      : `<b class="nvt">NOIR / SIGNATURE</b><small class="nvs">Home Control</small>`;
    const preview = this._phone(head, tiles, d.primary && d.w === 240 && d.h === 320 ? null : g);
    const at = (i) => `data-l="tiles" data-i="${i}"`;
    const rows = list.map((t, i) => {
      const ent = t.opens === "toggle" ? `<input list="dl-toggle" ${at(i)} data-f="entity" value="${esc(t.entity)}" placeholder="light.… / switch.…">`
        : t.opens === "run" ? `<input list="dl-run" ${at(i)} data-f="entity" value="${esc(t.entity)}" placeholder="scene.… / script.…">`
        : INFO[t.opens] ? `<input list="dl-${t.opens}" ${at(i)} data-f="entity" value="${esc(t.entity)}" placeholder="${INFO[t.opens][0]}.…">`
        : `<span class="muted">—</span>`;
      const size = [["half", L("Half")], ["full", L("Full")], ["large", L("Large")]].map(([k, l]) => `<option value="${k}" ${(t.size || "half") === k ? "selected" : ""}>${l}</option>`).join("");
      return `<div class="row home ${sel === i ? "sel" : ""}">${this._icon("icon", t.icon, at(i))}
        <input ${at(i)} data-f="label" value="${esc(t.label)}" maxlength="24" placeholder="${L("Name")}">
        <select ${at(i)} data-f="opens">${OPENS.map(([k, l]) => `<option value="${k}" ${k === t.opens ? "selected" : ""}>${L(l)}</option>`).join("")}</select>
        ${ent}
        <select ${at(i)} data-f="size">${size}</select>
        ${this._acts("tiles", i, list.length)}</div>`;
    }).join("");
    const shared = !d.primary && !m.devices?.[d.name]
      ? `<p class="note">${L("This device shows the same tiles as your first remote. Change something and it gets its own home screen.")}</p>` : "";
    return { preview, hint: `${L("Tap a tile to edit it")} · ${g.cols} ${L("columns")}`, body: `<div class="card"><h3>${L("Tiles on your home screen")}</h3>${shared}
      <div class="row home head"><span></span><span>${L("Name")}</span><span>${L("Opens")}</span><span>${L("Entity")}</span><span>${L("Size")}</span><span></span></div>
      ${rows}<button class="btn" data-a="add" data-l="tiles"><ha-icon icon="mdi:plus"></ha-icon>${L("Add tile")}</button></div>` };
  }
  _lights() {
    const m = this.s.model, st = this._hass?.states || {};
    const shown = m.lights.filter((l) => l.entity).map((l, i) => {
      const z = st[l.entity], on = z?.state === "on", b = z?.attributes?.brightness;
      const txt = !z ? "?" : !on ? P("Uit", "Off") : b != null ? `${Math.round(b / 2.55)}%` : P("Aan", "On");
      return `<span class="nvk ${on ? "" : "off"}" style="left:0;top:${40 + i * 46}px;width:216px;height:38px;justify-content:center">${esc(l.label || l.entity)} · ${txt}</span>`;
    }).join("");
    const at = (i) => `data-l="lights" data-i="${i}"`;
    const rows = m.lights.map((l, i) => `<div class="row lights">
      <input ${at(i)} data-f="label" value="${esc(l.label)}" maxlength="24" placeholder="${L("Name")}">
      <input list="dl-toggle" ${at(i)} data-f="entity" value="${esc(l.entity)}" placeholder="light.… / switch.…">
      ${(l.entity || "").startsWith("light.") ? `<label><input type="checkbox" ${at(i)} data-f="dim" ${l.dim !== false ? "checked" : ""}> ${L("Dimmable")}</label>` : `<span class="muted">${L("On/off")}</span>`}
      ${this._acts("lights", i, m.lights.length)}</div>`).join("");
    return { preview: this._phone(`<b class="nvt">${P("Verlichting", "Lights")}</b>`, shown), hint: L("Tap = on/off · hold = dim"), body: `<div class="card"><h3>${L("Lights")}</h3>
      <p>${L("Also used by")} <b>${L("All off")}</b>.</p>
      <div class="row lights head"><span>${L("Name")}</span><span>${L("Entity")}</span><span>${L("Dimmable")}</span><span></span></div>
      ${rows}<button class="btn" data-a="add" data-l="lights"><ha-icon icon="mdi:plus"></ha-icon>${L("Add light")}</button></div>` };
  }
  _scenes() {
    const m = this.s.model;
    const all = [{ label: P("Alles uit", "All off"), icon: "mdi:power" }, ...m.scenes.filter((s) => s.label)];
    const tiles = all.map((s, n) => `<span class="nvk big" style="left:${(n % 2) * 112}px;top:${36 + Math.floor(n / 2) * 64}px;width:104px;height:56px"><ha-icon icon="${esc(haIcon(s.icon))}"></ha-icon><b>${esc(s.label)}</b></span>`).join("");
    const at = (i) => `data-l="scenes" data-i="${i}"`;
    const rows = m.scenes.map((s, i) => `<div class="row scenes">${this._icon("icon", s.icon, at(i))}
      <input ${at(i)} data-f="label" value="${esc(s.label)}" maxlength="24" placeholder="${L("Name")}">
      <input list="dl-run" ${at(i)} data-f="entity" value="${esc(s.entity)}" placeholder="scene.… / script.…">
      ${this._acts("scenes", i, m.scenes.length)}</div>`).join("");
    return { preview: this._phone(`<b class="nvt">${P("Scènes", "Scenes")}</b>`, tiles), hint: "“All off” is always there", body: `<div class="card"><h3>${L("Scenes")}</h3>
      
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
    const preview = this._phone(`<b class="nvt">TV</b><small class="nvs">${on ? "● " + P("Aan", "On") + (tvs.attributes.source ? " · " + esc(tvs.attributes.source) : "") : P("Uit", "Off")}</small>`, tiles);
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
    const sp = st[tv.speaker], spSrc = sp?.attributes?.source_list || [];
    const srcList = tv.speaker_source && !spSrc.includes(tv.speaker_source) ? [tv.speaker_source, ...spSrc] : spSrc;
    const spSel = tv.speaker ? `<p style="margin-top:10px">${L("Switch it on with")}</p><select ${at} data-f="speaker_source">
        <option value="">${L("On (standard)")}</option>${srcList.map((x) => `<option value="${esc(x)}" ${x === tv.speaker_source ? "selected" : ""}>${L("Source")}: ${esc(x)}</option>`).join("")}</select>
        <p class="note">${L("Speaker does not switch on? Choose the source it plays the TV on – choosing a source wakes it up.")}</p>` : "";
    const c3 = `<div class="card"><h3>3 · ${L("Speaker (optional)")}</h3><p>${L("Switches on with the TV, gets the volume keys.")}</p>${this._sel("speaker", tv.speaker, ["media_player"], at)}${spSel}</div>`;
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
    const noIr = `<div class="card"><h3>2 · ${L("Switching on and keys")}</h3><p class="note">${L("This device has no infrared – the TV goes through Home Assistant.")}</p></div>`;
    return { preview, hint: "", body: c1 + (this._has("ir") ? c2 : noIr) + c3 + c4 };
  }
  _remote() {
    const eco = this.s.model.mode === "eco";
    const card = (k, ic, t, sub) => `<div class="mode ${(k === "eco") === eco ? "on" : ""}" data-a="mode" data-m="${k}"><ha-icon icon="${ic}"></ha-icon><b>${t}</b><span>${sub}</span></div>`;
    const mode = this._has("battery") ? `<div class="card"><h3>${L("Mode")}</h3><div class="modes">
        ${card("full", "mdi:lightning-bolt", "Full", L("Always ready"))}
        ${card("eco", "mdi:leaf", "Eco", L("Longer battery"))}</div></div>` : "";
    return { preview: this._phone(`<b class="nvt">NOIR / SIGNATURE</b><small class="nvs">Home Control</small>`, ""), hint: "", body: `
      ${mode}
      <div class="card"><h3>${L("Language")}</h3><div class="modes">
        ${(this.s.languages?.length ? this.s.languages : [{ code: "nl", name: "Nederlands" }, { code: "en", name: "English" }]).map((x) =>
          `<div class="mode ${LANG === x.code ? "on" : ""}" data-a="lang" data-v="${esc(x.code)}"><b>${esc(x.name)}</b><span>Remote + Builder</span></div>`).join("")}</div></div>
      ${this._updates()}
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
    LANG = m.lang || "nl";
    const t = { home: this._home, lights: this._lights, scenes: this._scenes, tv: this._tv, audio: this._audio, airco: this._airco, remote: this._remote }[this.s.tab].call(this);
    const y = this.scrollTop;   // the panel scrolls itself (Home Assistant gives it a fixed height)
    this.shadowRoot.innerHTML = `<style>${CSS}</style><div class="wrap">
      <div class="top">${this._narrow ? "<ha-menu-button></ha-menu-button>" : ""}<h1>NOIR Builder</h1></div><p class="sub">${L("What you see left is what your remote shows.")}</p>
      ${this._devBar()}<div class="tabs">${TABS.map(([k, l, ic]) => `<button class="tab ${k === this.s.tab ? "on" : ""}" data-a="tab" data-t="${k}"><ha-icon icon="${ic}"></ha-icon>${L(l)}</button>`).join("")}</div>
      <div class="main"><div class="phone">${t.preview}<small class="hint">${t.hint}</small></div>
        <div class="ed">${t.body}
          <div class="bar"><button class="btn primary" data-a="save" ${this.s.dirty ? "" : "disabled"}><ha-icon icon="mdi:content-save"></ha-icon>${L("Save")}</button>
            ${this.s.dirty ? `<span class="dirty">${L("Unsaved changes")}</span>` : this.s.msg ? `<span class="ok">${esc(this.s.msg)}</span>` : ""}</div></div></div>
      ${this._list("dl-toggle", ["light", "switch", "input_boolean", "fan"])}${this._list("dl-run", ["scene", "script"])}
      ${Object.entries(INFO).map(([k, d]) => this._list("dl-" + k, d)).join("")}</div>`;
    this.scrollTop = y;
    const mb = this.shadowRoot.querySelector("ha-menu-button");   // phone: the sidebar opens from here
    if (mb) { mb.hass = this._hass; mb.narrow = true; }
  }

  _click(e) {
    const b = e.target.closest("[data-a]");
    if (!b) return;
    const a = b.dataset.a, m = this.s.model, l = b.dataset.l, i = Number(b.dataset.i);
    if (a === "tab") { this.s.tab = b.dataset.t; this.s.sel = null; }
    else if (a === "dev") { this.s.dev = b.dataset.v; this.s.sel = null; }
    else if (a === "pick") { this.s.sel = i; }
    else if (a === "mode") { m.mode = b.dataset.m; this._touch(); }
    else if (a === "lang") {   // switch language; standard tile names follow
      // a standard name in any language -> the same name in the new language
      const STD = [["Verlichting", "Lights"], ["Scènes", "Scenes"], ["Alles uit", "All off"], ["Instellingen", "Settings"], ["Airco", "Airco"]];
      const name = (nl, en, lang) => lang === "nl" ? nl : lang === "en" ? en : (PACKS[lang]?.builder?.[en] || en);
      const langs = ["nl", "en", ...Object.keys(PACKS)];
      const to = b.dataset.v;
      for (const t of [m.tiles, ...Object.values(m.devices || {}).map((x) => x.tiles)].flat()) for (const [nl, en] of STD) if (langs.some((lg) => name(nl, en, lg) === t.label)) t.label = name(nl, en, to);
      m.lang = to;
      this._touch();
    }
    else if (a === "profile") { m.tv.profile = b.dataset.v; this.s.testMsg = ""; this._touch(); }
    else if (a === "test") { this._test(b.dataset.k); return; }
    else if (a === "addsrc") { this._addSource(b.dataset.v); return; }
    else if (a === "addasrc") {
      const src = b.dataset.v, au = m.audio;
      if (au.sources.length < 6 && !au.sources.some((x) => x.source === src)) au.sources.push({ name: src.slice(0, 14), source: src });
      this._touch();
    }
    else if (a === "channel") { m.channel = b.dataset.v; this._touch(); }
    else if (a === "update_all") { this._updateAll(); return; }
    else if (a === "app_restart") { this._restart(false); return; }
    else if (a === "save") { this._save(); return; }
    else if (a === "add") {
      if (l === "tiles") { const ts = this._tiles(true); ts.push({ icon: "mdi:lightbulb", label: L("New tile"), opens: "toggle", entity: "", size: "half" }); this.s.sel = ts.length - 1; }
      if (l === "lights") m.lights.push({ label: "", entity: "", dim: true });
      if (l === "scenes") m.scenes.push({ icon: "mdi:play", label: "", entity: "" });
      if (l === "units") m.airco.units.push({ name: "", control: "" });
      this._touch();
    } else if (a === "del") { this._list_(l).splice(i, 1); this.s.sel = null; this._touch(); }
    else if (a === "up" || a === "down") {
      const j = a === "up" ? i - 1 : i + 1, arr = this._list_(l);
      if (j < 0 || j >= arr.length) return;
      [arr[i], arr[j]] = [arr[j], arr[i]];
      if (this.s.sel === i) this.s.sel = j;
      this._touch();
    } else return;
    this._render();
  }
  // The remote's firmware update (ESPHome update entity, e.g. update.noir_remote_firmware)
  _fwEntity(device) {
    // ESPHome's update entity of the remote, found through its device (the entity id can differ: _2, renamed, ...)
    const h = this._hass || {}, st = h.states || {}, ents = h.entities || {}, devs = h.devices || {};
    const norm = (x) => String(x || "").toLowerCase().replace(/[^a-z0-9]/g, "");
    const want = norm(device || this.s.device || "noir-remote");
    const ours = (e) => {
      const r = ents[e];
      if (!r || r.platform !== "esphome") return false;
      const d = devs[r.device_id];
      return d ? [d.name, d.name_by_user].some((n) => norm(n).includes(want)) : norm(e).includes(want);
    };
    return Object.keys(st).find((e) => e.startsWith("update.") && ours(e)) || "";
  }
  // NOIR Remote itself (this integration + the Builder): HACS keeps an update entity for it (platform hacs)
  // NOIR Remote in HACS: what HACS itself shows on its page (installed / available) - the update entity can lag behind
  async _hacs(refresh) {
    try {
      const find = async () => ((await this._hass.callWS({ type: "hacs/repositories/list" })) || [])
        .find((x) => String(x.full_name || "").toLowerCase() === "ronsone/noir_remote");
      let r = await find();
      if (r && refresh) { await this._hass.callWS({ type: "hacs/repository/refresh", repository: String(r.id) }); r = await find(); }
      this.s.hacs = r ? { id: String(r.id), installed: String(r.installed_version || ""), available: String(r.available_version || ""),
                          pending: !!r.pending_upgrade } : null;   // pending_upgrade: HACS's own decision (respects a chosen version)
    } catch (e) { this.s.hacs = this.s.hacs || null; }   // no HACS (or another version): the update entity tells what it knows
    return this.s.hacs;
  }
  _appInfo() {   // {installed, available, update} from HACS, else from its update entity
    const h = this.s.hacs;
    if (h && h.installed) return { installed: h.installed, available: h.available, update: h.pending };
    const ae = this._appEntity(), as = ae && this._hass.states[ae], aa = as?.attributes || {};
    return as ? { installed: aa.installed_version || "?", available: aa.latest_version || "", update: as.state === "on" } : null;
  }
  _appEntity() {
    // HACS names it after the repository ("NOIR Remote"); the remote's own firmware update is ESPHome's - skip that one
    const h = this._hass || {}, st = h.states || {}, ents = h.entities || {}, fw = this._fwEntity();
    const ours = (e) => {
      const a = st[e].attributes || {}, p = ents[e]?.platform;
      if (e === fw || p === "esphome") return false;
      return p === "hacs" ? /noir.?remote/i.test(e + " " + (a.title || "") + " " + (a.friendly_name || ""))
        : /noir.?remote/i.test(String(a.release_url || "")) || /^noir.?remote$/i.test(String(a.title || a.friendly_name || "").trim());
    };
    return Object.keys(st).find((e) => e.startsWith("update.") && ours(e)) || "";
  }
  // One card, one button: check both, then update what needs it - the remote's firmware (it downloads and restarts by
  // itself) and NOIR Remote in Home Assistant (HACS, then a restart and this page reloads). Nothing new = nothing happens.
  _updates() {
    const app = this._appInfo();
    const devs = this.s.devices?.length ? this.s.devices : [{ name: this.s.device, caps: ["battery"] }];
    const fwLines = devs.map((d) => this._fwLine(d)).join("");
    const appLine = !app ? L("Not installed through HACS – update it the way you installed it.")
      : app.update ? `${L("New version")}: <b>${esc(app.available)}</b> (${L("now")} ${esc(app.installed)})`
      : `${L("Up to date")} · ${esc(app.installed)}`;
    const ch = this.s.model.channel === "test" ? "test" : "stable";
    const busy = !!this.s.appBusy || devs.some((d) => this._fwBusy(this._fwEntity(d.name)));
    return `<div class="card"><h3>${L("Updates")}</h3>${fwLines}
      <p><ha-icon icon="mdi:home-assistant" style="--mdc-icon-size:18px"></ha-icon> <b>${L("NOIR app in Home Assistant")}</b> · ${appLine}</p>
      <div class="bar"><button class="btn primary" data-a="update_all" ${busy ? "disabled" : ""}><ha-icon icon="mdi:update"></ha-icon>${L("Update")}</button>
      <button class="btn" data-a="app_restart" ${busy ? "disabled" : ""}><ha-icon icon="mdi:restart"></ha-icon>${L("Restart Home Assistant")}</button>
      ${this.s.appMsg ? `<span class="${this.s.appErr ? "dirty" : "ok"}">${esc(this.s.appMsg)}</span>` : ""}</div>
      <p class="note">${L("Update checks both and only does what is needed. Home Assistant only restarts when NOIR Remote itself got a new version.")}</p>
      <div class="modes" style="margin-top:12px">
      <div class="mode ${ch === "stable" ? "on" : ""}" data-a="channel" data-v="stable"><ha-icon icon="mdi:shield-check-outline"></ha-icon><b>${L("Stable")}</b><span>${L("Released versions")}</span></div>
      <div class="mode ${ch === "test" ? "on" : ""}" data-a="channel" data-v="test"><ha-icon icon="mdi:flask-outline"></ha-icon><b>${L("Test channel")}</b><span>${L("Newest build – for testers")}</span></div></div>
      <p class="note">${L("Press Save after changing the channel.")}</p></div>`;
  }
  _fwBusy(fe) {
    const fa = (fe && this._hass.states[fe]?.attributes) || {};
    return fa.in_progress === true || typeof fa.update_percentage === "number" || typeof fa.in_progress === "number";
  }
  _fwNow(fe) {   // what the device runs: its device info (sent on every connection) beats the update entity
    const fa = (fe && this._hass.states[fe]?.attributes) || {};
    const dev = this._hass.devices?.[this._hass.entities?.[fe]?.device_id];
    return String(dev?.sw_version || "").split(" (")[0] || fa.installed_version || "?";
  }
  _fwLine(d) {
    const fe = this._fwEntity(d.name), fs = fe && this._hass.states[fe], fa = fs?.attributes || {};
    const fwNow = this._fwNow(fe);
    const pct = typeof fa.update_percentage === "number" ? fa.update_percentage : (typeof fa.in_progress === "number" ? fa.in_progress : null);
    const line = !fs ? (d.primary ? `${L("No update found on your remote yet. Remotes with firmware 0.2 install the new firmware once via the installer (USB); after that, updates come here with one button.")}
        <a href="${INSTALL_URL}" target="_blank" rel="noopener">${L("Open the installer")}</a>` : L("not checked yet"))
      : this._fwBusy(fe) ? `${L("Updating…")}${pct != null ? " " + pct + "%" : ""} – ${L("keep the remote close to your Wi-Fi.")}`
      : fs.state === "on" && fa.latest_version !== fwNow ? `${L("New version")}: <b>${esc(fa.latest_version || "")}</b> (${L("now")} ${esc(fwNow)})`
      : `${fa.latest_version ? L("Up to date") : L("not checked yet")} · ${esc(fwNow)}`;
    const icon = (d.caps || []).includes("battery") ? "mdi:remote" : "mdi:tablet";
    const name = (this.s.devices || []).length > 1 ? esc(d.name) : L("Remote");
    return `<p><ha-icon icon="${icon}" style="--mdc-icon-size:18px"></ha-icon> <b>${name}</b> · ${line}</p>`;
  }
  async _updateAll() {
    const say = (t, err) => { this.s.appMsg = t; this.s.appErr = !!err; this._render(); };
    const st = (e) => e && this._hass.states[e];
    this.s.appBusy = true;
    say(L("Checking…"));
    const devs = (this.s.devices?.length ? this.s.devices : [{ name: this.s.device }]).map((d) => ({ ...d, fe: this._fwEntity(d.name) })).filter((d) => d.fe);
    const ae = this._appEntity();
    // HACS only looks at GitHub now and then: let it look now (as its own "Update information")
    await this._hacs(true);
    // ask everything for its newest version first (each device fetches its manifest itself), then wait for the answers
    await Promise.all([...devs.map((d) => d.fe), ae].filter(Boolean).map((e) => this._hass.callService("homeassistant", "update_entity", { entity_id: e }).catch(() => {})));
    await new Promise((r) => setTimeout(r, 4000));
    const todo = devs.filter((d) => st(d.fe)?.state === "on" && st(d.fe)?.attributes?.latest_version !== this._fwNow(d.fe));
    const app = !!this._appInfo()?.update;
    if (!todo.length && !app) { this.s.appBusy = false; say(L("Everything is up to date.")); return; }
    // the devices one after the other, each until it runs its new firmware: a Home Assistant restart in the middle of a
    // download leaves a device hanging
    for (const d of todo) {
      const target = st(d.fe)?.attributes?.latest_version || "";
      const runs = () => { const s = st(d.fe); return !!target && s && s.state !== "unavailable" && (this._fwNow(d.fe) === target || s.attributes?.installed_version === target); };
      try { await this._hass.callService("update", "install", { entity_id: d.fe }); }
      catch (err) { this.s.appBusy = false; say(`${d.name}: ${err?.message || err}`, true); return; }
      const until = Date.now() + 6 * 60000;
      while (!runs()) {
        if (Date.now() > until) {
          this.s.appBusy = false;
          say(`${d.name}: ${L("The remote takes long – NOIR Remote itself was not updated yet. Press Update again when the remote is back.")}`, true);
          return;
        }
        const p = st(d.fe)?.attributes?.update_percentage ?? st(d.fe)?.attributes?.in_progress;
        say(`${todo.length > 1 ? d.name + ": " : ""}${L("Updating the remote…")}${typeof p === "number" ? " " + p + "%" : ""} – ${L("keep the remote close to your Wi-Fi.")}`);
        await new Promise((r) => setTimeout(r, 3000));
      }
    }
    say(L("The remote is up to date."));
    if (app) { await this._restart(true); return; }
    this.s.appBusy = false;
    this._render();
  }
  // install = true: HACS downloads NOIR Remote first. Then Home Assistant restarts and this page reloads when it is
  // back, so the new Builder opens.
  async _restart(install) {
    const say = (t, err) => { this.s.appMsg = t; this.s.appErr = !!err; this._render(); };
    if (!window.confirm(L("Home Assistant restarts: lights, automations and the remote are offline for about a minute. Continue?"))) {
      this.s.appBusy = false; say(""); return;
    }
    this.s.appBusy = true;
    try {
      if (install) {
        say(L("Downloading the update…"));
        const h = this.s.hacs;
        if (h?.id) await this._hass.callWS({ type: "hacs/repository/download", repository: h.id });   // HACS picks the version, as its own Update
        else await this._hass.callService("update", "install", { entity_id: this._appEntity() });
      }
      const conn = this._hass.connection;
      let down = false;
      conn.addEventListener("disconnected", () => { down = true; });
      conn.addEventListener("ready", () => { if (down) window.location.reload(); });
      setTimeout(() => say(L("Still waiting for Home Assistant… reload this page when it is back."), true), 240000);
      say(L("Home Assistant is restarting – this page reloads by itself."));
      this._hass.callService("homeassistant", "restart", {}).catch(() => {});   // the connection drops: that is the restart
    } catch (err) {
      this.s.appBusy = false;
      say(String(err?.message || err), true);
    }
  }
  _change(e) {
    const t = e.target, l = t.dataset.l, f = t.dataset.f;
    if (!l || !f) return;
    const m = this.s.model, i = Number(t.dataset.i);
    const v = t.type === "checkbox" ? t.checked : String(t.value).trim();
    const st = this._hass?.states || {};
    if (l === "tv") m.tv[f] = v;
    else if (l === "audio") m.audio[f] = v;
    else if (l === "units") { const it = m.airco.units[i]; if (!it) return; it[f] = f === "name" ? v.slice(0, 16) : v; }
    else if (l === "asrc") { const it = m.audio.sources[i]; if (!it) return; it[f] = v.slice(0, 14) || it.source; }
    else if (l === "apps") {
      const it = m.tv.apps[i];
      if (!it) return;
      it[f] = f === "name" ? v.slice(0, 20) : v;
      if (f === "name" && !it.source) it.source = v;
    } else {
      const it = this._list_(l)[i];
      if (!it) return;
      if (f === "opens") {
        const was = it.opens;
        it.opens = v;
        if (OPENS_LABEL[was] && it.label === L(OPENS_LABEL[was])) it.label = "";   // a standard name follows the choice
        if (!["toggle", "run"].includes(v)) {
          if (!INFO[v]) it.entity = "";
          else if (it.entity && !INFO[v].includes(it.entity.split(".")[0])) it.entity = "";
          it.icon = OPENS_ICON[v] || it.icon;
          if (!it.label || it.label === L("New tile")) it.label = L(OPENS_LABEL[v] || it.label);
          if (v === "calendar" && (it.size || "half") === "half") it.size = "large";   // a calendar needs room
        }
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
