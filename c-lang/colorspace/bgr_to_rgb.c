/*
 * BGR to RGB convert
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
rgb_swap(uint8_t* src, int wd, int ht, uint8_t* dst)
{
  /* wdは16の倍数である事 */

  int i;
  int j;

  uint8x16x3_t a;
  uint8x16_t t;

  wd *= 3;

  for (i = 0; i < ht; i++) {
    for (j = 0; j < wd; j += 48) {
      a = vld3q_u8(src + j);
      t = a.val[0];

      a.val[0] = a.val[2];
      a.val[2] = t;

      vst3q_u8(dst + j, a);
    }

    src += wd;
    dst += wd;
  }
}

void
bgr_to_argb(uint8_t* src, int wd, int ht, uint8_t* dst)
{
  /* wdは16の倍数である事 */

  int i;
  int j;
  int k;
  int w3;
  int w4;

  uint8x16x3_t a;
  uint8x16x4_t b;

  w3 = wd * 3;
  w4 = wd * 4;

  b.val[0] = vdupq_n_u8(0xff);

  for (i = 0; i < ht; i++) {
    for (j = 0, k = 0; j < w3; j += 48, k += 64) {
      a = vld3q_u8(src + j);

      b.val[1] = a.val[2];
      b.val[2] = a.val[1];
      b.val[3] = a.val[0];

      vst4q_u8(dst + k, b);
    }

    src += w3;
    dst += w4;
  }
}

#else /* defined(ENABLE_NEON) */
void
rgb_swap(uint8_t* src, int wd, int ht, uint8_t* dst)
{
  int i;
  int j;

  wd *= 3;

  for (i = 0; i < ht; i++) {
    for (j = 0; j < wd; j += 3) {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
    }

    src += wd;
    dst += wd;
  }
}

void
bgr_to_argb(uint8_t* src, int wd, int ht, uint8_t* dst)
{
  int i;
  int j;
  int k;

  int w3;
  int w4;

  w3 = wd * 3;
  w4 = wd * 4;

  for (i = 0; i < ht; i++) {
    for (j = 0, k = 0; j < w3; j += 3, k += 4) {
      dst[j + 0] = 0xff;
      dst[j + 1] = src[k + 2];
      dst[j + 2] = src[k + 1];
      dst[j + 3] = src[k + 0];
    }

    src += w3;
    dst += w4;
  }
}

#endif /* defined(ENABLE_NEON) */
