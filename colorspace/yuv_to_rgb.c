/*
 * YUV to RGB convert
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gamil.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#else /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
#ifdef ENABLE_NEON
#error "ARM NEON instruction is not supported."
#endif /* defined(ENABLE_NEON) */
#endif /* defined(__ARM_NEON) || defined(__ARM_NEON__) */

#define SATURATE8(x)      (uint8_t)(((x) < 0)? 0:(((x) > 255)? 255:(x)))

#ifdef ENABLE_NEON
void
i420_to_rgb(uint8_t* _y, uint8_t* _u, uint8_t* _v, int wd, int ht, uint8_t* _d)
{
  /*
   * 2x2ピクセルを1ユニットとして処理する。
   * ピクセルに対するレジスタのレーン配置は以下の通り。
   *
   *    0 1
   *    2 3
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
   *
   */

  int i;
  int j;

  int32x4_t c16;
  int32x4_t min;
  int32x4_t max;

  c16  = vmovq_n_s32(16);
  min  = vmovq_n_s32(0);
  max  = vmovq_n_s32(255);

#pragma omp parallel for private(j) shared(c16,min,max)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    uint8_t* d1;
    uint8_t* d2;

    int32x4_t tl;  // as "temporary for load"
    int32x4_t vy;
    int32x4_t vr;
    int32x4_t vg;
    int32x4_t vb;

    y1 = _y + (i * wd);
    y2 = y1 + wd;

    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    d1 = _d + (i * (wd * 3));
    d2 = d1 + (wd * 3);

    for (j = 0; j < wd; j += 2) {
      /*
       * Y
       */
      tl = vsetq_lane_s32(y1[0], tl, 0);
      tl = vsetq_lane_s32(y1[1], tl, 1);
      tl = vsetq_lane_s32(y2[0], tl, 2);
      tl = vsetq_lane_s32(y2[1], tl, 3);
      tl = vsubq_s32(tl, c16);

      vy = vmulq_n_s32(tl, 1192);
        
      /*
       * U
       */
      tl = vmovq_n_s32(u[0] - 128);

      vg = vmlsq_n_s32(vy, tl, 400);
      vb = vmlaq_n_s32(vy, tl, 2066);

      /*
       * V
       */
      tl = vmovq_n_s32(v[0] - 128);

      vr = vmlaq_n_s32(vy, tl, 1634); 
      vg = vmlsq_n_s32(vy, tl, 833);

      /*
       * スケールの戻しと飽和処理
       */
      vr = vshrq_n_s32(vr, 10);
      vr = vmaxq_s32(vr, min);
      vr = vminq_s32(vr, max);

      vg = vshrq_n_s32(vg, 10);
      vg = vmaxq_s32(vg, min);
      vg = vminq_s32(vg, max);

      vb = vshrq_n_s32(vb, 10);
      vb = vmaxq_s32(vb, min);
      vb = vminq_s32(vb, max);

      /*
       *  output RGB pixels
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
       * increase pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      d1 += 6;
      d2 += 6;
    }
  }
}
#else /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
void
i420_to_rgb(uint8_t* _y, uint8_t* _u, uint8_t* _v, int wd, int ht, uint8_t* _d)
{
  /*
   * 2x2ピクセルを1ユニットとして処理する。
   * ピクセルに対するレジスタのレーン配置は以下の通り。
   *
   *    0 1
   *    2 3
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


  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    uint8_t* d1;
    uint8_t* d2;

    int c;
    int d;
    int e;

    int r0;
    int g0;
    int b0;

    int r;
    int g;
    int b;

    y1 = _y + (i * wd);
    y2 = y1 + wd;

    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    d1 = _d + (i * (wd * 3));
    d2 = d1 + (wd * 3);

    for (j = 0; j < wd; j += 2) {
      d  = ((int)u[0]) - 128;
      e  = ((int)v[0]) - 128;
      r0 = (e * 1634);
      g0 = (d * 400) + (e * 833);
      b0 = (d * 2066);

      /*
       * 0,0
       */
      c = (((int)y1[0]) - 16) * 1192;
      r = (c + r0) >> 10;
      g = (c - g0) >> 10;
      b = (c + b0) >> 10;

      d1[0] = SATURATE8(r);
      d1[1] = SATURATE8(g);
      d1[2] = SATURATE8(b);

      /*
       * 0,1
       */
      c = (((int)y1[1]) - 16) * 1192;
      r = (c + r0) >> 10;
      g = (c - g0) >> 10;
      b = (c + b0) >> 10;

      d1[3] = SATURATE8(r);
      d1[4] = SATURATE8(g);
      d1[5] = SATURATE8(b);

      /*
       * 1,0
       */
      c = (((int)y2[0]) - 16) * 1192;
      r = (c + r0) >> 10;
      g = (c - g0) >> 10;
      b = (c + b0) >> 10;

      d2[0] = SATURATE8(r);
      d2[1] = SATURATE8(g);
      d2[2] = SATURATE8(b);

      /*
       * 1,1
       */
      c = (((int)y2[1]) - 16) * 1192;
      r = (c + r0) >> 10;
      g = (c - g0) >> 10;
      b = (c + b0) >> 10;

      d2[3] = SATURATE8(r);
      d2[4] = SATURATE8(g);
      d2[5] = SATURATE8(b);

      /*
       * increase pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      d1 += 6;
      d2 += 6;
    }
  }
}
#endif /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
