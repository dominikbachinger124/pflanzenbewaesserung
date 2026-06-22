# RemindMe - ESP32 Bewässerung Projekt

**Zusammenfassung aller wichtigen Projekt-Ergebnisse für zukünftige Erweiterungen**

---

## 1. Hardware-Plattform

### LilyGo T-Display S3
- **Board:** ESP32-S3 mit 170x320 TFT Display (ST7789)
- **Framework:** Arduino / PlatformIO
- **Akkumessung:** Eingebauter Spannungsteiler (GPIO 44, LCD_BAT_VOLT)
- **Besonderheit:** Display muss über GPIO 15 eingeschaltet werden

### Bauteile-Liste
| Komponente | Anschluss | Hinweis |
|------------|-----------|---------|
| Pumpe | GPIO 43 | Über MOSFET |
| Bodenfeuchte | GPIO 1 (Signal), GPIO 2 (Power) | Kapazitiv, nur bei Messung einschalten |
| Schwimmer oben | GPIO 10 | NC (Normally Closed), 10kΩ Pull-Up |
| Schwimmer unten | GPIO 11 | NC (Normally Closed), 10kΩ Pull-Up |
| Batteriemessung | GPIO 44 | LCD_BAT_VOLT |
| Display Power | GPIO 15 | Muss HIGH sein |
| Display Backlight | GPIO 38 | Muss HIGH sein |

---

## 2. Pinout-Referenz

### Belegte Pins (nicht verfügbar)
```
GPIO 4   → Frei (Touch0 / ADC1_CH0)
GPIO 44  → Batteriemessung (LCD_BAT_VOLT)
GPIO 5-9 → TFT Display (RST, CS, DC, WR, RD)
GPIO 15  → LCD Power
GPIO 38  → Backlight
GPIO 39-42 → TFT Daten D0-D3
GPIO 43  → Pumpe
GPIO 45-48 → TFT Daten D4-D7
```

### Freie Pins (für Erweiterungen)
```
GPIO 1   → ADC1_CH0 (genutzt für Bodenfeuchte)
GPIO 2   → Digital Out (genutzt für Sensor-Power)
GPIO 10  → Touch10 / ADC1_CH9 (genutzt für Schwimmer)
GPIO 11  → Touch11 / ADC2_CH0 (genutzt für Schwimmer)
GPIO 12  → Touch12 / ADC2_CH1
GPIO 13  → Touch13 / ADC2_CH2
GPIO 14  → Boot-Pin! (vermeiden)
GPIO 16  → ADC2_CH5
GPIO 17  → UART TX
GPIO 18  → UART RX / ADC2_CH7
GPIO 21  → Frei
```

**Empfohlene neue Sensoren:**
- GPIO 12, 13, 16, 17, 18, 21

---

## 3. Schaltungs-Muster

### Bodenfeuchte-Sensor anschließen
```
LilyGo T-Display S3       Capacitive Soil Sensor
     GPIO 2  ───────────────► VCC (3.3V)
     GPIO 1  ◄─────────────── AOUT (Analog)
     GND     ───────────────► GND
```
**Wichtig:** GPIO 2 nur während Messung auf HIGH!

### Schwimmerschalter (NC) anschließen
```
3.3V ───[10kΩ]───┬──────── GPIO 10 oder 11
                 │
          NC-Schalter (schließt bei Wasser niedrig)
                 │
                GND
```

**NC-Logik:**
- Wasser HOCH → Schalter OFFEN → GPIO liest HIGH
- Wasser NIEDRIG → Schalter GESCHLOSSEN → GPIO liest LOW

---

## 4. Code-Struktur

### Hauptkomponenten
```cpp
// 1. Pin-Definitionen
#define PUMP_PIN        43
#define SOIL_POWER_PIN  2
#define SOIL_ANALOG_PIN 1
#define FLOAT_UPPER_PIN 10
#define FLOAT_LOWER_PIN 11

// 2. Sensoren initialisieren
void initBodenfeuchte() {
    pinMode(SOIL_POWER_PIN, OUTPUT);
    digitalWrite(SOIL_POWER_PIN, LOW);
}

void initWasserstand() {
    pinMode(FLOAT_UPPER_PIN, INPUT_PULLUP);
    pinMode(FLOAT_LOWER_PIN, INPUT_PULLUP);
}

// 3. Messen
int leseBodenfeuchte() {
    digitalWrite(SOIL_POWER_PIN, HIGH);
    delay(500);
    int wert = analogRead(SOIL_ANALOG_PIN);
    digitalWrite(SOIL_POWER_PIN, LOW);  // Strom sparen!
    return wert;
}

bool leseWasserstandOben() {
    return digitalRead(FLOAT_UPPER_PIN) == HIGH;
}
```

### Bewässerungs-Logik
```
Prüfung in Reihenfolge:
1. Akku >= 20% ?
2. Wasserstand nicht leer ?
3. Bodenfeuchte < 40% (trocken) ?

→ Nur wenn ALLE wahr: Pumpe einschalten für 15 Sekunden
```

---

## 5. Display-Anzeige

### Layout (170x320 Pixel)
```
Zeile 0-25:   Titel "Smart Bew."
Zeile 25-90:  Akku-Balken (Prozent + Farbe)
Zeile 90-160: Bodenfeuchte-Balken (Prozent)
Zeile 160-190: Wasserstand (2 Kreise + Text)
Zeile 190-220: Status-Text (groß)
Zeile 220-280: Countdown (nur beim Gießen)
```

### Farben
```cpp
#define FARBE_AKKU_VOLL     TFT_GREEN   // > 50%
#define FARBE_AKKU_HALB     TFT_YELLOW  // 20-50%
#define FARBE_AKKU_LEER     TFT_RED     // < 20%
#define FARBE_ERDE_TROCKEN  TFT_RED     // < 30%
#define FARBE_ERDE_FEUCHT   TFT_GREEN   // > 30%
#define FARBE_WASSER        TFT_BLUE
#define FARBE_WARNUNG       TFT_ORANGE
```

---

## 6. Kalibrierung

### Bodenfeuchte-Sensor (ADC-Werte)
```
4095 (3.3V) = Trockene Erde      → 0%
2500        = Schwellwert "trocken" → 40%
1500        = Schwellwert "nass"    → 70%
0           = In Wasser getaucht    → 100%
```

**Formel:**
```cpp
prozent = map(adcWert, 4095, 0, 0, 100);
```

### Batterie (Spannungsteiler 100k/100k)
```
ADC-Wert → Spannung = (adc / 4095.0) * 3.3 * 2.0
3.0V = 0% (Leer)
4.2V = 100% (Voll)
```

---

## 7. Stromverbrauch Optimieren

### Deep Sleep
```cpp
esp_sleep_enable_timer_wakeup(3600 * 1000000ULL); // 1 Stunde
esp_deep_sleep_start();
```

### Sensoren nur bei Bedarf einschalten
```cpp
// GUT (Stromsparend):
digitalWrite(SENSOR_POWER, HIGH);
delay(500);  // Stabilisieren
int wert = analogRead(SENSOR_PIN);
digitalWrite(SENSOR_POWER, LOW);

// SCHLECHT (Stromfressend):
int wert = analogRead(SENSOR_PIN);  // Sensor immer an
```

### Stromverbrauch
| Zustand | Strom | Dauer |
|---------|-------|-------|
| Messen | ~80mA | ~5s |
| Pumpen | ~200mA | 15s |
| Deep Sleep | ~0.01mA | ~1h |

**Ergebnis:** ~2-4 Wochen Laufzeit mit 2000mAh LiPo

---

## 8. Dateistruktur

```
pflanzenbewaesserung/
├── include/
│   └── User_Setup.h              # TFT Konfiguration
├── src/
│   └── main.cpp                  # Hauptprogramm (PlatformIO)
├── esp32.ino                     # Arduino IDE Version
├── platformio.ini                # PlatformIO Config
├── Schaltplan_Solar.md           # Solar-Lade-Schaltung
├── Schaltplan_Sensoren.md        # Sensor-Verkabelung
├── ESP32_Bewaesserung_Projektstart.md  # Projekt-Notizen
└── README.md                     # Haupt-Dokumentation
```

---

## 9. Häufige Fehler

### Problem: Display bleibt schwarz
**Lösung:** GPIO 15 (LCD_POWER) muss auf HIGH gesetzt werden!

### Problem: Falsche ADC-Werte
**Ursache:** ADC11db Attenuation nicht gesetzt
```cpp
analogReadResolution(12);
analogSetAttenuation(ADC_11db);
```

### Problem: Wasserstand erkannt nicht
**Ursache:** Pull-Up Widerstand fehlt
**Lösung:** 10kΩ zwischen 3.3V und GPIO

### Problem: Bodenfeuchte ändert sich nicht
**Ursache:** Sensor nicht eingeschaltet
**Lösung:** GPIO 2 auf HIGH vor Messung

---

## 10. Erweiterungs-Möglichkeiten

### Mögliche neue Sensoren (freie Pins)
```
GPIO 12, 13 → Weitere Bodenfeuchte-Sensoren (andere Pflanzen)
GPIO 16, 17, 18 → DHT22 (Temperatur/Luftfeuchte)
GPIO 21 → Lichtsensor (BH1750, I2C)
```

### Neue Features
- **Mehrere Zonen:** 2-4 Pumpen für verschiedene Pflanzen
- **WiFi:** ESP-NOW oder WiFi für Fernüberwachung
- **Datenlogging:** Feuchtigkeitsverlauf auf SD-Karte
- **Wetter-API:** Bewässerung bei Regen überspringen

### Code-Vorlage für neuen Sensor
```cpp
// In main.cpp hinzufügen:

// 1. Pin definieren
#define NEUER_SENSOR_PIN 12

// 2. Initialisieren
void initNeuerSensor() {
    pinMode(NEUER_SENSOR_PIN, INPUT);
}

// 3. Auslesen
int leseNeuerSensor() {
    return analogRead(NEUER_SENSOR_PIN);
}

// 4. In Setup aufrufen
setup() {
    // ... bestehender Code ...
    initNeuerSensor();
    // ...
}

// 5. In Logik einbauen
void loop() {
    int wert = leseNeuerSensor();
    if (wert > SCHWELLE) {
        // Aktion
    }
}
```

---

## 11. Wichtige Links & Referenzen

### GitHub Repository
```
https://github.com/dominikbachinger124/pflanzenbewaesserung
```

### Datenblätter
- LilyGo T-Display S3: https://www.lilygo.cc/products/t-display-s3
- Capacitive Soil Sensor: V1.2 (korrosionsfrei)
- Schwimmerschalter: SUS 304, NC, 0-220V

### ADC Kanäle ESP32-S3
```
ADC1_CH0 = GPIO 1  (genutzt: Bodenfeuchte)
ADC1_CH1 = GPIO 2
ADC1_CH2 = GPIO 3
ADC1_CH9 = GPIO 10
ADC2_CH0 = GPIO 11
ADC2_CH1 = GPIO 12
ADC2_CH2 = GPIO 13
ADC2_CH5 = GPIO 16
ADC2_CH7 = GPIO 18
```

---

## 12. Schnell-Referenz

### Terminal-Befehle
```bash
# Kompilieren & Flashen
pio run --target upload

# Serial Monitor
pio device monitor

# Git push
git add -A && git commit -m "Update" && git push
```

### Wichtigste Konstanten
```cpp
PUMP_DURATION      = 15     // Sekunden
SLEEP_DURATION     = 3600   // Sekunden (1 Stunde)
BAT_MIN            = 20     // Prozent
SOIL_DRY_THRESHOLD = 2500   // ADC-Wert
SOIL_WET_THRESHOLD = 1500   // ADC-Wert
```

---

**Erstellt:** Juni 2026  
**Version:** 2.0 (mit Sensoren)  
**Autor:** dominikbachinger124  

**Letzter Stand:** Code + Schaltpläne + Dokumentation komplett auf GitHub
