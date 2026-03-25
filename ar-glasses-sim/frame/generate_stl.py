#!/usr/bin/env python3
"""
Generate print-ready STL files for the AR glasses frame.
All parts oriented for optimal printing on Anycubic Kobra 2 Pro (220x220x250mm).
No supports needed unless noted.
"""

import numpy as np
from stl import mesh
import os

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "stl")
os.makedirs(OUTPUT_DIR, exist_ok=True)


def box(w, h, d, offset=(0, 0, 0)):
    """Create a solid box mesh."""
    ox, oy, oz = offset
    vertices = np.array([
        [ox, oy, oz], [ox+w, oy, oz], [ox+w, oy+h, oz], [ox, oy+h, oz],
        [ox, oy, oz+d], [ox+w, oy, oz+d], [ox+w, oy+h, oz+d], [ox, oy+h, oz+d],
    ])
    faces = np.array([
        [0,3,1],[1,3,2],  # front
        [4,5,7],[5,6,7],  # back
        [0,1,4],[1,5,4],  # bottom
        [2,3,7],[2,7,6],  # top
        [0,4,3],[3,4,7],  # left
        [1,2,5],[2,6,5],  # right
    ])
    return vertices, faces


def hollow_box(outer_w, outer_h, outer_d, wall, offset=(0, 0, 0)):
    """Create a hollow box (outer - inner)."""
    ox, oy, oz = offset
    # We'll build it as 6 walls
    parts = []
    # Bottom
    parts.append(box(outer_w, wall, outer_d, (ox, oy, oz)))
    # Top
    parts.append(box(outer_w, wall, outer_d, (ox, oy+outer_h-wall, oz)))
    # Left
    parts.append(box(wall, outer_h, outer_d, (ox, oy, oz)))
    # Right
    parts.append(box(wall, outer_h, outer_d, (ox+outer_w-wall, oy, oz)))
    # Front
    parts.append(box(outer_w, outer_h, wall, (ox, oy, oz)))
    # Back
    parts.append(box(outer_w, outer_h, wall, (ox, oy, oz+outer_d-wall)))
    return parts


def combine_parts(parts_list):
    """Combine multiple (vertices, faces) into one mesh."""
    all_verts = []
    all_faces = []
    offset = 0
    for verts, faces in parts_list:
        all_verts.append(verts)
        all_faces.append(faces + offset)
        offset += len(verts)
    return np.vstack(all_verts), np.vstack(all_faces)


def make_mesh(vertices, faces):
    """Create an STL mesh from vertices and faces."""
    m = mesh.Mesh(np.zeros(len(faces), dtype=mesh.Mesh.dtype))
    for i, f in enumerate(faces):
        for j in range(3):
            m.vectors[i][j] = vertices[f[j]]
    return m


# =============================================
# PART 1: Front Frame
# =============================================
def generate_front_frame():
    """
    Front frame piece — the main glasses front.
    Print flat on bed, no supports needed.
    """
    parts = []
    w = 155  # total width
    h = 45   # height
    d = 20   # depth
    wall = 2.5
    bridge = 18  # nose bridge gap
    bridge_start = w/2 - bridge/2

    # Main outer frame - left section
    left_w = bridge_start
    parts.extend(hollow_box(left_w, h, d, wall, (0, 0, 0)))

    # Main outer frame - right section
    right_start = w/2 + bridge/2
    right_w = w - right_start
    parts.extend(hollow_box(right_w, h, d, wall, (right_start, 0, 0)))

    # Top bridge connector
    parts.append(box(bridge + 10, wall * 2, d, (bridge_start - 5, h - wall * 2, 0)))

    # Nose bridge supports (angled pads)
    for side in [-1, 1]:
        nx = w/2 + side * (bridge/2 + 2)
        parts.append(box(10, 15, 4, (nx - 5, 0, d/2 - 2)))

    # Camera mount tab (front, right side above eye)
    cam_x = w/2 + 20
    parts.append(box(14, 14, 10, (cam_x, h, 0)))
    # Camera hole (represented as a thin inset)
    parts.append(box(10, 10, 2, (cam_x + 2, h + 2, -2)))

    # Display housing (right eye, extends back)
    disp_x = right_start + 5
    parts.append(box(30, 30, 25, (disp_x, 8, d)))
    # Hollow out display area
    parts.extend(hollow_box(30, 30, 25, wall, (disp_x, 8, d)))

    # Prism mount (angled slot inside display housing)
    parts.append(box(28, 3, 20, (disp_x + 1, 8 + 1, d + 1)))

    # Temple hinge tabs (both sides)
    for x in [0, w - 12]:
        parts.append(box(12, 12, 8, (x, h/2 - 6, d)))
        # Screw hole marker (small cylinder approximated as small box)
        parts.append(box(3, 3, 10, (x + 4.5, h/2 - 1.5, d - 1)))

    # Strap mount loops
    for x in [-5, w - 7]:
        parts.append(box(12, 12, 8, (max(0, x), h/2 - 6, d + 8)))

    verts, faces = combine_parts(parts)
    m = make_mesh(verts, faces)
    path = os.path.join(OUTPUT_DIR, "01_front_frame.stl")
    m.save(path)
    print(f"  Saved: {path}")
    return path


# =============================================
# PART 2: Right Temple (Compute Module Mount)
# =============================================
def generate_right_temple():
    """
    Right temple arm with RPi 5 + Hailo-8L enclosure.
    Print on its side for strength along the arm axis.
    """
    parts = []

    arm_len = 130
    arm_w = 12
    arm_t = 5

    # Main arm
    parts.append(box(arm_len, arm_w, arm_t, (0, 0, 0)))

    # Ear hook (angled end piece)
    parts.append(box(25, arm_w, arm_t, (arm_len, 0, 0)))
    parts.append(box(15, arm_w, arm_t + 5, (arm_len + 10, 0, -5)))

    # Hinge tab (connects to front frame)
    parts.append(box(15, 14, 8, (-15, arm_w/2 - 7, -1.5)))

    # RPi 5 + Hailo enclosure (mounted on top of arm)
    pi_l = 85
    pi_w = 56
    pi_t = 22  # Pi + HAT + heatsink
    pi_x = 25  # offset along arm
    pi_y = arm_w/2 - pi_w/2

    # Enclosure walls
    parts.extend(hollow_box(pi_l, pi_w, pi_t, 2.5, (pi_x, pi_y, arm_t)))

    # Ventilation slots (top) - represented as thin channels
    for i in range(5):
        slot_x = pi_x + 8 + i * 16
        parts.append(box(2, pi_w - 10, 1, (slot_x, pi_y + 5, arm_t + pi_t - 1)))

    # SD card slot access (side)
    parts.append(box(15, 3, 5, (pi_x + pi_l - 15, pi_y - 3, arm_t + 5)))

    # USB-C power access (bottom/side)
    parts.append(box(12, 3, 8, (pi_x + pi_l/2 - 6, pi_y - 3, arm_t + 2)))

    # Cable routing channel (along arm to front)
    parts.append(box(pi_x, 8, 4, (0, arm_w/2 - 4, arm_t)))

    # Mounting screw posts (4 corners for Pi)
    for dx, dy in [(3, 3), (3, 49), (61, 3), (61, 49)]:
        parts.append(box(4, 4, pi_t, (pi_x + dx, pi_y + dy, arm_t)))

    verts, faces = combine_parts(parts)
    m = make_mesh(verts, faces)
    path = os.path.join(OUTPUT_DIR, "02_right_temple.stl")
    m.save(path)
    print(f"  Saved: {path}")
    return path


# =============================================
# PART 3: Left Temple (Battery Mount)
# =============================================
def generate_left_temple():
    """
    Left temple arm with battery + charger enclosure.
    Print on its side for strength.
    """
    parts = []

    arm_len = 130
    arm_w = 12
    arm_t = 5

    # Main arm
    parts.append(box(arm_len, arm_w, arm_t, (0, 0, 0)))

    # Ear hook
    parts.append(box(25, arm_w, arm_t, (arm_len, 0, 0)))
    parts.append(box(15, arm_w, arm_t + 5, (arm_len + 10, 0, -5)))

    # Hinge tab
    parts.append(box(15, 14, 8, (-15, arm_w/2 - 7, -1.5)))

    # Battery compartment
    batt_l = 72
    batt_w = 22
    batt_t = 20
    batt_x = 20
    batt_y = arm_w/2 - batt_w/2

    # Enclosure walls
    parts.extend(hollow_box(batt_l, batt_w, batt_t, 2, (batt_x, batt_y, arm_t)))

    # Charger module pocket (end of battery compartment)
    chrg_l = 20
    parts.extend(hollow_box(chrg_l, batt_w, 12, 2,
                            (batt_x + batt_l, batt_y, arm_t)))

    # USB-C charge port access
    parts.append(box(12, 3, 8, (batt_x + batt_l + 4, batt_y - 3, arm_t + 2)))

    # Power switch cutout marker
    parts.append(box(8, 3, 5, (batt_x + batt_l - 10, batt_y + batt_w, arm_t + 5)))

    # Wire routing channel to front
    parts.append(box(batt_x, 6, 4, (0, arm_w/2 - 3, arm_t)))

    # Lid snap ridges
    for x_off in [5, batt_l - 8]:
        parts.append(box(3, batt_w + 4, 2, (batt_x + x_off, batt_y - 2, arm_t + batt_t)))

    verts, faces = combine_parts(parts)
    m = make_mesh(verts, faces)
    path = os.path.join(OUTPUT_DIR, "03_left_temple.stl")
    m.save(path)
    print(f"  Saved: {path}")
    return path


# =============================================
# PART 4: Battery Lid
# =============================================
def generate_battery_lid():
    """
    Snap-on lid for the battery compartment.
    Print flat, no supports.
    """
    parts = []
    lid_l = 72
    lid_w = 22
    lid_t = 2

    # Lid plate
    parts.append(box(lid_l, lid_w, lid_t, (0, 0, 0)))

    # Snap tabs
    parts.append(box(3, lid_w - 2, 3, (5, 1, -3)))
    parts.append(box(3, lid_w - 2, 3, (lid_l - 8, 1, -3)))

    # Grip ridge
    parts.append(box(20, 3, 1, (lid_l/2 - 10, lid_w/2 - 1.5, lid_t)))

    verts, faces = combine_parts(parts)
    m = make_mesh(verts, faces)
    path = os.path.join(OUTPUT_DIR, "04_battery_lid.stl")
    m.save(path)
    print(f"  Saved: {path}")
    return path


# =============================================
# PART 5: Pi Enclosure Lid
# =============================================
def generate_pi_lid():
    """
    Lid for the RPi enclosure with ventilation slots.
    Print flat, no supports.
    """
    parts = []
    lid_l = 85
    lid_w = 56
    lid_t = 2

    # Main lid
    parts.append(box(lid_l, lid_w, lid_t, (0, 0, 0)))

    # Vent slots (leave gaps — just make the solid parts between them)
    # 5 vent slots, 2mm wide, spaced 16mm apart
    # We represent the solid sections between vents
    for i in range(6):
        section_x = i * 14
        section_w = 12
        if section_x + section_w > lid_l:
            section_w = lid_l - section_x
        parts.append(box(section_w, lid_w - 10, 1, (section_x, 5, lid_t)))

    # Snap tabs (4 corners)
    for x in [3, lid_l - 7]:
        for y in [3, lid_w - 7]:
            parts.append(box(4, 4, 3, (x, y, -3)))

    verts, faces = combine_parts(parts)
    m = make_mesh(verts, faces)
    path = os.path.join(OUTPUT_DIR, "05_pi_enclosure_lid.stl")
    m.save(path)
    print(f"  Saved: {path}")
    return path


# =============================================
# PART 6: Nose Pads (x2)
# =============================================
def generate_nose_pads():
    """
    Nose pad with silicone insert recess.
    Print 2 copies. No supports.
    """
    parts = []

    pad_w = 12
    pad_l = 18
    pad_t = 4

    # Main pad body
    parts.append(box(pad_w, pad_l, pad_t, (0, 0, 0)))

    # Silicone recess (thinner center)
    parts.append(box(pad_w - 4, pad_l - 4, 1.5, (2, 2, pad_t - 1.5)))

    # Mounting tab
    parts.append(box(6, 6, 6, (pad_w/2 - 3, pad_l, 0)))

    verts, faces = combine_parts(parts)
    m = make_mesh(verts, faces)
    path = os.path.join(OUTPUT_DIR, "06_nose_pad.stl")
    m.save(path)
    print(f"  Saved: {path} (print x2)")
    return path


# =============================================
# Generate All
# =============================================
if __name__ == "__main__":
    print("Generating AR Glasses STL files...")
    print(f"Output: {OUTPUT_DIR}/")
    print()

    generate_front_frame()
    generate_right_temple()
    generate_left_temple()
    generate_battery_lid()
    generate_pi_lid()
    generate_nose_pads()

    print()
    print("All STL files generated!")
    print()
    print("Print order:")
    print("  1. 01_front_frame.stl     — main glasses front")
    print("  2. 02_right_temple.stl    — right arm (Pi mount)")
    print("  3. 03_left_temple.stl     — left arm (battery)")
    print("  4. 04_battery_lid.stl     — battery compartment lid")
    print("  5. 05_pi_enclosure_lid.stl — Pi enclosure lid")
    print("  6. 06_nose_pad.stl        — nose pad (print 2x)")
    print()
    print("Total: 7 pieces (6 unique parts)")
