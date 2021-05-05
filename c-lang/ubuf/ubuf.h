/*
 * universal buffer
 *
 *  Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __UNIVERSAL_BUFFER_H__
#define __UNIVERSAL_BUFFER_H__
#ifdef __cplusplus
extern "C" {
#endif /* !defined(__cplusplus) */

#include <stdlib.h>

typedef struct {
  void* data;
  size_t capa;    // capacity for this buffer object
  size_t size;    // used size for this buffer object
} ubuf_t;

int ubuf_new(size_t size, ubuf_t** dst);
int ubuf_new2(void* data, size_t size, ubuf_t** dst);
int ubuf_new3(char* s, ubuf_t** dst);
int ubuf_destroy(ubuf_t* ptr);

int ubuf_append(ubuf_t* ptr, void* data, size_t size);
int ubuf_strcat(ubuf_t* ptr, char* s);
int ubuf_slice(ubuf_t* ptr, size_t size, void* dst, size_t* dsz);
int ubuf_eject(ubuf_t* ptr, void** dst, size_t* dsz);
int ubuf_clear(ubuf_t* ptr);
int ubuf_resize(ubuf_t* ptr, size_t size);

#ifdef __cplusplus
}
#endif /* !defined(__cplusplus) */
#endif /* !defined(__UNIVERSAL_BUFFER_H__) */
