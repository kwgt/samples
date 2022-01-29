/*
 * image shrink by bi-linear method
 *
 * Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "nnm.h"

#define DEFAULT_ERROR     __LINE__
#define NUM_CHANNEL       3

// 元の長さと縮小後の長さが素な場合、計算量が爆発してしまうので
// 計算を簡略させる。このための係数。
#define REDCUTION_FACTOR  5

#define ALLOC(t)      ((t*)malloc(sizeof(t)))
#define SWAP(a,b)     do {a = a ^ b; b = a ^ b; a = a ^ b;} while (0)

int
blm_shrinker_new(blm_shrinker_t** dst)
{
  int ret;
  blm_shrinker_t* obj;

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
    obj = ALLOC(blm_shrinker_t);
    if (obj == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * setup context
   */
  if (!ret) {
    obj->src.wd = -1;
    obj->src.ht = -1;
    obj->src.st = -1;

    obj->dst.wd = -1;
    obj->dst.ht = -1;
    obj->dst.st = -1;
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
blm_shrinker_destroy(blm_shrinker_t* ptr)
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
    free(ptr);
  }

  return ret;
}

int
blm_shrinker_setup(blm_shrinker_t* ptr,
                  int sw, int sh, int ss, int dw, int dh, int ds)
{
  int ret;
  int iw;
  int ih;

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
   * set context
   */
  if (!ret) {
    ptr->src.wd = sw;
    ptr->src.ht = sh;
    ptr->src.st = ss;
           
    ptr->dst.wd = dw;
    ptr->dst.ht = dh;
    ptr->dst.st = ds;
  }

  return ret;
}

int
blm_shrinker_alloc(blm_shrinker_t* ptr, void** dst)
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
blm_shrinker_proc(blm_shrinker_t* ptr, void* _src, void* _dst)
{
  int ret;
  uint8_t* src0;
  uint8_t* src;
  uint8_t* dst;

  int si;   // as "Source-plane index"
  int ii;   // as "Intermediate-plane index"

  float rx;
  float ry;
  int dx;
  int dy;

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
   * do shrink of horizontal
   */
  if (!ret) {
    rx = (float)ptr->src.wd / (float)ptr->dst.wd;
    ry = (float)ptr->src.ht / (float)ptr->dst.ht;

    for (dy = 0; dy < ptr->dst.ht; dy++) {
      dst  = (uint8_t*)_dst + (dy * ptr->dst.st);
      src0 = (uint8_t*)_src + ((int)((dy * ry) + 0.5) * ptr->src.st);

      for (dx = 0; dx < ptr->dst.wd; dx++) {
        src = src0 + (((int)((dx * rx) + 0.5)) * NUM_CHANNEL);

        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];

        dst += NUM_CHANNEL;
      }
    }
  }

  return ret;
}

