import { useState, useEffect, useRef } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { open, save } from '@tauri-apps/plugin-dialog';
import {
  Image,
  Save,
  FolderOpen,
  Settings as SettingsIcon,
  Video,
  ZoomIn,
  ZoomOut,
  RotateCcw,
  Layers,
  Sliders
} from 'lucide-react';
import './App.css';

interface DitheringSettings {
  algorithm: string;
  colors: number;
  threshold: number;
  error_diffusion: number;
  pattern_scale: number;
  serpentine: boolean;
  color_mode: string;
}

interface ImageInfo {
  width: number;
  height: number;
  color_type: string;
}

const ALGORITHMS = [
  { value: 'FloydSteinberg', label: 'Floyd-Steinberg (Classic)', description: 'Most popular error diffusion algorithm (1976)' },
  { value: 'Atkinson', label: 'Atkinson', description: 'Classic Macintosh dithering (1984)' },
  { value: 'JarvisJudiceNinke', label: 'Jarvis-Judice-Ninke', description: 'High quality error diffusion (1976)' },
  { value: 'Stucki', label: 'Stucki', description: 'Balanced error diffusion (1981)' },
  { value: 'Burkes', label: 'Burkes', description: 'Fast error diffusion (1988)' },
  { value: 'Sierra', label: 'Sierra (Three-Row)', description: 'High quality Sierra variant (1989)' },
  { value: 'SierraTwoRow', label: 'Sierra Two-Row', description: 'Faster Sierra variant (1990)' },
  { value: 'SierraLite', label: 'Sierra Lite', description: 'Lightweight Sierra variant' },
  { value: 'FalseFloydSteinberg', label: 'False Floyd-Steinberg', description: 'Simplified Floyd-Steinberg' },
  { value: 'StevenPigeon', label: 'Steven Pigeon', description: 'Optimized error diffusion' },
  { value: 'Bayer2x2', label: 'Bayer 2×2', description: 'Ordered dithering - tiny pattern' },
  { value: 'Bayer4x4', label: 'Bayer 4×4', description: 'Ordered dithering - small pattern' },
  { value: 'Bayer8x8', label: 'Bayer 8×8', description: 'Ordered dithering - medium pattern' },
  { value: 'Bayer16x16', label: 'Bayer 16×16', description: 'Ordered dithering - large pattern' },
  { value: 'BlueNoise', label: 'Blue Noise', description: 'Visually pleasing noise pattern' },
  { value: 'WhiteNoise', label: 'White Noise', description: 'Random noise dithering' },
  { value: 'SimpleThreshold', label: 'Simple Threshold', description: 'Basic threshold binarization' },
  { value: 'RandomThreshold', label: 'Random Threshold', description: 'Randomized threshold' },
  { value: 'Pattern', label: 'Pattern Dithering', description: 'Fixed pattern dithering' },
  { value: 'ClusteredDot', label: 'Clustered Dot', description: 'Classic halftone style' },
  { value: 'HalftoneCircle', label: 'Halftone Circle', description: 'Circular halftone pattern' },
  { value: 'HalftoneDiamond', label: 'Halftone Diamond', description: 'Diamond halftone pattern' },
  { value: 'Riemersma', label: 'Riemersma', description: 'Space-filling curve dithering' },
  { value: 'GradientBased', label: 'Gradient-Based', description: 'Edge-aware dithering' },
];

const COLOR_MODES = [
  { value: 'Monochrome', label: 'Monochrome (Black & White)' },
  { value: 'Grayscale', label: 'Grayscale' },
  { value: 'FullColor', label: 'Full Color' },
];

function App() {
  const [originalImage, setOriginalImage] = useState<string | null>(null);
  const [processedImage, setProcessedImage] = useState<string | null>(null);
  const [imagePath, setImagePath] = useState<string>('');
  const [imageInfo, setImageInfo] = useState<ImageInfo | null>(null);
  const [isProcessing, setIsProcessing] = useState(false);
  const [videoProgress, setVideoProgress] = useState<number>(0);
  const [zoom, setZoom] = useState(100);
  const [showSettings, setShowSettings] = useState(true);
  const [statusMessage, setStatusMessage] = useState('Ready');

  const [settings, setSettings] = useState<DitheringSettings>({
    algorithm: 'FloydSteinberg',
    colors: 2,
    threshold: 0.5,
    error_diffusion: 1.0,
    pattern_scale: 2,
    serpentine: true,
    color_mode: 'Monochrome',
  });

  const canvasRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    // Auto-process when settings change
    if (originalImage && imagePath) {
      processImage();
    }
  }, [settings]);

  const loadImage = async () => {
    try {
      const selected = await open({
        multiple: false,
        filters: [{
          name: 'Image',
          extensions: ['png', 'jpg', 'jpeg', 'bmp', 'gif', 'webp']
        }, {
          name: 'Video',
          extensions: ['mp4', 'mov', 'avi', 'mkv', 'webm']
        }]
      });

      if (selected) {
        setStatusMessage('Loading file...');
        const path = typeof selected === 'string' ? selected : selected.path;
        setImagePath(path);

        // Check if it's a video
        const ext = path.split('.').pop()?.toLowerCase();
        if (['mp4', 'mov', 'avi', 'mkv', 'webm'].includes(ext || '')) {
          setStatusMessage('Video file loaded. Click "Process Video" to dither.');
          return;
        }

        const base64 = await invoke<string>('load_image', { path });
        setOriginalImage(base64);
        setProcessedImage(null);

        const info = await invoke<ImageInfo>('get_image_info', { path });
        setImageInfo(info);
        setStatusMessage(`Image loaded: ${info.width}×${info.height}px`);

        // Auto-process
        await processImage(path, base64);
      }
    } catch (error) {
      console.error('Failed to load file:', error);
      setStatusMessage(`Error: ${error}`);
    }
  };

  const processImage = async (path?: string, original?: string) => {
    if (!imagePath && !path) return;

    try {
      setIsProcessing(true);
      setStatusMessage('Processing...');

      const result = await invoke<string>('apply_dither', {
        imagePath: path || imagePath,
        settings
      });

      setProcessedImage(result);
      setStatusMessage('Processing complete');
    } catch (error) {
      console.error('Failed to process image:', error);
      setStatusMessage(`Error: ${error}`);
    } finally {
      setIsProcessing(false);
    }
  };

  const processVideo = async () => {
    if (!imagePath) return;

    try {
      const outputPath = await save({
        filters: [{
          name: 'Video',
          extensions: ['mp4']
        }]
      });

      if (outputPath) {
        setIsProcessing(true);
        setVideoProgress(0);
        setStatusMessage('Processing video...');

        await invoke('process_video', {
          inputPath: imagePath,
          outputPath,
          settings
        });

        setStatusMessage('Video processing complete');
        setVideoProgress(100);
      }
    } catch (error) {
      console.error('Failed to process video:', error);
      setStatusMessage(`Error: ${error}`);
    } finally {
      setIsProcessing(false);
    }
  };

  const saveImage = async () => {
    if (!processedImage) return;

    try {
      const outputPath = await save({
        filters: [{
          name: 'Image',
          extensions: ['png', 'jpg', 'bmp', 'gif', 'webp']
        }]
      });

      if (outputPath) {
        await invoke('save_image', {
          imageData: processedImage,
          outputPath
        });
        setStatusMessage(`Saved to ${outputPath}`);
      }
    } catch (error) {
      console.error('Failed to save image:', error);
      setStatusMessage(`Error: ${error}`);
    }
  };

  const resetSettings = () => {
    setSettings({
      algorithm: 'FloydSteinberg',
      colors: 2,
      threshold: 0.5,
      error_diffusion: 1.0,
      pattern_scale: 2,
      serpentine: true,
      color_mode: 'Monochrome',
    });
  };

  const selectedAlgorithm = ALGORITHMS.find(a => a.value === settings.algorithm);

  return (
    <div className="flex flex-col h-screen bg-app-bg text-gray-200">
      {/* Top Toolbar */}
      <div className="flex items-center gap-2 px-4 py-2 bg-panel-bg border-b border-panel-border">
        <button
          onClick={loadImage}
          className="flex items-center gap-2 px-4 py-2 bg-accent hover:bg-accent-hover rounded transition-colors"
        >
          <FolderOpen size={18} />
          Open File
        </button>

        <button
          onClick={saveImage}
          disabled={!processedImage || isProcessing}
          className="flex items-center gap-2 px-4 py-2 bg-input-bg hover:bg-button-hover rounded transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <Save size={18} />
          Save Image
        </button>

        <button
          onClick={processVideo}
          disabled={!imagePath || isProcessing}
          className="flex items-center gap-2 px-4 py-2 bg-input-bg hover:bg-button-hover rounded transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <Video size={18} />
          Process Video
        </button>

        <div className="flex-1" />

        <button
          onClick={() => setZoom(Math.max(10, zoom - 10))}
          className="p-2 bg-input-bg hover:bg-button-hover rounded transition-colors"
        >
          <ZoomOut size={18} />
        </button>
        <span className="text-sm w-16 text-center">{zoom}%</span>
        <button
          onClick={() => setZoom(Math.min(400, zoom + 10))}
          className="p-2 bg-input-bg hover:bg-button-hover rounded transition-colors"
        >
          <ZoomIn size={18} />
        </button>

        <button
          onClick={() => setShowSettings(!showSettings)}
          className={`p-2 rounded transition-colors ${showSettings ? 'bg-accent' : 'bg-input-bg hover:bg-button-hover'}`}
        >
          <Sliders size={18} />
        </button>
      </div>

      <div className="flex flex-1 overflow-hidden">
        {/* Main Canvas Area */}
        <div className="flex-1 flex flex-col overflow-hidden">
          <div
            ref={canvasRef}
            className="flex-1 flex items-center justify-center overflow-auto checkerboard relative"
          >
            {processedImage ? (
              <div className="relative">
                <img
                  src={processedImage}
                  alt="Processed"
                  style={{
                    width: `${zoom}%`,
                    imageRendering: zoom > 100 ? 'pixelated' : 'auto'
                  }}
                  className="max-w-none shadow-2xl"
                />
                {isProcessing && (
                  <div className="absolute inset-0 flex items-center justify-center bg-black bg-opacity-50">
                    <div className="text-white text-lg">Processing...</div>
                  </div>
                )}
              </div>
            ) : originalImage ? (
              <div className="relative">
                <img
                  src={originalImage}
                  alt="Original"
                  style={{
                    width: `${zoom}%`,
                    imageRendering: zoom > 100 ? 'pixelated' : 'auto'
                  }}
                  className="max-w-none shadow-2xl"
                />
              </div>
            ) : (
              <div className="text-center text-gray-500">
                <Image size={64} className="mx-auto mb-4 opacity-50" />
                <p className="text-lg">Drop an image or video file here</p>
                <p className="text-sm mt-2">or click "Open File" to get started</p>
              </div>
            )}
          </div>

          {/* Video Progress Bar */}
          {isProcessing && videoProgress > 0 && (
            <div className="px-4 py-2 bg-panel-bg border-t border-panel-border">
              <div className="flex items-center gap-2">
                <span className="text-sm">Video Progress:</span>
                <div className="flex-1 h-2 bg-input-bg rounded overflow-hidden">
                  <div
                    className="h-full bg-accent transition-all duration-300"
                    style={{ width: `${videoProgress}%` }}
                  />
                </div>
                <span className="text-sm w-12 text-right">{videoProgress.toFixed(0)}%</span>
              </div>
            </div>
          )}
        </div>

        {/* Settings Panel */}
        {showSettings && (
          <div className="w-80 bg-panel-bg border-l border-panel-border overflow-y-auto">
            <div className="p-4">
              <div className="flex items-center justify-between mb-4">
                <h2 className="text-lg font-semibold flex items-center gap-2">
                  <SettingsIcon size={20} />
                  Dithering Settings
                </h2>
                <button
                  onClick={resetSettings}
                  className="p-1 hover:bg-button-hover rounded transition-colors"
                  title="Reset to defaults"
                >
                  <RotateCcw size={16} />
                </button>
              </div>

              {/* Algorithm Selection */}
              <div className="mb-4">
                <label className="block text-sm font-medium mb-2">
                  Algorithm
                </label>
                <select
                  value={settings.algorithm}
                  onChange={(e) => setSettings({ ...settings, algorithm: e.target.value })}
                  className="w-full px-3 py-2 bg-input-bg border border-panel-border rounded focus:outline-none focus:ring-2 focus:ring-accent"
                >
                  {ALGORITHMS.map(algo => (
                    <option key={algo.value} value={algo.value}>
                      {algo.label}
                    </option>
                  ))}
                </select>
                {selectedAlgorithm && (
                  <p className="text-xs text-gray-400 mt-1">
                    {selectedAlgorithm.description}
                  </p>
                )}
              </div>

              {/* Color Mode */}
              <div className="mb-4">
                <label className="block text-sm font-medium mb-2">
                  Color Mode
                </label>
                <select
                  value={settings.color_mode}
                  onChange={(e) => setSettings({ ...settings, color_mode: e.target.value })}
                  className="w-full px-3 py-2 bg-input-bg border border-panel-border rounded focus:outline-none focus:ring-2 focus:ring-accent"
                >
                  {COLOR_MODES.map(mode => (
                    <option key={mode.value} value={mode.value}>
                      {mode.label}
                    </option>
                  ))}
                </select>
              </div>

              {/* Colors */}
              <div className="mb-4">
                <label className="block text-sm font-medium mb-2">
                  Colors per Channel: {settings.colors}
                </label>
                <input
                  type="range"
                  min="2"
                  max="16"
                  step="1"
                  value={settings.colors}
                  onChange={(e) => setSettings({ ...settings, colors: parseInt(e.target.value) })}
                  className="w-full"
                />
                <div className="flex justify-between text-xs text-gray-400 mt-1">
                  <span>2 (B&W)</span>
                  <span>16 (High)</span>
                </div>
              </div>

              {/* Threshold */}
              <div className="mb-4">
                <label className="block text-sm font-medium mb-2">
                  Threshold: {settings.threshold.toFixed(2)}
                </label>
                <input
                  type="range"
                  min="0"
                  max="1"
                  step="0.01"
                  value={settings.threshold}
                  onChange={(e) => setSettings({ ...settings, threshold: parseFloat(e.target.value) })}
                  className="w-full"
                />
                <div className="flex justify-between text-xs text-gray-400 mt-1">
                  <span>0.0 (Dark)</span>
                  <span>1.0 (Light)</span>
                </div>
              </div>

              {/* Error Diffusion */}
              {['FloydSteinberg', 'Atkinson', 'JarvisJudiceNinke', 'Stucki', 'Burkes', 'Sierra', 'SierraTwoRow', 'SierraLite', 'FalseFloydSteinberg', 'StevenPigeon', 'Riemersma'].includes(settings.algorithm) && (
                <div className="mb-4">
                  <label className="block text-sm font-medium mb-2">
                    Error Diffusion: {settings.error_diffusion.toFixed(2)}
                  </label>
                  <input
                    type="range"
                    min="0"
                    max="1"
                    step="0.01"
                    value={settings.error_diffusion}
                    onChange={(e) => setSettings({ ...settings, error_diffusion: parseFloat(e.target.value) })}
                    className="w-full"
                  />
                  <div className="flex justify-between text-xs text-gray-400 mt-1">
                    <span>0.0 (None)</span>
                    <span>1.0 (Full)</span>
                  </div>
                </div>
              )}

              {/* Pattern Scale */}
              {['Pattern', 'HalftoneCircle', 'HalftoneDiamond'].includes(settings.algorithm) && (
                <div className="mb-4">
                  <label className="block text-sm font-medium mb-2">
                    Pattern Scale: {settings.pattern_scale}px
                  </label>
                  <input
                    type="range"
                    min="2"
                    max="32"
                    step="1"
                    value={settings.pattern_scale}
                    onChange={(e) => setSettings({ ...settings, pattern_scale: parseInt(e.target.value) })}
                    className="w-full"
                  />
                  <div className="flex justify-between text-xs text-gray-400 mt-1">
                    <span>2px (Fine)</span>
                    <span>32px (Coarse)</span>
                  </div>
                </div>
              )}

              {/* Serpentine Scanning */}
              {['FloydSteinberg', 'Atkinson', 'JarvisJudiceNinke', 'Stucki', 'Burkes', 'Sierra', 'SierraTwoRow', 'SierraLite', 'FalseFloydSteinberg', 'StevenPigeon'].includes(settings.algorithm) && (
                <div className="mb-4">
                  <label className="flex items-center gap-2 cursor-pointer">
                    <input
                      type="checkbox"
                      checked={settings.serpentine}
                      onChange={(e) => setSettings({ ...settings, serpentine: e.target.checked })}
                      className="w-4 h-4 text-accent bg-input-bg border-panel-border rounded focus:ring-accent"
                    />
                    <span className="text-sm">Serpentine Scanning</span>
                  </label>
                  <p className="text-xs text-gray-400 mt-1 ml-6">
                    Alternates scan direction per row for better results
                  </p>
                </div>
              )}

              {/* Image Info */}
              {imageInfo && (
                <div className="mt-6 pt-4 border-t border-panel-border">
                  <h3 className="text-sm font-medium mb-2 flex items-center gap-2">
                    <Layers size={16} />
                    Image Information
                  </h3>
                  <div className="text-xs space-y-1 text-gray-400">
                    <div>Dimensions: {imageInfo.width} × {imageInfo.height}px</div>
                    <div>Color Type: {imageInfo.color_type}</div>
                    <div>Total Pixels: {(imageInfo.width * imageInfo.height).toLocaleString()}</div>
                  </div>
                </div>
              )}
            </div>
          </div>
        )}
      </div>

      {/* Status Bar */}
      <div className="px-4 py-2 bg-panel-bg border-t border-panel-border text-sm flex items-center justify-between">
        <span>{statusMessage}</span>
        {imageInfo && (
          <span className="text-gray-400">
            {imageInfo.width} × {imageInfo.height}px
          </span>
        )}
      </div>
    </div>
  );
}

export default App;
