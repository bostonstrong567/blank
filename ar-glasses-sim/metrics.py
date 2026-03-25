"""FPS tracker and CSV logger."""

import csv
import os
import time
from collections import deque
from datetime import datetime

from config import CSV_LOG_FILE


class FPSTracker:
    """Tracks FPS using a rolling window."""

    def __init__(self, window_size: int = 60):
        self._timestamps: deque[float] = deque(maxlen=window_size)

    def tick(self):
        self._timestamps.append(time.perf_counter())

    @property
    def fps(self) -> float:
        if len(self._timestamps) < 2:
            return 0.0
        dt = self._timestamps[-1] - self._timestamps[0]
        if dt <= 0:
            return 0.0
        return (len(self._timestamps) - 1) / dt


class CSVLogger:
    """Logs per-frame metrics to CSV."""

    def __init__(self, filepath: str = CSV_LOG_FILE):
        self.filepath = filepath
        self._file = None
        self._writer = None
        self.active = False

    def toggle(self):
        if self.active:
            self.stop()
        else:
            self.start()

    def start(self):
        if self.active:
            return
        self._file = open(self.filepath, "a", newline="")
        self._writer = csv.writer(self._file)
        if os.path.getsize(self.filepath) == 0:
            self._writer.writerow([
                "timestamp", "fps", "inference_ms", "render_ms",
                "total_ms", "person_count",
            ])
        self.active = True

    def stop(self):
        if not self.active:
            return
        if self._file:
            self._file.close()
            self._file = None
            self._writer = None
        self.active = False

    def log(self, fps: float, inference_ms: float, render_ms: float,
            total_ms: float, person_count: int):
        if not self.active or self._writer is None:
            return
        self._writer.writerow([
            datetime.now().isoformat(timespec="milliseconds"),
            f"{fps:.1f}",
            f"{inference_ms:.1f}",
            f"{render_ms:.1f}",
            f"{total_ms:.1f}",
            person_count,
        ])
        self._file.flush()

    def __del__(self):
        self.stop()
