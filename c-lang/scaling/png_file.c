/*
 * PNG file I/O utility
 *
 * Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

#include "png_file.h"

#define DEFAULT_ERROR     __LINE__

#define PIXEL_FORMAT      PNG_COLOR_TYPE_RGB
#define NUM_CHANNEL       3

#define ALLOC(t)          ((t*)malloc(sizeof(t)))

int
read_png_file(char* file, void** dst, int* dw, int* dh, int* ds)
{
  int ret;

  FILE* fp;
  png_image* ctx;
  size_t stride;
  size_t size;
  void* buf;

  /*
   * initialize
   */
  ret = 0;
  fp  = NULL;
  ctx = NULL;
  buf = NULL;

  /*
   * argument check
   */
  do {
    if (file == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dw == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dh == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (ds == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);


  /*
   * read png
   */
  if (!ret) do {
    fp = fopen(file, "rb");
    if (fp == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    ctx = ALLOC(png_image);
    if (ctx == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    memset(ctx, 0, sizeof(png_image));

    ctx->version = PNG_IMAGE_VERSION;
    ctx->format  = PIXEL_FORMAT;

    png_image_begin_read_from_stdio(ctx, fp);
    if (PNG_IMAGE_FAILED(*ctx)) {
      ret = DEFAULT_ERROR;
      break;
    }

    stride = PNG_IMAGE_ROW_STRIDE(*ctx);
    size   = PNG_IMAGE_BUFFER_SIZE(*ctx, stride);

    buf = malloc(size);
    if (buf == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    png_image_finish_read(ctx, NULL, buf, stride, NULL);
    if (PNG_IMAGE_FAILED(*ctx)) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * put return paramter
   */
  if (!ret) {
    *dst = buf;
    *dw  = ctx->width;
    *dh  = ctx->height;
    *ds  = stride;
  }

  /*
   * post process
   */
  if (ret) {
    if (buf != NULL) free(buf);
  }

  if (ctx != NULL) {
    png_image_free(ctx);
    free(ctx);

  } else {
    if (fp != NULL) fclose(fp);
  }

  return ret;
}

int
write_png_file(void* src, int sw, int sh, int ss, char* file)
{
  int ret;
  FILE* fp;
  png_image* ctx;

  /*
   * initialize
   */
  ret = 0;
  fp  = NULL;
  ctx = NULL;

  /*
   * argument check
   */
  do {
    if (src == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (sw < 16) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (sh < 16) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (ss < (sw * NUM_CHANNEL)) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (file == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * file open
   */
  if (!ret) {
    fp = fopen(file, "wb");
    if (fp == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * write png
   */
  if (!ret) do {
    ctx = ALLOC(png_image);
    if (ctx == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    memset(ctx, 0, sizeof(png_image));

    ctx->version = PNG_IMAGE_VERSION;
    ctx->format  = PIXEL_FORMAT;
    ctx->width   = sw;
    ctx->height  = sh;

    png_image_write_to_stdio(ctx, fp, 0, src, ss, NULL);
    if (PNG_IMAGE_FAILED(*ctx)) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  if (fp != NULL) fclose(fp);
  if (ctx != NULL) {
    png_image_free(ctx);
    free(ctx);
  }

  return ret;
}

