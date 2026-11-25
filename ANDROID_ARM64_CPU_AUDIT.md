# ARM64 CPU compatibility audit

## Scope

This is a focused follow-up to `ANDROID_PORT_AUDIT.md`, based on `runtime/x86.h` (475 lines) and `tools/lift.py`.

## Confirmed x86-specific dependencies

- `runtime/x86.h` unconditionally includes `<emmintrin.h>`, an x86 SSE2 intrinsic header. This is not a portable Android NDK header on ARM64.
- `XL(x)` and `XS(x, v)` use `_mm_loadu_ps` and `_mm_storeu_ps` to move 128-bit float vectors.
- The `mmx()` helper uses SSE2 integer intrinsics to emulate Xbox MMX operations, including packed add/subtract, saturating arithmetic, multiply, comparisons, unpack/pack, min/max, average, sum-of-absolute-differences, and shifts.
- `runtime/x86.h` also uses MSVC `__declspec(dllimport/dllexport/noreturn)` annotations in the Windows build. These need portable macros for an Android shared library/static link.
- The CPU context itself is represented in ordinary C structures, and many arithmetic/flag helpers are scalar C. This is a useful foundation, but does not prove the full generated game code is ARM64-ready.

## Important distinction

The Xbox CPU being x86 does not require the Android host CPU to be x86 if the lifter faithfully expresses guest instructions as portable C. However, host-specific intrinsics, calling conventions, compiler built-ins, and generated-code assumptions must all be handled. ARM64 NEON can implement the vector operations, or a scalar reference implementation can be used first for correctness (with a likely performance cost).

## Recommended next technical task

1. Introduce a small host-CPU abstraction header for vector load/store and MMX helper operations.
2. Keep the existing SSE2 implementation for Windows x64.
3. Add an ARM64 NEON implementation behind compile-time architecture checks, with a scalar fallback for testing.
4. Create unit tests that compare every MMX helper operation against a scalar reference for edge cases (signed/unsigned saturation, overflow, shifts at/above lane width, packing, and shuffle behavior).
5. Only then try compiling generated game C with the Android NDK.

## Not yet verified

- No full generated `recomp/*.c` output was included in the tracked source tree at audit time; the lifter generates it from the user's game image and analysis files.
- No ARM64 compilation has been run.
- No device performance or correctness has been measured.
- This audit does not claim that replacing SSE2 intrinsics alone will make the game run on Android.
