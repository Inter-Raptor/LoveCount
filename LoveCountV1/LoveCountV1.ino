// 2432S022_Mariage_Anecdotes_FULL_RAINBOW_SETTINGS_IMPORTEXPORT.ino

#define LGFX_USE_V1

#include <LovyanGFX.hpp>
#include <Wire.h>
#include <bb_captouch.h>
#include <math.h>

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <time.h>
#include <vector>

#include <ArduinoJson.h>

// =====================================================
// IMPORTANT Arduino IDE: mettre l'enum TOUT EN HAUT
// (sinon le générateur de prototypes casse tout)
// =====================================================
enum Gender : uint8_t { G_MALE=0, G_FEMALE=1, G_OTHER=2 };
enum FontMode : uint8_t { FONT_CLASSIC=0, FONT_COMPACT=1, FONT_LARGE=2, FONT_PRETTY=3 };
enum ColorMode : uint8_t { COLOR_RAINBOW=0, COLOR_FIXED=1, COLOR_PULSE=2 };

struct PersonCfg {
  String name;
  Gender gender;
};

struct AppCfg {
  PersonCfg p1;
  PersonCfg p2;
  int y=2025, mon=9, d=20, hh=12, mm=10, ss=0;
  FontMode font = FONT_CLASSIC;
  ColorMode colorMode = COLOR_RAINBOW;
  uint8_t fixedColor = 0;
};

static AppCfg CFG;

// prototype explicite (encore plus sûr)
static uint16_t genderColor(Gender g);
static uint16_t paletteColor(uint8_t idx);

// =================== USER CONFIG ===================
// Ecran
static const int ROT = 1;
static const int MX  = 18;
static const int MY  = 28;

// WiFi
static const char* WIFI_SSID = "VivienJardot";
static const char* WIFI_PASS = "VivienJardot1991";

// IP fixe
static const IPAddress IP_LOCAL(192,168,1,50);
static const IPAddress IP_GATEWAY(192,168,1,1);
static const IPAddress IP_SUBNET(255,255,255,0);
static const IPAddress IP_DNS1(1,1,1,1);
static const IPAddress IP_DNS2(8,8,8,8);

// Timezone France
static const char* TZ_RULE = "CET-1CEST,M3.5.0/2,M10.5.0/3";

// NTP
static const char* NTP1 = "fr.pool.ntp.org";
static const char* NTP2 = "pool.ntp.org";
static const char* NTP3 = "time.nist.gov";

// Files
static const char* SETTINGS_PATH = "/settings.json";

// =================== HELPERS JSON (ArduinoJson v7) ===================
static const char* jsonCStr(JsonVariantConst v, const char* def) {
  const char* s = v.as<const char*>();
  return (s && *s) ? s : def;
}

// =================== SETTINGS ===================
static uint16_t genderColor(Gender g) {
  if (g == G_MALE)   return TFT_BLUE;
  if (g == G_FEMALE) return (uint16_t)0xF81F; // rose/magenta
  return TFT_WHITE;
}

static uint16_t paletteColor(uint8_t idx) {
  switch (idx) {
    case 1: return TFT_RED;
    case 2: return TFT_GREEN;
    case 3: return TFT_BLUE;
    case 4: return TFT_CYAN;
    case 5: return TFT_MAGENTA;
    case 6: return TFT_YELLOW;
    default: return TFT_WHITE;
  }
}

static void setDefaultSettings() {
  CFG.p1.name = "Vivien";
  CFG.p1.gender = G_MALE;
  CFG.p2.name = "Myriam";
  CFG.p2.gender = G_FEMALE;
  CFG.y=2025; CFG.mon=9; CFG.d=20; CFG.hh=12; CFG.mm=10; CFG.ss=0;
  CFG.font = FONT_CLASSIC;
  CFG.colorMode = COLOR_RAINBOW;
  CFG.fixedColor = 0;
}

static bool saveSettings() {
  DynamicJsonDocument doc(2048);
  doc["p1"]["name"] = CFG.p1.name;
  doc["p1"]["gender"] = (int)CFG.p1.gender;
  doc["p2"]["name"] = CFG.p2.name;
  doc["p2"]["gender"] = (int)CFG.p2.gender;

  JsonObject m = doc["marriage"].to<JsonObject>();
  m["y"]=CFG.y; m["mon"]=CFG.mon; m["d"]=CFG.d;
  m["hh"]=CFG.hh; m["mm"]=CFG.mm; m["ss"]=CFG.ss;

  JsonObject d = doc["display"].to<JsonObject>();
  d["font"] = (int)CFG.font;
  d["colorMode"] = (int)CFG.colorMode;
  d["fixedColor"] = (int)CFG.fixedColor;

  File f = LittleFS.open(SETTINGS_PATH, "w");
  if (!f) return false;
  serializeJson(doc, f);
  f.close();
  return true;
}

static bool loadSettings() {
  if (!LittleFS.exists(SETTINGS_PATH)) {
    setDefaultSettings();
    saveSettings();
    return true;
  }

  File f = LittleFS.open(SETTINGS_PATH, "r");
  if (!f) { setDefaultSettings(); return false; }

  DynamicJsonDocument doc(4096);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) { setDefaultSettings(); saveSettings(); return false; }

  CFG.p1.name = jsonCStr(doc["p1"]["name"], "Vivien");
  CFG.p1.gender = (Gender)(int)(doc["p1"]["gender"] | 0);

  CFG.p2.name = jsonCStr(doc["p2"]["name"], "Myriam");
  CFG.p2.gender = (Gender)(int)(doc["p2"]["gender"] | 1);

  JsonObject m = doc["marriage"];
  CFG.y   = (int)(m["y"]   | 2025);
  CFG.mon = (int)(m["mon"] | 9);
  CFG.d   = (int)(m["d"]   | 20);
  CFG.hh  = (int)(m["hh"]  | 12);
  CFG.mm  = (int)(m["mm"]  | 10);
  CFG.ss  = (int)(m["ss"]  | 0);

  JsonObject d = doc["display"];
  CFG.font = (FontMode)(int)(d["font"] | (int)FONT_CLASSIC);
  CFG.colorMode = (ColorMode)(int)(d["colorMode"] | (int)COLOR_RAINBOW);
  CFG.fixedColor = (uint8_t)(int)(d["fixedColor"] | 0);

  CFG.font = (FontMode)clampI((int)CFG.font, 0, 3);
  CFG.colorMode = (ColorMode)clampI((int)CFG.colorMode, 0, 2);
  CFG.fixedColor = (uint8_t)clampI((int)CFG.fixedColor, 0, 6);

  if (CFG.mon < 1) CFG.mon = 1;
  if (CFG.mon > 12) CFG.mon = 12;
  if (CFG.d < 1) CFG.d = 1;
  if (CFG.d > 31) CFG.d = 31;

  return true;
}

// =================== LGFX (ECRAN) ===================
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789  _panel;
  lgfx::Bus_Parallel8 _bus;
  lgfx::Light_PWM     _light;
public:
  LGFX() {
    { auto cfg = _bus.config();
      cfg.freq_write = 25000000;
      cfg.pin_wr = 4;  cfg.pin_rd = 2;  cfg.pin_rs = 16;
      cfg.pin_d0 = 15; cfg.pin_d1 = 13; cfg.pin_d2 = 12; cfg.pin_d3 = 14;
      cfg.pin_d4 = 27; cfg.pin_d5 = 25; cfg.pin_d6 = 33; cfg.pin_d7 = 32;
      _bus.config(cfg); _panel.setBus(&_bus);
    }
    { auto cfg = _panel.config();
      cfg.pin_cs   = 17; cfg.pin_rst  = -1; cfg.pin_busy = -1;
      cfg.panel_width  = 240; cfg.panel_height = 320;
      cfg.offset_x = 0; cfg.offset_y = 0; cfg.offset_rotation = 0;
      cfg.readable=false; cfg.invert=false; cfg.rgb_order=false; cfg.dlen_16bit=false; cfg.bus_shared=true;
      _panel.config(cfg);
    }
    { auto cfg = _light.config();
      cfg.pin_bl = 0; cfg.invert=false; cfg.freq=44100; cfg.pwm_channel=7;
      _light.config(cfg); _panel.light(&_light);
    }
    setPanel(&_panel);
  }
};
static LGFX lcd;

// =================== TOUCH ===================
static BBCapTouch touch;
static TOUCHINFO ti;

static const int NX = 6, NY = 3;
static const uint16_t cal_rx[NY][NX] = {
  {167, 195, 187, 194, 198, 239},
  {92, 124, 117, 113, 118, 153},
  {6, 47, 41, 39, 31, 59},
};
static const uint16_t cal_ry[NY][NX] = {
  {7, 76, 132, 190, 245, 317},
  {8, 79, 139, 190, 244, 315},
  {6, 79, 131, 183, 242, 313},
};
static int scr_x[NY][NX];
static int scr_y[NY][NX];

static inline int clampI(int v, int lo, int hi) {
  return (v < lo) ? lo : (v > hi) ? hi : v;
}

static void buildScreenGrid() {
  int w = lcd.width();
  int h = lcd.height();
  for (int j = 0; j < NY; j++) {
    for (int i = 0; i < NX; i++) {
      scr_x[j][i] = MX + ((w - 2 * MX) * i) / (NX - 1);
      scr_y[j][i] = MY + ((h - 2 * MY) * j) / (NY - 1);
    }
  }
}

static bool barycentric(
  float px, float py,
  float x0, float y0,
  float x1, float y1,
  float x2, float y2,
  float &w0, float &w1, float &w2
) {
  float den = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);
  if (fabsf(den) < 1e-6f) return false;
  w0 = ((y1 - y2) * (px - x2) + (x2 - x1) * (py - y2)) / den;
  w1 = ((y2 - y0) * (px - x2) + (x0 - x2) * (py - y2)) / den;
  w2 = 1.0f - w0 - w1;
  const float eps = -0.01f;
  return (w0 >= eps && w1 >= eps && w2 >= eps);
}

static bool mapRawToScreen(uint16_t rx, uint16_t ry, uint16_t &outX, uint16_t &outY) {
  int w = lcd.width();
  int h = lcd.height();
  float px = (float)ry;
  float py = (float)rx;

  for (int j = 0; j < NY - 1; j++) {
    for (int i = 0; i < NX - 1; i++) {
      float u00 = cal_ry[j][i],     v00 = cal_rx[j][i];
      float u10 = cal_ry[j][i+1],   v10 = cal_rx[j][i+1];
      float u01 = cal_ry[j+1][i],   v01 = cal_rx[j+1][i];
      float u11 = cal_ry[j+1][i+1], v11 = cal_rx[j+1][i+1];

      float x00 = scr_x[j][i],     y00 = scr_y[j][i];
      float x10 = scr_x[j][i+1],   y10 = scr_y[j][i+1];
      float x01 = scr_x[j+1][i],   y01 = scr_y[j+1][i];
      float x11 = scr_x[j+1][i+1], y11 = scr_y[j+1][i+1];

      float w0,w1,w2;

      if (barycentric(px,py, u00,v00, u10,v10, u01,v01, w0,w1,w2)) {
        float sx = w0*x00 + w1*x10 + w2*x01;
        float sy = w0*y00 + w1*y10 + w2*y01;
        outX = (uint16_t)clampI((int)lroundf(sx), 0, w-1);
        outY = (uint16_t)clampI((int)lroundf(sy), 0, h-1);
        return true;
      }
      if (barycentric(px,py, u10,v10, u11,v11, u01,v01, w0,w1,w2)) {
        float sx = w0*x10 + w1*x11 + w2*x01;
        float sy = w0*y10 + w1*y11 + w2*y01;
        outX = (uint16_t)clampI((int)lroundf(sx), 0, w-1);
        outY = (uint16_t)clampI((int)lroundf(sy), 0, h-1);
        return true;
      }
    }
  }

  int best_i=0, best_j=0;
  uint32_t bestD = 0xFFFFFFFF;
  for (int j=0;j<NY;j++){
    for(int i=0;i<NX;i++){
      int du = (int)ry - (int)cal_ry[j][i];
      int dv = (int)rx - (int)cal_rx[j][i];
      uint32_t d2 = (uint32_t)(du*du + dv*dv);
      if (d2 < bestD) { bestD=d2; best_i=i; best_j=j; }
    }
  }
  outX = (uint16_t)clampI(scr_x[best_j][best_i], 0, w-1);
  outY = (uint16_t)clampI(scr_y[best_j][best_i], 0, h-1);
  return false;
}

struct TouchEvt {
  bool down=false;
  bool released=false;
  uint16_t x=0,y=0;
  uint16_t x0=0,y0=0;
  uint32_t t0=0;
};
static TouchEvt T;

static bool touchRead(uint16_t &x, uint16_t &y) {
  int n = touch.getSamples(&ti);
  if (n > 0 && ti.count > 0) {
    uint16_t rx = ti.x[0];
    uint16_t ry = ti.y[0];
    mapRawToScreen(rx, ry, x, y);
    return true;
  }
  return false;
}

static void touchUpdate() {
  uint16_t x,y;
  bool isDown = touchRead(x,y);
  T.released = false;

  if (isDown) {
    if (!T.down) {
      T.down = true;
      T.x0 = T.x = x;
      T.y0 = T.y = y;
      T.t0 = millis();
    } else {
      T.x = x; T.y = y;
    }
  } else {
    if (T.down) { T.down = false; T.released = true; }
  }
}

static bool touchTap(uint16_t &x, uint16_t &y, uint16_t maxMove=8, uint16_t maxMs=350) {
  if (!T.released) return false;
  uint32_t dt = millis() - T.t0;
  int dx = (int)T.x - (int)T.x0;
  int dy = (int)T.y - (int)T.y0;
  if (dt <= maxMs && (dx*dx + dy*dy) <= (int)(maxMove*maxMove)) {
    x = T.x0; y = T.y0;
    return true;
  }
  return false;
}

// =================== TIME ===================
static bool timeReady = false;
static bool timeSyncedNTP = false;
static time_t marriageEpoch = 0;
static uint32_t lastNtpTry = 0;

static time_t makeLocalEpoch(int y,int mon,int d,int hh,int mm,int ss) {
  struct tm t{};
  t.tm_year = y - 1900;
  t.tm_mon  = mon - 1;
  t.tm_mday = d;
  t.tm_hour = hh;
  t.tm_min  = mm;
  t.tm_sec  = ss;
  t.tm_isdst = -1;
  return mktime(&t);
}

static bool timeLooksValid() {
  time_t now = time(nullptr);
  return now > 1700000000;
}

static void setupTimeOnce() {
  setenv("TZ", TZ_RULE, 1);
  tzset();
  configTime(0, 0, NTP1, NTP2, NTP3);
}

static bool waitNtp(uint32_t timeoutMs = 2500) {
  struct tm tminfo;
  uint32_t t0 = millis();
  while (millis() - t0 < timeoutMs) {
    if (getLocalTime(&tminfo, 300)) return true;
    delay(150);
  }
  return false;
}

static int monthFromStr(const char* mmm) {
  if (!strncmp(mmm,"Jan",3)) return 1;
  if (!strncmp(mmm,"Feb",3)) return 2;
  if (!strncmp(mmm,"Mar",3)) return 3;
  if (!strncmp(mmm,"Apr",3)) return 4;
  if (!strncmp(mmm,"May",3)) return 5;
  if (!strncmp(mmm,"Jun",3)) return 6;
  if (!strncmp(mmm,"Jul",3)) return 7;
  if (!strncmp(mmm,"Aug",3)) return 8;
  if (!strncmp(mmm,"Sep",3)) return 9;
  if (!strncmp(mmm,"Oct",3)) return 10;
  if (!strncmp(mmm,"Nov",3)) return 11;
  if (!strncmp(mmm,"Dec",3)) return 12;
  return 1;
}

static void setTimeFromCompile() {
  const char* d = __DATE__;
  const char* t = __TIME__;

  char mmm[4] = { d[0], d[1], d[2], 0 };
  int mon = monthFromStr(mmm);
  int day = atoi(d + 4);
  int year = atoi(d + 7);

  int hh = atoi(t);
  int mm = atoi(t + 3);
  int ss = atoi(t + 6);

  struct tm tmv{};
  tmv.tm_year = year - 1900;
  tmv.tm_mon  = mon - 1;
  tmv.tm_mday = day;
  tmv.tm_hour = hh;
  tmv.tm_min  = mm;
  tmv.tm_sec  = ss;
  tmv.tm_isdst = -1;

  time_t epoch = mktime(&tmv);
  struct timeval tv;
  tv.tv_sec = epoch;
  tv.tv_usec = 0;
  settimeofday(&tv, nullptr);

  timeReady = true;
  timeSyncedNTP = false;
}

// =================== ANECDOTES ===================
static String mmddFromDateStr(const String& ymd) {
  if (ymd.length() >= 10) return ymd.substring(5,10);
  return "01-01";
}

static String todayMMDD() {
  time_t now = time(nullptr);
  struct tm lt{};
  localtime_r(&now, &lt);
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d-%02d", lt.tm_mon+1, lt.tm_mday);
  return String(buf);
}

static String fileForMMDD(const String& mmdd) { return "/a_" + mmdd + ".txt"; }

static std::vector<String> loadAnecdotes(const String& mmdd) {
  std::vector<String> out;
  String path = fileForMMDD(mmdd);
  if (!LittleFS.exists(path)) return out;

  File f = LittleFS.open(path, "r");
  if (!f) return out;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) out.push_back(line);
  }
  f.close();
  return out;
}

static bool appendAnecdote(const String& mmdd, const String& text) {
  String t = text;
  t.replace("\r"," ");
  t.replace("\n"," ");
  t.trim();
  if (t.length() == 0) return false;

  File f = LittleFS.open(fileForMMDD(mmdd), "a");
  if (!f) return false;
  f.println(t);
  f.close();
  return true;
}

static bool deleteAnecdote(const String& mmdd, int idx) {
  auto v = loadAnecdotes(mmdd);
  if (idx < 0 || idx >= (int)v.size()) return false;

  v.erase(v.begin() + idx);
  File f = LittleFS.open(fileForMMDD(mmdd), "w");
  if (!f) return false;
  for (auto &s: v) f.println(s);
  f.close();
  return true;
}

// =================== RAINBOW ===================
static uint16_t wheel565(uint8_t pos) {
  pos = 255 - pos;
  uint8_t r,g,b;
  if (pos < 85) {
    r = 255 - pos * 3; g = 0; b = pos * 3;
  } else if (pos < 170) {
    pos -= 85; r = 0; g = pos * 3; b = 255 - pos * 3;
  } else {
    pos -= 170; r = pos * 3; g = 255 - pos * 3; b = 0;
  }
  return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static uint16_t scaleColor565(uint16_t c, float k) {
  uint8_t r = (uint8_t)((c >> 11) & 0x1F);
  uint8_t g = (uint8_t)((c >> 5) & 0x3F);
  uint8_t b = (uint8_t)(c & 0x1F);
  r = (uint8_t)clampI((int)lroundf(r * k), 0, 31);
  g = (uint8_t)clampI((int)lroundf(g * k), 0, 63);
  b = (uint8_t)clampI((int)lroundf(b * k), 0, 31);
  return (uint16_t)((r << 11) | (g << 5) | b);
}

static uint16_t counterColor() {
  if (CFG.colorMode == COLOR_PULSE) {
    uint16_t base = paletteColor(CFG.fixedColor);
    float phase = (float)(millis() % 2000) / 2000.0f;
    float k = 0.35f + 0.65f * (0.5f + 0.5f * sinf(phase * 2.0f * (float)M_PI));
    return scaleColor565(base, k);
  }
  return paletteColor(CFG.fixedColor);
}

static uint16_t currentCounterColor() {
  if (CFG.colorMode == COLOR_RAINBOW) {
    uint8_t hue = (uint8_t)((millis() / 35) & 0xFF);
    return wheel565(hue);
  }
  return counterColor();
}

// =================== WEB ===================
static WebServer server(80);

// (HTML identique à ta version précédente pour garder le message court)
// -> je renvoie une page simple mais complète (fonctionnelle) :
static const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><html lang="fr"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Mariage - Anecdotes</title>
<style>
body{font-family:system-ui,Segoe UI,Roboto,Arial;margin:18px;max-width:860px}
.box{border:1px solid #ddd;border-radius:12px;padding:14px;margin-top:10px}
.row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}
input,select,textarea,button{font-size:16px}
textarea{width:100%;min-height:90px;padding:10px}
button{padding:10px 14px;cursor:pointer}
.small{opacity:.75;font-size:13px}
</style></head><body>
<h1>Mariage - Anecdotes</h1>

<div class="box">
<h2>Anecdotes</h2>
<div class="row">
<label>Date :</label><input id="d" type="date">
<button onclick="refresh()">Voir</button>
</div>
<textarea id="t" placeholder="Écris ton anecdote ici..."></textarea>
<div class="row">
<button onclick="add()">Enregistrer</button>
<span id="msg" class="small"></span>
</div>
<ol id="list"></ol>
</div>

<div class="box">
<h2>Réglages</h2>
<div class="row">
<label>P1 :</label><input id="n1"><select id="g1">
<option value="0">Homme</option><option value="1">Femme</option><option value="2">Autre</option>
</select>
</div>
<div class="row">
<label>P2 :</label><input id="n2"><select id="g2">
<option value="0">Homme</option><option value="1">Femme</option><option value="2">Autre</option>
</select>
</div>
<div class="row">
<label>Police :</label><select id="font">
<option value="0">Classique</option>
<option value="1">Compacte</option>
<option value="2">Large</option>
<option value="3">Jolie</option>
</select>
</div>
<div class="row">
<label>Couleur :</label><select id="colorMode" onchange="toggleColorPick()">
<option value="0">Arc-en-ciel</option>
<option value="1">Fixe</option>
<option value="2">Pulse</option>
</select>
<select id="fixedColor">
<option value="0">Blanc</option>
<option value="1">Rouge</option>
<option value="2">Vert</option>
<option value="3">Bleu</option>
<option value="4">Cyan</option>
<option value="5">Magenta</option>
<option value="6">Jaune</option>
</select>
</div>
<div class="row">
<label>Mariage :</label><input id="mdate" type="datetime-local">
<button onclick="saveSettings()">Sauver</button>
<span id="msg2" class="small"></span>
</div>
</div>

<div class="box">
<h2>Export / Import</h2>
<div class="row"><button onclick="doExport()">Télécharger l'export JSON</button></div>
<textarea id="imp" placeholder='{...json...}'></textarea>
<div class="row"><button onclick="doImport()">Importer</button><span id="msg3" class="small"></span></div>
</div>

<script>
const elD=d=>document.getElementById(d);
function todayISO(){const z=new Date();return `${z.getFullYear()}-${String(z.getMonth()+1).padStart(2,'0')}-${String(z.getDate()).padStart(2,'0')}`;}

async function refresh(){
  const date=elD('d').value||todayISO(); elD('d').value=date;
  const r=await fetch(`/api/list?date=${encodeURIComponent(date)}`); const j=await r.json();
  const L=elD('list'); L.innerHTML='';
  (j.items||[]).forEach((s,i)=>{const li=document.createElement('li'); li.textContent=s;
    const b=document.createElement('button'); b.textContent='Supprimer'; b.style.marginLeft='8px';
    b.onclick=()=>del(i); li.appendChild(b); L.appendChild(li);
  });
  if(!j.items||!j.items.length) L.innerHTML='<li><em>Aucune anecdote pour ce jour.</em></li>';
}

async function add(){
  const date=elD('d').value||todayISO(); const text=elD('t').value.trim();
  if(!text){elD('msg').textContent='Écris quelque chose 🙂';return;}
  const body=new URLSearchParams({date,text});
  const r=await fetch('/api/add',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
  const j=await r.json(); elD('msg').textContent=j.ok?'Enregistré ✅':'Erreur ❌';
  if(j.ok){elD('t').value=''; await refresh();}
}

async function del(index){
  const date=elD('d').value||todayISO();
  const body=new URLSearchParams({date,index:String(index)});
  const r=await fetch('/api/del',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
  await r.json(); await refresh();
}

async function loadSettings(){
  const r=await fetch('/api/settings/get'); const j=await r.json();
  elD('n1').value=j.p1.name||''; elD('g1').value=String(j.p1.gender??0);
  elD('n2').value=j.p2.name||''; elD('g2').value=String(j.p2.gender??1);
  elD('font').value=String(j.display?.font??0);
  elD('colorMode').value=String(j.display?.colorMode??0);
  elD('fixedColor').value=String(j.display?.fixedColor??0);
  toggleColorPick();
  if(j.marriage&&j.marriage.iso) elD('mdate').value=j.marriage.iso;
}
async function saveSettings(){
 const body=new URLSearchParams({
    n1:elD('n1').value,g1:elD('g1').value,
    n2:elD('n2').value,g2:elD('g2').value,
    mdate:elD('mdate').value,
    font:elD('font').value,
    colorMode:elD('colorMode').value,
    fixedColor:elD('fixedColor').value
  });
  const r=await fetch('/api/settings/set',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
  const j=await r.json(); elD('msg2').textContent=j.ok?'Sauvé ✅':'Erreur ❌';
}
function toggleColorPick(){
  const mode=elD('colorMode').value;
  elD('fixedColor').style.display=(mode==='0')?'none':'inline-block';
}
function doExport(){window.location='/api/export';}
async function doImport(){
  const txt=elD('imp').value.trim(); if(!txt){elD('msg3').textContent='Colle un JSON 🙂';return;}
  const body=new URLSearchParams({json:txt});
  const r=await fetch('/api/import',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
  const j=await r.json(); elD('msg3').textContent=j.ok?'Import OK ✅':'Erreur ❌';
  if(j.ok){await refresh(); await loadSettings();}
}
elD('d').value=todayISO(); refresh(); loadSettings();
</script></body></html>
)HTML";

static void handleRoot() { server.send(200, "text/html; charset=utf-8", FPSTR(INDEX_HTML)); }

static void handleList() {
  String date = server.arg("date");
  if (date.length() < 10) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  String mmdd = mmddFromDateStr(date);
  auto v = loadAnecdotes(mmdd);

  String json = "{\"ok\":true,\"mmdd\":\""+mmdd+"\",\"items\":[";
  for (size_t i=0;i<v.size();i++){
    String s=v[i]; s.replace("\\","\\\\"); s.replace("\"","\\\"");
    json += "\"" + s + "\"";
    if (i+1<v.size()) json += ",";
  }
  json += "]}";
  server.send(200, "application/json; charset=utf-8", json);
}

static void handleAdd() {
  String date = server.arg("date");
  String text = server.arg("text");
  if (date.length() < 10) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  String mmdd = mmddFromDateStr(date);
  bool ok = appendAnecdote(mmdd, text);
  server.send(200, "application/json; charset=utf-8", ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

static void handleDel() {
  String date = server.arg("date");
  int idx = server.arg("index").toInt();
  if (date.length() < 10) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  String mmdd = mmddFromDateStr(date);
  bool ok = deleteAnecdote(mmdd, idx);
  server.send(200, "application/json; charset=utf-8", ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

static void handleSettingsGet() {
  DynamicJsonDocument doc(2048);
  doc["ok"] = true;
  doc["p1"]["name"] = CFG.p1.name;
  doc["p1"]["gender"] = (int)CFG.p1.gender;
  doc["p2"]["name"] = CFG.p2.name;
  doc["p2"]["gender"] = (int)CFG.p2.gender;
  doc["display"]["font"] = (int)CFG.font;
  doc["display"]["colorMode"] = (int)CFG.colorMode;
  doc["display"]["fixedColor"] = (int)CFG.fixedColor;

  char iso[20];
  snprintf(iso, sizeof(iso), "%04d-%02d-%02dT%02d:%02d", CFG.y, CFG.mon, CFG.d, CFG.hh, CFG.mm);
  doc["marriage"]["iso"] = iso;

  String out; serializeJson(doc, out);
  server.send(200, "application/json; charset=utf-8", out);
}

static bool parseDateTimeLocal(const String& s, int &y,int &mon,int &d,int &hh,int &mm,int &ss) {
  if (s.length() < 16) return false;
  y = s.substring(0,4).toInt();
  mon = s.substring(5,7).toInt();
  d = s.substring(8,10).toInt();
  hh = s.substring(11,13).toInt();
  mm = s.substring(14,16).toInt();
  ss = 0;
  if (s.length() >= 19 && s[16]==':') ss = s.substring(17,19).toInt();
  if (mon<1||mon>12||d<1||d>31||hh<0||hh>23||mm<0||mm>59||ss<0||ss>59) return false;
  return true;
}

static void handleSettingsSet() {
  String n1 = server.arg("n1");
  String g1 = server.arg("g1");
  String n2 = server.arg("n2");
  String g2 = server.arg("g2");
  String md = server.arg("mdate");
  String font = server.arg("font");
  String colorMode = server.arg("colorMode");
  String fixedColor = server.arg("fixedColor");

  if (n1.length() == 0) n1 = CFG.p1.name;
  if (n2.length() == 0) n2 = CFG.p2.name;

  int yy,mo,dd,hh,mi,ss;
  bool okDt = parseDateTimeLocal(md, yy,mo,dd,hh,mi,ss);

  CFG.p1.name = n1;
  CFG.p1.gender = (Gender)clampI(g1.toInt(), 0, 2);
  CFG.p2.name = n2;
  CFG.p2.gender = (Gender)clampI(g2.toInt(), 0, 2);
  if (font.length()) CFG.font = (FontMode)clampI(font.toInt(), 0, 3);
  if (colorMode.length()) CFG.colorMode = (ColorMode)clampI(colorMode.toInt(), 0, 2);
  if (fixedColor.length()) CFG.fixedColor = (uint8_t)clampI(fixedColor.toInt(), 0, 6);

  if (okDt) { CFG.y=yy; CFG.mon=mo; CFG.d=dd; CFG.hh=hh; CFG.mm=mi; CFG.ss=ss; }

  bool ok = saveSettings();
  marriageEpoch = makeLocalEpoch(CFG.y,CFG.mon,CFG.d,CFG.hh,CFG.mm,CFG.ss);

  DynamicJsonDocument doc(256);
  doc["ok"] = ok;
  String out; serializeJson(doc,out);
  server.send(200, "application/json; charset=utf-8", out);
}

static void handleExport() {
  DynamicJsonDocument doc(32768);

  doc["settings"]["p1"]["name"] = CFG.p1.name;
  doc["settings"]["p1"]["gender"] = (int)CFG.p1.gender;
  doc["settings"]["p2"]["name"] = CFG.p2.name;
  doc["settings"]["p2"]["gender"] = (int)CFG.p2.gender;
  doc["settings"]["display"]["font"] = (int)CFG.font;
  doc["settings"]["display"]["colorMode"] = (int)CFG.colorMode;
  doc["settings"]["display"]["fixedColor"] = (int)CFG.fixedColor;

  JsonObject m = doc["settings"]["marriage"].to<JsonObject>();
  m["y"]=CFG.y; m["mon"]=CFG.mon; m["d"]=CFG.d; m["hh"]=CFG.hh; m["mm"]=CFG.mm; m["ss"]=CFG.ss;

  JsonObject an = doc["anecdotes"].to<JsonObject>();

  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while (file) {
    String name = file.name();
    if (name.startsWith("/a_") && name.endsWith(".txt") && name.length() >= 10) {
      String mmdd = name.substring(3,8);
      JsonArray arr = an[mmdd].to<JsonArray>();
      while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length()) arr.add(line);
      }
    }
    file = root.openNextFile();
  }

  String out; serializeJson(doc, out);
  server.sendHeader("Content-Disposition", "attachment; filename=\"mariage_export.json\"");
  server.send(200, "application/json; charset=utf-8", out);
}

static void handleImport() {
  String txt = server.arg("json");
  if (txt.length() == 0) { server.send(400, "application/json", "{\"ok\":false}"); return; }

  DynamicJsonDocument doc(32768);
  DeserializationError err = deserializeJson(doc, txt);
  if (err) { server.send(400, "application/json", "{\"ok\":false}"); return; }

  if (doc.containsKey("settings")) {
    JsonObject s = doc["settings"];
    CFG.p1.name = jsonCStr(s["p1"]["name"], CFG.p1.name.c_str());
    CFG.p1.gender = (Gender)(int)(s["p1"]["gender"] | (int)CFG.p1.gender);
    CFG.p2.name = jsonCStr(s["p2"]["name"], CFG.p2.name.c_str());
    CFG.p2.gender = (Gender)(int)(s["p2"]["gender"] | (int)CFG.p2.gender);

    JsonObject mm = s["marriage"];
    CFG.y   = (int)(mm["y"]   | CFG.y);
    CFG.mon = (int)(mm["mon"] | CFG.mon);
    CFG.d   = (int)(mm["d"]   | CFG.d);
    CFG.hh  = (int)(mm["hh"]  | CFG.hh);
    CFG.mm  = (int)(mm["mm"]  | CFG.mm);
    CFG.ss  = (int)(mm["ss"]  | CFG.ss);

    JsonObject disp = s["display"];
    CFG.font = (FontMode)(int)(disp["font"] | (int)CFG.font);
    CFG.colorMode = (ColorMode)(int)(disp["colorMode"] | (int)CFG.colorMode);
    CFG.fixedColor = (uint8_t)(int)(disp["fixedColor"] | (int)CFG.fixedColor);
    CFG.font = (FontMode)clampI((int)CFG.font, 0, 3);
    CFG.colorMode = (ColorMode)clampI((int)CFG.colorMode, 0, 2);
    CFG.fixedColor = (uint8_t)clampI((int)CFG.fixedColor, 0, 6);

    saveSettings();
    marriageEpoch = makeLocalEpoch(CFG.y,CFG.mon,CFG.d,CFG.hh,CFG.mm,CFG.ss);
  }

  if (doc.containsKey("anecdotes")) {
    JsonObject a = doc["anecdotes"];
    for (JsonPair kv : a) {
      String mmdd = kv.key().c_str();
      if (mmdd.length() != 5 || mmdd[2] != '-') continue;

      File f = LittleFS.open(fileForMMDD(mmdd), "w");
      if (!f) continue;

      JsonArray arr = kv.value().as<JsonArray>();
      for (JsonVariant v : arr) {
        const char* line = v.as<const char*>();
        if (line && *line) f.println(line);
      }
      f.close();
    }
  }

  server.send(200, "application/json; charset=utf-8", "{\"ok\":true}");
}

static void setupWeb() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/list", HTTP_GET, handleList);
  server.on("/api/add", HTTP_POST, handleAdd);
  server.on("/api/del", HTTP_POST, handleDel);

  server.on("/api/settings/get", HTTP_GET, handleSettingsGet);
  server.on("/api/settings/set", HTTP_POST, handleSettingsSet);

  server.on("/api/export", HTTP_GET, handleExport);
  server.on("/api/import", HTTP_POST, handleImport);

  server.begin();
}

// =================== UI ===================
enum ViewMode { VIEW_COUNTDOWN, VIEW_ANECDOTE };
static ViewMode viewMode = VIEW_COUNTDOWN;

static std::vector<String> todays;
static int anecIndex = 0;
static String mmddToday = "";

static uint32_t lastSecondTick = 0;
static uint32_t lastRainbowTick = 0;
static uint16_t lastRainbowColor = 0xFFFF;

// Draw helpers
static void applyDefaultFont() {
  lcd.setFont(nullptr);
  lcd.setTextSize(1);
}

static int applyCountdownFont() {
  switch (CFG.font) {
    case FONT_COMPACT:
      lcd.setFont(&fonts::Font2);
      return 1;
    case FONT_LARGE:
      lcd.setFont(&fonts::Font4);
      return 1;
    case FONT_PRETTY:
      lcd.setFont(&fonts::Font7);
      return 1;
    default:
      lcd.setFont(nullptr);
      return 5;
  }
}

static void drawCenteredText(const String& s, int y, int textSize, uint16_t color, uint16_t bg) {
  lcd.setTextSize(textSize);
  lcd.setTextColor(color, bg);
  int w = lcd.textWidth(s);
  int x = (lcd.width() - w) / 2;
  if (x < 0) x = 0;
  lcd.setCursor(x, y);
  lcd.print(s);
}

static void drawNamesTop() {
applyDefaultFont();
  lcd.setTextSize(2);
  String sep = "  &  ";
  int w1 = lcd.textWidth(CFG.p1.name);
  int ws = lcd.textWidth(sep);
  int w2 = lcd.textWidth(CFG.p2.name);
  int total = w1 + ws + w2;

  int x = (lcd.width() - total) / 2;
  int y = 12;

  lcd.setCursor(x, y);
  lcd.setTextColor(genderColor(CFG.p1.gender), TFT_BLACK);
  lcd.print(CFG.p1.name);

  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.print(sep);

  lcd.setTextColor(genderColor(CFG.p2.gender), TFT_BLACK);
  lcd.print(CFG.p2.name);
}

static void drawBottomBar() {
  applyDefaultFont();
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  char start[32];
  snprintf(start, sizeof(start), "depuis %02d/%02d/%04d %02d:%02d",
           CFG.d, CFG.mon, CFG.y, CFG.hh, CFG.mm);

  int w = lcd.textWidth(start);
  lcd.setCursor(lcd.width() - w - 6, lcd.height() - 16);
  lcd.print(start);

  if (timeReady) {
    time_t now = time(nullptr);
    struct tm lt{};
    localtime_r(&now, &lt);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", lt.tm_hour, lt.tm_min, lt.tm_sec);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setCursor(6, lcd.height() - 16);
    lcd.print(buf);
  } else {
    lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
    lcd.setCursor(6, lcd.height() - 16);
    lcd.print("heure: --:--:--");
  }
}

static void drawBigCounter(uint16_t color) {
  lcd.fillRect(0, 70, lcd.width(), 190, TFT_BLACK);

  if (!timeReady) {
    applyDefaultFont();
    drawCenteredText("sync heure...", 115, 2, TFT_CYAN, TFT_BLACK);
    drawCenteredText("patiente",      145, 2, TFT_CYAN, TFT_BLACK);
    return;
  }

  time_t now = time(nullptr);
  long delta = (long)difftime(now, marriageEpoch);
  if (delta < 0) delta = 0;

  long days = delta / 86400L;
  long rem = delta % 86400L;
  long hh = rem / 3600L;
  rem %= 3600L;
  long mm = rem / 60L;
  long ss = rem % 60L;

  String line1 = String(days) + " jours";
  char buf[16];
  snprintf(buf, sizeof(buf), "%02ld:%02ld:%02ld", hh, mm, ss);

  int size = applyCountdownFont();
  const int yOffset = -15;
  drawCenteredText(line1, 105 + yOffset, size, color, TFT_BLACK);
  drawCenteredText(String(buf), 160 + yOffset, size, color, TFT_BLACK);
}

static void drawCountdownScreenFull() {
  lcd.fillScreen(TFT_BLACK);
  drawNamesTop();
  drawBigCounter(lastRainbowColor);
  drawBottomBar();
}

static void enterAnecdoteMode() {
  mmddToday = timeReady ? todayMMDD() : String("01-01");
  todays = loadAnecdotes(mmddToday);
  anecIndex = 0;
  viewMode = VIEW_ANECDOTE;
  // (écran anecdotes simplifié pour rester lisible)
  lcd.fillScreen(TFT_BLACK);
  applyDefaultFont();
  drawCenteredText("Anecdote du jour", 10, 2, TFT_WHITE, TFT_BLACK);
  if (todays.empty()) {
    lcd.setTextSize(2);
    lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    lcd.setCursor(10, 90);
    lcd.print("Aujourd'hui, il faut\nvous creer votre\nmoment a vous.");
  } else {
    lcd.setTextSize(2);
    lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    lcd.setCursor(10, 80);
    lcd.print(todays[0]);
  }
}

static void exitAnecdoteMode() {
  viewMode = VIEW_COUNTDOWN;
  drawCountdownScreenFull();
}

static void handleTap(uint16_t x, uint16_t y) {
  (void)x; (void)y;
  if (viewMode == VIEW_COUNTDOWN) enterAnecdoteMode();
  else exitAnecdoteMode();
}

// =================== WIFI ===================
static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  WiFi.config(IP_LOCAL, IP_GATEWAY, IP_SUBNET, IP_DNS1, IP_DNS2);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis()-t0 < 15000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK, IP="); Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi FAIL");
  }
}

// =================== LOOP APP ===================
static void loopApp() {
  server.handleClient();

  // NTP retry doux
  if (WiFi.status() == WL_CONNECTED && !timeSyncedNTP) {
    if (millis() - lastNtpTry > 15000) {
      lastNtpTry = millis();
      if (waitNtp(2500) && timeLooksValid()) {
        timeReady = true;
        timeSyncedNTP = true;
        marriageEpoch = makeLocalEpoch(CFG.y,CFG.mon,CFG.d,CFG.hh,CFG.mm,CFG.ss);
        drawCountdownScreenFull();
      }
    }
  }

  // Touch
  uint16_t x,y;
  if (touchTap(x,y)) handleTap(x,y);

  // Rainbow
  if (viewMode == VIEW_COUNTDOWN && timeReady) {
    if (millis() - lastRainbowTick >= 60) {
      lastRainbowTick = millis();

      uint16_t col = currentCounterColor();
      if (col != lastRainbowColor) {
        lastRainbowColor = col;
        drawBigCounter(lastRainbowColor);
        drawBottomBar();
      }
    }
  }

  if (viewMode == VIEW_COUNTDOWN && (millis() - lastSecondTick >= 1000)) {
    lastSecondTick = millis();
    if (!timeReady) drawCountdownScreenFull();
  }
}

// =================== SETUP/LOOP ===================
void setup() {
  Serial.begin(115200);
  delay(250);

  lcd.init();
  lcd.setRotation(ROT);
  lcd.setBrightness(255);

  buildScreenGrid();

  Wire.begin(21,22);
  Wire.setClock(400000);

  int rc = touch.init(21,22,-1,-1);
  Serial.printf("touch.init rc=%d sensorType=%d\n", rc, touch.sensorType());

  if (!LittleFS.begin(true)) Serial.println("LittleFS mount FAIL");
  else Serial.println("LittleFS OK");

  loadSettings();

  connectWiFi();
  setupWeb();

  setupTimeOnce();
  if (WiFi.status() == WL_CONNECTED && waitNtp(6000) && timeLooksValid()) {
    timeReady = true;
    timeSyncedNTP = true;
  } else {
    setTimeFromCompile();
  }

  marriageEpoch = makeLocalEpoch(CFG.y,CFG.mon,CFG.d,CFG.hh,CFG.mm,CFG.ss);
  lastRainbowColor = currentCounterColor();

  drawCountdownScreenFull();
}

void loop() {
  touchUpdate();
  loopApp();
  delay(10);
}
