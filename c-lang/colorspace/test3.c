#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define DATABASE  "test"
#define WIDTH     1920
#define HEIGHT    1080

void bgr_to_argb(uint8_t* src, int wd, int ht, uint8_t* dst);

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


int
main(int argc, char* argv[])
{
  void* bgr;
  void* rgb;
  int i;
  int j;
  tmmes_t tm;
  FILE* fp;
  uint8_t* p;


  bgr = malloc(WIDTH * HEIGHT * 3);
  rgb = malloc(WIDTH * HEIGHT * 4);

  p   = (uint8_t*)bgr;

  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      p[0] = 0xff;
      p[1] = 0x00;
      p[2] = 0x00;

      p += 3;
    }
  }

  fp = fopen("src.ppm", "wb");
  fprintf(fp, "P6\n#test\n%d %d\n255\n", WIDTH, HEIGHT);
  fwrite(bgr, WIDTH * HEIGHT * 3, 1, fp);
  fclose(fp);

  start_timer(&tm);
  for (i = 0; i < 200; i++) {
    bgr_to_argb(bgr, WIDTH, HEIGHT, rgb);
  }
  stop_timer(&tm);

  fprintf(stderr, "%10.2f msec\n", tm.tm / 1000000.0);

  fp = fopen("dst.ppm", "wb");
  fprintf(fp, "P6\n#test\n%d %d\n255\n", WIDTH, HEIGHT);
  fwrite(rgb, WIDTH * HEIGHT * 3, 1, fp);
  fclose(fp);

  free(bgr);
  free(rgb);

  return 0;
}
