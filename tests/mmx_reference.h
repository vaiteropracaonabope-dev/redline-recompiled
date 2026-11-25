/* Independent scalar MMX oracle mirrored from runtime/runtime.c.
 * Include after x86.h so Mmx and MX_* are defined. */
#pragma once
/* MMX lane by lane, as written from the manuals: the reference for x86.h's SSE2 mmx */
static int sat(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
static uint64_t mmx_ref(int op, uint64_t qa, uint64_t qb) {
    Mmx a = {qa}, b = {qb}, r = {0};
    uint64_t n = qb; /* shift count */
    int i;
    switch (op) {
    case MX_PADDB: for (i = 0; i < 8; i++) r.b[i] = a.b[i] + b.b[i]; break;
    case MX_PADDW: for (i = 0; i < 4; i++) r.w[i] = a.w[i] + b.w[i]; break;
    case MX_PADDD: for (i = 0; i < 2; i++) r.d[i] = a.d[i] + b.d[i]; break;
    case MX_PADDQ: r.q = a.q + b.q; break;
    case MX_PSUBB: for (i = 0; i < 8; i++) r.b[i] = a.b[i] - b.b[i]; break;
    case MX_PSUBW: for (i = 0; i < 4; i++) r.w[i] = a.w[i] - b.w[i]; break;
    case MX_PSUBD: for (i = 0; i < 2; i++) r.d[i] = a.d[i] - b.d[i]; break;
    case MX_PSUBQ: r.q = a.q - b.q; break;
    case MX_PADDSB: for (i = 0; i < 8; i++) r.sb[i] = (int8_t)sat(a.sb[i] + b.sb[i], -128, 127); break;
    case MX_PADDSW: for (i = 0; i < 4; i++) r.sw[i] = (int16_t)sat(a.sw[i] + b.sw[i], -32768, 32767); break;
    case MX_PADDUSB: for (i = 0; i < 8; i++) r.b[i] = (uint8_t)sat(a.b[i] + b.b[i], 0, 255); break;
    case MX_PADDUSW: for (i = 0; i < 4; i++) r.w[i] = (uint16_t)sat(a.w[i] + b.w[i], 0, 65535); break;
    case MX_PSUBSB: for (i = 0; i < 8; i++) r.sb[i] = (int8_t)sat(a.sb[i] - b.sb[i], -128, 127); break;
    case MX_PSUBSW: for (i = 0; i < 4; i++) r.sw[i] = (int16_t)sat(a.sw[i] - b.sw[i], -32768, 32767); break;
    case MX_PSUBUSB: for (i = 0; i < 8; i++) r.b[i] = (uint8_t)sat(a.b[i] - b.b[i], 0, 255); break;
    case MX_PSUBUSW: for (i = 0; i < 4; i++) r.w[i] = (uint16_t)sat(a.w[i] - b.w[i], 0, 65535); break;
    case MX_PMULLW: for (i = 0; i < 4; i++) r.w[i] = (uint16_t)(a.sw[i] * b.sw[i]); break;
    case MX_PMULHW: for (i = 0; i < 4; i++) r.w[i] = (uint16_t)((a.sw[i] * b.sw[i]) >> 16); break;
    case MX_PMULHUW: for (i = 0; i < 4; i++) r.w[i] = (uint16_t)(((uint32_t)a.w[i] * b.w[i]) >> 16); break;
    case MX_PMADDWD: for (i = 0; i < 2; i++) r.sd[i] = a.sw[2 * i] * b.sw[2 * i] + a.sw[2 * i + 1] * b.sw[2 * i + 1]; break;
    case MX_PAND: r.q = a.q & b.q; break;
    case MX_PANDN: r.q = ~a.q & b.q; break;
    case MX_POR: r.q = a.q | b.q; break;
    case MX_PXOR: r.q = a.q ^ b.q; break;
    case MX_PCMPEQB: for (i = 0; i < 8; i++) r.b[i] = a.b[i] == b.b[i] ? 0xFF : 0; break;
    case MX_PCMPEQW: for (i = 0; i < 4; i++) r.w[i] = a.w[i] == b.w[i] ? 0xFFFF : 0; break;
    case MX_PCMPEQD: for (i = 0; i < 2; i++) r.d[i] = a.d[i] == b.d[i] ? 0xFFFFFFFFu : 0; break;
    case MX_PCMPGTB: for (i = 0; i < 8; i++) r.b[i] = a.sb[i] > b.sb[i] ? 0xFF : 0; break;
    case MX_PCMPGTW: for (i = 0; i < 4; i++) r.w[i] = a.sw[i] > b.sw[i] ? 0xFFFF : 0; break;
    case MX_PCMPGTD: for (i = 0; i < 2; i++) r.d[i] = a.sd[i] > b.sd[i] ? 0xFFFFFFFFu : 0; break;
    case MX_PUNPCKLBW: for (i = 0; i < 4; i++) r.b[2 * i] = a.b[i], r.b[2 * i + 1] = b.b[i]; break;
    case MX_PUNPCKHBW: for (i = 0; i < 4; i++) r.b[2 * i] = a.b[i + 4], r.b[2 * i + 1] = b.b[i + 4]; break;
    case MX_PUNPCKLWD: for (i = 0; i < 2; i++) r.w[2 * i] = a.w[i], r.w[2 * i + 1] = b.w[i]; break;
    case MX_PUNPCKHWD: for (i = 0; i < 2; i++) r.w[2 * i] = a.w[i + 2], r.w[2 * i + 1] = b.w[i + 2]; break;
    case MX_PUNPCKLDQ: r.d[0] = a.d[0]; r.d[1] = b.d[0]; break;
    case MX_PUNPCKHDQ: r.d[0] = a.d[1]; r.d[1] = b.d[1]; break;
    case MX_PACKSSWB: for (i = 0; i < 4; i++) r.sb[i] = (int8_t)sat(a.sw[i], -128, 127), r.sb[i + 4] = (int8_t)sat(b.sw[i], -128, 127); break;
    case MX_PACKSSDW: for (i = 0; i < 2; i++) r.sw[i] = (int16_t)sat(a.sd[i], -32768, 32767), r.sw[i + 2] = (int16_t)sat(b.sd[i], -32768, 32767); break;
    case MX_PACKUSWB: for (i = 0; i < 4; i++) r.b[i] = (uint8_t)sat(a.sw[i], 0, 255), r.b[i + 4] = (uint8_t)sat(b.sw[i], 0, 255); break;
    case MX_PAVGB: for (i = 0; i < 8; i++) r.b[i] = (uint8_t)((a.b[i] + b.b[i] + 1) >> 1); break;
    case MX_PAVGW: for (i = 0; i < 4; i++) r.w[i] = (uint16_t)((a.w[i] + b.w[i] + 1) >> 1); break;
    case MX_PMINUB: for (i = 0; i < 8; i++) r.b[i] = a.b[i] < b.b[i] ? a.b[i] : b.b[i]; break;
    case MX_PMAXUB: for (i = 0; i < 8; i++) r.b[i] = a.b[i] > b.b[i] ? a.b[i] : b.b[i]; break;
    case MX_PMINSW: for (i = 0; i < 4; i++) r.sw[i] = a.sw[i] < b.sw[i] ? a.sw[i] : b.sw[i]; break;
    case MX_PMAXSW: for (i = 0; i < 4; i++) r.sw[i] = a.sw[i] > b.sw[i] ? a.sw[i] : b.sw[i]; break;
    case MX_PSADBW: { uint32_t s = 0; for (i = 0; i < 8; i++) s += a.b[i] > b.b[i] ? a.b[i] - b.b[i] : b.b[i] - a.b[i]; r.q = s; break; }
    case MX_PSLLW: for (i = 0; i < 4; i++) r.w[i] = n > 15 ? 0 : (uint16_t)(a.w[i] << n); break;
    case MX_PSLLD: for (i = 0; i < 2; i++) r.d[i] = n > 31 ? 0 : a.d[i] << n; break;
    case MX_PSLLQ: r.q = n > 63 ? 0 : a.q << n; break;
    case MX_PSRLW: for (i = 0; i < 4; i++) r.w[i] = n > 15 ? 0 : (uint16_t)(a.w[i] >> n); break;
    case MX_PSRLD: for (i = 0; i < 2; i++) r.d[i] = n > 31 ? 0 : a.d[i] >> n; break;
    case MX_PSRLQ: r.q = n > 63 ? 0 : a.q >> n; break;
    case MX_PSRAW: for (i = 0; i < 4; i++) r.sw[i] = (int16_t)(a.sw[i] >> (n > 15 ? 15 : n)); break;
    case MX_PSRAD: for (i = 0; i < 2; i++) r.sd[i] = a.sd[i] >> (n > 31 ? 31 : n); break;
    }
    return r.q;
}

