# AR Glasses Person Detection Simulator — Project Context

## Project Overview
Building AR glasses that overlay real-time 2D bounding boxes around detected persons. This repo contains both the desktop/web simulator and the full hardware design for physical glasses.

## Hardware Target
- **Compute:** Raspberry Pi 5 (8GB) + Hailo-8L M.2 (13 TOPS, 2.5W)
- **Camera:** ArduCam OV9281 global shutter, 120fps, MIPI CSI-2
- **Display:** Wisecoco 0.39" MicroOLED (1920x1080) with beam splitter prism
- **Model:** YOLOv8-nano (compiled to .hef for Hailo)
- **Target:** 60 FPS sustained, <16ms total pipeline latency
- **BOM:** ~$290-385
- **3D Printer:** Anycubic Kobra 2 Pro (PETG, profile included)

## Repo Structure
```
ar-glasses-sim/
├── main.py                  # Desktop simulator entry point (OpenCV + ultralytics)
├── detector.py              # YOLOv8 wrapper with timing
├── renderer.py              # HUD overlay (corner brackets, vignette, scanlines)
├── metrics.py               # FPS tracker + CSV logger
├── config.py                # Resolution presets, thresholds, colors
├── requirements.txt         # Python deps: opencv-python, ultralytics, numpy
├── web/
│   └── index.html           # Mobile web simulator (TF.js + COCO-SSD, works on iPhone Safari)
├── embedded/
│   ├── main_embedded.py     # RPi 5 production pipeline (--mode dev|prod|bench)
│   ├── hailo_detector.py    # HailoRT Python API wrapper for Hailo-8L
│   ├── camera.py            # picamera2/libcamera capture at 120fps
│   ├── display.py           # MicroOLED output (HDMI/framebuffer/OpenCV) + HUD renderer
│   └── setup_rpi.sh         # RPi 5 setup script (HailoRT, PCIe, camera)
├── hardware/
│   ├── BOM.md               # Full bill of materials with prices and sources
│   ├── wiring.svg           # Wiring diagram (Pi ↔ Hailo ↔ Camera ↔ Display ↔ Power)
│   └── power_schematic.md   # Power system: LiPo → TP4056 → Boost → USB-C PD → Pi
├── frame/
│   ├── glasses_frame.scad           # Parametric OpenSCAD source
│   ├── generate_stl.py              # Generates all STL files
│   ├── kobra2pro_profile.curaprofile # Pre-configured Cura slicer profile
│   ├── PRINT_GUIDE.md               # Print instructions for Kobra 2 Pro
│   └── stl/                         # Print-ready STL files (6 parts, 7 pieces)
│       ├── 01_front_frame.stl
│       ├── 02_right_temple.stl      # Has RPi 5 + Hailo enclosure
│       ├── 03_left_temple.stl       # Has battery compartment
│       ├── 04_battery_lid.stl
│       ├── 05_pi_enclosure_lid.stl
│       └── 06_nose_pad.stl          # Print 2x
└── ASSEMBLY_GUIDE.md        # Step-by-step build instructions
```

## Key Design Decisions
- Python is the right language — inference runs on Hailo silicon (~4ms), camera/rendering calls C++ under the hood. Python overhead is ~1-2ms, well within the 16ms budget.
- Corner bracket bounding boxes (not full rectangles) for AR HUD aesthetic in cyan (#88AAFF).
- ByteTrack for person ID tracking across frames.
- Weight distribution: compute on right temple, battery on left temple, elastic strap takes ~80% of weight off ears.
- Desktop simulator uses ultralytics YOLOv8n (auto-downloads weights). Embedded uses HailoRT with pre-compiled .hef.

## Development Workflow
- **Branch:** `claude/ar-glasses-simulator-iE9iI`
- **Desktop simulator:** `cd ar-glasses-sim && pip install -r requirements.txt && python main.py`
- **Web simulator:** Live at GitHub Pages (gh-pages branch) or open `web/index.html` locally
- **Embedded (RPi):** Run `setup_rpi.sh`, then `python embedded/main_embedded.py --mode prod`
- **3D printing:** Open STLs in Cura, import `kobra2pro_profile.curaprofile`, slice, print

## What's Next
- Test desktop simulator with webcam locally
- Print frame parts on Kobra 2 Pro
- Order hardware from BOM
- Flash RPi 5, run setup_rpi.sh, deploy embedded pipeline
- Assemble following ASSEMBLY_GUIDE.md
