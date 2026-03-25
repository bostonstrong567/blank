# AR Glasses — Assembly Guide

## Overview

```
┌─────────────────────────────────────────────────┐
│                  FRONT VIEW                      │
│                                                  │
│   [CAM]                                          │
│  ┌──┐  ╔══════════╗     ╔══════════╗             │
│  │OV│  ║ LEFT EYE ║─────║RIGHT EYE ║──[PRISM]   │
│  │92│  ║ (clear)  ║     ║(display) ║──[OLED]    │
│  │81│  ╚══════════╝     ╚══════════╝             │
│  └──┘        │                │                  │
│         [BATTERY]     [RPi5 + HAILO]             │
│         (left arm)    (right arm)                │
└─────────────────────────────────────────────────┘
```

## Tools Needed

- Phillips #1 screwdriver
- Soldering iron (for power wiring)
- Wire strippers
- Heat shrink tubing
- Double-sided tape / hot glue
- M2 screws and standoffs (from BOM)
- 3D printer (or order from JLCPCB/PCBWay)

## Step-by-Step Assembly

### Step 1: Print the Frame

1. Open `frame/glasses_frame.scad` in OpenSCAD
2. Adjust parameters if needed (face width, nose bridge)
3. Export as STL
4. Print settings:
   - Material: **PETG** (heat resistant) or PLA (prototype)
   - Layer height: 0.2mm
   - Infill: 25%
   - Supports: YES
5. Print parts separately: front frame, left temple, right temple
6. Sand the nose bridge smooth for comfort
7. Insert M2 threaded inserts at hinge points with soldering iron

### Step 2: Prepare Raspberry Pi 5 + Hailo-8L

1. Attach the **M.2 HAT+** to the Pi 5:
   - Connect the PCIe FPC cable from Pi to HAT
   - Secure with standoffs
2. Insert **Hailo-8L M.2** module into the HAT's M.2 slot
   - Apply thermal pad on top of Hailo chip
3. Apply **heatsink** to Pi 5's SoC
4. Connect **ArduCam OV9281** via 15cm FPC cable to CSI port
5. Flash **Raspberry Pi OS (64-bit Bookworm)** to MicroSD
6. Boot and run `setup_rpi.sh` to install all software

### Step 3: Set Up the Display

**Option A: Wisecoco MicroOLED (recommended)**
1. Connect MicroOLED panel to HDMI-to-MIPI driver board
2. Connect driver board to Pi 5's Micro HDMI 0 via FPC adapter
3. Mount MicroOLED in the right eye area of the frame
4. Position beam splitter prism at 45° angle in front of the display
   - The prism reflects the display image toward the eye
   - While remaining transparent to the real world

```
  Side view (right eye):

  [EYE] ←── [PRISM 45°] ←── [MicroOLED]
                ↑
           see-through
           (real world)
```

**Option B: Vufine+ (easier)**
1. Mount Vufine+ on the right temple using its built-in clip
2. Connect to Pi 5 via Micro HDMI cable
3. No prism needed — Vufine has its own optics

### Step 4: Wire the Power System

```
  Soldering order:

  1. TP4056 BAT+ / BAT- → LiPo battery (with JST connector)
  2. TP4056 OUT+ → SPDT switch center pin
  3. Switch ON pin → Boost converter VIN+
  4. TP4056 OUT- → Boost converter VIN-
  5. Boost VOUT → USB-C cable (cut and solder to VBUS/GND)
     OR use a PD boost board with built-in USB-C
  6. USB-C → Pi 5 power input
```

**SAFETY:**
- Double-check polarity before connecting battery
- Use a multimeter to verify 5.0-5.2V output before connecting Pi
- Add inline 5A fuse between battery and boost converter
- Secure all solder joints with heat shrink

### Step 5: Mount Components in Frame

**Right temple (compute):**
1. Place Pi 5 + HAT stack in the right temple enclosure
2. Route camera FPC cable through the frame to the front
3. Route HDMI FPC cable to the display housing
4. Secure with M2 screws through the mounting holes

**Left temple (power):**
1. Place LiPo battery in the left temple battery compartment
2. Mount TP4056 + boost converter alongside the battery
3. Route USB-C cable through the frame to the Pi
4. Place power switch at an accessible position

**Front frame:**
1. Mount ArduCam in the camera housing (front, above right eye)
2. Mount MicroOLED + prism in the display housing
3. Attach nose pads with adhesive
4. Thread elastic strap through strap mounts

### Step 6: Cable Management

- Use small zip ties or adhesive cable clips inside the temples
- Ensure FPC cables have gentle bends (no sharp folds)
- Leave enough slack at hinges for folding
- Tuck excess cable into the enclosure cavities

### Step 7: Final Assembly

1. Attach temple arms to front frame with M2 screws
2. Verify all connections
3. Power on (flip switch)
4. Run the software:

```bash
cd /opt/ar-glasses
source venv/bin/activate
python main_embedded.py --mode prod
```

5. Adjust display position until the HUD overlay is clearly visible
6. Tighten all screws
7. Add elastic strap for secure fit

## Fit Adjustment

- **Nose bridge:** Sand wider/narrower, or add silicone pads
- **Temple pressure:** Bend temple arms gently with heat gun
- **Display alignment:** Loosen prism mount, adjust angle, re-tighten
- **Weight balance:** Shift battery position along left temple

## Troubleshooting

| Issue | Fix |
|-------|-----|
| No video from camera | Check FPC cable seated fully. Run `libcamera-still -o test.jpg` |
| Hailo not detected | Check M.2 seated, PCIe enabled in config.txt. Run `hailortcli fw-control identify` |
| Display blank | Check HDMI cable. Try `tvservice -s` to see if display is detected |
| Low FPS | Reduce resolution. Check thermal throttling: `vcgencmd measure_temp` |
| Pi won't boot | Check power supply. Pi 5 needs reliable 5V/5A. Test with official PSU first |
| Battery drains fast | Check for short circuits. Measure current draw with multimeter |
| Overheating | Add vents to frame. Ensure heatsink has thermal paste. Reduce to 30fps |

## Weight Distribution

For comfortable wear, keep face weight under 80g:

```
  ┌── Front: ~60g ──┐
  │  Camera: 5g      │
  │  Display: 12g    │
  │  Frame: 25g      │
  │  Prism: 8g       │
  │  Cables: 10g     │
  └──────────────────┘

  ┌── Right temple: ~85g ──┐
  │  Pi 5 + HAT: 62g       │
  │  Heatsink: 8g           │
  │  Enclosure: 15g         │
  └─────────────────────────┘

  ┌── Left temple: ~110g ──┐
  │  Battery: 95g           │
  │  Charger + boost: 10g   │
  │  Enclosure: 15g         │
  └─────────────────────────┘

  Strap takes ~80% of temple weight off the ears.
```
