/* Compile-only coverage for the ARM64 host CPU helpers.
 * This is not a full runtime or game build. */
#include "x86.h"

int main(void) {
    float lanes[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    __m128 a = _mm_loadu_ps(lanes);
    __m128 b = _mm_set1_ps(2.0f);
    __m128 c = _mm_add_ps(a, b);
    _mm_storeu_ps(lanes, c);

    uint64_t x = UINT64_C(0x0102030405060708);
    uint64_t y = UINT64_C(0x0807060504030201);
    uint64_t z = mmx(MX_PADDUSB, x, y);
    z ^= mmx(MX_PMADDWD, x, y);
    z ^= mmx(MX_PSRAW, x, UINT64_C(16));
    z ^= mmx(MX_PUNPCKHBW, x, y);
    z ^= mmx(MX_PACKUSWB, x, y);
    return (int)(lanes[0] + (float)(z & 1));
}
