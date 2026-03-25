# AR Glasses Person Detection Simulator

Desktop simulation of an AR glasses person detection pipeline using YOLOv8-nano. Validates the detection UX before deploying to Hailo-8L + Raspberry Pi 5 hardware.

## Hardware Target

| Component | Spec |
|-----------|------|
| Compute | Hailo-8L M.2 (13 TOPS, 2.5W) on Raspberry Pi 5 |
| Camera | 120fps global shutter |
| Display | Monocular MicroOLED near-eye |
| Model | YOLOv8-nano |
| Target | 60 FPS sustained, <16ms pipeline latency |
| BOM | ~$285-450 |

## Setup

```bash
# Create virtual environment (recommended)
python3 -m venv venv
source venv/bin/activate  # Linux/macOS
# venv\Scripts\activate   # Windows

# Install dependencies
pip install -r requirements.txt

# Run (auto-downloads YOLOv8-nano weights on first run)
python main.py
```

Requires Python 3.10+ and a webcam.

## Keyboard Controls

| Key | Action |
|-----|--------|
| `Q` | Quit |
| `R` | Cycle resolution (1280x720 → 640x480 → 320x240) |
| `T` | Cycle confidence threshold (0.3 → 0.9) |
| `S` | Save screenshot to `screenshots/` |
| `L` | Toggle CSV metrics logging |
| `P` | Toggle person-only vs all COCO classes |
| `X` | Toggle scanline effect |

## Features

- **HUD-style bounding boxes** — corner brackets with confidence labels and person IDs (via ByteTrack)
- **Live performance dashboard** — FPS, inference/render/total latency, person count
- **Vignette effect** — simulates monocular lens optics
- **Scanline overlay** — optional retro AR aesthetic
- **Resolution cycling** — simulate different camera module outputs
- **CSV logging** — per-frame metrics for post-analysis (`metrics_log.csv`)
- **GPU acceleration** — automatic CUDA/MPS detection; CPU fallback

## Performance Expectations

| Platform | Expected FPS |
|----------|-------------|
| Modern laptop CPU | 20-30 FPS |
| CUDA GPU | 40-80 FPS |
| Apple MPS | 30-50 FPS |
| Hailo-8L (projected) | 60+ FPS (~4ms inference) |

## File Structure

```
ar-glasses-sim/
├── main.py              # Entry point
├── detector.py          # YOLOv8 wrapper with timing
├── renderer.py          # HUD overlay drawing
├── metrics.py           # FPS tracker + CSV logger
├── config.py            # Resolution presets, thresholds, colors
├── requirements.txt     # Dependencies
└── README.md            # This file
```
