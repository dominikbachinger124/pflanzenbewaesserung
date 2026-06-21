# 🌱 ESP32 Automatische Bewässerungsanlage – Projektkontext

> **Datei erstellt:** 21.06.2026  
> **Status:** Hardware verkabelt ✅, Software in Entwicklung 🛠️

---

## 🎯 Ziel des Projekts

Solarbetriebene automatische Pflanzenbewässerung mit ESP32-S3, DC-Pumpe, Akku-Überwachung und LCD-Display. Der ESP32 wacht stündlich aus dem Deep Sleep auf, misst den Batteriestand und bewässert bei ausreichend Ladung für 5 Minuten.

---

## 🧩 Hardware

| Komponente | Spezifikation |
|---|---|
| **Controller** | LilyGO T-Display S3 (ESP32-S3R8, 1.9" LCD, ST7789, 170×320) |
| **Pumpe** | DC Wasserpumpe 5V |
| **Schalter** | MOSFET IRLZ44N (Logik-Pegel) |
| **Schutzdiode** | Freilaufdiode 1N4007 (parallel zur Pumpe) |
| **Akku** | LiPo Akku (Solar-Panel → Laderegler → LiPo) |
| **Boost** | MT3608 Step-Up Converter (5V Ausgang für Pumpe) |
| **Vorwiderstand** | 100Ω (zwischen GPIO 13 und MOSFET Gate) |

---

## 🔌 Pin-Belegung

| Funktion | GPIO | Bemerkung |
|---|---|---|
| MOSFET Gate (Pumpe) | **GPIO 13** | Über 100Ω Vorwiderstand |
| Batterie ADC | **GPIO 4** | Spannungsteiler 100k/100kΩ |
| LCD Backlight | **GPIO 38** | High = Backlight an |
| LCD Power On | **GPIO 15** | High = Display an |

> **Hinweis:** Die LCD-Datenpins (D0–D7, WR, RD, CS, RES, DC) werden intern über **TFT_eSPI** verwaltet – kein externer I2C-Adapter nötig!

---

## ⚡ Schaltplan Übersicht

```
Solarmodul → Laderegler → LiPo Akku (V_BATT)
                          │
                          ▼
               ┌────────────────────┐
               │   MT3608 5V        │
               │   (Step-Up)        │
               └─────────┬──────────┘
                         │
              Pumpe(+) ──┤
                         │
Pumpe(-) ──→ Diode ──→ MOSFET Drain
                      │
                      ├── Gate ← GPIO 13 (+ 100Ω Vorwiderstand)
                      │
                      └── Source → GND
```

**Spannungsteiler für ADC:**
```
V_BATT ──┬── 100kΩ ──→ GPIO 4 ── 100kΩ ── GND

ADC-Berechnung: V_BATT = ADC × 3.3V / 4095 × 2
Kalibration: 3.0V = 0%, 4.2V = 100%
```

---

## 📚 Bibliotheken (Arduino / PlatformIO)

```bash
# TFT_eSPI (Bodmer) – Displayansteuerung
# esp_sleep.h – Deep Sleep (in ESP32 Framework enthalten)
```

**TFT_eSPI Konfiguration:**
1. Arduino IDE → Library Manager → **TFT_eSPI von Bodmer** installieren
2. Datei öffnen: `libraries/TFT_eSPI/User_Setup_Select.h`
3. Diese Zeile **entkommentieren** (auskommentieren entfernen):

```cpp
#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>
```

---

## ⚙️ Funktionsablauf

```
┌──────────────────────────────────────┐
│           ESP32 START                │
└──────────────┬───────────────────────┘
               ▼
┌──────────────────────────────────────┐
│ 1. GPIO 15 HIGH (LCD Power)          │
│ 2. GPIO 38 HIGH (Backlight)          │
│ 3. TFT_eSPI Display initialisieren   │
└──────────────┬───────────────────────┘
               ▼
┌──────────────────────────────────────┐
│ 4. Batterie ADC messen (GPIO 4)      │
│    → Akkustand in % berechnen        │
└──────────────┬───────────────────────┘
               ▼
        ┌──────┴──────┐
        │ Akku ≥ 20%? │
        └──────┬──────┘
          Ja   │   Nein
          ▼    │    ▼
  ┌────────────┐  ┌─────────────┐
  │ LCD:       │  │ LCD:       │
  │ "Bewässerung"│ │ "Akku leer!" │
  │ GPIO13 HIGH│  │ Pumpe aus   │
  │ Pumpe an   │  └──────┬──────┘
  │ 5 Min warten│        ▼
  │ GPIO13 LOW │  ┌─────────────┐
  │ Pumpe aus  │  │ 10 Sek warten│
  └──────┬─────┘  └──────┬──────┘
         │               │
         └───────┬───────┘
                 ▼
┌──────────────────────────────────────┐
│ LCD: "Sleep in X Sek..."            │
│ GPIO 38 LOW, GPIO 15 LOW (LCD aus)  │
│ Deep Sleep für 1 Stunde             │
└──────────────────────────────────────┘
```

---

## 📄 Aktueller Code-Stand

**Datei:** `main.cpp`

```cpp
// === TFT_eSPI Setup ===
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

// === Pin Definitionen ===
#define PUMPE_PIN       13
#define BACKLIGHT_PIN   38
#define LCD_POWER_PIN   15
#define ADC_PIN         4

// === Konfiguration ===
#define SCHWELLWERT_AKKU      20    // % Minimum für Bewässerung
#define BEWAESSERUNGSZEIT     300   // Sekunden (5 Min)
#define SCHLAFZEIT_SEKUNDEN   3600  // 1 Stunde

// === Akku-Spannung messen ===
int leseAkkustand() {
    int adc = analogRead(ADC_PIN);
    float spannung = adc * 3.3f / 4095.0f * 2.0f;  // ×2 wegen Spannungsteiler
    float prozent = (spannung - 3.0f) / (4.2f - 3.0f) * 100.0f;
    return constrain((int)prozent, 0, 100);
}

// === Setup ===
void setup() {
    pinMode(PUMPE_PIN, OUTPUT);
    pinMode(BACKLIGHT_PIN, OUTPUT);
    pinMode(LCD_POWER_PIN, OUTPUT);
    pinMode(ADC_PIN, INPUT);

    digitalWrite(LCD_POWER_PIN, HIGH);
    digitalWrite(BACKLIGHT_PIN, HIGH);
    delay(100);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    int akku = leseAkkustand();

    if (akku >= SCHWELLWERT_AKKU) {
        // --- Bewässerung ---
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("Akku: " + String(akku) + "%", 10, 10, 2);
        tft.drawString("BEWAESSERUNG", 10, 40, 2);
        
        digitalWrite(PUMPE_PIN, HIGH);
        delay(BEWAESSERUNGSZEIT * 1000UL);
        digitalWrite(PUMPE_PIN, LOW);

    } else {
        // --- Akku leer ---
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Akku: " + String(akku) + "%", 10, 10, 2);
        tft.drawString("AKKU LEER!", 10, 40, 2);
        delay(10000);
    }

    // --- Deep Sleep vorbereiten ---
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Sleep...", 10, 80, 2);
    delay(1000);

    digitalWrite(BACKLIGHT_PIN, LOW);
    digitalWrite(LCD_POWER_PIN, LOW);

    esp_deep_sleep_start();
}

// === Loop (nie erreicht) ===
void loop() {}
```

---

## 📋 Offene Punkte / TODO

- [ ] **TFT_eSPI User_Setup_Select.h konfigurieren** – Setup206 aktivieren
- [ ] Batterie-ADC-Messung kalibrieren (Realer Akkustand testen)
- [ ] LCD-Layout gestalten (Fortschrittsbalken, Countdown)
- [ ] Deep Sleep korrekt konfigurieren
- [ ] PlatformIO-Projektstruktur erstellen ODER Arduino IDE Projekt
- [ ] Freilaufdiode + Spannungsteiler auf Funktion testen

---

## 💻 VS Code Konfiguration

### Benötigte Extensions:
- **[PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)** – ESP32-Entwicklung
- **GitHub Copilot** oder **Cursor AI** – für Kontexthilfe (diese Datei nutzen!)

### platformio.ini:
```ini
[env:lilygo-t-display-s3]
platform = espressif32
board = esp32s3
framework = arduino
lib_deps = bodmer/TFT_eSPI@^2.5.43
board_build.partitions = huge_app.csv
upload_speed = 921600
monitor_speed = 115200
```

### Board-Einstellungen:
| Einstellung | Wert |
|---|---|
| **Board** | esp32s3 |
| **Framework** | Arduino |
| **Upload Speed** | 921600 |
| **Monitor Speed** | 115200 |

---

## 🔄 Prompt zum Wiederstarten

Kopiere den folgenden Text in deinen VS Code AI Assistant (GitHub Copilot Chat, Cursor AI, etc.) um die Sitzung fortzusetzen:

---

```
## 📌 Kontext für VS Code AI Assistant

Ich programmiere ein ESP32-Projekt: Automatische Pflanzenbewässerung.

### Hardware:
- LilyGO T-Display S3 (ESP32-S3R8, 1.9" LCD ST7789 170x320)
- DC Pumpe 5V gesteuert mit MOSFET IRLZ44N an GPIO 13 (mit 100Ω Vorwiderstand)
- Freilaufdiode 1N4007 parallel zur Pumpe
- LiPo Akku mit MT3608 Step-Up auf 5V
- Batterie-ADC an GPIO 4 (Spannungsteiler 100k/100kΩ)
- LCD Backlight GPIO 38, LCD Power GPIO 15

### Ziel:
ESP32 wacht jede Stunde aus Deep Sleep auf, misst Akkustand, bewässert 5 Min wenn Akku ≥ 20%, zeigt alles auf integriertem LCD (TFT_eSPI).

### Aktuelle Aufgabe:
[HIER EIGENE FRAGE EINFÜGEN]

### Letzter Stand:
- Hardware ist verkabelt ✅
- TFT_eSPI Setup206_LilyGo_T_Display_S3.h muss aktiviert werden
- Code in main.cpp mit Akku-Messung und Bewässerungslogik vorhanden
```

---

## 🗂️ Dateistruktur (Vorschlag)

```
esp32-bewaesserung/
├── platformio.ini
├── src/
│   └── main.cpp
├── lib/
│   └── TFT_eSPI/          # (via lib_deps)
├── include/
│   └── README
└── docs/
    └── schaltplan.png
```

---

> 💡 **Tipp:** Diese Datei in `.vscode/` oder im Projekt-Root ablegen und bei Fragen an den AI Assistant referenzieren!
