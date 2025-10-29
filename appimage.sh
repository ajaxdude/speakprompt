#!/bin/bash

set -e

APPDIR="SpeakPrompt.AppDir"
APP_NAME="SpeakPrompt"
VERSION="1.0.0"

echo "Creating AppImage for $APP_NAME v$VERSION..."

# Clean up previous build
rm -rf "$APPDIR"
mkdir -p "$APPDIR"

# Copy executable
mkdir -p "$APPDIR/usr/bin"
cp build/speakprompt "$APPDIR/usr/bin/"

# Copy desktop file
mkdir -p "$APPDIR/usr/share/applications"
cp speakprompt.desktop "$APPDIR/usr/share/applications/"

# Copy icon (create a simple one if not exists)
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
if [ ! -f "icon.png" ]; then
    echo "Creating a simple icon..."
    # Create a simple text-based icon using ImageMagick if available
    if command -v convert >/dev/null 2>&1; then
        convert -size 256x256 xc:lightblue -font Arial -pointsize 72 -fill darkblue -gravity center -annotate +0+0 "SP" icon.png 2>/dev/null || true
    fi
fi

if [ -f "icon.png" ]; then
    cp icon.png "$APPDIR/usr/share/icons/hicolor/256x256/apps/speakprompt.png"
fi

# Copy model file
mkdir -p "$APPDIR/usr/share/speakprompt/models"
if [ -f "models/ggml-base.en.bin" ]; then
    cp models/ggml-base.en.bin "$APPDIR/usr/share/speakprompt/models/"
fi

# Create AppRun
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${HERE}/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}"
export PATH="${HERE}/usr/bin:${PATH}"
exec "${HERE}/usr/bin/speakprompt" "$@"
EOF
chmod +x "$APPDIR/AppRun"

# Download and run appimagetool
if [ ! -f "appimagetool-x86_64.AppImage" ]; then
    echo "Downloading appimagetool..."
    wget -O appimagetool-x86_64.AppImage https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi

# Create the AppImage
ARCH=x86_64 ./appimagetool-x86_64.AppImage "$APPDIR" "${APP_NAME}-${VERSION}-x86_64.AppImage"

echo "AppImage created: ${APP_NAME}-${VERSION}-x86_64.AppImage"
