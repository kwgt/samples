/*
 * Colorspace converter (i420 -> RGB)
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif /* defined(_OPENMP) */

#if defined(ENABLE_NEON)
#include "neon.h"

void
i420_to_rgb(int wd, int ht, int y_stride, int uv_stride,
            uint8_t* src_y, uint8_t* src_u, uint8_t* src_v,
            int stride, uint8_t* dst)
{
  int i;
  int j;

  /*
   * do convert
   *
   * 2x2ピクセルを1ユニットとして処理。ピクセルに対するレーン配置は以下の通り
   *
   * 0 1
   * 2 3
   *
   * YUVからRGBへの変換式は以下の通り
   *
   *   R = (1.164f * (y - 16)) + (1.596f * (v - 128))
   *   G = (1.164f * (y - 16)) - (0.813f * (v - 128)) - (0.391f * (u - 128))
   *   B = (1.164f * (y - 16)) + (2.018f * (u - 128))
   *
   * 上記を、整数演算化による高速化を狙って以下の様に実装する。
   *
   *   R = ((1192 * (y - 16)) + (1634 * (v - 128))) >> 10
   *   G = ((1192 * (y - 16)) - ( 833 * (v - 128)) - (400 * (u - 128))) >> 10
   *   B = ((1192 * (y - 16)) + (2066 * (u - 128))) >> 10
   */
#if defined(_OPENMP) && defined(NUM_THREADS)
  omp_set_num_threads(NUM_THREADS);
#endif /* defined(_OPENMP) && defined(NUM_THREADS) */

#pragma omp parallel private(j) shared(ptr,src_y,src_u,src_v)
  {
    int32x4_t c16  = vmovq_n_s32(16);
    int32x4_t c0   = vmovq_n_s32(0);
    int32x4_t c255 = vmovq_n_s32(255);

    uint8_t* d1;
    uint8_t* d2;

    uint8_t* yp1;
    uint8_t* yp2;
    uint8_t* up;
    uint8_t* vp;

    int32x4_t vy;
    int32x4_t vu;
    int32x4_t vv;
    int32x4_t vr;
    int32x4_t vg;
    int32x4_t vb;

#pragma omp for
    for (i = 0; i < ht; i += 2) {
      d1  = (uint8_t*)dst + (i * stride);
      d2  = d1 + stride;

      yp1 = src_y + (i * y_stride);
      yp2 = yp1 + y_stride;
      up  = src_u + ((i / 2) * uv_stride);
      vp  = src_v + ((i / 2) * uv_stride);

      for (j = 0; j < wd; j += 2) {
        /*
         * Y
         */
        vy = vsetq_lane_s32(yp1[0], vy, 0);
        vy = vsetq_lane_s32(yp1[1], vy, 1);
        vy = vsetq_lane_s32(yp2[0], vy, 2);
        vy = vsetq_lane_s32(yp2[1], vy, 3);

        vy = vsubq_s32(vy, c16);
        vy = vmulq_n_s32(vy, 1192);

        /*
         * U
         */
        vu = vmovq_n_s32(up[0] - 128);

        /*
         * V
         */
        vv = vmovq_n_s32(vp[0] - 128);

        /*
         * B
         */
        vb = vmlaq_n_s32(vy, vu, 2066);
        vb = vshrq_n_s32(vb, 10);
        vb = vmaxq_s32(vb, c0);
        vb = vminq_s32(vb, c255);

        /*
         * R
         */
        vr = vmlaq_n_s32(vy, vv, 1634);
        vr = vshrq_n_s32(vr, 10);
        vr = vmaxq_s32(vr, c0);
        vr = vminq_s32(vr, c255); 

        /*
         * G
         */
        vg = vmlsq_n_s32(vg, vv, 833);
        vg = vmlsq_n_s32(vy, vu, 400);
        vg = vshrq_n_s32(vg, 10);
        vg = vmaxq_s32(vg, c0);
        vg = vminq_s32(vg, c255);

        /*
         *  store result
         */
        d1[0] = vgetq_lane_s32(vr, 0);
        d1[1] = vgetq_lane_s32(vg, 0);
        d1[2] = vgetq_lane_s32(vb, 0);

        d1[3] = vgetq_lane_s32(vr, 1);
        d1[4] = vgetq_lane_s32(vg, 1);
        d1[5] = vgetq_lane_s32(vb, 1);

        d2[0] = vgetq_lane_s32(vr, 2);
        d2[1] = vgetq_lane_s32(vg, 2);
        d2[2] = vgetq_lane_s32(vb, 2);

        d2[3] = vgetq_lane_s32(vr, 3);
        d2[4] = vgetq_lane_s32(vg, 3);
        d2[5] = vgetq_lane_s32(vb, 3);

        /*
         * update pointer
         */
        yp1 += 2;
        yp2 += 2;
        up  += 1;
        vp  += 1;

        d1  += 6;
        d2  += 6;
      }
    }
  }
}

#elif defined(ENABLE_AVX)
#include "avx.h"

void
i420_to_rgb(int wd, int ht, int y_stride, int uv_stride,
            uint8_t* src_y, uint8_t* src_u, uint8_t* src_v,
            int stride, uint8_t* dst)
{
  int i;
  int j;
  int k;


  /*
   * do convert
   *
   * 4x2ピクセルを1ユニットとして処理。ピクセルに対するレーン配置は以下の通り
   *
   * 0 1 2 3
   * 4 5 6 7
   *
   * YUVからRGBへの変換式は以下の通り
   *
   *   R = (1.164f * (y - 16)) + (1.596f * (v - 128))
   *   G = (1.164f * (y - 16)) - (0.813f * (v - 128)) - (0.391f * (u - 128))
   *   B = (1.164f * (y - 16)) + (2.018f * (u - 128))
   *
   * 上記を、整数演算化による高速化を狙って以下の様に実装する。
   *
   *   R = ((1192 * (y - 16)) + (1634 * (v - 128))) >> 10
   *   G = ((1192 * (y - 16)) - ( 833 * (v - 128)) - (400 * (u - 128))) >> 10
   *   B = ((1192 * (y - 16)) + (2066 * (u - 128))) >> 10
   */
#if defined(_OPENMP) && defined(NUM_THREADS)
  omp_set_num_threads(NUM_THREADS);
#endif /* defined(_OPENMP) && defined(NUM_THREADS) */

#pragma omp parallel private(j,k) shared(ptr,src_y,src_u,src_v)
  {
    __m256i c16   = _mm256_set1_epi32(16);
    __m256i c128  = _mm256_set1_epi32(128);
    __m256i c1192 = _mm256_set1_epi32(1192);
    __m256i c400  = _mm256_set1_epi32(400);
    __m256i c2066 = _mm256_set1_epi32(2066);
    __m256i c1634 = _mm256_set1_epi32(1634);
    __m256i c833  = _mm256_set1_epi32(833);

    uint8_t* d1;      // destination pointer for even line
    uint8_t* d2;      // destination pointer for odd line

    uint8_t* yp1;     // y-plane pointer for even line
    uint8_t* yp2;     // y-plane pointer for odd line
    uint8_t* up;      // u-plane pointer
    uint8_t* vp;      // v-plane pointer

    __m256i vy;
    __m256i vu;
    __m256i vv;
    __m256i vr;
    __m256i vg;
    __m256i vb;

#pragma omp for 
    for (i = 0; i < ht; i += 2) {
      d1  = dst + (i * stride);
      d2  = d1 + stride;

      yp1 = src_y + (i * y_stride);
      yp2 = yp1 + y_stride;
      up  = src_u + ((i / 2) * uv_stride);
      vp  = src_v + ((i / 2) * uv_stride);

      for (j = 0; j < wd; j += 4) {
        /*
         * 飽和演算はストア時のパック処理で併せて行っているので注意。
         */

        /*
         * Y
         */
        vy = _mm256_set_epi32(yp2[3], yp2[2], yp2[1], yp2[0],
                              yp1[3], yp1[2], yp1[1], yp1[0]);

        vy = _mm256_sub_epi32(vy, c16);
        vy = _mm256_mullo_epi32(vy, c1192);

        /*
         * U
         */
        vu = _mm256_set_epi32(up[1], up[1], up[0], up[0],
                              up[1], up[1], up[0], up[0]);

        vu = _mm256_sub_epi32(vu, c128);

        /*
         * V
         */
        vv = _mm256_set_epi32(vp[1], vp[1], vp[0], vp[0],
                              vp[1], vp[1], vp[0], vp[0]);

        vv = _mm256_sub_epi32(vv, c128);

        /*
         * B
         */
        vb = _mm256_mullo_epi32(vu, c2066);
        vb = _mm256_add_epi32(vy, vb);
        vb = _mm256_srai_epi32(vb, 10);

        /*
         * R
         */
        vr = _mm256_mullo_epi32(vv, c1634);
        vr = _mm256_add_epi32(vy, vr);
        vr = _mm256_srai_epi32(vr, 10);

        /*
         * G
         */
        vu = _mm256_mullo_epi32(vu, c400);
        vv = _mm256_mullo_epi32(vv, c833);
        vg = _mm256_sub_epi32(vy, vv);
        vg = _mm256_sub_epi32(vg, vu);
        vg = _mm256_srai_epi32(vg, 10);

        /*
         * store result
         */

        {
          alignas(32) union {
            __m256i ymm;
            uint8_t u8[32];
          } buf;

          /*
           * vr:
           *    0           16
           *   +--+--+--+--+--+--+--+--+
           *   |R7|R6|R5|R4|R3|R2|R1|R0|
           *   +--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           *
           * vg:
           *    0           16
           *   +--+--+--+--+--+--+--+--+
           *   |G7|G6|G5|G4|G3|G2|G1|G0|
           *   +--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           *
           *    |
           *    V
           *
           * vr:
           *    0           8           16          24
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |G7|G6|G5|G4|R7|R6|R5|R4|G3|G2|G1|G0|R3|R2|R1|R0|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           */
          vr = _mm256_packs_epi32(vr, vg);

          /*
           * vb:
           *    0           16
           *   +--+--+--+--+--+--+--+--+
           *   |B7|B6|B5|B4|B3|B2|B1|B0|
           *   +--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           *
           * vb:
           *    0           16
           *   +--+--+--+--+--+--+--+--+
           *   |B7|B6|B5|B4|B3|B2|B1|B0|
           *   +--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           *
           *    |
           *    V
           *
           * vb:
           *    0           8           16          24
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |B7|B6|B5|B4|B7|B6|B5|B4|B3|B2|B1|B0|B3|B2|B1|B0|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           */
          vb = _mm256_packs_epi32(vb, vb);

          /*
           * vr:
           *    0           8           16          24
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |G7|G6|G5|G4|R7|R6|R5|R4|G3|G2|G1|G0|R3|R2|R1|R0|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           *
           * vb:
           *    0           8           16          24
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |B7|B6|B5|B4|B7|B6|B5|B4|B3|B2|B1|B0|B3|B2|B1|B0|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           * 
           *   |
           *   V
           *
           * vr:
           *    0           4           8           12
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |B7|B6|B5|B4|B7|B6|B5|B4|G7|G6|G5|G4|R7|R6|R5|R4|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |B3|B2|B1|B0|B3|B2|B1|B0|G3|G2|G1|G0|R3|R2|R1|R0|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           */
          vr = _mm256_packus_epi16(vr, vb);

          /*
           * store
           *
           * ymm:
           *    0           4           8           12
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |B7|B6|B5|B4|B7|B6|B5|B4|G7|G6|G5|G4|R7|R6|R5|R4|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |B3|B2|B1|B0|B3|B2|B1|B0|G3|G2|G1|G0|R3|R2|R1|R0|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           *
           *   | (64bit単位のリトルエンディアンなので、
           *   |  メモリイメージは64bit単位で反転)
           *   V
           *
           * mem:
           *    0           4           8           12
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |R4|R5|R6|R7|G4|G5|G6|G7|B4|B5|B6|B7|B4|B5|B6|B7|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   |R0|R1|R2|R3|G0|G1|G2|G3|B0|B1|B2|B3|B0|B1|B2|B3|
           *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
           *   ※1単位 1byte(8bit)
           */
          _mm256_store_si256(&buf.ymm, vr);

          for (k = 0; k < 4; k++) {
            d1[0] = buf.u8[k + 0];
            d1[1] = buf.u8[k + 4];
            d1[2] = buf.u8[k + 8];

            d2[0] = buf.u8[k + 16];
            d2[1] = buf.u8[k + 20];
            d2[2] = buf.u8[k + 24];

            d1 += 3;
            d2 += 3;
          }
        }

        /*
         * update pointer
         */
        yp1 += 4;
        yp2 += 4;
        up  += 2;
        vp  += 2;
      }
    }
  }
}

#else /* * */
#define CLIP(v)         (((v) < 0)? 0: ((v) > 255)? 255: (v))

void
i420_to_rgb(int wd, int ht, int y_stride, int uv_stride,
            uint8_t* src_y, uint8_t* src_u, uint8_t* src_v,
            int stride, uint8_t* dst)
{
  int i;

  /*
   * do convert
   */
#ifdef NUM_THREADS
  omp_set_num_threads(NUM_THREADS);
#endif /* defined(NUM_THREADS) */
#pragma omp parallel for
  for (i = 0; i < ht; i += 2) {
    int j;
    uint8_t* d1;      // destination pointer for even line
    uint8_t* d2;      // destination pointer for odd line
    uint8_t* yp1;     // y-plane pointer for even line
    uint8_t* yp2;     // y-plane pointer for odd line
    uint8_t* up;      // u-plane pointer
    uint8_t* vp;      // v-plane pointer

    d1  = dst + (i * stride);
    d2  = d1 + stride;
    yp1 = src_y + (i * y_stride);
    yp2 = yp1 + y_stride;
    up  = src_u + ((i / 2) * uv_stride);
    vp  = src_v + ((i / 2) * uv_stride);

    for (j = 0; j < wd; j += 2) {
      int y;
      int u;
      int v;

      int r0;
      int g0;
      int b0;

      u  = (int)*up - 128;
      v  = (int)*vp - 128;

      r0 = (v * 1634);
      g0 = (v * 833) + (u * 400);
      b0 = (u * 2066);

      /* [0,0] */
      y = ((int)yp1[0] - 16) * 1192;

      *d1++ = CLIP((y + r0) >> 10);
      *d1++ = CLIP((y - g0) >> 10);
      *d1++ = CLIP((y + b0) >> 10);

      /* [0,1] */
      y = ((int)yp2[0] - 16) * 1192;

      *d2++ = CLIP((y + r0) >> 10);
      *d2++ = CLIP((y - g0) >> 10);
      *d2++ = CLIP((y + b0) >> 10);

      /* [1,0] */
      y = ((int)yp1[1] - 16) * 1192;

      *d1++ = CLIP((y + r0) >> 10);
      *d1++ = CLIP((y - g0) >> 10);
      *d1++ = CLIP((y + b0) >> 10);

      /* [1,1] */
      y = ((int)yp2[1] - 16) * 1192;

      *d2++ = CLIP((y + r0) >> 10);
      *d2++ = CLIP((y - g0) >> 10);
      *d2++ = CLIP((y + b0) >> 10);

      yp1 += 2;
      yp2 += 2;
      up++;
      vp++;
    }
  }
}
#endif /* * */

