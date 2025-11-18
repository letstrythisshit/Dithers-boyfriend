#!/bin/bash

# Dither's Boyfriend - Automated Build Script
# This script automatically installs dependencies, downloads required libraries, and builds the application

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "========================================="
echo "Dither's Boyfriend - Automated Build"
echo "========================================="
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[*]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

# Detect OS
print_status "Detecting operating system..."
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
    print_success "Detected Linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
    print_success "Detected macOS"
else
    print_warning "Unknown OS: $OSTYPE"
    OS="unknown"
fi

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    print_warning "Running as root. This is not recommended."
    SUDO=""
else
    SUDO="sudo"
fi

# Install dependencies
print_status "Installing system dependencies..."

if [ "$OS" == "linux" ]; then
    # Check for package manager
    if command -v apt-get &> /dev/null; then
        print_status "Using apt package manager..."
        $SUDO apt-get update -qq
        $SUDO apt-get install -y \
            build-essential \
            git \
            cmake \
            pkg-config \
            libopencv-dev \
            libglfw3-dev \
            libgl1-mesa-dev \
            libglu1-mesa-dev \
            xorg-dev
        print_success "Dependencies installed via apt"
    elif command -v yum &> /dev/null; then
        print_status "Using yum package manager..."
        $SUDO yum groupinstall -y "Development Tools"
        $SUDO yum install -y \
            git \
            cmake \
            opencv-devel \
            glfw-devel \
            mesa-libGL-devel \
            mesa-libGLU-devel
        print_success "Dependencies installed via yum"
    elif command -v dnf &> /dev/null; then
        print_status "Using dnf package manager..."
        $SUDO dnf groupinstall -y "Development Tools"
        $SUDO dnf install -y \
            git \
            cmake \
            opencv-devel \
            glfw-devel \
            mesa-libGL-devel \
            mesa-libGLU-devel
        print_success "Dependencies installed via dnf"
    elif command -v pacman &> /dev/null; then
        print_status "Using pacman package manager..."
        $SUDO pacman -S --noconfirm \
            base-devel \
            git \
            cmake \
            opencv \
            glfw-x11 \
            mesa \
            glu
        print_success "Dependencies installed via pacman"
    else
        print_error "No supported package manager found!"
        print_warning "Please install the following packages manually:"
        echo "  - build-essential / base-devel"
        echo "  - git"
        echo "  - cmake"
        echo "  - opencv / libopencv-dev"
        echo "  - glfw / libglfw3-dev"
        echo "  - OpenGL development libraries"
        exit 1
    fi
elif [ "$OS" == "macos" ]; then
    if command -v brew &> /dev/null; then
        print_status "Using Homebrew package manager..."
        brew install opencv glfw cmake pkg-config
        print_success "Dependencies installed via Homebrew"
    else
        print_error "Homebrew not found!"
        print_warning "Please install Homebrew first: https://brew.sh"
        exit 1
    fi
else
    print_error "Unsupported operating system!"
    exit 1
fi

# Download Dear ImGui if not present
print_status "Checking for Dear ImGui..."
if [ ! -d "external/imgui" ]; then
    print_status "Downloading Dear ImGui..."
    mkdir -p external
    git clone --depth 1 https://github.com/ocornut/imgui.git external/imgui
    print_success "Dear ImGui downloaded"
else
    print_success "Dear ImGui already present"
fi

# Create build directory
print_status "Creating build directory..."
mkdir -p build
print_success "Build directory ready"

# Build the application
print_status "Building Dither's Boyfriend..."
echo ""

if make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2); then
    echo ""
    print_success "Build completed successfully!"
    echo ""
    echo "========================================="
    echo "Build Summary"
    echo "========================================="
    echo "Executable: ./dithers-boyfriend"
    echo ""
    echo "To run the application:"
    echo "  ./dithers-boyfriend"
    echo ""
    echo "To run with an image:"
    echo "  ./dithers-boyfriend path/to/image.png"
    echo ""
    echo "For help:"
    echo "  make help"
    echo "========================================="
else
    echo ""
    print_error "Build failed!"
    echo ""
    echo "Troubleshooting:"
    echo "1. Check if all dependencies are installed"
    echo "2. Review the error messages above"
    echo "3. Try running 'make clean' and rebuilding"
    echo ""
    exit 1
fi

# Test if executable exists and is runnable
if [ -x "./dithers-boyfriend" ]; then
    print_success "Executable is ready to run!"
else
    print_error "Executable not found or not executable!"
    exit 1
fi

echo ""
print_success "Setup complete! Enjoy dithering! 🎨"
echo ""
