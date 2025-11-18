use crate::dither::{apply_dithering, DitheringSettings};
use image::DynamicImage;
use std::process::{Command, Stdio};
use std::path::Path;

pub async fn process_video_file<F>(
    input_path: &str,
    output_path: &str,
    settings: &DitheringSettings,
    mut progress_callback: F,
) -> Result<(), String>
where
    F: FnMut(f32),
{
    // Create temporary directory for frames
    let temp_dir = std::env::temp_dir().join(format!("dither_video_{}", std::process::id()));
    std::fs::create_dir_all(&temp_dir).map_err(|e| e.to_string())?;

    // Extract frames using ffmpeg
    let frames_pattern = temp_dir.join("frame_%06d.png");
    let extract_status = Command::new("ffmpeg")
        .args([
            "-i",
            input_path,
            "-vf",
            "fps=30",
            frames_pattern.to_str().unwrap(),
        ])
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .status();

    if extract_status.is_err() {
        return Err("FFmpeg not found. Please install FFmpeg to process videos.".to_string());
    }

    // Get list of frames
    let mut frames: Vec<_> = std::fs::read_dir(&temp_dir)
        .map_err(|e| e.to_string())?
        .filter_map(|e| e.ok())
        .filter(|e| e.path().extension().and_then(|s| s.to_str()) == Some("png"))
        .collect();

    frames.sort_by_key(|f| f.path());

    let total_frames = frames.len();

    // Process each frame
    for (i, frame) in frames.iter().enumerate() {
        let img = image::open(frame.path()).map_err(|e| e.to_string())?;
        let dithered = apply_dithering(&img, settings);
        dithered.save(frame.path()).map_err(|e| e.to_string())?;

        progress_callback((i + 1) as f32 / total_frames as f32 * 100.0);
    }

    // Reassemble video
    let input_pattern = temp_dir.join("frame_%06d.png");
    let reassemble_status = Command::new("ffmpeg")
        .args([
            "-framerate",
            "30",
            "-i",
            input_pattern.to_str().unwrap(),
            "-c:v",
            "libx264",
            "-pix_fmt",
            "yuv420p",
            "-y",
            output_path,
        ])
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .status()
        .map_err(|e| e.to_string())?;

    // Clean up
    let _ = std::fs::remove_dir_all(&temp_dir);

    if reassemble_status.success() {
        Ok(())
    } else {
        Err("Failed to reassemble video".to_string())
    }
}
