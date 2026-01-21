# ❤️ LoveCount (ESP32_2432S022)

**LoveCount** is a little “love counter” for the ESP32_2432S022: it displays **the time elapsed since an important date** (first meeting, wedding, moving in together, birth, etc.) in this format:

- **X days**
- **hh:mm:ss**

On top of the counter, you can show a **Daily Anecdote** (short memories linked to a date) and customize the vibe with **animations**, **colors**, and **fonts**.

![LoveCount demo](minipresentationlovecount.gif)

🎥 Video (demo): https://youtu.be/BuQu25AQVwY

---

## ✨ Features

- **Counter** since a chosen date/time (days + hh:mm:ss)
- **Automatic time**
  - Sync via **NTP** if Wi‑Fi is OK
  - Otherwise fallback to the compile time (it still boots and runs)
- **Customization**
  - Names (P1 & P2) + gender (name color)
  - **Font** (more readable / classy / compact…)
  - **Counter color**: Rainbow / Fixed / Pulse
- **Animations** (heart + more if present)
  - Animation can be set **per day (MM-DD)** from the web page
- **Daily Anecdote**
  - Add / delete anecdotes per date from the web page
  - Display on screen via touch
- Built‑in **Web UI** (phone/PC)
- **JSON Export / Import** (full backup of settings + anecdotes)

---

## 📦 Repository content

- `LoveCountavecSD/` : version with **microSD** save (JSON)
- `LoveCountsansSD/` : version without microSD (saves **inside the ESP32**)
- `minipresentationlovecount.gif` : demo GIF (used in this README)

> Both versions behave the same. Only the storage method changes.

---

## 🧠 What is it for? (use ideas)

- Counter since your **first meeting**
- Counter since your **wedding**
- Counter since a **birth**
- Counter since **moving in together**
- A small “memory / decor” object on a shelf

The goal is to have a living screen, configurable without recompiling, showing an emotional counter… plus short daily memories.

---

## 🖥️ On‑screen usage

### Main screen: the counter
You’ll see:
- Top: the **names** (P1 & P2)
- Top‑right: an **animation** (heart, star, etc.)
- Center: **days** + **hh:mm:ss**
- Bottom: current time + “since DD/MM/YYYY HH:MM”

### 🖐️ “Daily Anecdote” mode
- **Tap** the screen → shows “Daily Anecdote”
- If multiple anecdotes exist for today, you can navigate (depending on where you tap)
- After a few seconds without interaction, it automatically returns to the counter
- If no anecdote exists, a small message is shown to “invite you to create a moment”

---

## ✨ Animations, fonts, colors: why?

### Animations
They make the screen feel alive (like a tiny animated frame).
You can even set a different animation **per day** (MM‑DD). Example:
- 02‑14 → heart
- 09‑20 → star
- 01‑01 → drop

### Font
Lets you improve readability and style (compact / large / elegant / bold…).

### Counter color
- **Rainbow**: continuously changing color
- **Fixed**: one color only
- **Pulse**: “breathing” brightness effect

Everything is easy to change from the **web page**.

---

## 🌐 Web page (control panel)

The ESP32 hosts a web interface to:
- write / delete **anecdotes** by date
- choose today’s **animation** by date
- change names and genders
- set the counter start date/time
- choose fonts & colors
- export / import a full JSON file

Default address:
- **http://192.168.1.50/**

> If your LAN is not `192.168.1.x`, change the fixed IP in the code.

---

## 🧰 Hardware

- **ESP32 2432S022** (240×320 ST7789 display + touch)
- 2.4GHz Wi‑Fi
- (Optional) **microSD** if you use the `avecSD` version

---

## 🔧 Installation / Build (Arduino IDE)

### 1) Install ESP32 support
- Arduino IDE 2.x
- `Tools → Board → Boards Manager…`
- Search **ESP32 by Espressif Systems** and install it

### 2) Install libraries
`Tools → Manage Libraries…`
- **LovyanGFX**
- **ArduinoJson** (v7 recommended)
- **bb_captouch**

### 3) Open the correct sketch
- `LoveCountsansSD/...ino` **or**
- `LoveCountavecSD/...ino`

### 4) Select the board
Menu: `Tools → Board → esp32 → ESP32 Dev Module`

> On most 2432S022 boards, **ESP32 Dev Module** works fine.

### 5) Plug the board and select the PORT (IMPORTANT)
- Plug the board via USB
- `Tools → Port` → select the port that appears (e.g. `COM5` on Windows)

Tip: unplug/replug — the port that disappears and reappears is the correct one.

### 6) Set your Wi‑Fi (SSID / password)
In the code, replace:

```cpp
static const char* WIFI_SSID = "SSID";
static const char* WIFI_PASS = "PASS";
```

with your Wi‑Fi credentials.

### 7) Fixed IP (web access)
By default the project uses:

```cpp
static const IPAddress IP_LOCAL(192,168,1,50);
```

So the web UI is:
- **http://192.168.1.50/**

⚠️ Make sure:
- your network is `192.168.1.x`
- `192.168.1.50` is not already used (IP conflict)

### 8) Upload
Click **Upload**.

You can open the **Serial Monitor** (115200) to see:
- `WiFi OK, IP=...` or `WiFi FAIL`

---

## 📝 Editing names / date: web or code?

✅ **Recommended: use the web page**  
Instant changes, no recompilation.

### Option: edit defaults in the code
In `setDefaultSettings()`:

**Names:**
```cpp
CFG.p1.name = "Messieur";
CFG.p2.name = "Madame";
```

**Start date/time:**
```cpp
CFG.y=2020; CFG.mon=1; CFG.d=1; CFG.hh=1; CFG.mm=0; CFG.ss=0;
```

---

## 🐛 Quick troubleshooting

- **Web page not reachable**
  - Check you are on the same network
  - Check the fixed IP (192.168.1.50) or change it
  - Look at Serial Monitor for Wi‑Fi status

- **WiFi FAIL**
  - Wrong SSID / password
  - 5GHz‑only network (ESP32 needs 2.4GHz)
  - IP conflict when using fixed IP

- **Touch is offset**
  - Calibration values are board‑specific (adjust if needed)

---

## 📜 License
To be defined (MIT/GPL/… or leave as‑is).

---

## 🙌 Credits
- LovyanGFX
- ArduinoJson
- bb_captouch

Project: **LoveCount** — by Inter-Raptor
