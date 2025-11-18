# Dither's Boyfriend - Makefile

CXX = g++

# Detect OpenCV using pkg-config
OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv 2>/dev/null)
OPENCV_LIBS := $(shell pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv 2>/dev/null)

# If pkg-config fails, use default paths
ifeq ($(OPENCV_CFLAGS),)
    OPENCV_CFLAGS = -I/usr/include/opencv4
endif
ifeq ($(OPENCV_LIBS),)
    OPENCV_LIBS = -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui
endif

CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -I./external/imgui -I./external/imgui/backends $(OPENCV_CFLAGS)
LDFLAGS = -lGL -lglfw $(OPENCV_LIBS)

# Source files
IMGUI_DIR = external/imgui
SRC = src/main.cpp src/dithering.cpp
IMGUI_SRC = $(IMGUI_DIR)/imgui.cpp \
            $(IMGUI_DIR)/imgui_demo.cpp \
            $(IMGUI_DIR)/imgui_draw.cpp \
            $(IMGUI_DIR)/imgui_tables.cpp \
            $(IMGUI_DIR)/imgui_widgets.cpp \
            $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
            $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

# Object files
OBJ_DIR = build
IMGUI_OBJS = $(OBJ_DIR)/imgui.o $(OBJ_DIR)/imgui_demo.o $(OBJ_DIR)/imgui_draw.o \
             $(OBJ_DIR)/imgui_tables.o $(OBJ_DIR)/imgui_widgets.o \
             $(OBJ_DIR)/imgui_impl_glfw.o $(OBJ_DIR)/imgui_impl_opengl3.o
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/dithering.o $(IMGUI_OBJS)

# Target executables
TARGET = dithers-boyfriend
TARGET_CLI = dithers-boyfriend-cli

# Default target
all: $(TARGET) $(TARGET_CLI)

# Create build directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link GUI version
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@echo "GUI version complete! Run with: ./$(TARGET)"

# Link CLI version
$(TARGET_CLI): $(OBJ_DIR)/cli.o $(OBJ_DIR)/dithering.o
	$(CXX) $^ -o $@ $(OPENCV_LIBS)
	@echo "CLI version complete! Run with: ./$(TARGET_CLI)"

# Compile source files
$(OBJ_DIR)/main.o: src/main.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/dithering.o: src/dithering.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/cli.o: src/cli.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile ImGui core files
$(OBJ_DIR)/imgui.o: $(IMGUI_DIR)/imgui.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_demo.o: $(IMGUI_DIR)/imgui_demo.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_draw.o: $(IMGUI_DIR)/imgui_draw.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_tables.o: $(IMGUI_DIR)/imgui_tables.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_widgets.o: $(IMGUI_DIR)/imgui_widgets.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile ImGui backend files
$(OBJ_DIR)/imgui_impl_glfw.o: $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_impl_opengl3.o: $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TARGET_CLI)
	@echo "Clean complete!"

# Install dependencies (Debian/Ubuntu)
deps:
	@echo "Installing dependencies..."
	@command -v apt-get >/dev/null 2>&1 && sudo apt-get update && \
		sudo apt-get install -y build-essential git cmake pkg-config \
		libopencv-dev libglfw3-dev libgl1-mesa-dev zenity || \
		echo "Please install dependencies manually: build-essential, git, cmake, pkg-config, libopencv-dev, libglfw3-dev, libgl1-mesa-dev, zenity"
	@echo "Dependencies installed!"
	@echo "Note: zenity provides native file dialogs (optional but recommended)"

# Download ImGui
imgui:
	@echo "Downloading Dear ImGui..."
	@if [ ! -d "$(IMGUI_DIR)" ]; then \
		git clone --depth 1 https://github.com/ocornut/imgui.git $(IMGUI_DIR); \
		echo "Dear ImGui downloaded!"; \
	else \
		echo "Dear ImGui already exists!"; \
	fi

# Setup (install deps + download imgui)
setup: deps imgui
	@echo "Setup complete!"

# Run
run: $(TARGET)
	./$(TARGET)

# Help
help:
	@echo "Dither's Boyfriend - Makefile targets:"
	@echo "  make          - Build both GUI and CLI versions"
	@echo "  make setup    - Install dependencies and download ImGui"
	@echo "  make deps     - Install system dependencies"
	@echo "  make imgui    - Download Dear ImGui"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make run      - Build and run the GUI application"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Executables:"
	@echo "  ./dithers-boyfriend        - GUI version with visual interface"
	@echo "  ./dithers-boyfriend-cli    - CLI version for batch processing"

.PHONY: all clean deps imgui setup run help
