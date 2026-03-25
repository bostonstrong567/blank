#!/usr/bin/env python3
"""
AR Glasses — Main embedded pipeline for Raspberry Pi 5 + Hailo-8L.

Pipeline:
  Camera (120fps) → Hailo-8L (YOLOv8n, ~4ms) → HUD Overlay → MicroOLED

Usage:
  # Development (desktop, webcam, OpenCV window)
  python main_embedded.py --mode dev

  # Production (RPi 5, ArduCam, HDMI to MicroOLED)
  python main_embedded.py --mode prod

  # Benchmark (no display, measure raw throughput)
  python main_embedded.py --mode bench
"""

import argparse
import signal
import sys
import time
from collections import deque

import numpy as np

# Conditional imports — graceful fallback
try:
    import cv2
except ImportError:
    print("ERROR: OpenCV required. pip install opencv-python")
    sys.exit(1)


def parse_args():
    parser = argparse.ArgumentParser(description="AR Glasses Embedded Pipeline")
    parser.add_argument(
        "--mode", choices=["dev", "prod", "bench"], default="dev",
        help="dev=desktop webcam, prod=RPi+Hailo+MicroOLED, bench=no display"
    )
    parser.add_argument("--width", type=int, default=640)
    parser.add_argument("--height", type=int, default=480)
    parser.add_argument("--fps", type=int, default=120)
    parser.add_argument("--conf", type=float, default=0.5)
    parser.add_argument("--model", type=str, default="yolov8n_hailo8l.hef")
    parser.add_argument("--no-vignette", action="store_true")
    parser.add_argument("--csv-log", type=str, default=None, help="CSV log file path")
    return parser.parse_args()


def main():
    args = parse_args()

    # === Initialize components ===
    print("=" * 50)
    print("  AR GLASSES — EMBEDDED PIPELINE")
    print("=" * 50)
    print(f"  Mode:       {args.mode}")
    print(f"  Resolution: {args.width}x{args.height}")
    print(f"  Target FPS: {args.fps}")
    print(f"  Confidence: {args.conf}")
    print()

    # Camera
    from camera import Camera, DummyCamera, FrameConfig

    if args.mode == "bench":
        camera = DummyCamera(args.width, args.height)
    else:
        camera = Camera(FrameConfig(
            width=args.width,
            height=args.height,
            fps=args.fps,
        ))

    # Detector
    detector = None
    use_hailo = False

    try:
        from hailo_detector import HailoDetector
        detector = HailoDetector(model_path=args.model, conf=args.conf)
        use_hailo = True
        print("  Detector: Hailo-8L")
    except (RuntimeError, ImportError) as e:
        print(f"  Hailo not available: {e}")
        try:
            from ultralytics import YOLO
            detector = YOLO("yolov8n.pt")
            print("  Detector: YOLOv8n (CPU/GPU fallback)")
        except ImportError:
            print("  WARNING: No detector available. Running display-only.")

    # Display
    from display import Display, DisplayMode, HUDRenderer

    if args.mode == "bench":
        display = None
    elif args.mode == "prod":
        display = Display(
            mode=DisplayMode.HDMI,
            width=args.width,
            height=args.height,
            target_fps=60,
        )
    else:
        display = Display(
            mode=DisplayMode.OPENCV,
            width=args.width,
            height=args.height,
            target_fps=60,
        )

    hud = HUDRenderer()

    # CSV logger
    csv_file = None
    csv_writer = None
    if args.csv_log:
        import csv
        csv_file = open(args.csv_log, "w", newline="")
        csv_writer = csv.writer(csv_file)
        csv_writer.writerow(["timestamp", "fps", "inference_ms", "render_ms", "total_ms", "persons"])

    # === Start ===
    camera.start()
    if display:
        display.start()

    # FPS tracking
    frame_times = deque(maxlen=120)
    running = True

    def signal_handler(sig, frame):
        nonlocal running
        running = False
    signal.signal(signal.SIGINT, signal_handler)

    print("\nPipeline running. Press Ctrl+C or 'q' to stop.\n")

    # === Main loop ===
    while running:
        t_start = time.perf_counter()

        # 1. Grab frame
        frame = camera.get_frame()
        if frame is None:
            time.sleep(0.001)
            continue

        # Ensure BGR for OpenCV
        if len(frame.shape) == 3 and frame.shape[2] == 3:
            # picamera2 gives RGB, OpenCV expects BGR
            if not hasattr(camera, '_backend') or getattr(camera, '_backend', '') == 'picamera2':
                frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

        # 2. Detect
        inference_ms = 0.0
        detections = []

        if detector is not None:
            if use_hailo:
                result = detector.detect(frame)
                detections = result.detections
                inference_ms = result.inference_ms
            else:
                # Ultralytics fallback
                t_inf = time.perf_counter()
                results = detector.track(
                    frame, conf=args.conf, classes=[0],
                    persist=True, verbose=False,
                )
                inference_ms = (time.perf_counter() - t_inf) * 1000

                for r in results:
                    if r.boxes is None:
                        continue
                    for i in range(len(r.boxes)):
                        xyxy = r.boxes.xyxy[i].cpu().numpy().astype(int)
                        from hailo_detector import Detection
                        det = Detection(
                            x1=xyxy[0], y1=xyxy[1], x2=xyxy[2], y2=xyxy[3],
                            confidence=float(r.boxes.conf[i].cpu().numpy()),
                            class_id=0,
                            track_id=int(r.boxes.id[i].cpu().numpy()) if r.boxes.id is not None else None,
                        )
                        detections.append(det)

        person_count = len(detections)

        # 3. Render HUD
        t_render = time.perf_counter()

        for det in detections:
            hud.draw_detection(frame, det)

        if not args.no_vignette:
            hud.apply_vignette(frame)

        # Calculate FPS
        now = time.perf_counter()
        frame_times.append(now)
        while frame_times[0] < now - 1.0:
            frame_times.popleft()
        fps = len(frame_times)

        render_ms = (time.perf_counter() - t_render) * 1000
        total_ms = (time.perf_counter() - t_start) * 1000

        # Dashboard
        hud.draw_dashboard(frame, {
            "fps": fps,
            "inference_ms": inference_ms,
            "render_ms": render_ms,
            "total_ms": total_ms,
            "persons": person_count,
            "cam_fps": getattr(camera, 'capture_fps', 0),
        })

        # 4. Display
        if display:
            key = display.show(frame)
            if key == ord("q"):
                running = False
        else:
            # Bench mode — print stats periodically
            if int(now) != int(now - total_ms / 1000):
                print(f"FPS: {fps:3d} | Infer: {inference_ms:5.1f}ms | "
                      f"Render: {render_ms:5.1f}ms | Total: {total_ms:5.1f}ms | "
                      f"Persons: {person_count}")

        # 5. Log
        if csv_writer:
            from datetime import datetime
            csv_writer.writerow([
                datetime.now().isoformat(timespec="milliseconds"),
                f"{fps:.1f}", f"{inference_ms:.1f}", f"{render_ms:.1f}",
                f"{total_ms:.1f}", person_count,
            ])

    # === Cleanup ===
    print("\nShutting down...")
    camera.stop()
    if display:
        display.stop()
    if csv_file:
        csv_file.close()
    print("Done.")


if __name__ == "__main__":
    main()
