# ESP32 Pflanzenbewässerung

Intelligentes Bewässerungssystem basierend auf dem **LilyGo T-Display S3** mit automatischer Feuchtigkeitsmessung, Wasserstandsüberwachung und energiesparendem Deep-Sleep.

![Projekt Screenshot](Screenshot%20from%202026-06-21%2012-56-35.png)

## Features

- **Intelligente Bewässerung** - Nur gießen wenn nötig (Bodenfeuchte + Wasserstand)
- **Bodenfeuchte-Sensor** - Verhindert Überwässerung
- **Wasserstands-Überwachung** - Doppelter Schwimmerschalter (Tank leer = kein Gießen)
- **TFT-Display** - Zeigt alle Sensorwerte in Echtzeit
- **Deep-Sleep Modus** - 1 Stunde Schlaf, dann erneute Prüfung
- **Stromsparen** - Sensoren werden nur bei Bedarf eingeschaltet

## Hardware

### Verwendete Komponenten

| Komponente | Modell | Beschreibung |
|------------|--------|--------------|
| Mikrocontroller | LilyGo T-Display S3 | ESP32-S3 mit 170x320 TFT |
| Bodenfeuchte | Capacitive Soil Moisture v1.2 | Korrosionsfrei, analog |
| Wasserstand | Einfacher Schwimmerschalter (NO) | Ein Pegel: "leer" Warnung |
| Pumpe | 5V/12V Mini-Pumpe | Mit MOSFET gesteuert |
| Akku | 3.7V LiPo | 2000mAh empfohlen |

### Pin-Belegung

| Funktion | GPIO | Beschreibung |
|----------|------|--------------|
| **Pumpe** | 43 | MOSFET Gate |
| **Batterie** | 44 | ADC für Akkustand |
| **LCD Power** | 15 | Display-Stromversorgung |
| **LCD Backlight** | 38 | Display-Beleuchtung |
| **Bodenfeuchte VCC** | 2 | Sensor-Strom (ein/aus) |
| **Bodenfeuchte Signal** | 1 | Analoger Eingang (ADC1_CH0) |
| **Wasserstand** | 11 | Einfacher Schwimmer (NO, Pull-Up) |
| *(Reserviert)* | 10 | Für spätere Erweiterungen |

### Schaltplan

#### Bodenfeuchte-Sensor (Capacitive)
```
LilyGo T-Display S3          Capacitive Soil Sensor
     GPIO 2  ──────────────────► VCC (3.3V)
     GPIO 1  ◄────────────────── AOUT (Analog)
     GND     ──────────────────► GND
```

#### Wasserstand (Einfacher Schwimmerschalter NO)
```
LilyGo T-Display S3          Schwimmerschalter (NO)
     GPIO 11 ──┬──[10kΩ]── 3.3V
               │
               └── Schwimmer (schließt bei "leer")
               
     GND ──────┴────────────── Kontakt
```

**NO = Normally Open:** 
- **Wasser vorhanden** → Schwimmer schwimmt → Schalter **OFFEN** → GPIO liest **HIGH** (Pull-Up)
- **Tank leer** → Schwimmer hängt → Schalter **GESCHLOSSEN** → GPIO liest **LOW** → Bewässerung STOPP!

## Software

### Bewässerungslogik

Das System prüft in dieser Reihenfolge:

1. **Akkustand** ≥ 20%?
2. **Wasserstand** nicht leer?
3. **Bodenfeuchte** unter 40% (trocken)?

→ Nur wenn ALLE Bedingungen erfüllt sind, wird bewässert!

### Schwellenwerte

| Parameter | Wert | Beschreibung |
|-----------|------|--------------|
| `SOIL_DRY_THRESHOLD` | 2500 (ADC) | Ab hier: Erde zu trocken |
| `SOIL_WET_THRESHOLD` | 1500 (ADC) | Ab hier: Erde feucht genug |
| `PUMP_DURATION` | 15 Sekunden | Dauer der Bewässerung |
| `SLEEP_DURATION` | 3600 Sekunden | Pause bis zur nächsten Prüfung |
| `BAT_MIN` | 20% | Mindestakkustand |

### Display-Anzeige

```
┌─────────────────┐
│  Smart Bew.     │  ← Titel
│─────────────────│
│ Akku: [████░░]  │  ← Akkustand mit Balken
│ Erde: [██░░░░]  │  ← Bodenfeuchte 0-100%
│ Wasser: ○ OK    │  ← Schwimmer (○ = leer, ● = OK)
│─────────────────│
│ Bewaessere!     │  ← Status
│     12s ~~~     │  ← Countdown (bei Gießen)
└─────────────────┘
```

**Wasserstand-Indikator (einfacher NO Schalter):**
- `●` = OK (Schwimmer schwimmt, Schalter offen)
- `○` = LEER! (Schwimmer hängt, Schalter geschlossen)
- Text: `OK` oder `LEER!`

## Installation

### 1. Repository klonen
```bash
git clone https://github.com/dominikbachinger124/pflanzenbewaesserung.git
cd pflanzenbewaesserung
```

### 2. In VS Code + PlatformIO öffnen

### 3. Abhängigkeiten installieren
```bash
pio lib install
```

### 4. Kompilieren & Flashen
```bash
pio run --target upload
```

### 5. Serial Monitor (optional)
```bash
pio device monitor
```

## Konfiguration

Die wichtigsten Einstellungen sind in `src/main.cpp`:

```cpp
// Pumpendauer
#define PUMP_DURATION       15      // Sekunden

// Prüfintervall
#define SLEEP_DURATION      3600    // Sekunden (1 Stunde)

// Bodenfeuchte Schwellen (0-4095)
#define SOIL_DRY_THRESHOLD  2500    // Gießen ab diesem Wert
#define SOIL_WET_THRESHOLD  1500    // Stoppen unter diesem Wert
```

## Kalibrierung

### Bodenfeuchte-Sensor
Der Sensor gibt Werte von 0-4095 (12-bit ADC):
- **~3500-4095** = Trockene Erde
- **~1500-2500** = Optimale Feuchtigkeit
- **~0-1500** = Sehr nass

**Schritte zur Kalibrierung:**
1. Sensor in trockene Erde stecken → Wert notieren
2. Sensor in optimale feuchte Erde stecken → Wert notieren
3. Werte in `SOIL_DRY_THRESHOLD` eintragen

## Stromverbrauch

| Zustand | Strom | Dauer |
|---------|-------|-------|
| Messen & Anzeigen | ~80mA | ~5 Sekunden |
| Pumpen | ~200mA | 15 Sekunden |
| Deep Sleep | ~0.01mA | ~1 Stunde |

**Geschätzte Laufzeit:** 2-4 Wochen mit 2000mAh LiPo

## Fehlersuche

### Display bleibt schwarz
- Prüfe ob `LCD_POWER_PIN` (GPIO 15) korrekt verbunden ist
- Akku ist leer → laden

### Bewässert nicht
| Anzeige | Ursache | Lösung |
|---------|---------|--------|
| "Akku leer!" | Ladung < 20% | Akku laden |
| "Tank leer!" | Wasserstand kritisch | Wasser nachfüllen |
| "Erde nass" | Bodenfeuchte hoch | Warten bis trockener |

### Falsche Sensorwerte
- Bodenfeuchte: Kalibrieren (siehe oben)
- Wasserstand: Prüfen ob Schwimmer nicht klemmt

## Dateistruktur

```
├── include/
│   └── User_Setup.h          # TFT Display Konfiguration
├── src/
│   └── main.cpp              # Hauptprogramm
├── esp32.ino                 # Arduino IDE Version
├── platformio.ini            # PlatformIO Konfiguration
├── Schaltplan_Solar.md       # Detaillierter Schaltplan
├── ESP32_Bewaesserung_Projektstart.md  # Entwicklungsnotizen
└── README.md                 # Diese Datei
```

## Weiterentwicklung

Mögliche Erweiterungen:
- **WiFi/Bluetooth** - Status per App abrufen
- **Datenlogging** - Feuchtigkeitsverlauf speichern
- **Mehrere Pflanzen** - Erweiterung auf 2-4 Zonen
- **Sonnenlicht-Sensor** - Bewässerung nur bei Bedarf tagsüber

## Lizenz

Dieses Projekt ist Open Source - frei verwendbar und modifizierbar.

---

**Autor:** dominikbachinger124  
**Hardware:** LilyGo T-Display S3 | ESP32-S3  
**Framework:** Arduino / PlatformIO  
**Version:** 2.0 (mit Sensoren)
