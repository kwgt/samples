/*
 * unicode utility
 *
 *  Copyright (C) 2021 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>

#include"ucs.h"

#define DEFAULT_ERROR     __LINE__
#define ALLOC(t)          ((t*)malloc(sizeof(t)))
#define NALLOC(t,n)       ((t*)malloc(sizeof(t)* (n)))

static int
min3(int a, int b, int c)
{
  return (a > b)? ((b > c)? c: b) : ((a > c)? c: a);
}

static int
calc_dist(char32_t* uc1, size_t n1, char32_t* uc2, size_t n2, int* dst)
{
  int ret;
  int w;
  int h;

  int* tbl;
  int* p;

  int i;
  int j;

  int b;
  int c;

  /*
   * initialize 
   */
  ret = 0;
  w   = n1 + 1;
  h   = n2 + 1;

  /*
   * work memory allocate
   */
  tbl = NALLOC(int, w * h);
  if (tbl == NULL) ret = DEFAULT_ERROR;

  /*
   * initialize table
   */
  if (!ret) {
    for (i = 0; i < w; i++) tbl[i] = i;
    for (i = 0, p = tbl; i < h; i++, p += w) p[0]  = i;
  }

  /*
   * do calc
   */
  if (!ret) {
    /*
     * for (i = 0; i < n1; i++) {
     *   for (j = 0; j < n2; j++) {
     *     a = rows[j+1][i+0] + 1;
     *     b = rows[j+0][i+1] + 1;
     *     c = rows[j+0][i+0] + ((uc1[i] == uc2[j])? 0: 1);
     * 
     *     rows[j+1][i+1] = min3(a, b, c);
     *   }
     * }
     *
     * *dst = rows[n2][n1];
     *
     * 上記を最適化
     */

    for (i = 0; i < n1; i++) {
      p = tbl + i;

      for (j = 0; j < n2; j++) {
        b = p[1] + 1;
        c = p[0] + ((uc1[i] == uc2[j])? 0: 1);

        p += w;
        p[1] = min3(p[0] + 1, b, c);
      }
    }
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = tbl[(n2 * w) + n1];

  /*
   * post process
   */
  if (tbl != NULL) free(tbl);

  return ret;
}

int
levenshtein_dist(char* s1, char* s2, int* dst)
{
  int ret;
  int err;
  char32_t* uc1;
  size_t n1;
  char32_t* uc2;
  size_t n2;
  int dist;

  /*
   * initialize
   */
  ret = 0;
  uc1 = NULL;
  uc2 = NULL;

  /*
   * argument check
   */
  do {
    if (s1 == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (s2 == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * convert UTF8 string to UCS array 
   */
  if (!ret) {
    err = utf8_to_ucs(s1, &uc1, &n1);
    if (err) ret = DEFAULT_ERROR;
  }

  if (!ret) {
    err = utf8_to_ucs(s2, &uc2, &n2);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * calc distance
   */
  if (!ret) {
    if (n1 == 0 || n2 == 0) {
      dist = (n1 > n2)? n1: n2;

    } else {
      err = calc_dist(uc1, n1, uc2, n2, &dist);
      if (err) ret = DEFAULT_ERROR;
    }
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = dist;

  /*
   * post process
   */
  if (uc1 != NULL) free(uc1);
  if (uc2 != NULL) free(uc2);

  return ret;
}


#if 1
int
main(int argc, char* argv[])
{
  int dist;

  levenshtein_dist(argv[1], argv[2], &dist);
  printf("(%s) (%s) -> %d\n", argv[1], argv[2], dist);
}
#endif
