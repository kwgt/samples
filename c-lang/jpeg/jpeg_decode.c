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

#define ALLOC(t)            ((t*)malloc(sizeof(t)))

#define ALLOC_ARRAY() \
        ((JSAMPARRAY)malloc(sizeof(JSAMPROW) * UNIT_LINES))

#define ALLOC_ROWS(w,c) \
        ((JSAMPROW)malloc(sizeof(JSAMPLE) * (w) * (c) * UNIT_LINES))

#define DEFAULT_ERROR       __LINE__

static void
decode_output_message(j_common_ptr cinfo)
{
  jpeg_error_t* err;

  err = (jpeg_error_t*)cinfo->err;

  (*err->jerr.format_message)(cinfo, err->msg);
}

static void
decode_emit_message(j_common_ptr cinfo, int msg_level)
{
  jpeg_error_t* err;

  if (msg_level < 0) {
    err = (jpeg_error_t*)cinfo->err;
    (*err->jerr.format_message)(cinfo, err->msg);
    // longjmp(err->jmpbuf, 1);
  }
}

static void
decode_error_exit(j_common_ptr cinfo)
{
  jpeg_error_t* err;

  err = (jpeg_error_t*)cinfo->err;
  (*err->jerr.format_message)(cinfo, err->msg);
  longjmp(err->jmpbuf, 1);
}


int
jpeg_decoder_new(jpeg_decoder_t** dst)
{
  int ret;
  jpeg_decoder_t* obj;
  struct jpeg_decompress_struct* cinfo;
  JSAMPARRAY array;
  jpeg_error_t* error;

  /*
   * initialize
   */
  ret   = 0;
  obj   = NULL;
  cinfo = NULL;
  array = NULL;
  error = NULL;

  /*
   * argument check
   */
  if (dst == NULL) ret = DEFAULT_ERROR;

  /*
   * alloc memory
   */
  if (!ret) do {
    obj = ALLOC(jpeg_decoder_t);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    cinfo = ALLOC(struct jpeg_decompress_struct);
    if (cinfo == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    array = ALLOC_ARRAY();
    if (array == NULL) {
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
   * set initial values
   */
  if (!ret) {
    cinfo->err = jpeg_std_error(&error->jerr);

    error->jerr.output_message = decode_output_message;
    error->jerr.emit_message   = decode_emit_message;
    error->jerr.error_exit     = decode_error_exit;

    obj->cinfo = cinfo;
    obj->array = array;
    obj->error = error;
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
    if (error != NULL) free(error);
  }

  return ret;
}

int
jpeg_decoder_destroy(jpeg_decoder_t* ptr)
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
    free(ptr->error);
    free(ptr);
  }

  return ret;
}

int
jpeg_decoder_decode(jpeg_decoder_t* ptr,
                    void* src, size_t size, void** dst, size_t *wd, size_t* ht)
{
  int ret;
  struct jpeg_decompress_struct* cinfo;
  JSAMPARRAY array;
  size_t width;
  size_t height;
  size_t stride;
  size_t raw_sz;
  uint8_t* raw;
  int i;
  int j;

  /*
   * initialize
   */
  ret = 0;
  raw = NULL;

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
  } while (0);

  /*
   * create decode structure
   */
  if (!ret) {
    cinfo = ptr->cinfo;

    if (!setjmp(ptr->error->jmpbuf)) {
      jpeg_create_decompress(cinfo);

    } else {
      jpeg_destroy_decompress(cinfo);
      ret = DEFAULT_ERROR;
    }
  }

  /*
   * do decode
   */
  if (!ret) {
    array = ptr->array;

    if (!setjmp(ptr->error->jmpbuf)) {
      jpeg_mem_src(cinfo, src, (unsigned long)size);
      jpeg_read_header(cinfo, TRUE);

      cinfo->raw_data_out             = FALSE;
      cinfo->out_color_space          = JCS_EXT_BGR;
      cinfo->out_color_components     = 3;
      cinfo->scale_num                = 1;
      cinfo->scale_denom              = 1;
      cinfo->output_gamma             = 0.0;
      cinfo->do_fancy_upsampling      = FALSE;
      cinfo->do_block_smoothing       = TRUE;
      cinfo->quantize_colors          = FALSE;
      cinfo->dither_mode              = JDITHER_NONE;
      cinfo->two_pass_quantize        = TRUE;
      cinfo->desired_number_of_colors = 0;
      cinfo->enable_1pass_quant       = FALSE;
      cinfo->enable_external_quant    = FALSE;
      cinfo->enable_2pass_quant       = FALSE;

      jpeg_calc_output_dimensions(cinfo);
      jpeg_start_decompress(cinfo);

      width  = cinfo->output_width;
      height = cinfo->output_height;
      stride = cinfo->output_components * cinfo->output_width;
      raw_sz = stride * cinfo->output_height;

      raw = malloc(raw_sz);   
      if (raw == NULL) {
        longjmp(ptr->error->jmpbuf, 1);

      } else {
        while (cinfo->output_scanline < cinfo->output_height) {
          for (i = 0, j = cinfo->output_scanline; i < UNIT_LINES; i++, j++) {
            array[i] = raw + (j * stride);
          }

          jpeg_read_scanlines(cinfo, array, UNIT_LINES);
        }
      }

      jpeg_finish_decompress(cinfo);

    } else {
      jpeg_abort_decompress(cinfo);
      ret = DEFAULT_ERROR;
    }

    jpeg_destroy_decompress(cinfo);
  }

  /*
   * put return parameter
   */
  if (!ret) {
    *dst = raw;
    if (wd != NULL) *wd = width;
    if (ht != NULL) *ht = height;
  }

  /*
   * post process
   */
  if (ret) {
    if (raw != NULL) free(raw);
  }

  return ret;
}
