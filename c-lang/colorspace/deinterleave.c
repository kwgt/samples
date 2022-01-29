/*
 * deinterleaver (intervleaved data to planer data convert)
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gamil.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef ENABLE_NEON
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#else /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
#error "ARM NEON instruction is not supported."
#endif /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
#endif /* defined(ENABLE_NEON) */

#ifdef ENABLE_SSE2
#if defined(__SSE2__)

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <x86intrin.h>
#endif /* defined(*) */

#else /* defined(__SSE2__) */
#error "SSE2 instruction is not supported."
#endif /* defined(__SSE2__) */
#endif /* defined(ENABLE_SSE2) */

#ifdef ENABLE_NEON
void
deintl3(uint8_t* src, int wd, int ht,
        uint8_t* dst1, uint8_t* dst2, uint8_t* dst3)
{
  /* wdは16の倍数である事 */

  int i;
  int j;

  uint8x16x3_t a;
  uint8x16_t t;

  wd *= 3;

  for (i = 0; i < ht; i++) {
    for (j = 0; j < wd; j += 48) {
      a = vld3q_u8(src);

      vst1q_u8(dst1, a.val[0]);
      vst1q_u8(dst2, a.val[1]);
      vst1q_u8(dst3, a.val[2]);

      src  += 48;
      dst1 += 16;
      dst2 += 16;
      dst3 += 16;
    }
  }
}
#else /* defined(ENABLE_NEON) */
#error "not supported"
#endif /* defined(ENABLE_NEON) */
