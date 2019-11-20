/*
 * string slicer
 *
 *  Copyright (C) 2012 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdlib.h>
#include <string.h>

#include "slicer.h"

#define DEFAULT_ERROR       __LINE__
#define DEFAULT_DELIMITER   "\r\n"

#define ALLOC(t)            ((t*)malloc(sizeof(t)))

int
slicer_new(char* src, size_t len, char* dlm, slicer_t** dst)
{
	int ret;
  slicer_t* obj;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;

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
   * memory allocation
   */
  if (!ret) {
    obj = ALLOC(slicer_t);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
    }
  }

  /*
   * put return parameter
   */
  if (!ret) {
    obj->src = src;
    obj->len = (len > 0)? len: strlen(src);
    obj->dlm = (dlm != NULL)? dlm: DEFAULT_DELIMITER;
    obj->pos = 0;

    *dst = obj;
  }

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
  }

  return ret;
}

int
slicer_destroy(slicer_t* ptr)
{
  int ret;

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
    free(ptr);
  }

  return ret;
}

int
slicer_reset(slicer_t* ptr)
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
   * do reset
   */
  if (!ret) {
    ptr->pos = 0;
  }

  return ret;
}

static int
isdelimiter(slicer_t* ptr, int i)
{
  char c;

  c = ptr->src[i];

  for (i = 0; ptr->dlm[i] != '\0'; i++) {
    if (c == ptr->dlm[i]) break;
  }

  return (c == ptr->dlm[i]);
}

int
slicer_next(slicer_t* ptr, char** dst, size_t* dsz)
{
  int ret;
  char pr;
  char* s;
  size_t l;
  int i;

  /*
   * initialize
   */
  ret = 0;
  s   = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * check object state
   */
  if (!ret) {
    if (ptr->pos >= ptr->len) {
      ret = ERR_EMPTY;
    }
  }

  /*
   * slice line
   */
  if (!ret) {
    pr = '\0';

    /* 空行をスキップ */
    for (i = ptr->pos; i < ptr->len; i++) {
      if (!isdelimiter(ptr, i)) break;
      ptr->pos++;
    }

    for (i = ptr->pos; i < ptr->len; i++) {
      if (isdelimiter(ptr, i)) break;
      pr = ptr->src[i];
    }

    l = i - ptr->pos;

    if (l == 0) {
      ret = ERR_EMPTY;
    }
  }

  if (!ret) {
    s = (char*)malloc(l + 1);

    if (s != NULL) {
      memcpy(s, ptr->src + ptr->pos, l); s[l] = '\0';
      ptr->pos = (i + 1);

    } else {
      ret = DEFAULT_ERROR;
    }
  }

  /*
   * put return parameter
   */
  if (!ret) {
    *dst = s;
    if (dsz != NULL) *dsz = l;
  }

  /*
   * post process
   */
  if (ret) {
    if (s != NULL) free(s);
  }

  return ret;
}
