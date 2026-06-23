# Project Status

Zeigt den aktuellen Stand des ESP32-Pflanzenbewässerungsprojekts an.

## Zweck

Schneller Überblick über:
- Code-Status (was ist implementiert)
- Hardware-Status (was ist angeschlossen)
- Offene TODOs
- Nächste Schritte

## Anweisungen

Lese die folgenden Dateien, um den Status zu ermitteln:
1. `/home/komodor/esp32_pumpe/src/main.cpp` - Hauptcode
2. `/home/komodor/esp32_pumpe/platformio.ini` - Konfiguration
3. `/home/komodor/esp32_pumpe/README.md` - Dokumentation
4. `/home/komodor/esp32_pumpe/Schaltplan_Sensoren.md` - Hardware

## Status-Report Format

Erstelle eine übersichtliche Zusammenfassung:

```markdown
# 🌱 ESP32 Bewässerung - Projekt Status

## 📊 Code-Status

### ✅ Implementiert
- [Feature] - [Kurze Beschreibung]
- [Feature] - [Kurze Beschreibung]

### 🚧 Konfiguriert aber nicht getestet
- [Feature] - [Was fehlt]

### ❌ Noch nicht implementiert
- [Feature] - [Warum]

## 🔌 Hardware-Status

### Verkabelt & Getestet
| Komponente | Pin | Status |
|------------|-----|--------|
| Pumpe | GPIO 43 | ✅ Verkabelt |
| ... | ... | ... |

### Konfiguriert aber nicht physisch angeschlossen
| Komponente | Pin | Status |
|------------|-----|--------|
| Bodenfeuchte | GPIO 1/2 | ⚠️ Code bereit |
| ... | ... | ... |

## 📋 Offene TODOs (Top 5)

1. **[Priorität]** [TODO Beschreibung]
2. ...

## 🎯 Nächste Schritte

Empfohlene Reihenfolge:
1. [Schritt 1]
2. [Schritt 2]
3. [Schritt 3]

## 💡 Quick-Info

- **Letzter Commit:** [Datum/Message]
- **Build-Status:** [Kompilierbar?]
- **Aktiver Branch:** [Branch-Name]
```

## Beispiel-Ausgabe

```markdown
# 🌱 ESP32 Bewässerung - Projekt Status

## 📊 Code-Status

### ✅ Implementiert
- Grundlegende Bewässerungslogik (15 Sekunden, stündlich)
- Akku-Überwachung (GPIO 44, Spannungsteiler)
- Display-Anzeige mit Animation
- Deep Sleep mit Timer-Wakeup

### 🚧 Konfiguriert aber nicht getestet
- Bodenfeuchte-Sensor (Code vorhanden, Sensor nicht angeschlossen)
- Wasserstand-Sensor (Code vorhanden, Schwimmer nicht verkabelt)

### ❌ Noch nicht implementiert
- Datenlogging (SD-Karte nicht konfiguriert)
- WiFi/Bluetooth (nicht geplant für v1)

## 🔌 Hardware-Status

### Verkabelt & Getestet
| Komponente | Pin | Status |
|------------|-----|--------|
| Pumpe | GPIO 43 | ✅ MOSFET + Pulldown |
| Akku-Messung | GPIO 44 | ✅ Spannungsteiler |
| Display | GPIO 15/38 | ✅ LCD + Backlight |

### Noch nicht physisch angeschlossen
| Komponente | Pin | Status |
|------------|-----|--------|
| Bodenfeuchte VCC | GPIO 2 | ⚠️ Code bereit |
| Bodenfeuchte Signal | GPIO 1 | ⚠️ Code bereit |
| Wasserstand 50% | GPIO 10 | ⚠️ Pull-Up erforderlich |
| Wasserstand 25% | GPIO 11 | ⚠️ Pull-Up erforderlich |

## 📋 Top 5 TODOs

1. **🔴 HIGH** Bodenfeuchte-Sensor physikalisch anschließen (3 Kabel)
2. **🔴 HIGH** Schwimmer für Wasserstand verkabeln (2x 10kΩ Pull-Up)
3. **🟡 MEDIUM** Sensoren testen und kalibrieren
4. **🟡 MEDIUM** Gesamtsystem-Test (alle 3 Sensoren)
5. **🟢 LOW** README aktualisieren wenn Hardware läuft

## 🎯 Empfohlene nächste Schritte

1. **Bodenfeuchte-Sensor anschließen:**
   - VCC (rot) → GPIO 2
   - Signal (gelb) → GPIO 1
   - GND (schwarz) → GND

2. **Wasserstand-Schwimmer verkabeln:**
   - GPIO 10 → 10kΩ → 3.3V (und Schwimmer)
   - GPIO 11 → 10kΩ → 3.3V (und Schwimmer)
   - Gemeinsamer GND

3. **Test-Modus:** In `main.cpp` DEBUG definieren für Serial-Output

## 💡 Quick-Info

- **Letzter Commit:** "Add prime-tools command"
- **Build:** ✅ Kompilierbar (PlatformIO)
- **Branch:** master
- **Repo:** https://github.com/dominikbachinger124/pflanzenbewaesserung
```

## Regeln

1. **Sei ehrlich** - Zeige auch unvollständige/falsche Dinge
2. **Priorisiere** - Top 5 TODOs, nicht alles auflisten
3. **Visuell** - Nutze Emojis und Tabellen für schnelles Scannen
4. **Aktion-orientiert** - Zeige konkrete nächste Schritte
5. **Hardware-Fokus** - Für ESP32-Projekte ist Verkabelung oft der Engpass

## Anti-Patterns

❌ Nicht tun:
- Alles auflisten (Information Overload)
- Nur "alles super" zeigen (auch Probleme anzeigen)
- Keine konkreten nächsten Schritte

✅ Stattdessen:
- Top 5 Prioritäten
- Ehrliche Status-Anzeige
- Konkrete Handlungsanweisungen
