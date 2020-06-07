/*
 * image shrink by area avaraged method
 *
 * Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __AAM_SHRINK_H__
#define __AAM_SHRINK_H__
typedef struct {
  struct {
    int wd;
    int ht;
    int st;    // as "STride"
    int wu;    // 横1ピクセルに対する中間プレーンのピクセル数
    int hu;    // 縦1ピクセルに対する中間プレーンのピクセル数
  } src;

  struct {
    int wd;
    int ht;
    int st;    // as "STride"

    int wu;    // 横1ピクセルに対する中間プレーンのピクセル数
    int wn; 
    int ws;

    int hu;    // 縦1ピクセルに対する中間プレーンのピクセル数
    int hn; 
    int hs;
  } dst;
                                 
  int iw;    // as "Intermediate-plane Width"
  int ih;    // as "Intermediate-plane Height"

  void* tpl; // as "Temporary PLane"
} aam_shrinker_t;

int aam_shrinker_new(aam_shrinker_t** dst);
int aam_shrinker_destroy(aam_shrinker_t* ptr);
int aam_shrinker_setup(aam_shrinker_t* ptr,
                       int sw, int sh, int ss, int dw, int dh, int ds);
int aam_shrinker_alloc(aam_shrinker_t* ptr, void** dst);
int aam_shrinker_proc(aam_shrinker_t* ptr, void* _src, void* _dst);

#endif /* !defined(__AAM_SHRINK_H__) */
