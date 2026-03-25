"""Configuration constants for the AR Glasses Simulator."""

# Resolution presets (width, height) — cycle with 'r'
RESOLUTIONS = [
    (1280, 720),
    (640, 480),
    (320, 240),
]

# Confidence threshold presets — cycle with 't'
CONFIDENCE_THRESHOLDS = [0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]

# Default confidence threshold index
DEFAULT_THRESHOLD_IDX = 2  # 0.5

# HUD colors (BGR for OpenCV)
CYAN = (255, 170, 136)          # #88AAFF in BGR
CYAN_DIM = (200, 130, 100)
TEXT_BG = (30, 30, 30)
WHITE = (255, 255, 255)
GREEN = (100, 255, 100)
YELLOW = (80, 230, 255)
RED = (80, 80, 255)

# HUD layout
HUD_FONT_SCALE = 0.5
HUD_THICKNESS = 1
HUD_LINE_HEIGHT = 22
HUD_PADDING = 8

# Corner bracket length as fraction of box side
BRACKET_FRACTION = 0.2
BRACKET_THICKNESS = 2
BRACKET_MIN_LEN = 10

# Vignette strength (0.0 = none, 1.0 = full black corners)
VIGNETTE_STRENGTH = 0.35

# Scanline parameters
SCANLINE_SPACING = 3
SCANLINE_ALPHA = 0.07

# COCO person class ID
PERSON_CLASS_ID = 0

# YOLOv8 model name (auto-downloaded)
MODEL_NAME = "yolov8n.pt"

# CSV log file
CSV_LOG_FILE = "metrics_log.csv"

# Screenshot directory
SCREENSHOT_DIR = "screenshots"
