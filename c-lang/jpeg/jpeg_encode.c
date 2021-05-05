/*
 * libjpeg wrapper
 *
 *  Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>

#include <jpeglib.h>
#include "jpeg.h"

#define UNIT_LINES          10
#define ENCODE_QUALITY      75

#define ALLOC(t)            ((t*)malloc(sizeof(t)))

#define ALLOC_ARRAY() \
        ((JSAMPARRAY)malloc(sizeof(JSAMPROW) * UNIT_LINES))

#define ALLOC_ROWS(w,c) \
        ((JSAMPROW)malloc(sizeof(JSAMPLE) * (w) * (c) * UNIT_LINES))

#define DEFAULT_ERROR       __LINE__

static void
output_message(j_common_ptr cinfo)
{
  jpeg_error_t* err;

  err = (jpeg_error_t*)cinfo->err;

  (*err->jerr.format_message)(cinfo, err->msg);
}

static void
emit_message(j_common_ptr cinfo, int msg_level)
{
  jpeg_error_t* err;

  if (msg_level < 0) {
    err = (jpeg_error_t*)cinfo->err;
    (*err->jerr.format_message)(cinfo, err->msg);
    // longjmp(err->jmpbuf, 1);
  }
}

static void
error_exit(j_common_ptr cinfo)
{
  jpeg_error_t* err;

  err = (jpeg_error_t*)cinfo->err;
  (*err->jerr.format_message)(cinfo, err->msg);
  longjmp(err->jmpbuf, 1);
}

int
jpeg_encoder_new(size_t wd, size_t ht, size_t st, jpeg_encoder_t** dst)
{
  int ret;
  jpeg_encoder_t* obj;
  struct jpeg_compress_struct* cinfo;
  JSAMPARRAY array;
  JSAMPROW rows;
  jpeg_error_t* error;
  int i;

  /*
   * initialize
   */
  ret   = 0;
  obj   = NULL;
  cinfo = NULL;
  array = NULL;
  rows  = NULL;
  error = NULL;

  /*
   * argument check
   */
  do {
    if (wd < 64) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (ht < 64) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (st != 0 && st < (wd * 3)) {
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
  if (!ret) do {
    obj = ALLOC(jpeg_encoder_t);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    cinfo = ALLOC(struct jpeg_compress_struct);
    if (cinfo == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    array = ALLOC_ARRAY();
    if (array == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    rows = ALLOC_ROWS(wd, 3);
    if (rows == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    error = ALLOC(jpeg_error_t);
    if (error == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * setup context
   */
  if (!ret) {
    error->jerr.output_message = output_message;
    error->jerr.emit_message   = emit_message;
    error->jerr.error_exit     = error_exit;

    for (i = 0; i < UNIT_LINES; i++) array[i] = rows + (i * wd * 3);

    obj->width  = wd;
    obj->height = ht;
    obj->stride = (st != 0)? st: (wd * 3);
    obj->cinfo  = cinfo;
    obj->array  = array;
    obj->rows   = rows;
    obj->error  = error;
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = obj;

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
    if (cinfo != NULL) free(cinfo);
    if (array != NULL) free(array);
    if (rows != NULL) free(rows);
    if (error != NULL) free(error);
  }

  return ret;
}

int
jpeg_encoder_destroy(jpeg_encoder_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * release memory
   */
  if (!ret) {
    free(ptr->cinfo);
    free(ptr->array);
    free(ptr->rows);
    free(ptr->error);
    free(ptr);
  }

  return ret;
}

int
jpeg_encoder_encode(jpeg_encoder_t* ptr, void* src, void** dst, size_t *dsz)
{
  int ret;
  struct jpeg_compress_struct* cinfo;
  JSAMPARRAY array;
  JSAMPROW rows;
  uint8_t* raw;
  int i;
  int n;
  int width;

  unsigned char* mem;
  unsigned long size;

  /*
   * initialize
   */
  ret = 0;
  mem = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dsz == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * create encode structure
   */
  if (!ret) {
    cinfo = ptr->cinfo;

    if (!setjmp(ptr->error->jmpbuf)) {
      cinfo->err = jpeg_std_error(&ptr->error->jerr);
      jpeg_create_compress(cinfo);

    } else {
      jpeg_destroy_compress(cinfo);
      ret = DEFAULT_ERROR;
    }
  }

  /*
   * do encode
   */
  if (!ret) {
    array = ptr->array;
    width = ptr->width * 3;
    raw   = (uint8_t*)src;

    if (!setjmp(ptr->error->jmpbuf)) {
      jpeg_mem_dest(cinfo, &mem, &size);

      cinfo->image_width      = ptr->width;
      cinfo->image_height     = ptr->height;
      cinfo->in_color_space   = JCS_EXT_BGR;
      cinfo->input_components = 3;
      cinfo->optimize_coding  = TRUE;
      cinfo->arith_code       = TRUE;
      cinfo->raw_data_in      = FALSE;
      cinfo->dct_method       = JDCT_IFAST;

      jpeg_set_defaults(cinfo);
      jpeg_set_quality(cinfo, ENCODE_QUALITY, TRUE);
      jpeg_suppress_tables(cinfo, TRUE);

      jpeg_start_compress(cinfo, TRUE);

      while (cinfo->next_scanline < cinfo->image_height) {
        n = cinfo->image_height - cinfo->next_scanline;
        if (n > UNIT_LINES) n = UNIT_LINES;

        rows = ptr->rows;

        for (i = 0; i < n; i++) {
          memcpy(rows, raw, width);

          rows += width;
          raw  += ptr->stride;
        }

        jpeg_write_scanlines(cinfo, array, n);
      }

      jpeg_finish_compress(cinfo);

    } else {
      jpeg_abort_compress(cinfo);
      ret = DEFAULT_ERROR;
    }

    jpeg_destroy_compress(cinfo);
  }

  /*
   * put return parameter
   */
  if (!ret) {
    *dst = mem;
    *dsz = size;
  }

  /*
   * post process
   */
  if (ret) {
    if (mem != NULL) free(mem);
  }

  return ret;
}
