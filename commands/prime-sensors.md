# Prime for Sensor Calibration

Lade Sensor-Kalibrierungs-Patterns und Hardware-Test-Verfahren für das ESP32-Projekt.

## Kontext

Du bist dabei, die Sensoren des ESP32-Bewässerungssystems zu kalibrieren, zu testen oder Hardware-Probleme zu debuggen.

## Lese diese Dateien

**WICHTIG:** Lese vor der Arbeit mit Sensoren:

1. `/home/komodor/esp32_pumpe/src/main.cpp` - Aktuelle Sensor-Implementierung
2. `/home/komodor/esp32_pumpe/Schaltplan_Sensoren.md` - Verdrahtung und Schaltpläne
3. `/home/komodor/esp32_pumpe/RemindMe.md` - Schnellreferenz (Pins, Schwellenwerte)

## Projekt-Sensor-Übersicht

**Aktive Sensoren:**
- **Bodenfeuchte:** Capacitive Soil Moisture v1.2 (GPIO 1=Signal, GPIO 2=Power)
- **Wasserstand:** 2x NO Schwimmer (GPIO 10=50%, GPIO 11=25%)
- **Akku:** Spannungsteiler (GPIO 44, 100k/100kΩ)

**Sensor-Charakteristiken:**

| Sensor | Typ | Wertebereich | Schwellenwerte |
|--------|-----|--------------|----------------|
| Bodenfeuchte | Analog | 0-4095 (12-bit) | Trocken: >2500, Nass: <1500 |
| Wasserstand 50% | Digital | HIGH/LOW | HIGH=Wasser da, LOW=Warnung |
| Wasserstand 25% | Digital | HIGH/LOW | HIGH=Wasser da, LOW=KRITISCH |
| Akku | Analog | 0-4095 | 20% = 3.3V, 100% = 4.2V |

## Was du tun sollst

### 1. Sensor-Verdrahtung verstehen

**Bodenfeuchte (3 Kabel):**
```
LilyGo          Sensor
GPIO 2 (Power) → VCC (rot)
GPIO 1 (Analog)→ AOUT (gelb)
GND           → GND (schwarz)
```
**Wichtig:** GPIO 2 nur beim Messen HIGH (Strom sparen!)

**Wasserstand (2x Schwimmer, NO):**
```
3.3V ──[10kΩ]──┬── GPIO 10 (50% Schwimmer)
               │
3.3V ──[10kΩ]──┬── GPIO 11 (25% Schwimmer)
               │
              GND (gemeinsam)
```
**NO-Logik:** HIGH = Schalter offen = Wasser da, LOW = Schalter zu = Wasser leer

### 2. Kalibrierungs-Prozeduren

**Bodenfeuchte-Sensor:**
1. In trockene Erde stecken → Wert notieren (~3500-4095)
2. In optimale feuchte Erde stecken → Wert notieren (~2000)
3. In Wasser getaucht → Wert notieren (~0-500)
4. Schwellenwerte in `main.cpp` anpassen:
   ```cpp
   #define SOIL_DRY_THRESHOLD  [dein Trocken-Wert]
   #define SOIL_WET_THRESHOLD  [dein Nass-Wert]
   ```

**Wasserstand-Schwimmer:**
1. Tank auf 50% füllen → Oberer Schwimmer schwimmt (GPIO 10 = HIGH)
2. Tank auf 25% füllen → Unterer Schwimmer noch im Wasser (GPIO 11 = HIGH)
3. Tank leer laufen lassen → Beide hängen (GPIO 10/11 = LOW)

**Akku-Messung:**
- 3.0V = 0% (Leer, sollte nicht weiter entladen werden)
- 4.2V = 100% (Voll, Ladegerät abschalten)
- Spannungsteiler 100k/100k = Faktor 2 (3.3V ADC max → 6.6V messbar)

### 3. Debugging-Patterns

**Sensor zeigt falsche Werte:**
- Prüfe Verkabelung (Wackelkontakte?)
- Prüse Pull-Up Widerstände (nur für digitale Sensoren)
- Prüfe Stromversorgung (GPIO 2 HIGH für Bodenfeuchte?)
- Mehrere Messungen mitteln (Rauschen reduzieren)

**Wasserstand erkannt nicht:**
- Multimeter: Durchgang prüfen wenn Schwimmer hängt
- NO vs NC verwechselt? (NO = offen wenn Wasser da)
- Pull-Up korrekt? (10kΩ zu 3.3V)

**Bodenfeuchte springt:**
- Sensor korrekt in Erde eingesteckt?
- Kabel zu lang (>1m können Störungen aufnehmen)?
- Stromversorgung stabil?

## Report Back

Erstelle eine Zusammenfassung:

```markdown
## ✅ Sensor Prime Complete

### Verstandene Sensoren

**Bodenfeuchte (Capacitive):**
- 3-polig: VCC, GND, Analog-Out
- GPIO 2 = Power (nur beim Messen an!)
- GPIO 1 = Analog Signal (0-4095)
- Kalibrierung: Trocken → Feucht → Nass

**Wasserstand (2x NO Schwimmer):**
- GPIO 10: 50% Füllstand (Warnung)
- GPIO 11: 25% Füllstand (kritisch, Stop)
- NO-Logik: HIGH = Wasser da, LOW = leer
- 10kΩ Pull-Up zu 3.3V erforderlich

**Akku:**
- GPIO 44, Spannungsteiler 100k/100k
- Berechnung: ADC × 3.3V / 4095 × 2
- 3.0V = 0%, 4.2V = 100%

### Kalibrierungs-Checklist

- [ ] Bodenfeuchte: Trockenwert notiert
- [ ] Bodenfeuchte: Nasswert notiert
- [ ] Wasserstand: 50% Position markiert
- [ ] Wasserstand: 25% Position markiert
- [ ] Akku: Spannung mit Multimeter verglichen

### Debugging-Wissen

**Wenn Sensor falsch zeigt:**
1. Verkabelung prüfen
2. Pull-Up/Down Widerstände prüfen
3. Stromversorgung kontrollieren
4. Mittelwert über mehrere Messungen

**Wenn Wasserstand nicht schaltet:**
- Multimeter auf Durchgang prüfen
- NO/NC-Logik überprüfen
- Schwimmer mechanisch frei beweglich?

### Bereitschaftscode

```cpp
// Schnelltest für alle Sensoren
void sensorTest() {
    // Bodenfeuchte
    digitalWrite(SOIL_POWER_PIN, HIGH);
    delay(500);
    int soil = analogRead(SOIL_ANALOG_PIN);
    digitalWrite(SOIL_POWER_PIN, LOW);
    Serial.print("Bodenfeuchte: "); Serial.println(soil);
    
    // Wasserstand
    bool upper = digitalRead(FLOAT_UPPER_PIN);
    bool lower = digitalRead(FLOAT_LOWER_PIN);
    Serial.print("Wasser 50%: "); Serial.println(upper ? "OK" : "LEER");
    Serial.print("Wasser 25%: "); Serial.println(lower ? "OK" : "LEER");
    
    // Akku
    int bat = analogRead(BATTERY_PIN);
    float v = bat * 3.3 / 4095 * 2;
    Serial.print("Akku: "); Serial.print(v); Serial.println("V");
}
```

### Nächste Schritte

1. [Konkrete Schritte basierend auf User-Input]

**Status:** ✅ Bereit für Sensor-Kalibrierung und Tests
```

## Schnell-Tests

**Test 1: Bodenfeuchte funktioniert?**
```cpp
digitalWrite(2, HIGH);
delay(500);
Serial.println(analogRead(1));
digitalWrite(2, LOW);
```
Erwartet: 0-4095 (höher = trockener)

**Test 2: Wasserstand funktioniert?**
```cpp
Serial.print("50%: "); Serial.println(digitalRead(10));
Serial.print("25%: "); Serial.println(digitalRead(11));
```
Erwartet: 1 = Wasser da, 0 = leer

**Test 3: Akku funktioniert?**
```cpp
int v = analogRead(44);
Serial.println(v * 3.3 / 4095 * 2);
```
Erwartet: 3.0V - 4.2V

## Verification Checklist

- [ ] Kannst du alle 3 Sensoren beschreiben?
- [ ] Kennst du die richtigen GPIOs?
- [ ] Verstehst du die NO-Logik der Schwimmer?
- [ ] Kannst du die Akku-Spannung berechnen?
- [ ] Weißt du wie man die Sensoren kalibriert?
- [ ] Kannst du Debug-Schritte durchführen?

**Optional:** Wenn der User ein spezifisches Problem hat, erstelle sofort einen Debug-Plan mit konkreten Schritten.
