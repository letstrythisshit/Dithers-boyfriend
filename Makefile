# Dither's Boyfriend - Makefile

CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -I./external/imgui -I./external/imgui/backends
LDFLAGS = -lGL -lGLFW -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui

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
OBJS = $(SRC:src/%.cpp=$(OBJ_DIR)/%.o) $(IMGUI_SRC:$(IMGUI_DIR)/%.cpp=$(OBJ_DIR)/imgui_%.o)

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
	$(CXX) $^ -o $@ -lopencv_core -lopencv_imgproc -lopencv_imgcodecs
	@echo "CLI version complete! Run with: ./$(TARGET_CLI)"

# Compile source files
$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile ImGui files
$(OBJ_DIR)/imgui_%.o: $(IMGUI_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_%.o: $(IMGUI_DIR)/backends/%.cpp | $(OBJ_DIR)
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
		libopencv-dev libglfw3-dev libgl1-mesa-dev || \
		echo "Please install dependencies manually: build-essential, git, cmake, pkg-config, libopencv-dev, libglfw3-dev, libgl1-mesa-dev"
	@echo "Dependencies installed!"

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
