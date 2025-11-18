# 🚀 Quick Start Guide

Get dithering in under 2 minutes!

## Step 1: Build

```bash
./build.sh
```

Wait for the script to complete. It will:
- Install all dependencies automatically
- Download Dear ImGui
- Compile the application

## Step 2: Run

```bash
./dithers-boyfriend
```

## Step 3: Dither!

1. Click **"Load Test Image"** to load a gradient test image
2. Select an algorithm from the dropdown (try **Floyd-Steinberg** first)
3. Choose a palette (try **Monochrome** for classic black & white)
4. Adjust the sliders to see different effects
5. Click **File → Save As** to export

## Popular Presets

### Classic Black & White
- Algorithm: **Floyd-Steinberg**
- Palette: **Monochrome**
- Strength: **1.0**
- Serpentine: **On**

### Retro Game Boy
- Algorithm: **Atkinson**
- Palette: **Game Boy**
- Strength: **0.75**
- Gamma: **1.2**

### Modern Blue Noise
- Algorithm: **Blue Noise**
- Palette: **Grayscale 8**
- Strength: **1.0**
- Seed: **42**

### Pixel Art Style
- Algorithm: **Ordered Bayer 4x4**
- Palette: **PICO-8**
- Strength: **1.5**

## Load Your Own Images

From command line:
```bash
./dithers-boyfriend path/to/your/image.png
```

Or use **File → Open Image** in the application.

## Troubleshooting

**Build fails?**
```bash
# Install dependencies manually
make setup
```

**Application won't start?**
- Make sure you have OpenGL 3.3+ support
- Update graphics drivers

**Need help?**
```bash
make help
```

## Next Steps

- Read the full [README.md](README.md) for detailed documentation
- Experiment with different algorithm combinations
- Try processing videos
- Create custom palettes

---

**Have fun dithering! 🎨**
