# ESP32 Pflanzenbewässerung

Automatisches Bewässerungssystem basierend auf dem **LilyGo T-Display S3** mit integriertem TFT-Display und Deep-Sleep für energiesparenden Betrieb.

![Projekt Screenshot](Screenshot%20from%202026-06-21%2012-56-35.png)

## Features

- **Automatische Bewässerung** alle Stunde für 15 Sekunden
- **TFT-Display** mit Akkustatus-Anzeige und Animation
- **Batterieüberwachung** mit visuellem Balken und Prozentanzeige
- **Tiefschlaf-Modus** für minimalen Stromverbrauch
- **Überlastschutz** - Bewässerung nur bei ausreichend Akku (>20%)

## Hardware

| Komponente | Pin | Beschreibung |
|------------|-----|--------------|
| Pumpe | GPIO 43 | MOSFET Gate zur Pumpensteuerung |
| Batterie | GPIO 4 | ADC zur Spannungsmessung |
| LCD Power | GPIO 15 | Display-Stromversorgung |
| LCD Backlight | GPIO 38 | Display-Hintergrundbeleuchtung |

### Verwendete Hardware

- [LilyGo T-Display S3](https://www.lilygo.cc/products/t-display-s3) (ESP32-S3 mit 170x320 TFT)
- 3.7V LiPo Akku
- Mini-Wasserpumpe (5V/12V mit MOSFET)
- Spannungsteiler 100k/100k für Batteriemessung

## Software

### PlattformIO Konfiguration

Das Projekt ist für **PlatformIO** konfiguriert:

```ini
[env:lilygo-t-display-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
lib_deps = bodmer/TFT_eSPI@^2.5.43
```

### Wichtige Einstellungen

| Parameter | Wert | Beschreibung |
|-----------|------|--------------|
| `PUMP_DURATION` | 15 Sekunden | Dauer der Bewässerung |
| `SLEEP_DURATION` | 3600 Sekunden | Pause zwischen Bewässerungen (1 Stunde) |
| `BAT_MIN` | 20% | Mindestladung für Bewässerung |

## Installation

1. **Repository klonen:**
   ```bash
   git clone https://github.com/dominikbachinger124/pflanzenbewaesserung.git
   cd pflanzenbewaesserung
   ```

2. **In VS Code + PlatformIO öffnen**

3. **Abhängigkeiten installieren:**
   ```bash
   pio lib install
   ```

4. **Kompilieren & Flashen:**
   ```bash
   pio run --target upload
   ```

## Schaltplan

Siehe [Schaltplan_Solar.md](Schaltplan_Solar.md) für detaillierte Verdrahtungsinformationen.

## Anzeige

Das Display zeigt:
- **Titel**: "Bewaesserung"
- **Akkustatus**: Prozentwert mit farbigem Balken (Grün/Gelb/Rot)
- **Aktion**: Aktueller Status ("Bewaessere!", "Fertig!", "Akku leer!")
- **Countdown**: Verbleibende Sekunden während der Bewässerung
- **Animation**: Wassertropfen-Animation

## Akku-Berechnung

Die Batteriespannung wird über einen Spannungsteiler (100k/100k) gemessen:

```
Batteriespannung = (ADC-Wert / 4095) * 3.3V * 2

Prozent = map(Spannung, 3.0V, 4.2V, 0%, 100%)
```

- **3.0V** = 0% (Leer)
- **4.2V** = 100% (Voll)

## Stromverbrauch

- **Aktiv**: ~80mA (Display + Pumpe)
- **Deep Sleep**: ~0.01mA
- **Geschätzte Laufzeit**: Mehrere Wochen mit 2000mAh LiPo

## Projektstart

Siehe [ESP32_Bewaesserung_Projektstart.md](ESP32_Bewaesserung_Projektstart.md) für den ursprünglichen Projektplan und Entwicklungsnotizen.

## Lizenz

Dieses Projekt ist Open Source - frei verwendbar und modifizierbar.

---

**Autor:** dominikbachinger124  
**Hardware:** LilyGo T-Display S3 | ESP32-S3  
**Framework:** Arduino / PlatformIO
