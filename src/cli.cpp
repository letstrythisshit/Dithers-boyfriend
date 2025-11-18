#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "dithering.h"

void printUsage(const char* program) {
    std::cout << "Dither's Boyfriend - CLI Version\n";
    std::cout << "Usage: " << program << " [options] input_file output_file\n\n";
    std::cout << "Options:\n";
    std::cout << "  -a, --algorithm <name>    Dithering algorithm (default: floyd-steinberg)\n";
    std::cout << "  -p, --palette <name>      Color palette (default: monochrome)\n";
    std::cout << "  -s, --strength <float>    Strength (0.0-2.0, default: 1.0)\n";
    std::cout << "  -g, --gamma <float>       Gamma correction (0.1-3.0, default: 1.0)\n";
    std::cout << "  -c, --contrast <float>    Contrast (0.0-3.0, default: 1.0)\n";
    std::cout << "  -b, --brightness <float>  Brightness (-1.0-1.0, default: 0.0)\n";
    std::cout << "  --saturation <float>      Saturation (0.0-2.0, default: 1.0)\n";
    std::cout << "  --serpentine              Enable serpentine scanning\n";
    std::cout << "  --seed <int>              Random seed (default: 42)\n";
    std::cout << "  -h, --help                Show this help message\n\n";

    std::cout << "Algorithms:\n";
    std::cout << "  floyd-steinberg, atkinson, jarvis, stucki, burkes, sierra,\n";
    std::cout << "  sierra-two, sierra-lite, bayer-2x2, bayer-4x4, bayer-8x8,\n";
    std::cout << "  bayer-16x16, blue-noise, white-noise, random, pattern,\n";
    std::cout << "  dot-diffusion, riemersma, gradient, variable, ostromoukhov,\n";
    std::cout << "  fan, shiau-fan, steven-pigeon\n\n";

    std::cout << "Palettes:\n";
    std::cout << "  monochrome, gray4, gray8, gray16, cga, ega, vga,\n";
    std::cout << "  gameboy, pico8\n\n";

    std::cout << "Examples:\n";
    std::cout << "  " << program << " input.jpg output.png\n";
    std::cout << "  " << program << " -a atkinson -p gameboy input.jpg output.png\n";
    std::cout << "  " << program << " -a bayer-8x8 -p pico8 -s 1.5 input.jpg output.png\n";
}

Dithering::Algorithm parseAlgorithm(const std::string& name) {
    if (name == "floyd-steinberg") return Dithering::Algorithm::FLOYD_STEINBERG;
    if (name == "atkinson") return Dithering::Algorithm::ATKINSON;
    if (name == "jarvis") return Dithering::Algorithm::JARVIS_JUDICE_NINKE;
    if (name == "stucki") return Dithering::Algorithm::STUCKI;
    if (name == "burkes") return Dithering::Algorithm::BURKES;
    if (name == "sierra") return Dithering::Algorithm::SIERRA;
    if (name == "sierra-two") return Dithering::Algorithm::SIERRA_TWO_ROW;
    if (name == "sierra-lite") return Dithering::Algorithm::SIERRA_LITE;
    if (name == "bayer-2x2") return Dithering::Algorithm::ORDERED_BAYER_2X2;
    if (name == "bayer-4x4") return Dithering::Algorithm::ORDERED_BAYER_4X4;
    if (name == "bayer-8x8") return Dithering::Algorithm::ORDERED_BAYER_8X8;
    if (name == "bayer-16x16") return Dithering::Algorithm::ORDERED_BAYER_16X16;
    if (name == "blue-noise") return Dithering::Algorithm::BLUE_NOISE;
    if (name == "white-noise") return Dithering::Algorithm::WHITE_NOISE;
    if (name == "random") return Dithering::Algorithm::RANDOM_DITHER;
    if (name == "pattern") return Dithering::Algorithm::PATTERN_DITHER;
    if (name == "dot-diffusion") return Dithering::Algorithm::DOT_DIFFUSION;
    if (name == "riemersma") return Dithering::Algorithm::RIEMERSMA;
    if (name == "gradient") return Dithering::Algorithm::GRADIENT_BASED;
    if (name == "variable") return Dithering::Algorithm::VARIABLE_ERROR_DIFFUSION;
    if (name == "ostromoukhov") return Dithering::Algorithm::OSTROMOUKHOV;
    if (name == "fan") return Dithering::Algorithm::FAN;
    if (name == "shiau-fan") return Dithering::Algorithm::SHIAU_FAN;
    if (name == "steven-pigeon") return Dithering::Algorithm::STEVENPIGEON;

    std::cerr << "Unknown algorithm: " << name << ", using floyd-steinberg\n";
    return Dithering::Algorithm::FLOYD_STEINBERG;
}

Dithering::PaletteMode parsePalette(const std::string& name) {
    if (name == "monochrome") return Dithering::PaletteMode::MONOCHROME;
    if (name == "gray4") return Dithering::PaletteMode::GRAYSCALE_4;
    if (name == "gray8") return Dithering::PaletteMode::GRAYSCALE_8;
    if (name == "gray16") return Dithering::PaletteMode::GRAYSCALE_16;
    if (name == "cga") return Dithering::PaletteMode::CGA;
    if (name == "ega") return Dithering::PaletteMode::EGA;
    if (name == "vga") return Dithering::PaletteMode::VGA;
    if (name == "gameboy") return Dithering::PaletteMode::GAMEBOY;
    if (name == "pico8") return Dithering::PaletteMode::PICO8;

    std::cerr << "Unknown palette: " << name << ", using monochrome\n";
    return Dithering::PaletteMode::MONOCHROME;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    Dithering::Parameters params;
    std::string inputFile, outputFile;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-a" || arg == "--algorithm") {
            if (i + 1 < argc) {
                params.algorithm = parseAlgorithm(argv[++i]);
            }
        }
        else if (arg == "-p" || arg == "--palette") {
            if (i + 1 < argc) {
                params.paletteMode = parsePalette(argv[++i]);
            }
        }
        else if (arg == "-s" || arg == "--strength") {
            if (i + 1 < argc) {
                params.strength = std::stof(argv[++i]);
            }
        }
        else if (arg == "-g" || arg == "--gamma") {
            if (i + 1 < argc) {
                params.gamma = std::stof(argv[++i]);
            }
        }
        else if (arg == "-c" || arg == "--contrast") {
            if (i + 1 < argc) {
                params.contrast = std::stof(argv[++i]);
            }
        }
        else if (arg == "-b" || arg == "--brightness") {
            if (i + 1 < argc) {
                params.brightness = std::stof(argv[++i]);
            }
        }
        else if (arg == "--saturation") {
            if (i + 1 < argc) {
                params.saturation = std::stof(argv[++i]);
            }
        }
        else if (arg == "--serpentine") {
            params.serpentine = 1.0f;
        }
        else if (arg == "--seed") {
            if (i + 1 < argc) {
                params.seed = std::stoi(argv[++i]);
            }
        }
        else if (inputFile.empty()) {
            inputFile = arg;
        }
        else if (outputFile.empty()) {
            outputFile = arg;
        }
    }

    if (inputFile.empty() || outputFile.empty()) {
        std::cerr << "Error: Input and output files are required\n";
        printUsage(argv[0]);
        return 1;
    }

    // Load image
    std::cout << "Loading " << inputFile << "...\n";
    cv::Mat input = cv::imread(inputFile, cv::IMREAD_COLOR);
    if (input.empty()) {
        std::cerr << "Error: Could not load image: " << inputFile << "\n";
        return 1;
    }

    std::cout << "Image size: " << input.cols << "x" << input.rows << "\n";
    std::cout << "Algorithm: " << Dithering::getAlgorithmName(params.algorithm) << "\n";
    std::cout << "Palette: " << Dithering::getPaletteModeName(params.paletteMode) << "\n";

    // Process image
    std::cout << "Processing...\n";
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat output = Dithering::ditherImage(input, params);
    auto end = std::chrono::high_resolution_clock::now();

    float elapsed = std::chrono::duration<float, std::milli>(end - start).count();
    std::cout << "Processing time: " << elapsed << " ms\n";

    // Save image
    std::cout << "Saving to " << outputFile << "...\n";
    if (!cv::imwrite(outputFile, output)) {
        std::cerr << "Error: Could not save image: " << outputFile << "\n";
        return 1;
    }

    std::cout << "Done!\n";
    return 0;
}
