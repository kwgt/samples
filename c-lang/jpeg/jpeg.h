/*
 * libjpeg wrapper
 *
 *  Copyright (C) 2021 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#ifndef __JPEG_H__
#define __JPEG_H__

#include <jpeglib.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

typedef struct {
  struct jpeg_error_mgr jerr;
  char msg[JMSG_LENGTH_MAX+10];
  jmp_buf jmpbuf;
} jpeg_error_t;

typedef struct {
  struct jpeg_decompress_struct* cinfo;
  JSAMPARRAY array;
  jpeg_error_t* error;
} jpeg_decoder_t;

extern int jpeg_decoder_new(jpeg_decoder_t** dst);
extern int jpeg_decoder_destroy(jpeg_decoder_t* ptr);

extern int jpeg_decoder_decode(jpeg_decoder_t* ptr, void* src, size_t size,
                               void** dst, size_t* wd, size_t* ht);

typedef struct {
  int width;
  int height;
  int stride;

  struct jpeg_compress_struct* cinfo;
  JSAMPARRAY array;
  JSAMPROW rows;
  jpeg_error_t* error;
} jpeg_encoder_t;

extern int jpeg_encoder_new(size_t width, size_t height, size_t stride,
                            jpeg_encoder_t** dst);
extern int jpeg_encoder_destroy(jpeg_encoder_t* ptr);
extern int jpeg_encoder_encode(jpeg_encoder_t* ptr, void* src,
                               void** dst, size_t* dsz);
#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */
#endif /* !defined(__JPEG_H__) */
