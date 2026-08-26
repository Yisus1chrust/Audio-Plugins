# Photo Synth (JUCE 7) — VST3/AU/Standalone

This project is a JUCE 7 C++ rebuild scaffold of the uploaded Photo Synth WebAudio/React instrument.

## Implemented in this project

- JUCE CMake plugin project with targets: **VST3**, **AU**, **Standalone**
- 16-voice polyphonic synthesizer with note stealing
- 6 engine modes:
  - `analog_brass`
  - `digital_fm_bells`
  - `hybrid_wavetable`
  - `acoustic_piano_organ`
  - `overdriven_saw_stack`
  - `fm_square_bell`
- Image analyzer module:
  - FNV-1a 64-bit hashing
  - brightness/saturation extraction
  - Sobel complexity analysis
  - temporal era inference
  - patch generation from image features
- Morph engine for blending image-derived metrics and patch states
- Physical modeling stage:
  - body resonance filters
  - tape flutter modulation
  - analog saturation transfer curve
- 6 effects stage:
  - delay
  - reverb
  - chorus
  - phaser
  - flanger
  - distortion
- APVTS parameter system for synth/effect/model controls
- Preset save/load (`*.photosynthpreset` XML)
- UI layout inspired by React app:
  - left gauges panel
  - center image dropzone
  - right parameter knobs panel
  - oscilloscope
  - virtual MIDI keyboard

## Project layout

- `Source/` — C++ source files
- `Assets/` — bundled images/logo
- `Builds/` — generated CMake build directory
- `CMakeLists.txt` — root CMake build

## Build prerequisites

- CMake 3.22+
- C++20 compiler
- JUCE 7.x source
- macOS for AU build

## Configure JUCE

Either:
1. Add JUCE as submodule at `./JUCE`, or
2. Provide `-DJUCE_DIR=/absolute/path/to/JUCE`

## Build commands

### macOS (AU + VST3 + Standalone)
```bash
cd /home/ubuntu/photo_synth_plugin
cmake -S . -B Builds -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR=/path/to/JUCE
cmake --build Builds --config Release
```

### Windows (VST3 + Standalone)
```bash
cd /home/ubuntu/photo_synth_plugin
cmake -S . -B Builds -G "Visual Studio 17 2022" -A x64 -DJUCE_DIR=C:/JUCE
cmake --build Builds --config Release
```

## Notes

- Default tuning assumes 44.1 kHz but supports host rates up to 192 kHz.
- Audio parameter smoothing uses 20 ms ramps for core synth controls.
- Limiter defaults: threshold -3 dB, ratio 20:1, attack 0.5 ms.
- Buffer sizes 64–2048 are supported by JUCE host negotiation.

## Output locations

- VST3: JUCE-generated VST3 output folder under `Builds/`
- AU (macOS): JUCE-generated AU component output folder under `Builds/`
- Standalone app: JUCE-generated standalone target output folder

