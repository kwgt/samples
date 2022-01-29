/*
 * image shrink by bi-liner method
 *
 * Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __BLM_SHRINK_H__
#define __BLM_SHRINK_H__
typedef struct {
  struct {
    int wd;
    int ht;
    int st;    // as "STride"
  } src;

  struct {
    int wd;
    int ht;
    int st;    // as "STride"
  } dst;
} blm_shrinker_t;

int blm_shrinker_new(blm_shrinker_t** dst);
int blm_shrinker_destroy(blm_shrinker_t* ptr);
int blm_shrinker_setup(blm_shrinker_t* ptr,
                      int sw, int sh, int ss, int dw, int dh, int ds);
int blm_shrinker_alloc(blm_shrinker_t* ptr, void** dst);
int blm_shrinker_proc(blm_shrinker_t* ptr, void* _src, void* _dst);

#endif /* !defined(__BLM_SHRINK_H__) */
