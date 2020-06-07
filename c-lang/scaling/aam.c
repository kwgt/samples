/*
 * image shrink by area avaraged method
 *
 * Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "aam.h"

#define DEFAULT_ERROR     __LINE__
#define NUM_CHANNEL       3

// 元の長さと縮小後の長さが素な場合、計算量が爆発してしまうので
// 計算を簡略させる。このための係数。
#define REDCUTION_FACTOR  5

#define ALLOC(t)      ((t*)malloc(sizeof(t)))
#define SWAP(a,b)     do {a = a ^ b; b = a ^ b; a = a ^ b;} while (0)

/* 最大公約数 */
static int 
gcd(int m, int n)
{
  int t;

  if (m < n) SWAP(m, n);

  while (n != 0) {
    t = n;
    n = m % n;
    m = t;
  }

  return m;
}

/* 最小公倍数 */
static int 
lcm(int m, int n)
{
  return (m * n) / gcd(m, n);
}

int
aam_shrinker_new(aam_shrinker_t** dst)
{
  int ret;
  aam_shrinker_t* obj;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;

  /*
   * argument check
   */
  if (dst == NULL) ret = DEFAULT_ERROR;

  /*
   * alloc memory
   */
  if (!ret) {
    obj = ALLOC(aam_shrinker_t);
    if (obj == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * setup context
   */
  if (!ret) {
    obj->src.wd = -1;
    obj->src.ht = -1;
    obj->src.st = -1;
    obj->src.wu = -1;
    obj->src.hu = -1;

    obj->dst.wd = -1;
    obj->dst.ht = -1;
    obj->dst.st = -1;
    obj->dst.wu = -1;
    obj->dst.hu = -1;

    obj->tpl    = NULL;
  }

  /*
   * put return paramter
   */
  if (!ret) *dst = obj;

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
  }

  return ret;
}

int
aam_shrinker_destroy(aam_shrinker_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * arcgument check
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * release memory
   */
  if (!ret) {
    if (ptr->tpl != NULL) free(ptr->tpl);
    free(ptr);
  }

  return ret;
}

int
aam_shrinker_setup(aam_shrinker_t* ptr,
                   int sw, int sh, int ss, int dw, int dh, int ds)
{
  int ret;
  void* tpl;
  int iw;
  int ih;

  /*
   * initialize
   */
  ret = 0;
  tpl = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (sw <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (sh <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (ss < (sw * NUM_CHANNEL)) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dw <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dh <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (ds < (dw * NUM_CHANNEL)) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);
  
  /*
   * alloc memory
   */
  if (!ret) {
    if (sh != ptr->src.ht || ds != ptr->dst.wd) {
      // 縮小作業中に必要となる横方向のみを縮小した画像を格納する領域を確保
      tpl = realloc(ptr->tpl, sh * ds * NUM_CHANNEL);
      if (tpl == NULL)  ret = DEFAULT_ERROR;
    }
  }

  /*
   * set context
   */
  if (!ret) {
    iw = lcm(sw, dw);
    ih = lcm(sh, dh);

    ptr->src.wd = sw;
    ptr->src.ht = sh;
    ptr->src.st = ss;
    ptr->src.wu = iw / sw;
    ptr->src.hu = ih / sh;
           
    ptr->dst.wd = dw;
    ptr->dst.ht = dh;
    ptr->dst.st = ds;

    ptr->dst.wu = iw / dw;
    if (ptr->dst.wu > REDCUTION_FACTOR) {
      ptr->dst.wn = REDCUTION_FACTOR;
      ptr->dst.ws = ptr->dst.wu / REDCUTION_FACTOR;

    } else {
      ptr->dst.wn = ptr->dst.wu;
      ptr->dst.ws = 1;
    }

    ptr->dst.hu = ih / dh;
    if (ptr->dst.hu > REDCUTION_FACTOR) {
      ptr->dst.hn = REDCUTION_FACTOR;
      ptr->dst.hs = ptr->dst.wu / REDCUTION_FACTOR;

    } else {
      ptr->dst.hn = ptr->dst.hu;
      ptr->dst.hs = 1;
    }

    ptr->iw     = iw;
    ptr->ih     = ih;

    if (tpl != NULL) ptr->tpl = tpl;
  }

  /*
   * post process
   */
  if (ret) {
    if (tpl != NULL) free(tpl);
  }

  return ret;
}

int
aam_shrinker_alloc(aam_shrinker_t* ptr, void** dst)
{
  int ret;
  void* buf;

  /*
   * initialize
   */
  ret = 0;
  buf = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * alloc memory
   */
  if (!ret) {
    buf = malloc(ptr->dst.st * ptr->dst.ht);
    if (buf == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * put return paramter
   */
  if (!ret) *dst = buf;

  /*
   * post process
   */
  if (ret) {
    if (buf != NULL) free(buf);
  }

  return ret;
}

int
aam_shrinker_proc(aam_shrinker_t* ptr, void* _src, void* _dst)
{
  int ret;
  uint8_t* src;
  uint8_t* dst;

  int si;   // as "Source-plane index"
  int ii;   // as "Intermediate-plane index"

  int i;
  int j;
  int k;

  int r;
  int g;
  int b;

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

    if (_src == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (_dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * object state check
   */
  if (!ret) {
    if (ptr->tpl == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * do shrink of horizontal
   */
  if (!ret) {
    for (i = 0; i < ptr->src.ht; i++) {
      src = (uint8_t*)_src + (i * ptr->src.st);
      dst = (uint8_t*)ptr->tpl + (i * ptr->dst.st);

      for (j = 0; j < ptr->dst.wd; j++) {
        r  = 0;
        g  = 0;
        b  = 0;

        ii = j * ptr->dst.wu;

        for (k = 0; k < ptr->dst.wn; k++) {
          si  = (ii / ptr->src.wu) * NUM_CHANNEL;

          r  += src[si + 0];
          g  += src[si + 1];
          b  += src[si + 2];

          ii += ptr->dst.ws;
        }

        dst[0] = r / ptr->dst.wn;
        dst[1] = g / ptr->dst.wn;
        dst[2] = b / ptr->dst.wn;

        dst += NUM_CHANNEL;
      }
    }

#if 0
    {
      FILE* fp;
      fp = fopen("afo.ppm", "wb");
      fprintf(fp, "P6\n# %s\n%d %d\n255\n", "test", ptr->dst.wd, ptr->src.ht);
      fwrite(ptr->tpl, ptr->dst.wd * ptr->src.st, 1, fp);
      fclose(fp);
    }
#endif
  }

  /*
   * do shrink of vertical
   */
  if (!ret) {
    for (i = 0; i < ptr->dst.wd; i++) {
      src = (uint8_t*)ptr->tpl + (i * NUM_CHANNEL);
      dst = (uint8_t*)_dst + (i * NUM_CHANNEL);

      for (j = 0; j < ptr->dst.ht; j++) {
        r  = 0;
        g  = 0;
        b  = 0;

        ii = j * ptr->dst.hu;

        for (k = 0; k < ptr->dst.hn; k++) {
          si  = (ii / ptr->src.hu) * ptr->dst.st;

          r  += src[si + 0];
          g  += src[si + 1];
          b  += src[si + 2];

          ii += ptr->dst.hs;
        }

        dst[0] = r / ptr->dst.hn;
        dst[1] = g / ptr->dst.hn;
        dst[2] = b / ptr->dst.hn;

        dst += ptr->dst.st;
      }
    }
  }

  return ret;
}

