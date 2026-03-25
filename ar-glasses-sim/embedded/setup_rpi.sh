#!/bin/bash
# ============================================
# AR Glasses — Raspberry Pi 5 Setup Script
# ============================================
# Run this on a fresh Raspberry Pi OS (64-bit, Bookworm)
# after connecting Hailo-8L M.2 HAT+ and ArduCam OV9281.
#
# Usage: chmod +x setup_rpi.sh && sudo ./setup_rpi.sh

set -e

echo "============================================"
echo "  AR Glasses — RPi 5 Setup"
echo "============================================"

# Update system
echo "[1/7] Updating system..."
apt update && apt upgrade -y

# Install system dependencies
echo "[2/7] Installing system dependencies..."
apt install -y \
    python3-pip python3-venv python3-dev \
    python3-opencv python3-picamera2 \
    libcamera-dev libcamera-apps \
    cmake build-essential \
    git wget curl

# Install Hailo Runtime
echo "[3/7] Installing HailoRT..."
if ! command -v hailortcli &> /dev/null; then
    # Add Hailo APT repository
    wget -qO - https://hailo.ai/developer-zone/hailo-linux-repo.gpg.key | apt-key add -
    echo "deb https://hailo.ai/developer-zone/hailo-linux-repo bookworm main" > \
        /etc/apt/sources.list.d/hailo.list
    apt update
    apt install -y hailort hailort-pcie-driver

    # Verify
    hailortcli fw-control identify
    echo "HailoRT installed successfully!"
else
    echo "HailoRT already installed."
    hailortcli fw-control identify
fi

# Enable PCIe for M.2 HAT+
echo "[4/7] Configuring PCIe for M.2 HAT+..."
if ! grep -q "dtparam=pciex1" /boot/firmware/config.txt; then
    echo "" >> /boot/firmware/config.txt
    echo "# Enable PCIe for M.2 HAT+" >> /boot/firmware/config.txt
    echo "dtparam=pciex1" >> /boot/firmware/config.txt
    echo "dtparam=pciex1_gen=2" >> /boot/firmware/config.txt
    echo "PCIe enabled. Reboot required."
fi

# Enable camera
echo "[5/7] Configuring camera..."
if ! grep -q "camera_auto_detect=1" /boot/firmware/config.txt; then
    echo "camera_auto_detect=1" >> /boot/firmware/config.txt
fi

# Setup Python environment
echo "[6/7] Setting up Python environment..."
VENV_DIR="/opt/ar-glasses/venv"
mkdir -p /opt/ar-glasses
python3 -m venv --system-site-packages "$VENV_DIR"
source "$VENV_DIR/bin/activate"

pip install --upgrade pip
pip install \
    numpy \
    opencv-python-headless \
    hailort

# Download and compile YOLOv8n model for Hailo
echo "[7/7] Preparing YOLOv8n model..."
MODEL_DIR="/opt/ar-glasses/models"
mkdir -p "$MODEL_DIR"

if [ ! -f "$MODEL_DIR/yolov8n_hailo8l.hef" ]; then
    echo "Downloading pre-compiled YOLOv8n HEF for Hailo-8L..."
    echo ""
    echo "NOTE: You need to download the YOLOv8n HEF from Hailo Model Zoo:"
    echo "  https://hailo.ai/developer-zone/model-zoo/"
    echo ""
    echo "Or compile from ONNX:"
    echo "  1. pip install ultralytics"
    echo "  2. yolo export model=yolov8n.pt format=onnx"
    echo "  3. hailo_compiler yolov8n.onnx --hw-arch hailo8l --output $MODEL_DIR/yolov8n_hailo8l.hef"
    echo ""
fi

echo ""
echo "============================================"
echo "  Setup complete!"
echo "============================================"
echo ""
echo "Next steps:"
echo "  1. Reboot: sudo reboot"
echo "  2. Verify Hailo: hailortcli fw-control identify"
echo "  3. Verify camera: libcamera-still -o test.jpg"
echo "  4. Copy embedded code to /opt/ar-glasses/"
echo "  5. Run: python /opt/ar-glasses/main_embedded.py --mode prod"
echo ""
echo "For development without Hailo:"
echo "  python main_embedded.py --mode dev"
echo ""
