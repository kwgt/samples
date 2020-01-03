/*
 * OPenH264 decode abstracter
 *
 *  Copyright (C) Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "h264dec.h"
#include "wels/codec_api.h"

#define ALLOC(t)        ((t*)malloc(sizeof(t)))
#define N(x)            (sizeof(x) / sizeof(*x))

#define DEFAULT_ERROR   __LINE__

int
h264dec_new(h264dec_t** dst)
{
  int ret;
  int err;

  h264dec_t* obj;
  ISVCDecoder* dec;
  SBufferInfo* bi;
  SDecodingParam* par;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;
  dec = NULL;
  bi  = NULL;
  par = NULL;

  /*
   * check argument
   */
  do {
    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while(0);

  /*
   * alloc memory
   */
  if (!ret) do {
    obj = ALLOC(h264dec_t);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    bi = ALLOC(SBufferInfo);
    if (bi == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    par = ALLOC(SDecodingParam);
    if (par == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while(0);

  /*
   * create decoder object
   */
  if (!ret) {
    err = WelsCreateDecoder(&dec);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * initialize object;
   */
  if (!ret) {
    // h264dec_t
    memset(obj, 0, sizeof(*obj));

    // SBufferInfo
    memset(bi, 0, sizeof(*bi));

    // SDecodingParam
    memset(par, 0, sizeof(*par));
    par->sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
  }

  /*
   * initialize decoder
   */
  if (!ret) {
    err = (*dec)->Initialize(dec, par);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * set return parameter
   */
  if (!ret) {
    obj->decoder = dec;
    obj->buffer  = bi;

    *dst = obj;
  }

  /*
   * post process
   */
  if (ret) {
    if (dec) {
      (*dec)->Uninitialize(dec);
      WelsDestroyDecoder(dec);
    }

    if (bi) free(bi);
    if (obj) free(obj);
  }

  if (par) free(par);

  return ret;
}

int
h264dec_destroy(h264dec_t* ptr)
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
    (*ptr->decoder)->Uninitialize(ptr->decoder);
    WelsDestroyDecoder(ptr->decoder);

    free(ptr->buffer);
    free(ptr);
  }

  return ret;
}

int
h264dec_decode(h264dec_t* ptr, void* slice, size_t size, int* state)
{
  int ret;
  int err;

  ISVCDecoder* dec;
  SBufferInfo* bi;
  uint8_t* dst[3];

  /*
   * intialize
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

    if (slice == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while(0);

  /*
   * decode
   */
  if (!ret) {
    dec = ptr->decoder;
    bi  = ptr->buffer;

    dst[0] = NULL;
    dst[1] = NULL;
    dst[2] = NULL;

    //err = (*dec)->DecodeFrameNoDelay(dec, slice, size, dst, bi);
    err = (*dec)->DecodeFrame2(dec, slice, size, dst, bi);
    if (err) {
      ret = DEFAULT_ERROR;

    } else {
      if (bi->iBufferStatus == 1) {
        SSysMEMBuffer* sb;
        
        sb = &(bi->UsrData.sSystemBuffer);

        ptr->width     = sb->iWidth;
        ptr->height    = sb->iHeight;
        ptr->y         = dst[0];
        ptr->u         = dst[1];
        ptr->v         = dst[2];
        ptr->y_stride  = sb->iStride[0];
        ptr->uv_stride = sb->iStride[1];

      } else {
        ptr->width     = -1;
        ptr->height    = -1;
        ptr->y         = NULL;
        ptr->u         = NULL;
        ptr->v         = NULL;
        ptr->y_stride  = -1;
        ptr->uv_stride = -1;
      }
    }
  }

  /*
   * set return parameter
   */
  if (!ret) {
    *state = bi->iBufferStatus;
  }

  return ret;
}

