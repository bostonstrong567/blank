#!/usr/bin/env python3
"""
Camera capture module for ArduCam OV9281 on Raspberry Pi 5.

Uses libcamera (picamera2) for 120fps global shutter capture.
Falls back to OpenCV VideoCapture if picamera2 is not available.
"""

import time
from dataclasses import dataclass
from threading import Thread, Event
from collections import deque

import numpy as np

try:
    from picamera2 import Picamera2
    PICAMERA_AVAILABLE = True
except ImportError:
    PICAMERA_AVAILABLE = False

try:
    import cv2
    OPENCV_AVAILABLE = True
except ImportError:
    OPENCV_AVAILABLE = False


@dataclass
class FrameConfig:
    """Camera configuration."""
    width: int = 640
    height: int = 480
    fps: int = 120
    format: str = "RGB888"  # or "YUV420" for lower overhead
    buffer_count: int = 2   # double-buffer for low latency
    hflip: bool = False
    vflip: bool = False


class Camera:
    """
    High-performance camera capture with ring buffer.

    Runs capture in a background thread to decouple camera FPS
    from processing FPS. Always provides the latest frame.
    """

    def __init__(self, config: FrameConfig | None = None):
        self.config = config or FrameConfig()
        self._frame_buffer: deque[np.ndarray] = deque(maxlen=2)
        self._running = Event()
        self._thread: Thread | None = None
        self._camera = None
        self._capture_fps = 0.0
        self._frame_count = 0
        self._last_fps_time = time.perf_counter()

    def start(self):
        """Start camera capture in background thread."""
        if PICAMERA_AVAILABLE:
            self._init_picamera()
        elif OPENCV_AVAILABLE:
            self._init_opencv()
        else:
            raise RuntimeError("No camera backend available (need picamera2 or opencv)")

        self._running.set()
        self._thread = Thread(target=self._capture_loop, daemon=True)
        self._thread.start()
        print(f"Camera started: {self.config.width}x{self.config.height} @ {self.config.fps}fps")

    def stop(self):
        """Stop camera capture."""
        self._running.clear()
        if self._thread:
            self._thread.join(timeout=2.0)
        if self._camera:
            if PICAMERA_AVAILABLE and isinstance(self._camera, Picamera2):
                self._camera.stop()
                self._camera.close()
            elif OPENCV_AVAILABLE:
                self._camera.release()
        print("Camera stopped.")

    def get_frame(self) -> np.ndarray | None:
        """Get the latest frame (non-blocking). Returns None if no frame."""
        if len(self._frame_buffer) == 0:
            return None
        return self._frame_buffer[-1]

    @property
    def capture_fps(self) -> float:
        return self._capture_fps

    def _init_picamera(self):
        """Initialize picamera2 for RPi camera."""
        cam = Picamera2()

        # Configure for low-latency capture
        camera_config = cam.create_video_configuration(
            main={
                "size": (self.config.width, self.config.height),
                "format": self.config.format,
            },
            buffer_count=self.config.buffer_count,
            controls={
                "FrameRate": self.config.fps,
                "ExposureTime": min(8000, int(1_000_000 / self.config.fps)),
                # Short exposure for global shutter — reduce motion blur
            },
        )

        if self.config.hflip or self.config.vflip:
            camera_config["transform"] = {
                "hflip": self.config.hflip,
                "vflip": self.config.vflip,
            }

        cam.configure(camera_config)
        cam.start()

        # Warm up
        time.sleep(0.3)
        self._camera = cam
        self._backend = "picamera2"
        print(f"  Backend: picamera2 (libcamera)")

    def _init_opencv(self):
        """Initialize OpenCV VideoCapture fallback."""
        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            raise RuntimeError("Cannot open camera with OpenCV")

        cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.config.width)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.config.height)
        cap.set(cv2.CAP_PROP_FPS, self.config.fps)
        cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

        self._camera = cap
        self._backend = "opencv"
        print(f"  Backend: OpenCV (V4L2)")

    def _capture_loop(self):
        """Background capture loop."""
        fps_frames = 0
        fps_t0 = time.perf_counter()

        while self._running.is_set():
            frame = self._grab_frame()
            if frame is not None:
                self._frame_buffer.append(frame)
                fps_frames += 1

                # Update FPS every second
                now = time.perf_counter()
                if now - fps_t0 >= 1.0:
                    self._capture_fps = fps_frames / (now - fps_t0)
                    fps_frames = 0
                    fps_t0 = now

    def _grab_frame(self) -> np.ndarray | None:
        """Grab a single frame from the camera."""
        if self._backend == "picamera2":
            # picamera2 returns RGB
            frame = self._camera.capture_array("main")
            return frame
        else:
            # OpenCV returns BGR
            ret, frame = self._camera.read()
            return frame if ret else None


class DummyCamera:
    """Generates synthetic test frames for development without hardware."""

    def __init__(self, width: int = 640, height: int = 480):
        self.width = width
        self.height = height
        self._frame_count = 0
        self.capture_fps = 60.0

    def start(self):
        print(f"DummyCamera started: {self.width}x{self.height}")

    def stop(self):
        print("DummyCamera stopped.")

    def get_frame(self) -> np.ndarray:
        """Generate a test pattern frame."""
        frame = np.zeros((self.height, self.width, 3), dtype=np.uint8)

        # Moving gradient bar
        offset = (self._frame_count * 3) % self.width
        for x in range(self.width):
            val = int(((x + offset) % self.width) / self.width * 255)
            frame[:, x, 0] = val
            frame[:, x, 1] = 128
            frame[:, x, 2] = 255 - val

        # Simulated person rectangle (moves)
        cx = int(self.width / 2 + 100 * np.sin(self._frame_count * 0.05))
        cy = self.height // 2
        pw, ph = 80, 200
        frame[cy-ph//2:cy+ph//2, cx-pw//2:cx+pw//2] = [200, 180, 160]

        self._frame_count += 1
        return frame
