# Schaltplan - Sensoren für ESP32 Bewässerung

Dieses Dokument beschreibt die Verkabelung der beiden neuen Sensoren: **Bodenfeuchte** und **Wasserstand**.

---

## Übersicht - Komplette Schaltung

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         LILYGO T-DISPLAY S3                                 │
│                                                                             │
│    ┌─────────────────────────────────────────────────────────────────┐     │
│    │  GPIO 1  ────────┐                                             │     │
│    │  GPIO 2  ────────┼──────────┐                                  │     │
│    │  GPIO 10 ────────┼──────────┼──────┐                           │     │
│    │  GPIO 11 ────────┼──────────┼──────┼──────┐                    │     │
│    │  GND     ────────┼──────────┼──────┼──────┼────── 3.3V         │     │
│    │                  │          │      │      │      (5V)          │     │
│    └──────────────────┼──────────┼──────┼──────┼────────────────────┘     │
│                       │          │      │      │                           │
└───────────────────────┼──────────┼──────┼──────┼───────────────────────────┘
                        │          │      │      │
                        ▼          ▼      ▼      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│  ┌─────────────────┐    ┌────────────────────────────────────────────────┐  │
│  │  BODENFEUCHTE   │    │           WASSERSTAND (Doppelter Schwimmer)    │  │
│  │    SENSOR       │    │                                                │  │
│  │                 │    │   ┌─────────────┐      ┌─────────────┐         │  │
│  │  ┌───────────┐  │    │   │  Schwimmer  │      │  Schwimmer  │         │  │
│  │  │   VCC     │◄─┼────┘   │   OBEN      │      │   UNTEN     │         │  │
│  │  │  (3.3V)   │  │        │   (NC)      │      │   (NC)      │         │  │
│  │  └───────────┘  │        │             │      │             │         │  │
│  │                 │        │  ┌─────┐    │      │  ┌─────┐    │         │  │
│  │  ┌───────────┐  │        │  │     │◄───┼──────┼──┤     │    │         │  │
│  │  │   AOUT    ├──┼────────┼──┤ NC  │    │      │  │ NC  │    │         │  │
│  │  │ (Analog)  │  │        │  │     ├──┐ │      │  │     ├──┐ │         │  │
│  │  └───────────┘  │        │  └──┬──┘  │ │      │  └──┬──┘  │ │         │  │
│  │                 │        │     │     │ │      │     │     │ │         │  │
│  │  ┌───────────┐  │        └─────┼─────┘ └──────┴─────┼─────┘ │         │  │
│  │  │   GND     ├──┼──────────────┼────────────────────┼───────┘         │  │
│  │  │           │  │              │                    │                  │  │
│  │  └───────────┘  │              │                    │                  │  │
│  └─────────────────┘              │                    │                  │  │
│                                   ▼                    ▼                  │  │
│                              ┌────────┐          ┌────────┐               │  │
│                              │ 10kΩ   │          │ 10kΩ   │               │  │
│                              │Pull-Up │          │Pull-Up │               │  │
│                              └───┬────┘          └───┬────┘               │  │
│                                  │                   │                     │  │
│                                  └─────────┬─────────┘                     │  │
│                                            │                               │  │
│                                            ▼                               │  │
│                                          ────                              │  │
│                                           GND                              │  │
│                                          ────                              │  │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 1. Bodenfeuchte-Sensor (Capacitive Soil Moisture v1.2)

### Warum kapazitiv statt resistiv?
- **Keine Korrosion** - Kein direkter Stromfluss durch die Erde
- **Längere Lebensdauer** - Keine elektrolytische Zersetzung
- **Genauere Messung** - Unabhängig von der Leitfähigkeit des Wassers

### Schaltung

```
ESP32 T-Display S3                      Capacitive Soil Sensor
┌─────────────────────┐                 ┌─────────────────────────┐
│                     │                 │                         │
│    GPIO 2  ─────────┼────────────────►│ VCC (3.3V)              │
│    (Power)          │                 │                         │
│                     │                 │    ┌─────────────┐      │
│    GPIO 1  ◄────────┼─────────────────│    │  Capacitive │      │
│    (Analog)         │                 │    │   Sensor    │      │
│                     │                 │    │   Chip      │      │
│    GND ─────────────┼────────────────►│ GND│             │      │
│                     │                 │    │  ┌───────┐  │      │
│                     │                 │    │  │Platine│  │      │
│                     │                 │    │  │(300kHz│  │      │
│                     │                 │    │  │Signal)│  │      │
│                     │                 │    │  └───┬───┘  │      │
│                     │                 │    │      │      │      │
│                     │                 │    └──────┼──────┘      │
│                     │                 │           │             │
│                     │                 │       [Spitzen]         │
│                     │                 │       (in Erde)         │
│                     │                 │                         │
└─────────────────────┘                 └─────────────────────────┘
```

### Verkabelung

| LilyGo Pin | Sensor Pin | Farbe (Empfehlung) | Funktion |
|------------|-----------|-------------------|----------|
| GPIO 2 | VCC | Rot | Stromversorgung (ein/aus schaltbar) |
| GPIO 1 | AOUT | Gelb | Analoges Signal (0-3.3V) |
| GND | GND | Schwarz | Masse |

### Wichtig: Stromsparen!
```cpp
// In main.cpp:
digitalWrite(SOIL_POWER_PIN, HIGH);  // Sensor AN
// ... messen ...
digitalWrite(SOIL_POWER_PIN, LOW);   // Sensor AUS (Strom sparen!)
```

---

## 2. Wasserstand-Sensor (Doppelter Schwimmerschalter)

### Funktionsweise

**NC (Normally Closed)** bedeutet:
- Schwimmer **hängt unten** (Wasser niedrig) → Schalter ist **geschlossen**
- Schwimmer **schwimmt oben** (Wasser hoch) → Schalter ist **geöffnet**

### Tank-Levels

```
        ┌─────────────────────────┐
        │      TANK/WASSER        │
        │                         │
        │    ╭─────────────╮      │
        │    │ ● Schwimmer │ ← Oberer Kontakt (GPIO 10)
        │    │     OBEN    │     "Tank voll"
        │    ╰─────────────╯      │
        │           ~             │
        │           ~             │
        │    ╭─────────────╮      │
        │    │ ● Schwimmer │ ← Unterer Kontakt (GPIO 11)
        │    │    UNTEN    │     "Tank leer" WARNUNG
        │    ╰─────────────╯      │
        │                         │
        └─────────────────────────┘
```

### Elektrische Schaltung (pro Schwimmer)

```
                    3.3V
                     │
                    [10kΩ]  ← Pull-Up Widerstand
                     │
                     ├──────────────────► GPIO (10 oder 11)
                     │                      (Input mit Pull-Up)
                     │
              ┌──────┴──────┐
              │             │
              │   NC        │  ← Schließt bei "Wasser niedrig"
              │  Kontakt    │
              │             │
              └──────┬──────┘
                     │
                     │
                    GND
```

### Zustandstabelle (NC Schalter)

| Wasserstand | Schwimmer | Schalter | GPIO liest |
|-------------|-----------|----------|------------|
| **Hoch** | Schwimmt oben | **OFFEN** | HIGH (1) |
| **Niedrig** | Hängt unten | **GESCHLOSSEN** | LOW (0) |

### Kombination beider Schwimmer

| Oberer (GPIO 10) | Unterer (GPIO 11) | Zustand | Bedeutung |
|------------------|-------------------|---------|-----------|
| HIGH (voll) | HIGH (voll) | ✅ **Tank voll** | Optimal |
| LOW (leer) | HIGH (voll) | ⚠️ **Mitte** | Noch OK |
| LOW (leer) | LOW (leer) | 🚨 **Tank LEER** | Bewässerung STOPP |

```
Logik im Code:
┌──────────────┐
│ Wasser oben? │──HIGH?──┐
└──────────────┘         ├──BOTH HIGH?──► Tank VOLL
│ Wasser unten? │──HIGH?──┘
└──────────────┘

┌──────────────┐
│ Wasser oben? │──LOW?───┐
└──────────────┘         ├──BOTH LOW?───► Tank LEER!
│ Wasser unten? │──LOW?──┘
└──────────────┘
```

---

## 3. Komplettes Verdrahtungsdiagramm (Ansicht von oben)

```
┌────────────────────────────────────────────────────────────────┐
│                    LilyGo T-Display S3                         │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  USB                                                     │  │
│  │                                                          │  │
│  │  GPIO  1 ─────┬────────────────────────────────────────  │  │
│  │  GPIO  2 ─────┼────────┐                                 │  │
│  │  GPIO 10 ─────┼────────┼────────┐                        │  │
│  │  GPIO 11 ─────┼────────┼────────┼────────┐               │  │
│  │               │        │        │        │               │  │
│  │  3.3V ────────┼────────┼───┐    │    ┌───┼───┐           │  │
│  │               │        │   │    │    │   │   │           │  │
│  │  GND ─────────┼────────┼───┴────┼────┴───┼───┴────       │  │
│  │               │        │        │        │               │  │
│  └───────────────┼────────┼────────┼────────┼───────────────┘  │
│                  │        │        │        │                  │
└──────────────────┼────────┼────────┼────────┼──────────────────┘
                   │        │        │        │
         ┌─────────┘        │        │        └──────────┐
         │                  │        │                   │
         ▼                  ▼        ▼                   ▼
┌────────────────┐  ┌─────────────────────────────────────────┐
│  Bodenfeuchte  │  │         Doppelter Schwimmerschalter     │
│    Sensor      │  │                                         │
│  ┌──────────┐  │  │  ┌─────────┐        ┌─────────┐        │
│  │ VCC (R)  │◄─┘  │  │ GPIO 10 │        │ GPIO 11 │        │
│  │ AOUT (G) ├──┐  │  │    │    │        │    │    │        │
│  │ GND (B)  │◄─┘  │  │   [10kΩ]│        │   [10kΩ]│        │
│  └──────────┘     │  │    │    │        │    │    │        │
│                   │  │    ├────┴────────┴────┤    │        │
│                   │  │    │                  │    │        │
│                   │  │    │   NC Kontakte    │    │        │
│                   │  │    │   (Schwimmer)    │    │        │
│                   │  │    │                  │    │        │
│                   │  │    └────────┬─────────┘    │        │
│                   │  │             │              │        │
│                   │  │            GND            │        │
│                   │  │                           │        │
│                   │  └───────────────────────────┘        │
└───────────────────┘                                       └──────
```

---

## 4. Benötigte Bauteile

### Für Bodenfeuchte:
| Bauteil | Anzahl | Hinweis |
|---------|--------|---------|
| Capacitive Soil Moisture Sensor v1.2 | 1 | Korrisionsfrei, 3.3V kompatibel |
| Jumper Kabel | 3 | Female-Male oder Male-Male |

### Für Wasserstand:
| Bauteil | Anzahl | Hinweis |
|---------|--------|---------|
| Doppelter Schwimmerschalter (NC) | 1 | SUS 304, 0-220V DC |
| 10kΩ Widerstand | 2 | Pull-Up für beide Schwimmer |
| Jumper Kabel | 6 | Für Verbindungen |
| Lochrasterplatine (optional) | 1 | Für saubere Verkabelung |

---

## 5. Montage-Tipps

### Bodenfeuchte-Sensor
```
Empfohlene Einbauweise:

    ┌─────────────┐
    │   PFLANZE   │
    │             │
    │    🌱       │
    │             │
    │ ┌─────────┐ │
    │ │ Sensor  │ │ ← Ca. 5-10cm tief in Erde
    │ │ [====]  │ │   (Nicht direkt an Wurzeln!)
    │ │  Spitze │ │
    │ └─────────┘ │
    │             │
    └─────────────┘
    
Kabel nach außen führen
zum LilyGo Board
```

### Schwimmerschalter im Tank
```
         Tank-Deckel
    ┌─────────────────┐
    │    ┌─────┐      │
    │    │ ●   │ ← Oberer Schwimmer
    │    │  \  │   (Max. Wasserstand)
    │    │   \ │      
    │    │    \│      
    │    │     ● ← Unterer Schwimmer
    │    │     │   (Min. Wasserstand)
    │    └─────┘      
    │                 
    └─────────────────┘
    
    Einbau-Höhe:
    - Oberer: 90% Tankhöhe
    - Unterer: 20% Tankhöhe
```

---

## 6. Fehlersuche

### Problem: Bodenfeuchte zeigt falsche Werte
| Symptom | Ursache | Lösung |
|---------|---------|--------|
| Immer 0% | Sensor nicht eingeschaltet | Prüfe GPIO 2 Verbindung |
| Immer 100% | Sensor in Wasser getaucht | Kalibrieren: Trocken = ~3500, Nass = ~1500 |
| Werte springen | Schlechter Kontakt | Kabel prüfen, Lötverbindungen überprüfen |

### Problem: Wasserstand erkannt nicht korrekt
| Symptom | Ursache | Lösung |
|---------|---------|--------|
| Immer "voll" | Pull-Up fehlt | 10kΩ Widerstand einbauen |
| Immer "leer" | Kurzschluss nach GND | Verkabelung prüfen |
| Wackelkontakt | Schalter klemmt | Schwimmer mechanisch prüfen |

---

## 7. Sicherheitshinweise

⚠️ **Wichtig:**
- Der Schwimmerschalter ist für **220V AC** spezifiziert, wir nutzen nur **3.3V DC**!
- Bei hoher Spannung müsste ein Relais dazwischen geschaltet werden
- Unsere Schaltung ist **Low-Voltage** und ungefährlich

🔋 **Bodenfeuchte-Sensor:**
- Nie an 5V anschließen (außer Sensor ist 5V-tolerant)!
- Unser Sensor läuft mit 3.3V (GPIO 2 vom LilyGo)

---

**Nächster Schritt:** Siehe `README.md` für die Software-Konfiguration!
