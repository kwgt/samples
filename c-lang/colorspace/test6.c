#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#define DEFAULT_ERROR   (__LINE__)

#define WIDTH           640
#define HEIGHT          360

typedef struct {
  long long tm;
  struct timespec t0;
} tmmes_t;

void
start_timer(tmmes_t* tm)
{
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tm->t0);
}

void
stop_timer(tmmes_t* tm)
{
  struct timespec t1;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);

  tm->tm = (((t1.tv_sec - tm->t0.tv_sec) * 1000000000LL) +
           (t1.tv_nsec - tm->t0.tv_nsec));
}

void
i420_to_yuv444(int wd, int ht, int y_stride, int uv_stride,
               uint8_t* src_y, uint8_t* src_u, uint8_t* src_v,
               int stride, uint8_t* dst);

int
read_i420(char* path, int wd, int ht, void** dsy, void** dsu, void** dsv)
{
  int ret;
  int err;
  FILE* fp;
  size_t psz;
  void* y;
  void* u;
  void* v;
 
  /*
   * initialize
   */
  ret = 0;
  fp  = NULL;
  y   = NULL;
  u   = NULL;
  v   = NULL;

  /*
   * argument check
   */
  do {
    if (path == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dsy == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dsu == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dsv == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * memory allocation
   */
  if (!ret) {
    psz = wd * ht;
    y   = malloc(psz + (psz /2));

    if (y == NULL) {
      ret = DEFAULT_ERROR;

    } else {
      u = y + psz;
      v = u + (psz / 4);
    }
  }

  /*
   * file open
   */
  if (!ret) {
    fp = fopen(path, "rb");
    if (fp == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * read data
   */
  if (!ret) do {
    err = fread(y, psz, 1, fp);
    if (err != 1) {
      ret = DEFAULT_ERROR;
      break;
    }

    err = fread(u, psz / 4, 1, fp);
    if (err != 1) {
      ret = DEFAULT_ERROR;
      break;
    }

    err = fread(v, psz / 4, 1, fp);
    if (err != 1) {
      ret = DEFAULT_ERROR;
      break;
    }

  } while (0);

  /*
   * put return parameter
   */
  if (!ret) {
    *dsy = y;
    *dsu = u;
    *dsv = v;
  }
 
  /*
   * post process
   */
  if (ret) {
    if (y != NULL) free(y);
  }
 
  if(fp != NULL) fclose(fp);
 
  return ret;
}

int
main(int argc, char* argv[])
{
  int err;
  void* y;
  void* u;
  void* v;
  void* yuv;
  int i;
  FILE* fp;
  tmmes_t tm;

  err = read_i420("data/color-bar.i420", WIDTH, HEIGHT, &y, &u, &v);

  yuv = malloc(WIDTH * HEIGHT * 3);
  fprintf(stderr, "come %s:%d %d\n", __FILE__, __LINE__, err);

  start_timer(&tm);
  for (i = 0; i < 500; i++) {
    i420_to_yuv444(WIDTH, HEIGHT, WIDTH, WIDTH / 2, y, u, v, WIDTH * 3, yuv);
  }
  stop_timer(&tm);
  fprintf(stderr, "%10.2f msec\n", (tm.tm / 1000000.0) / 500.0);

  fp = fopen("test6.ppm", "wb");
  fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
  fwrite(yuv, WIDTH * HEIGHT * 3, 1, fp);
  fclose(fp);

  fp = fopen("test6.yuv", "wb");
  fwrite(yuv, WIDTH * HEIGHT * 3, 1, fp);
  fclose(fp);

  free(y);
  free(yuv);

  return 0;
}
