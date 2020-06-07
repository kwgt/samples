#include <stdio.h>
#include <stdlib.h>

#include "aam.h"
#include "png_file.h"
#include "cronog.h"

static void
shrink_test(int n)
{
  char path1[64];
  char path2[64];

  int err;

  cronog_t* cg;
  int i;
  int64_t dura;
  int64_t sum;

  void* rgb1;
  int wd1;
  int ht1;
  int st1;

  void* rgb2;
  int wd2;
  int ht2;
  int st2;

  aam_shrinker_t* sh;

  FILE* fp;

  cg   = NULL;
  sum  = 0;
  rgb1 = NULL;
  rgb2 = NULL;
  sh   = NULL;
  fp   = NULL;

  sprintf(path1, "data/input%d.png", n);
  sprintf(path2, "data/output%d.ppm", n);

  do {
    err = cronog_new(&cg);
    if (err) {
      fprintf(stderr, "cronog_new() failed [err=%d]\n", err);
      break;
    }

    err = read_png_file(path1, &rgb1, &wd1, &ht1, &st1);
    if (err) {
      fprintf(stderr, "read_png_file() failed [err=%d]\n", err);
      break;
    }

    err = aam_shrinker_new(&sh);
    if (err) {
      fprintf(stderr, "aam_shrinker_new() failed [err=%d]\n", err);
      break;
    }

    wd2 = (wd1 * 2) / 3;
    ht2 = (ht1 * 2) / 3;
    st2 = wd2 * 3;

    err = aam_shrinker_setup(sh, wd1, ht1, st1, wd2, ht2, st2);
    if (err) {
      fprintf(stderr, "aam_shrinker_setup() failed [err=%d]\n", err);
      break;
    }

    err = aam_shrinker_alloc(sh, &rgb2);
    if (err) {
      fprintf(stderr, "aam_shrinker_alloc() failed [err=%d]\n", err);
      break;
    }

    for (i = 0; i < 10; i++) {
      cronog_reset(cg);
      cronog_start(cg);

      err = aam_shrinker_proc(sh, rgb1, rgb2);
      if (err) {
        fprintf(stderr, "aam_shrinker_proc() failed [err=%d]\n", err);
        break;
      }

      cronog_stop(cg);
      cronog_result(cg, &dura);

      sum += dura;
    }

    printf("%ldms\n", sum / 10);

    fp = fopen(path2, "wb");
    if (fp == NULL) {
      fprintf(stderr, "fopen(\"%s\") failed\n", path2);
      break;
    }

    fprintf(fp, "P6\n# %s\n%d %d\n255\n", path2, wd2, ht2);
    fwrite(rgb2, st2 * ht2, 1, fp);

#if 0
    err = write_png_file(rgb2, wd2, ht2, st2, path2);
    if (err) {
      fprintf(stderr, "write_png_file() failed [err=%d]\n", err);
      break;
    }
#endif
  } while (0);

  if (cg != NULL) cronog_destroy(cg);
  if (rgb1 != NULL) free(rgb1);
  if (rgb2 != NULL) free(rgb2);
  if (sh != NULL) aam_shrinker_destroy(sh);
  if (fp != NULL) fclose(fp);
}

int
main(int argc, char* argv[])
{
  shrink_test(1);
  shrink_test(2);
  shrink_test(3);
  shrink_test(4);

  return 0;
}
