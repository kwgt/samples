/*
 * YUV to RGB convert
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gamil.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ROTATE0           0x00000000
#define ROTATE90          0x00000001
#define ROTATE180         0x00000002
#define ROTATE270         0x00000003

#define FLIP              0x00000080

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

#define SATURATE8(x)      (uint8_t)(((x) < 0)? 0:(((x) > 255)? 255:(x)))

struct dest_info {
  uint8_t* b0;
  uint8_t* g0;
  uint8_t* r0;
  uint8_t* b1;
  uint8_t* g1;
  uint8_t* r1;

  uint8_t* b2;
  uint8_t* g2;
  uint8_t* r2;
  uint8_t* b3;
  uint8_t* g3;
  uint8_t* r3;
};

static inline void
set0(uint8_t*base, int wd, int ht, int y, struct dest_info* di)
{
  uint8_t* p;

  p = base + (y * (wd * 3));

  di->r0 = p + 0;
  di->g0 = p + 1;
  di->b0 = p + 2;
  di->r1 = p + 3;
  di->g1 = p + 4;
  di->b1 = p + 5;

  p += (wd *3); 

  di->r2 = p + 0;
  di->g2 = p + 1;
  di->b2 = p + 2;
  di->r3 = p + 3;
  di->g3 = p + 4;
  di->b3 = p + 5;
}

static inline void
set0f(uint8_t*base, int wd, int ht, int y, struct dest_info* di)
{
  uint8_t* p;

  p = base + ((y + 1) * (wd * 3));

  di->r0 = p - 6;
  di->g0 = p - 5;
  di->b0 = p - 4;
  di->r1 = p - 3;
  di->g1 = p - 2;
  di->b1 = p - 1;

  p += (wd * 3); 

  di->r2 = p - 6;
  di->g2 = p - 5;
  di->b2 = p - 4;
  di->r3 = p - 3;
  di->g3 = p - 2;
  di->b3 = p - 1;
}

static inline void
inc0(int wd, int ht, struct dest_info* di)
{
  di->b0 += 6;
  di->g0 += 6;
  di->r0 += 6;
  di->b1 += 6;
  di->g1 += 6;
  di->r1 += 6;

  di->b2 += 6;
  di->g2 += 6;
  di->r2 += 6;
  di->b3 += 6;
  di->g3 += 6;
  di->r3 += 6;
}

static inline void
set90(uint8_t*base, int wd, int ht, int y, struct dest_info* di)
{
  uint8_t* p;

  p = base + ((ht - y) * 3);

  di->r0 = p - 3;
  di->g0 = p - 2;
  di->b0 = p - 1;

  di->r2 = p - 6;
  di->g2 = p - 5;
  di->b2 = p - 4;

  p += (ht * 3); 

  di->r1 = p - 3;
  di->g1 = p - 2;
  di->b1 = p - 1;

  di->r3 = p - 6;
  di->g3 = p - 5;
  di->b3 = p - 4;
}

static inline void
set90f(uint8_t*base, int wd, int ht, int y, struct dest_info* di)
{
  uint8_t* p;

  p = base + (y * 3);

  di->r0 = p + 0;
  di->g0 = p + 1;
  di->b0 = p + 2;

  di->r2 = p + 3;
  di->g2 = p + 4;
  di->b2 = p + 5;

  p += (ht * 3); 

  di->r1 = p + 0;
  di->g1 = p + 1;
  di->b1 = p + 2;
                
  di->r3 = p + 3;
  di->g3 = p + 4;
  di->b3 = p + 5;
}


static inline void
inc90(int wd, int ht, struct dest_info* di)
{
  int st;

  st = ht * 6;

  di->b0 += st;
  di->g0 += st;
  di->r0 += st;
  di->b1 += st;
  di->g1 += st;
  di->r1 += st;

  di->b2 += st;
  di->g2 += st;
  di->r2 += st;
  di->b3 += st;
  di->g3 += st;
  di->r3 += st;
}

static inline void
set180(uint8_t*base, int wd, int ht, int y, struct dest_info* di)
{
  uint8_t* p;

  p = base + ((ht - y) * (wd * 3));

  di->r0 = p - 3;
  di->g0 = p - 2;
  di->b0 = p - 1;

  di->r2 = p - 6;
  di->g2 = p - 5;
  di->b2 = p - 4;

  p -= (wd * 3); 

  di->r1 = p - 3;
  di->g1 = p - 2;
  di->b1 = p - 1;

  di->r3 = p - 6;
  di->g3 = p - 5;
  di->b3 = p - 4;
}

static inline void
set180f(uint8_t*base, int wd, int ht, int y, struct dest_info* di)
{
  uint8_t* p;

  p = base + ((ht - (y + 1)) * (wd * 3));

  di->r0 = p + 0;
  di->g0 = p + 1;
  di->b0 = p + 2;

  di->r2 = p + 3;
  di->g2 = p + 4;
  di->b2 = p + 5;

  p -= (wd * 3); 

  di->r1 = p + 0;
  di->g1 = p + 1;
  di->b1 = p + 2;
                
  di->r3 = p + 3;
  di->g3 = p + 4;
  di->b3 = p + 5;
}

static void
inc180(int wd, int ht, struct dest_info* di)
{
  di->b0 -= 6;
  di->g0 -= 6;
  di->r0 -= 6;
  di->b1 -= 6;
  di->g1 -= 6;
  di->r1 -= 6;

  di->b2 -= 6;
  di->g2 -= 6;
  di->r2 -= 6;
  di->b3 -= 6;
  di->g3 -= 6;
  di->r3 -= 6;
}

static void
set270(uint8_t* base, int wd, int ht, int y, struct dest_info* di)
{
  uint8_t* p;

  p = base + (((ht * (wd - 1)) + y) * 3);

  di->r0 = p + 0;
  di->g0 = p + 1;
  di->b0 = p + 2;

  di->r2 = p + 3;
  di->g2 = p + 4;
  di->b2 = p + 5;

  p -= (ht * 3); 

  di->r1 = p + 0;
  di->g1 = p + 1;
  di->b1 = p + 2;

  di->r3 = p + 3;
  di->g3 = p + 4;
  di->b3 = p + 5;
}

static void
set270f(uint8_t* base, int wd, int ht, int y, struct dest_info* di)
{
  uint8_t* p;

  p = base + (((ht * wd) - y) * 3);

  di->r0 = p - 3;
  di->g0 = p - 2;
  di->b0 = p - 1;

  di->r2 = p - 6;
  di->g2 = p - 5;
  di->b2 = p - 4;

  p -= (ht * 3); 

  di->r1 = p - 3;
  di->g1 = p - 2;
  di->b1 = p - 1;
                
  di->r3 = p - 6;
  di->g3 = p - 5;
  di->b3 = p - 4;
}

static void
inc270(int wd, int ht, struct dest_info* di)
{
  int st;

  st = ht * 6;

  di->b0 -= st;
  di->g0 -= st;
  di->r0 -= st;
  di->b1 -= st;
  di->g1 -= st;
  di->r1 -= st;

  di->b2 -= st;
  di->g2 -= st;
  di->r2 -= st;
  di->b3 -= st;
  di->g3 -= st;
  di->r3 -= st;
}

#ifdef ENABLE_NEON
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
static inline void
conv(uint8_t* y1, uint8_t* y2, uint8_t* u, uint8_t* v,
     int32x4_t c16, int32x4_t min, int32x4_t max,
     struct dest_info* di)
{
  int32x4_t tl;  // as "temporary for load"
  int32x4_t vy;
  int32x4_t vr;
  int32x4_t vg;
  int32x4_t vb;

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
  vg = vmlsq_n_s32(vg, tl, 833);

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
  *(di->r0) = vgetq_lane_s32(vr, 0);
  *(di->g0) = vgetq_lane_s32(vg, 0);
  *(di->b0) = vgetq_lane_s32(vb, 0);
  *(di->r1) = vgetq_lane_s32(vr, 1);
  *(di->g1) = vgetq_lane_s32(vg, 1);
  *(di->b1) = vgetq_lane_s32(vb, 1);

  *(di->r2) = vgetq_lane_s32(vr, 2);
  *(di->g2) = vgetq_lane_s32(vg, 2);
  *(di->b2) = vgetq_lane_s32(vb, 2);
  *(di->r3) = vgetq_lane_s32(vr, 3);
  *(di->g3) = vgetq_lane_s32(vg, 3);
  *(di->b3) = vgetq_lane_s32(vb, 3);
}

void
i420_to_rgb_0(uint8_t* _y, uint8_t* _u, uint8_t* _v,
              int wd, int ht, uint8_t* _d)
{
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
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;
    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set0(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, c16, min, max, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointer
       */
      inc0(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_0f(uint8_t* _y, uint8_t* _u, uint8_t* _v,
               int wd, int ht, uint8_t* _d)
{
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
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;

    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set0f(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, c16, min, max, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointer
       */
      inc180(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_90(uint8_t* _y, uint8_t* _u, uint8_t* _v,
              int wd, int ht, uint8_t* _d)
{
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
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;

    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set90(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, c16, min, max, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointer
       */
      inc90(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_90f(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                int wd, int ht, uint8_t* _d)
{
  int i;
  int j;

  int32x4_t c16;
  int32x4_t min;
  int32x4_t max;
  struct dest_info di;

  c16  = vmovq_n_s32(16);
  min  = vmovq_n_s32(0);
  max  = vmovq_n_s32(255);

#pragma omp parallel for private(j) shared(c16,min,max)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;

    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set90f(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, c16, min, max, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointer
       */
      inc90(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_180(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                int wd, int ht, uint8_t* _d)
{
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
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;

    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set180(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, c16, min, max, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointer
       */
      inc180(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_180f(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                 int wd, int ht, uint8_t* _d)
{
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
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;

    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set180f(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, c16, min, max, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointer
       */
      inc0(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_270(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                int wd, int ht, uint8_t* _d)
{
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
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;

    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set270(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, c16, min, max, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointer
       */
      inc270(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_270f(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                 int wd, int ht, uint8_t* _d)
{
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
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;

    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set270f(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, c16, min, max, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointer
       */
      inc270(wd, ht, &di);
    }
  }
}

#else /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
  /*
   * 2x2ピクセルを1ユニットとして処理する。
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
static inline void
conv(uint8_t* y1, uint8_t* y2, uint8_t* u, uint8_t* v, struct dest_info* di)
{
  int c;
  int d;
  int e;

  int r0;
  int g0;
  int b0;

  int r;
  int g;
  int b;

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

  *(di->r0) = SATURATE8(r);
  *(di->g0) = SATURATE8(g);
  *(di->b0) = SATURATE8(b);

  /*
   * 0,1
   */
  c = (((int)y1[1]) - 16) * 1192;
  r = (c + r0) >> 10;
  g = (c - g0) >> 10;
  b = (c + b0) >> 10;

  *(di->r1) = SATURATE8(r);
  *(di->g1) = SATURATE8(g);
  *(di->b1) = SATURATE8(b);

  /*
   * 1,0
   */
  c = (((int)y2[0]) - 16) * 1192;
  r = (c + r0) >> 10;
  g = (c - g0) >> 10;
  b = (c + b0) >> 10;

  *(di->r2) = SATURATE8(r);
  *(di->g2) = SATURATE8(g);
  *(di->b2) = SATURATE8(b);

  /*
   * 1,1
   */
  c = (((int)y2[1]) - 16) * 1192;
  r = (c + r0) >> 10;
  g = (c - g0) >> 10;
  b = (c + b0) >> 10;

  *(di->r3) = SATURATE8(r);
  *(di->g3) = SATURATE8(g);
  *(di->b3) = SATURATE8(b);
}

void
i420_to_rgb_0(uint8_t* _y, uint8_t* _u, uint8_t* _v,
              int wd, int ht, uint8_t* _d)
{
  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;
    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set0(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointers
       */
      inc0(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_0f(uint8_t* _y, uint8_t* _u, uint8_t* _v,
               int wd, int ht, uint8_t* _d)
{
  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;
    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set0f(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointers
       */
      inc180(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_90(uint8_t* _y, uint8_t* _u, uint8_t* _v,
               int wd, int ht, uint8_t* _d)
{
  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;
    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set90(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointers
       */
      inc90(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_90f(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                int wd, int ht, uint8_t* _d)
{
  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;
    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set90f(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointers
       */
      inc90(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_180(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                int wd, int ht, uint8_t* _d)
{
  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;
    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set180(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointers
       */
      inc180(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_180f(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                 int wd, int ht, uint8_t* _d)
{
  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;
    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set180f(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointers
       */
      inc0(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_270(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                int wd, int ht, uint8_t* _d)
{
  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;
    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set270(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointers
       */
      inc270(wd, ht, &di);
    }
  }
}

void
i420_to_rgb_270f(uint8_t* _y, uint8_t* _u, uint8_t* _v,
                 int wd, int ht, uint8_t* _d)
{
  int i;
  int j;

#pragma omp parallel for private(j)
  for (i = 0; i < ht; i += 2) {
    uint8_t* y1;
    uint8_t* y2;
    uint8_t* u;
    uint8_t* v;
    struct dest_info di;

    y1 = _y + (i * wd);
    y2 = y1 + wd;
    u  = _u + ((i / 2) * (wd / 2));
    v  = _v + ((i / 2) * (wd / 2));

    set270f(_d, wd, ht, i, &di);

    for (j = 0; j < wd; j += 2) {
      /*
       * do convert
       */
      conv(y1, y2, u, v, &di);

      /*
       * increase source pointers
       */
      y1 += 2;
      y2 += 2;
      u  += 1;
      v  += 1;

      /*
       * increase destination pointers
       */
      inc270(wd, ht, &di);
    }
  }
}
#endif /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
