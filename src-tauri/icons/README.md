# Application Icons

To build the application with custom icons, place the following files in this directory:

- `32x32.png` - 32x32 pixel icon
- `128x128.png` - 128x128 pixel icon
- `128x128@2x.png` - 256x256 pixel icon (high DPI)
- `icon.icns` - macOS icon bundle
- `icon.ico` - Windows icon

For now, Tauri will use default icons if these are not present.

## Generating Icons

You can use the Tauri icon generator:

```bash
npm install -g @tauri-apps/cli
tauri icon path/to/your-icon.png
```

This will automatically generate all required icon sizes and formats.
