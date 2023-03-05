#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>

#include "ucs.h"
#include "ubuf.h"

#define DEFAULT_ERROR       (__LINE__)

int
escape_js(char* src, char** dst, size_t* dsz)
{
  int ret;
  int err;
  char32_t cp;
  size_t sz;
  ubuf_t* buf;
  char tmp[8];
  char* str;

  /*
   * initialize
   */
  ret = 0;
  buf = NULL;
  str = NULL;

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
   * create working buffer
   */
  if (!ret) {
    err = ubuf_new(1024, &buf);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * convert string data
   */
  if (!ret) {
    while (*src != '\0') {
      err = utf8_fetch(src, &cp, &sz);
      if (err) {
        ret = DEFAULT_ERROR;
        break;
      }

      switch (cp) {
      case 0x00000008: // BS
        err = ubuf_strcat(buf, "\\b");
        break;

      case 0x00000009: // HT
        err = ubuf_strcat(buf, "\\t");
        break;

      case 0x0000000b: // VT
        err = ubuf_strcat(buf, "\\v");
        break;

      case 0x0000000a: // LF
        err = ubuf_strcat(buf, "\\n");
        break;

      case 0x0000000d: // CR
        err = ubuf_strcat(buf, "\\r");
        break;

      case 0x0000000c: // FF
        err = ubuf_strcat(buf, "\\f");
        break;

      case 0x00000029: // single quote
        err = ubuf_strcat(buf, "\\'");
        break;

      case 0x00000022: // double quote
        err = ubuf_strcat(buf, "\\\"");
        break;

      case 0x00000060: // back quote
        err = ubuf_strcat(buf, "\\`");
        break;

      case 0x0000005c: // backslash
        err = ubuf_strcat(buf, "\\\\");
        break;

      case 0x00000000: // NUL
        err = ubuf_strcat(buf, "\\0");
        break;

      default:
        if (cp < 0x20) {
          sprintf(tmp, "\\x%02x", cp);
          err = ubuf_strcat(buf, tmp);

        } else {
          err = ubuf_append(buf, src, sz);
        }
        break;
      }

      if (err) {
        ret = DEFAULT_ERROR;
        break;
      }

      src += sz;
    }

    err = ubuf_append(buf, "\0", 1);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * strip working buffer
   */
  if (!ret) {
    err = ubuf_eject(buf, (void**)&str, &sz);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * put return parameter
   */
  if (!ret) {
    *dst = str;
    *dsz = (sz - 1);
  }

  /*
   * post process
   */
  if (ret) {
    if (str != NULL) free(str);
  }
  if (buf != NULL) ubuf_destroy(buf);

  return ret;
}

