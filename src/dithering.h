#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <random>

namespace Dithering {

// Dithering algorithm types
enum class Algorithm {
    FLOYD_STEINBERG,
    ATKINSON,
    JARVIS_JUDICE_NINKE,
    STUCKI,
    BURKES,
    SIERRA,
    SIERRA_TWO_ROW,
    SIERRA_LITE,
    ORDERED_BAYER_2X2,
    ORDERED_BAYER_4X4,
    ORDERED_BAYER_8X8,
    ORDERED_BAYER_16X16,
    BLUE_NOISE,
    WHITE_NOISE,
    RANDOM_DITHER,
    PATTERN_DITHER,
    DOT_DIFFUSION,
    RIEMERSMA,
    GRADIENT_BASED,
    VARIABLE_ERROR_DIFFUSION,
    OSTROMOUKHOV,
    FAN,
    SHIAU_FAN,
    STEVENPIGEON
};

// Palette modes
enum class PaletteMode {
    MONOCHROME,
    GRAYSCALE_4,
    GRAYSCALE_8,
    GRAYSCALE_16,
    CGA,
    EGA,
    VGA,
    GAMEBOY,
    PICO8,
    CUSTOM
};

// Dithering parameters
struct Parameters {
    Algorithm algorithm = Algorithm::FLOYD_STEINBERG;
    PaletteMode paletteMode = PaletteMode::MONOCHROME;
    std::vector<cv::Vec3b> customPalette;

    // Adjustable parameters
    float strength = 1.0f;          // Error diffusion strength
    float serpentine = 1.0f;        // Serpentine scanning (0=off, 1=on)
    float colorization = 0.0f;      // Add slight color tint
    int levels = 2;                 // Number of intensity levels
    float gamma = 1.0f;             // Gamma correction
    float contrast = 1.0f;          // Contrast adjustment
    float brightness = 0.0f;        // Brightness adjustment
    float saturation = 1.0f;        // Saturation adjustment
    int bayerSize = 8;              // Bayer matrix size
    unsigned int seed = 42;         // Random seed
    bool useBlueNoise = true;       // Use blue noise for ordered dithering
    float ditherScale = 1.0f;       // Scale factor for dither pattern
};

// Core dithering function
cv::Mat ditherImage(const cv::Mat& input, const Parameters& params);

// Individual algorithm implementations
cv::Mat floydSteinberg(const cv::Mat& input, const Parameters& params);
cv::Mat atkinson(const cv::Mat& input, const Parameters& params);
cv::Mat jarvisJudiceNinke(const cv::Mat& input, const Parameters& params);
cv::Mat stucki(const cv::Mat& input, const Parameters& params);
cv::Mat burkes(const cv::Mat& input, const Parameters& params);
cv::Mat sierra(const cv::Mat& input, const Parameters& params);
cv::Mat sierraTwo(const cv::Mat& input, const Parameters& params);
cv::Mat sierraLite(const cv::Mat& input, const Parameters& params);
cv::Mat orderedDither(const cv::Mat& input, const Parameters& params);
cv::Mat blueNoiseDither(const cv::Mat& input, const Parameters& params);
cv::Mat whiteNoiseDither(const cv::Mat& input, const Parameters& params);
cv::Mat randomDither(const cv::Mat& input, const Parameters& params);
cv::Mat patternDither(const cv::Mat& input, const Parameters& params);
cv::Mat dotDiffusion(const cv::Mat& input, const Parameters& params);
cv::Mat riemersma(const cv::Mat& input, const Parameters& params);
cv::Mat gradientBased(const cv::Mat& input, const Parameters& params);
cv::Mat variableErrorDiffusion(const cv::Mat& input, const Parameters& params);
cv::Mat ostromoukhov(const cv::Mat& input, const Parameters& params);
cv::Mat fan(const cv::Mat& input, const Parameters& params);
cv::Mat shiauFan(const cv::Mat& input, const Parameters& params);
cv::Mat stevenPigeon(const cv::Mat& input, const Parameters& params);

// Utility functions
std::vector<cv::Vec3b> getPalette(PaletteMode mode);
cv::Vec3b findClosestColor(const cv::Vec3b& color, const std::vector<cv::Vec3b>& palette);
cv::Mat generateBlueNoiseTexture(int size, unsigned int seed);
cv::Mat generateBayerMatrix(int size);
std::string getAlgorithmName(Algorithm algo);
std::string getPaletteModeName(PaletteMode mode);

} // namespace Dithering
