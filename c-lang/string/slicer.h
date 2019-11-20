/*
 * line slicer
 *
 *  Copyright (C) 2012 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __LINE_SLICER_H__
#define __LINE_SLICER_H__
#include <stdlib.h>

typedef struct {
  char* src;
  size_t len;
  int pos;
  char* dlm;
} slicer_t;

#define ERR_EMPTY     -1

int slicer_new(char* src, size_t len, char* dlm, slicer_t** dst);
int slicer_destroy(slicer_t* ptr);
int slicer_reset(slicer_t* ptr);
int slicer_next(slicer_t* ptr, char** dst, size_t* len);

#endif /* !defined(__LINE_SLICER_H__) */
