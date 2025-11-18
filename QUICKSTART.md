# Quick Start Guide

## 🚀 Run the App (5 minutes)

### Step 1: Install Prerequisites

**Linux (Ubuntu/Debian):**
```bash
# Install system dependencies
sudo apt update
sudo apt install -y libwebkit2gtk-4.1-dev build-essential curl wget file libssl-dev librsvg2-dev

# Install Node.js (if not installed)
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt install -y nodejs

# Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source $HOME/.cargo/env
```

**macOS:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install Node.js and Rust
brew install node rust
```

**Windows:**
1. Install [Node.js](https://nodejs.org/) (LTS version)
2. Install [Rust](https://rustup.rs/)
3. Install [Visual Studio Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)

### Step 2: Install Dependencies

```bash
cd dithers-boyfriend
npm install
```

### Step 3: Run!

```bash
npm run tauri dev
```

That's it! The app will open automatically.

## 🎨 Try It Out

1. Click **"Open File"** and select an image
2. Choose an algorithm (try **Floyd-Steinberg** first)
3. Adjust settings in the right panel
4. Watch the real-time preview
5. Click **"Save Image"** when you're happy

## 🎬 Processing Videos (Optional)

### Install FFmpeg

**Linux:**
```bash
sudo apt install ffmpeg
```

**macOS:**
```bash
brew install ffmpeg
```

**Windows:**
Download from [ffmpeg.org](https://ffmpeg.org/download.html) and add to PATH

### Process a Video

1. Click **"Open File"** and select a video
2. Adjust dithering settings
3. Click **"Process Video"**
4. Choose output location
5. Wait for processing (with progress bar)

## 🏗️ Building Executables

### Build for Your Platform

```bash
npm run tauri build
```

Outputs will be in `src-tauri/target/release/bundle/`:
- **Linux:** `.deb`, `.AppImage`
- **macOS:** `.app`, `.dmg`
- **Windows:** `.exe`, `.msi`

## 🎯 Algorithm Quick Reference

**Start with these:**
- **Floyd-Steinberg** - Classic, great for photos
- **Atkinson** - Lighter, artistic look
- **Bayer 8×8** - Fast, regular patterns

**For artistic effects:**
- **Halftone Circle** - Newspaper style
- **Blue Noise** - Smooth, modern
- **Riemersma** - Unique patterns

**For retro/pixel art:**
- **Bayer 2×2** - Tiny patterns
- **Pattern Dithering** - Fixed patterns
- **Simple Threshold** - Pure black & white

## ⚙️ Settings Explained

- **Colors per Channel:** 2 = black & white, 16 = more colors
- **Threshold:** Controls brightness (0.5 = balanced)
- **Error Diffusion:** How much error spreads (1.0 = full)
- **Pattern Scale:** Size of patterns (larger = coarser)
- **Serpentine:** Improves quality for error diffusion

## 🐛 Troubleshooting

### App won't start
```bash
# Clean and rebuild
rm -rf node_modules src-tauri/target
npm install
npm run tauri dev
```

### Linux WebKit errors
```bash
sudo apt install libwebkit2gtk-4.1-dev librsvg2-dev
```

### Video processing fails
- Make sure FFmpeg is installed: `ffmpeg -version`
- Check that video file format is supported

### Build fails
- Update Rust: `rustup update`
- Update Node packages: `npm update`
- Check prerequisites are installed

## 💡 Tips

1. **Start with Monochrome** mode to see algorithms clearly
2. **Use Zoom** to inspect fine details (up to 400%)
3. **Try Serpentine** for smoother gradients
4. **Lower Error Diffusion** for more contrast
5. **Increase Colors** for smoother tones

## 📚 Learn More

- Full documentation in [README.md](README.md)
- Algorithm details in comments in `src-tauri/src/dither.rs`
- Tauri docs: [tauri.app](https://tauri.app)

---

**Have fun dithering! 🎨**
