/*
 * unicode utility
 *
 *  Copyright (C) 2021 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>

#include"ucs.h"

#define DEFAULT_ERROR   __LINE__
#define ALLOC(t)        ((t*)malloc(sizeof(t)))
#define NALLOC(t,n)     ((t*)malloc(sizeof(t)*(n)))

size_t
utf8_len(char* src, size_t* dst)
{
  int ret;
  size_t len;
  size_t sz;
  int c;

  /*
   * initialize
   */
  ret = 0;
  len = 0;

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
   * count UCS length
   */
  if (!ret) {
    while ((c = *src)) {
      if ((c & 0xf8) == 0xf0) {         // 1111 0xxx
        /*
         * when 21bit (use 4 octets)
         */
        sz = 4;

      } else if ((c & 0xf0) == 0xe0) {  // 1110 xxxx
        /*
         * when 16bit (use 3 octets)
         */
        sz = 3;

      } else if ((c & 0xd0) == 0xc0) {  // 110x xxxx
        /*
         * when 11bit (use 2 octets)
         */
        sz = 2;

      } else if ((c & 0x80) == 0x00){   // 0xxx xxxx
        /*
         * when 7bit (use 1 octet)
         */
        sz = 1;

      } else {
        /*
         * ここまで来たらどのパターンにも当てはまらないのでエラー
         */
        ret = DEFAULT_ERROR;
        break;
      }

      len++;
      src += sz;
    }
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = len;

  return ret;
}

static inline int
utf8_decode(char* src, char32_t* dst, size_t* dsz)
{
  int ret;
  char32_t cp;
  size_t sz;
  int c0;
  int c1;
  int c2;
  int c3;

  /*
   * initialize
   */
  ret = 0;

  /*
   * decode one charactor
   */

  do {
    if (!(c0 = src[0])) {
      ret = DEFAULT_ERROR;
      break;
    }

    if ((c0 & 0xf8) == 0xf0) {         // 1111 0xxx
      /*
       * when 21bit (use 4 octets)
       */

      if (!((c1 = src[1]) && (c2 = src[2]) && (c3 = src[3]))) {
        ret = DEFAULT_ERROR;
        break;
      }

      cp = ((((char32_t)c0) << 18) & 0x001c0000)|
           ((((char32_t)c1) << 12) & 0x0003f000)|
           ((((char32_t)c2) <<  6) & 0x00000fc0)|
           ((((char32_t)c3) <<  0) & 0x0000003f);
      sz = 4;

    } else if ((c0 & 0xf0) == 0xe0) {  // 1110 xxxx
      /*
       * when 16bit (use 3 octets)
       */

      if (!((c1 = src[1]) && (c2 = src[2]))) {
        ret = DEFAULT_ERROR;
        break;
      }

      cp = ((((char32_t)c0) << 12) & 0x0000f000)|
           ((((char32_t)c1) <<  6) & 0x00000fc0)|
           ((((char32_t)c2) <<  0) & 0x0000003f);
      sz = 3;

    } else if ((c0 & 0xd0) == 0xc0) {  // 110x xxxx
      /*
       * when 11bit (use 2 octets)
       */

      if (!(c1 = src[1])) {
        ret = DEFAULT_ERROR;
        break;
      }

      cp = ((((char32_t)c0) <<  6) & 0x000007c0)|
           ((((char32_t)c1) <<  0) & 0x0000003f);
      sz = 2;

    } else if ((c0 & 0x80) == 0x00){  // 0xxx xxxx
      /*
       * when 7bit (use 1 octet)
       */
      cp = (char32_t)c0;
      sz = 1;

    } else {
      /*
       * ここまで来たらどのパターンにも当てはまらないのでエラー
       */
      ret = DEFAULT_ERROR;
    }
  } while (0);

  /*
   * put return parameter
   */
  if (!ret) {
    *dst = cp;
    *dsz = sz;
  }

  return ret;
}

int
utf8_extract(char* src, char32_t* dst)
{
  int ret;
  char32_t cp;
  size_t sz;
  int c;

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
   * convert to codepoint list
   */
  if (!ret) {
    while ((c = *src)) {
      ret = utf8_decode(src, &cp, &sz);

      *dst++ = cp;
      src += sz;
    }
  }

  return ret;
}

int
utf8_to_ucs(char* src, char32_t** dst, size_t* dsz)
{
  int ret;
  int err;
  char32_t* ucs;
  size_t n;

  /*
   * initialize
   */
  ret = 0;
  ucs = NULL;

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
   * count UCS length
   */
  if (!ret) {
    err = utf8_len(src, &n);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * memory allocate
   */
  if (!ret) {
    ucs = NALLOC(char32_t, n);
    if (ucs == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * extact to UCS array
   */
  if (!ret) {
    err = utf8_extract(src, ucs);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * put return parameter
   */
  if (!ret) {
    *dst = ucs;
    if (dsz != NULL) *dsz = n;
  }

  /*
   * post process
   */
  if (ret) {
    if (ucs != NULL) free(ucs);
  }

  return ret;
}

int
utf8_fetch(char* src, char32_t* dst, size_t* dsz)
{
  int ret;
  char32_t cp;
  size_t sz;

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

    if (dsz == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * decode UTF-8
   */
  if (!ret) ret = utf8_decode(src, &cp, &sz);

  /*
   * put return parameter
   */
  if (!ret) {
    *dst = cp;
    *dsz = sz;
  }

  return ret;
}

