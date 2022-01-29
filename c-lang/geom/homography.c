/*
 * Homography transform library
 *
 *  Copyright (C) 2017 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

/*
 * $Original: homography.c 155 2017-09-02 23:01:42Z pi $
 * $Id: homography.c 3 2017-10-05 05:04:53Z kuwagata.hiroshi $
 */

/*
 * 以下を参考に作成
 * http://yaju3d.hatenablog.jp/entry/2013/09/02/013609
 */

#include <stdio.h>
#include <stdlib.h>

#include "homography.h"

#define ERR_DEFAULT       __LINE__

typedef double mat44_t[4][4];

/*
 * 4x4の行列を逆行列に変換する 
 */
static void
mat44_invert(mat44_t mat)
{
  double a;
  double b;
  double c;
  double d;

  double e;
  double f;
  double g;
  double h;

  double i;
  double j;
  double k;
  double l;

  double m;
  double n;
  double o;
  double p;

  double q;
  double r;
  double s;
  double t;
  double u;
  double v;
  double w;
  double x;
  double y;
  double z;

  double aa;
  double bb;

  double det;

  a  = mat[0][0];
  b  = mat[0][1];
  c  = mat[0][2];
  d  = mat[0][3];

  e  = mat[1][0];
  f  = mat[1][1];
  g  = mat[1][2];
  h  = mat[1][3];

  i  = mat[2][0];
  j  = mat[2][1];
  k  = mat[2][2];
  l  = mat[2][3];

  m  = mat[3][0];
  n  = mat[3][1];
  o  = mat[3][2];
  p  = mat[3][3];

  /* 0割を避けるための処理です(精度は若干落ちますが...) */
  if (a == 0.0) a = 1.0e-5;
  if (b == 0.0) b = 1.0e-5;
  if (c == 0.0) c = 1.0e-5;
  if (d == 0.0) d = 1.0e-5;
  if (e == 0.0) e = 1.0e-5;
  if (f == 0.0) f = 1.0e-5;
  if (g == 0.0) g = 1.0e-5;
  if (h == 0.0) h = 1.0e-5;
  if (i == 0.0) i = 1.0e-5;
  if (j == 0.0) j = 1.0e-5;
  if (k == 0.0) k = 1.0e-5;
  if (l == 0.0) l = 1.0e-5;
  if (m == 0.0) m = 1.0e-5;
  if (n == 0.0) n = 1.0e-5;
  if (o == 0.0) o = 1.0e-5;
  if (p == 0.0) p = 1.0e-5;

  q  = (a * f) - (b * e);
  r  = (a * g) - (c * e);
  s  = (a * h) - (d * e);
  t  = (b * g) - (c * f);
  u  = (b * h) - (d * f);
  v  = (c * h) - (d * g);
  w  = (i * n) - (j * m);
  x  = (i * o) - (k * m);
  y  = (i * p) - (l * m);
  z  = (j * o) - (k * n);

  aa = (j * p) - (l * n);
  bb = (k * p) - (l * o);

  det = 1.0 / ((q * bb) - (r * aa) + (s * z) + (t * y) - (u * x) + (v * w));

  mat[0][0] = (( f * bb) - (g * aa) + (h * z)) * det;
  mat[0][1] = ((-b * bb) + (c * aa) - (d * z)) * det;
  mat[0][2] = (( n *  v) - (o *  u) + (p * t)) * det;
  mat[0][3] = ((-j *  v) + (k *  u) - (l * t)) * det;

  mat[1][0] = ((-e * bb) + (g *  y) - (h * x)) * det;
  mat[1][1] = (( a * bb) - (c *  y) + (d * x)) * det;
  mat[1][2] = ((-m *  v) + (o *  s) - (p * r)) * det;
  mat[1][3] = (( i *  v) - (k *  s) + (l * r)) * det;

  mat[2][0] = (( e * aa) - (f *  y) + (h * w)) * det;
  mat[2][1] = ((-a * aa) + (b *  y) - (d * w)) * det;
  mat[2][2] = (( m *  u) - (n *  s) + (p * q)) * det;
  mat[2][3] = ((-i *  u) + (j *  s) - (l * q)) * det;

  mat[3][0] = ((-e *  z) + (f *  x) - (g * w)) * det;
  mat[3][1] = (( a *  z) - (b *  x) + (c * w)) * det;
  mat[3][2] = ((-m *  t) + (n *  r) - (o * q)) * det;
  mat[3][3] = (( i *  t) - (j *  r) + (k * q)) * det;
}

/*
 * 8次元連立一次方程式を解いて射影変換のためのパラメータを計算する関数
 */
static void
calc_params(double src[8], double dst[8], double param[8])
{
  mat44_t m;

  double kx1;
  double kc1;
  double kx2;
  double kc2;
  double kx3;
  double kc3;
  double kx4;
  double kc4;

  double ky1;
  double kf1;
  double ky2;
  double kf2;
  double ky3;
  double kf3;
  double ky4;
  double kf4;

  double det;
  double c;
  double f;

  /*
   * X座標側の計算
   */

  m[0][0] =  src[0];
  m[0][1] =  src[1];
  m[0][2] = -src[0] * dst[0];
  m[0][3] = -src[1] * dst[0];

  m[1][0] =  src[2];
  m[1][1] =  src[3];
  m[1][2] = -src[2] * dst[2];
  m[1][3] = -src[3] * dst[2];

  m[2][0] =  src[4];
  m[2][1] =  src[5];
  m[2][2] = -src[4] * dst[4];
  m[2][3] = -src[5] * dst[4];

  m[3][0] =  src[6];
  m[3][1] =  src[7];
  m[3][2] = -src[6] * dst[6];
  m[3][3] = -src[7] * dst[6];

  mat44_invert(m);

  kx1 = (m[0][0] * dst[0]) + (m[0][1] * dst[2]) +
        (m[0][2] * dst[4]) + (m[0][3] * dst[6]);

  kx2 = (m[1][0] * dst[0]) + (m[1][1] * dst[2]) +
        (m[1][2] * dst[4]) + (m[1][3] * dst[6]);

  kx3 = (m[2][0] * dst[0]) + (m[2][1] * dst[2]) +
        (m[2][2] * dst[4]) + (m[2][3] * dst[6]);

  kx4 = (m[3][0] * dst[0]) + (m[3][1] * dst[2]) +
        (m[3][2] * dst[4]) + (m[3][3] * dst[6]);

  kc1 = m[0][0] + m[0][1] + m[0][2] + m[0][3];
  kc2 = m[1][0] + m[1][1] + m[1][2] + m[1][3];
  kc3 = m[2][0] + m[2][1] + m[2][2] + m[2][3];
  kc4 = m[3][0] + m[3][1] + m[3][2] + m[3][3];

  /*
   * Y座標側の計算
   */

  m[0][0] =  src[0];
  m[0][1] =  src[1];
  m[0][2] = -src[0] * dst[1];
  m[0][3] = -src[1] * dst[1];


  m[1][0] =  src[2];
  m[1][1] =  src[3];
  m[1][2] = -src[2] * dst[3];
  m[1][3] = -src[3] * dst[3];

  m[2][0] =  src[4];
  m[2][1] =  src[5];
  m[2][2] = -src[4] * dst[5];
  m[2][3] = -src[5] * dst[5];

  m[3][0] =  src[6];
  m[3][1] =  src[7];
  m[3][2] = -src[6] * dst[7];
  m[3][3] = -src[7] * dst[7];

  mat44_invert(m);

  ky1 = (m[0][0] * dst[1]) + (m[0][1] * dst[3]) +
        (m[0][2] * dst[5]) + (m[0][3] * dst[7]);

  ky2 = (m[1][0] * dst[1]) + (m[1][1] * dst[3]) +
        (m[1][2] * dst[5]) + (m[1][3] * dst[7]);

  ky3 = (m[2][0] * dst[1]) + (m[2][1] * dst[3]) +
        (m[2][2] * dst[5]) + (m[2][3] * dst[7]);

  ky4 = (m[3][0] * dst[1]) + (m[3][1] * dst[3]) +
        (m[3][2] * dst[5]) + (m[3][3] * dst[7]);

  kf1 = m[0][0] + m[0][1] + m[0][2] + m[0][3];
  kf2 = m[1][0] + m[1][1] + m[1][2] + m[1][3];
  kf3 = m[2][0] + m[2][1] + m[2][2] + m[2][3];
  kf4 = m[3][0] + m[3][1] + m[3][2] + m[3][3];

  /*
   * 出力値の計算
   */
  det = (kc3 * -kf4) - (-kf3 * kc4);
  if (det == 0.0) det = 0.0001;

  det = 1.0 / det;

  c = (-kf4 * det) * (kx3 - ky3) + (kf3 * det) * (kx4 - ky4);
  f = (-kc4 * det) * (kx3 - ky3) + (kc3 * det) * (kx4 - ky4);

  param[2] = c;
  param[5] = f;
  param[6] = kx3 - (c * kc3);
  param[7] = kx4 - (c * kc4);
  param[0] = kx1 - (c * kc1);
  param[1] = kx2 - (c * kc2);
  param[3] = ky1 - (f * kf1);
  param[4] = ky2 - (f * kf2);
}

int
homography_new(double src[8], double dst[8], homography_t** _obj)
{
  int ret;
  homography_t* obj;
  double tmp[8];

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;

  do {
    /*
     * check argument
     */
    if (src == NULL) {
      ret = ERR_DEFAULT;
      break;
    }

    if (dst == NULL) {
      ret = ERR_DEFAULT;
      break;
    }

    if (_obj == NULL) {
      ret = ERR_DEFAULT;
      break;
    }

    /*
     * memory allocate
     */
    obj = (homography_t*)malloc(sizeof(homography_t));
    if (obj == NULL) {
      ret = ERR_DEFAULT;
      break;
    }

    /*
     * set return parameter
     */
    calc_params(src, dst, tmp);

    obj->a = tmp[0];
    obj->b = tmp[1];
    obj->c = tmp[2];
    obj->d = tmp[3];
    obj->e = tmp[4];
    obj->f = tmp[5];
    obj->g = tmp[6];
    obj->h = tmp[7];

    *_obj = obj;

  } while(0);

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
  }

  return ret;
}

int
homography_transform(homography_t* ptr,
      double sx, double sy, double* dx, double* dy)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * main process
   */
  do {
    register double tmp;

    /*
     * argument check
     */
    if (ptr == NULL) {
      ret = ERR_DEFAULT;
      break;
    }

    if (dx == NULL) {
      ret = ERR_DEFAULT;
      break;
    }

    if (dy == NULL) {
      ret = ERR_DEFAULT;
      break;
    }

    /*
     * calculate
     */
    tmp = ((ptr->g * sx) + (ptr->h * sy) + 1.0);
    *dx = ((ptr->a * sx) + (ptr->b * sy) + ptr->c) / tmp;
    *dy = ((ptr->d * sx) + (ptr->e * sy) + ptr->f) / tmp;
  } while(0);

  return ret;
}

int
homography_destroy(homography_t* ptr)
{
  int ret;
  
  /*
   * initialize
   */
  ret = 0;

  /*
   * main process
   */
  do {
    /*
     * argument check
     */
    if (ptr == NULL) {
      ret = ERR_DEFAULT;
      break;
    }

    /*
     * release resource
     */
    free(ptr);

  } while(0);

  return ret;
}
