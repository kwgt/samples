/*
 * Object array
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "objarray.h"

#define GROW(n)           ((n * 13) / 10)
#define ALLOC(t)          ((t*)malloc(sizeof(t)))
#define NALLOC(t,n)       ((t*)malloc(sizeof(t) * (n)))
#define NREALLOC(p,t,n)   ((t*)realloc((p), sizeof(t) * (n)))

#define DEFAULT_ERROR     __LINE__
#define DEFAULT_SIZE      15

int
objary_new(objary_t** dst)
{
  return objary_new2(DEFAULT_SIZE, dst);
}

int
objary_new2(int capa, objary_t** dst)
{
  int ret;
  objary_t* obj;
  void** t;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;
  t   = NULL;

  /*
   * argument check
   */
  do {
    if (capa <= 0) {
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
  if (!ret) do {
    obj = ALLOC(objary_t);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    t = NALLOC(void*, capa);
    if (t == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * put return parameter
   */
  if (!ret) {
    obj->t    = t;
    obj->capa = capa;
    obj->size = 0;

    *dst = obj;
  }

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
    if (t != NULL) free(t);
  }

  return ret;
}

int
objary_destroy(objary_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * check argument
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * release memory
   */
  if (!ret) {
    free(ptr->t);
    free(ptr);
  }

  return ret;
}

static int
resize(objary_t* ptr, int size)
{
  int ret;
  int capa;
  void** t;

  ret  = 0;
  capa = GROW(size);

  t = NREALLOC(ptr->t, void*, capa);
  if (t != NULL) {
    ptr->t    = t;
    ptr->capa = capa;

  } else {
    ret = DEFAULT_ERROR;
  }

  return ret;
}

int
objary_aset(objary_t* ptr, int idx, void* obj)
{
  int ret;
  
  /*
   * initialize
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

    if (idx < -(ptr->size)) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * eval index
   */
  if (!ret) {
    if (idx >= ptr->capa) {
      ret = resize(ptr, idx);

    } else if (idx < 0) {
      idx = ptr->size + idx;
    }
  }

  /*
   * update context
   */
  if (!ret) {
    if (idx >= ptr->size) {
      memset(ptr->t + ptr->size, 0, sizeof(void**) * (idx - ptr->size));
      ptr->size = (idx + 1);
    }

    ptr->t[idx] = obj;
  }

  return ret;
}

int
objary_aref(objary_t* ptr, int idx, void** dst)
{
  int ret;
  
  /*
   * initialize
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

    if (idx < -(ptr->size)) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * put return parameter
   */
  if (!ret) {
    if (idx < 0) idx = ptr->size + idx;

    if (idx < ptr->size) {
      *dst = ptr->t[idx];
    } else {
      *dst = NULL;
    }
  }

  return ret;
}

int
objary_push(objary_t* ptr, void* obj)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * check argument
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * resize core
   */
  if (!ret) {
    if (ptr->size == ptr->capa) {
      ret = resize(ptr, ptr->capa);
    }
  }

  /*
   * push object
   */
  if (!ret) {
    ptr->t[ptr->size++] = obj;
  }

  return ret;
}

int
objary_pop(objary_t* ptr, void** dst)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * check argument
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * resize core
   */
  if (!ret) {
    if (ptr->size > DEFAULT_SIZE && ptr->size < (ptr->capa / 2)) {
      ret = resize(ptr, ptr->size);
    }
  }

  /*
   * pop object
   */
  if (!ret) {
    ptr->size--;
    if (dst != NULL) *dst = ptr->t[ptr->size];
  }

  return ret;
}

int
objary_remove(objary_t* ptr, int idx, void** dst)
{
  int ret;
  void** p;

  /*
   * initialize
   */
  ret = 0;

  /*
   * check argument
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (idx >= ptr->size) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (idx < -(ptr->size)) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * resize core
   */
  if (!ret) {
    if (ptr->size > DEFAULT_SIZE && ptr->size < (ptr->capa / 2)) {
      ret = resize(ptr, ptr->size);
    }
  }

  /*
   * remove data
   */
  if (!ret) {
    if (idx < 0) idx = ptr->size + idx;

    p = ptr->t + idx;
    if (dst) *dst = *p;

    memmove(p, p + 1, sizeof(void**) * (--ptr->size - idx));
  }

  return ret;
}
