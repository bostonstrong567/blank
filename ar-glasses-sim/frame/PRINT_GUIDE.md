# 3D Print Guide — Anycubic Kobra 2 Pro

## Files to Print

| # | File | Copies | Est. Time | Est. Filament |
|---|------|--------|-----------|---------------|
| 1 | `stl/01_front_frame.stl` | 1 | ~2h 30min | ~35g |
| 2 | `stl/02_right_temple.stl` | 1 | ~3h 00min | ~45g |
| 3 | `stl/03_left_temple.stl` | 1 | ~2h 30min | ~40g |
| 4 | `stl/04_battery_lid.stl` | 1 | ~15min | ~5g |
| 5 | `stl/05_pi_enclosure_lid.stl` | 1 | ~20min | ~8g |
| 6 | `stl/06_nose_pad.stl` | **2** | ~10min each | ~3g each |
| | **TOTAL** | **7 pieces** | **~9 hours** | **~140g** |

## Setup — Zero Config

1. Open **Cura** (or Anycubic Slicer)
2. Import the profile: `kobra2pro_profile.curaprofile`
   - Cura: Preferences → Profiles → Import
3. Load each STL file
4. Hit **Slice** → **Print**

That's it. All settings are pre-configured.

## Material

**PETG (recommended)**
- Nozzle: 235°C
- Bed: 80°C
- Stronger and heat resistant (Pi 5 gets warm)
- Slight flex = won't snap on your face

**PLA (for prototyping only)**
- Nozzle: 205°C
- Bed: 60°C
- If using PLA, change in Cura: material_print_temperature = 205, material_bed_temperature = 60

## Print Settings (already in profile)

- Layer height: 0.2mm
- Walls: 3 lines (1.2mm)
- Infill: 25% gyroid
- Supports: YES (buildplate only)
- Adhesion: Brim (5mm)
- Speed: 80mm/s (walls at 40mm/s for quality)

## Print Order (recommended)

Print the small parts first to test your settings:

1. **06_nose_pad.stl** (x2) — quick test print, 10min each
2. **04_battery_lid.stl** — small, tests snap fit
3. **05_pi_enclosure_lid.stl** — tests vent slot detail
4. **01_front_frame.stl** — the main piece
5. **03_left_temple.stl** — battery arm
6. **02_right_temple.stl** — compute arm (longest print)

## Orientation on Bed

| Part | Orientation | Supports? |
|------|------------|-----------|
| Front frame | Flat, front face down | YES (for display housing overhang) |
| Right temple | On its side, arm flat | YES (for Pi enclosure) |
| Left temple | On its side, arm flat | YES (for battery compartment) |
| Battery lid | Flat, top face up | NO |
| Pi lid | Flat, top face up | NO |
| Nose pads | Flat, pad face down | NO |

## Post-Print

1. **Remove supports** — snap off, clean with flush cutters
2. **Remove brim** — peel off, sand edge if needed
3. **Sand nose bridge** — 220 grit, then 400 grit for comfort
4. **Test fit** — dry-assemble all parts before inserting electronics
5. **Insert threaded inserts** — heat M2 inserts into hinge holes with soldering iron at 220°C
6. **Glue nose pads** — use super glue or silicone adhesive

## Troubleshooting

| Issue | Fix |
|-------|-----|
| Warping on front frame | Increase bed temp to 85°C, add more brim |
| Supports won't remove | Increase support Z distance to 0.25mm |
| Snap tabs too tight | Sand lightly or scale lid 99% in X/Y |
| Snap tabs too loose | Scale lid 101% in X/Y, or add tape |
| Pi doesn't fit | Scale right temple 102% — tolerances may vary |
| Layer separation | Increase nozzle temp by 5°C |
