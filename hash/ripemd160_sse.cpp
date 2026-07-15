/*
 * This file is part of the VanitySearch distribution (https://github.com/JeanLucPons/VanitySearch).
 * Copyright (c) 2019 Jean Luc PONS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ripemd160.h"
#include <string.h>
#include <immintrin.h>

#ifndef KEYHUNT_AVX2_ONLY
// Internal SSE RIPEMD-160 implementation.
namespace ripemd160sse {

#ifdef WIN64
  static const __declspec(align(16)) uint32_t _init[] = {
#else
  static const uint32_t _init[] __attribute__ ((aligned (16))) = {
#endif
      0x67452301ul,0x67452301ul,0x67452301ul,0x67452301ul,
      0xEFCDAB89ul,0xEFCDAB89ul,0xEFCDAB89ul,0xEFCDAB89ul,
      0x98BADCFEul,0x98BADCFEul,0x98BADCFEul,0x98BADCFEul,
      0x10325476ul,0x10325476ul,0x10325476ul,0x10325476ul,
      0xC3D2E1F0ul,0xC3D2E1F0ul,0xC3D2E1F0ul,0xC3D2E1F0ul
  };

//#define f1(x, y, z) (x ^ y ^ z)
//#define f2(x, y, z) ((x & y) | (~x & z))
//#define f3(x, y, z) ((x | ~y) ^ z)
//#define f4(x, y, z) ((x & z) | (~z & y))
//#define f5(x, y, z) (x ^ (y | ~z))

#define ROL(x,n) _mm_or_si128( _mm_slli_epi32(x, n) , _mm_srli_epi32(x, 32 - n) )

#ifdef WIN64

#define not(x) _mm_andnot_si128(x, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()))
#define f1(x,y,z) _mm_xor_si128(x, _mm_xor_si128(y, z))
#define f2(x,y,z) _mm_or_si128(_mm_and_si128(x,y),_mm_andnot_si128(x,z))
#define f3(x,y,z) _mm_xor_si128(_mm_or_si128(x,not(y)),z)
#define f4(x,y,z) _mm_or_si128(_mm_and_si128(x,z),_mm_andnot_si128(z,y))
#define f5(x,y,z) _mm_xor_si128(x,_mm_or_si128(y,not(z)))

#else

#define f1(x,y,z) _mm_xor_si128(x, _mm_xor_si128(y, z))
#define f2(x,y,z) _mm_or_si128(_mm_and_si128(x,y),_mm_andnot_si128(x,z))
#define f3(x,y,z) _mm_xor_si128(_mm_or_si128(x,~(y)),z)
#define f4(x,y,z) _mm_or_si128(_mm_and_si128(x,z),_mm_andnot_si128(z,y))
#define f5(x,y,z) _mm_xor_si128(x,_mm_or_si128(y,~(z)))

#endif


#define add3(x0, x1, x2 ) _mm_add_epi32(_mm_add_epi32(x0, x1), x2)
#define add4(x0, x1, x2, x3) _mm_add_epi32(_mm_add_epi32(x0, x1), _mm_add_epi32(x2, x3))

#define Round(a,b,c,d,e,f,x,k,r) \
  u = add4(a,f,x,_mm_set1_epi32(k)); \
  a = _mm_add_epi32(ROL(u, r),e); \
  c = ROL(c, 10);

#define R11(a,b,c,d,e,x,r) Round(a, b, c, d, e, f1(b, c, d), x, 0, r)
#define R21(a,b,c,d,e,x,r) Round(a, b, c, d, e, f2(b, c, d), x, 0x5A827999ul, r)
#define R31(a,b,c,d,e,x,r) Round(a, b, c, d, e, f3(b, c, d), x, 0x6ED9EBA1ul, r)
#define R41(a,b,c,d,e,x,r) Round(a, b, c, d, e, f4(b, c, d), x, 0x8F1BBCDCul, r)
#define R51(a,b,c,d,e,x,r) Round(a, b, c, d, e, f5(b, c, d), x, 0xA953FD4Eul, r)
#define R12(a,b,c,d,e,x,r) Round(a, b, c, d, e, f5(b, c, d), x, 0x50A28BE6ul, r)
#define R22(a,b,c,d,e,x,r) Round(a, b, c, d, e, f4(b, c, d), x, 0x5C4DD124ul, r)
#define R32(a,b,c,d,e,x,r) Round(a, b, c, d, e, f3(b, c, d), x, 0x6D703EF3ul, r)
#define R42(a,b,c,d,e,x,r) Round(a, b, c, d, e, f2(b, c, d), x, 0x7A6D76E9ul, r)
#define R52(a,b,c,d,e,x,r) Round(a, b, c, d, e, f1(b, c, d), x, 0, r)

#define LOADW(i) _mm_set_epi32(*((uint32_t *)blk[0]+i),*((uint32_t *)blk[1]+i),*((uint32_t *)blk[2]+i),*((uint32_t *)blk[3]+i))

  // Initialize RIPEMD-160 state
  void Initialize(__m128i *s) {
    memcpy(s, _init, sizeof(_init));
  }

  // Perform 4 RIPE in parallel using SSE2
  void Transform(__m128i *s, uint8_t *blk[4]) {

    __m128i a1 = _mm_load_si128(s + 0);
    __m128i b1 = _mm_load_si128(s + 1);
    __m128i c1 = _mm_load_si128(s + 2);
    __m128i d1 = _mm_load_si128(s + 3);
    __m128i e1 = _mm_load_si128(s + 4);
    __m128i a2 = a1;
    __m128i b2 = b1;
    __m128i c2 = c1;
    __m128i d2 = d1;
    __m128i e2 = e1;
    __m128i u;
    __m128i w[16];


    w[0] = LOADW(0);
    w[1] = LOADW(1);
    w[2] = LOADW(2);
    w[3] = LOADW(3);
    w[4] = LOADW(4);
    w[5] = LOADW(5);
    w[6] = LOADW(6);
    w[7] = LOADW(7);
    w[8] = LOADW(8);
    w[9] = LOADW(9);
    w[10] = LOADW(10);
    w[11] = LOADW(11);
    w[12] = LOADW(12);
    w[13] = LOADW(13);
    w[14] = LOADW(14);
    w[15] = LOADW(15);

    R11(a1, b1, c1, d1, e1, w[0], 11);
    R12(a2, b2, c2, d2, e2, w[5], 8);
    R11(e1, a1, b1, c1, d1, w[1], 14);
    R12(e2, a2, b2, c2, d2, w[14], 9);
    R11(d1, e1, a1, b1, c1, w[2], 15);
    R12(d2, e2, a2, b2, c2, w[7], 9);
    R11(c1, d1, e1, a1, b1, w[3], 12);
    R12(c2, d2, e2, a2, b2, w[0], 11);
    R11(b1, c1, d1, e1, a1, w[4], 5);
    R12(b2, c2, d2, e2, a2, w[9], 13);
    R11(a1, b1, c1, d1, e1, w[5], 8);
    R12(a2, b2, c2, d2, e2, w[2], 15);
    R11(e1, a1, b1, c1, d1, w[6], 7);
    R12(e2, a2, b2, c2, d2, w[11], 15);
    R11(d1, e1, a1, b1, c1, w[7], 9);
    R12(d2, e2, a2, b2, c2, w[4], 5);
    R11(c1, d1, e1, a1, b1, w[8], 11);
    R12(c2, d2, e2, a2, b2, w[13], 7);
    R11(b1, c1, d1, e1, a1, w[9], 13);
    R12(b2, c2, d2, e2, a2, w[6], 7);
    R11(a1, b1, c1, d1, e1, w[10], 14);
    R12(a2, b2, c2, d2, e2, w[15], 8);
    R11(e1, a1, b1, c1, d1, w[11], 15);
    R12(e2, a2, b2, c2, d2, w[8], 11);
    R11(d1, e1, a1, b1, c1, w[12], 6);
    R12(d2, e2, a2, b2, c2, w[1], 14);
    R11(c1, d1, e1, a1, b1, w[13], 7);
    R12(c2, d2, e2, a2, b2, w[10], 14);
    R11(b1, c1, d1, e1, a1, w[14], 9);
    R12(b2, c2, d2, e2, a2, w[3], 12);
    R11(a1, b1, c1, d1, e1, w[15], 8);
    R12(a2, b2, c2, d2, e2, w[12], 6);

    R21(e1, a1, b1, c1, d1, w[7], 7);
    R22(e2, a2, b2, c2, d2, w[6], 9);
    R21(d1, e1, a1, b1, c1, w[4], 6);
    R22(d2, e2, a2, b2, c2, w[11], 13);
    R21(c1, d1, e1, a1, b1, w[13], 8);
    R22(c2, d2, e2, a2, b2, w[3], 15);
    R21(b1, c1, d1, e1, a1, w[1], 13);
    R22(b2, c2, d2, e2, a2, w[7], 7);
    R21(a1, b1, c1, d1, e1, w[10], 11);
    R22(a2, b2, c2, d2, e2, w[0], 12);
    R21(e1, a1, b1, c1, d1, w[6], 9);
    R22(e2, a2, b2, c2, d2, w[13], 8);
    R21(d1, e1, a1, b1, c1, w[15], 7);
    R22(d2, e2, a2, b2, c2, w[5], 9);
    R21(c1, d1, e1, a1, b1, w[3], 15);
    R22(c2, d2, e2, a2, b2, w[10], 11);
    R21(b1, c1, d1, e1, a1, w[12], 7);
    R22(b2, c2, d2, e2, a2, w[14], 7);
    R21(a1, b1, c1, d1, e1, w[0], 12);
    R22(a2, b2, c2, d2, e2, w[15], 7);
    R21(e1, a1, b1, c1, d1, w[9], 15);
    R22(e2, a2, b2, c2, d2, w[8], 12);
    R21(d1, e1, a1, b1, c1, w[5], 9);
    R22(d2, e2, a2, b2, c2, w[12], 7);
    R21(c1, d1, e1, a1, b1, w[2], 11);
    R22(c2, d2, e2, a2, b2, w[4], 6);
    R21(b1, c1, d1, e1, a1, w[14], 7);
    R22(b2, c2, d2, e2, a2, w[9], 15);
    R21(a1, b1, c1, d1, e1, w[11], 13);
    R22(a2, b2, c2, d2, e2, w[1], 13);
    R21(e1, a1, b1, c1, d1, w[8], 12);
    R22(e2, a2, b2, c2, d2, w[2], 11);

    R31(d1, e1, a1, b1, c1, w[3], 11);
    R32(d2, e2, a2, b2, c2, w[15], 9);
    R31(c1, d1, e1, a1, b1, w[10], 13);
    R32(c2, d2, e2, a2, b2, w[5], 7);
    R31(b1, c1, d1, e1, a1, w[14], 6);
    R32(b2, c2, d2, e2, a2, w[1], 15);
    R31(a1, b1, c1, d1, e1, w[4], 7);
    R32(a2, b2, c2, d2, e2, w[3], 11);
    R31(e1, a1, b1, c1, d1, w[9], 14);
    R32(e2, a2, b2, c2, d2, w[7], 8);
    R31(d1, e1, a1, b1, c1, w[15], 9);
    R32(d2, e2, a2, b2, c2, w[14], 6);
    R31(c1, d1, e1, a1, b1, w[8], 13);
    R32(c2, d2, e2, a2, b2, w[6], 6);
    R31(b1, c1, d1, e1, a1, w[1], 15);
    R32(b2, c2, d2, e2, a2, w[9], 14);
    R31(a1, b1, c1, d1, e1, w[2], 14);
    R32(a2, b2, c2, d2, e2, w[11], 12);
    R31(e1, a1, b1, c1, d1, w[7], 8);
    R32(e2, a2, b2, c2, d2, w[8], 13);
    R31(d1, e1, a1, b1, c1, w[0], 13);
    R32(d2, e2, a2, b2, c2, w[12], 5);
    R31(c1, d1, e1, a1, b1, w[6], 6);
    R32(c2, d2, e2, a2, b2, w[2], 14);
    R31(b1, c1, d1, e1, a1, w[13], 5);
    R32(b2, c2, d2, e2, a2, w[10], 13);
    R31(a1, b1, c1, d1, e1, w[11], 12);
    R32(a2, b2, c2, d2, e2, w[0], 13);
    R31(e1, a1, b1, c1, d1, w[5], 7);
    R32(e2, a2, b2, c2, d2, w[4], 7);
    R31(d1, e1, a1, b1, c1, w[12], 5);
    R32(d2, e2, a2, b2, c2, w[13], 5);

    R41(c1, d1, e1, a1, b1, w[1], 11);
    R42(c2, d2, e2, a2, b2, w[8], 15);
    R41(b1, c1, d1, e1, a1, w[9], 12);
    R42(b2, c2, d2, e2, a2, w[6], 5);
    R41(a1, b1, c1, d1, e1, w[11], 14);
    R42(a2, b2, c2, d2, e2, w[4], 8);
    R41(e1, a1, b1, c1, d1, w[10], 15);
    R42(e2, a2, b2, c2, d2, w[1], 11);
    R41(d1, e1, a1, b1, c1, w[0], 14);
    R42(d2, e2, a2, b2, c2, w[3], 14);
    R41(c1, d1, e1, a1, b1, w[8], 15);
    R42(c2, d2, e2, a2, b2, w[11], 14);
    R41(b1, c1, d1, e1, a1, w[12], 9);
    R42(b2, c2, d2, e2, a2, w[15], 6);
    R41(a1, b1, c1, d1, e1, w[4], 8);
    R42(a2, b2, c2, d2, e2, w[0], 14);
    R41(e1, a1, b1, c1, d1, w[13], 9);
    R42(e2, a2, b2, c2, d2, w[5], 6);
    R41(d1, e1, a1, b1, c1, w[3], 14);
    R42(d2, e2, a2, b2, c2, w[12], 9);
    R41(c1, d1, e1, a1, b1, w[7], 5);
    R42(c2, d2, e2, a2, b2, w[2], 12);
    R41(b1, c1, d1, e1, a1, w[15], 6);
    R42(b2, c2, d2, e2, a2, w[13], 9);
    R41(a1, b1, c1, d1, e1, w[14], 8);
    R42(a2, b2, c2, d2, e2, w[9], 12);
    R41(e1, a1, b1, c1, d1, w[5], 6);
    R42(e2, a2, b2, c2, d2, w[7], 5);
    R41(d1, e1, a1, b1, c1, w[6], 5);
    R42(d2, e2, a2, b2, c2, w[10], 15);
    R41(c1, d1, e1, a1, b1, w[2], 12);
    R42(c2, d2, e2, a2, b2, w[14], 8);

    R51(b1, c1, d1, e1, a1, w[4], 9);
    R52(b2, c2, d2, e2, a2, w[12], 8);
    R51(a1, b1, c1, d1, e1, w[0], 15);
    R52(a2, b2, c2, d2, e2, w[15], 5);
    R51(e1, a1, b1, c1, d1, w[5], 5);
    R52(e2, a2, b2, c2, d2, w[10], 12);
    R51(d1, e1, a1, b1, c1, w[9], 11);
    R52(d2, e2, a2, b2, c2, w[4], 9);
    R51(c1, d1, e1, a1, b1, w[7], 6);
    R52(c2, d2, e2, a2, b2, w[1], 12);
    R51(b1, c1, d1, e1, a1, w[12], 8);
    R52(b2, c2, d2, e2, a2, w[5], 5);
    R51(a1, b1, c1, d1, e1, w[2], 13);
    R52(a2, b2, c2, d2, e2, w[8], 14);
    R51(e1, a1, b1, c1, d1, w[10], 12);
    R52(e2, a2, b2, c2, d2, w[7], 6);
    R51(d1, e1, a1, b1, c1, w[14], 5);
    R52(d2, e2, a2, b2, c2, w[6], 8);
    R51(c1, d1, e1, a1, b1, w[1], 12);
    R52(c2, d2, e2, a2, b2, w[2], 13);
    R51(b1, c1, d1, e1, a1, w[3], 13);
    R52(b2, c2, d2, e2, a2, w[13], 6);
    R51(a1, b1, c1, d1, e1, w[8], 14);
    R52(a2, b2, c2, d2, e2, w[14], 5);
    R51(e1, a1, b1, c1, d1, w[11], 11);
    R52(e2, a2, b2, c2, d2, w[0], 15);
    R51(d1, e1, a1, b1, c1, w[6], 8);
    R52(d2, e2, a2, b2, c2, w[3], 13);
    R51(c1, d1, e1, a1, b1, w[15], 5);
    R52(c2, d2, e2, a2, b2, w[9], 11);
    R51(b1, c1, d1, e1, a1, w[13], 6);
    R52(b2, c2, d2, e2, a2, w[11], 11);

    __m128i t = s[0];
    s[0] = add3(s[1],c1,d2);
    s[1] = add3(s[2],d1,e2);
    s[2] = add3(s[3],e1,a2);
    s[3] = add3(s[4],a1,b2);
    s[4] = add3(t,b1,c2);
  }

} // namespace ripemd160sse

#ifdef WIN64

#define DEPACK(d,i) \
((uint32_t *)d)[0] = s[0].m128i_u32[i]; \
((uint32_t *)d)[1] = s[1].m128i_u32[i]; \
((uint32_t *)d)[2] = s[2].m128i_u32[i]; \
((uint32_t *)d)[3] = s[3].m128i_u32[i]; \
((uint32_t *)d)[4] = s[4].m128i_u32[i];

#else

#define DEPACK(d,i) \
((uint32_t *)d)[0] = s0[i]; \
((uint32_t *)d)[1] = s1[i]; \
((uint32_t *)d)[2] = s2[i]; \
((uint32_t *)d)[3] = s3[i]; \
((uint32_t *)d)[4] = s4[i];

#endif

static const uint64_t sizedesc_32 = 32 << 3;
static const unsigned char pad[64] = { 0x80 };

void ripemd160sse_32(
  unsigned char *i0,
  unsigned char *i1,
  unsigned char *i2,
  unsigned char *i3,
  unsigned char *d0,
  unsigned char *d1,
  unsigned char *d2,
  unsigned char *d3) {

  __m128i s[5];
  uint8_t *bs[] = { i0,i1,i2,i3 };

  ripemd160sse::Initialize(s);
  memcpy(i0 + 32, pad, 24);
  memcpy(i0 + 56, &sizedesc_32, 8);
  memcpy(i1 + 32, pad, 24);
  memcpy(i1 + 56, &sizedesc_32, 8);
  memcpy(i2 + 32, pad, 24);
  memcpy(i2 + 56, &sizedesc_32, 8);
  memcpy(i3 + 32, pad, 24);
  memcpy(i3 + 56, &sizedesc_32, 8);

  ripemd160sse::Transform(s, bs);

#ifndef WIN64
  uint32_t *s0 = (uint32_t *)&s[0];
  uint32_t *s1 = (uint32_t *)&s[1];
  uint32_t *s2 = (uint32_t *)&s[2];
  uint32_t *s3 = (uint32_t *)&s[3];
  uint32_t *s4 = (uint32_t *)&s[4];
#endif

  DEPACK(d0,3);
  DEPACK(d1,2);
  DEPACK(d2,1);
  DEPACK(d3,0);

}

#endif // !KEYHUNT_AVX2_ONLY

#ifdef KEYHUNT_AVX2_ONLY
namespace ripemd160avx2 {

static const uint8_t indexLeft[80] = {
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
   7, 4,13, 1,10, 6,15, 3,12, 0, 9, 5, 2,14,11, 8,
   3,10,14, 4, 9,15, 8, 1, 2, 7, 0, 6,13,11, 5,12,
   1, 9,11,10, 0, 8,12, 4,13, 3, 7,15,14, 5, 6, 2,
   4, 0, 5, 9, 7,12, 2,10,14, 1, 3, 8,11, 6,15,13
};
static const uint8_t indexRight[80] = {
   5,14, 7, 0, 9, 2,11, 4,13, 6,15, 8, 1,10, 3,12,
   6,11, 3, 7, 0,13, 5,10,14,15, 8,12, 4, 9, 1, 2,
  15, 5, 1, 3, 7,14, 6, 9,11, 8,12, 2,10, 0, 4,13,
   8, 6, 4, 1, 3,11,15, 0, 5,12, 2,13, 9, 7,10,14,
  12,15,10, 4, 1, 5, 8, 7, 6, 2,13,14, 0, 3, 9,11
};
static const uint8_t rotateLeft[80] = {
  11,14,15,12, 5, 8, 7, 9,11,13,14,15, 6, 7, 9, 8,
   7, 6, 8,13,11, 9, 7,15, 7,12,15, 9,11, 7,13,12,
  11,13, 6, 7,14, 9,13,15,14, 8,13, 6, 5,12, 7, 5,
  11,12,14,15,14,15, 9, 8, 9,14, 5, 6, 8, 6, 5,12,
   9,15, 5,11, 6, 8,13,12, 5,12,13,14,11, 8, 5, 6
};
static const uint8_t rotateRight[80] = {
   8, 9, 9,11,13,15,15, 5, 7, 7, 8,11,14,14,12, 6,
   9,13,15, 7,12, 8, 9,11, 7, 7,12, 7, 6,15,13,11,
   9, 7,15,11, 8, 6, 6,14,12,13, 5,14,13,13, 7, 5,
  15, 5, 8,11,14,14, 6,14, 6, 9,12, 9,12, 5,15, 8,
   8, 5,12, 9,12, 5,14, 6, 8,13, 6, 5,15,13,11,11
};

#if defined(__GNUC__) || defined(__clang__)
__attribute__((target("avx2")))
#endif
static void transform(uint8_t **input, uint8_t **digest) {
  __m256i x[16];
  for (int word = 0; word < 8; ++word) {
    uint32_t value[8];
    for (int lane = 0; lane < 8; ++lane) {
      const uint8_t *p = input[lane] + 4 * word;
      value[lane] = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                    ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    }
    x[word] = _mm256_loadu_si256((const __m256i *)value);
  }
  x[8] = _mm256_set1_epi32(0x00000080);
  for (int i = 9; i < 14; ++i) x[i] = _mm256_setzero_si256();
  x[14] = _mm256_set1_epi32(256);
  x[15] = _mm256_setzero_si256();

  const __m256i h0 = _mm256_set1_epi32(0x67452301);
  const __m256i h1 = _mm256_set1_epi32(0xEFCDAB89);
  const __m256i h2 = _mm256_set1_epi32(0x98BADCFE);
  const __m256i h3 = _mm256_set1_epi32(0x10325476);
  const __m256i h4 = _mm256_set1_epi32(0xC3D2E1F0);
  __m256i al=h0, bl=h1, cl=h2, dl=h3, el=h4;
  __m256i ar=h0, br=h1, cr=h2, dr=h3, er=h4;

  for (int i = 0; i < 80; ++i) {
    __m256i fl, fr;
    uint32_t kl, kr;
    if (i < 16) {
      fl = _mm256_xor_si256(bl,_mm256_xor_si256(cl,dl)); kl = 0;
      fr = _mm256_xor_si256(br,_mm256_or_si256(cr,_mm256_xor_si256(dr,_mm256_set1_epi32(-1)))); kr = 0x50A28BE6;
    } else if (i < 32) {
      fl = _mm256_or_si256(_mm256_and_si256(bl,cl),_mm256_andnot_si256(bl,dl)); kl = 0x5A827999;
      fr = _mm256_or_si256(_mm256_and_si256(br,dr),_mm256_andnot_si256(dr,cr)); kr = 0x5C4DD124;
    } else if (i < 48) {
      fl = _mm256_xor_si256(_mm256_or_si256(bl,_mm256_xor_si256(cl,_mm256_set1_epi32(-1))),dl); kl = 0x6ED9EBA1;
      fr = _mm256_xor_si256(_mm256_or_si256(br,_mm256_xor_si256(cr,_mm256_set1_epi32(-1))),dr); kr = 0x6D703EF3;
    } else if (i < 64) {
      fl = _mm256_or_si256(_mm256_and_si256(bl,dl),_mm256_andnot_si256(dl,cl)); kl = 0x8F1BBCDC;
      fr = _mm256_or_si256(_mm256_and_si256(br,cr),_mm256_andnot_si256(br,dr)); kr = 0x7A6D76E9;
    } else {
      fl = _mm256_xor_si256(bl,_mm256_or_si256(cl,_mm256_xor_si256(dl,_mm256_set1_epi32(-1)))); kl = 0xA953FD4E;
      fr = _mm256_xor_si256(br,_mm256_xor_si256(cr,dr)); kr = 0;
    }

    __m256i tl = _mm256_add_epi32(al,fl);
    tl = _mm256_add_epi32(tl,x[indexLeft[i]]);
    tl = _mm256_add_epi32(tl,_mm256_set1_epi32(kl));
    __m256i shift = _mm256_set1_epi32(rotateLeft[i]);
    tl = _mm256_or_si256(_mm256_sllv_epi32(tl,shift),
                         _mm256_srlv_epi32(tl,_mm256_set1_epi32(32-rotateLeft[i])));
    tl = _mm256_add_epi32(tl,el);
    al=el; el=dl; dl=_mm256_or_si256(_mm256_slli_epi32(cl,10),_mm256_srli_epi32(cl,22)); cl=bl; bl=tl;

    __m256i tr = _mm256_add_epi32(ar,fr);
    tr = _mm256_add_epi32(tr,x[indexRight[i]]);
    tr = _mm256_add_epi32(tr,_mm256_set1_epi32(kr));
    shift = _mm256_set1_epi32(rotateRight[i]);
    tr = _mm256_or_si256(_mm256_sllv_epi32(tr,shift),
                         _mm256_srlv_epi32(tr,_mm256_set1_epi32(32-rotateRight[i])));
    tr = _mm256_add_epi32(tr,er);
    ar=er; er=dr; dr=_mm256_or_si256(_mm256_slli_epi32(cr,10),_mm256_srli_epi32(cr,22)); cr=br; br=tr;
  }

  __m256i state[5];
  state[0] = _mm256_add_epi32(h1,_mm256_add_epi32(cl,dr));
  state[1] = _mm256_add_epi32(h2,_mm256_add_epi32(dl,er));
  state[2] = _mm256_add_epi32(h3,_mm256_add_epi32(el,ar));
  state[3] = _mm256_add_epi32(h4,_mm256_add_epi32(al,br));
  state[4] = _mm256_add_epi32(h0,_mm256_add_epi32(bl,cr));
  uint32_t words[5][8];
  for (int i = 0; i < 5; ++i) _mm256_storeu_si256((__m256i *)words[i],state[i]);
  for (int lane = 0; lane < 8; ++lane)
    for (int word = 0; word < 5; ++word)
      ((uint32_t *)digest[lane])[word] = words[word][lane];
}

} // namespace ripemd160avx2

void ripemd160avx2_32_impl(uint8_t **input, uint8_t **digest) {
  ripemd160avx2::transform(input,digest);
}

#else
#include "../cpu_features.h"
void ripemd160avx2_32_impl(uint8_t **input, uint8_t **digest);

void ripemd160avx2_32(uint8_t **input, uint8_t **digest) {
  if (keyhunt_avx2_available()) {
    ripemd160avx2_32_impl(input,digest);
    return;
  }
  ripemd160sse_32(input[0],input[1],input[2],input[3],digest[0],digest[1],digest[2],digest[3]);
  ripemd160sse_32(input[4],input[5],input[6],input[7],digest[4],digest[5],digest[6],digest[7]);
}

void ripemd160sse_test() {

  unsigned char h0[20];
  unsigned char h1[20];
  unsigned char h2[20];
  unsigned char h3[20];
  unsigned char ch0[20];
  unsigned char ch1[20];
  unsigned char ch2[20];
  unsigned char ch3[20];
  unsigned char m0[64];
  unsigned char m1[64];
  unsigned char m2[64];
  unsigned char m3[64];

  strcpy((char *)m0, "This is a test message to test01");
  strcpy((char *)m1, "This is a test message to test02");
  strcpy((char *)m2, "This is a test message to test03");
  strcpy((char *)m3, "This is a test message to test04");

  ripemd160_32(m0, ch0);
  ripemd160_32(m1, ch1);
  ripemd160_32(m2, ch2);
  ripemd160_32(m3, ch3);

  ripemd160sse_32(m0, m1, m2, m3, h0, h1, h2, h3);

  if ((ripemd160_hex(h0) != ripemd160_hex(ch0)) ||
    (ripemd160_hex(h1) != ripemd160_hex(ch1)) ||
    (ripemd160_hex(h2) != ripemd160_hex(ch2)) ||
    (ripemd160_hex(h3) != ripemd160_hex(ch3))) {

    printf("RIPEMD160() Results Wrong !\n");
    printf("RIP: %s\n", ripemd160_hex(ch0).c_str());
    printf("RIP: %s\n", ripemd160_hex(ch1).c_str());
    printf("RIP: %s\n", ripemd160_hex(ch2).c_str());
    printf("RIP: %s\n\n", ripemd160_hex(ch3).c_str());
    printf("SSE: %s\n", ripemd160_hex(h0).c_str());
    printf("SSE: %s\n", ripemd160_hex(h1).c_str());
    printf("SSE: %s\n", ripemd160_hex(h2).c_str());
    printf("SSE: %s\n\n", ripemd160_hex(h3).c_str());

  }

  printf("RIPE() Results OK !\n");

}

#endif // KEYHUNT_AVX2_ONLY
