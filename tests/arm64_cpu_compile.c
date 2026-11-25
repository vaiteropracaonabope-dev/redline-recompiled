/* Runtime checks for ARM64 host CPU helpers.
 * This is not a full runtime or game build. */
#include "x86.h"

int main(void) {
    float lanes[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    __m128 a = _mm_loadu_ps(lanes);
    __m128 b = _mm_set1_ps(2.0f);
    _mm_storeu_ps(lanes, _mm_add_ps(a, b));
    if (lanes[0] != 3.0f || lanes[1] != 4.0f ||
        lanes[2] != 5.0f || lanes[3] != 6.0f) return 1;

    /* Intel MINPS/MAXPS return the second operand for equal values or NaNs. */
    {
        float nan = NAN;
        float left[4] = {nan, -0.0f, 5.0f, 2.0f};
        float right[4] = {7.0f, 0.0f, 5.0f, nan};
        float out[4];
        _mm_storeu_ps(out, _mm_min_ps(_mm_loadu_ps(left), _mm_loadu_ps(right)));
        if (memcmp(out, right, sizeof out) != 0) return 2;
        _mm_storeu_ps(out, _mm_max_ps(_mm_loadu_ps(left), _mm_loadu_ps(right)));
        if (memcmp(out, right, sizeof out) != 0) return 3;
    }

    if (mmx(MX_PADDUSB, 0x00FF00F0FF000000ull, 0x0002000F01000000ull) !=
        0x00FF00FFFF000000ull) return 4;
    if (mmx(MX_PUNPCKLBW, 0x0000000044332211ull, 0x00000000DDCCBBAAull) !=
        0xDD44CC33BB22AA11ull) return 5;
    if (mmx(MX_PSRAW, 0x8000400000020001ull, 1) !=
        0xC000200000010000ull) return 6;
    if (mmx(MX_PMADDWD, 0x0002000300040005ull, 0x0006000700080009ull) !=
        (((uint64_t)(2 * 6 + 3 * 7) << 32) | (4 * 8 + 5 * 9))) return 7;
    if (mmx(MX_PACKUSWB, 0x0100FFFF00800010ull, 0) !=
        0x00000000FF008010ull) return 8;

    return 0;
}
