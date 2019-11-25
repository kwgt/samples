/*
 * OpenH264 decoder abstracter
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#ifndef __H264_DECODER_H__
#define __H264_DECODER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "wels/codec_api.h"

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

typedef struct {
  ISVCDecoder* decoder;
  SBufferInfo* buffer;

  int width;
  int height;
  int y_stride;
  int uv_stride;

  uint8_t* y;
  uint8_t* u;
  uint8_t* v;
} h264dec_t;


int h264dec_new(h264dec_t** dst);
int h264dec_destroy(h264dec_t* ptr);
int h264dec_decode(h264dec_t* ptr, void* slice, size_t size, int* state);

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */
#endif /* !defined(__H264_DECODER_H__) */
