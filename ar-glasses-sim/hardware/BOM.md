# AR Glasses — Bill of Materials

## Core Compute

| # | Component | Spec | Qty | Est. Price | Source |
|---|-----------|------|-----|-----------|--------|
| 1 | Raspberry Pi 5 (8GB) | Quad Cortex-A76, 8GB RAM | 1 | $80 | raspberrypi.com, Adafruit, PiShop |
| 2 | Hailo-8L M.2 AI Accelerator | 13 TOPS, M.2 2242, 2.5W TDP | 1 | $70 | hailo.ai, Seeed Studio |
| 3 | Raspberry Pi M.2 HAT+ | M.2 Key-M to Pi 5 PCIe adapter | 1 | $12 | raspberrypi.com |
| 4 | MicroSD Card (32GB+) | Class 10, A2 rated | 1 | $10 | Amazon |

## Camera

| # | Component | Spec | Qty | Est. Price | Source |
|---|-----------|------|-----|-----------|--------|
| 5 | ArduCam OV9281 Global Shutter | 1MP, 120fps, MIPI CSI-2 | 1 | $30 | ArduCam, Amazon |
| 6 | RPi Camera Cable (15cm) | 22-pin to 15-pin FPC | 1 | $4 | Amazon, Adafruit |

## Display

| # | Component | Spec | Qty | Est. Price | Source |
|---|-----------|------|-----|-----------|--------|
| 7 | Wisecoco 0.39" MicroOLED | 1920x1080, MIPI DSI, monocular | 1 | $55 | AliExpress, Wisecoco |
| 8 | MicroOLED Driver Board | HDMI-to-MIPI adapter board | 1 | $20 | AliExpress (bundled w/ display) |
| 9 | Beam Splitter / Prism | 25mm semi-mirror, 50/50 | 1 | $8 | AliExpress, Amazon |

**Alternative display (easier to source):**
| 7a | Vufine+ Wearable Display | 720p monocular, HDMI input | 1 | $150 | vufine.com |

## Power

| # | Component | Spec | Qty | Est. Price | Source |
|---|-----------|------|-----|-----------|--------|
| 10 | LiPo Battery 3.7V | 5000mAh, 18650 or pouch cell | 1 | $12 | Amazon, Adafruit |
| 11 | USB-C PD Boost Converter | 3.7V → 5V/5A (25W) USB-C PD out | 1 | $10 | Amazon, AliExpress |
| 12 | TP4056 LiPo Charger Module | USB-C in, 1A charge, protection | 1 | $3 | Amazon, AliExpress |
| 13 | Power Switch | SPDT mini slide switch | 1 | $1 | Amazon |

## Frame & Structure

| # | Component | Spec | Qty | Est. Price | Source |
|---|-----------|------|-----|-----------|--------|
| 14 | 3D Printed Frame | PLA/PETG, custom design (see frame/) | 1 | $5 | Self-print or JLCPCB 3D print |
| 15 | M2 Screws + Standoffs Kit | Nylon, assorted lengths | 1 | $8 | Amazon |
| 16 | Silicone Nose Pads | Adhesive-backed | 2 | $3 | Amazon |
| 17 | Elastic Head Strap | Adjustable, 25mm wide | 1 | $4 | Amazon |

## Cables & Misc

| # | Component | Spec | Qty | Est. Price | Source |
|---|-----------|------|-----|-----------|--------|
| 18 | Micro HDMI to HDMI FPC Cable | 20cm flexible flat | 1 | $6 | Amazon, AliExpress |
| 19 | USB-C Power Cable (short) | 15cm, right-angle | 1 | $4 | Amazon |
| 20 | Heat Sink Kit for Pi 5 | Aluminum, adhesive | 1 | $5 | Amazon, Adafruit |
| 21 | Thermal Pad (1mm) | For Hailo-8L | 1 | $3 | Amazon |
| 22 | Heatshrink / Wire | Assorted, for tidy wiring | 1 | $4 | Amazon |

---

## Cost Summary

| Configuration | Total |
|--------------|-------|
| **Budget (Wisecoco MicroOLED)** | **~$290** |
| **Easy (Vufine+ display)** | **~$385** |

## Power Budget

| Component | Typical Draw | Peak Draw |
|-----------|-------------|-----------|
| Raspberry Pi 5 | 5W | 12W |
| Hailo-8L | 1.5W | 2.5W |
| Camera (OV9281) | 0.3W | 0.3W |
| MicroOLED + driver | 0.5W | 0.8W |
| **Total** | **~7.3W** | **~15.6W** |

**Battery life estimate (5000mAh @ 3.7V = 18.5Wh):**
- Typical use: ~2.5 hours
- Light use (lower FPS): ~3.5 hours

## Weight Budget

| Component | Weight |
|-----------|--------|
| Raspberry Pi 5 | 47g |
| Hailo-8L + HAT | 15g |
| Camera module | 5g |
| MicroOLED + prism | 12g |
| Battery (5000mAh pouch) | 95g |
| 3D printed frame | ~40g |
| Cables + misc | ~15g |
| **Total** | **~229g** |

**Note:** Most weight (battery + Pi) should be rear-mounted or temple-mounted
to keep face weight under 80g. See frame design for weight distribution.
