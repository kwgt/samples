/*
 * Colorspace converter (i420 -> RGB)
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#ifndef __I420_H__
#define __I420_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  int width;
  int height;
  int stride;
  int size;
  void* plane;    // RGB plane

  int y_stride;
  int uv_stride;
} i420_t;

int i420_new(i420_t** dst);
int i420_destroy(i420_t* ptr);
int i420_update(i420_t* ptr, int w, int h, int y_st, int uv_st);
int i420_conv(i420_t* ptr, uint8_t* y, uint8_t* u, uint8_t* v);

#endif /* !defined(__COLSPACE_H__) */

