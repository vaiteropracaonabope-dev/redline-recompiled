# ARM64 SIMD Compatibility Audit

Status: source audit complete; no ARM64 implementation has been merged yet.

## Confirmed code-generation boundary

- `tools/lift.py` emits the packed single-precision SSE operations as host intrinsics: `_mm_add_ps`, `_mm_sub_ps`, `_mm_mul_ps`, `_mm_div_ps`, `_mm_min_ps`, `_mm_max_ps`, `_mm_sqrt_ps`, `_mm_set1_ps`, `_mm_and_ps`, `_mm_or_ps`, `_mm_xor_ps`, and `_mm_andnot_ps`.
- The generated statements use the host type `__m128` through `XL()` and `XS()` in `runtime/x86.h`. This means replacing only the MMX helper is insufficient: lifted C files also require a portable 128-bit float-vector interface.
- Most other SSE instructions handled by the lifter are expressed directly in terms of the `Xmm` union lanes, so they do not require host intrinsics.
- MMX instructions are emitted as calls to `mmx()`, `mmx_pshufw()`, and `mmx_pmovmskb()`.

## Existing scalar MMX reference

`runtime/runtime.c` already contains a scalar `mmx_ref()` implementation and `mmx_selftest()`, which compares the current SSE2 `mmx()` implementation against the scalar reference over edge cases and deterministic pseudo-random inputs (including shift counts 0 through 70).

This is valuable groundwork, but `mmx_ref()` is currently private to `runtime.c` and uses helper types/constants declared in `x86.h`. It cannot simply be selected by the lifted DLL without reorganizing the interface. The self-test currently validates the SSE2 implementation on the existing Windows x64 target; it is not evidence of an ARM64 build.

## Implementation now present on `android-port`

- `runtime/host_simd.h` now maps the packed-float intrinsic names emitted by the lifter to NEON on AArch64 and keeps SSE2 aliases on x64.
- `runtime/x86.h` now selects a scalar lane-by-lane MMX implementation on AArch64 and retains the original SSE2 implementation on x64. The scalar implementation is based on the existing reference in `runtime/runtime.c`; that independent reference and its startup differential test remain in place.
- The SIMD abstraction is included by `x86.h`, so the lifter's emitted `__m128` declarations and packed-float intrinsic calls have an ARM64 host representation.

This is source-level integration only. It has not yet been compiled with an Android NDK/ARM64 toolchain. The existing runtime is still Windows-specific, so this does not produce an Android executable or APK.

## Safe implementation order

1. Introduce a host SIMD abstraction with a stable 128-bit float-vector type and load/store helpers. Keep the current SSE2 implementation selected for Windows x64.
2. Add an ARM64 implementation using NEON, or a scalar correctness fallback first. The abstraction must cover every intrinsic emitted by `tools/lift.py`, including exact lane and bitwise behavior.
3. Move or expose the scalar MMX reference through a shared internal interface, then select it for ARM64 until a NEON implementation is proven equivalent.
4. Keep and extend the existing differential MMX tests. Add independent tests for the packed SSE abstraction, including NaNs, signed zero, min/max operand selection, and bitwise operations.
5. Only after the host SIMD layer builds and passes tests on ARM64 should the project begin replacing the Windows-specific runtime services.

## Scope and non-goals

This audit does not implement Android support, change Windows behavior, or claim that the runtime can currently compile on Android. The runtime still depends heavily on Win32, Direct3D/DXGI, Media Foundation, WASAPI, and Windows DLL loading. SIMD compatibility is one prerequisite, not a complete Android port.
