# SpeakPrompt

A high-performance Linux terminal application that provides **real-time speech transcription** for CLI developers. Uses optimized Whisper.cpp with Vulkan GPU acceleration for instant, accurate transcription of spoken commands and prompts.

## ✨ Features

- **🚀 Real-time Streaming**: 2-second chunk processing with 1-second overlap for immediate feedback
- **⚡ Vulkan GPU Acceleration**: Utilizes AMD/NVIDIA GPUs for 10x faster transcription
- **🎯 High Accuracy**: Large V3 Turbo model with optimized parameters
- **🎙️ Smart Audio Capture**: PipeWire/PulseAudio support with automatic detection
- **📝 Continuous Output**: Clean paragraph formatting without timestamps
- **⌨️ Simple Controls**: Enter to start/stop, Ctrl+C to quit

## 🚀 Quick Start

### Prerequisites

```bash
# Fedora/RHEL
sudo dnf install cmake gcc-c++ pulseaudio-libs-devel pkg-config vulkan-devel

# Ubuntu/Debian  
sudo apt install cmake g++ libpulse-dev pkg-config libvulkan-dev

# Arch Linux
sudo pacman -S cmake gcc pulseaudio libpulse pkgconf vulkan-devel
```

### Build & Run

```bash
# Clone the repository
git clone https://github.com/yourusername/speakprompt.git
cd speakprompt

# Configure with Vulkan support
mkdir build && cd build
cmake .. -DGGML_VULKAN=1

# Build
make -j$(nproc)

# Download the optimized model (optional - auto-downloads if missing)
wget -O ../models/ggml-large-v3-turbo.bin https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-large-v3-turbo.bin

# Run
./speakprompt
```

## 📖 Usage

1. **Start the application**: `./speakprompt`
2. **Press Enter** to begin transcription → Shows `[STATUS] ON AIR`
3. **Speak clearly** into your microphone
4. **Watch real-time transcription** appear as continuous text
5. **Press Enter** again to stop → Shows `[STATUS] OFF AIR`
6. **Copy** the transcribed text for use in CLI tools

### Controls
- `Enter` - Toggle recording ON/OFF
- `Ctrl+C` - Quit application

## 🎯 Performance

- **Response Time**: 2-3 seconds (vs 30+ seconds in basic implementations)
- **Model**: Large V3 Turbo (1.6GB) with GPU acceleration
- **Processing**: 8-thread parallel processing with Vulkan
- **Audio**: Real-time 16kHz, 16-bit mono capture
- **Output**: Clean paragraph format without timestamps or silence markers

## 🔧 Configuration

### Model Priority (auto-selects first available):
1. `ggml-large-v3-turbo.bin` (Fastest & most accurate)
2. `ggml-base.en.bin` (Fallback option)

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
│   └── terminal_output.h/cpp    # Clean output formatting
├── models/                      # Whisper model files
├── whisper.cpp/                 # Whisper.cpp submodule
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

**Audio Issues:**
```bash
# Check audio system
pactl info

# Verify PipeWire is running
systemctl --user status pipewire pipewire-pulse

# Test microphone
arecord -d 5 test.wav && aplay test.wav
```

**GPU Issues:**
```bash
# Check Vulkan support
vulkaninfo --summary

# Install GPU drivers if needed
# AMD: sudo dnf install mesa-vulkan-drivers
# NVIDIA: Install proprietary drivers
```

**Build Issues:**
```bash
# Clean build
rm -rf build && mkdir build && cd build
cmake .. -DGGML_VULKAN=1
make -j$(nproc)
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
