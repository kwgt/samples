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

#define PIXEL_FORMAT      PNG_COLOR_TYPE_GRAY
#define NUM_CHANNEL       1

#define ALLOC(t)          ((t*)malloc(sizeof(t)))

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

