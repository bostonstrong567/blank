#!/usr/bin/env python3
"""
MicroOLED display driver for AR glasses.

Outputs the HUD-overlaid frame to either:
  - HDMI (for Wisecoco MicroOLED via HDMI-to-MIPI driver board)
  - Framebuffer (for direct SPI/DPI displays)
  - OpenCV window (for desktop development)

The MicroOLED is 0.39" 1920x1080 but we render at a lower internal
resolution and let the driver board upscale, to maintain 60fps.
"""

import time
from enum import Enum

import numpy as np

try:
    import cv2
    OPENCV_AVAILABLE = True
except ImportError:
    OPENCV_AVAILABLE = False


class DisplayMode(Enum):
    HDMI = "hdmi"           # HDMI output (for MicroOLED driver board)
    FRAMEBUFFER = "fb"      # Direct framebuffer (/dev/fb0)
    OPENCV = "opencv"       # OpenCV window (desktop dev)


class Display:
    """
    Display output manager.

    For the actual AR glasses, this outputs via HDMI to the MicroOLED
    driver board. For development, uses an OpenCV window.
    """

    def __init__(
        self,
        mode: DisplayMode = DisplayMode.OPENCV,
        width: int = 640,
        height: int = 480,
        target_fps: int = 60,
    ):
        self.mode = mode
        self.width = width
        self.height = height
        self.target_fps = target_fps
        self._frame_interval = 1.0 / target_fps
        self._last_frame_time = 0.0
        self._fb = None
        self._window_name = "AR Glasses"

    def start(self):
        """Initialize display output."""
        if self.mode == DisplayMode.OPENCV:
            if not OPENCV_AVAILABLE:
                raise RuntimeError("OpenCV not available for display")
            cv2.namedWindow(self._window_name, cv2.WINDOW_NORMAL)
            cv2.resizeWindow(self._window_name, self.width, self.height)
            print(f"Display: OpenCV window ({self.width}x{self.height})")

        elif self.mode == DisplayMode.FRAMEBUFFER:
            self._init_framebuffer()

        elif self.mode == DisplayMode.HDMI:
            # HDMI output via OpenCV fullscreen on the HDMI-connected display
            if not OPENCV_AVAILABLE:
                raise RuntimeError("OpenCV not available for HDMI display")
            cv2.namedWindow(self._window_name, cv2.WND_PROP_FULLSCREEN)
            cv2.setWindowProperty(
                self._window_name,
                cv2.WND_PROP_FULLSCREEN,
                cv2.WINDOW_FULLSCREEN,
            )
            print(f"Display: HDMI fullscreen ({self.width}x{self.height})")

    def show(self, frame: np.ndarray) -> int:
        """
        Display a frame. Returns key press (for input handling).

        Respects target FPS — will skip frames if called too fast.
        """
        now = time.perf_counter()
        dt = now - self._last_frame_time
        if dt < self._frame_interval:
            # Optionally sleep to save CPU
            remaining = self._frame_interval - dt
            if remaining > 0.001:
                time.sleep(remaining * 0.8)  # sleep 80%, spin the rest

        self._last_frame_time = time.perf_counter()

        if self.mode in (DisplayMode.OPENCV, DisplayMode.HDMI):
            # Resize if needed
            if frame.shape[1] != self.width or frame.shape[0] != self.height:
                frame = cv2.resize(frame, (self.width, self.height))
            cv2.imshow(self._window_name, frame)
            return cv2.waitKey(1) & 0xFF

        elif self.mode == DisplayMode.FRAMEBUFFER:
            self._write_framebuffer(frame)
            return -1

        return -1

    def stop(self):
        """Clean up display resources."""
        if self.mode in (DisplayMode.OPENCV, DisplayMode.HDMI):
            cv2.destroyAllWindows()
        elif self.mode == DisplayMode.FRAMEBUFFER and self._fb:
            self._fb.close()
        print("Display stopped.")

    def _init_framebuffer(self):
        """Initialize Linux framebuffer for direct display output."""
        import struct
        import fcntl

        FB_DEVICE = "/dev/fb0"

        try:
            self._fb = open(FB_DEVICE, "wb")
            print(f"Display: Framebuffer ({FB_DEVICE})")
        except PermissionError:
            raise RuntimeError(
                f"Cannot open {FB_DEVICE}. Run with sudo or add user to 'video' group."
            )

    def _write_framebuffer(self, frame: np.ndarray):
        """Write frame directly to Linux framebuffer."""
        if self._fb is None:
            return

        # Convert BGR to BGRA (32-bit framebuffer)
        if frame.shape[2] == 3:
            alpha = np.full((*frame.shape[:2], 1), 255, dtype=np.uint8)
            frame = np.concatenate([frame, alpha], axis=2)

        # Resize to display resolution
        if frame.shape[1] != self.width or frame.shape[0] != self.height:
            import cv2
            frame = cv2.resize(frame, (self.width, self.height))

        self._fb.seek(0)
        self._fb.write(frame.tobytes())
        self._fb.flush()


class HUDRenderer:
    """
    Renders AR HUD overlay onto frames.

    Native Python/OpenCV implementation matching the web simulator style.
    Optimized for low overhead on RPi 5.
    """

    CYAN = (255, 170, 136)       # BGR
    TEXT_BG = (30, 30, 30)
    WHITE = (255, 255, 255)
    GREEN = (100, 255, 100)
    YELLOW = (80, 230, 255)
    RED = (80, 80, 255)

    def __init__(self):
        self._vignette_cache = {}

    def draw_detection(self, frame, det, bracket_frac=0.2):
        """Draw corner bracket box with label."""
        x1, y1, x2, y2 = det.x1, det.y1, det.x2, det.y2
        w, h = x2 - x1, y2 - y1
        bx = max(int(w * bracket_frac), 10)
        by = max(int(h * bracket_frac), 10)
        c = self.CYAN
        t = 2

        # Corner brackets
        cv2.line(frame, (x1, y1), (x1 + bx, y1), c, t)
        cv2.line(frame, (x1, y1), (x1, y1 + by), c, t)
        cv2.line(frame, (x2, y1), (x2 - bx, y1), c, t)
        cv2.line(frame, (x2, y1), (x2, y1 + by), c, t)
        cv2.line(frame, (x1, y2), (x1 + bx, y2), c, t)
        cv2.line(frame, (x1, y2), (x1, y2 - by), c, t)
        cv2.line(frame, (x2, y2), (x2 - bx, y2), c, t)
        cv2.line(frame, (x2, y2), (x2, y2 - by), c, t)

        # Label
        id_str = f"#{det.track_id}" if det.track_id else ""
        label = f"person {id_str} {det.confidence:.0%}"
        self._draw_label(frame, label, x1, y1 - 8)

    def draw_dashboard(self, frame, stats: dict):
        """Draw performance dashboard overlay."""
        lines = [
            (f"FPS: {stats.get('fps', 0):.0f}", 0.7,
             self.GREEN if stats.get('fps', 0) >= 50 else
             self.YELLOW if stats.get('fps', 0) >= 25 else self.RED),
            (f"Infer:  {stats.get('inference_ms', 0):.1f}ms", 0.45, self.WHITE),
            (f"Render: {stats.get('render_ms', 0):.1f}ms", 0.45, self.WHITE),
            (f"Total:  {stats.get('total_ms', 0):.1f}ms", 0.45, self.WHITE),
            (f"Persons: {stats.get('persons', 0)}", 0.45, self.CYAN),
            (f"Cam: {stats.get('cam_fps', 0):.0f}fps", 0.45, (100, 130, 200)),
        ]

        # Background panel
        overlay = frame.copy()
        cv2.rectangle(overlay, (5, 5), (220, 15 + len(lines) * 22), self.TEXT_BG, -1)
        cv2.addWeighted(overlay, 0.6, frame, 0.4, 0, frame)

        y = 25
        for text, scale, color in lines:
            cv2.putText(frame, text, (12, y), cv2.FONT_HERSHEY_SIMPLEX,
                        scale, color, 1, cv2.LINE_AA)
            y += int(22 * (1.4 if scale > 0.5 else 1.0))

    def apply_vignette(self, frame):
        """Fast vignette using cached mask."""
        h, w = frame.shape[:2]
        key = (w, h)
        if key not in self._vignette_cache:
            x = np.linspace(-1, 1, w)
            y = np.linspace(-1, 1, h)
            xx, yy = np.meshgrid(x, y)
            r = np.sqrt(xx**2 + yy**2)
            mask = np.clip(1.0 - (r - 0.7) * 0.7, 0, 1).astype(np.float32)
            self._vignette_cache[key] = mask

        mask = self._vignette_cache[key]
        for c in range(3):
            frame[:, :, c] = (frame[:, :, c].astype(np.float32) * mask).astype(np.uint8)

    def _draw_label(self, frame, text, x, y):
        font = cv2.FONT_HERSHEY_SIMPLEX
        scale, thickness = 0.45, 1
        (tw, th), _ = cv2.getTextSize(text, font, scale, thickness)
        y = max(y, th + 4)
        overlay = frame.copy()
        cv2.rectangle(overlay, (x-3, y-th-3), (x+tw+3, y+3), self.TEXT_BG, -1)
        cv2.addWeighted(overlay, 0.6, frame, 0.4, 0, frame)
        cv2.putText(frame, text, (x, y), font, scale, self.CYAN, thickness, cv2.LINE_AA)


# Need cv2 for HUDRenderer
import cv2
