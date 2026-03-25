"""YOLOv8 wrapper with inference timing."""

import time
from dataclasses import dataclass

import numpy as np
from ultralytics import YOLO

from config import MODEL_NAME, PERSON_CLASS_ID


@dataclass
class Detection:
    """Single detection result."""
    x1: int
    y1: int
    x2: int
    y2: int
    confidence: float
    class_id: int
    class_name: str
    track_id: int | None = None


class Detector:
    def __init__(self):
        self.model = YOLO(MODEL_NAME)
        self._last_inference_ms = 0.0

    @property
    def inference_ms(self) -> float:
        return self._last_inference_ms

    def detect(
        self,
        frame: np.ndarray,
        confidence: float = 0.5,
        persons_only: bool = True,
    ) -> list[Detection]:
        """Run detection on a frame and return detections with timing."""
        t0 = time.perf_counter()

        classes = [PERSON_CLASS_ID] if persons_only else None
        results = self.model.track(
            frame,
            conf=confidence,
            classes=classes,
            persist=True,
            verbose=False,
            tracker="bytetrack.yaml",
        )

        self._last_inference_ms = (time.perf_counter() - t0) * 1000.0

        detections = []
        for result in results:
            boxes = result.boxes
            if boxes is None:
                continue
            for i in range(len(boxes)):
                xyxy = boxes.xyxy[i].cpu().numpy().astype(int)
                cls_id = int(boxes.cls[i].cpu().numpy())
                conf = float(boxes.conf[i].cpu().numpy())
                track_id = None
                if boxes.id is not None:
                    track_id = int(boxes.id[i].cpu().numpy())
                detections.append(Detection(
                    x1=xyxy[0], y1=xyxy[1], x2=xyxy[2], y2=xyxy[3],
                    confidence=conf,
                    class_id=cls_id,
                    class_name=result.names[cls_id],
                    track_id=track_id,
                ))
        return detections
