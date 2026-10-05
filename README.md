# CTB Vocal Suite v0.2

**Capture The Beats — all-in-one vocal processor**

This repository is configured so the macOS plugin can be built remotely with **GitHub Actions**. You do not need Xcode to compile it yourself.

## Signal chain

`Input → HPF → Compressor → Saturation → Presence → Air → De-Esser → Limiter → Output → Mix`

## Controls

- Input: -24 to +24 dB
- HPF: 20–500 Hz
- Presence: ±12 dB around 3.5 kHz
- Air: ±12 dB shelf at 10 kHz
- Compressor threshold, ratio, attack and release
- De-Esser: 0–100%
- Saturation: 0–100%
- Output: -24 to +12 dB
- Mix: 0–100%

## Build without Xcode

1. Create a free GitHub account if you do not already have one.
2. Create a **new empty repository** on GitHub, for example `ctb-vocal-suite`.
3. Upload **the contents of this folder** to that repository. Make sure `.github/workflows/build-macos.yml` is included.
4. Open the repository on GitHub.
5. Go to **Actions → Build CTB Vocal Suite**.
6. Choose **Run workflow**.
7. When the workflow finishes, open the completed run and download the artifact named:

   `CTB-Vocal-Suite-macOS-arm64`

8. The ZIP contains the compiled **VST3** and **AU** plugins.

The workflow targets **Apple Silicon arm64**, suitable for an M-series Mac such as the MacBook Pro M4 Pro.

## Installing the VST3 on your Mac

After downloading and unzipping the artifact, copy `CTB Vocal Suite.vst3` to:

`~/Library/Audio/Plug-Ins/VST3/`

For the AU version, copy the `.component` bundle to:

`~/Library/Audio/Plug-Ins/Components/`

Then restart/rescan plugins in your DAW.

## Local build

If you later want to build locally, CMake 3.22+ can fetch JUCE 8.0.10 automatically:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

## Development notes

This is an early functional DSP release. The next versions can add:

- Input/output and gain-reduction meters
- Preset browser
- Proper band-split de-esser
- Oversampling
- Better compressor character controls
- Vocal-specific EQ bands
- High-quality saturation modes
- CTB factory presets
- Larger polished commercial-style interface
- Universal macOS build (arm64 + Intel) if needed
