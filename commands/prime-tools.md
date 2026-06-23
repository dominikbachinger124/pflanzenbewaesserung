# Prime for Tool Development

Lade Tool-Entwicklungs-Patterns und Projekt-Kontext für OpenCode.

## Kontext

Du bist dabei, ein neues Tool für das ESP32-Pflanzenbewässerungsprojekt zu entwickeln oder ein bestehendes zu modifizieren.

## Lese diese Dateien

**WICHTIG:** Lese die folgenden Referenz-Dateien, bevor du mit der Tool-Entwicklung beginnst:

1. `/home/komodor/esp32_pumpe/reference/adding_tools_guide.md` - Tool Docstring Patterns
2. `/home/komodor/esp32_pumpe/reference/rule.md` - Kurzversion der Tool-Regeln

## Projekt-Verständnis

**Projekt:** ESP32 Automatische Pflanzenbewässerung
- **Hardware:** LilyGo T-Display S3 (ESP32-S3 mit 170x320 TFT)
- **Sensoren:** Bodenfeuchte (kapazitiv), Wasserstand (2x Schwimmer NO), Akku-Messung
- **Aktoren:** Pumpe (GPIO 43 via MOSFET)
- **Framework:** Arduino/PlatformIO (C++)
- **Besonderheit:** Deep Sleep für Energiesparen, alle Sensoren nur bei Bedarf aktiv

**Aktuelle Features:**
- Intelligente Bewässerung (nur wenn Akku OK, Wasser da, Erde trocken)
- TFT-Display mit allen Sensorwerten
- Stündliche Prüfung mit Deep Sleep
- Bodenfeuchte-Sensor (GPIO 1/2) - nur beim Messen eingeschaltet
- Wasserstand (GPIO 10/11) - 2x NO Schwimmer (50% und 25%)

## Was du tun sollst

### 1. Tool Docstring Patterns verstehen

Aus den Reference-Dateien:

**7 Pflicht-Elemente für Agent-Tools:**
1. **One-Line Summary** - Klare Zweckbeschreibung
2. **Use This When** - 3-5 Szenarien wann das Tool richtig ist
3. **Do NOT Use This For** - Szenarien wo ANDERE Tools besser sind
4. **Args** - Parameter mit Erklärung WANN man welche Werte wählt
5. **Returns** - Rückgabewert mit Format-Details
6. **Performance Notes** - Token-Nutzung, Execution Time, Limits
7. **Examples** - 2-4 realistische Beispiele (keine "foo", "bar")

**Wichtige Prinzipien:**
- Guide Tool Selection → Agent wählt das richtige Tool
- Prevent Token Waste → Effiziente Parameter
- Enable Composition → Multi-Step Workflows
- Set Expectations → Performance & Limitations

### 2. Projekt-Kontext internalisieren

**Pin-Belegung (wichtig für Hardware-Tools):**
- GPIO 43: Pumpe (MOSFET)
- GPIO 44: Akku-ADC (Spannungsteiler)
- GPIO 1: Bodenfeuchte Signal (Analog)
- GPIO 2: Bodenfeuchte Power (Digital)
- GPIO 10: Wasserstand oben (50%, NO)
- GPIO 11: Wasserstand unten (25%, NO)
- GPIO 15: LCD Power
- GPIO 38: LCD Backlight

**Schwellenwerte:**
- Akku-Minimum: 20%
- Bodenfeuchte trocken: >2500 ADC (ca. 40%)
- Bodenfeuchte nass: <1500 ADC (ca. 70%)
- Pumpdauer: 15 Sekunden
- Schlafdauer: 3600 Sekunden (1 Stunde)

### 3. Tool-Vorschläge basierend auf Projekt-Kontext

Wenn das neue Tool eines der folgenden Problemfelder adressiert, schlage konkrete Implementierungen vor:

**Fehlende Features (Vorschläge):**
- **Scheduling:** Flexiblere Bewässerungszeiten (mehrmals täglich, wochentags)
- **Datenlogging:** Speichern von Feuchtigkeitsverläufen auf SD-Karte
- **WiFi/Bluetooth:** Fernsteuerung und Status-Update per App
- **Kalibrierung:** Automatische Kalibrierung der Bodenfeuchte-Sensoren
- **Mehrere Zonen:** Erweiterung auf 2-4 Pflanzen mit separaten Pumpen
- **Wetter-Integration:** Überspringen bei Regen (API-Anbindung)

**Tool-Kategorien für dieses Projekt:**
- Hardware-Abstraction (Sensor-Reader, Pump-Controller)
- Konfiguration (Threshold-Manager, Schedule-Editor)
- Monitoring (Data-Logger, Alert-System)
- Kommunikation (WiFi-Manager, Bluetooth-Interface)

## Report Back

Erstelle eine Zusammenfassung in diesem Format:

```markdown
## ✅ Prime Complete - Tool Development Ready

### Verstandene Patterns

**Key Principles (3-5 bullets):**
- Tool Docstrings schreiben für Agent Comprehension (nicht nur Humans)
- "Use This When" + "Do NOT Use" für klare Tool-Auswahl
- Performance Notes für Token-Effizienz
- Realistische Beispiele (nicht "foo", "bar")
- 7 Required Elements in Reihenfolge

**Template Structure:**
- One-Line Summary → Use This When → Do NOT Use → Args → Returns → Performance → Examples
- Parameter Guidance: WARUM welche Werte wählen
- Token-Kosten explizit dokumentieren

**Anti-Patterns to Avoid:**
- Vague guidance: "Use this when you need to work with notes" ❌
- Missing negative guidance → Tool confusion
- Toy examples: read_note("test.md") ❌
- No token/performance info ❌

### Projekt-Kontext

**Hardware-Setup:**
- LilyGo T-Display S3 (ESP32-S3, 170x320 TFT)
- 3 Sensoren: Bodenfeuchte (kapazitiv), Wasserstand (2x NO), Akku
- 1 Aktor: Pumpe (GPIO 43, MOSFET)
- Deep Sleep, stündliche Prüfung

**Aktueller Code-Stand:**
- Intelligente Bewässerungslogik implementiert
- Alle Sensoren + Display funktionieren
- Pin-Belegung definiert
- Schwellenwerte konfigurierbar

**Vorgeschlagene Tool-Erweiterungen (falls relevant):**
[Basierend auf dem Tool, das der User bauen will, oder generell nützlich:]

1. **schedule_manager** - Flexible Bewässerungszeiten
   - Use when: User wants multiple watering times per day
   - Do NOT use: For emergency watering (use pump_control directly)

2. **data_logger** - Speichern von Sensorwerten
   - Use when: Tracking moisture trends over time
   - Do NOT use: For real-time monitoring (use sensor_reader)

3. **wifi_manager** - Fernsteuerung
   - Use when: Remote control and status updates needed
   - Do NOT use: If device is offline-only (current mode)

### Bereitschaftscode

```cpp
// Template für ESP32 Tool-Implementation
// Nutzt diese Struktur als Ausgangspunkt

class ToolName {
public:
    // Constructor mit Pin-Konfiguration
    ToolName(int pin);
    
    // Hauptmethoden mit klaren Docstrings
    bool begin();  // Initialisierung
    ResultType action();  // Hauptaktion
    
    // Status/Getter
    StatusType getStatus();
    
private:
    int _pin;
    // Interne Zustände
};
```

### Nächste Schritte

Sobald der User das gewünschte Tool beschreibt:
1. Tool Docstring erstellen (7 Elemente)
2. Hardware-Pins zuordnen (falls Hardware-Tool)
3. Beispiel-Code mit realistischen Daten
4. Performance-Charakteristiken dokumentieren

**Status:** ✅ Bereit für Tool-Entwicklung
```

## Verification Checklist

Verifiziere dein Verständnis:

- [ ] Kannst du die 7 Required Elements für Tool Docstrings aufzählen?
- [ ] Verstehst du den Unterschied zwischen "Use This When" und "Do NOT Use This For"?
- [ ] Kennst du die Pin-Belegung des ESP32-Projekts?
- [ ] Kannst du Token-Effizienz-Berechnungen erklären?
- [ ] Hast du realistische Beispiele aus dem Projekt (z.B. "gpio=43" statt "gpio=1")?

**Optional:** Wenn der User das neue Tool bereits beschrieben hat, erstelle sofort einen Vorschlag mit vollständigem Tool Docstring basierend auf den Patterns.
