/* Runtime checks for ARM64 host CPU helpers.
 * This is not a full runtime or game build. */
#include "x86.h"
#include "mmx_reference.h"
#include "host_path.h"
#include <stdio.h>

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

    /* Preserve the original known-answer checks for quick, readable failures. */
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

    /* Differentially exercise every MMX opcode against the independent oracle. */
    {
        static const uint64_t edge[] = {
            0, ~0ull, 0x8000800080008000ull, 0x7FFF7FFF7FFF7FFFull,
            0x80808080807F7F7Full, 0x00FF00FF00FF00FFull,
            0x8000000080000000ull, 0x0123456789ABCDEFull
        };
        uint64_t rnd = 0x9E3779B97F4A7C15ull;
        unsigned checks = 0;
        for (int op = MX_PADDB; op <= MX_PSRAD; op++) {
            for (int i = 0; i < 600; i++) {
                uint64_t x, y;
                if (i < 64) {
                    x = edge[i & 7];
                    y = edge[i >> 3];
                } else {
                    rnd ^= rnd << 13; rnd ^= rnd >> 7; rnd ^= rnd << 17; x = rnd;
                    rnd ^= rnd << 13; rnd ^= rnd >> 7; rnd ^= rnd << 17; y = rnd;
                }
                if (op >= MX_PSLLW) y = (uint64_t)(i % 71);
                uint64_t actual = mmx(op, x, y);
                uint64_t expected = mmx_ref(op, x, y);
                if (actual != expected) {
                    fprintf(stderr,
                        "ARM64 MMX mismatch: op=%d a=%016llX b=%016llX got=%016llX expected=%016llX\n",
                        op, (unsigned long long)x, (unsigned long long)y,
                        (unsigned long long)actual, (unsigned long long)expected);
                    return 9;
                }
                checks++;
            }
        }
        if (checks != (unsigned)(MX_PSRAD - MX_PADDB + 1) * 600u) return 10;
    }


    /* Portable Xbox device-path parser: no Windows APIs are needed. */
    {
        RedlineXboxPath path;
        if (!redline_xbox_path_classify("\\\\Device\\\\CdRom0\\\\media\\\\cars.xpr", &path) ||
            path.device != REDLINE_XBOX_PATH_DISC ||
            strcmp(path.suffix, "\\\\media\\\\cars.xpr") != 0) return 11;
        if (!redline_xbox_path_classify("\\\\??\\\\Device\\\\Harddisk0\\\\Partition12", &path) ||
            path.device != REDLINE_XBOX_PATH_PARTITION || path.partition != 12 ||
            !redline_xbox_path_is_partition_root("\\\\Device\\\\Harddisk0\\\\Partition12\\\\")) return 12;
        if (!redline_xbox_path_classify("\\\\Device\\\\Harddisk0\\\\Partition3\\\\cache\\\\file.dat", &path) ||
            path.device != REDLINE_XBOX_PATH_PARTITION || path.partition != 3 ||
            strcmp(path.suffix, "\\\\cache\\\\file.dat") != 0) return 13;
        if (redline_xbox_path_classify("\\\\Device\\\\CdRom01\\\\media", &path) ||
            redline_xbox_path_classify("\\\\Device\\\\Harddisk0\\\\PartitionX", &path) ||
            redline_xbox_path_is_partition_root("\\\\Device\\\\Harddisk0\\\\Partition3\\\\folder")) return 14;
    }

    return 0;
}
