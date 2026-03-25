# Power Management Schematic

## Circuit Overview

```
                    ┌──────────────┐
  USB-C Charge ────►│  TP4056      │
  (5V input)        │  LiPo Charger│
                    │  + DW01 Prot.│
                    └──────┬───────┘
                           │ 3.7V-4.2V
                    ┌──────▼───────┐
                    │  LiPo Cell   │
                    │  5000mAh     │
                    │  3.7V nominal│
                    └──────┬───────┘
                           │
                    ┌──────▼───────┐
                    │  SPDT Switch │
                    │  (Power ON)  │
                    └──────┬───────┘
                           │
                    ┌──────▼───────┐
                    │  Boost Conv. │
                    │  MT3608 /    │
                    │  IP5306      │
                    │  3.7V → 5V   │
                    │  5A max      │
                    └──────┬───────┘
                           │ 5V / 5A USB-C PD
              ┌────────────┼────────────────┐
              │            │                │
       ┌──────▼──┐  ┌──────▼──────┐  ┌──────▼──────┐
       │ Pi 5    │  │ MicroOLED   │  │ Status LED  │
       │ 5W typ. │  │ + Driver    │  │ (optional)  │
       │ 12W pk  │  │ 0.5W        │  │             │
       └────┬────┘  └─────────────┘  └─────────────┘
            │
     ┌──────▼──────┐
     │ Hailo-8L    │  (powered via PCIe from Pi)
     │ 2.5W        │
     └─────────────┘
```

## TP4056 Charger Wiring

```
          TP4056 Module
  ┌─────────────────────────┐
  │  IN+  ──── USB-C VBUS   │
  │  IN-  ──── USB-C GND    │
  │  BAT+ ──── LiPo +       │
  │  BAT- ──── LiPo -       │
  │  OUT+ ──── To Switch     │
  │  OUT- ──── Common GND    │
  │  CHRG ──── (LED, opt.)   │
  │  STBY ──── (LED, opt.)   │
  └─────────────────────────┘

  Rprog resistor: 1.2kΩ (1A charge rate)
  For faster charge: 0.6kΩ (2A) — check USB-C source capacity
```

## Boost Converter Wiring

```
  MT3608 or IP5306 Module
  ┌─────────────────────────┐
  │  VIN+ ──── Switch OUT    │
  │  VIN- ──── Common GND    │
  │  VOUT+ ─── Pi 5 USB-C +  │
  │  VOUT- ─── Pi 5 USB-C -  │
  │                           │
  │  Adjust pot → set 5.1V    │
  └─────────────────────────┘

  IMPORTANT: Pi 5 expects USB-C PD negotiation.
  Options:
    A) Use IP5306-based power bank board (handles PD)
    B) Add a PD trigger chip (CH224K) set to 5V/3A
    C) Use a PD-compatible boost board (recommended)

  Recommended board: "PD 5V 5A boost board" from AliExpress
  — has built-in PD negotiation, just connect LiPo in, USB-C out
```

## CH224K PD Trigger (if using raw boost converter)

```
  CH224K PD Trigger Chip
  ┌─────────────────────────┐
  │  VIN ──── Boost 5V OUT   │
  │  CC1 ──── USB-C CC1 pin  │
  │  CC2 ──── USB-C CC2 pin  │
  │  CFG1,2,3 → set voltage: │
  │    All float = 5V         │
  │    CFG1=GND = 9V          │
  │    CFG2=GND = 12V         │
  │    CFG3=GND = 20V         │
  │  VBUS ─── USB-C VBUS out  │
  │  GND ──── Common GND      │
  └─────────────────────────┘

  For Pi 5: Leave all CFG pins floating = 5V output
```

## Battery Life Calculations

```
  Battery: 5000mAh × 3.7V = 18.5 Wh

  Boost efficiency: ~90%
  Usable energy: 18.5 × 0.9 = 16.65 Wh

  Load scenarios:
  ┌──────────────────┬──────────┬───────────┐
  │ Mode             │ Draw (W) │ Runtime   │
  ├──────────────────┼──────────┼───────────┤
  │ Full (60fps det) │ 7.3W     │ 2h 17min  │
  │ Eco (30fps)      │ 5.5W     │ 3h 02min  │
  │ Standby (no det) │ 3.0W     │ 5h 33min  │
  │ Idle (display on)│ 1.5W     │ 11h 06min │
  └──────────────────┴──────────┴───────────┘

  For 4+ hours: use 10000mAh battery (adds ~95g)
```

## Safety Notes

1. **Never** short LiPo terminals — fire hazard
2. TP4056 with DW01 protection handles over-discharge, over-charge, short
3. Add a 5A fuse inline between battery and boost converter
4. Battery should be in a fire-resistant enclosure (or use pouch cell with built-in protection)
5. Keep battery away from heatsink / hot components
6. For flight/travel: battery under 100Wh (18.5Wh is well under)
