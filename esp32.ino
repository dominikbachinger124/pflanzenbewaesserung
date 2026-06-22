#include <Arduino.h>
#include <TFT_eSPI.h>
#include "esp_sleep.h"

// ═════════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONEN
// ═════════════════════════════════════════════════════════════════════════════

// ── Pumpen & Strom ─────────────────────
#define PUMP_PIN        43    // MOSFET Gate (GPIO 43)
#define BATTERY_PIN     44     // Batterie ADC (GPIO 44 - LCD_BAT_VOLT)
#define LCD_POWER_PIN   15    // LCD Power On (GPIO 15)

// ── Bodenfeuchte-Sensor ─────────────────
#define SOIL_POWER_PIN  2     // VCC für Bodenfeuchte (ein/aus)
#define SOIL_ANALOG_PIN 1     // Analoger Eingang (ADC1_CH0)

// ── Wasserstand-Sensoren (Doppelter Schwimmerschalter NC) ─────────────────
#define FLOAT_UPPER_PIN 10    // Oberer Schwimmer (Tank voll)
#define FLOAT_LOWER_PIN 11    // Unterer Schwimmer (Tank leer)

// ═════════════════════════════════════════════════════════════════════════════
// EINSTELLUNGEN
// ═════════════════════════════════════════════════════════════════════════════
#define PUMP_DURATION       15      // Sekunden Pumpzeit
#define SLEEP_DURATION      3600    // Sekunden Schlafdauer (1 Stunde)
#define BAT_MIN             20      // Mindestladung %
#define R1                  100000  // Spannungsteiler R1
#define R2                  100000  // Spannungsteiler R2

// ── Bodenfeuchte Schwellenwerte ─────────
// Capacitive Sensor: 0-4095 (trocken = hoch, nass = niedrig)
#define SOIL_DRY_THRESHOLD  2500    // Ab hier ist es trocken (gießen!)
#define SOIL_WET_THRESHOLD  1500    // Ab hier ist es nass (nicht gießen)
#define SOIL_MEASURE_TIME   500     // ms zum Stabilisieren

// ═════════════════════════════════════════════════════════════════════════════
// DISPLAY
// ═════════════════════════════════════════════════════════════════════════════
TFT_eSPI tft = TFT_eSPI();

// ── Farben ──────────────────────────────
#define FARBE_HINTERGRUND   TFT_BLACK
#define FARBE_TEXT          TFT_WHITE
#define FARBE_AKKU_VOLL     TFT_GREEN
#define FARBE_AKKU_HALB     TFT_YELLOW
#define FARBE_AKKU_LEER     TFT_RED
#define FARBE_WASSER        TFT_BLUE
#define FARBE_WARNUNG       TFT_ORANGE
#define FARBE_ERDE_TROCKEN  TFT_RED
#define FARBE_ERDE_FEUCHT   TFT_GREEN

// ═════════════════════════════════════════════════════════════════════════════
// DATENSTRUKTUREN
// ═════════════════════════════════════════════════════════════════════════════
struct SensorDaten {
    int akkuProzent;
    int bodenfeuchte;       // 0-4095 (raw ADC)
    int bodenfeuchteProzent; // 0-100% (trocken = 0%, nass = 100%)
    bool wasserOben;        // true = voll
    bool wasserUnten;       // true = voll
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

// ═════════════════════════════════════════════════════════════════════════════
// DISPLAY FUNKTIONEN
// ═════════════════════════════════════════════════════════════════════════════

void displayAn() {
    pinMode(LCD_POWER_PIN, OUTPUT);
    digitalWrite(LCD_POWER_PIN, HIGH);
    delay(100);
    
    pinMode(38, OUTPUT);       // LCD_BL GPIO 38
    digitalWrite(38, HIGH);
    
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(FARBE_HINTERGRUND);
}

void displayAus() {
    tft.fillScreen(TFT_BLACK);
    digitalWrite(38, LOW);
    digitalWrite(LCD_POWER_PIN, LOW);
}

// ═════════════════════════════════════════════════════════════════════════════
// AKKU FUNKTIONEN
// ═════════════════════════════════════════════════════════════════════════════

int batterieProzent() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    long summe = 0;
    for (int i = 0; i < 10; i++) {
        summe += analogRead(BATTERY_PIN);
        delay(5);
    }
    int adcWert = summe / 10;
    
    float spannung = (adcWert / 4095.0) * 3.3;
    float batSpannung = spannung * 2.0;
    
    int prozent = map((int)(batSpannung * 100), 300, 420, 0, 100);
    return constrain(prozent, 0, 100);
}

uint32_t akkuFarbe(int prozent) {
    if (prozent > 50) return FARBE_AKKU_VOLL;
    if (prozent > 20) return FARBE_AKKU_HALB;
    return FARBE_AKKU_LEER;
}

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
    
    // ADC auf korrekten Pin setzen
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
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

bool sollGiessen(int bodenfeuchteRaw) {
    return bodenfeuchteRaw > SOIL_DRY_THRESHOLD;
}

// ═════════════════════════════════════════════════════════════════════════════
// WASSERSTAND FUNKTIONEN (Doppelter NC Schwimmerschalter)
// ═════════════════════════════════════════════════════════════════════════════

void initWasserstand() {
    pinMode(FLOAT_UPPER_PIN, INPUT_PULLUP);  // Pull-Up für NC Schalter
    pinMode(FLOAT_LOWER_PIN, INPUT_PULLUP);
}

bool leseWasserstandOben() {
    // NC Schalter: LOW = geschlossen (Wasser hoch), HIGH = offen (Wasser niedrig)
    return digitalRead(FLOAT_UPPER_PIN) == HIGH;
}

bool leseWasserstandUnten() {
    // NC Schalter: LOW = geschlossen (Wasser niedrig), HIGH = offen (Wasser hoch)
    return digitalRead(FLOAT_LOWER_PIN) == HIGH;
}

// ═════════════════════════════════════════════════════════════════════════════
// ALLE SENSOREN AUSLESEN
// ═════════════════════════════════════════════════════════════════════════════

SensorDaten leseAlleSensoren() {
    SensorDaten daten;
    
    daten.akkuProzent = batterieProzent();
    daten.bodenfeuchte = leseBodenfeuchte();
    daten.bodenfeuchteProzent = bodenfeuchteZuProzent(daten.bodenfeuchte);
    daten.wasserOben = leseWasserstandOben();
    daten.wasserUnten = leseWasserstandUnten();
    
    // Logik für Tank-Zustand
    // Oberer offen (HIGH) + Unterer offen (HIGH) = Tank voll
    // Oberer geschlossen (LOW) + Unterer geschlossen (LOW) = Tank leer
    daten.tankVoll = daten.wasserOben && daten.wasserUnten;
    daten.tankLeer = !daten.wasserOben && !daten.wasserUnten;
    
    return daten;
}

// ═════════════════════════════════════════════════════════════════════════════
// DISPLAY ZEICHNEN
// ═════════════════════════════════════════════════════════════════════════════

void zeichneAkkuBalken(int prozent) {
    tft.drawRect(10, 40, 150, 20, TFT_WHITE);
    int breite = map(prozent, 0, 100, 0, 146);
    tft.fillRect(12, 42, breite, 16, akkuFarbe(prozent));
    tft.fillRect(160, 46, 6, 8, TFT_WHITE);
    
    tft.setTextSize(1);
    tft.setTextColor(FARBE_TEXT);
    tft.setCursor(60, 45);
    tft.print(prozent);
    tft.print("%");
}

void zeichneBodenfeuchteBalken(int prozent) {
    int yPos = 110;
    uint32_t farbe = (prozent < 30) ? FARBE_ERDE_TROCKEN : FARBE_ERDE_FEUCHT;
    
    // Rahmen
    tft.drawRect(10, yPos, 150, 20, TFT_WHITE);
    // Füllung
    int breite = map(prozent, 0, 100, 0, 146);
    tft.fillRect(12, yPos + 2, breite, 16, farbe);
    
    // Text
    tft.setTextSize(1);
    tft.setTextColor(FARBE_TEXT);
    tft.setCursor(55, yPos + 5);
    tft.print(prozent);
    tft.print("%");
}

void zeichneWasserstand(SensorDaten& daten) {
    int yPos = 170;
    
    tft.setTextSize(1);
    tft.setTextColor(FARBE_TEXT);
    tft.setCursor(10, yPos);
    tft.print("Wasser:");
    
    // Oberer Schwimmer (Tank voll)
    if (daten.wasserOben) {
        tft.fillCircle(80, yPos + 5, 8, FARBE_WASSER);
        tft.setTextColor(TFT_BLACK);
        tft.setCursor(77, yPos + 2);
        tft.print("O");
    } else {
        tft.drawCircle(80, yPos + 5, 8, FARBE_TEXT);
        tft.setTextColor(FARBE_TEXT);
        tft.setCursor(77, yPos + 2);
        tft.print("O");
    }
    
    // Unterer Schwimmer (Tank leer)
    if (daten.wasserUnten) {
        tft.fillCircle(110, yPos + 5, 8, FARBE_WASSER);
        tft.setTextColor(TFT_BLACK);
        tft.setCursor(107, yPos + 2);
        tft.print("U");
    } else {
        tft.drawCircle(110, yPos + 5, 8, FARBE_TEXT);
        tft.setTextColor(FARBE_TEXT);
        tft.setCursor(107, yPos + 2);
        tft.print("U");
    }
    
    // Status Text
    tft.setTextColor(FARBE_TEXT);
    tft.setCursor(130, yPos);
    if (daten.tankLeer) {
        tft.setTextColor(FARBE_ERDE_TROCKEN);
        tft.print("LEER!");
    } else if (daten.tankVoll) {
        tft.setTextColor(FARBE_ERDE_FEUCHT);
        tft.print("OK");
    } else {
        tft.setTextColor(FARBE_WARNUNG);
        tft.print("MITTE");
    }
}

void zeigeHauptscreen(SensorDaten& daten, String status, uint32_t statusFarbe) {
    tft.fillScreen(FARBE_HINTERGRUND);
    
    // Titel
    tft.setTextColor(FARBE_WASSER);
    tft.setTextSize(2);
    tft.setCursor(20, 5);
    tft.print("Smart Bew.");
    
    tft.drawLine(0, 25, 170, 25, TFT_WHITE);
    
    // Akku
    tft.setTextSize(1);
    tft.setTextColor(FARBE_TEXT);
    tft.setCursor(10, 30);
    tft.print("Akku:");
    zeichneAkkuBalken(daten.akkuProzent);
    
    // Bodenfeuchte
    tft.setTextSize(1);
    tft.setTextColor(FARBE_TEXT);
    tft.setCursor(10, 100);
    tft.print("Erde:");
    zeichneBodenfeuchteBalken(daten.bodenfeuchteProzent);
    
    // Wasserstand
    zeichneWasserstand(daten);
    
    // Trennlinie
    tft.drawLine(0, 190, 170, 190, TFT_WHITE);
    
    // Status
    tft.setTextSize(2);
    tft.setTextColor(statusFarbe);
    tft.setCursor(10, 200);
    tft.print(status);
}

// ═════════════════════════════════════════════════════════════════════════════
// BEWÄSSERUNGSFUNKTION
// ═════════════════════════════════════════════════════════════════════════════

BewaesserungsStatus pruefeBewaesserung(SensorDaten& daten, String& grund) {
    // 1. Akku prüfen
    if (daten.akkuProzent < BAT_MIN) {
        grund = "Akku leer!";
        return STATUS_AKKU_LEER;
    }
    
    // 2. Wasserstand prüfen
    if (daten.tankLeer) {
        grund = "Tank leer!";
        return STATUS_TANK_LEER;
    }
    
    // 3. Bodenfeuchte prüfen
    if (!sollGiessen(daten.bodenfeuchte)) {
        grund = "Erde nass";
        return STATUS_ZU_NASS;
    }
    
    grund = "Bewaessere!";
    return STATUS_GIESST;
}

void pumpeStarten(SensorDaten& daten) {
    digitalWrite(PUMP_PIN, HIGH);
    
    for (int sek = PUMP_DURATION; sek > 0; sek--) {
        String dummy;
        zeigeHauptscreen(daten, "Bewaessere!", FARBE_WASSER);
        
        // Countdown
        tft.setTextSize(3);
        tft.setTextColor(TFT_CYAN);
        tft.setCursor(45, 230);
        tft.print(sek);
        tft.print("s");
        
        // Animation
        int tropfen = (sek % 3);
        tft.setTextSize(2);
        tft.setCursor(90, 235);
        if (tropfen == 0) tft.print("~");
        if (tropfen == 1) tft.print("~~");
        if (tropfen == 2) tft.print("~~~");
        
        delay(1000);
    }
    
    digitalWrite(PUMP_PIN, LOW);
    delay(500);
}

void zeigeErgebnis(BewaesserungsStatus status, SensorDaten& daten) {
    String meldung;
    uint32_t farbe;
    
    switch (status) {
        case STATUS_GIESST:
            meldung = "Fertig!";
            farbe = FARBE_ERDE_FEUCHT;
            break;
        case STATUS_ZU_NASS:
            meldung = "Nass -> OK";
            farbe = FARBE_WASSER;
            break;
        case STATUS_TANK_LEER:
            meldung = "Wasser?";
            farbe = FARBE_ERDE_TROCKEN;
            break;
        case STATUS_AKKU_LEER:
            meldung = "Laden!";
            farbe = FARBE_ERDE_TROCKEN;
            break;
        default:
            meldung = "OK";
            farbe = FARBE_TEXT;
    }
    
    zeigeHauptscreen(daten, meldung, farbe);
    delay(3000);
}

void geheSchlafen() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(FARBE_TEXT);
    tft.setTextSize(2);
    tft.setCursor(30, 150);
    tft.print("Schlafe...");
    
    tft.setTextSize(1);
    tft.setCursor(50, 170);
    tft.print("Naechste Pruefung:");
    tft.setCursor(60, 185);
    tft.print("in 1 Stunde");
    
    delay(2000);
    
    digitalWrite(38, LOW);
    delay(100);
    
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000ULL);
    esp_deep_sleep_start();
}

// ═════════════════════════════════════════════════════════════════════════════
// SETUP & LOOP
// ═════════════════════════════════════════════════════════════════════════════

void setup() {
    Serial.begin(115200);
    
    // Pumpen-Pin
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);
    
    // Sensoren initialisieren
    initBodenfeuchte();
    initWasserstand();
    
    // Display starten
    displayAn();
    
    // Startscreen
    tft.setTextColor(FARBE_WASSER);
    tft.setTextSize(2);
    tft.setCursor(25, 140);
    tft.print("Starte...");
    delay(1500);
    
    // Alle Sensoren auslesen
    SensorDaten daten = leseAlleSensoren();
    
    // Debug Ausgabe
    Serial.println("=== Sensorwerte ===");
    Serial.print("Akku: "); Serial.print(daten.akkuProzent); Serial.println("%");
    Serial.print("Bodenfeuchte: "); Serial.print(daten.bodenfeuchte); 
    Serial.print(" ("); Serial.print(daten.bodenfeuchteProzent); Serial.println("%)");
    Serial.print("Wasser oben: "); Serial.println(daten.wasserOben ? "VOLL" : "LEER");
    Serial.print("Wasser unten: "); Serial.println(daten.wasserUnten ? "VOLL" : "LEER");
    Serial.print("Tank leer: "); Serial.println(daten.tankLeer ? "JA" : "NEIN");
    
    // Bewässerungsentscheidung
    String grund;
    BewaesserungsStatus status = pruefeBewaesserung(daten, grund);
    
    // Anzeigen
    zeigeHauptscreen(daten, grund, 
        status == STATUS_GIESST ? FARBE_WASSER : 
        status == STATUS_ZU_NASS ? FARBE_ERDE_FEUCHT : FARBE_ERDE_TROCKEN);
    delay(2000);
    
    // Bewässern falls nötig
    if (status == STATUS_GIESST) {
        pumpeStarten(daten);
    }
    
    // Ergebnis anzeigen
    zeigeErgebnis(status, daten);
    
    // Schlafen gehen
    geheSchlafen();
}

void loop() {
    // Nicht genutzt - Deep Sleep Modus
}
