/*
 * simple queue
 *
 *  Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __SIMPLE_QUEUE_H__
#define __SIMPLE_QUEUE_H__
#ifdef __cplusplus
extern "C" {
#endif /* !defined(__cplusplus) */

#include <stdlib.h>

typedef struct {
  void** body;
  size_t capa;
  size_t size;
  int head;
  int tail;
} que_t;

#define QUE_IS_FULL(ptr)    (ptr->size == ptr->capa)
#define QUE_IS_EMPTY(ptr)   (ptr->size == 0)

int que_new(size_t size, que_t** dst);
int que_destroy(que_t* ptr);
int que_enque(que_t* ptr, void* src);
int que_deque(que_t* ptr, void** dst);

#ifdef __cplusplus
}
#endif /* !defined(__cplusplus) */
#endif /* !defined(__SIMPLE_QUEUE_H__) */
