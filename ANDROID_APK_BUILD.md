# Android APK build scaffold

This branch now contains a minimal Android Gradle application so CI can produce and verify a real installable debug APK.

## Build

The GitHub Actions workflow `.github/workflows/android-apk.yml` runs:

```sh
gradle assembleDebug
```

The APK is uploaded as the `redline-android-debug-apk` workflow artifact.

## Important status

This is a **development shell APK only**. It confirms the Android application packaging/build pipeline. It does not run Redline Recompiled, does not include the Xbox game, and is not a playable Forza build.

The current desktop runtime still depends on Windows APIs and Direct3D 11. The Android runtime, graphics backend, audio/input integration, guest-memory integration, and game boot path must be ported before this shell can launch the game.

No copyrighted game data is included. Use only game files supplied by the user in a future compatible runtime.
