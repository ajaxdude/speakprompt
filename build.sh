#!/bin/bash

set -e

echo "Building SpeakPrompt..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

echo "Build complete. Executable: build/speakprompt"

# Download Whisper model if not exists
MODEL_DIR="../models"
MODEL_FILE="$MODEL_DIR/ggml-base.en.bin"

if [ ! -f "$MODEL_FILE" ]; then
    echo "Downloading Whisper base English model..."
    mkdir -p "$MODEL_DIR"
    cd "$MODEL_DIR"
    
    # Download using wget or curl
    if command -v wget >/dev/null 2>&1; then
        wget -O ggml-base.en.bin https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin
    elif command -v curl >/dev/null 2>&1; then
        curl -L -o ggml-base.en.bin https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin
    else
        echo "Error: Neither wget nor curl found. Please download the model manually."
        echo "Download from: https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin"
        echo "Place it in: $MODEL_FILE"
        exit 1
    fi
    
    echo "Model downloaded successfully."
    cd ../../build || cd ../build
else
    echo "Whisper model already exists: $MODEL_FILE"
fi

echo "Ready to run: ./build/speakprompt"
