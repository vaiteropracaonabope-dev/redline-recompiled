# Android port: source audit

This audit is based on the current `android-port` branch. It identifies the actual source boundaries that must change before an Android APK can be built.

## Repository inventory

The branch contains 358 tracked files at the time of this audit. The key runtime sources are in `runtime/`; the x86-to-C static lifter is `tools/lift.py`; the Windows build orchestration is `tools/build.py`. No Android Gradle project or Android NDK build is present.

## Findings by subsystem

| Area | Current implementation | Android work required |
|---|---|---|
| Process startup and guest memory | `runtime/main.c`: Win32 process APIs, 4 GiB virtual address reservation, XBE mapping, Windows command-line and crash handling | Design an Android-native entry point and memory mapping strategy; validate whether the 4 GiB guest address layout is reservable on supported Android devices |
| Recompiled-code loader | `runtime/runtime.c`: loads `forza_recomp.dll` and resolves Windows DLL exports | Replace DLL loading/export assumptions with statically linked or Android `.so` symbols |
| Graphics | `runtime/render.c`: Direct3D 11.1, DXGI swap chain, HLSL compilation and Windows display/window APIs; `runtime/nv2a.c` and `runtime/pgraph.c` model Xbox GPU state | A real Vulkan/OpenGL ES renderer or a validated translation layer is needed; this is a major rewrite, not a build-flag change |
| Video | `runtime/video.c`: Media Foundation and D3D11 video textures | Replace with Android-compatible decoding and GPU texture upload, or initially disable optional upscaled video playback |
| Audio | `runtime/audio.c`: WASAPI, Windows COM and audio render thread | Replace host output with SDL Android audio, AAudio or OpenSL ES |
| Filesystem | `runtime/file.c`: Win32 file handles, FindFirstFile, Windows paths and directories | Implement Android app-private storage and a user-selected game-data directory; preserve Xbox path semantics |
| Threads and synchronization | `runtime/thread.c`: Win32 threads, events, wait objects and timing | Map to pthreads/C11 primitives or SDL; preserve Xbox dispatcher semantics and guest scheduling |
| CPU/lifter | `tools/lift.py` emits C from x86 instructions; `runtime/x86.h` uses x86/SSE intrinsics and Windows DLL import/export annotations | Determine generated C portability and replace x86-only intrinsics/ABI assumptions; compile for ARM64 only after this is proven |
| Build | `tools/build.py` invokes MSVC or Windows-targeting Zig and links a Windows EXE/DLL | Add a separate Android NDK CMake/Gradle pipeline after native entry points and graphics backend are defined |

## Critical architecture decision: the lifter

The existing lifter translates Xbox x86 instructions into C source. That is not the same as translating the game to ARM64 by changing a compiler flag. The generated C and `runtime/x86.h` must be audited for x86 intrinsics, calling-convention assumptions, inline assembly, and Windows DLL interfaces. The generated game code must then be compiled by the Android NDK for `arm64-v8a` and linked into the Android native library.

## Recommended implementation sequence

1. **Portability proof:** build a tiny Android NDK shared library using the compiler and a small subset of the current runtime headers. Resolve ARM64 compile errors without touching rendering yet.
2. **CPU compatibility:** inventory every x86 intrinsic and ABI dependency in `runtime/x86.h` and generated output; implement portable equivalents and tests.
3. **Memory and OS layer:** separate guest memory, file, thread, timing and loader operations from Win32.
4. **Graphics backend:** specify the NV2A-to-Vulkan/OpenGL ES mapping and shader translation plan before attempting a renderer.
5. **Android app shell:** add Gradle, manifest, native activity/SDL integration, permissions, and file picker.
6. **Incremental device tests:** validate startup, game data access, audio, controller/touch input and graphics on physical ARM64 devices.

## Explicitly not done

- No Android build system or APK has been created yet.
- No Windows runtime implementation has been replaced.
- No Forza game data is included or should be committed.
- No claim is made that the current project can run on Android at this stage.
