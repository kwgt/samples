/*
 * Colorspace converter (i420 -> YUV444)
 *
 *  Copyright (C) 2022 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif /* defined(_OPENMP) */

#ifdef ENABLE_NEON
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#else /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
#error "ARM NEON instruction is not supported."
#endif /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
#endif /* defined(ENABLE_NEON) */

void
i420_to_yuv444(int wd, int ht, int y_stride, int uv_stride,
               uint8_t* src_y, uint8_t* src_u, uint8_t* src_v,
               int stride, uint8_t* dst)
{
  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* sy;
    uint8_t* su;
    uint8_t* sv;

    uint8x8_t u;
    uint8x8_t v;
    uint8x8x3_t a;

    uint8_t* d1;
    uint8_t* d2;

    sy = src_y + (i * y_stride);
    su = src_u + ((i / 2) * uv_stride);
    sv = src_v + ((i / 2) * uv_stride);

    d1 = (uint8_t*)dst + (i * stride);
    d2 = d1 + stride;

    for (j = 0; j < wd; j += 16) {
      u = vld1_u8(su);
      v = vld1_u8(sv);

      // top-left
      a.val[0] = vld1_u8(sy);
      a.val[1] = vzip1_u8(u, u);
      a.val[2] = vzip1_u8(v, v);

      vst3_u8(d1, a);

      // bottom-left
      a.val[0] = vld1_u8(sy + y_stride);

      vst3_u8(d2, a);

      // top-right
      a.val[0] = vld1_u8(sy + 8);
      a.val[1] = vzip2_u8(u, u);
      a.val[2] = vzip2_u8(v, v);

      vst3_u8(d1 + 24, a);

      // bottom-right
      a.val[0] = vld1_u8(sy + 8 + y_stride);

      vst3_u8(d2 + 24, a);

      sy += 16;
      su += 8;
      sv += 8;
         
      d1 += 48;
      d2 += 48;
    }
  }
}
