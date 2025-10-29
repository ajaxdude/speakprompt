# SpeakPrompt

A Linux desktop application that provides real-time speech transcription for CLI developers. Use a global hotkey to start/pause transcription of spoken prompts, making it easier to generate commands for AI coding assistants.

## Features

- **Global Hotkey Support**: Configure custom hotkey to start/pause transcription
- **Real-time Transcription**: Uses Whisper.cpp for live microphone transcription
- **Terminal Output**: Transcription results displayed in terminal for easy copy-paste
- **AppImage Distribution**: Self-contained executable with minimal dependencies
- **Simple GUI**: Minimal interface for hotkey configuration and status display

## Quick Start

### Building from Source

1. Install dependencies (Fedora):
   ```bash
   sudo dnf install cmake gcc-c++ gtk3-devel pulseaudio-libs-devel libX11-devel
   ```

2. Clone and build:
   ```bash
   git clone --recursive https://github.com/yourusername/speakprompt.git
   cd speakprompt
   ./build.sh
   ```

3. Run:
   ```bash
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
2. Configure your preferred hotkey (default: Ctrl+Shift+Space)
3. Press the hotkey to start transcription
4. Speak your command/prompt clearly
5. Press the hotkey again to stop transcription
6. Copy the transcribed text from terminal to your CLI tool

## Requirements

- Linux (Fedora/Ubuntu tested)
- PulseAudio or ALSA for audio capture
- X11 for global hotkey support
- GTK3 libraries

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

- **C++17** compatible compiler
- **CMake** 3.16+
- **GTK3** development libraries
- **PulseAudio** development libraries
- **X11** development libraries
- **Whisper.cpp** (included as submodule)

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
