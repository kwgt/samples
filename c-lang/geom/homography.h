/*
 * Homography transform library
 *
 *  Copyright (C) 2017 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

/*
 * $Original: homography.h 155 2017-09-02 23:01:42Z pi $
 * $Id: homography.h 3 2017-10-05 05:04:53Z kuwagata.hiroshi $
 */

#ifndef __HOMOGRAPHY_H__
#define __HOMOGRAPHY_H__

typedef struct {
  double a;
  double b;
  double c;
  double d;
  double e;
  double f;
  double g;
  double h;
} homography_t;

/*
 * homography_new()には変換元の座標と変換後の座標を渡して下さい
 * 引数が多くなるため、doubleの配列にパックして渡す仕様にしています。
 * 以下の様にパックして下さい。
 *
 * src[0]  変換元P0のX座標
 * src[1]  変換元P0のY座標
 * src[2]  変換元P1のX座標
 * src[3]  変換元P1のY座標
 *  :
 *  :
 * src[6]  変換元P3のX座標
 * src[7]  変換元P3のY座標
 *
 * dst[0]  変換後P0のX座標
 * dst[1]  変換後P0のY座標
 * dst[2]  変換後P1のX座標
 * dst[3]  変換後P1のY座標
 *  :
 *  :
 * dst[6]  変換後P3のX座標
 * dst[7]  変換後P3のY座標
 *
 * なお、座標は左上から時計回りにP0 -> P1 -> P2 -> P3です。
 * またsrcとdstの双方共に凸角形である必要があります。
 */

int homography_new(double src[8], double dst[8], homography_t** obj);
int homography_transform(homography_t* obj,
                          double sx, double sy, double* dx, double* dy);
int homography_destroy(homography_t* obj);

#endif /* !defined(__HOMOGRAPHY_H__) */
