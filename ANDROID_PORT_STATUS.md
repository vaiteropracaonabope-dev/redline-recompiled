# Android port status

## Current state

This repository is the Windows x64 Redline Recompiled port of Forza Motorsport (Xbox, 2005). The existing build is not an Android build and cannot produce an APK as-is.

The current build script, `tools/build.py`, invokes the Microsoft Visual C++ toolchain and links a Windows executable (`redline.exe`) plus a Windows game DLL (`forza_recomp.dll`). It links Windows system libraries and SDL3's Windows import library. The runtime also contains Windows-specific implementations, including WASAPI audio (`runtime/audio.c`) and Win32 file/device handling (`runtime/file.c`).

## Android port blockers

1. **CPU / recompilation target:** the current lifted game output targets x86-64 Windows. Android phones generally need an ARM64-v8a native library. The recompilation/lifting pipeline must be assessed and adapted to emit AArch64 code; changing only the compiler target is not sufficient.
2. **Graphics:** the current runtime uses Direct3D/Windows graphics interfaces. Android needs a Vulkan or OpenGL ES backend, or a proven translation layer, with the Xbox GPU behavior mapped correctly.
3. **Audio:** replace the WASAPI host output with Android-compatible audio (AAudio or OpenSL ES, or SDL's Android audio backend).
4. **Input and windowing:** use SDL3's Android support or Android NDK input/surface integration; add touch controls while retaining controller support.
5. **Filesystem and permissions:** replace Win32 paths, handles, file enumeration and hard-coded Windows assumptions with Android app storage and the Storage Access Framework where needed.
6. **Packaging:** create an Android Gradle/NDK application that packages the native runtime, assets supplied by the user, and ABI-specific libraries into an APK.

## Safe implementation order

- [x] Create a separate `android-port` branch so the Windows `main` branch remains untouched.
- [x] Inspect the existing build script and the main platform-specific runtime files.
- [x] Inventory the main Win32, Direct3D, WASAPI, and x86-64 assumptions across the runtime and build/lifter sources (see `ANDROID_PORT_AUDIT.md`).
- [x] Identify initial AArch64 blockers in the CPU/lifter interface (see `ANDROID_ARM64_CPU_AUDIT.md`); full generated-code compatibility remains unverified.
- [x] Add a host packed-float SIMD abstraction for x64 SSE2 and ARM64 NEON (`runtime/host_simd.h`) and expose the lifter's existing intrinsic spellings on ARM64.
- [x] Add a scalar ARM64 MMX implementation in `runtime/x86.h`, retaining the SSE2 implementation for x64 and the independent scalar reference/self-test in `runtime/runtime.c`.
- [x] Cross-compile the ARM64 CPU compatibility test with Android NDK 27.2.12479018; GitHub Actions run 45518316795 completed successfully.
- [x] Run native ARM64 runtime assertions for packed-float arithmetic, MINPS/MAXPS edge semantics, and selected MMX operations; GitHub Actions run 45518666662 passed this native ARM64 job.
- [x] Run expanded MMX differential coverage on native ARM64; GitHub Actions run 45518921741 passed the native ARM64 runtime job. It compares every MMX opcode against a separate scalar oracle over 8×8 edge combinations and deterministic pseudo-random inputs.
- [x] Finish the Android NDK cross-compile for the expanded CPU and POSIX filesystem checks; workflow run 45520455565 completed successfully.
- [ ] Design the remaining platform abstraction boundaries before replacing platform APIs.
- [ ] Add an Android NDK/Gradle skeleton only after the native entry points and graphics strategy are known.
- [ ] Port and test one subsystem at a time: startup/filesystem, input, audio, graphics, then game runtime.
- [ ] Build and test an ARM64 APK on a physical Android device.
- [ ] Optimize frame pacing, memory use, thermal behavior, and touch/controller UX.

## Important constraints

- This status file does not claim that an Android build or APK exists.
- Do not include, upload, or redistribute Forza game data. The user must provide their own legally obtained game files.
- Keep all Android work on `android-port` until it is independently buildable and tested. Do not merge experimental platform changes into `main`.

## Initial evidence inspected

- `tools/build.py`: Windows-only MSVC/Zig Windows target, Windows executable and DLL linking.
- `runtime/audio.c`: WASAPI / Windows COM audio output.
- `runtime/file.c`: Win32 file APIs, Windows path conventions, and host filesystem mapping.
- `README.md`: describes the current project as a Windows 10/11 x64 PC port requiring DirectX 11.1.

## Filesystem portability groundwork

- [x] Extract Xbox device-path classification into the platform-neutral header `runtime/host_path.h`; it parses disc and hard-disk partition paths without Win32 APIs.
- [x] Integrate that parser into the existing Windows filesystem mapper in `runtime/file.c`, preserving the host-specific file I/O implementation for now.
- [x] Run native ARM64 tests for disc paths, partition roots, partition subpaths, and invalid device names; GitHub Actions run 45519263315 passed the runtime test after correcting test-string escaping.
- [x] Finish Android NDK cross-compilation for the path/CPU smoke test; workflow run 45520455565 completed successfully.
- [ ] Define a host filesystem operations interface (open/read/write/stat/enumerate/create/remove) and implement Android/POSIX behavior before porting the full HLE file layer. The current `runtime/file.c` still directly depends on Win32 handles, FindFirstFile, CreateFile, ReadFile, and related APIs.

## Portable host filesystem interface

- [x] Add `runtime/host_fs.h`: platform-neutral operations for open/close, sequential and positioned I/O, seek, truncate, sync, metadata, directory creation/removal, and directory enumeration.
- [x] Add `runtime/host_fs_posix.c`: POSIX backend intended for Linux and Android; it uses ordinary host paths and does not yet replace the Win32 calls in `runtime/file.c`.
- [x] Add `tests/host_fs_posix.c` covering file creation, read/write, positioned I/O, truncate, sync, metadata, directory enumeration, and cleanup.
- [x] Confirm Android NDK cross-compilation and native ARM64 runtime test for the POSIX backend; workflow run 45520455565 passed.
- [x] Add and test the Windows backend for this interface.
- [x] Begin integrating the interface into `runtime/file.c`: mod-directory discovery and recursive mod-file enumeration now use `redline_fs_mkdir` and `redline_fs_dir_*`.
- [ ] Migrate the remaining Xbox HLE file operations incrementally; open/read/write/metadata/delete and per-handle directory enumeration still contain direct Win32 calls.

## Dual-platform host filesystem backend groundwork

- [x] Add `runtime/host_fs_win32.c`, the Windows implementation of the platform-neutral filesystem interface.
- [x] Add a Windows CI job to compile the backend; initialize the MSVC environment before invoking `cl`.
- [x] POSIX filesystem runtime tests passed on the native ARM64 runner in workflow run 45519799644.
- [x] Configure MSVC in CI; the backend reached compilation, which exposed an MSVC secure-CRT warning for `strcpy`. Replace it with a bounded copy and rerun the Windows check.
- [ ] Finish Android NDK cross-compilation for the POSIX backend in workflow run 45520068842; NDK installation is still in progress.
- [ ] Migrate the Xbox filesystem HLE in `runtime/file.c` to the interface in small tested pieces. The HLE still calls Win32 APIs directly; neither backend is integrated there yet.

## Windows filesystem backend validation

- [x] Fix the first MSVC compile warning by replacing deprecated `strcpy` with a bounded copy.
- [x] Make Windows truncate preserve the current file offset, matching the POSIX backend contract.
- [x] Add `tests/host_fs_win32.c` to exercise create/read/write/positioned I/O/truncate/stat/directory enumeration and cleanup.
- [x] Compile and run the Windows backend runtime test in GitHub Actions; the latest workflow run 45520455565 passed. The workflow now compiles the two source files to distinct object names before linking.
- [x] Integrate shared host filesystem directory enumeration and directory creation into mod discovery in the Xbox HLE.
- [x] Route Xbox file open/create, sequential and positioned read/write, flush, close, seek/query-position, truncate/end-of-file, and deletion-on-close handle operations through `host_fs`.
- [x] Preserve positioned-read EOF behavior across POSIX and Windows; both backend tests assert that a read beyond EOF returns zero bytes.
- [x] Add a Windows CI compile-only check for `runtime/file.c`; workflow run 45520963051 passed alongside ARM64 native/POSIX tests and Android NDK header checks.
- [ ] Remove remaining Win32 dependencies from the HLE: file metadata/timestamps, directory enumeration state, recursive directory deletion, host directory creation for HDD partitions, and `windows.h` types are still Windows-specific. The full `runtime/file.c` is not yet Android-compilable.
