/*
 * simple queue
 *
 *  Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include "que.h"

#define DEFAULT_ERROR     __LINE__
#define ALLOC(t)          ((t*)malloc(sizeof(t)))
#define NALLOC(t, n)      ((t*)malloc(sizeof(t) * (n)))

int
que_new(size_t size, que_t** dst)
{
  int ret;
  que_t* obj;
  void** body;

  /*
   * initialize
   */
  ret  = 0;
  obj  = NULL;
  body = NULL;

  /*
   * argument check
   */
  do {
    if (size <= 0) {
      ret = DEFAULT_ERROR;
      break;
    } 

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * memory allocate
   */
  if (!ret) do {
    obj = ALLOC(que_t);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    body = NALLOC(void*, size);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * setup object
   */
  if (!ret) {
    obj->body = body;
    obj->capa = size;
    obj->size = 0;
    obj->head = 0;
    obj->tail = 0;
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
    if (body != NULL) free(body);
  }

  return ret;
}

int
que_destroy(que_t* ptr)
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
    free(ptr->body);
    free(ptr);
  }

  return ret;
}

int
que_enque(que_t* ptr, void* src)
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

    if (src == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * object state check
   */
  if (!ret) {
    if (ptr->size == ptr->capa) ret = DEFAULT_ERROR;
  }

  /*
   * append to tail
   */
  if (!ret) {
    ptr->body[ptr->tail] = src;

    ptr->tail++;
    ptr->size++;

    if (ptr->tail == ptr->capa) ptr->tail = 0;
  }

  return ret;
}

int
que_deque(que_t* ptr, void** dst)
{
  int ret;
  void* obj;

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

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * object state check
   */
  if (!ret) {
    if (ptr->size == 0) ret = DEFAULT_ERROR;
  }

  /*
   * pull from head
   */
  if (!ret) {
    obj = ptr->body[ptr->head];

    ptr->size--;
    ptr->head++;

    if (ptr->head == ptr->capa) ptr->head = 0;
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = obj;

  return ret;
}
