# ESP32 Pflanzenbewässerung - Global Rules

**Projekt:** Intelligentes Bewässerungssystem mit LilyGo T-Display S3  
**Plattform:** ESP32-S3 (Arduino Framework)  
**Letzte Aktualisierung:** Juni 2026

---

## 1. Core Principles

### Non-negotiable Rules

1. **Energieeffizienz vor Komfort** - Alle Sensoren nur bei Bedarf aktivieren (Deep Sleep, Stromsparen)
2. **Hardware-Abstraktion** - Pins über #define konfigurierbar, nicht hardcoded
3. **Mehrfachmessung** - Immer Mittelwerte über 5-10 Messungen für Stabilität
4. **Deutsche Domänensprache** - Kommentare und Namen auf Deutsch (FARBE_AKKU_VOLL, bodenfeuchte)
5. **Sicherheit** - Pulldown-Widerstände für MOSFETs, Limits für alle Werte

### Code-Qualität

- Keine `delay()` in Produktivcode (außer für Hardware-Init)
- Alle GPIOs müssen dokumentiert sein (welcher Pin = welche Funktion)
- Magic Numbers vermeiden - über #define konfigurierbar
- Fehlerbehandlung für alle Hardware-Zugriffe

---

## 2. Tech Stack

### Hardware-Plattform
- **Controller:** ESP32-S3 (LilyGo T-Display S3)
- **Framework:** Arduino (PlatformIO)
- **Display:** ST7789 170x320 TFT (TFT_eSPI Library)

### Entwicklungsumgebung
- **IDE:** VS Code + PlatformIO Extension
- **Board:** esp32-s3-devkitc-1
- **Partition:** huge_app.csv

### Bibliotheken
```ini
lib_deps = bodmer/TFT_eSPI@^2.5.43
```

### Tools
- **Build:** `pio run`
- **Upload:** `pio run --target upload`
- **Monitor:** `pio device monitor` (115200 baud)
- **Test:** Hardware-in-the-Loop (keine Unit-Tests für Embedded)

---

## 3. Architecture

### Folder Structure
```
esp32_pflanzenbewaesserung/
├── src/
│   └── main.cpp              # Hauptanwendung
├── include/
│   └── User_Setup.h          # TFT Display Konfiguration
├── commands/                 # AI Assistant Commands
│   ├── prime.md
│   ├── tools_creation.md
│   ├── sensors_calibration.md
│   ├── planning.md
│   ├── execute.md
│   ├── commit.md
│   └── status.md
├── reference/                # Dokumentation & Guides
│   └── *.md
├── platformio.ini            # Build-Konfiguration
├── README.md                 # Projektdokumentation
├── RemindMe.md               # Kurzreferenz
└── CLAUDE.md                 # Diese Datei
```

### Code Organization

**PIN Definitionen** (oben in main.cpp):
```cpp
// ── Pumpen & Strom ─────────────────────
#define PUMP_PIN        43    // MOSFET Gate (GPIO 43)
#define BATTERY_PIN     44    // Batterie ADC (GPIO 44)
#define LCD_POWER_PIN   15    // LCD Power On (GPIO 15)

// ── Bodenfeuchte-Sensor ─────────────────
#define SOIL_POWER_PIN  2     // VCC für Bodenfeuchte
#define SOIL_ANALOG_PIN 1     // Analoger Eingang (ADC1_CH0)
```

**Datenstrukturen:**
```cpp
struct SensorDaten {
    int akkuProzent;
    int bodenfeuchte;        // 0-4095 (raw ADC)
    int bodenfeuchteProzent; // 0-100%
    bool wasserOben;         // true = >50%
    bool wasserUnten;        // true = >25%
    bool tankLeer;
    bool tankVoll;
};

enum BewaesserungsStatus {
    STATUS_GIESST,
    STATUS_ZU_NASS,
    STATUS_TANK_LEER,
    STATUS_AKKU_LEER,
    STATUS_FERTIG
};
```

**Funktionsstruktur:**
```cpp
// Initialisierung
void initSensorName() { }

// Messung
int leseSensorName() { }

// Konvertierung
int sensorZuProzent(int raw) { }

// Display
void zeichneSensorBalken(int wert) { }
```

---

## 4. Code Style

### Naming Conventions

| Element | Konvention | Beispiel |
|---------|------------|----------|
| #define Konstanten | UPPER_CASE mit Unterstrich | `PUMP_PIN`, `SOIL_DRY_THRESHOLD` |
| Globale Variablen | camelCase | `tft`, `sensorDaten` |
| Funktionen | camelCase, Verb + Objekt | `leseBodenfeuchte()`, `displayAn()` |
| Structs | PascalCase | `SensorDaten`, `BewaesserungsStatus` |
| Lokale Variablen | snake_case | `adc_wert`, `bat_spannung` |
| Enum-Werte | UPPER_CASE mit Unterstrich | `STATUS_GIESST`, `STATUS_TANK_LEER` |

### Kommentar-Style

**Abschnitts-Trenner:**
```cpp
// ═════════════════════════════════════════════════════════════════════════════
// ABSCHNITT NAME
// ═════════════════════════════════════════════════════════════════════════════
```

**Unterabschnitte:**
```cpp
// ── Pumpen & Strom ─────────────────────
```

**Inline-Kommentare:**
```cpp
delay(100);                 // LCD Stabilisierung
int prozent = map(...);     // Umrechnung: 3.0V = 0%, 4.2V = 100%
```

### Code-Beispiel

```cpp
// ═════════════════════════════════════════════════════════════════════════════
// BODENFEUCHTE FUNKTIONEN
// ═════════════════════════════════════════════════════════════════════════════

void initBodenfeuchte() {
    pinMode(SOIL_POWER_PIN, OUTPUT);
    digitalWrite(SOIL_POWER_PIN, LOW);  // Aus zum Stromsparen
}

int leseBodenfeuchte() {
    // Sensor einschalten
    digitalWrite(SOIL_POWER_PIN, HIGH);
    delay(SOIL_MEASURE_TIME);
    
    // Mehrere Messungen für Stabilität
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
    // Umrechnung: 4095 = trocken (0%), 0 = nass (100%)
    int prozent = map(rawWert, 4095, 0, 0, 100);
    return constrain(prozent, 0, 100);
}
```

---

## 5. Logging & Debugging

### Serial Output Format

```cpp
void debugSensorDaten(SensorDaten& daten) {
    Serial.println("=== Sensorwerte ===");
    Serial.print("Akku: "); Serial.print(daten.akkuProzent); Serial.println("%");
    Serial.print("Bodenfeuchte: "); Serial.print(daten.bodenfeuchte); 
    Serial.print(" ("); Serial.print(daten.bodenfeuchteProzent); Serial.println("%)");
    Serial.print("Wasser 50%: "); Serial.println(daten.wasserOben ? "OK" : "LEER");
    Serial.print("Wasser 25%: "); Serial.println(daten.wasserUnten ? "OK" : "LEER");
}
```

### Wichtige Events loggen

- Alle Sensorwerte nach dem Auslesen
- Bewässerungsentscheidung mit Begründung
- Fehler (Sensor nicht erreichbar, unplausible Werte)
- Deep Sleep Eintritt

### Debug-Modus

```cpp
#define DEBUG_MODE  // Auskommentieren für Produktion

#ifdef DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif
```

---

## 6. Testing Strategy

### Hardware-in-the-Loop Tests

**Test 1: Sensoren auslesen**
```cpp
void testSensoren() {
    Serial.begin(115200);
    initBodenfeuchte();
    initWasserstand();
    
    Serial.println("Test Bodenfeuchte:");
    Serial.println(leseBodenfeuchte());
    
    Serial.println("Test Wasserstand:");
    Serial.print("Oben: "); Serial.println(digitalRead(FLOAT_UPPER_PIN));
    Serial.print("Unten: "); Serial.println(digitalRead(FLOAT_LOWER_PIN));
}
```

**Test 2: Pumpe schalten**
```cpp
void testPumpe() {
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, HIGH);
    delay(5000);  // 5 Sekunden laufen
    digitalWrite(PUMP_PIN, LOW);
}
```

**Test 3: Display**
```cpp
void testDisplay() {
    displayAn();
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Test OK!");
    delay(5000);
    displayAus();
}
```

### Manuelle Tests

1. **Kalibrierung:** Sensor in trockener/feuchter Erde messen
2. **Schwimmer:** Tank leer laufen lassen, Position markieren
3. **Akku:** Spannung mit Multimeter vergleichen
4. **Langzeit:** 24h Laufzeit überwachen

---

## 7. Hardware Contracts

### Pin-Belegung (fest definiert)

| GPIO | Funktion | Typ | Details |
|------|----------|-----|---------|
| 1 | Bodenfeuchte Signal | Analog IN | ADC1_CH0, 0-3.3V |
| 2 | Bodenfeuchte Power | Digital OUT | HIGH = Sensor an |
| 10 | Wasserstand 50% | Digital IN | Pull-Up, NO Schalter |
| 11 | Wasserstand 25% | Digital IN | Pull-Up, NO Schalter |
| 15 | LCD Power | Digital OUT | HIGH = Display an |
| 38 | LCD Backlight | Digital OUT | HIGH = Backlight an |
| 43 | Pumpe | Digital OUT | MOSFET Gate |
| 44 | Batterie ADC | Analog IN | Spannungsteiler |

### Sensor-Wertebereiche

**Bodenfeuchte (ADC):**
- 3500-4095 = Trocken (0-10%)
- 1500-2500 = Optimal (30-70%)
- 0-1500 = Nass (70-100%)

**Wasserstand (Digital):**
- HIGH = Schwimmer schwimmt = Wasser vorhanden
- LOW = Schwimmer hängt = Wasser leer

**Akku (Spannung):**
- 3.0V = 0% (kritisch, nicht weiter entladen)
- 3.7V = 50%
- 4.2V = 100% (voll)

---

## 8. Common Patterns

### Pattern 1: Sensor mit Stromsparen

```cpp
class Sensor {
    int powerPin;
    int readPin;
    
public:
    Sensor(int power, int read) : powerPin(power), readPin(read) {}
    
    void begin() {
        pinMode(powerPin, OUTPUT);
        digitalWrite(powerPin, LOW);
    }
    
    int read() {
        digitalWrite(powerPin, HIGH);
        delay(500);  // Stabilisierung
        int value = analogRead(readPin);
        digitalWrite(powerPin, LOW);  // Strom sparen!
        return value;
    }
};
```

### Pattern 2: Zustandsmaschine mit Enum

```cpp
enum SystemZustand {
    ZUSTAND_MESSEN,
    ZUSTAND_BEWAESSERN,
    ZUSTAND_SCHLAFEN
};

void loop() {
    switch (aktuellerZustand) {
        case ZUSTAND_MESSEN:
            // Messen
            aktuellerZustand = ZUSTAND_BEWAESSERN;
            break;
        case ZUSTAND_BEWAESSERN:
            // Bewässern
            aktuellerZustand = ZUSTAND_SCHLAFEN;
            break;
        // ...
    }
}
```

### Pattern 3: Datenstruktur mit mehreren Sensoren

```cpp
struct AlleSensoren {
    int bodenfeuchte;
    int akkuProzent;
    bool wasserstandOK;
    unsigned long zeitstempel;
};

AlleSensoren leseAlleSensoren() {
    AlleSensoren daten;
    daten.bodenfeuchte = leseBodenfeuchte();
    daten.akkuProzent = leseAkku();
    daten.wasserstandOK = pruefeWasserstand();
    daten.zeitstempel = millis();
    return daten;
}
```

---

## 9. Development Commands

### PlatformIO (VS Code)

```bash
# Build
pio run

# Upload
pio run --target upload

# Serial Monitor
pio device monitor

# Clean
pio run --target clean

# Check
pio check
```

### Git Workflow

```bash
# Status checken
git status

# Alle Änderungen committen
git add -A
git commit -m "feat: beschreibung der änderung"

# Push
git push
```

### Board-Einstellungen (platformio.ini)

```ini
[env:lilygo-t-display-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
lib_deps = bodmer/TFT_eSPI@^2.5.43
upload_speed = 921600
monitor_speed = 115200
```

---

## 10. AI Coding Assistant Instructions

### Bevor du startest:

1. **Lese CLAUDE.md** - Diese Datei hat Priorität
2. **Prüfe Hardware-Pins** - GPIOs sind fest definiert, nicht willkürlich ändern
3. **Energieeffizienz beachten** - Alle Sensoren nur bei Bedarf aktivieren
4. **Deutsche Namen verwenden** - Domänensprache ist Deutsch (bodenfeuchte, nicht soil_moisture)

### Während der Entwicklung:

5. **Pin-Definitionen oben** - Neue #defines in den richtigen Abschnitten
6. **Mehrfachmessung** - Mittelwerte über 5-10 Messungen für Stabilität
7. **Keine delay() Schleifen** - Non-blocking Code bevorzugen
8. **Magic Numbers vermeiden** - Alles über #define konfigurierbar machen

### Code-Qualität:

9. **Kommentare auf Deutsch** - Mit Linien-Trennern (// ═══)
10. **Logging einbauen** - Serial.print für Debug-Informationen

### Vor dem Commit:

- [ ] Kompiliert ohne Fehler? (`pio run`)
- [ ] Pins dokumentiert?
- [ ] Test-Code entfernt (oder mit #ifdef DEBUG)?
- [ ] Konventionen eingehalten (Naming, Kommentare)?

---

## Quick Reference

**Wichtigste Schwellenwerte:**
```cpp
#define SOIL_DRY_THRESHOLD  2500    // ADC-Wert
#define SOIL_WET_THRESHOLD  1500
#define BAT_MIN             20      // Prozent
#define PUMP_DURATION       15      // Sekunden
#define SLEEP_DURATION      3600    // Sekunden
```

**Wichtigste GPIOs:**
```cpp
GPIO 1  = Bodenfeuchte Signal
GPIO 2  = Bodenfeuchte Power
GPIO 10 = Wasserstand 50%
GPIO 11 = Wasserstand 25%
GPIO 43 = Pumpe
GPIO 44 = Akku ADC
```

**Bibliothek:**
```cpp
#include <TFT_eSPI.h>   // Display
#include <Arduino.h>    // Core
#include "esp_sleep.h"  // Deep Sleep
```
