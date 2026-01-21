# ❤️ LoveCount (ESP32_2432S022)

**LoveCount** est un petit “compteur d’amour” pour ESP32_2432S022 : il affiche en grand **le temps écoulé depuis une date importante** (rencontre, mariage, emménagement, naissance, etc.) sous la forme :

- **X jours**
- **hh:mm:ss**

En plus du compteur, tu peux afficher une **Anecdote du jour** (petits souvenirs associés à une date), et personnaliser l’ambiance avec **animations**, **couleurs** et **polices**.

![LoveCount demo](minipresentationlovecount.gif)

🎥 Vidéo (démo) : https://youtu.be/BuQu25AQVwY

---

## ✨ Fonctionnalités

- **Compteur** depuis une date/heure (jours + hh:mm:ss)
- **Heure automatique**
  - Sync via **NTP** si Wi-Fi OK
  - Sinon fallback sur l’heure de compilation (ça démarre quand même)
- **Personnalisation**
  - Prénoms (P1 & P2) + genre (couleur des prénoms)
  - **Police** (plus lisible / plus “classe” / plus compacte…)
  - **Couleur du compteur** : Arc-en-ciel / Fixe / Pulse
- **Animations** (coeur + autres si présents)
  - Animation choisie **par jour (MM-DD)** via la page web
- **Anecdote du jour**
  - Ajout / suppression d’anecdotes par date via la page web
  - Affichage sur l’écran au touch
- **Interface Web** intégrée (depuis ton téléphone/PC)
- **Export / Import JSON** (sauvegarde complète réglages + anecdotes)

---

## 📦 Contenu du repo

- `LoveCountavecSD/` : version avec sauvegarde sur **microSD** (JSON)
- `LoveCountsansSD/` : version sans microSD (sauvegarde **interne ESP32**)
- `minipresentationlovecount.gif` : GIF de démonstration (utilisé dans ce README)

> Les deux versions ont le même comportement. Seule la manière de sauvegarder change.

---

## 🧠 À quoi ça sert (idées d’usage)

- Compteur depuis la **rencontre**
- Compteur depuis le **mariage**
- Compteur depuis la **naissance**
- Compteur depuis l’**emménagement**
- Objet “déco” / “souvenir” posé dans la maison

Le but est d’avoir un écran vivant, configurable sans recompiler, qui affiche un compteur émotionnel… et des petites phrases souvenirs.

---

## 🖥️ Utilisation sur l’écran

### Écran principal : le compteur
Tu vois :
- En haut : les **prénoms** (P1 & P2)
- En haut à droite : une **animation** (coeur, étoile, etc.)
- Au centre : le **nombre de jours** + **hh:mm:ss**
- En bas : l’heure actuelle + “depuis JJ/MM/AAAA HH:MM”

### 🖐️ Mode “Anecdote du jour”
- **Tap** sur l’écran → affiche “Anecdote du jour”
- Si plusieurs anecdotes existent pour la date du jour, tu peux naviguer (selon la zone tapée)
- Après quelques secondes sans interaction, l’écran revient tout seul au compteur
- Si aucune anecdote n’existe, un message s’affiche pour “inviter à créer un moment”

---

## ✨ Animations, police, couleurs : à quoi ça sert ?

### Animations
Elles rendent l’écran vivant (petit “cadre animé”).  
Tu peux même définir une animation **différente selon le jour** (MM-DD). Exemple :
- 14-02 → coeur
- 20-09 → etoile
- 01-01 → goutte

### Police
Permet d’améliorer la lisibilité et le style (compact / grand / élégant / gras…).

### Couleur du compteur
- **Arc-en-ciel** : change en continu
- **Fixe** : une couleur unique
- **Pulse** : “respire” avec une intensité variable

Tout se règle facilement via la **page web**.

---

## 🌐 Page Web (panneau de contrôle)

Le projet embarque une interface web pour :
- écrire / supprimer des **anecdotes** par date
- choisir l’**animation** du jour
- changer prénoms, genres
- régler la date/heure de départ
- choisir police & couleurs
- exporter / importer un JSON complet

Adresse par défaut :
- **http://192.168.1.50/**

> Si ton réseau n’est pas en `192.168.1.x`, change l’IP fixe dans le code.

---

## 🧰 Matériel

- **ESP32 2432S022** (écran 240×320 ST7789 + tactile)
- Wi-Fi 2.4GHz
- (Option) **microSD** si tu utilises la version `avecSD`

---

## 🔧 Installation / Compilation (Arduino IDE)

### 1) Installer le support ESP32
- Arduino IDE 2.x
- `Outils → Type de carte → Gestionnaire de cartes…`
- Cherche **ESP32 by Espressif Systems** et installe-le

### 2) Installer les bibliothèques
`Outils → Gérer les bibliothèques…`
- **LovyanGFX**
- **ArduinoJson** (v7 recommandé)
- **bb_captouch**

### 3) Ouvrir le bon sketch
- `LoveCountsansSD/...ino` **ou**
- `LoveCountavecSD/...ino`

### 4) Choisir la carte
Menu : `Outils → Type de carte → esp32 → ESP32 Dev Module`

> Sur la plupart des 2432S022, **ESP32 Dev Module** fonctionne très bien.

### 5) Brancher et choisir le PORT (IMPORTANT)
- Branche la carte en USB
- `Outils → Port` → sélectionne le port qui apparaît (ex: `COM5`)

Astuce : débranche/rebranche l’USB, le port qui disparaît puis réapparaît = le bon.

### 6) Mettre ton Wi-Fi (SSID / mot de passe)
Dans le code, remplace :

```cpp
static const char* WIFI_SSID = "SSID";
static const char* WIFI_PASS = "PASS";
```

par tes identifiants Wi-Fi.

### 7) IP fixe (accès web)
Par défaut, le projet utilise :

```cpp
static const IPAddress IP_LOCAL(192,168,1,50);
```

Donc l’interface web est :
- **http://192.168.1.50/**

⚠️ Vérifie que :
- ton réseau est bien en `192.168.1.x`
- `192.168.1.50` n’est pas déjà utilisé (conflit IP)

### 8) Téléverser
Clique sur **Téléverser** (flèche →).

Tu peux ouvrir le **Moniteur série** (115200) pour voir :
- `WiFi OK, IP=...` ou `WiFi FAIL`

---

## 📝 Modifier les prénoms / la date : web ou code ?

✅ **Recommandé : via la page web**  
C’est instantané, pas besoin de recompiler.

### Option : modifier dans le code (valeurs par défaut)
Dans `setDefaultSettings()` :

**Prénoms :**
```cpp
CFG.p1.name = "Messieur";
CFG.p2.name = "Madame";
```

**Date/heure de départ :**
```cpp
CFG.y=2020; CFG.mon=1; CFG.d=1; CFG.hh=1; CFG.mm=0; CFG.ss=0;
```

---

## 🐛 Dépannage rapide

- **Je n’ai pas la page web**
  - Vérifie que tu es sur le même réseau
  - Vérifie l’IP fixe (192.168.1.50) ou change-la
  - Regarde le Moniteur série pour l’état Wi-Fi

- **WiFi FAIL**
  - SSID / mot de passe incorrect
  - réseau 5GHz uniquement (il faut du 2.4GHz)
  - conflit IP si IP fixe

- **Tactile décalé**
  - Les valeurs de calibration sont spécifiques (à ajuster si besoin)

---

## 📜 Licence
À définir (MIT/GPL/… ou laisser tel quel).

---

## 🙌 Crédits
- LovyanGFX
- ArduinoJson
- bb_captouch

Projet : **LoveCount** — par Inter-Raptor
