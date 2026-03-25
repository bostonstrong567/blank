#!/usr/bin/env python3
"""
Hailo-8L person detection pipeline for Raspberry Pi 5.

Uses HailoRT Python API to run YOLOv8-nano on the Hailo-8L M.2 accelerator.
Expects a compiled .hef model file (HailoRT compiled from ONNX).

Setup:
    1. Install HailoRT: https://hailo.ai/developer-zone/
    2. Compile model: hailo_compiler yolov8n.onnx --hw-arch hailo8l
    3. Place .hef file in this directory
"""

import time
from dataclasses import dataclass, field

import numpy as np

try:
    from hailo_platform import (
        HEF, ConfigureParams, FormatType, HailoStreamInterface,
        InferVStreams, InputVStreamParams, OutputVStreamParams, VDevice,
    )
    HAILO_AVAILABLE = True
except ImportError:
    HAILO_AVAILABLE = False
    print("WARNING: HailoRT not installed. Using CPU fallback.")


# YOLOv8n input size
INPUT_W = 640
INPUT_H = 640
PERSON_CLASS = 0
CONFIDENCE_THRESHOLD = 0.5
NMS_IOU_THRESHOLD = 0.45

MODEL_PATH = "yolov8n_hailo8l.hef"


@dataclass
class Detection:
    x1: int
    y1: int
    x2: int
    y2: int
    confidence: float
    class_id: int
    track_id: int | None = None


@dataclass
class InferenceResult:
    detections: list[Detection] = field(default_factory=list)
    inference_ms: float = 0.0
    preprocess_ms: float = 0.0
    postprocess_ms: float = 0.0


class HailoDetector:
    """YOLOv8-nano detector running on Hailo-8L."""

    def __init__(self, model_path: str = MODEL_PATH, conf: float = CONFIDENCE_THRESHOLD):
        self.conf = conf
        self._device = None
        self._hef = None
        self._network_group = None
        self._input_vstreams = None
        self._output_vstreams = None

        if not HAILO_AVAILABLE:
            raise RuntimeError(
                "HailoRT not available. Install with: "
                "pip install hailort (or apt install hailort on RPi OS)"
            )

        self._init_device(model_path)

    def _init_device(self, model_path: str):
        """Initialize Hailo device and load model."""
        print(f"Initializing Hailo-8L with model: {model_path}")
        t0 = time.perf_counter()

        # Create virtual device (auto-detects Hailo-8L)
        self._device = VDevice()

        # Load HEF (Hailo Executable Format)
        self._hef = HEF(model_path)

        # Configure network
        configure_params = ConfigureParams.create_from_hef(
            self._hef, interface=HailoStreamInterface.PCIe
        )
        self._network_group = self._device.configure(self._hef, configure_params)[0]

        # Get stream info
        self._input_info = self._hef.get_input_vstream_infos()
        self._output_info = self._hef.get_output_vstream_infos()

        input_params = InputVStreamParams.make_from_network_group(
            self._network_group, quantized=False, format_type=FormatType.FLOAT32
        )
        output_params = OutputVStreamParams.make_from_network_group(
            self._network_group, quantized=False, format_type=FormatType.FLOAT32
        )

        self._input_params = input_params
        self._output_params = output_params

        dt = (time.perf_counter() - t0) * 1000
        print(f"Hailo-8L initialized in {dt:.0f}ms")
        print(f"  Input:  {self._input_info[0].shape}")
        print(f"  Output: {[o.shape for o in self._output_info]}")

    def detect(self, frame: np.ndarray) -> InferenceResult:
        """Run detection on a BGR frame."""
        result = InferenceResult()

        # Preprocess
        t0 = time.perf_counter()
        input_data = self._preprocess(frame)
        result.preprocess_ms = (time.perf_counter() - t0) * 1000

        # Inference on Hailo
        t1 = time.perf_counter()
        with InferVStreams(
            self._network_group, self._input_params, self._output_params
        ) as pipeline:
            input_dict = {self._input_info[0].name: input_data}
            raw_output = pipeline.infer(input_dict)
        result.inference_ms = (time.perf_counter() - t1) * 1000

        # Postprocess
        t2 = time.perf_counter()
        result.detections = self._postprocess(raw_output, frame.shape)
        result.postprocess_ms = (time.perf_counter() - t2) * 1000

        return result

    def _preprocess(self, frame: np.ndarray) -> np.ndarray:
        """Resize and normalize frame for YOLOv8 input."""
        import cv2
        # Letterbox resize
        h, w = frame.shape[:2]
        scale = min(INPUT_W / w, INPUT_H / h)
        new_w, new_h = int(w * scale), int(h * scale)
        resized = cv2.resize(frame, (new_w, new_h), interpolation=cv2.INTER_LINEAR)

        # Pad to INPUT_W x INPUT_H
        padded = np.full((INPUT_H, INPUT_W, 3), 114, dtype=np.uint8)
        pad_x = (INPUT_W - new_w) // 2
        pad_y = (INPUT_H - new_h) // 2
        padded[pad_y:pad_y+new_h, pad_x:pad_x+new_w] = resized

        # Normalize to [0, 1] and convert to float32
        blob = padded.astype(np.float32) / 255.0

        # Add batch dimension
        return np.expand_dims(blob, axis=0)

    def _postprocess(
        self, raw_output: dict, orig_shape: tuple
    ) -> list[Detection]:
        """Parse YOLOv8 output into detections."""
        # YOLOv8 output: [1, 84, 8400] — 84 = 4 (bbox) + 80 (classes)
        # Get the first output tensor
        output_name = self._output_info[0].name
        output = raw_output[output_name]

        if output.ndim == 3:
            output = output[0]  # remove batch dim

        # Transpose if needed: [84, 8400] -> [8400, 84]
        if output.shape[0] < output.shape[1]:
            output = output.T

        boxes = output[:, :4]      # cx, cy, w, h
        scores = output[:, 4:]     # class scores

        # Filter by person class confidence
        person_scores = scores[:, PERSON_CLASS]
        mask = person_scores > self.conf
        filtered_boxes = boxes[mask]
        filtered_scores = person_scores[mask]

        if len(filtered_boxes) == 0:
            return []

        # Convert cx, cy, w, h -> x1, y1, x2, y2
        x1 = filtered_boxes[:, 0] - filtered_boxes[:, 2] / 2
        y1 = filtered_boxes[:, 1] - filtered_boxes[:, 3] / 2
        x2 = filtered_boxes[:, 0] + filtered_boxes[:, 2] / 2
        y2 = filtered_boxes[:, 1] + filtered_boxes[:, 3] / 2

        # NMS
        keep = self._nms(x1, y1, x2, y2, filtered_scores, NMS_IOU_THRESHOLD)

        # Scale back to original frame
        orig_h, orig_w = orig_shape[:2]
        scale = min(INPUT_W / orig_w, INPUT_H / orig_h)
        pad_x = (INPUT_W - orig_w * scale) / 2
        pad_y = (INPUT_H - orig_h * scale) / 2

        detections = []
        for i in keep:
            det = Detection(
                x1=int((x1[i] - pad_x) / scale),
                y1=int((y1[i] - pad_y) / scale),
                x2=int((x2[i] - pad_x) / scale),
                y2=int((y2[i] - pad_y) / scale),
                confidence=float(filtered_scores[i]),
                class_id=PERSON_CLASS,
            )
            detections.append(det)

        return detections

    @staticmethod
    def _nms(x1, y1, x2, y2, scores, iou_thresh):
        """Non-maximum suppression."""
        areas = (x2 - x1) * (y2 - y1)
        order = scores.argsort()[::-1]
        keep = []

        while len(order) > 0:
            i = order[0]
            keep.append(i)

            xx1 = np.maximum(x1[i], x1[order[1:]])
            yy1 = np.maximum(y1[i], y1[order[1:]])
            xx2 = np.minimum(x2[i], x2[order[1:]])
            yy2 = np.minimum(y2[i], y2[order[1:]])

            inter = np.maximum(0, xx2 - xx1) * np.maximum(0, yy2 - yy1)
            iou = inter / (areas[i] + areas[order[1:]] - inter)

            remaining = np.where(iou <= iou_thresh)[0]
            order = order[remaining + 1]

        return keep
