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
- [x] Integrate the interface into `runtime/file.c` for mod-directory discovery, recursive mod-file enumeration, HDD partition directory creation, recursive directory cleanup, and file/directory deletion.
- [x] Migrate per-handle Xbox directory enumeration to `redline_fs_dir_open/read/close`, with case-insensitive `*`/`?` matching and the Windows-compatible `*.*` all-files behavior.
- [x] Route Xbox file metadata queries and timestamp updates through `host_fs`; POSIX exposes access/modified times and explicitly reports creation time as unavailable, while Windows preserves creation/access/modified timestamps.
- [x] Populate creation, access, write, and change timestamps in `NtQueryDirectoryFile` from the host metadata (creation remains zero on POSIX when unavailable).

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
- [x] Compile and run the Windows backend runtime test in GitHub Actions; latest workflow run 45520963051 passed. Backend and test sources use distinct object names before linking.
- [x] Compile `runtime/file.c` as a Windows translation unit in CI after migrating primary file-handle I/O to `host_fs`.
- [x] Route Xbox file open/create, sequential and positioned read/write, flush, close, seek/query-position, truncate/end-of-file, delete-on-close, `NtDeleteFile`, HDD partition directory creation, and recursive partition cleanup through `host_fs`.
- [x] Preserve positioned-read EOF behavior across POSIX and Windows; both backend tests assert that a read beyond EOF returns zero bytes.
- [x] Add a Windows CI compile-only check for `runtime/file.c`; workflow run 45520963051 passed alongside ARM64 native/POSIX tests and Android NDK header checks.
- [x] Remove the direct `windows.h` include and Windows-specific timing/string helpers from `runtime/file.c`; replace the remaining Windows file-time type with a local fixed-width representation.
- [x] Remove direct Win32 dependencies from `runtime/file.c`; the file HLE now uses `host_fs` and portable fixed-width/time/string helpers.
- [x] Cross-compile `runtime/file.c` with Android NDK 27.2.12479018 using `-Wall -Wextra -Werror`; GitHub Actions run 45522839458 passed all three jobs (ARM64 NDK, native ARM64/POSIX, and Windows backend).
- [ ] Port remaining runtime subsystems; `runtime/thread.c`, graphics, audio, startup, and the full build pipeline still need Android-specific work. The complete runtime is not yet Android-buildable.


## Thread and synchronization portability groundwork

- [x] Confirm the directory timestamp migration and all three CI jobs in workflow run 45523074811.
- [x] Add `runtime/host_sync.h`, a platform-neutral contract for atomic 32-bit operations, wait/wake, monotonic and realtime clocks, sleep/yield, and host thread IDs.
- [x] Add a POSIX implementation in `runtime/host_sync_posix.c` using C atomics, pthread condition variables, and POSIX clocks; intended for Linux/Android.
- [x] Add a Windows implementation in `runtime/host_sync_win32.c` using Interlocked, WaitOnAddress, and Windows clocks.
- [x] Add POSIX and Windows smoke tests and wire them into the ARM64/Windows CI workflow.
- [x] Confirm workflow run 45523373515: ARM64 native, Android NDK header check, and Windows filesystem backend all passed for the initial host synchronization groundwork.
- [x] Confirm workflow run 45523656707: Windows host synchronization link/test, `thread.c` MSVC compile, ARM64 native tests, and Android NDK header check all passed.
- [x] Route the guest lock (GIL) ticket counters, waiter count, wakeups, urgent handoff, and 1 ms scheduling quantum through `host_sync`; GIL timing now uses the portable monotonic nanosecond clock.
- [x] Route `thread.c` sleep/yield calls, guest wait-object polling, and relative/absolute timeout clock calculations through `host_sync`; GIL and DPC scheduling clocks now use the portable monotonic clock.
- [x] Add `host_thread.h` with POSIX and Windows backends for detached thread creation and one-shot suspended-thread resume.
- [x] Route guest thread creation/resume, watchdog creation, and DPC thread creation through `host_thread`; keep host thread tokens outside 32-bit guest KTHREAD memory to avoid pointer truncation.
- [x] Add a suspended-start/resume lifecycle test; ARM64 native test and Windows backend compile/runtime test passed in workflow run 45524290355.
- [x] Complete Android NDK syntax-only checks for the POSIX thread backend; workflow run 45524290355 passed all three jobs.
- [x] Add nanosecond timed waits to `host_sync`; POSIX preserves sub-millisecond deadlines and Windows rounds to milliseconds. Add timeout coverage to both backend tests.
- [x] Replace the DPC thread Win32 event, high-resolution waitable timer, and multi-object wait with an atomic host-sync wake word and timed wait. `dpc_kick()` now signals through `redline_sync_store/wake_all`.
- [x] Move host thread naming and best-effort priority changes behind `host_thread`; POSIX uses `pthread_setname_np` and a best-effort `SCHED_FIFO` request, while Windows uses `SetThreadDescription` and `SetThreadPriority`.
- [x] Remove the direct `windows.h` dependency from `runtime/thread.c`; replace Win32 thread IDs/types with fixed-width types and `host_sync` IDs.
- [x] Route watchdog marker-file deletion through `host_fs` and process termination through `fatal_exit`.
- [x] Add Android NDK syntax-only compilation of `runtime/thread.c`; Windows MSVC translation-unit compilation and ARM64 NDK syntax check both passed in workflow run 45525130504.
- [x] Resolve portability issues exposed by the new checks: fixed-width GIL waiter declaration, local NT wait status constants after removing `windows.h`, and portable 64-bit diagnostic formatting.
- [ ] `thread.c` is now syntax-checkable for Android but not yet linked into a complete Android runtime. Event-object scheduling semantics and platform-specific priority behavior still require runtime validation.


## Dispatcher wait portability

- [x] Replace 1 ms polling for wait-any and wait-all dispatcher waits with a shared host synchronization sequence; waiters capture the sequence while holding the guest lock, release the lock, and sleep until a signal notification or timeout.
- [x] Route dispatcher object signal changes through the shared wake sequence, including thread/event/semaphore/mutant/timer paths in `thread.c`.
- [x] Notify dispatcher waiters when synchronous file I/O completion signals an event in `file.c`.
- [x] Android NDK syntax checks for `thread.c` and `file.c`, Windows MSVC translation-unit checks, and native ARM64 runtime tests passed in workflow run 45525519820.
- [ ] Runtime stress tests for wait-any/wait-all, synchronization events, APCs, and timeout races remain necessary; CI syntax checks do not prove full guest-kernel semantics.


## Dispatcher semaphore and mutant correctness

- [x] Extract pure semaphore/mutant state transitions into `runtime/dispatcher_objects.h` so the guest HLE and tests share exactly the same rules.
- [x] Validate semaphore creation bounds (`0 <= InitialCount <= MaximumCount`, positive maximum) and release counts/limits without signed overflow.
- [x] Make mutant recursion explicit: recursive acquisitions decrement the signal count, releases restore it, and ownership clears only after the outermost release. Non-owner release is rejected.
- [x] Make wait-all account for duplicate handles: repeated semaphore handles require multiple tokens, while a synchronization event/timer cannot be consumed twice from one signal.
- [x] Reject zero or over-limit multi-object wait counts rather than silently truncating them.
- [ ] Abandoned-mutant handling when a guest thread exits, full object-handle validation/closing, and runtime stress tests remain open.
- [x] Dispatcher semantics tests passed on Android NDK, native ARM64, and Windows CI; final validation is workflow run 45525897147 (all three jobs successful).


## Abandoned mutant ownership

- [x] Track created guest mutants explicitly (bounded registry of 4096 objects) rather than scanning arbitrary guest kernel memory.
- [x] When a guest thread exits, release every mutant it still owns, mark it abandoned, and wake dispatcher waiters.
- [x] The next successful waiter acquires an abandoned mutant and receives `STATUS_ABANDONED_WAIT_0` (or the matching index for wait-any); wait-all returns the abandoned status after acquiring all requested objects.
- [x] Extend pure dispatcher tests for recursive owner exit, abandonment state, next-thread acquisition, and clearing the abandoned flag.
- [ ] Full runtime tests with actual guest thread exit, wait-any/wait-all races, and mutant lifetime/handle references remain pending. The registry is intentionally bounded because dispatcher objects are not yet reference-counted or reclaimed.
- [x] Android NDK compile, native ARM64 runtime tests, and Windows MSVC checks passed in workflow run 45526294099 (all three jobs successful).


## Guest kernel handle references

- [x] Add checked reference-count primitives for raw guest object-address handles, including overflow/underflow rejection.
- [x] Initialize dispatcher-object references and file-object references at creation.
- [x] Duplicate handles now acquire a reference; `NtClose` releases one reference and reports invalid handles.
- [x] `ObReferenceObjectByHandle` and `ObfDereferenceObject` now maintain object references.
- [x] Final file-handle release closes the host file/directory resources; final dispatcher-object release logically invalidates the object (guest kernel bump allocations are not reused).
- [x] Validate handles before multi-object waits; invalid or already-closed objects return `STATUS_INVALID_HANDLE`.
- [ ] KTHREAD objects remain retained as stable wait targets; full thread-object lifetime/reference accounting and actual guest ABI validation for duplicate/close calls remain open.
- [ ] Guest kernel object memory is bump-allocated, so logical invalidation does not reclaim guest memory. A free-list/allocator and object-type-specific cleanup are still future work.
- [x] Android NDK compile, native ARM64 tests, and Windows MSVC checks passed in workflow run 45526682805 (all three jobs successful).
