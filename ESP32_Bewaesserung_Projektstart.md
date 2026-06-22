# 🌱 ESP32 Automatische Bewässerungsanlage – Projektkontext

> **Datei erstellt:** 21.06.2026  
> **Status:** Hardware verkabelt ✅, Software in Entwicklung 🛠️  
> **Letztes Update:** Intelligente Bewässerung mit Sensoren ✅

---

## 🎯 Ziel des Projekts

Solarbetriebene automatische Pflanzenbewässerung mit ESP32-S3, DC-Pumpe, Akku-Überwachung und LCD-Display. Der ESP32 wacht stündlich aus dem Deep Sleep auf, misst den Batteriestand und bewässert bei ausreichend Ladung für 5 Minuten.

### ⬆️ UPDATE: Intelligente Bewässerung mit Sensoren

Das Projekt wurde um zwei wichtige Sensoren erweitert:

1. **Bodenfeuchte-Sensor** (Capacitive Soil Moisture v1.2)
   - Misst aktuelle Feuchtigkeit in der Erde
   - Bewässert nur wenn nötig (verhindert Überwässerung)
   - Wird nur bei Messung eingeschaltet (Stromsparen)

2. **Wasserstand-Sensor** (Doppelter Schwimmerschalter, NC)
   - Zwei Pegel: "Tank voll" und "Tank leer"
   - Bewässert nur wenn genug Wasser im Tank ist
   - Schützt Pumpe vor Trockenlauf

**Neue Bewässerungslogik:**
```
1. Akkustand ≥ 20%?
2. Wasserstand nicht leer?
3. Bodenfeuchte < 40% (trocken)?
→ Nur wenn ALLE Bedingungen erfüllt: 15 Sekunden bewässern!
```

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

### ⬆️ UPDATE: Zusätzliche Sensoren

| Komponente | Spezifikation | Anschluss |
|---|---|---|
| **Bodenfeuchte-Sensor** | Capacitive Soil Moisture v1.2 (korrosionsfrei) | GPIO 1 (Signal), GPIO 2 (Power) |
| **Wasserstand-Sensor** | Doppelter Schwimmerschalter, NC, SUS 304 | GPIO 10 (oben), GPIO 11 (unten) |
| **Pull-Up Widerstände** | 10kΩ für beide Schwimmer | 3.3V → GPIO über Widerstand |

**Hinweis zum Bodenfeuchte-Sensor:**
- Kapazitiv = keine Korrosion (kein Strom durch Erde)
- Wird nur beim Messen über GPIO 2 mit Strom versorgt
- Analoger Ausgang 0-3.3V (0-4095 ADC)

**Hinweis zum Schwimmerschalter:**
- NC (Normally Closed) = Schalter ist zu wenn Wasser niedrig
- Pull-Up Widerstand erforderlich (10kΩ)
- Zwei Schwimmer für zwei Pegel: "voll" und "leer"

---

## 🔌 Pin-Belegung

| Funktion | GPIO | Bemerkung |
|---|---|---|
| MOSFET Gate (Pumpe) | **GPIO 13** | Über 100Ω Vorwiderstand |
| Batterie ADC | **GPIO 4** | Spannungsteiler 100k/100kΩ |
| LCD Backlight | **GPIO 38** | High = Backlight an |
| LCD Power On | **GPIO 15** | High = Display an |

### ⬆️ UPDATE: Neue Sensor-Pins

| Funktion | GPIO | Bemerkung |
|---|---|---|
| Bodenfeuchte VCC | **GPIO 2** | Digital Out, nur bei Messung HIGH |
| Bodenfeuchte Signal | **GPIO 1** | Analog In (ADC1_CH0) |
| Schwimmer oben (Tank voll) | **GPIO 10** | INPUT_PULLUP, NC Schalter |
| Schwimmer unten (Tank leer) | **GPIO 11** | INPUT_PULLUP, NC Schalter |

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

### ⬆️ UPDATE: Sensor-Schaltpläne

**Bodenfeuchte-Sensor (Capacitive):**
```
LilyGo T-Display S3       Capacitive Soil Sensor
     GPIO 2  ───────────────► VCC (3.3V)
                              (nur bei Messung HIGH!)
     GPIO 1  ◄─────────────── AOUT (Analog 0-3.3V)
     
     GND     ───────────────► GND
```

**Wasserstand-Sensor (Doppelter Schwimmer, NC):**
```
3.3V ───[10kΩ]───┬──────── GPIO 10 (Oberer Schwimmer)
                 │
          NC-Schalter (schließt bei "Wasser niedrig")
                 │
                GND

3.3V ───[10kΩ]───┬──────── GPIO 11 (Unterer Schwimmer)
                 │
          NC-Schalter (schließt bei "Wasser niedrig")
                 │
                GND
```

**NC-Logik (Normally Closed):**
- Wasser HOCH → Schwimmer schwimmt → Schalter OFFEN → GPIO liest HIGH (Pull-Up)
- Wasser NIEDRIG → Schwimmer hängt → Schalter GESCHLOSSEN → GPIO liest LOW

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

### ⬆️ UPDATE: Neuer intelligenter Ablauf

```
START
  │
  ▼
┌─────────────────────┐
│ Display & Sensoren  │
│ initialisieren      │
└──────────┬──────────┘
           ▼
┌─────────────────────┐
│ ALLE Sensoren lesen │
│ - Akku (GPIO 4)     │
│ - Bodenfeuchte      │
│   (GPIO 1/2)        │
│ - Wasserstand       │
│   (GPIO 10/11)      │
└──────────┬──────────┘
           ▼
        ┌──┴──┐
        │Akku │
        │≥20%?│
        └──┬──┘
      NEIN │ JA
        ▼  │  ▼
   ┌──────┐│ ┌──────┐
   │"Akku ││ │Wasser│
   │leer!"││ │leer? │
   └──────┘│ └──┬───┘
        │  │NEIN│ JA
        │  │  ▼ │  ▼
        │  │┌───┐│┌────┐
        │  ││Bod│││"Tank│
        │  ││en │││leer"│
        │  ││feu││└────┘
        │  ││chte││   │
        │  │└─┬─┘│   │
        │  │  │   │   │
        │  │┌─┴──┐│   │
        │  ││<40%?│   │
        │  │└─┬──┘   │
        │  │JA│ NEIN  │
        │  │▼ │   ▼   │
        │  │┌─┐│┌────┐│
        │  ││ │││"Erde││
        │  ││ │││nass"││
        │  ││ ││└────┘│
        │  ││ ││   │  │
        │  ││ ││   │  │
        └──┴┴─┴┴───┴──┘
                  │
                  ▼
        ┌─────────────────┐
        │ 15 Sek pumpen!  │
        │ Anzeige mit     │
        │ Countdown       │
        └────────┬────────┘
                 │
                 ▼
        ┌─────────────────┐
        │ Ergebnis zeigen │
        │ (abhaengig von  │
        │  Ablehngrund)   │
        └────────┬────────┘
                 │
                 ▼
        ┌─────────────────┐
        │ Deep Sleep 1h   │
        └─────────────────┘
```

**Mögliche Ergebnisse:**
- ✅ "Fertig!" - Erfolgreich bewässert
- 💧 "Erde nass" - Keine Bewässerung nötig
- 🚱 "Tank leer!" - Wasser nachfüllen
- 🔋 "Akku leer!" - Laden

---

## 📄 Aktueller Code-Stand

**Datei:** `main.cpp` (ORIGINAL - vor Sensor-Update)

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

### ⬆️ UPDATE: Neuer Code mit Sensoren

**Datei:** `src/main.cpp` (aktuell auf GitHub)

```cpp
// === Neue Pin Definitionen ===
#define PUMP_PIN        43    // MOSFET Gate (GPIO 43)
#define BATTERY_PIN     4     // Batterie ADC
#define LCD_POWER_PIN   15    // LCD Power On

// --- Bodenfeuchte-Sensor ---
#define SOIL_POWER_PIN  2     // VCC für Sensor (ein/aus)
#define SOIL_ANALOG_PIN 1     // Analoger Eingang

// --- Wasserstand-Sensoren ---
#define FLOAT_UPPER_PIN 10    // Oberer Schwimmer (Tank voll)
#define FLOAT_LOWER_PIN 11    // Unterer Schwimmer (Tank leer)

// === Neue Konfiguration ===
#define PUMP_DURATION       15      // Sekunden (statt 300!)
#define SLEEP_DURATION      3600    // Sekunden (1 Stunde)
#define BAT_MIN             20      // Mindestladung %

// --- Bodenfeuchte Schwellen ---
#define SOIL_DRY_THRESHOLD  2500    // ADC-Wert: ab hier trocken
#define SOIL_WET_THRESHOLD  1500    // ADC-Wert: unterhalb nass
#define SOIL_MEASURE_TIME   500     // ms zum Stabilisieren

// === Datenstruktur für alle Sensoren ===
struct SensorDaten {
    int akkuProzent;
    int bodenfeuchte;        // 0-4095 (raw ADC)
    int bodenfeuchteProzent; // 0-100%
    bool wasserOben;         // true = Tank voll
    bool wasserUnten;        // true = noch Wasser da
    bool tankLeer;
    bool tankVoll;
};

// === Bodenfeuchte-Sensor Funktionen ===
void initBodenfeuchte() {
    pinMode(SOIL_POWER_PIN, OUTPUT);
    digitalWrite(SOIL_POWER_PIN, LOW);  // Aus zum Stromsparen
}

int leseBodenfeuchte() {
    // Sensor einschalten
    digitalWrite(SOIL_POWER_PIN, HIGH);
    delay(SOIL_MEASURE_TIME);
    
    // Messen
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    long summe = 0;
    for (int i = 0; i < 5; i++) {
        summe += analogRead(SOIL_ANALOG_PIN);
        delay(10);
    }
    int wert = summe / 5;
    
    // Sensor ausschalten (Strom sparen!)
    digitalWrite(SOIL_POWER_PIN, LOW);
    
    return wert;
}

int bodenfeuchteZuProzent(int rawWert) {
    // 4095 = trocken (0%), 0 = nass (100%)
    return map(rawWert, 4095, 0, 0, 100);
}

bool sollGiessen(int bodenfeuchteRaw) {
    return bodenfeuchteRaw > SOIL_DRY_THRESHOLD;
}

// === Wasserstand Funktionen ===
void initWasserstand() {
    pinMode(FLOAT_UPPER_PIN, INPUT_PULLUP);
    pinMode(FLOAT_LOWER_PIN, INPUT_PULLUP);
}

bool leseWasserstandOben() {
    // NC Schalter: HIGH = offen = Wasser hoch
    return digitalRead(FLOAT_UPPER_PIN) == HIGH;
}

bool leseWasserstandUnten() {
    // NC Schalter: HIGH = offen = Wasser noch da
    return digitalRead(FLOAT_LOWER_PIN) == HIGH;
}

// === Hauptprogramm ===
void setup() {
    // ... Initialisierung ...
    
    // Alle Sensoren auslesen
    SensorDaten daten;
    daten.akkuProzent = leseAkkustand();
    daten.bodenfeuchte = leseBodenfeuchte();
    daten.bodenfeuchteProzent = bodenfeuchteZuProzent(daten.bodenfeuchte);
    daten.wasserOben = leseWasserstandOben();
    daten.wasserUnten = leseWasserstandUnten();
    
    // Tank-Zustand berechnen
    daten.tankVoll = daten.wasserOben && daten.wasserUnten;
    daten.tankLeer = !daten.wasserOben && !daten.wasserUnten;
    
    // Intelligente Bewässerungsentscheidung
    String grund;
    if (daten.akkuProzent < BAT_MIN) {
        grund = "Akku leer!";
    } else if (daten.tankLeer) {
        grund = "Tank leer!";
    } else if (!sollGiessen(daten.bodenfeuchte)) {
        grund = "Erde nass";
    } else {
        grund = "Bewaessere!";
        // Pumpe starten für 15 Sekunden
        pumpeStarten(daten);
    }
    
    // Ergebnis anzeigen und Deep Sleep
    // ...
}
```

**Wichtige Änderungen:**
- Pumpe jetzt auf **GPIO 43** (statt GPIO 13)
- Pumpzeit reduziert auf **15 Sekunden** (statt 5 Minuten)
- **Sensor-Datenstruktur** für alle Messwerte
- **Nur bei Bedarf bewässern** - spart Wasser und Akku!
- **Erweiterte Display-Anzeige** mit allen Sensorwerten

---

## 📋 Offene Punkte / TODO

- [x] **TFT_eSPI User_Setup_Select.h konfigurieren** – Setup206 aktivieren ✅
- [x] Batterie-ADC-Messung kalibrieren (Realer Akkustand testen) ✅
- [x] LCD-Layout gestalten (Fortschrittsbalken, Countdown) ✅
- [x] Deep Sleep korrekt konfigurieren ✅
- [x] PlatformIO-Projektstruktur erstellen ODER Arduino IDE Projekt ✅
- [x] Freilaufdiode + Spannungsteiler auf Funktion testen ✅

### ⬆️ UPDATE: Neue TODOs für Sensoren

- [ ] **Bodenfeuchte-Sensor anschließen** - GPIO 1 (Signal), GPIO 2 (Power)
- [ ] **Schwimmerschalter anschließen** - GPIO 10 (oben), GPIO 11 (unten), 10kΩ Pull-Ups
- [ ] **Bodenfeuchte kalibrieren** - Werte in trockener/feuchter Erde notieren
- [ ] **Schwimmer testen** - Position im Tank festlegen
- [ ] **Gesamtsystem testen** - Alle drei Sensoren + Bewässerungslogik

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
- DC Pumpe 5V gesteuert mit MOSFET IRLZ44N an GPIO 43
- Freilaufdiode 1N4007 parallel zur Pumpe
- LiPo Akku mit MT3608 Step-Up auf 5V
- Batterie-ADC an GPIO 4 (Spannungsteiler 100k/100kΩ)
- LCD Backlight GPIO 38, LCD Power GPIO 15

### UPDATE: Neue Sensoren
- Bodenfeuchte-Sensor (Capacitive) an GPIO 1 (Signal) und GPIO 2 (Power)
- Doppelter Schwimmerschalter (NC) an GPIO 10 (oben) und GPIO 11 (unten)
- 10kΩ Pull-Up Widerstände für Schwimmer an 3.3V

### Ziel:
ESP32 wacht jede Stunde aus Deep Sleep auf, misst alle Sensoren (Akku, 
Bodenfeuchte, Wasserstand), bewässert nur wenn nötig für 15 Sekunden, 
zeigt alles auf integriertem LCD (TFT_eSPI).

### Aktuelle Aufgabe:
[HIER EIGENE FRAGE EINFÜGEN]

### Letzter Stand:
- Hardware ist verkabelt ✅
- TFT_eSPI Setup206_LilyGo_T_Display_S3.h aktiviert ✅
- Code mit allen drei Sensoren fertig ✅
- Intelligente Bewässerungslogik implementiert ✅
- Dokumentation auf GitHub aktualisiert ✅
- Nur noch Sensoren physisch anschließen und testen 🛠️
```

---

## 🗂️ Dateistruktur (Aktuell)

```
esp32-bewaesserung/
├── platformio.ini              # PlatformIO Konfiguration
├── src/
│   └── main.cpp                # Hauptprogramm mit Sensoren
├── esp32.ino                   # Arduino IDE Version
├── include/
│   └── User_Setup.h            # TFT Display Konfiguration
├── Schaltplan_Solar.md         # Solar-Lade-Schaltung
├── Schaltplan_Sensoren.md      # Sensor-Verkabelung (NEU!)
├── ESP32_Bewaesserung_Projektstart.md  # Diese Datei
├── RemindMe.md                 # Kurzreferenz (NEU!)
└── README.md                   # Haupt-Dokumentation
```

---

## 📊 Display-Layout (Update)

```
┌─────────────────┐
│  Smart Bew.     │  ← Titel
│─────────────────│
│ Akku: [████░░]  │  ← Akkustand mit Balken
│ Erde: [██░░░░]  │  ← Bodenfeuchte 0-100%
│ Wasser: ● ○ OK  │  ← Oberer/Unt. Schwimmer
│─────────────────│
│ Bewaessere!     │  ← Status-Text
│     12s ~~~     │  ← Countdown (nur beim Gießen)
└─────────────────┘
```

**Anzeigen:**
- **Akku:** Farbiger Balken (grün/gelb/rot) + Prozent
- **Erde:** Balken (rot wenn < 30%, grün wenn > 30%)
- **Wasser:** Zwei Kreise (● = voll, ○ = leer) + Text (OK/MITTE/LEER!)

---

> 💡 **Tipp:** Diese Datei in `.vscode/` oder im Projekt-Root ablegen und bei Fragen an den AI Assistant referenzieren!

**GitHub Repository:** https://github.com/dominikbachinger124/pflanzenbewaesserung
