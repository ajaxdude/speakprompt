#!/bin/bash

echo "Testing SpeakPrompt application..."

# Test 1: Check if executable exists
if [ ! -f "build/speakprompt" ]; then
    echo "ERROR: speakprompt executable not found"
    exit 1
fi

echo "✓ Executable exists"

# Test 2: Check if model exists
if [ ! -f "models/ggml-base.en.bin" ]; then
    echo "ERROR: Whisper model not found"
    exit 1
fi

echo "✓ Whisper model exists"

# Test 3: Run a quick test (timeout after 5 seconds)
echo "✓ Running quick functionality test..."
timeout 5s ./build/speakprompt < /dev/null > /tmp/speakprompt_test.log 2>&1

# Check if the application started correctly
if grep -q "SpeakPrompt - Simple Speech-to-Text" /tmp/speakprompt_test.log; then
    echo "✓ Application starts correctly"
else
    echo "ERROR: Application failed to start"
    cat /tmp/speakprompt_test.log
    exit 1
fi

# Check if Whisper model loaded
if grep -q "Whisper model loaded successfully" /tmp/speakprompt_test.log; then
    echo "✓ Whisper model loads correctly"
else
    echo "ERROR: Whisper model failed to load"
    cat /tmp/speakprompt_test.log
    exit 1
fi

# Clean up
rm -f /tmp/speakprompt_test.log

echo ""
echo "All tests passed! ✅"
echo ""
echo "To run the application:"
echo "  ./build/speakprompt"
echo ""
echo "To create an AppImage:"
echo "  ./appimage.sh"
