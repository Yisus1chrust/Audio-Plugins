# Photo Synth Plugin - Build Instructions

## ⚠️ IMPORTANT: Compilation Requirements

This JUCE C++ project provides the **complete source code** for the Photo Synth VST3/AU plugin. To obtain working plugin binaries (.vst3 and .component files), you **must compile the project on your own machine** using the appropriate development tools.

### Why Can't This Be Pre-Compiled?

- **VST3 (Windows)**: Requires Visual Studio 2019+ with C++ Desktop Development workload
- **AU (macOS Only)**: Requires Xcode 13+ on macOS (Audio Units cannot be built on Windows/Linux)
- **Code Signing**: macOS plugins must be code-signed to avoid Gatekeeper warnings
- **Architecture**: Plugins must be built for your specific CPU architecture (Intel x64 or Apple Silicon ARM64)

---

## Platform-Specific Build Instructions

### macOS (for AU + VST3)

#### Prerequisites
1. **Xcode 13+**: Install from Mac App Store
2. **Command Line Tools**: 
   ```bash
   xcode-select --install
   ```
3. **CMake 3.15+**: 
   ```bash
   brew install cmake
   ```

#### Build Steps
1. Open Terminal and navigate to project:
   ```bash
   cd /path/to/photo_synth_plugin
   ```

2. Configure CMake (creates Xcode project):
   ```bash
   cmake -B Builds -G Xcode
   ```

3. **Option A - Command Line Build:**
   ```bash
   cmake --build Builds --config Release
   ```

4. **Option B - Xcode GUI Build:**
   ```bash
   open Builds/PhotoSynth.xcodeproj
   ```
   - Select "PhotoSynth_AU" or "PhotoSynth_VST3" scheme
   - Build → Build (⌘B)

#### Installation Locations
After successful build:
- **AU Plugin**: `~/Library/Audio/Plug-Ins/Components/Photo Synth.component`
- **VST3 Plugin**: `~/Library/Audio/Plug-Ins/VST3/Photo Synth.vst3`
- **Standalone App**: `Builds/PhotoSynth_artefacts/Release/Standalone/Photo Synth.app`

#### Code Signing (Recommended)
```bash
# Self-sign for local testing
codesign --force --sign - "~/Library/Audio/Plug-Ins/Components/Photo Synth.component"
codesign --force --sign - "~/Library/Audio/Plug-Ins/VST3/Photo Synth.vst3"
```

For distribution, replace `-` with your Apple Developer certificate ID.

---

### Windows (for VST3 Only)

#### Prerequisites
1. **Visual Studio 2019/2022**: Install "Desktop development with C++" workload
2. **CMake 3.15+**: Download from https://cmake.org/download/

#### Build Steps
1. Open Command Prompt (cmd) or PowerShell:
   ```cmd
   cd C:\path\to\photo_synth_plugin
   ```

2. Configure CMake (creates Visual Studio solution):
   ```cmd
   cmake -B Builds -G "Visual Studio 17 2022" -A x64
   ```
   (Use "Visual Studio 16 2019" if using VS 2019)

3. **Option A - Command Line Build:**
   ```cmd
   cmake --build Builds --config Release
   ```

4. **Option B - Visual Studio GUI Build:**
   - Open `Builds\PhotoSynth.sln` in Visual Studio
   - Set "PhotoSynth_VST3" as StartUp Project
   - Build → Build Solution (Ctrl+Shift+B)

#### Installation Location
After successful build:
- **VST3 Plugin**: `C:\Program Files\Common Files\VST3\Photo Synth.vst3`

---

## Troubleshooting

### "JUCE not found" Error
The CMakeLists.txt uses `FetchContent` to automatically download JUCE 7.0.12. Ensure:
- You have internet connection during first CMake configuration
- CMake version is 3.15 or higher: `cmake --version`

### macOS Gatekeeper Warning
If you see "Photo Synth.component is damaged and can't be opened":
```bash
xattr -cr ~/Library/Audio/Plug-Ins/Components/Photo\ Synth.component
codesign --force --sign - ~/Library/Audio/Plug-Ins/Components/Photo\ Synth.component
```

### VST3 Not Showing in DAW (Windows)
1. Verify plugin is in correct folder: `C:\Program Files\Common Files\VST3\`
2. Rescan plugins in your DAW settings
3. Check DAW supports VST3 (not just VST2)

### Build Errors About Missing Headers
Ensure JUCE downloaded correctly:
```bash
# Clean and reconfigure
rm -rf Builds
cmake -B Builds -G Xcode  # macOS
# or
cmake -B Builds -G "Visual Studio 17 2022" -A x64  # Windows
```

---

## Testing Your Plugin

### Standalone App (Quick Test)
The easiest way to test before installing to DAW:
```bash
# macOS
./Builds/PhotoSynth_artefacts/Release/Standalone/Photo\ Synth.app/Contents/MacOS/Photo\ Synth

# Windows
Builds\PhotoSynth_artefacts\Release\Standalone\Photo Synth.exe
```

### In Your DAW
1. Launch your DAW (Ableton Live, Logic Pro, FL Studio, etc.)
2. Create a new track → Add instrument
3. Search for "Photo Synth" in plugin list
4. Drag an image into the plugin window
5. Play notes on MIDI keyboard to hear image-derived sounds

---

## Selling Your Plugin Online

If you plan to distribute/sell Photo Synth commercially:

### macOS Distribution
1. **Get Apple Developer Account** ($99/year)
2. **Code Sign with Distribution Certificate**:
   ```bash
   codesign --deep --force --verify --verbose \
     --sign "Developer ID Application: Your Name (TEAMID)" \
     --timestamp \
     --options runtime \
     "Photo Synth.component"
   ```
3. **Notarize** (required for macOS 10.15+):
   ```bash
   xcrun notarytool submit Photo\ Synth.component.zip \
     --apple-id "your@email.com" \
     --password "app-specific-password" \
     --team-id TEAMID
   ```
4. **Staple Notarization**:
   ```bash
   xcrun stapler staple "Photo Synth.component"
   ```

### Windows Distribution
1. **Optional Code Signing** (not required but recommended):
   - Purchase code signing certificate from DigiCert, Sectigo, etc.
   - Use `signtool.exe` to sign .vst3 file
2. **No notarization** equivalent on Windows

### Installer Packaging
Consider using:
- **macOS**: Packages (built-in `pkgbuild`) or JUCE Projucer installer
- **Windows**: Inno Setup or NSIS

---

## Development & Modifications

To modify the plugin:
1. Edit C++ source files in `Source/` directory
2. Rebuild: `cmake --build Builds --config Release`
3. Test changes in standalone or DAW

Key files:
- `Source/PluginProcessor.cpp` - Audio processing, parameter system
- `Source/SynthVoice.cpp` - Voice allocation, DSP synthesis
- `Source/ImageAnalyzer.cpp` - Image metrics extraction
- `Source/PluginEditor.cpp` - UI layout, drag-and-drop
- `Source/EffectsChain.cpp` - Reverb, delay, etc.

---

## License & Commercial Use

This project is provided as-is. JUCE framework has its own licensing:
- **GPL License**: Free for open-source projects
- **Commercial License**: Required if selling closed-source ($40-900/year)
  See: https://juce.com/juce-7-license/

For Photo Synth commercial distribution, you may need a JUCE Commercial License.

---

## Support Resources

- **JUCE Forum**: https://forum.juce.com/
- **JUCE Documentation**: https://docs.juce.com/
- **CMake Docs**: https://cmake.org/documentation/

