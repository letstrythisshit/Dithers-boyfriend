# Dither's Boyfriend 🎨

A professional, cross-platform dithering application supporting **24 unique dithering algorithms** for both images and videos. Built with Tauri + React for maximum performance and portability.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)

## ✨ Features

### 🎯 24 Dithering Algorithms

**Error Diffusion Algorithms:**
- Floyd-Steinberg (1976) - Classic, most popular
- Atkinson (1984) - Used in classic Macintosh
- Jarvis-Judice-Ninke (1976) - High quality
- Stucki (1981) - Balanced quality
- Burkes (1988) - Fast processing
- Sierra (Three-Row) (1989) - High quality
- Sierra Two-Row (1990) - Faster variant
- Sierra Lite - Lightweight version
- False Floyd-Steinberg - Simplified version
- Steven Pigeon - Optimized variant

**Ordered Dithering:**
- Bayer 2×2 - Tiny pattern
- Bayer 4×4 - Small pattern
- Bayer 8×8 - Medium pattern
- Bayer 16×16 - Large pattern

**Noise-Based:**
- Blue Noise - Visually pleasing
- White Noise - Random dithering

**Threshold Methods:**
- Simple Threshold - Basic binarization
- Random Threshold - Randomized
- Gradient-Based - Edge-aware

**Pattern & Halftone:**
- Pattern Dithering - Fixed patterns
- Clustered Dot - Classic halftone
- Halftone Circle - Circular patterns
- Halftone Diamond - Diamond patterns

**Advanced:**
- Riemersma - Space-filling curve algorithm

### 🎬 Media Support
- **Images:** PNG, JPG, JPEG, BMP, GIF, WebP
- **Videos:** MP4, MOV, AVI, MKV, WebM (requires FFmpeg)

### ⚙️ Extensive Settings
- **Color Modes:** Monochrome, Grayscale, Full Color
- **Colors per Channel:** 2-16 levels
- **Threshold Control:** Fine-tune brightness
- **Error Diffusion:** Adjustable strength (0.0-1.0)
- **Pattern Scale:** Customizable pattern sizes
- **Serpentine Scanning:** Improved quality for error diffusion
- **Real-time Preview:** See changes instantly

### 💎 Professional UI
- Dark theme inspired by professional image editors
- Real-time zoom (10%-400%)
- Drag & drop support
- Checkerboard transparency view
- Video processing progress indicator
- Detailed image information panel

## 🚀 Getting Started

### Prerequisites

#### All Platforms
- [Node.js](https://nodejs.org/) (v18 or later)
- [Rust](https://rustup.rs/) (latest stable)

#### Linux
```bash
sudo apt update
sudo apt install libwebkit2gtk-4.1-dev \
    build-essential \
    curl \
    wget \
    file \
    libxdo-dev \
    libssl-dev \
    libayatana-appindicator3-dev \
    librsvg2-dev
```

#### macOS
```bash
xcode-select --install
```

#### Windows
- [Microsoft Visual C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)
- [WebView2](https://developer.microsoft.com/en-us/microsoft-edge/webview2/) (usually pre-installed on Windows 11)

#### Video Processing (Optional)
For video dithering, install [FFmpeg](https://ffmpeg.org/download.html):
- **Linux:** `sudo apt install ffmpeg`
- **macOS:** `brew install ffmpeg`
- **Windows:** Download from [ffmpeg.org](https://ffmpeg.org/download.html)

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/yourusername/dithers-boyfriend.git
cd dithers-boyfriend
```

2. **Install dependencies**
```bash
npm install
```

3. **Run in development mode**
```bash
npm run tauri dev
```

### Building

#### Development Build
```bash
npm run tauri dev
```

#### Production Build
```bash
npm run tauri build
```

The built application will be in `src-tauri/target/release/bundle/`:
- **Windows:** `.msi` and `.exe` installers
- **Linux:** `.deb`, `.AppImage`
- **macOS:** `.app` and `.dmg`

## 📖 Usage

### Processing Images

1. **Open an Image**
   - Click "Open File" or drag & drop an image
   - Supported formats: PNG, JPG, JPEG, BMP, GIF, WebP

2. **Choose Algorithm**
   - Select from 24 dithering algorithms
   - Each has a description explaining its characteristics

3. **Adjust Settings**
   - Color Mode: Monochrome, Grayscale, or Full Color
   - Colors: 2-16 colors per channel
   - Threshold: Control brightness (0.0-1.0)
   - Error Diffusion: Adjust error spreading (for applicable algorithms)
   - Pattern Scale: Change pattern size (for pattern-based algorithms)
   - Serpentine: Enable alternating scan direction

4. **Preview & Save**
   - Changes apply in real-time
   - Use zoom controls to inspect details
   - Click "Save Image" to export

### Processing Videos

1. **Open a Video**
   - Click "Open File" and select a video file
   - Supported formats: MP4, MOV, AVI, MKV, WebM

2. **Configure Settings**
   - Choose algorithm and settings (same as images)

3. **Process**
   - Click "Process Video"
   - Choose output location
   - Monitor progress bar
   - FFmpeg required

## 🎨 Algorithm Guide

### When to Use Each Algorithm

**Floyd-Steinberg**
- Best all-around choice
- Excellent for photographs
- Good detail preservation

**Atkinson**
- Lighter, more artistic look
- Great for logos and graphics
- Retro Macintosh aesthetic

**Bayer Matrix**
- Fast processing
- Good for textures
- Regular, predictable patterns

**Blue Noise**
- Visually pleasing randomness
- Smooth gradients
- Modern aesthetic

**Halftone**
- Classic print look
- Artistic effects
- Newspaper style

**Riemersma**
- Unique space-filling patterns
- Experimental look
- Good edge preservation

## 🏗️ Architecture

```
dithers-boyfriend/
├── src/                    # React frontend
│   ├── App.tsx            # Main application component
│   ├── App.css            # Component styles
│   ├── index.css          # Global styles
│   └── main.tsx           # Entry point
├── src-tauri/             # Rust backend
│   └── src/
│       ├── main.rs        # Tauri application
│       ├── dither.rs      # 24 dithering algorithms
│       └── video.rs       # Video processing
├── package.json           # Node dependencies
└── Cargo.toml            # Rust dependencies
```

## 🛠️ Technology Stack

- **Frontend:** React 18 + TypeScript
- **Backend:** Rust (Tauri 2.0)
- **Image Processing:** Rust `image` crate
- **Video Processing:** FFmpeg
- **Styling:** Tailwind CSS
- **Icons:** Lucide React
- **Build:** Vite

## 🎯 Performance

- **Fast:** Rust-powered processing
- **Efficient:** Native performance
- **Parallel:** Multi-threaded where applicable
- **Small:** ~10-20MB bundle size (vs 100MB+ Electron apps)

## 🔧 Development

### Project Structure
- `src/` - React TypeScript frontend
- `src-tauri/` - Rust backend with dithering algorithms
- `src-tauri/src/dither.rs` - All dithering implementations

### Adding New Algorithms

1. Add algorithm variant to `DitheringAlgorithm` enum in `dither.rs`
2. Implement the algorithm function
3. Add match case in `apply_dithering()`
4. Add to `ALGORITHMS` array in `App.tsx`

### Testing
```bash
# Run tests
cargo test --manifest-path src-tauri/Cargo.toml

# Format code
cargo fmt --manifest-path src-tauri/Cargo.toml
npm run format
```

## 📝 License

MIT License - see LICENSE file for details

## 🙏 Acknowledgments

- Dithering algorithms based on research papers and historical implementations
- Floyd-Steinberg (1976), Atkinson (1984), and other pioneers of digital halftoning
- Tauri team for the excellent framework
- Rust community for image processing libraries

## 🐛 Known Issues

- Video processing requires FFmpeg to be installed separately
- Large videos may take significant time to process
- Linux WebKit dependencies required for building

## 🚧 Future Enhancements

- [ ] Custom color palette support
- [ ] Batch processing
- [ ] Live camera feed dithering
- [ ] Additional export formats
- [ ] Preset management
- [ ] Before/after comparison view
- [ ] Animation export (GIF)
- [ ] Plugin system for custom algorithms

## 📧 Contact

For issues, feature requests, or contributions, please open an issue on GitHub.

---

**Made with ❤️ and lots of pixels**
