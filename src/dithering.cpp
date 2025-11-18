#include "dithering.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace Dithering {

// Helper function to apply preprocessing (gamma, contrast, brightness, saturation)
cv::Mat preprocessImage(const cv::Mat& input, const Parameters& params) {
    cv::Mat processed = input.clone();

    // Convert to float for processing
    processed.convertTo(processed, CV_32FC3, 1.0/255.0);

    // Brightness and contrast
    processed = processed * params.contrast + params.brightness;

    // Gamma correction
    if (params.gamma != 1.0f) {
        cv::pow(processed, params.gamma, processed);
    }

    // Saturation
    if (params.saturation != 1.0f) {
        cv::Mat hsv;
        cv::cvtColor(processed, hsv, cv::COLOR_BGR2HSV);
        std::vector<cv::Mat> channels;
        cv::split(hsv, channels);
        channels[1] = channels[1] * params.saturation;
        cv::merge(channels, hsv);
        cv::cvtColor(hsv, processed, cv::COLOR_HSV2BGR);
    }

    // Clamp and convert back
    processed = cv::max(cv::min(processed, 1.0), 0.0);
    processed.convertTo(processed, CV_8UC3, 255.0);

    return processed;
}

// Main dithering function dispatcher
cv::Mat ditherImage(const cv::Mat& input, const Parameters& params) {
    cv::Mat preprocessed = preprocessImage(input, params);

    switch (params.algorithm) {
        case Algorithm::FLOYD_STEINBERG:
            return floydSteinberg(preprocessed, params);
        case Algorithm::ATKINSON:
            return atkinson(preprocessed, params);
        case Algorithm::JARVIS_JUDICE_NINKE:
            return jarvisJudiceNinke(preprocessed, params);
        case Algorithm::STUCKI:
            return stucki(preprocessed, params);
        case Algorithm::BURKES:
            return burkes(preprocessed, params);
        case Algorithm::SIERRA:
            return sierra(preprocessed, params);
        case Algorithm::SIERRA_TWO_ROW:
            return sierraTwo(preprocessed, params);
        case Algorithm::SIERRA_LITE:
            return sierraLite(preprocessed, params);
        case Algorithm::ORDERED_BAYER_2X2:
        case Algorithm::ORDERED_BAYER_4X4:
        case Algorithm::ORDERED_BAYER_8X8:
        case Algorithm::ORDERED_BAYER_16X16:
            return orderedDither(preprocessed, params);
        case Algorithm::BLUE_NOISE:
            return blueNoiseDither(preprocessed, params);
        case Algorithm::WHITE_NOISE:
            return whiteNoiseDither(preprocessed, params);
        case Algorithm::RANDOM_DITHER:
            return randomDither(preprocessed, params);
        case Algorithm::PATTERN_DITHER:
            return patternDither(preprocessed, params);
        case Algorithm::DOT_DIFFUSION:
            return dotDiffusion(preprocessed, params);
        case Algorithm::RIEMERSMA:
            return riemersma(preprocessed, params);
        case Algorithm::GRADIENT_BASED:
            return gradientBased(preprocessed, params);
        case Algorithm::VARIABLE_ERROR_DIFFUSION:
            return variableErrorDiffusion(preprocessed, params);
        case Algorithm::OSTROMOUKHOV:
            return ostromoukhov(preprocessed, params);
        case Algorithm::FAN:
            return fan(preprocessed, params);
        case Algorithm::SHIAU_FAN:
            return shiauFan(preprocessed, params);
        case Algorithm::STEVENPIGEON:
            return stevenPigeon(preprocessed, params);
        default:
            return floydSteinberg(preprocessed, params);
    }
}

// Error diffusion helper
void diffuseError(cv::Mat& errors, int x, int y, const cv::Vec3f& error,
                  const std::vector<std::pair<int, int>>& offsets,
                  const std::vector<float>& weights, float strength, bool serpentine) {
    int direction = (serpentine && (y % 2 == 1)) ? -1 : 1;

    for (size_t i = 0; i < offsets.size(); ++i) {
        int nx = x + offsets[i].first * direction;
        int ny = y + offsets[i].second;

        if (nx >= 0 && nx < errors.cols && ny >= 0 && ny < errors.rows) {
            errors.at<cv::Vec3f>(ny, nx) += error * weights[i] * strength;
        }
    }
}

// Floyd-Steinberg dithering
cv::Mat floydSteinberg(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {{1, 0}, {-1, 1}, {0, 1}, {1, 1}};
    std::vector<float> weights = {7.0f/16.0f, 3.0f/16.0f, 5.0f/16.0f, 1.0f/16.0f};

    for (int y = 0; y < input.rows; ++y) {
        bool reverse = (params.serpentine > 0.5f) && (y % 2 == 1);
        int start = reverse ? input.cols - 1 : 0;
        int end = reverse ? -1 : input.cols;
        int step = reverse ? -1 : 1;

        for (int x = start; x != end; x += step) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, params.serpentine > 0.5f);
        }
    }

    return result;
}

// Atkinson dithering (used in early Mac systems)
cv::Mat atkinson(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {{1, 0}, {2, 0}, {-1, 1}, {0, 1}, {1, 1}, {0, 2}};
    std::vector<float> weights(6, 1.0f/8.0f);

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Jarvis-Judice-Ninke dithering
cv::Mat jarvisJudiceNinke(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {
        {1, 0}, {2, 0},
        {-2, 1}, {-1, 1}, {0, 1}, {1, 1}, {2, 1},
        {-2, 2}, {-1, 2}, {0, 2}, {1, 2}, {2, 2}
    };
    std::vector<float> weights = {
        7.0f/48.0f, 5.0f/48.0f,
        3.0f/48.0f, 5.0f/48.0f, 7.0f/48.0f, 5.0f/48.0f, 3.0f/48.0f,
        1.0f/48.0f, 3.0f/48.0f, 5.0f/48.0f, 3.0f/48.0f, 1.0f/48.0f
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Stucki dithering
cv::Mat stucki(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {
        {1, 0}, {2, 0},
        {-2, 1}, {-1, 1}, {0, 1}, {1, 1}, {2, 1},
        {-2, 2}, {-1, 2}, {0, 2}, {1, 2}, {2, 2}
    };
    std::vector<float> weights = {
        8.0f/42.0f, 4.0f/42.0f,
        2.0f/42.0f, 4.0f/42.0f, 8.0f/42.0f, 4.0f/42.0f, 2.0f/42.0f,
        1.0f/42.0f, 2.0f/42.0f, 4.0f/42.0f, 2.0f/42.0f, 1.0f/42.0f
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Burkes dithering
cv::Mat burkes(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {
        {1, 0}, {2, 0},
        {-2, 1}, {-1, 1}, {0, 1}, {1, 1}, {2, 1}
    };
    std::vector<float> weights = {
        8.0f/32.0f, 4.0f/32.0f,
        2.0f/32.0f, 4.0f/32.0f, 8.0f/32.0f, 4.0f/32.0f, 2.0f/32.0f
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Sierra dithering
cv::Mat sierra(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {
        {1, 0}, {2, 0},
        {-2, 1}, {-1, 1}, {0, 1}, {1, 1}, {2, 1},
        {-1, 2}, {0, 2}, {1, 2}
    };
    std::vector<float> weights = {
        5.0f/32.0f, 3.0f/32.0f,
        2.0f/32.0f, 4.0f/32.0f, 5.0f/32.0f, 4.0f/32.0f, 2.0f/32.0f,
        2.0f/32.0f, 3.0f/32.0f, 2.0f/32.0f
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Sierra Two-Row dithering
cv::Mat sierraTwo(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {
        {1, 0}, {2, 0},
        {-2, 1}, {-1, 1}, {0, 1}, {1, 1}, {2, 1}
    };
    std::vector<float> weights = {
        4.0f/16.0f, 3.0f/16.0f,
        1.0f/16.0f, 2.0f/16.0f, 3.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Sierra Lite dithering
cv::Mat sierraLite(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {
        {1, 0},
        {-1, 1}, {0, 1}
    };
    std::vector<float> weights = {
        2.0f/4.0f,
        1.0f/4.0f, 1.0f/4.0f
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Generate Bayer matrix
cv::Mat generateBayerMatrix(int size) {
    if (size == 2) {
        cv::Mat bayer = (cv::Mat_<float>(2, 2) << 0, 2, 3, 1);
        return bayer / 4.0f;
    }

    int halfSize = size / 2;
    cv::Mat smaller = generateBayerMatrix(halfSize);
    cv::Mat bayer = cv::Mat::zeros(size, size, CV_32F);

    for (int y = 0; y < halfSize; ++y) {
        for (int x = 0; x < halfSize; ++x) {
            float val = smaller.at<float>(y, x);
            bayer.at<float>(y, x) = 4 * val + 0;
            bayer.at<float>(y, x + halfSize) = 4 * val + 2;
            bayer.at<float>(y + halfSize, x) = 4 * val + 3;
            bayer.at<float>(y + halfSize, x + halfSize) = 4 * val + 1;
        }
    }

    return bayer / (size * size);
}

// Ordered dithering (Bayer matrix)
cv::Mat orderedDither(const cv::Mat& input, const Parameters& params) {
    int size = params.bayerSize;
    cv::Mat bayerMatrix = generateBayerMatrix(size);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);
    cv::Mat result = input.clone();

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            float threshold = bayerMatrix.at<float>(y % size, x % size);
            cv::Vec3b pixel = input.at<cv::Vec3b>(y, x);

            cv::Vec3f pixelF(pixel[0], pixel[1], pixel[2]);
            cv::Vec3f adjusted = pixelF + cv::Vec3f(threshold * 255.0f - 127.5f) * params.strength;

            adjusted = cv::Vec3f(
                std::clamp(adjusted[0], 0.0f, 255.0f),
                std::clamp(adjusted[1], 0.0f, 255.0f),
                std::clamp(adjusted[2], 0.0f, 255.0f)
            );

            cv::Vec3b adjustedPixel(
                static_cast<uchar>(adjusted[0]),
                static_cast<uchar>(adjusted[1]),
                static_cast<uchar>(adjusted[2])
            );

            result.at<cv::Vec3b>(y, x) = findClosestColor(adjustedPixel, palette);
        }
    }

    return result;
}

// Blue noise dithering
cv::Mat blueNoiseDither(const cv::Mat& input, const Parameters& params) {
    cv::Mat blueNoise = generateBlueNoiseTexture(256, params.seed);
    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);
    cv::Mat result = input.clone();

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            float noise = blueNoise.at<float>(y % 256, x % 256);
            cv::Vec3b pixel = input.at<cv::Vec3b>(y, x);

            cv::Vec3f pixelF(pixel[0], pixel[1], pixel[2]);
            cv::Vec3f adjusted = pixelF + cv::Vec3f(noise * 255.0f - 127.5f) * params.strength;

            adjusted = cv::Vec3f(
                std::clamp(adjusted[0], 0.0f, 255.0f),
                std::clamp(adjusted[1], 0.0f, 255.0f),
                std::clamp(adjusted[2], 0.0f, 255.0f)
            );

            cv::Vec3b adjustedPixel(
                static_cast<uchar>(adjusted[0]),
                static_cast<uchar>(adjusted[1]),
                static_cast<uchar>(adjusted[2])
            );

            result.at<cv::Vec3b>(y, x) = findClosestColor(adjustedPixel, palette);
        }
    }

    return result;
}

// White noise dithering
cv::Mat whiteNoiseDither(const cv::Mat& input, const Parameters& params) {
    std::mt19937 rng(params.seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);
    cv::Mat result = input.clone();

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            float noise = dist(rng);
            cv::Vec3b pixel = input.at<cv::Vec3b>(y, x);

            cv::Vec3f pixelF(pixel[0], pixel[1], pixel[2]);
            cv::Vec3f adjusted = pixelF + cv::Vec3f(noise * 255.0f - 127.5f) * params.strength;

            adjusted = cv::Vec3f(
                std::clamp(adjusted[0], 0.0f, 255.0f),
                std::clamp(adjusted[1], 0.0f, 255.0f),
                std::clamp(adjusted[2], 0.0f, 255.0f)
            );

            cv::Vec3b adjustedPixel(
                static_cast<uchar>(adjusted[0]),
                static_cast<uchar>(adjusted[1]),
                static_cast<uchar>(adjusted[2])
            );

            result.at<cv::Vec3b>(y, x) = findClosestColor(adjustedPixel, palette);
        }
    }

    return result;
}

// Random dithering
cv::Mat randomDither(const cv::Mat& input, const Parameters& params) {
    return whiteNoiseDither(input, params);
}

// Pattern dithering
cv::Mat patternDither(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    // Create a 4x4 pattern
    float pattern[4][4] = {
        {0.0f, 0.5f, 0.125f, 0.625f},
        {0.75f, 0.25f, 0.875f, 0.375f},
        {0.1875f, 0.6875f, 0.0625f, 0.5625f},
        {0.9375f, 0.4375f, 0.8125f, 0.3125f}
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            float threshold = pattern[y % 4][x % 4];
            cv::Vec3b pixel = input.at<cv::Vec3b>(y, x);

            cv::Vec3f pixelF(pixel[0], pixel[1], pixel[2]);
            cv::Vec3f adjusted = pixelF + cv::Vec3f(threshold * 255.0f - 127.5f) * params.strength;

            adjusted = cv::Vec3f(
                std::clamp(adjusted[0], 0.0f, 255.0f),
                std::clamp(adjusted[1], 0.0f, 255.0f),
                std::clamp(adjusted[2], 0.0f, 255.0f)
            );

            cv::Vec3b adjustedPixel(
                static_cast<uchar>(adjusted[0]),
                static_cast<uchar>(adjusted[1]),
                static_cast<uchar>(adjusted[2])
            );

            result.at<cv::Vec3b>(y, x) = findClosestColor(adjustedPixel, palette);
        }
    }

    return result;
}

// Dot diffusion dithering
cv::Mat dotDiffusion(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    // 8x8 class matrix for dot diffusion
    int classMatrix[8][8] = {
        {39, 23, 15, 31, 38, 22, 14, 30},
        {24,  7,  1,  9, 25,  8,  2, 10},
        {16,  3, 47, 43, 17,  4, 48, 44},
        {32, 11, 41, 27, 33, 12, 42, 28},
        {37, 21, 13, 29, 40, 26, 18, 34},
        {26,  6,  0,  8, 27,  5, 61, 13},
        {19,  2, 46, 42, 20,  1, 49, 45},
        {35, 10, 40, 26, 36,  9, 43, 25}
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            int classVal = classMatrix[y % 8][x % 8];
            float threshold = classVal / 64.0f;

            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF += cv::Vec3f(threshold * 128.0f - 64.0f) * params.strength;

            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;
        }
    }

    return result;
}

// Riemersma dithering (space-filling curve)
cv::Mat riemersma(const cv::Mat& input, const Parameters& params) {
    // Simplified Riemersma using Hilbert curve approximation
    cv::Mat result = input.clone();
    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    // Use a simple serpentine scan as approximation
    cv::Vec3f error(0, 0, 0);
    const float decay = 0.8f;

    for (int y = 0; y < input.rows; ++y) {
        bool reverse = (y % 2 == 1);
        int start = reverse ? input.cols - 1 : 0;
        int end = reverse ? -1 : input.cols;
        int step = reverse ? -1 : 1;

        for (int x = start; x != end; x += step) {
            cv::Vec3b pixel = input.at<cv::Vec3b>(y, x);
            cv::Vec3f pixelF(pixel[0], pixel[1], pixel[2]);
            pixelF += error * params.strength;

            pixelF = cv::Vec3f(
                std::clamp(pixelF[0], 0.0f, 255.0f),
                std::clamp(pixelF[1], 0.0f, 255.0f),
                std::clamp(pixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b pixelB(
                static_cast<uchar>(pixelF[0]),
                static_cast<uchar>(pixelF[1]),
                static_cast<uchar>(pixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(pixelB, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            error = (pixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2])) * decay;
        }
    }

    return result;
}

// Gradient-based dithering
cv::Mat gradientBased(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    // Calculate gradients
    cv::Mat gray, gradX, gradY, gradient;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    cv::Sobel(gray, gradX, CV_32F, 1, 0);
    cv::Sobel(gray, gradY, CV_32F, 0, 1);
    cv::magnitude(gradX, gradY, gradient);
    cv::normalize(gradient, gradient, 0, 1, cv::NORM_MINMAX);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            float gradWeight = gradient.at<float>(y, x);
            float adaptiveStrength = params.strength * (0.5f + gradWeight * 0.5f);

            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);

            // Use Floyd-Steinberg-like diffusion with adaptive strength
            std::vector<std::pair<int, int>> offsets = {{1, 0}, {-1, 1}, {0, 1}, {1, 1}};
            std::vector<float> weights = {7.0f/16.0f, 3.0f/16.0f, 5.0f/16.0f, 1.0f/16.0f};
            diffuseError(errors, x, y, quantError, offsets, weights, adaptiveStrength, false);
        }
    }

    return result;
}

// Variable error diffusion
cv::Mat variableErrorDiffusion(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);
    std::mt19937 rng(params.seed);
    std::uniform_real_distribution<float> dist(0.7f, 1.3f);

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);

            // Variable weights
            float var = dist(rng);
            std::vector<std::pair<int, int>> offsets = {{1, 0}, {-1, 1}, {0, 1}, {1, 1}};
            std::vector<float> weights = {
                7.0f/16.0f * var,
                3.0f/16.0f * var,
                5.0f/16.0f * var,
                1.0f/16.0f * var
            };
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Ostromoukhov dithering
cv::Mat ostromoukhov(const cv::Mat& input, const Parameters& params) {
    // Simplified Ostromoukhov (using adaptive weights based on intensity)
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            // Adaptive weights based on intensity
            float intensity = (newPixelF[0] + newPixelF[1] + newPixelF[2]) / (3.0f * 255.0f);
            float w1 = 7.0f * (1.0f - intensity) + 3.0f * intensity;
            float w2 = 3.0f * (1.0f - intensity) + 7.0f * intensity;
            float w3 = 5.0f;
            float w4 = 1.0f;
            float sum = w1 + w2 + w3 + w4;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            std::vector<std::pair<int, int>> offsets = {{1, 0}, {-1, 1}, {0, 1}, {1, 1}};
            std::vector<float> weights = {w1/sum, w2/sum, w3/sum, w4/sum};
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Fan dithering
cv::Mat fan(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {{1, 0}, {0, 1}, {1, 1}, {-1, 1}};
    std::vector<float> weights = {7.0f/16.0f, 1.0f/16.0f, 5.0f/16.0f, 3.0f/16.0f};

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Shiau-Fan dithering
cv::Mat shiauFan(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {
        {1, 0}, {2, 0},
        {-2, 1}, {-1, 1}, {0, 1}, {1, 1}, {2, 1}
    };
    std::vector<float> weights = {
        4.0f/16.0f, 1.0f/16.0f,
        1.0f/16.0f, 1.0f/16.0f, 2.0f/16.0f, 4.0f/16.0f, 2.0f/16.0f
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Steven Pigeon dithering
cv::Mat stevenPigeon(const cv::Mat& input, const Parameters& params) {
    cv::Mat result = input.clone();
    cv::Mat errors = cv::Mat::zeros(input.rows, input.cols, CV_32FC3);

    std::vector<cv::Vec3b> palette = getPalette(params.paletteMode);

    std::vector<std::pair<int, int>> offsets = {
        {1, 0}, {2, 0},
        {-2, 1}, {-1, 1}, {0, 1}, {1, 1}, {2, 1},
        {-1, 2}, {0, 2}, {1, 2}
    };
    std::vector<float> weights = {
        2.0f/14.0f, 1.0f/14.0f,
        1.0f/14.0f, 2.0f/14.0f, 2.0f/14.0f, 2.0f/14.0f, 1.0f/14.0f,
        1.0f/14.0f, 1.0f/14.0f, 1.0f/14.0f
    };

    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b oldPixel = result.at<cv::Vec3b>(y, x);
            cv::Vec3f errorVal = errors.at<cv::Vec3f>(y, x);

            cv::Vec3f newPixelF = cv::Vec3f(oldPixel[0], oldPixel[1], oldPixel[2]) + errorVal;
            newPixelF = cv::Vec3f(
                std::clamp(newPixelF[0], 0.0f, 255.0f),
                std::clamp(newPixelF[1], 0.0f, 255.0f),
                std::clamp(newPixelF[2], 0.0f, 255.0f)
            );

            cv::Vec3b newPixel(
                static_cast<uchar>(newPixelF[0]),
                static_cast<uchar>(newPixelF[1]),
                static_cast<uchar>(newPixelF[2])
            );

            cv::Vec3b quantized = findClosestColor(newPixel, palette);
            result.at<cv::Vec3b>(y, x) = quantized;

            cv::Vec3f quantError = newPixelF - cv::Vec3f(quantized[0], quantized[1], quantized[2]);
            diffuseError(errors, x, y, quantError, offsets, weights, params.strength, false);
        }
    }

    return result;
}

// Generate blue noise texture
cv::Mat generateBlueNoiseTexture(int size, unsigned int seed) {
    // Simplified blue noise generation using void-and-cluster method approximation
    cv::Mat noise = cv::Mat::zeros(size, size, CV_32F);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Start with white noise
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            noise.at<float>(y, x) = dist(rng);
        }
    }

    // Apply Gaussian blur to create more uniform distribution
    cv::GaussianBlur(noise, noise, cv::Size(5, 5), 1.0);
    cv::normalize(noise, noise, 0, 1, cv::NORM_MINMAX);

    return noise;
}

// Get palette based on mode
std::vector<cv::Vec3b> getPalette(PaletteMode mode) {
    std::vector<cv::Vec3b> palette;

    switch (mode) {
        case PaletteMode::MONOCHROME:
            palette.push_back(cv::Vec3b(0, 0, 0));
            palette.push_back(cv::Vec3b(255, 255, 255));
            break;

        case PaletteMode::GRAYSCALE_4:
            for (int i = 0; i < 4; ++i) {
                uchar val = static_cast<uchar>(i * 255 / 3);
                palette.push_back(cv::Vec3b(val, val, val));
            }
            break;

        case PaletteMode::GRAYSCALE_8:
            for (int i = 0; i < 8; ++i) {
                uchar val = static_cast<uchar>(i * 255 / 7);
                palette.push_back(cv::Vec3b(val, val, val));
            }
            break;

        case PaletteMode::GRAYSCALE_16:
            for (int i = 0; i < 16; ++i) {
                uchar val = static_cast<uchar>(i * 255 / 15);
                palette.push_back(cv::Vec3b(val, val, val));
            }
            break;

        case PaletteMode::CGA:
            // CGA 16-color palette
            palette = {
                cv::Vec3b(0, 0, 0), cv::Vec3b(170, 0, 0), cv::Vec3b(0, 170, 0), cv::Vec3b(170, 170, 0),
                cv::Vec3b(0, 0, 170), cv::Vec3b(170, 0, 170), cv::Vec3b(0, 85, 170), cv::Vec3b(170, 170, 170),
                cv::Vec3b(85, 85, 85), cv::Vec3b(255, 85, 85), cv::Vec3b(85, 255, 85), cv::Vec3b(255, 255, 85),
                cv::Vec3b(85, 85, 255), cv::Vec3b(255, 85, 255), cv::Vec3b(85, 255, 255), cv::Vec3b(255, 255, 255)
            };
            break;

        case PaletteMode::GAMEBOY:
            // Game Boy 4-color palette
            palette = {
                cv::Vec3b(15, 56, 15),
                cv::Vec3b(48, 98, 48),
                cv::Vec3b(139, 172, 15),
                cv::Vec3b(155, 188, 15)
            };
            break;

        case PaletteMode::PICO8:
            // PICO-8 16-color palette
            palette = {
                cv::Vec3b(0, 0, 0), cv::Vec3b(95, 87, 79), cv::Vec3b(255, 0, 77), cv::Vec3b(171, 82, 54),
                cv::Vec3b(255, 163, 0), cv::Vec3b(255, 236, 39), cv::Vec3b(0, 228, 54), cv::Vec3b(41, 173, 255),
                cv::Vec3b(131, 118, 156), cv::Vec3b(255, 119, 168), cv::Vec3b(255, 204, 170), cv::Vec3b(41, 54, 111),
                cv::Vec3b(0, 87, 132), cv::Vec3b(194, 195, 199), cv::Vec3b(255, 241, 232), cv::Vec3b(242, 233, 222)
            };
            break;

        default:
            palette.push_back(cv::Vec3b(0, 0, 0));
            palette.push_back(cv::Vec3b(255, 255, 255));
            break;
    }

    return palette;
}

// Find closest color in palette
cv::Vec3b findClosestColor(const cv::Vec3b& color, const std::vector<cv::Vec3b>& palette) {
    float minDist = std::numeric_limits<float>::max();
    cv::Vec3b closest = palette[0];

    for (const auto& paletteColor : palette) {
        float dist = std::pow(color[0] - paletteColor[0], 2) +
                     std::pow(color[1] - paletteColor[1], 2) +
                     std::pow(color[2] - paletteColor[2], 2);

        if (dist < minDist) {
            minDist = dist;
            closest = paletteColor;
        }
    }

    return closest;
}

// Get algorithm name
std::string getAlgorithmName(Algorithm algo) {
    switch (algo) {
        case Algorithm::FLOYD_STEINBERG: return "Floyd-Steinberg";
        case Algorithm::ATKINSON: return "Atkinson";
        case Algorithm::JARVIS_JUDICE_NINKE: return "Jarvis-Judice-Ninke";
        case Algorithm::STUCKI: return "Stucki";
        case Algorithm::BURKES: return "Burkes";
        case Algorithm::SIERRA: return "Sierra";
        case Algorithm::SIERRA_TWO_ROW: return "Sierra Two-Row";
        case Algorithm::SIERRA_LITE: return "Sierra Lite";
        case Algorithm::ORDERED_BAYER_2X2: return "Ordered Bayer 2x2";
        case Algorithm::ORDERED_BAYER_4X4: return "Ordered Bayer 4x4";
        case Algorithm::ORDERED_BAYER_8X8: return "Ordered Bayer 8x8";
        case Algorithm::ORDERED_BAYER_16X16: return "Ordered Bayer 16x16";
        case Algorithm::BLUE_NOISE: return "Blue Noise";
        case Algorithm::WHITE_NOISE: return "White Noise";
        case Algorithm::RANDOM_DITHER: return "Random";
        case Algorithm::PATTERN_DITHER: return "Pattern";
        case Algorithm::DOT_DIFFUSION: return "Dot Diffusion";
        case Algorithm::RIEMERSMA: return "Riemersma";
        case Algorithm::GRADIENT_BASED: return "Gradient-Based";
        case Algorithm::VARIABLE_ERROR_DIFFUSION: return "Variable Error Diffusion";
        case Algorithm::OSTROMOUKHOV: return "Ostromoukhov";
        case Algorithm::FAN: return "Fan";
        case Algorithm::SHIAU_FAN: return "Shiau-Fan";
        case Algorithm::STEVENPIGEON: return "Steven Pigeon";
        default: return "Unknown";
    }
}

// Get palette mode name
std::string getPaletteModeName(PaletteMode mode) {
    switch (mode) {
        case PaletteMode::MONOCHROME: return "Monochrome";
        case PaletteMode::GRAYSCALE_4: return "Grayscale 4";
        case PaletteMode::GRAYSCALE_8: return "Grayscale 8";
        case PaletteMode::GRAYSCALE_16: return "Grayscale 16";
        case PaletteMode::CGA: return "CGA";
        case PaletteMode::EGA: return "EGA";
        case PaletteMode::VGA: return "VGA";
        case PaletteMode::GAMEBOY: return "Game Boy";
        case PaletteMode::PICO8: return "PICO-8";
        case PaletteMode::CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

} // namespace Dithering
