# SpeakPrompt Project Specification

## Project Overview
SpeakPrompt is a Linux desktop application that provides real-time speech transcription for CLI developers. The application allows users to use a global hotkey to start/pause transcription of spoken prompts, making it easier to generate commands for AI coding assistants.

## Key Features
- **Global Hotkey Support**: User can configure a custom hotkey to start/pause transcription
- **Real-time Transcription**: Uses Whisper.cpp for live microphone transcription
- **Terminal Output**: Transcription results are displayed in terminal for easy copy-paste into CLI tools
- **AppImage Distribution**: Self-contained executable with minimal dependencies
- **Simple GUI**: Minimal interface for hotkey configuration and status display

## Technical Requirements

### Architecture
- **Language**: C++ (for Whisper.cpp integration and GUI)
- **GUI Framework**: GTK3 or Qt5 (for cross-platform compatibility and AppImage support)
- **Audio Capture**: PulseAudio or ALSA for microphone input
- **Hotkey System**: Global hotkey interception using X11 or Wayland
- **Speech Recognition**: Embedded Whisper.cpp library
- **Build System**: CMake with AppImage packaging

### Dependencies
- Whisper.cpp (embedded)
- GTK3/Qt5
- PulseAudio/ALSA development libraries
- X11 or Wayland libraries
- AppImage tools

### Core Components
1. **Main Application**: GUI window with hotkey configuration
2. **Audio Capture**: Real-time microphone input processing
3. **Transcription Engine**: Whisper.cpp integration
4. **Hotkey Manager**: Global hotkey detection and handling
5. **Output Manager**: Terminal integration for transcription display

## User Workflow
1. User starts SpeakPrompt application
2. GUI shows current hotkey and status
3. User presses configured hotkey to start transcription
4. Real-time transcription appears in terminal
5. User presses hotkey again to stop/pause
6. Transcription text can be copied and used in CLI tools

## Development Phases
1. **Project Setup**: OpenSpec initialization and basic structure
2. **GUI Development**: Basic interface with hotkey configuration
3. **Audio Integration**: Microphone capture and processing
4. **Whisper Integration**: Real-time transcription implementation
5. **Hotkey System**: Global hotkey detection
6. **Terminal Output**: Integration with terminal applications
7. **AppImage Packaging**: Creating distributable AppImage
8. **GitHub Repository**: Project hosting and documentation

## Quality Requirements
- Real-time transcription with minimal latency
- Stable hotkey detection system
- Cross-platform compatibility (Fedora Linux focus)
- Minimal external dependencies
- Clean, efficient GUI with low resource usage
