#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "dithering.h"

// Application state
struct AppState {
    cv::Mat originalImage;
    cv::Mat processedImage;
    cv::Mat displayImage;
    GLuint originalTexture = 0;
    GLuint processedTexture = 0;

    Dithering::Parameters params;

    std::string currentFile;
    bool imageLoaded = false;
    bool processing = false;
    bool autoUpdate = true;

    // Video state
    cv::VideoCapture videoCapture;
    cv::VideoWriter videoWriter;
    bool isVideo = false;
    bool videoProcessing = false;
    int currentFrame = 0;
    int totalFrames = 0;
    float videoProgress = 0.0f;

    // UI state
    int selectedAlgorithm = 0;
    int selectedPalette = 0;
    float previewScale = 1.0f;
    bool showOriginal = true;
    bool showProcessed = true;
    bool splitView = true;

    // Performance
    float processingTime = 0.0f;
};

// Helper function to load texture from cv::Mat
GLuint loadTextureFromMat(const cv::Mat& mat) {
    if (mat.empty()) return 0;

    cv::Mat rgba;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgba, cv::COLOR_BGR2RGBA);
    } else if (mat.channels() == 4) {
        rgba = mat;
    } else {
        cv::cvtColor(mat, rgba, cv::COLOR_GRAY2RGBA);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba.cols, rgba.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data);

    return texture;
}

// Update texture with new mat
void updateTexture(GLuint& texture, const cv::Mat& mat) {
    if (texture != 0) {
        glDeleteTextures(1, &texture);
    }
    texture = loadTextureFromMat(mat);
}

// Process image with current parameters
void processImage(AppState& state) {
    if (!state.imageLoaded || state.originalImage.empty()) return;

    state.processing = true;
    auto start = std::chrono::high_resolution_clock::now();

    state.processedImage = Dithering::ditherImage(state.originalImage, state.params);
    updateTexture(state.processedTexture, state.processedImage);

    auto end = std::chrono::high_resolution_clock::now();
    state.processingTime = std::chrono::duration<float, std::milli>(end - start).count();
    state.processing = false;
}

// Load image file
bool loadImage(AppState& state, const std::string& filename) {
    cv::Mat img = cv::imread(filename, cv::IMREAD_COLOR);
    if (img.empty()) return false;

    state.originalImage = img;
    state.currentFile = filename;
    state.imageLoaded = true;
    state.isVideo = false;

    updateTexture(state.originalTexture, state.originalImage);
    processImage(state);

    return true;
}

// Save image file
bool saveImage(AppState& state, const std::string& filename) {
    if (state.processedImage.empty()) return false;
    return cv::imwrite(filename, state.processedImage);
}

// Process video
void processVideo(AppState& state, const std::string& inputPath, const std::string& outputPath) {
    state.videoProcessing = true;
    state.videoProgress = 0.0f;

    cv::VideoCapture cap(inputPath);
    if (!cap.isOpened()) {
        state.videoProcessing = false;
        return;
    }

    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    state.totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    cv::VideoWriter writer(outputPath, cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
                          fps, cv::Size(frameWidth, frameHeight));

    if (!writer.isOpened()) {
        state.videoProcessing = false;
        return;
    }

    cv::Mat frame;
    state.currentFrame = 0;

    while (cap.read(frame)) {
        cv::Mat dithered = Dithering::ditherImage(frame, state.params);
        writer.write(dithered);

        state.currentFrame++;
        state.videoProgress = static_cast<float>(state.currentFrame) / state.totalFrames;
    }

    cap.release();
    writer.release();
    state.videoProcessing = false;
}

// Main GUI rendering
void renderGUI(AppState& state) {
    ImGuiIO& io = ImGui::GetIO();

    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Image", "Ctrl+O")) {
                // File dialog would go here
                std::cout << "Open file dialog..." << std::endl;
            }
            if (ImGui::MenuItem("Open Video")) {
                std::cout << "Open video dialog..." << std::endl;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save As...", "Ctrl+S")) {
                if (state.imageLoaded && !state.processedImage.empty()) {
                    saveImage(state, "output.png");
                    std::cout << "Saved to output.png" << std::endl;
                }
            }
            if (ImGui::MenuItem("Export Video")) {
                std::cout << "Export video dialog..." << std::endl;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Split View", nullptr, &state.splitView);
            ImGui::MenuItem("Show Original", nullptr, &state.showOriginal);
            ImGui::MenuItem("Show Processed", nullptr, &state.showProcessed);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                std::cout << "Dither's Boyfriend - Advanced Dithering Application" << std::endl;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // Left panel - Controls
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2(400, io.DisplaySize.y - 20));
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 1.0f, 1.0f));
    ImGui::TextWrapped("Dither's Boyfriend");
    ImGui::PopStyleColor();
    ImGui::Separator();

    // Algorithm selection
    ImGui::Text("Dithering Algorithm");
    const char* algorithms[] = {
        "Floyd-Steinberg", "Atkinson", "Jarvis-Judice-Ninke", "Stucki",
        "Burkes", "Sierra", "Sierra Two-Row", "Sierra Lite",
        "Ordered Bayer 2x2", "Ordered Bayer 4x4", "Ordered Bayer 8x8", "Ordered Bayer 16x16",
        "Blue Noise", "White Noise", "Random", "Pattern",
        "Dot Diffusion", "Riemersma", "Gradient-Based", "Variable Error",
        "Ostromoukhov", "Fan", "Shiau-Fan", "Steven Pigeon"
    };

    if (ImGui::Combo("##Algorithm", &state.selectedAlgorithm, algorithms, IM_ARRAYSIZE(algorithms))) {
        state.params.algorithm = static_cast<Dithering::Algorithm>(state.selectedAlgorithm);
        if (state.autoUpdate) processImage(state);
    }

    ImGui::Separator();

    // Palette selection
    ImGui::Text("Color Palette");
    const char* palettes[] = {
        "Monochrome", "Grayscale 4", "Grayscale 8", "Grayscale 16",
        "CGA", "EGA", "VGA", "Game Boy", "PICO-8", "Custom"
    };

    if (ImGui::Combo("##Palette", &state.selectedPalette, palettes, IM_ARRAYSIZE(palettes))) {
        state.params.paletteMode = static_cast<Dithering::PaletteMode>(state.selectedPalette);
        if (state.autoUpdate) processImage(state);
    }

    ImGui::Separator();

    // Parameters
    ImGui::Text("Parameters");

    bool needsUpdate = false;

    if (ImGui::SliderFloat("Strength", &state.params.strength, 0.0f, 2.0f)) needsUpdate = true;
    if (ImGui::SliderFloat("Serpentine", &state.params.serpentine, 0.0f, 1.0f)) needsUpdate = true;
    if (ImGui::SliderFloat("Gamma", &state.params.gamma, 0.1f, 3.0f)) needsUpdate = true;
    if (ImGui::SliderFloat("Contrast", &state.params.contrast, 0.0f, 3.0f)) needsUpdate = true;
    if (ImGui::SliderFloat("Brightness", &state.params.brightness, -1.0f, 1.0f)) needsUpdate = true;
    if (ImGui::SliderFloat("Saturation", &state.params.saturation, 0.0f, 2.0f)) needsUpdate = true;

    if (state.selectedAlgorithm >= 8 && state.selectedAlgorithm <= 11) {
        int bayerSizes[] = {2, 4, 8, 16};
        state.params.bayerSize = bayerSizes[state.selectedAlgorithm - 8];
    }

    if (ImGui::SliderInt("Random Seed", reinterpret_cast<int*>(&state.params.seed), 0, 1000)) needsUpdate = true;

    if (needsUpdate && state.autoUpdate) {
        processImage(state);
    }

    ImGui::Separator();

    // Action buttons
    ImGui::Checkbox("Auto Update", &state.autoUpdate);

    if (!state.autoUpdate) {
        if (ImGui::Button("Process Image", ImVec2(-1, 30))) {
            processImage(state);
        }
    }

    if (ImGui::Button("Load Test Image", ImVec2(-1, 30))) {
        // Create a test gradient image
        state.originalImage = cv::Mat(512, 512, CV_8UC3);
        for (int y = 0; y < 512; ++y) {
            for (int x = 0; x < 512; ++x) {
                state.originalImage.at<cv::Vec3b>(y, x) = cv::Vec3b(
                    static_cast<uchar>(x * 255 / 512),
                    static_cast<uchar>(y * 255 / 512),
                    static_cast<uchar>((x + y) * 255 / 1024)
                );
            }
        }
        state.imageLoaded = true;
        state.currentFile = "test_gradient.png";
        updateTexture(state.originalTexture, state.originalImage);
        processImage(state);
    }

    if (state.imageLoaded && ImGui::Button("Reset Parameters", ImVec2(-1, 30))) {
        state.params = Dithering::Parameters();
        state.selectedAlgorithm = 0;
        state.selectedPalette = 0;
        if (state.autoUpdate) processImage(state);
    }

    ImGui::Separator();

    // Stats
    ImGui::Text("Statistics");
    if (state.imageLoaded) {
        ImGui::Text("Image: %dx%d", state.originalImage.cols, state.originalImage.rows);
        ImGui::Text("Processing time: %.2f ms", state.processingTime);
    }

    if (state.videoProcessing) {
        ImGui::Text("Processing video...");
        ImGui::ProgressBar(state.videoProgress);
        ImGui::Text("Frame %d / %d", state.currentFrame, state.totalFrames);
    }

    ImGui::End();

    // Right panel - Image display
    ImGui::SetNextWindowPos(ImVec2(400, 20));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 400, io.DisplaySize.y - 20));
    ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    if (state.imageLoaded) {
        ImVec2 availSize = ImGui::GetContentRegionAvail();

        if (state.splitView) {
            // Split view - original on left, processed on right
            float halfWidth = availSize.x * 0.5f - 10;

            if (state.showOriginal && state.originalTexture) {
                ImGui::BeginChild("Original", ImVec2(halfWidth, availSize.y), true);
                ImGui::Text("Original");
                float scale = std::min(halfWidth / state.originalImage.cols,
                                     (availSize.y - 30) / state.originalImage.rows);
                ImVec2 imgSize(state.originalImage.cols * scale, state.originalImage.rows * scale);
                ImGui::Image((void*)(intptr_t)state.originalTexture, imgSize);
                ImGui::EndChild();
            }

            ImGui::SameLine();

            if (state.showProcessed && state.processedTexture) {
                ImGui::BeginChild("Processed", ImVec2(halfWidth, availSize.y), true);
                ImGui::Text("Dithered");
                float scale = std::min(halfWidth / state.processedImage.cols,
                                     (availSize.y - 30) / state.processedImage.rows);
                ImVec2 imgSize(state.processedImage.cols * scale, state.processedImage.rows * scale);
                ImGui::Image((void*)(intptr_t)state.processedTexture, imgSize);
                ImGui::EndChild();
            }
        } else {
            // Single view - show processed only
            if (state.processedTexture) {
                float scale = std::min(availSize.x / state.processedImage.cols,
                                     availSize.y / state.processedImage.rows);
                ImVec2 imgSize(state.processedImage.cols * scale, state.processedImage.rows * scale);

                // Center the image
                ImVec2 cursorPos = ImGui::GetCursorPos();
                ImGui::SetCursorPos(ImVec2(
                    cursorPos.x + (availSize.x - imgSize.x) * 0.5f,
                    cursorPos.y + (availSize.y - imgSize.y) * 0.5f
                ));

                ImGui::Image((void*)(intptr_t)state.processedTexture, imgSize);
            }
        }
    } else {
        ImVec2 availSize = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPos(ImVec2(availSize.x * 0.5f - 100, availSize.y * 0.5f));
        ImGui::Text("No image loaded");
        ImGui::Text("Use 'Load Test Image' to start");
    }

    ImGui::End();
}

// Setup Dear ImGui style (Photoshop-like dark theme)
void setupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Dark Photoshop-like theme
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.70f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.30f, 0.30f, 0.30f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.50f, 0.50f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

    style.WindowRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
}

// GLFW error callback
void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main(int argc, char** argv) {
    // Set GLFW error callback
    glfwSetErrorCallback(glfwErrorCallback);

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        std::cerr << "Make sure you have a display and graphics drivers installed." << std::endl;
        return -1;
    }

    // Try different OpenGL configurations
    GLFWwindow* window = nullptr;
    const char* glsl_version = nullptr;

    // Try OpenGL 3.3 Core first
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    window = glfwCreateWindow(1600, 900, "Dither's Boyfriend - Advanced Dithering", nullptr, nullptr);
    glsl_version = "#version 330";

    // Fallback to OpenGL 3.0
    if (!window) {
        std::cerr << "OpenGL 3.3 not available, trying OpenGL 3.0..." << std::endl;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
        window = glfwCreateWindow(1600, 900, "Dither's Boyfriend - Advanced Dithering", nullptr, nullptr);
        glsl_version = "#version 130";
    }

    // Fallback to OpenGL 2.1
    if (!window) {
        std::cerr << "OpenGL 3.0 not available, trying OpenGL 2.1..." << std::endl;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
        window = glfwCreateWindow(1600, 900, "Dither's Boyfriend - Advanced Dithering", nullptr, nullptr);
        glsl_version = "#version 120";
    }

    // Final check
    if (!window) {
        std::cerr << "Failed to create GLFW window with any OpenGL version!" << std::endl;
        std::cerr << "\nTroubleshooting:" << std::endl;
        std::cerr << "1. Check if DISPLAY is set: echo $DISPLAY" << std::endl;
        std::cerr << "2. Check if X server is running: ps aux | grep X" << std::endl;
        std::cerr << "3. Update graphics drivers" << std::endl;
        std::cerr << "4. Try running: export DISPLAY=:0" << std::endl;
        std::cerr << "\nFor headless systems, use the CLI version instead:" << std::endl;
        std::cerr << "  ./dithers-boyfriend-cli --help" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL context created successfully!" << std::endl;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    setupImGuiStyle();

    // Application state
    AppState state;

    // Load image from command line if provided
    if (argc > 1) {
        loadImage(state, argv[1]);
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render GUI
        renderGUI(state);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.18f, 0.18f, 0.18f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    if (state.originalTexture) glDeleteTextures(1, &state.originalTexture);
    if (state.processedTexture) glDeleteTextures(1, &state.processedTexture);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
