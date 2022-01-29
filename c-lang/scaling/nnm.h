/*
 * image shrink by nearest neighbor method
 *
 * Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __NNM_SHRINK_H__
#define __NNM_SHRINK_H__
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
} nnm_shrinker_t;

int nnm_shrinker_new(nnm_shrinker_t** dst);
int nnm_shrinker_destroy(nnm_shrinker_t* ptr);
int nnm_shrinker_setup(nnm_shrinker_t* ptr,
                      int sw, int sh, int ss, int dw, int dh, int ds);
int nnm_shrinker_alloc(nnm_shrinker_t* ptr, void** dst);
int nnm_shrinker_proc(nnm_shrinker_t* ptr, void* _src, void* _dst);

#endif /* !defined(__NNM_SHRINK_H__) */
