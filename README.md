# SpeakPrompt

A high-performance Linux terminal application that provides **real-time speech transcription with AI-powered text optimization** for CLI developers or anyone wanting to copy and paste speech-to-text. Uses optimized Whisper.cpp with Vulkan GPU acceleration for instant transcription, and LLaMA.cpp with local AI models to clean up and optimize transcribed text. In short you speak your prompt and AI cleans it up for you. Coded with Factory Droid using GLM 4.6 LLM as my first open-source project.

## ✨ Features

- **🚀 Real-time Streaming**: 2-second chunk processing with 1-second overlap for immediate feedback
- **⚡ Vulkan GPU Acceleration**: Utilizes AMD/NVIDIA GPUs for 10x faster transcription and AI processing
- **🎯 High Accuracy**: Large V3 Turbo model with optimized parameters
- **🧠 AI Text Optimization**: Local LLaMA.cpp integration with Magistral Small model for intelligent text cleanup
- **🎙️ Smart Audio Capture**: PipeWire/PulseAudio support with automatic detection
- **📝 Continuous Output**: Clean paragraph formatting without timestamps
- **⌨️ Simple Controls**: Enter to start/stop, Ctrl+C to quit

## :fire: Screenshot of SpeakPrompt in Action:

![alt text](https://github.com/ajaxdude/speakprompt/blob/main/pix/v1.png?raw=true)

## 🚀 Quick Start

### Prerequisites

#### Core Dependencies
```bash
# Fedora/RHEL
sudo dnf install cmake gcc-c++ pkg-config pulseaudio-libs-devel vulkan-devel SDL2 SDL2-devel

# Ubuntu/Debian
sudo apt install cmake g++ pkg-config libpulse-dev libvulkan-dev libsdl2-dev

# Arch Linux
sudo pacman -S cmake gcc pkgconf pulseaudio libpulse vulkan-devel sdl2

# openSUSE
sudo zypper install cmake gcc-c++ pkg-config pulseaudio-devel vulkan-devel libSDL2-devel

# Solus
sudo eopkg install cmake gcc pkgconfig pulseaudio-devel vulkan-devel sdl2-devel
```

#### Optional GPU Drivers (for Vulkan acceleration)
```bash
# AMD GPUs (most distributions)
# Usually included with mesa drivers

# NVIDIA GPUs
# Ubuntu/Debian: sudo apt install nvidia-driver-535
# Fedora: sudo dnf install akmod-nvidia
# Arch: sudo pacman -S nvidia

# Intel GPUs (integrated graphics usually work out of the box)
# Ubuntu: sudo apt install intel-media-driver
# Fedora: sudo dnf install intel-media-driver
```

### Build & Run

```bash
# Clone the repository
git clone https://github.com/ajaxdude/speakprompt.git
cd speakprompt

# Configure with Vulkan and SDL2 support
mkdir build && cd build
cmake .. -DGGML_VULKAN=1 -DWHISPER_SDL2=ON

# Build
make -j$(nproc)

# Download the optimized Whisper model (optional - auto-downloads if missing)
wget -O ../models/ggml-large-v3-turbo.bin https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-large-v3-turbo.bin

# Download the LLM model for text optimization (optional but recommended)
mkdir -p ../models/llm
wget -O ../models/llm/Magistral-Small-2509-Q4_K_M.gguf https://huggingface.co/MaziyarPanahi/Magistral-Small-2509-GGUF/resolve/main/Magistral-Small-2509-Q4_K_M.gguf

# Run
./speakprompt
```

## 📖 Usage

1. **Start the application**: `./speakprompt`
2. **Press Enter** to begin transcription → Shows `[STATUS] ON AIR`
3. **Speak clearly** into your microphone
4. **Watch real-time transcription** appear as continuous text
5. **Press Enter** again to stop → Shows `[STATUS] OFF AIR`
6. **AI Optimization**: The app will automatically optimize your transcribed text using the local LLM
7. **Copy** the optimized text for use in CLI tools

### Controls
- `Enter` - Toggle recording ON/OFF
- `Ctrl+C` - Quit application

### AI Text Optimization
When you stop recording, the application automatically:
- Removes filler words (um, uh, like, you know)
- Fixes grammar and sentence structure
- Makes text more concise and coherent
- Preserves original meaning and key points
- Organizes rambling thoughts into clear sentences

*Note: The LLM model is optional. If not found, the app will provide raw transcriptions.*

## 🎯 Performance

- **Response Time**: 2-3 seconds (vs 30+ seconds in basic implementations)
- **Transcription Model**: Large V3 Turbo (1.6GB) with GPU acceleration
- **AI Model**: Magistral Small (13GB) with Vulkan GPU acceleration
- **Processing**: 8-thread parallel processing with Vulkan
- **Audio**: Real-time 16kHz, 16-bit mono capture
- **Output**: Clean, AI-optimized text without timestamps or silence markers

## ☕ Buy Me a Coffee on COINBASE

If you find SpeakPrompt useful and want to support my open-source development work, consider buying me a coffee! Your support helps me continue developing and maintaining this project.

![Bitcoin Address](https://github.com/ajaxdude/speakprompt/blob/main/pix/bitcoinaddress.png?raw=true)

## 🔧 Configuration

### Whisper Model Priority (auto-selects first available):
1. `ggml-large-v3-turbo.bin` (Fastest & most accurate)
2. `ggml-base.en.bin` (Fallback option)

### LLM Model for Text Optimization:
- **Model**: `Magistral-Small-2509-Q4_K_M.gguf` (13GB, 4-bit quantized)
- **Location**: `models/llm/` directory
- **GPU**: Uses Vulkan for acceleration (CPU fallback if GPU unavailable)
- **Optional**: Application works without LLM model (raw transcription only)

### Audio System Support:
- **PipeWire** (Modern Linux - Fedora, Arch, Ubuntu 22.04+)
- **PulseAudio** (Traditional systems)
- **Automatic detection** with fallback support

## 📁 Project Structure

```
speakprompt/
├── src/
│   ├── main_simple.cpp          # Entry point & user interaction
│   ├── audio_capture.h/cpp      # Real-time audio capture (mic/WAV)
│   ├── transcription_engine.h/cpp # Whisper.cpp integration
│   ├── terminal_output.h/cpp    # Clean output formatting
│   └── llm_processor.h/cpp      # LLaMA.cpp integration for AI text optimization
├── models/
│   ├── ggml-large-v3-turbo.bin  # Whisper transcription model
│   └── llm/                     # LLM models directory
│       └── Magistral-Small-2509-Q4_K_M.gguf  # AI text optimization model
├── whisper.cpp/                 # Whisper.cpp submodule
├── llama.cpp/                   # LLaMA.cpp submodule
├── CMakeLists.txt               # Build configuration
└── README.md                    # This file
```

## 🛠️ Technical Details

### Real-time Processing
- **Chunk Size**: 2 seconds with 1-second overlap
- **Thread Pool**: 8 parallel processing threads
- **GPU Backend**: Vulkan with matrix acceleration
- **Audio Buffer**: Continuous streaming with smart overlap

### Output Formatting
- **No timestamps** - Clean, readable text
- **Continuous paragraphs** - No line breaks
- **Silence filtering** - No dots or blank markers
- **Status indicators** - ON AIR / OFF AIR

## 🔍 Troubleshooting

### Audio Issues
```bash
# Check audio system
pactl info

# Verify PipeWire is running
systemctl --user status pipewire pipewire-pulse

# Test microphone
arecord -d 5 test.wav && aplay test.wav

# Check available audio devices
pactl list sources

# Restart audio service if needed
systemctl --user restart pipewire pipewire-pulse
```

### SDL2 Issues
```bash
# Check if SDL2 is installed
pkg-config --modversion sdl2

# Install SDL2 if missing
# Ubuntu/Debian: sudo apt install libsdl2-dev
# Fedora/RHEL: sudo dnf install SDL2 SDL2-devel
# Arch Linux: sudo pacman -S sdl2

# Verify SDL2 detection
cmake .. -DWHISPER_SDL2=ON | grep SDL2
```

### GPU/Vulkan Issues
```bash
# Check Vulkan support
vulkaninfo --summary

# Install GPU drivers if needed
# AMD: sudo dnf install mesa-vulkan-drivers
# NVIDIA: Install proprietary drivers

# Test Vulkan with a simple program
vkcube  # Should show a rotating cube if Vulkan works
```

### Build Issues
```bash
# Clean build
rm -rf build && mkdir build && cd build
cmake .. -DGGML_VULKAN=1 -DWHISPER_SDL2=ON
make -j$(nproc)

# If build fails due to missing dependencies:
# Ubuntu/Debian:
sudo apt install cmake g++ pkg-config libpulse-dev libvulkan-dev libsdl2-dev

# Fedora/RHEL:
sudo dnf install cmake gcc-c++ pkg-config pulseaudio-libs-devel vulkan-devel SDL2 SDL2-devel

# Arch Linux:
sudo pacman -S cmake gcc pkgconf pulseaudio libpulse vulkan-devel sdl2
```

### Model Issues

#### Whisper Model
```bash
# Check if model exists
ls -la models/ggml-large-v3-turbo.bin

# Download model if missing
mkdir -p models
wget -O models/ggml-large-v3-turbo.bin https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-large-v3-turbo.bin

# Alternative model (smaller, faster download)
wget -O models/ggml-base.en.bin https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin
```

#### LLM Model (for AI text optimization)
```bash
# Check if LLM model exists
ls -la models/llm/Magistral-Small-2509-Q4_K_M.gguf

# Download LLM model if missing (13GB - large download)
mkdir -p models/llm
wget -O models/llm/Magistral-Small-2509-Q4_K_M.gguf https://huggingface.co/MaziyarPanahi/Magistral-Small-2509-GGUF/resolve/main/Magistral-Small-2509-Q4_K_M.gguf

# If LLM model loading takes too long, try reducing GPU layers or using CPU only
# Edit src/llm_processor.cpp and change model_params.n_gpu_layers = 0;
```

### LLM Processing Issues
```bash
# Check if Vulkan is working for LLM acceleration
vulkaninfo --summary

# If LLM is slow, ensure you have enough VRAM (at least 16GB recommended)
# For lower VRAM systems, the model may fall back to CPU processing

# Application will still work without LLM model - it will provide raw transcriptions
```

## 📄 License

MIT License - see LICENSE file for details.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test performance improvements
5. Submit a pull request

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/speakprompt/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/speakprompt/discussions)

---

**Built with ❤️ for CLI developers who want fast, accurate speech-to-text.**
