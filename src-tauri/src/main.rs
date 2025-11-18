// Prevents additional console window on Windows in release
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod dither;
mod video;

use dither::{apply_dithering, DitheringSettings};
use image::DynamicImage;
use std::path::PathBuf;
use tauri::Manager;

#[tauri::command]
async fn load_image(path: String) -> Result<String, String> {
    let img = image::open(&path).map_err(|e| e.to_string())?;
    let (width, height) = img.dimensions();

    // Convert to base64 for preview
    let mut buffer = Vec::new();
    img.write_to(&mut std::io::Cursor::new(&mut buffer), image::ImageFormat::Png)
        .map_err(|e| e.to_string())?;

    Ok(format!(
        "data:image/png;base64,{}",
        base64::engine::general_purpose::STANDARD.encode(&buffer)
    ))
}

#[tauri::command]
async fn apply_dither(
    image_path: String,
    settings: DitheringSettings,
) -> Result<String, String> {
    let img = image::open(&image_path).map_err(|e| e.to_string())?;
    let dithered = apply_dithering(&img, &settings);

    // Convert to base64
    let mut buffer = Vec::new();
    dithered
        .write_to(&mut std::io::Cursor::new(&mut buffer), image::ImageFormat::Png)
        .map_err(|e| e.to_string())?;

    Ok(format!(
        "data:image/png;base64,{}",
        base64::engine::general_purpose::STANDARD.encode(&buffer)
    ))
}

#[tauri::command]
async fn save_image(
    image_data: String,
    output_path: String,
) -> Result<(), String> {
    // Remove data URL prefix
    let base64_data = image_data
        .strip_prefix("data:image/png;base64,")
        .ok_or("Invalid image data format")?;

    let decoded = base64::engine::general_purpose::STANDARD
        .decode(base64_data)
        .map_err(|e| e.to_string())?;

    let img = image::load_from_memory(&decoded).map_err(|e| e.to_string())?;

    // Determine format from extension
    let path = PathBuf::from(&output_path);
    let format = match path.extension().and_then(|s| s.to_str()) {
        Some("png") => image::ImageFormat::Png,
        Some("jpg") | Some("jpeg") => image::ImageFormat::Jpeg,
        Some("bmp") => image::ImageFormat::Bmp,
        Some("gif") => image::ImageFormat::Gif,
        Some("webp") => image::ImageFormat::WebP,
        _ => image::ImageFormat::Png,
    };

    img.save_with_format(&output_path, format)
        .map_err(|e| e.to_string())?;

    Ok(())
}

#[tauri::command]
async fn get_image_info(path: String) -> Result<ImageInfo, String> {
    let img = image::open(&path).map_err(|e| e.to_string())?;
    let (width, height) = img.dimensions();

    Ok(ImageInfo {
        width,
        height,
        color_type: format!("{:?}", img.color()),
    })
}

#[tauri::command]
async fn process_video(
    input_path: String,
    output_path: String,
    settings: DitheringSettings,
    app_handle: tauri::AppHandle,
) -> Result<(), String> {
    video::process_video_file(&input_path, &output_path, &settings, move |progress| {
        let _ = app_handle.emit("video-progress", progress);
    })
    .await
}

#[derive(serde::Serialize)]
struct ImageInfo {
    width: u32,
    height: u32,
    color_type: String,
}

fn main() {
    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_fs::init())
        .plugin(tauri_plugin_shell::init())
        .invoke_handler(tauri::generate_handler![
            load_image,
            apply_dither,
            save_image,
            get_image_info,
            process_video,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
