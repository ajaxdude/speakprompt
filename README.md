# SpeakPrompt

A Linux desktop application that provides real-time speech transcription for CLI developers. Use a simple interface to transcribe spoken prompts, making it easier to generate commands for AI coding assistants.

## Features

- **Simple Interface**: Clean terminal-based interface
- **Real-time Transcription**: Uses Whisper.cpp for live microphone transcription
- **Terminal Output**: Transcription results displayed in terminal for easy copy-paste
- **PipeWire/PulseAudio Support**: Works with modern Linux audio systems
- **Cross-platform Audio**: Supports both PipeWire and traditional PulseAudio systems

## Quick Start

### Building from Source

#### Fedora / RHEL / CentOS
```bash
# Install dependencies
sudo dnf install cmake gcc-c++ pulseaudio-libs-devel pkg-config

# Clone and build
git clone https://github.com/yourusername/speakprompt.git
cd speakprompt
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./build/speakprompt
```

#### Ubuntu / Debian
```bash
# Install dependencies
sudo apt update
sudo apt install cmake g++ libpulse-dev pkg-config

# Clone and build
git clone https://github.com/yourusername/speakprompt.git
cd speakprompt
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./build/speakprompt
```

#### Arch Linux
```bash
# Install dependencies
sudo pacman -S cmake gcc pulseaudio libpulse pkgconf

# Clone and build
git clone https://github.com/yourusername/speakprompt.git
cd speakprompt
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./build/speakprompt
```

### AppImage Usage

1. Download the latest AppImage from [Releases](https://github.com/yourusername/speakprompt/releases)
2. Make it executable:
   ```bash
   chmod +x SpeakPrompt-*.AppImage
   ```
3. Run:
   ```bash
   ./SpeakPrompt-*.AppImage
   ```

## Usage

1. Start SpeakPrompt application
2. Press Enter to start transcription
3. Speak your command/prompt clearly
4. Press Enter again to stop transcription
5. Copy the transcribed text from terminal output to your CLI tool

**Simple Controls:**
- `Enter` - Start/Stop transcription
- `Ctrl+C` - Quit application

## Requirements

- Linux (Fedora/Ubuntu/Arch tested)
- **PipeWire** or **PulseAudio** for audio capture
- C++17 compatible compiler
- CMake 3.16+

### Audio System Compatibility

SpeakPrompt automatically detects and works with:

- **PipeWire** (default on modern Fedora, Arch, Ubuntu 22.04+)
- **PulseAudio** (traditional Linux audio systems)
- **PulseAudio compatibility layer** on PipeWire systems

**If audio capture fails to work:**
```bash
# Check if audio server is running
pactl info

# On PipeWire systems, ensure pipewire-pulse is running
systemctl --user status pipewire pipewire-pulse

# Install missing audio development packages:
# Fedora/RHEL: sudo dnf install pulseaudio-libs-devel
# Ubuntu/Debian: sudo apt install libpulse-dev
# Arch Linux: sudo pacman -S libpulse
```

## Development

### Project Structure

```
speakprompt/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── gui.h/cpp             # GTK3 GUI interface
│   ├── audio_capture.h/cpp   # PulseAudio audio capture
│   ├── transcription_engine.h/cpp # Whisper.cpp integration
│   ├── hotkey_manager.h/cpp  # Global hotkey handling
│   └── terminal_output.h/cpp # Terminal output management
├── whisper.cpp/              # Whisper.cpp submodule
├── CMakeLists.txt            # CMake build configuration
├── build.sh                  # Build script
├── appimage.sh               # AppImage packaging script
└── README.md                 # This file
```

### Dependencies

**Required:**
- **C++17** compatible compiler
- **CMake** 3.16+
- **PkgConfig** 
- **PulseAudio development libraries** (for PipeWire compatibility)

**Audio System:**
- **PipeWire** with pulseaudio-libs-devel (modern systems)
- **PulseAudio** with libpulse-dev (traditional systems)

**Build-time:**
- **Whisper.cpp** (included as git submodule)
- **Threading library** (pthread)

**Note:** No GUI dependencies required - SpeakPrompt runs in terminal mode

## License

MIT License - see LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## Support

- Issues: [GitHub Issues](https://github.com/yourusername/speakprompt/issues)
- Discussions: [GitHub Discussions](https://github.com/yourusername/speakprompt/discussions)
