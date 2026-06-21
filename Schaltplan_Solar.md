# Solarzellen Schaltplan für ESP32 Bewässerung

## Das Problem
- Eine Solarzelle gibt nur ca. **0.5-0.6V** unter Last
- Der TP4056 Lademodul braucht ca. **5V** Eingang

## Variante 1: Parallel (nur wenn Panel bereits 5V hat)
```
Solar Panel 5V
     │
     ├───┐
     │   │
    [+] [+]   ← Plus zu Plus
     │   │
     │   │
    [-] [-]   ← Minus zu Minus
     │   │
     └───┘
       │
       ▼
   TP4056 IN+
   TP4056 IN-
```

## Variante 2: Reihe (empfohlen für kleine Zellen)
```
Zelle 1       Zelle 2       Zelle 3... bis ~5-6V
  [+] ────────> [-] [+] ────────> [-] [+]
   │                               │
 0.5V                           0.5V     = 5V bei 10 Zellen
   │                               │
  [-] <────────────────────────── [-]
                  │
                  ▼
            TP4056 IN+
            TP4056 IN-
```

## Empfohlene Lösung: Fertiges Solarpanel 6V
```
┌──────────────────┐
│  Solarpanel 6V │────> TP4056 IN+
│   (5-6 Watt)   │
└──────────────────┘
         │
         └──────────> TP4056 IN-
```

## WICHTIGE REGELN

| Schaltung | Spannung | Strom |
|-----------|----------|-------|
| **Parallel** | Bleibt gleich | Addiert sich |
| **Reihe** | Addiert sich | Bleibt gleich |

### Beispiel mit 10 Zellen (je 0,5V / 100mA):
- **Parallel**: 0,5V / 1000mA → **Zu wenig Spannung für TP4056!**
- **Reihe**: 5V / 100mA → **Perfekt für TP4056!**

## Mein Vorschlag

**Kaufe ein fertiges 6V Solarpanel** (ca. 5-10 Watt) zum Anschluss an den TP4056.

Oder wenn du viele kleine Zellen hast:
- **10 Zellen in Reihe** schalten = ~5V bei Ausreichend Sonne
- Plus-Punkt der ersten Zelle an TP4056 IN+
- Minus-Punkt der letzten Zelle an TP4056 IN-

## Schutz bei vielen Zellen
Bei mehreren Zellen in Reihe kann bei voller Sonne die Spannung über 5V steigen!
→ Optional: Spannungsregler (5V Festspannungsregler) oder Laderegler mit Überhitzungsschutz
