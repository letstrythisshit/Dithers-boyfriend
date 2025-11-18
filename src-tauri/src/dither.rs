use image::{DynamicImage, GenericImageView, ImageBuffer, Rgb, Rgba};
use rayon::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum DitheringAlgorithm {
    FloydSteinberg,
    Atkinson,
    JarvisJudiceNinke,
    Stucki,
    Burkes,
    Sierra,
    SierraTwoRow,
    SierraLite,
    Bayer2x2,
    Bayer4x4,
    Bayer8x8,
    Bayer16x16,
    BlueNoise,
    WhiteNoise,
    SimpleThreshold,
    RandomThreshold,
    Pattern,
    ClusteredDot,
    HalftoneCircle,
    HalftoneDiamond,
    Riemersma,
    FalseFloydSteinberg,
    StevenPigeon,
    GradientBased,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DitheringSettings {
    pub algorithm: DitheringAlgorithm,
    pub colors: usize,          // Number of colors per channel
    pub threshold: f32,         // Threshold value (0.0-1.0)
    pub error_diffusion: f32,   // Error diffusion strength (0.0-1.0)
    pub pattern_scale: u32,     // Pattern scale for pattern dithering
    pub serpentine: bool,       // Use serpentine scanning
    pub color_mode: ColorMode,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum ColorMode {
    Monochrome,
    Grayscale,
    FullColor,
    CustomPalette(Vec<[u8; 3]>),
}

impl Default for DitheringSettings {
    fn default() -> Self {
        Self {
            algorithm: DitheringAlgorithm::FloydSteinberg,
            colors: 2,
            threshold: 0.5,
            error_diffusion: 1.0,
            pattern_scale: 2,
            serpentine: true,
            color_mode: ColorMode::Monochrome,
        }
    }
}

pub fn apply_dithering(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    match settings.algorithm {
        DitheringAlgorithm::FloydSteinberg => floyd_steinberg(img, settings),
        DitheringAlgorithm::Atkinson => atkinson(img, settings),
        DitheringAlgorithm::JarvisJudiceNinke => jarvis_judice_ninke(img, settings),
        DitheringAlgorithm::Stucki => stucki(img, settings),
        DitheringAlgorithm::Burkes => burkes(img, settings),
        DitheringAlgorithm::Sierra => sierra(img, settings),
        DitheringAlgorithm::SierraTwoRow => sierra_two_row(img, settings),
        DitheringAlgorithm::SierraLite => sierra_lite(img, settings),
        DitheringAlgorithm::Bayer2x2 => bayer(img, settings, 2),
        DitheringAlgorithm::Bayer4x4 => bayer(img, settings, 4),
        DitheringAlgorithm::Bayer8x8 => bayer(img, settings, 8),
        DitheringAlgorithm::Bayer16x16 => bayer(img, settings, 16),
        DitheringAlgorithm::BlueNoise => blue_noise(img, settings),
        DitheringAlgorithm::WhiteNoise => white_noise(img, settings),
        DitheringAlgorithm::SimpleThreshold => simple_threshold(img, settings),
        DitheringAlgorithm::RandomThreshold => random_threshold(img, settings),
        DitheringAlgorithm::Pattern => pattern_dither(img, settings),
        DitheringAlgorithm::ClusteredDot => clustered_dot(img, settings),
        DitheringAlgorithm::HalftoneCircle => halftone_circle(img, settings),
        DitheringAlgorithm::HalftoneDiamond => halftone_diamond(img, settings),
        DitheringAlgorithm::Riemersma => riemersma(img, settings),
        DitheringAlgorithm::FalseFloydSteinberg => false_floyd_steinberg(img, settings),
        DitheringAlgorithm::StevenPigeon => steven_pigeon(img, settings),
        DitheringAlgorithm::GradientBased => gradient_based(img, settings),
    }
}

fn quantize_color(value: f32, levels: usize) -> u8 {
    let level = (value * (levels - 1) as f32).round();
    ((level / (levels - 1) as f32) * 255.0) as u8
}

fn find_nearest_color(color: [f32; 3], levels: usize) -> [u8; 3] {
    [
        quantize_color(color[0], levels),
        quantize_color(color[1], levels),
        quantize_color(color[2], levels),
    ]
}

// Floyd-Steinberg dithering (1976)
fn floyd_steinberg(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 7.0 / 16.0),
            (-1, 1, 3.0 / 16.0),
            (0, 1, 5.0 / 16.0),
            (1, 1, 1.0 / 16.0),
        ],
    )
}

// Atkinson dithering (1984) - Used in classic Macintosh
fn atkinson(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 1.0 / 8.0),
            (2, 0, 1.0 / 8.0),
            (-1, 1, 1.0 / 8.0),
            (0, 1, 1.0 / 8.0),
            (1, 1, 1.0 / 8.0),
            (0, 2, 1.0 / 8.0),
        ],
    )
}

// Jarvis-Judice-Ninke (1976)
fn jarvis_judice_ninke(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 7.0 / 48.0),
            (2, 0, 5.0 / 48.0),
            (-2, 1, 3.0 / 48.0),
            (-1, 1, 5.0 / 48.0),
            (0, 1, 7.0 / 48.0),
            (1, 1, 5.0 / 48.0),
            (2, 1, 3.0 / 48.0),
            (-2, 2, 1.0 / 48.0),
            (-1, 2, 3.0 / 48.0),
            (0, 2, 5.0 / 48.0),
            (1, 2, 3.0 / 48.0),
            (2, 2, 1.0 / 48.0),
        ],
    )
}

// Stucki (1981)
fn stucki(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 8.0 / 42.0),
            (2, 0, 4.0 / 42.0),
            (-2, 1, 2.0 / 42.0),
            (-1, 1, 4.0 / 42.0),
            (0, 1, 8.0 / 42.0),
            (1, 1, 4.0 / 42.0),
            (2, 1, 2.0 / 42.0),
            (-2, 2, 1.0 / 42.0),
            (-1, 2, 2.0 / 42.0),
            (0, 2, 4.0 / 42.0),
            (1, 2, 2.0 / 42.0),
            (2, 2, 1.0 / 42.0),
        ],
    )
}

// Burkes (1988)
fn burkes(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 8.0 / 32.0),
            (2, 0, 4.0 / 32.0),
            (-2, 1, 2.0 / 32.0),
            (-1, 1, 4.0 / 32.0),
            (0, 1, 8.0 / 32.0),
            (1, 1, 4.0 / 32.0),
            (2, 1, 2.0 / 32.0),
        ],
    )
}

// Sierra (1989)
fn sierra(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 5.0 / 32.0),
            (2, 0, 3.0 / 32.0),
            (-2, 1, 2.0 / 32.0),
            (-1, 1, 4.0 / 32.0),
            (0, 1, 5.0 / 32.0),
            (1, 1, 4.0 / 32.0),
            (2, 1, 2.0 / 32.0),
            (-1, 2, 2.0 / 32.0),
            (0, 2, 3.0 / 32.0),
            (1, 2, 2.0 / 32.0),
        ],
    )
}

// Sierra Two-Row (1990)
fn sierra_two_row(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 4.0 / 16.0),
            (2, 0, 3.0 / 16.0),
            (-2, 1, 1.0 / 16.0),
            (-1, 1, 2.0 / 16.0),
            (0, 1, 3.0 / 16.0),
            (1, 1, 2.0 / 16.0),
            (2, 1, 1.0 / 16.0),
        ],
    )
}

// Sierra Lite (Filter Lite)
fn sierra_lite(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 2.0 / 4.0),
            (-1, 1, 1.0 / 4.0),
            (0, 1, 1.0 / 4.0),
        ],
    )
}

// False Floyd-Steinberg
fn false_floyd_steinberg(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 3.0 / 8.0),
            (0, 1, 3.0 / 8.0),
            (1, 1, 2.0 / 8.0),
        ],
    )
}

// Steven Pigeon's algorithm
fn steven_pigeon(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    error_diffusion_generic(
        img,
        settings,
        vec![
            (1, 0, 2.0 / 8.0),
            (2, 0, 1.0 / 8.0),
            (-1, 1, 1.0 / 8.0),
            (0, 1, 2.0 / 8.0),
            (1, 1, 2.0 / 8.0),
        ],
    )
}

// Generic error diffusion implementation
fn error_diffusion_generic(
    img: &DynamicImage,
    settings: &DitheringSettings,
    kernel: Vec<(i32, i32, f32)>,
) -> DynamicImage {
    let (width, height) = img.dimensions();
    let mut buffer = img.to_rgb8();
    let mut error_buffer: Vec<Vec<[f32; 3]>> = vec![vec![[0.0; 3]; width as usize]; height as usize];

    for y in 0..height {
        let range: Box<dyn Iterator<Item = u32>> = if settings.serpentine && y % 2 == 1 {
            Box::new((0..width).rev())
        } else {
            Box::new(0..width)
        };

        for x in range {
            let pixel = buffer.get_pixel(x, y);
            let mut color = [
                pixel[0] as f32 / 255.0 + error_buffer[y as usize][x as usize][0],
                pixel[1] as f32 / 255.0 + error_buffer[y as usize][x as usize][1],
                pixel[2] as f32 / 255.0 + error_buffer[y as usize][x as usize][2],
            ];

            // Clamp
            for c in &mut color {
                *c = c.clamp(0.0, 1.0);
            }

            let new_color = find_nearest_color(color, settings.colors);
            buffer.put_pixel(x, y, Rgb(new_color));

            // Calculate error
            let error = [
                color[0] - new_color[0] as f32 / 255.0,
                color[1] - new_color[1] as f32 / 255.0,
                color[2] - new_color[2] as f32 / 255.0,
            ];

            // Diffuse error
            for (dx, dy, weight) in &kernel {
                let (nx, ny) = if settings.serpentine && y % 2 == 1 {
                    (x as i32 - dx, y as i32 + dy)
                } else {
                    (x as i32 + dx, y as i32 + dy)
                };

                if nx >= 0 && nx < width as i32 && ny >= 0 && ny < height as i32 {
                    let w = weight * settings.error_diffusion;
                    error_buffer[ny as usize][nx as usize][0] += error[0] * w;
                    error_buffer[ny as usize][nx as usize][1] += error[1] * w;
                    error_buffer[ny as usize][nx as usize][2] += error[2] * w;
                }
            }
        }
    }

    DynamicImage::ImageRgb8(buffer)
}

// Bayer matrix dithering (ordered dithering)
fn bayer(img: &DynamicImage, settings: &DitheringSettings, size: usize) -> DynamicImage {
    let matrix = generate_bayer_matrix(size);
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);
        let threshold = matrix[y as usize % size][x as usize % size];

        let dithered = [
            if pixel[0] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[1] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[2] as f32 / 255.0 > threshold { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}

fn generate_bayer_matrix(size: usize) -> Vec<Vec<f32>> {
    let n = (size as f32).log2() as usize;
    let mut matrix = vec![vec![0.0; size]; size];

    if size == 2 {
        matrix[0][0] = 0.0 / 4.0;
        matrix[0][1] = 2.0 / 4.0;
        matrix[1][0] = 3.0 / 4.0;
        matrix[1][1] = 1.0 / 4.0;
    } else {
        // Recursive generation for larger matrices
        for i in 0..size {
            for j in 0..size {
                let mut value = 0.0;
                let mut x = i;
                let mut y = j;
                let mut mask = size / 2;

                while mask > 0 {
                    value = value * 4.0 + (if x & mask != 0 { 2.0 } else { 0.0 })
                                        + (if y & mask != 0 { 1.0 } else { 0.0 });
                    mask >>= 1;
                }

                matrix[i][j] = (value + 0.5) / (size * size) as f32;
            }
        }
    }

    matrix
}

// Blue noise dithering
fn blue_noise(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();
    let noise = generate_blue_noise(width as usize, height as usize);

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);
        let threshold = noise[y as usize][x as usize] * settings.threshold;

        let dithered = [
            if pixel[0] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[1] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[2] as f32 / 255.0 > threshold { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}

fn generate_blue_noise(width: usize, height: usize) -> Vec<Vec<f32>> {
    use std::collections::HashSet;
    let mut noise = vec![vec![0.0; width]; height];
    let mut used = HashSet::new();

    // Simple blue noise approximation using dart throwing
    let total_pixels = width * height;
    let mut rng_state = 12345u64;

    for intensity in 0..256 {
        let target = (total_pixels * intensity / 256).min(total_pixels - 1);
        let mut attempts = 0;

        while used.len() < target && attempts < total_pixels * 2 {
            let x = (lcg_rand(&mut rng_state) % width as u64) as usize;
            let y = (lcg_rand(&mut rng_state) % height as u64) as usize;
            let idx = y * width + x;

            if !used.contains(&idx) {
                noise[y][x] = intensity as f32 / 255.0;
                used.insert(idx);
            }
            attempts += 1;
        }
    }

    noise
}

// White noise dithering
fn white_noise(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();
    let mut rng_state = 42u64;

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);
        let threshold = (lcg_rand(&mut rng_state) as f32 / u64::MAX as f32) * settings.threshold;

        let dithered = [
            if pixel[0] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[1] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[2] as f32 / 255.0 > threshold { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}

// Simple LCG random number generator
fn lcg_rand(state: &mut u64) -> u64 {
    *state = state.wrapping_mul(6364136223846793005).wrapping_add(1);
    *state
}

// Simple threshold
fn simple_threshold(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);
        let threshold = settings.threshold;

        let dithered = [
            if pixel[0] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[1] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[2] as f32 / 255.0 > threshold { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}

// Random threshold
fn random_threshold(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();
    let mut rng_state = 99u64;

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);

        let dithered = [
            if (pixel[0] as f32 / 255.0) > ((lcg_rand(&mut rng_state) as f32 / u64::MAX as f32) * settings.threshold) { 255 } else { 0 },
            if (pixel[1] as f32 / 255.0) > ((lcg_rand(&mut rng_state) as f32 / u64::MAX as f32) * settings.threshold) { 255 } else { 0 },
            if (pixel[2] as f32 / 255.0) > ((lcg_rand(&mut rng_state) as f32 / u64::MAX as f32) * settings.threshold) { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}

// Pattern dithering
fn pattern_dither(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();
    let scale = settings.pattern_scale as usize;

    // 4x4 pattern matrix
    let pattern = [
        [0, 8, 2, 10],
        [12, 4, 14, 6],
        [3, 11, 1, 9],
        [15, 7, 13, 5],
    ];

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);
        let threshold = pattern[(y as usize / scale) % 4][(x as usize / scale) % 4] as f32 / 16.0;

        let dithered = [
            if pixel[0] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[1] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[2] as f32 / 255.0 > threshold { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}

// Clustered dot dithering
fn clustered_dot(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();

    // 4x4 clustered dot pattern
    let pattern = [
        [12, 5, 6, 13],
        [4, 0, 1, 7],
        [11, 3, 2, 8],
        [15, 10, 9, 14],
    ];

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);
        let threshold = pattern[y as usize % 4][x as usize % 4] as f32 / 16.0;

        let dithered = [
            if pixel[0] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[1] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[2] as f32 / 255.0 > threshold { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}

// Halftone circle pattern
fn halftone_circle(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();
    let scale = settings.pattern_scale.max(4) as f32;

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);

        let cell_x = (x as f32 % scale) - scale / 2.0;
        let cell_y = (y as f32 % scale) - scale / 2.0;
        let distance = (cell_x * cell_x + cell_y * cell_y).sqrt() / (scale / 2.0);

        let dithered = [
            if (pixel[0] as f32 / 255.0) > distance { 255 } else { 0 },
            if (pixel[1] as f32 / 255.0) > distance { 255 } else { 0 },
            if (pixel[2] as f32 / 255.0) > distance { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}

// Halftone diamond pattern
fn halftone_diamond(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();
    let scale = settings.pattern_scale.max(4) as f32;

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);

        let cell_x = (x as f32 % scale) - scale / 2.0;
        let cell_y = (y as f32 % scale) - scale / 2.0;
        let distance = (cell_x.abs() + cell_y.abs()) / scale;

        let dithered = [
            if (pixel[0] as f32 / 255.0) > distance { 255 } else { 0 },
            if (pixel[1] as f32 / 255.0) > distance { 255 } else { 0 },
            if (pixel[2] as f32 / 255.0) > distance { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}

// Riemersma dithering (space-filling curve)
fn riemersma(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let mut buffer = img.to_rgb8();

    // Hilbert curve traversal
    let max_level = ((width.max(height) as f32).log2().ceil() as usize).max(1);
    let size = 1 << max_level;

    let mut error = [0.0f32; 3];
    let decay = 0.7; // Error decay factor

    for i in 0..(size * size) {
        let (x, y) = hilbert_index_to_xy(i, max_level);
        if x >= width || y >= height {
            continue;
        }

        let pixel = buffer.get_pixel(x, y);
        let mut color = [
            (pixel[0] as f32 / 255.0 + error[0]).clamp(0.0, 1.0),
            (pixel[1] as f32 / 255.0 + error[1]).clamp(0.0, 1.0),
            (pixel[2] as f32 / 255.0 + error[2]).clamp(0.0, 1.0),
        ];

        let new_color = find_nearest_color(color, settings.colors);
        buffer.put_pixel(x, y, Rgb(new_color));

        // Update error
        for c in 0..3 {
            error[c] = (color[c] - new_color[c] as f32 / 255.0) * decay * settings.error_diffusion;
        }
    }

    DynamicImage::ImageRgb8(buffer)
}

// Hilbert curve coordinate mapping
fn hilbert_index_to_xy(index: usize, level: usize) -> (u32, u32) {
    let mut x = 0;
    let mut y = 0;
    let mut s = 1;

    for i in 0..level {
        let rx = 1 & (index >> (2 * i));
        let ry = 1 & (index >> (2 * i + 1));

        if ry == 0 {
            if rx == 1 {
                x = s - 1 - x;
                y = s - 1 - y;
            }
            std::mem::swap(&mut x, &mut y);
        }

        x += rx * s;
        y += ry * s;
        s *= 2;
    }

    (x as u32, y as u32)
}

// Gradient-based dithering
fn gradient_based(img: &DynamicImage, settings: &DitheringSettings) -> DynamicImage {
    let (width, height) = img.dimensions();
    let rgb = img.to_rgb8();

    let buffer: ImageBuffer<Rgb<u8>, Vec<u8>> = ImageBuffer::from_fn(width, height, |x, y| {
        let pixel = rgb.get_pixel(x, y);

        // Calculate gradient
        let gx = if x > 0 && x < width - 1 {
            let left = rgb.get_pixel(x - 1, y);
            let right = rgb.get_pixel(x + 1, y);
            ((right[0] as i32 - left[0] as i32).abs() +
             (right[1] as i32 - left[1] as i32).abs() +
             (right[2] as i32 - left[2] as i32).abs()) as f32 / 765.0
        } else {
            0.0
        };

        let threshold = settings.threshold * (1.0 - gx * 0.5);

        let dithered = [
            if pixel[0] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[1] as f32 / 255.0 > threshold { 255 } else { 0 },
            if pixel[2] as f32 / 255.0 > threshold { 255 } else { 0 },
        ];

        Rgb(dithered)
    });

    DynamicImage::ImageRgb8(buffer)
}
