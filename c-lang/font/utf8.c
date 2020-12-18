/*
 *  Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#include <stdlib.h>
#include <stdint.h>
#include <uchar.h>

#include "utf8.h"

#ifndef __STDC_UTF_32__
#error "not supported compile environment"
#endif /* !defined(__STDC_UTF_32__) */

#define DEFAULT_ERROR     __LINE__

#define IS_CONT(c)        (((c) & 0xc0) == 0x80)

#define IS_7BITS(p)       ((((p[0]) & 0x80) == 0x00))

#define IS_11BITS(p)      ((((p[0]) & 0xe0) == 0xc0) && \
                           IS_CONT(p[1]))

#define IS_16BITS(p)      ((((p[0]) & 0xf0) == 0xe0) && \
                           IS_CONT(p[1]) && \
                           IS_CONT(p[2]))

#define IS_21BITS(p)      ((((p[0]) & 0xf8) == 0xf0) && \
                           IS_CONT(p[1]) && \
                           IS_CONT(p[2]) && \
                           IS_CONT(p[3]))

int
utf8_len(char* s)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (s == NULL) ret = DEFAULT_ERROR;

  /*
   * count number of characters
   */
  if (!ret) {
    while (s[0] != '\0') {
      if (IS_7BITS(s)) {
        ret++;
        s += 1;

      } else if (IS_11BITS(s)) {
        ret++;
        s += 2;

      } else if (IS_16BITS(s)) {
        ret++;
        s += 3;

      } else if (IS_21BITS(s)) {
        ret++;
        s += 4;

      } else {
        ret = DEFAULT_ERROR;
        break;
      }
    }
  }

  return ret;
}

int
utf8_dec(char* src, char32_t * dst)
{
  int ret;
  char32_t w;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
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
   * decode UTF-8 to unicode
   */
  if (!ret) {
    while (src[0] != '\0') {
      if (IS_7BITS(src)) {
        w = src[0] & 0x7f;
        src += 1;

      } else if (IS_11BITS(src)) {
        w = (char32_t)((((char32_t)src[0] <<  6) & 0x00007c0)|
                       (((char32_t)src[1] <<  0) & 0x000003f));

        src += 2;

      } else if (IS_16BITS(src)) {
        w = (char32_t)((((char32_t)src[0] << 12) & 0x000f000)|
                       (((char32_t)src[1] <<  6) & 0x0000fc0)|
                       (((char32_t)src[2] <<  0) & 0x000003f));
        src += 3;

      } else if (IS_21BITS(src)) {
        w = (char32_t)((((char32_t)src[0] << 18) & 0x02c0000)|
                       (((char32_t)src[1] << 12) & 0x003f000)|
                       (((char32_t)src[2] <<  6) & 0x0000fcf)|
                       (((char32_t)src[3] <<  0) & 0x000003f));
        src += 4;

      } else {
        ret = DEFAULT_ERROR;
        break;
      }

      *dst++ = w;
    }
  }

  return ret;
}
