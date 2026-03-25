#!/usr/bin/env python3
"""AR Glasses Person Detection Simulator — main entry point."""

import os
import sys
import time

import cv2
import numpy as np

from config import (
    CONFIDENCE_THRESHOLDS, DEFAULT_THRESHOLD_IDX,
    RESOLUTIONS, SCREENSHOT_DIR,
)
from detector import Detector
from metrics import CSVLogger, FPSTracker
from renderer import Renderer


def main():
    # State
    res_idx = 0
    thresh_idx = DEFAULT_THRESHOLD_IDX
    persons_only = True

    target_w, target_h = RESOLUTIONS[res_idx]
    confidence = CONFIDENCE_THRESHOLDS[thresh_idx]

    # Initialize components
    print("Loading YOLOv8-nano model (auto-download on first run)...")
    detector = Detector()
    renderer = Renderer()
    fps_tracker = FPSTracker()
    csv_logger = CSVLogger()

    # Open webcam
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("ERROR: Could not open webcam. Check camera connection.")
        sys.exit(1)

    # Try to maximize capture framerate
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, RESOLUTIONS[0][0])
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, RESOLUTIONS[0][1])
    cap.set(cv2.CAP_PROP_FPS, 120)
    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

    os.makedirs(SCREENSHOT_DIR, exist_ok=True)

    print(f"Webcam opened. Resolution: {target_w}x{target_h}")
    print("Controls: Q=Quit R=Resolution T=Threshold S=Screenshot L=Log P=PersonFilter X=Scanlines")

    render_ms = 0.0

    while True:
        t_frame_start = time.perf_counter()

        ret, frame = cap.read()
        if not ret:
            print("ERROR: Failed to read frame from webcam.")
            break

        # Resize to target resolution
        frame = cv2.resize(frame, (target_w, target_h))

        # Detect
        detections = detector.detect(frame, confidence=confidence, persons_only=persons_only)
        inference_ms = detector.inference_ms

        person_count = sum(1 for d in detections if d.class_id == 0)

        # Render HUD
        t_render_start = time.perf_counter()
        resolution_label = f"{target_w}x{target_h}"
        display = renderer.draw(
            frame, detections,
            fps=fps_tracker.fps,
            inference_ms=inference_ms,
            render_ms=render_ms,
            total_ms=(render_ms + inference_ms),
            person_count=person_count,
            resolution_label=resolution_label,
            confidence_threshold=confidence,
            persons_only=persons_only,
            logging_active=csv_logger.active,
        )
        render_ms = (time.perf_counter() - t_render_start) * 1000.0

        total_ms = (time.perf_counter() - t_frame_start) * 1000.0

        fps_tracker.tick()

        # Log
        csv_logger.log(fps_tracker.fps, inference_ms, render_ms, total_ms, person_count)

        # Display
        cv2.imshow("AR Glasses Simulator", display)

        # Handle keyboard
        key = cv2.waitKey(1) & 0xFF

        if key == ord("q"):
            break
        elif key == ord("r"):
            res_idx = (res_idx + 1) % len(RESOLUTIONS)
            target_w, target_h = RESOLUTIONS[res_idx]
            print(f"Resolution: {target_w}x{target_h}")
        elif key == ord("t"):
            thresh_idx = (thresh_idx + 1) % len(CONFIDENCE_THRESHOLDS)
            confidence = CONFIDENCE_THRESHOLDS[thresh_idx]
            print(f"Confidence threshold: {confidence:.1f}")
        elif key == ord("s"):
            ts = time.strftime("%Y%m%d_%H%M%S")
            path = os.path.join(SCREENSHOT_DIR, f"screenshot_{ts}.png")
            cv2.imwrite(path, display)
            print(f"Screenshot saved: {path}")
        elif key == ord("l"):
            csv_logger.toggle()
            print(f"CSV logging: {'ON' if csv_logger.active else 'OFF'}")
        elif key == ord("p"):
            persons_only = not persons_only
            print(f"Filter: {'Person only' if persons_only else 'All COCO classes'}")
        elif key == ord("x"):
            renderer.show_scanlines = not renderer.show_scanlines
            print(f"Scanlines: {'ON' if renderer.show_scanlines else 'OFF'}")

    csv_logger.stop()
    cap.release()
    cv2.destroyAllWindows()
    print("Simulator closed.")


if __name__ == "__main__":
    main()
