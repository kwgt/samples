﻿#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef ENABLE_AVX
#include "avx.h"
#endif /* defined(ENABLE_AVX) */

#ifdef _OPENMP
#include <omp.h>
#endif /* defined(_OPENMP) */

#include "i420.h"

#define ALLOC(t)        ((t*)malloc(sizeof(t)))
#define CLIP(v)         (((v) < 0)? 0: ((v) > 255)? 255: (v))
#define DEFAULT_ERROR   __LINE__

int
i420_new(i420_t** dst)
{
  int ret;
  i420_t* obj;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;

  /*
   * argument check
   */
  if (dst == NULL) {
    ret = DEFAULT_ERROR;
  }

  /*
   * alloc memory
   */
  if (!ret) {
    obj = ALLOC(i420_t);
    if (obj == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * put return parameter
   */
  if (!ret) {
    obj->width     = -1;
    obj->height    = -1;
    obj->stride    = -1;
    obj->size      = -1;
    obj->plane     = NULL;

    obj->y_stride  = -1;
    obj->uv_stride = -1;

    *dst = obj;
  }

  /*
   * post process
   */
  if (ret) {
    if (obj) free(obj);
  }
  
  return ret;
}

int
i420_destroy(i420_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) {
    ret = DEFAULT_ERROR;
  }

  /*
   * release memory
   */
  if (!ret) {
    if (ptr->plane) free(ptr->plane);
    free(ptr);
  }

  return ret;
}

int
i420_update(i420_t* ptr, int width, int height, int y_stride, int uv_stride)
{
  int ret;
  int size;
  void* plane;

  /*
   * initialize
   */
  ret   = 0;
  plane = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (width <= 0 || width & 1) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (height <= 0 || height & 1) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (y_stride < width ) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (uv_stride < (width / 2)) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while(0);

  /*
   * alloc memory
   */
  if (!ret) {
    size  = width * height * 3;

    if (ptr->size == size) {
      plane = ptr->plane;

    } else {
      plane = (ptr->plane == NULL)? malloc(size): realloc(ptr->plane, size);
      if (plane == NULL) ret = DEFAULT_ERROR;
    }
  }

  /*
   * update context
   */
  if (!ret) {
    ptr->width     = width;
    ptr->height    = height;
    ptr->stride    = width * 3;
    ptr->size      = size;
    ptr->plane     = plane;

    ptr->y_stride  = y_stride;
    ptr->uv_stride = uv_stride;
  }

  /*
   * post process
   */
  if (ret) {
    if (plane) free(plane);
  }

  return ret;
}

#ifdef ENABLE_AVX

#define DEBUG(r) \
        printf("%016llx %016llx %016llx %016llx\n", r[0], r[1], r[2], r[3])

int
i420_conv(i420_t* ptr, uint8_t* src_y, uint8_t* src_u, uint8_t* src_v)
{
  int ret;
  int i;
  int j;
  int k;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src_y == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src_u == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src_v == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * do convert
   *
   * 4x2ピクセルを1ユニットとして処理。ピクセルに対するレーン配置は以下の通り
   *
   * 0 1 2 3
   * 4 5 6 7
   *
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
  if (!ret) {
#ifdef _OPENMP
#ifdef NUM_THREADS
    omp_set_num_threads(NUM_THREADS);
#endif /* defined(NUM_THREADS) */
#else /* !defined(_OPENMP) */
    __m256i c16   = _mm256_set1_epi32(16);
    __m256i c128  = _mm256_set1_epi32(128);
    __m256i c1192 = _mm256_set1_epi32(1192);
    __m256i c400  = _mm256_set1_epi32(400);
    __m256i c2066 = _mm256_set1_epi32(2066);
    __m256i c1634 = _mm256_set1_epi32(1634);
    __m256i c833  = _mm256_set1_epi32(833);
    __m256i c0    = _mm256_setzero_pd();
    __m256i c255  = _mm256_set1_epi32(255);
#endif /* !defined(_OPENMP) */

#pragma omp parallel for private(j,k)
    for (i = 0; i < ptr->height; i += 2) {
#ifdef _OPENMP
      __m256i c16   = _mm256_set1_epi32(16);
      __m256i c128  = _mm256_set1_epi32(128);
      __m256i c1192 = _mm256_set1_epi32(1192);
      __m256i c400  = _mm256_set1_epi32(400);
      __m256i c2066 = _mm256_set1_epi32(2066);
      __m256i c1634 = _mm256_set1_epi32(1634);
      __m256i c833  = _mm256_set1_epi32(833);
      __m256i c0    = _mm256_setzero_pd();
      __m256i c255  = _mm256_set1_epi32(255);
#endif /* defined(_OPENMP) */

      uint8_t* d1;      // destination pointer for even line
      uint8_t* d2;      // destination pointer for odd line

      uint8_t* yp1;     // y-plane pointer for even line
      uint8_t* yp2;     // y-plane pointer for odd line
      uint8_t* up;      // u-plane pointer
      uint8_t* vp;      // v-plane pointer

      d1  = ptr->plane + (i * ptr->stride);
      d2  = d1 + ptr->stride;

      yp1 = src_y + (i * ptr->y_stride);
      yp2 = yp1 + ptr->y_stride;
      up  = src_u + ((i / 2) * ptr->uv_stride);
      vp  = src_v + ((i / 2) * ptr->uv_stride);

      for (j = 0; j < ptr->width; j += 4) {
        __m256i vy;
        __m256i vu;
        __m256i vv;
        __m256i vr;
        __m256i vg;
        __m256i vb;

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
        vb = _mm256_max_epi32(vb, c0);
        vb = _mm256_min_epi32(vb, c255);

        /*
         * R
         */
        vr = _mm256_mullo_epi32(vv, c1634);
        vr = _mm256_add_epi32(vy, vr);
        vr = _mm256_srai_epi32(vr, 10);
        vr = _mm256_max_epi32(vr, c0);
        vr = _mm256_min_epi32(vr, c255);

        /*
         * G
         */
        vu = _mm256_mullo_epi32(vu, c400);
        vv = _mm256_mullo_epi32(vv, c833);
        vg = _mm256_sub_epi32(vy, vv);
        vg = _mm256_sub_epi32(vg, vu);
        vg = _mm256_srai_epi32(vg, 10);
        vg = _mm256_max_epi32(vg, c0);
        vg = _mm256_min_epi32(vg, c255);

        /*
         * store result
         */
        for (k = 0; k < 4; k++) {
          d1[0] = _mm256_extract_epi32(vr, k);
          d1[1] = _mm256_extract_epi32(vg, k);
          d1[2] = _mm256_extract_epi32(vb, k);

          d1 += 3;
        }

        for (k = 4; k < 8; k++) {
          d2[0] = _mm256_extract_epi32(vr, k);
          d2[1] = _mm256_extract_epi32(vg, k);
          d2[2] = _mm256_extract_epi32(vb, k);

          d2 += 3;
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

  return ret;
}
#else /* defined(ENABLE_AVX) */
int
i420_conv(i420_t* ptr, uint8_t* src_y, uint8_t* src_u, uint8_t* src_v)
{
  int ret;
  int i;
  int j;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src_y == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src_u == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src_v == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * do convert
   */
  if (!ret) {

#pragma omp parallel for private(j)
    for (i = 0; i < ptr->height; i += 2) {
      uint8_t* d1;      // destination pointer for even line
      uint8_t* d2;      // destination pointer for odd line
      uint8_t* yp1;     // y-plane pointer for even line
      uint8_t* yp2;     // y-plane pointer for odd line
      uint8_t* up;      // u-plane pointer
      uint8_t* vp;      // v-plane pointer

      d1  = ptr->plane + (i * ptr->stride);
      d2  = d1 + ptr->stride;
      yp1 = src_y + (i * ptr->y_stride);
      yp2 = yp1 + ptr->y_stride;
      up  = src_u + ((i / 2) * ptr->uv_stride);
      vp  = src_v + ((i / 2) * ptr->uv_stride);

      for (j = 0; j < ptr->width; j += 2) {
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

  return ret;
}
#endif /* defined(ENABLE_AVX) */
