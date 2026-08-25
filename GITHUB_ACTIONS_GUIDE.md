# GitHub Actions - Automated Plugin Builds

## 🎉 Your Plugin Builds Automatically!

Every time you push code to this repo, GitHub Actions will automatically:
- ✅ Build VST3 + AU for **macOS**
- ✅ Build VST3 for **Windows**
- ✅ Build VST3 for **Linux** (testing only)

**Cost: $0** (GitHub Actions free tier gives you 2,000 minutes/month)

---

## How to Download Your Compiled Plugins

### Step 1: View Build Status
1. Go to your repo: https://github.com/Yisus1chrust/Plugins
2. Click the **"Actions"** tab at the top
3. You'll see a list of build runs

### Step 2: Wait for Build to Complete
- **Green checkmark ✅** = Build succeeded
- **Yellow circle 🟡** = Build in progress (wait 5-10 minutes)
- **Red X ❌** = Build failed (check logs)

### Step 3: Download Plugin Files
1. Click on the most recent **successful** build (green checkmark)
2. Scroll down to **"Artifacts"** section
3. Download:
   - **PhotoSynth-macOS** → Contains `.vst3` and `.component` (AU) files
   - **PhotoSynth-Windows** → Contains `.vst3` file
   - **PhotoSynth-Linux** → Contains `.vst3` (for testing only)

### Step 4: Install Plugins on Your Computer

#### macOS:
```bash
# Unzip the downloaded PhotoSynth-macOS.zip
cd ~/Downloads
unzip PhotoSynth-macOS.zip

# Move plugins to system folders
cp -R PhotoSynth.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R PhotoSynth.component ~/Library/Audio/Plug-Ins/Components/

# Remove quarantine (macOS security)
xattr -cr ~/Library/Audio/Plug-Ins/VST3/PhotoSynth.vst3
xattr -cr ~/Library/Audio/Plug-Ins/Components/PhotoSynth.component
```

#### Windows:
```cmd
# Extract PhotoSynth-Windows.zip
# Copy PhotoSynth.vst3 to:
C:\Program Files\Common Files\VST3\
```

---

## Triggering Manual Builds

You can trigger builds without pushing code:

1. Go to **Actions** tab
2. Click **"Build Photo Synth Plugin"** on the left
3. Click **"Run workflow"** button on the right
4. Select branch: `main`
5. Click green **"Run workflow"** button

---

## Understanding Build Times

- **First build**: 8-12 minutes (JUCE download + compilation)
- **Subsequent builds**: 5-8 minutes (JUCE cached)
- **All 3 platforms run in parallel** (not sequential)

---

## Troubleshooting

### ❌ Build Failed - How to Fix

1. Click on the failed build
2. Click on the failed job (e.g., "macOS Build")
3. Expand the red step to see error message
4. Common fixes:
   - **CMake error**: Check `CMakeLists.txt` syntax
   - **Compile error**: Check C++ source code in `Source/`
   - **JUCE not found**: GitHub Actions auto-downloads it, so just retry

### ⚠️ No Artifacts Found

If you see "No artifacts were uploaded" warning:
- The build might have failed before packaging
- Check build logs for errors
- Verify plugin actually compiled by looking at "Build Plugin" step logs

### 🔒 macOS Gatekeeper Warnings

Downloaded plugins will show "damaged" warnings because they're not code-signed.

**Fix:**
```bash
xattr -cr ~/Library/Audio/Plug-Ins/VST3/PhotoSynth.vst3
xattr -cr ~/Library/Audio/Plug-Ins/Components/PhotoSynth.component
```

For **commercial distribution**, you need:
1. Apple Developer Account ($99/year)
2. Add signing secrets to GitHub Actions (see below)

---

## Adding Code Signing (Optional - For Distribution)

If you want to **sell** this plugin, you need to code-sign it.

### macOS Code Signing Setup:

1. Get Apple Developer Account
2. Generate certificates in Xcode
3. Export certificate as `.p12` file
4. Go to repo **Settings** → **Secrets and variables** → **Actions**
5. Add these secrets:
   - `MACOS_CERTIFICATE`: Base64-encoded `.p12` file
   - `MACOS_CERTIFICATE_PASSWORD`: Your certificate password
   - `KEYCHAIN_PASSWORD`: Any random password
6. Modify `.github/workflows/build-plugins.yml` to add signing steps

---

## File Sizes & Download Speeds

- **macOS artifacts**: ~15-25 MB (VST3 + AU + Standalone)
- **Windows artifacts**: ~8-15 MB (VST3 + Standalone)
- **Download time**: 5-10 seconds

---

## What's Being Built

### macOS (runs on macos-latest):
- ✅ `PhotoSynth.vst3` - Universal (Intel + Apple Silicon)
- ✅ `PhotoSynth.component` - Audio Unit (AU)
- ✅ `PhotoSynth.app` - Standalone app

### Windows (runs on windows-latest):
- ✅ `PhotoSynth.vst3` - 64-bit Windows
- ✅ `PhotoSynth.exe` - Standalone app

### Linux (runs on ubuntu-latest):
- ✅ `PhotoSynth.vst3` - For testing only (not for macOS/Windows DAWs)

---

## Making Changes to the Plugin

1. Edit source files in `Source/` directory
2. Commit and push changes:
   ```bash
   git add .
   git commit -m "Updated filter algorithm"
   git push
   ```
3. GitHub Actions automatically rebuilds
4. Download new artifacts from Actions tab

---

## Free Tier Limits

GitHub Actions free tier (for public repos):
- ✅ **2,000 minutes/month** included
- ✅ Each build takes ~8 minutes × 3 platforms = ~24 minutes
- ✅ You can do **~80 builds per month for free**

For private repos:
- 🔒 500 minutes/month free
- Additional minutes cost $0.008/minute

---

## Support

If builds keep failing:
1. Check the **Issues** tab on GitHub
2. Post build logs in JUCE Forum: https://forum.juce.com/
3. Verify your local build works first (follow `BUILD_INSTRUCTIONS.md`)

---

**Your plugins are building right now!** 🎛️

Check the Actions tab in ~5-10 minutes to download your VST3 and AU files.
