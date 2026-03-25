"""HUD overlay drawing — AR heads-up display style."""

import cv2
import numpy as np

from config import (
    BRACKET_FRACTION, BRACKET_MIN_LEN, BRACKET_THICKNESS,
    CYAN, CYAN_DIM, GREEN, HUD_FONT_SCALE, HUD_LINE_HEIGHT,
    HUD_PADDING, HUD_THICKNESS, RED, SCANLINE_ALPHA, SCANLINE_SPACING,
    TEXT_BG, VIGNETTE_STRENGTH, WHITE, YELLOW,
)
from detector import Detection


class Renderer:
    def __init__(self):
        self._vignette_cache: dict[tuple[int, int], np.ndarray] = {}
        self._scanline_cache: dict[tuple[int, int], np.ndarray] = {}
        self.show_scanlines = False

    def draw(
        self,
        frame: np.ndarray,
        detections: list[Detection],
        fps: float,
        inference_ms: float,
        render_ms: float,
        total_ms: float,
        person_count: int,
        resolution_label: str,
        confidence_threshold: float,
        persons_only: bool,
        logging_active: bool,
    ) -> np.ndarray:
        """Draw full HUD overlay on frame and return it."""
        out = frame.copy()

        # Vignette
        self._apply_vignette(out)

        # Scanlines
        if self.show_scanlines:
            self._apply_scanlines(out)

        # Bounding boxes
        for det in detections:
            self._draw_bracket_box(out, det)

        # Dashboard
        self._draw_dashboard(
            out, fps, inference_ms, render_ms, total_ms,
            person_count, resolution_label, confidence_threshold,
            persons_only, logging_active,
        )

        # Controls help (bottom)
        self._draw_controls(out)

        return out

    def _draw_bracket_box(self, frame: np.ndarray, det: Detection):
        """Draw corner-bracket bounding box with label."""
        x1, y1, x2, y2 = det.x1, det.y1, det.x2, det.y2
        w, h = x2 - x1, y2 - y1
        blen_x = max(int(w * BRACKET_FRACTION), BRACKET_MIN_LEN)
        blen_y = max(int(h * BRACKET_FRACTION), BRACKET_MIN_LEN)
        color = CYAN
        t = BRACKET_THICKNESS

        # Top-left
        cv2.line(frame, (x1, y1), (x1 + blen_x, y1), color, t)
        cv2.line(frame, (x1, y1), (x1, y1 + blen_y), color, t)
        # Top-right
        cv2.line(frame, (x2, y1), (x2 - blen_x, y1), color, t)
        cv2.line(frame, (x2, y1), (x2, y1 + blen_y), color, t)
        # Bottom-left
        cv2.line(frame, (x1, y2), (x1 + blen_x, y2), color, t)
        cv2.line(frame, (x1, y2), (x1, y2 - blen_y), color, t)
        # Bottom-right
        cv2.line(frame, (x2, y2), (x2 - blen_x, y2), color, t)
        cv2.line(frame, (x2, y2), (x2, y2 - blen_y), color, t)

        # Label
        id_str = f"#{det.track_id}" if det.track_id is not None else ""
        label = f"{det.class_name} {id_str} {det.confidence:.0%}"
        self._draw_label(frame, label, x1, y1 - 8, color)

    def _draw_label(self, frame: np.ndarray, text: str, x: int, y: int, color):
        """Draw text with semi-transparent background."""
        font = cv2.FONT_HERSHEY_SIMPLEX
        scale = HUD_FONT_SCALE
        thickness = HUD_THICKNESS
        (tw, th), baseline = cv2.getTextSize(text, font, scale, thickness)
        pad = 4
        y = max(y, th + pad)

        # Background
        overlay = frame.copy()
        cv2.rectangle(
            overlay,
            (x - pad, y - th - pad),
            (x + tw + pad, y + baseline + pad),
            TEXT_BG, -1,
        )
        cv2.addWeighted(overlay, 0.6, frame, 0.4, 0, frame)

        cv2.putText(frame, text, (x, y), font, scale, color, thickness, cv2.LINE_AA)

    def _draw_dashboard(
        self, frame, fps, inference_ms, render_ms, total_ms,
        person_count, resolution_label, confidence_threshold,
        persons_only, logging_active,
    ):
        """Draw performance dashboard in top-left."""
        h, w = frame.shape[:2]
        font = cv2.FONT_HERSHEY_SIMPLEX

        lines = [
            (f"FPS: {fps:.1f}", 0.8, GREEN if fps >= 20 else YELLOW if fps >= 10 else RED),
            (f"Inference: {inference_ms:.1f} ms", HUD_FONT_SCALE, WHITE),
            (f"Render:    {render_ms:.1f} ms", HUD_FONT_SCALE, WHITE),
            (f"Total:     {total_ms:.1f} ms", HUD_FONT_SCALE, WHITE),
            (f"Persons:   {person_count}", HUD_FONT_SCALE, CYAN),
            (f"Res:       {resolution_label}", HUD_FONT_SCALE, CYAN_DIM),
            (f"Conf:      {confidence_threshold:.1f}", HUD_FONT_SCALE, CYAN_DIM),
            (f"Filter:    {'Person' if persons_only else 'All COCO'}", HUD_FONT_SCALE, CYAN_DIM),
        ]
        if logging_active:
            lines.append(("[LOG]", HUD_FONT_SCALE, RED))

        # Background panel
        panel_w = 260
        panel_h = 20 + len(lines) * HUD_LINE_HEIGHT + 10
        overlay = frame.copy()
        cv2.rectangle(overlay, (5, 5), (5 + panel_w, 5 + panel_h), TEXT_BG, -1)
        cv2.addWeighted(overlay, 0.55, frame, 0.45, 0, frame)

        y = 28
        for text, scale, color in lines:
            cv2.putText(frame, text, (15, y), font, scale, color, HUD_THICKNESS, cv2.LINE_AA)
            y += int(HUD_LINE_HEIGHT * (1.6 if scale > HUD_FONT_SCALE else 1.0))

    def _draw_controls(self, frame: np.ndarray):
        """Draw controls help at bottom of frame."""
        h, w = frame.shape[:2]
        text = "Q:Quit  R:Resolution  T:Threshold  S:Screenshot  L:Log  P:Person/All  X:Scanlines"
        font = cv2.FONT_HERSHEY_SIMPLEX
        scale = 0.4
        (tw, th), _ = cv2.getTextSize(text, font, scale, 1)

        overlay = frame.copy()
        cv2.rectangle(overlay, (0, h - th - 16), (w, h), TEXT_BG, -1)
        cv2.addWeighted(overlay, 0.55, frame, 0.45, 0, frame)

        cv2.putText(frame, text, (10, h - 8), font, scale, CYAN_DIM, 1, cv2.LINE_AA)

    def _apply_vignette(self, frame: np.ndarray):
        """Apply vignette darkening at edges."""
        h, w = frame.shape[:2]
        key = (w, h)
        if key not in self._vignette_cache:
            x = np.linspace(-1, 1, w)
            y = np.linspace(-1, 1, h)
            xx, yy = np.meshgrid(x, y)
            r = np.sqrt(xx ** 2 + yy ** 2)
            vignette = np.clip(1.0 - (r - 0.7) * VIGNETTE_STRENGTH * 2, 0, 1)
            self._vignette_cache[key] = vignette.astype(np.float32)

        mask = self._vignette_cache[key]
        for c in range(3):
            frame[:, :, c] = (frame[:, :, c].astype(np.float32) * mask).astype(np.uint8)

    def _apply_scanlines(self, frame: np.ndarray):
        """Apply subtle scanline effect."""
        h, w = frame.shape[:2]
        key = (w, h)
        if key not in self._scanline_cache:
            scanlines = np.ones((h, w), dtype=np.float32)
            scanlines[::SCANLINE_SPACING, :] = 1.0 - SCANLINE_ALPHA
            self._scanline_cache[key] = scanlines

        mask = self._scanline_cache[key]
        for c in range(3):
            frame[:, :, c] = (frame[:, :, c].astype(np.float32) * mask).astype(np.uint8)
