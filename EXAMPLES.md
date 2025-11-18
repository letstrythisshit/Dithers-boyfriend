# 📷 Examples & Showcase

This document provides example use cases and best practices for Dither's Boyfriend.

## Algorithm Comparison

Here's a guide to help you choose the right algorithm for your needs:

### For Photographs

**Best Choice: Floyd-Steinberg or Jarvis-Judice-Ninke**
- Settings:
  - Palette: Grayscale 16 or Monochrome
  - Strength: 1.0
  - Serpentine: On
  - Gamma: 1.0-1.2

**Alternative: Gradient-Based**
- Settings:
  - Palette: Grayscale 8
  - Strength: 1.2
  - Contrast: 1.2

### For Line Art & Illustrations

**Best Choice: Atkinson**
- Settings:
  - Palette: Monochrome
  - Strength: 0.75
  - Gamma: 1.2

**Alternative: Sierra Lite**
- Settings:
  - Palette: Grayscale 4
  - Strength: 0.8

### For Retro Gaming Aesthetic

**Best Choice: Ordered Bayer 4x4**
- Settings:
  - Palette: Game Boy, CGA, or PICO-8
  - Strength: 1.5
  - Gamma: 1.1

**Alternative: Atkinson**
- Settings:
  - Palette: Game Boy
  - Strength: 0.75

### For Pixel Art

**Best Choice: Ordered Bayer 2x2 or 4x4**
- Settings:
  - Palette: PICO-8 or Custom
  - Strength: 1.2-1.8
  - Contrast: 1.2

### For Print Media

**Best Choice: Blue Noise**
- Settings:
  - Palette: Monochrome
  - Strength: 1.0
  - Seed: 42 (or any)

**Alternative: Stucki or Jarvis-Judice-Ninke**
- Settings:
  - Palette: Monochrome
  - Strength: 1.0
  - Serpentine: On

### For Artistic/Experimental

**Best Choice: Dot Diffusion or Pattern Dither**
- Settings:
  - Palette: Any
  - Strength: 1.0-2.0
  - Experiment with all parameters!

**Alternative: Variable Error Diffusion**
- Settings:
  - Palette: Custom or PICO-8
  - Strength: 1.2
  - Seed: Vary for different results

## Color Palette Guide

### Monochrome
Perfect for: Classic black and white, high contrast, print
```
Colors: Black, White
Use with: Any error diffusion algorithm
```

### Grayscale 4/8/16
Perfect for: Smooth gradients, photographs, general use
```
Colors: 4, 8, or 16 shades of gray
Use with: Floyd-Steinberg, Jarvis-Judice-Ninke
```

### Game Boy
Perfect for: Retro games, nostalgic feel
```
Colors: 4 shades of green
Use with: Atkinson, Bayer 4x4
```

### CGA
Perfect for: IBM PC retro aesthetic
```
Colors: 16 classic CGA colors
Use with: Bayer matrices, Pattern Dither
```

### PICO-8
Perfect for: Modern pixel art, fantasy console style
```
Colors: 16 carefully chosen colors
Use with: Ordered Bayer, Floyd-Steinberg
```

## Advanced Techniques

### Creating Smooth Gradients

1. Use Floyd-Steinberg or Stucki
2. Set Strength to 1.0
3. Enable Serpentine scanning
4. Use Grayscale 16 palette
5. Adjust Gamma to 1.0-1.2

### High-Contrast Images

1. Use Atkinson or Sierra Lite
2. Set Strength to 0.7-0.9
3. Increase Contrast to 1.5-2.0
4. Use Monochrome palette
5. Adjust Brightness as needed

### Retro Game Look

1. Use Bayer 4x4 or Pattern Dither
2. Choose Game Boy or CGA palette
3. Set Strength to 1.5
4. Increase Saturation to 1.2
5. Set Gamma to 1.1

### Artistic Noise

1. Use Blue Noise or White Noise
2. Try different seeds (0-1000)
3. Adjust Strength (0.8-1.5)
4. Experiment with custom palettes
5. Layer multiple exports in image editor

### Video Processing Tips

1. Use consistent settings across all frames
2. Faster algorithms (Sierra Lite, Bayer) for long videos
3. Higher quality algorithms (Blue Noise, Ostromoukhov) for short clips
4. Match palette to video content
5. Consider color grading before dithering

## Parameter Guide

### Strength (0.0 - 2.0)
- **0.0-0.5**: Subtle dithering, maintains more original image
- **0.5-1.0**: Standard dithering effect
- **1.0-1.5**: Strong dithering, more pattern visible
- **1.5-2.0**: Extreme dithering, artistic effect

### Gamma (0.1 - 3.0)
- **< 1.0**: Darkens image, enhances shadows
- **1.0**: No change (recommended start)
- **> 1.0**: Lightens image, enhances highlights
- **1.8-2.2**: Standard for web/display

### Contrast (0.0 - 3.0)
- **< 1.0**: Reduces contrast, softer look
- **1.0**: No change
- **> 1.0**: Increases contrast, more dramatic
- **1.5-2.0**: High contrast, bold look

### Brightness (-1.0 - 1.0)
- **< 0**: Darkens image
- **0**: No change
- **> 0**: Lightens image
- Use sparingly, often gamma is better

### Saturation (0.0 - 2.0)
- **0.0**: Grayscale
- **0.5-0.8**: Desaturated, muted colors
- **1.0**: Original saturation
- **1.2-2.0**: Enhanced, vivid colors

## Common Recipes

### Classic Newspaper Print
```
Algorithm: Floyd-Steinberg
Palette: Monochrome
Strength: 1.0
Serpentine: On
Gamma: 1.8
Contrast: 1.2
```

### Retro Mac Screenshot
```
Algorithm: Atkinson
Palette: Monochrome
Strength: 0.75
Gamma: 1.2
Contrast: 1.0
```

### GameBoy Camera Effect
```
Algorithm: Atkinson or Bayer 4x4
Palette: Game Boy
Strength: 0.8
Gamma: 1.3
Contrast: 1.4
```

### Modern Minimal
```
Algorithm: Blue Noise
Palette: Grayscale 8
Strength: 0.9
Gamma: 1.0
Contrast: 1.1
```

### Psychedelic Art
```
Algorithm: Variable Error Diffusion
Palette: PICO-8
Strength: 1.5
Saturation: 1.5
Seed: Vary
```

### High-Quality Print
```
Algorithm: Stucki or Jarvis-Judice-Ninke
Palette: Monochrome
Strength: 1.0
Serpentine: On
Gamma: 2.2
```

## Workflow Tips

1. **Start Simple**: Begin with Floyd-Steinberg and Monochrome
2. **Preprocess**: Adjust brightness/contrast before dithering
3. **Experiment**: Try different algorithms on same image
4. **Save Settings**: Note your favorite combinations
5. **Batch Process**: Use same settings for series
6. **Post-Process**: Can combine dithered images in layers
7. **Compare**: Use split view to judge results
8. **Iterate**: Small parameter changes = big differences

## Optimization Tips

### For Speed
- Use Sierra Lite or Bayer algorithms
- Disable Auto Update
- Process in batches
- Reduce image size first

### For Quality
- Use Blue Noise or Ostromoukhov
- Higher grayscale levels (8 or 16)
- Enable Serpentine scanning
- Adjust gamma for monitor

### For File Size
- Use fewer palette colors
- Monochrome is smallest
- PNG format compresses well
- Consider dithering at lower resolution

## Troubleshooting Results

### Too Dark
- Increase Gamma (1.2-1.8)
- Increase Brightness (0.1-0.3)
- Reduce Contrast slightly

### Too Noisy
- Reduce Strength (0.7-0.9)
- Use Atkinson or Sierra Lite
- Increase palette colors
- Disable Serpentine

### Lost Detail
- Use Jarvis-Judice-Ninke or Stucki
- Increase Contrast (1.2-1.5)
- Use more palette colors
- Try Gradient-Based algorithm

### Too Patterned
- Switch to error diffusion algorithm
- Use Blue Noise instead of Bayer
- Enable Serpentine
- Reduce Strength

### Banding in Gradients
- Use Floyd-Steinberg or Stucki
- Enable Serpentine
- Increase palette colors
- Adjust Gamma

## Video Processing Examples

### Music Video Effect
```
Algorithm: Bayer 8x8 or Blue Noise
Palette: PICO-8 or CGA
Strength: 1.2
High Saturation: 1.4
```

### Documentary Style
```
Algorithm: Floyd-Steinberg
Palette: Grayscale 8
Strength: 0.9
Gamma: 1.8
```

### Retro Gaming
```
Algorithm: Bayer 4x4
Palette: Game Boy or CGA
Strength: 1.5
Contrast: 1.2
```

---

**Experiment and have fun! The best results come from trying different combinations!** 🎨
