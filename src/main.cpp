#include <Arduino.h>
#include <TFT_eSPI.h>
#include "esp_sleep.h"

// ── Pins ──────────────────────────────
#define PUMP_PIN        43    // MOSFET Gate (GPIO 43)
#define BATTERY_PIN     4     // Batterie ADC (GPIO 4 - LCD_BAT_VOLT)
#define LCD_POWER_PIN   15    // LCD Power On (GPIO 15)

// ── Einstellungen ─────────────────────
#define PUMP_DURATION   15    // 15 Sekunden (Pumpe ist stark!)
#define SLEEP_DURATION  3600  // 1 Stunde
#define BAT_MIN         20    // % Mindestladung
#define R1              100000
#define R2              100000

// ── Display ───────────────────────────
TFT_eSPI tft = TFT_eSPI();

// ── Farben ────────────────────────────
#define FARBE_HINTERGRUND  TFT_BLACK
#define FARBE_TEXT         TFT_WHITE
#define FARBE_AKKU_VOLL    TFT_GREEN
#define FARBE_AKKU_HALB    TFT_YELLOW
#define FARBE_AKKU_LEER    TFT_RED
#define FARBE_WASSER       TFT_BLUE

// ── Funktionen ────────────────────────

void displayAn() {
  // LCD Power erst einschalten (wichtig!)
  pinMode(LCD_POWER_PIN, OUTPUT);
  digitalWrite(LCD_POWER_PIN, HIGH);
  delay(100);
  
  // Backlight erst nach Display-Init
  pinMode(38, OUTPUT);       // LCD_BL GPIO 38
  digitalWrite(38, HIGH);
  
  tft.init();
  tft.setRotation(0);        // Hochformat
  tft.fillScreen(FARBE_HINTERGRUND);
}

void displayAus() {
  tft.fillScreen(TFT_BLACK);
  digitalWrite(38, LOW);     // Backlight aus
  digitalWrite(LCD_POWER_PIN, LOW);  // LCD Power aus
}

int batterieProzent() {
  // ADC auf 12-Bit stellen
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  // Mehrere Messungen für stabileren Wert
  long summe = 0;
  for (int i = 0; i < 10; i++) {
    summe += analogRead(BATTERY_PIN);
    delay(5);
  }
  int adcWert = summe / 10;
  
  // Berechnung: 3.3V Referenz, Spannungsteiler 100k/100k (Faktor 2)
  float spannung = (adcWert / 4095.0) * 3.3;
  float batSpannung = spannung * 2.0;  // Spannungsteiler 1:1
  
  // Debug: Spannung ausgeben
  Serial.print("ADC Wert: ");
  Serial.print(adcWert);
  Serial.print(" | Spannung: ");
  Serial.print(batSpannung);
  Serial.println("V");
  
  // Umrechnung: 3.0V = 0%, 4.2V = 100%
  int prozent = map((int)(batSpannung * 100), 300, 420, 0, 100);
  return constrain(prozent, 0, 100);
}

uint32_t akkuFarbe(int prozent) {
  if (prozent > 50) return FARBE_AKKU_VOLL;
  if (prozent > 20) return FARBE_AKKU_HALB;
  return FARBE_AKKU_LEER;
}

void zeichneAkkuBalken(int prozent) {
  // Rahmen
  tft.drawRect(10, 50, 150, 30, TFT_WHITE);
  // Füllung
  int breite = map(prozent, 0, 100, 0, 146);
  tft.fillRect(12, 52, breite, 26, akkuFarbe(prozent));
  // Plus Pol
  tft.fillRect(160, 60, 8, 12, TFT_WHITE);
}

void zeigeHauptscreen(int prozent, String aktion, uint32_t aktionFarbe) {
  tft.fillScreen(FARBE_HINTERGRUND);
  
  // Titel
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(15, 10);
  tft.print("Bewaesserung");
  
  // Trennlinie
  tft.drawLine(0, 35, 170, 35, TFT_WHITE);
  
  // Akku Text
  tft.setTextSize(2);
  tft.setTextColor(akkuFarbe(prozent));
  tft.setCursor(10, 90);
  tft.print("Akku: ");
  tft.print(prozent);
  tft.print("%");
  
  // Akku Balken
  zeichneAkkuBalken(prozent);
  
  // Trennlinie
  tft.drawLine(0, 130, 170, 130, TFT_WHITE);
  
  // Aktion
  tft.setTextSize(2);
  tft.setTextColor(aktionFarbe);
  tft.setCursor(10, 145);
  tft.print(aktion);
}

void pumpeStarten(int prozent) {
  digitalWrite(PUMP_PIN, HIGH);
  
  for (int sek = PUMP_DURATION; sek > 0; sek--) {
    
    zeigeHauptscreen(prozent, "Bewaessere!", FARBE_WASSER);
    
    // Countdown
    tft.setTextSize(3);
    tft.setTextColor(TFT_CYAN);
    tft.setCursor(30, 200);
    tft.print(sek);
    tft.print(" Sek");
    
    // Wassertropfen Animation
    int tropfen = (sek % 3);
    tft.setTextSize(4);
    tft.setCursor(70, 260);
    if (tropfen == 0) tft.print(" ~  ");
    if (tropfen == 1) tft.print(" ~~  ");
    if (tropfen == 2) tft.print(" ~~~ ");
    
    delay(1000);
  }
  
  digitalWrite(PUMP_PIN, LOW);
  delay(500);  // Pausen nach Pumpe (Stabilisierung)
}

void geheSchlafen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(30, 150);
  tft.print("Schlafe...");
  tft.setCursor(30, 175);
  tft.print("1 Stunde");
  delay(2000);
  
  // Backlight aus, aber LCD Power bleibt an (verhindert Reset)
  digitalWrite(38, LOW);
  delay(100);
  
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000ULL);
  esp_deep_sleep_start();
}

// ── Setup ─────────────────────────────
void setup() {
  Serial.begin(115200);
  
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  
  // Display starten
  displayAn();
  
  // Startscreen
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(20, 140);
  tft.print("Starte...");
  delay(2000);
  
  // Batterie messen
  int prozent = batterieProzent();

  if (prozent >= BAT_MIN) {
    // Pumpe starten
    pumpeStarten(prozent);
    
    // Fertig Anzeige
    zeigeHauptscreen(prozent, "Fertig!", TFT_GREEN);
    delay(2000);
    
  } else {
    // Zu wenig Akku
    zeigeHauptscreen(prozent, "Akku leer!", TFT_RED);
    delay(3000);
  }

  geheSchlafen();
}

// ── Loop ──────────────────────────────
void loop() {
  // Nicht genutzt
}
