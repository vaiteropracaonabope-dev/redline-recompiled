# Android APK and native bridge

The Android app includes a Forza-inspired setup screen, device diagnostics, a folder picker for a user-provided game dump, saved graphics preferences, and a link to the latest GitHub Actions APK artifact.

## Build

The `.github/workflows/android-apk.yml` workflow installs Android SDK 35, NDK 27.2.12479018 and CMake 3.22.1, then runs `gradle assembleDebug`. The APK is uploaded as `redline-android-debug-apk`.

## Native bridge status

The APK now packages `libredline_android.so` for `arm64-v8a`. The JNI/NDK bridge confirms that native code is loaded inside Android. This is an integration foundation, **not the Forza game runtime**.

The desktop runtime still depends on Windows APIs and Direct3D 11. The Android guest execution path, memory mapping, Xbox HLE integration, Vulkan/OpenGL renderer, audio, input and boot flow remain unported and unvalidated. Graphics preferences in the UI are saved only; they do not alter rendering yet.

## Game files and downloads

No copyrighted Forza game data is hosted or included. The folder picker only checks for the expected top-level `Forza.xbe` and `media` entries in files supplied by the user. The in-app download button opens the project's GitHub Actions page, where the debug APK artifact can be downloaded. Artifacts expire after 14 days.

Use only game files you are legally entitled to use.
