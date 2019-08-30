#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#else /* defined(__ARM_NEON) || defined(__ARM_NEON__) */
#error "ARM NEON instruction is not supported."
#endif /* defined(__ARM_NEON) || defined(__ARM_NEON__) */

void
planer_yuv_to_rgb(uint8_t* y1, uint8_t* u, uint8_t* v,
                              int wd, int ht, uint8_t* d1)
{
  int i;
  int j;

  float32x4_t c16;
  uint32x4_t lim;

  float32x4_t tl; // as "temporary for load"
  uint32x4_t ts;  // as "temporary for store"

  float32x4_t vr;
  float32x4_t vg;
  float32x4_t vb;

  uint8_t* y2;
  uint8_t* d2;

  c16  = vmovq_n_f32(16.0f);
  lim  = vmovq_n_u32(255);

  y2   = y1 + wd;
  d2   = d1 + (wd * 3);

  for (i = 0; i < ht; i += 2) {
    for (j = 0; j < wd; j += 2) {
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
       */

      /*
       * Y
       */
      tl = vsetq_lane_f32(y1[0], tl, 0);
      tl = vsetq_lane_f32(y1[1], tl, 1);
      tl = vsetq_lane_f32(y2[0], tl, 2);
      tl = vsetq_lane_f32(y2[1], tl, 3);
      tl = vsubq_f32(tl, c16);

      vr = vmulq_n_f32(tl, 1.164f);
      vg = vmulq_n_f32(tl, 1.164f);
      vb = vmulq_n_f32(tl, 1.164f);
        
      /*
       * U
       */
      tl = vmovq_n_f32(u[0] - 128.0f);

      vg = vmlsq_n_f32(vg, tl, 0.391f);
      vb = vmlaq_n_f32(vb, tl, 2.018f);

      /*
       * V
       */
      tl = vmovq_n_f32(v[0] - 128.0f);

      vr = vmlaq_n_f32(vr, tl, 1.596f); 
      vg = vmlsq_n_f32(vg, tl, 0.813f);

      /*
       * RGBのストア処理
       *   vcvtq_u32_f32() で浮動小数点から整数値への変換を行うが、
       *   同時にベクタ中の0未満の値は0に飽和される。従って、上限
       *   側のみ飽和処理を行えば格納する値の正規化が完了する。
       */

      /* R */
      ts = vcvtq_u32_f32(vr);
      ts = vminq_u32(ts, lim);

      d1[0] = vgetq_lane_u32(ts, 0);
      d1[3] = vgetq_lane_u32(ts, 1);
      d2[0] = vgetq_lane_u32(ts, 2);
      d2[3] = vgetq_lane_u32(ts, 3);

      /* G */
      ts = vcvtq_u32_f32(vg);
      ts = vminq_u32(ts, lim);

      d1[1] = vgetq_lane_u32(ts, 0);
      d1[4] = vgetq_lane_u32(ts, 1);
      d2[1] = vgetq_lane_u32(ts, 2);
      d2[4] = vgetq_lane_u32(ts, 3);

      /* B */
      ts = vcvtq_u32_f32(vb);
      ts = vminq_u32(ts, lim);

      d1[2] = vgetq_lane_u32(ts, 0);
      d1[5] = vgetq_lane_u32(ts, 1);
      d2[2] = vgetq_lane_u32(ts, 2);
      d2[5] = vgetq_lane_u32(ts, 3);

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

    y1 += wd;
    y2 += wd;
    d1 += (wd * 3);
    d2 += (wd * 3);
  }
}
