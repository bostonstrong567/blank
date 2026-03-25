// AR Glasses Frame — OpenSCAD
// Designed for: RPi 5 + Hailo-8L + ArduCam OV9281 + MicroOLED 0.39"
// Print in PETG for heat resistance, PLA for prototyping
// All dimensions in mm

/* === PARAMETERS === */

// Frame dimensions
frame_width = 155;          // total width across face
frame_height = 45;          // lens area height
frame_depth = 20;           // front-to-back thickness
bridge_width = 18;          // nose bridge gap
corner_radius = 5;

// Temple arms (sides)
temple_length = 130;        // arm length
temple_width = 12;
temple_thickness = 5;
temple_angle = 8;           // slight downward angle

// Camera housing (right side, front-facing)
cam_width = 10;
cam_height = 10;
cam_depth = 8;
cam_offset_x = 25;         // offset from center

// Display housing (right eye, internal)
display_width = 15;
display_height = 10;
display_depth = 25;

// Prism housing
prism_size = 26;            // 25mm prism + tolerance
prism_depth = 15;

// RPi mount (on right temple, rear-weighted)
pi_width = 56;              // Pi 5 + HAT width
pi_length = 85;
pi_thickness = 20;          // Pi + HAT + heatsink stack

// Battery compartment (left temple, rear)
batt_width = 20;
batt_length = 70;
batt_thickness = 18;

// Nose pads
nose_pad_width = 12;
nose_pad_length = 18;
nose_pad_angle = 15;

// Ventilation
vent_slot_width = 2;
vent_slot_length = 15;
vent_count = 4;

// Wall thickness
wall = 2;

/* === MODULES === */

// Rounded box helper
module rounded_box(w, h, d, r) {
    hull() {
        for (x = [r, w-r]) for (y = [r, h-r]) for (z = [r, d-r])
            translate([x, y, z]) sphere(r, $fn=20);
    }
}

// Main front frame
module front_frame() {
    difference() {
        // Outer shell
        rounded_box(frame_width, frame_height, frame_depth, corner_radius);

        // Right eye cutout (display eye)
        translate([frame_width/2 + bridge_width/2 + 5, wall + 3, -1])
            rounded_box(55, frame_height - wall*2 - 6, frame_depth + 2, 3);

        // Left eye cutout (see-through)
        translate([5, wall + 3, -1])
            rounded_box(frame_width/2 - bridge_width/2 - 10, frame_height - wall*2 - 6, frame_depth + 2, 3);

        // Bridge cutout (nose)
        translate([frame_width/2 - bridge_width/2, -1, -1])
            cube([bridge_width, frame_height/2, frame_depth + 2]);
    }
}

// Camera housing (front-mounted, above right eye)
module camera_housing() {
    translate([frame_width/2 + cam_offset_x, frame_height - 2, 0]) {
        difference() {
            rounded_box(cam_width + wall*2, cam_height + wall*2, cam_depth + wall, 2);
            // Camera module cavity
            translate([wall, wall, wall])
                cube([cam_width, cam_height, cam_depth + 1]);
            // Lens hole (front)
            translate([wall + cam_width/2, wall + cam_height/2, -1])
                cylinder(d=6, h=wall + 2, $fn=30);
            // Cable slot (bottom)
            translate([wall + 2, -1, wall])
                cube([cam_width - 4, wall + 2, 3]);
        }
    }
}

// Display + prism housing (right eye area)
module display_housing() {
    translate([frame_width/2 + bridge_width/2 + 8, 5, frame_depth - 2]) {
        difference() {
            rounded_box(prism_size + wall*2, prism_size + wall*2, prism_depth + display_depth, 2);
            // Prism cavity
            translate([wall, wall, wall])
                cube([prism_size, prism_size, prism_depth]);
            // Display module cavity (behind prism)
            translate([wall + 3, wall + 5, prism_depth])
                cube([display_width, display_height, display_depth + 1]);
            // Light path opening (bottom, toward eye)
            translate([wall + prism_size/2, -1, wall + prism_depth/2])
                rotate([-90, 0, 0])
                cylinder(d=20, h=wall + 2, $fn=40);
            // Cable routing slot
            translate([prism_size + wall, wall + 5, wall])
                cube([wall + 1, 8, 4]);
        }
    }
}

// Temple arm with device mount
module temple_arm(side="right") {
    mirror_x = (side == "left") ? 1 : 0;

    mirror([mirror_x, 0, 0]) {
        // Hinge area
        translate([frame_width - wall, frame_height/2 - temple_width/2, frame_depth/2 - temple_thickness/2]) {
            rotate([0, 0, 0]) {
                // Main arm
                difference() {
                    union() {
                        // Arm body
                        cube([temple_length, temple_width, temple_thickness]);

                        // Ear hook (curved end)
                        translate([temple_length - 10, 0, 0])
                            rotate([0, 0, -20])
                            cube([25, temple_width, temple_thickness]);
                    }

                    // Weight reduction slots
                    for (i = [0:3]) {
                        translate([20 + i*22, wall, -1])
                            rounded_box(15, temple_width - wall*2, temple_thickness + 2, 1);
                    }
                }

                // Device mount area (thicker section for Pi or battery)
                if (side == "right") {
                    // RPi + Hailo mount
                    translate([30, -pi_width/2 + temple_width/2, temple_thickness]) {
                        difference() {
                            rounded_box(pi_length, pi_width, pi_thickness, 3);
                            // Internal cavity
                            translate([wall, wall, wall])
                                cube([pi_length - wall*2, pi_width - wall*2, pi_thickness]);
                            // Ventilation slots
                            for (i = [0:vent_count-1]) {
                                translate([10 + i*(vent_slot_length + 5), -1, pi_thickness/2])
                                    cube([vent_slot_width, pi_width + 2, pi_thickness/2 + 1]);
                            }
                            // Cable pass-through (front)
                            translate([-1, pi_width/2 - 5, wall])
                                cube([wall + 2, 10, 8]);
                            // USB-C access (bottom)
                            translate([pi_length/2 - 6, -1, -1])
                                cube([12, wall + 2, pi_thickness + 2]);
                        }
                    }
                }
            }
        }
    }
}

// Battery compartment (left temple)
module battery_mount() {
    translate([0, frame_height/2 - temple_width/2, frame_depth/2 - temple_thickness/2]) {
        translate([-(30 + batt_length), -batt_width/2 + temple_width/2, temple_thickness]) {
            difference() {
                rounded_box(batt_length, batt_width, batt_thickness, 3);
                // Battery cavity
                translate([wall, wall, wall])
                    cube([batt_length - wall*2, batt_width - wall*2, batt_thickness]);
                // Wire pass-through
                translate([-1, batt_width/2 - 3, wall])
                    cube([wall + 2, 6, 6]);
                // Lid snap slots
                translate([5, -1, batt_thickness - 2])
                    cube([3, batt_width + 2, 3]);
                translate([batt_length - 8, -1, batt_thickness - 2])
                    cube([3, batt_width + 2, 3]);
            }
        }
    }
}

// Nose pads
module nose_pads() {
    for (side = [-1, 1]) {
        translate([frame_width/2 + side * (bridge_width/2 + 2), 0, frame_depth/2]) {
            rotate([nose_pad_angle * side, 0, 0]) {
                difference() {
                    rounded_box(nose_pad_width, nose_pad_length, 3, 1);
                    // Silicone pad recess
                    translate([1.5, 1.5, 1.5])
                        rounded_box(nose_pad_width - 3, nose_pad_length - 3, 2, 0.5);
                }
            }
        }
    }
}

// Strap mount points
module strap_mounts() {
    for (x = [-5, frame_width - 7]) {
        translate([x, frame_height/2 - 6, frame_depth - 3]) {
            difference() {
                cube([12, 12, 8]);
                // Strap slot
                translate([3, -1, 2])
                    cube([6, 14, 4]);
            }
        }
    }
}

/* === ASSEMBLY === */

module full_assembly() {
    color("#333333") front_frame();
    color("#444444") camera_housing();
    color("#2a2a3e") display_housing();
    color("#333333") temple_arm("right");
    color("#333333") temple_arm("left");
    color("#333333") battery_mount();
    color("#555555") nose_pads();
    color("#444444") strap_mounts();
}

// Render
full_assembly();

// === PRINT NOTES ===
// Print settings:
//   - Material: PETG (heat resistant) or PLA (prototype)
//   - Layer height: 0.2mm
//   - Infill: 20-30%
//   - Supports: YES (for prism housing overhang)
//   - Orientation: front frame flat on bed, temples separate
//
// Post-print:
//   - Sand nose bridge for comfort
//   - Insert M2 threaded inserts at hinge points
//   - Glue silicone nose pads into recesses
//   - Thread elastic strap through strap mounts
