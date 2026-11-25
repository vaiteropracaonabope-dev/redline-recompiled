/* Host-side packed-float operations used by lifted SSE code.
 *
 * This header is intentionally self-contained. The ARM64 path uses NEON while
 * the existing Windows x64 path retains the compiler's SSE2 intrinsics.
 * The separate MMX integer helper is selected in x86.h for each host architecture.
 */
#pragma once

#if defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>

typedef float32x4_t __m128;
typedef float32x4_t redline_host_m128;

static inline redline_host_m128 redline_mm_loadu_ps(const float *p) {
    return vld1q_f32(p);
}

static inline void redline_mm_storeu_ps(float *p, redline_host_m128 v) {
    vst1q_f32(p, v);
}

static inline redline_host_m128 redline_mm_set1_ps(float v) {
    return vdupq_n_f32(v);
}

static inline redline_host_m128 redline_mm_add_ps(redline_host_m128 a, redline_host_m128 b) {
    return vaddq_f32(a, b);
}

static inline redline_host_m128 redline_mm_sub_ps(redline_host_m128 a, redline_host_m128 b) {
    return vsubq_f32(a, b);
}

static inline redline_host_m128 redline_mm_mul_ps(redline_host_m128 a, redline_host_m128 b) {
    return vmulq_f32(a, b);
}

static inline redline_host_m128 redline_mm_div_ps(redline_host_m128 a, redline_host_m128 b) {
    return vdivq_f32(a, b);
}

static inline redline_host_m128 redline_mm_sqrt_ps(redline_host_m128 a) {
    return vsqrtq_f32(a);
}

/* Match Intel MINPS/MAXPS: when operands compare equal or either is NaN,
 * return the second operand. A plain NEON min/max is not equivalent here. */
static inline redline_host_m128 redline_mm_min_ps(redline_host_m128 a, redline_host_m128 b) {
    uint32x4_t take_a = vcltq_f32(a, b);
    return vbslq_f32(take_a, a, b);
}

static inline redline_host_m128 redline_mm_max_ps(redline_host_m128 a, redline_host_m128 b) {
    uint32x4_t take_a = vcgtq_f32(a, b);
    return vbslq_f32(take_a, a, b);
}

static inline redline_host_m128 redline_mm_and_ps(redline_host_m128 a, redline_host_m128 b) {
    return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b)));
}

static inline redline_host_m128 redline_mm_or_ps(redline_host_m128 a, redline_host_m128 b) {
    return vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b)));
}

static inline redline_host_m128 redline_mm_xor_ps(redline_host_m128 a, redline_host_m128 b) {
    return vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b)));
}

static inline redline_host_m128 redline_mm_andnot_ps(redline_host_m128 a, redline_host_m128 b) {
    return vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(b), vreinterpretq_u32_f32(a)));
}

/* Keep the lifter's emitted intrinsic spellings on the portable host API. */
#define _mm_loadu_ps redline_mm_loadu_ps
#define _mm_storeu_ps redline_mm_storeu_ps
#define _mm_set1_ps redline_mm_set1_ps
#define _mm_add_ps redline_mm_add_ps
#define _mm_sub_ps redline_mm_sub_ps
#define _mm_mul_ps redline_mm_mul_ps
#define _mm_div_ps redline_mm_div_ps
#define _mm_sqrt_ps redline_mm_sqrt_ps
#define _mm_min_ps redline_mm_min_ps
#define _mm_max_ps redline_mm_max_ps
#define _mm_and_ps redline_mm_and_ps
#define _mm_or_ps redline_mm_or_ps
#define _mm_xor_ps redline_mm_xor_ps
#define _mm_andnot_ps redline_mm_andnot_ps

#else
#include <emmintrin.h>

typedef __m128 redline_host_m128;

#define redline_mm_loadu_ps _mm_loadu_ps
#define redline_mm_storeu_ps _mm_storeu_ps
#define redline_mm_set1_ps _mm_set1_ps
#define redline_mm_add_ps _mm_add_ps
#define redline_mm_sub_ps _mm_sub_ps
#define redline_mm_mul_ps _mm_mul_ps
#define redline_mm_div_ps _mm_div_ps
#define redline_mm_sqrt_ps _mm_sqrt_ps
#define redline_mm_min_ps _mm_min_ps
#define redline_mm_max_ps _mm_max_ps
#define redline_mm_and_ps _mm_and_ps
#define redline_mm_or_ps _mm_or_ps
#define redline_mm_xor_ps _mm_xor_ps
#define redline_mm_andnot_ps _mm_andnot_ps
#endif
