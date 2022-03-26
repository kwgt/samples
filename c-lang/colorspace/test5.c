#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define DATABASE  "test"
#define WIDTH     160
#define HEIGHT    160

void deintl3(uint8_t* src, int wd, int ht,
             uint8_t* dst1, uint8_t* dst2, uint8_t* dst3);

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
read_file(char* path, void** _ptr, size_t* _size)
{
  int ret;
  int err;
  struct stat st;
  void* ptr;
  FILE* fp;
 
  /*
   * initialize
   */
  ptr = NULL;
  fp  = NULL;
 
  do {
    /*
     * entry process
     */
    ret = !0;
 
    /*
     * get file size
     */
    err = stat(path, &st);
    if (err < 0) break;
 
    /*
     * alloc memory
     */
    ptr = malloc(st.st_size);
    if (ptr == NULL) break;
 
    /*
     * open file
     */
    fp = fopen(path, "rb");
    if (fp == NULL) break;
 
    /*
     * read file
     */
    err = fread(ptr, st.st_size, 1, fp);
    if (err != 1) break;
 
    /*
     * set return parameter
     */
    *_ptr  = ptr;
    if (_size) *_size = st.st_size;
 
    /*
     * mark success
     */
    ret = 0;
  } while(0);
 
  /*
   * post process
   */
  if (ret) {
    if (ptr != NULL) free(ptr);
  }
 
  if(fp != NULL) fclose(fp);
 
  return ret;
}

int
main(int argc, char* argv[])
{
  void* ppm;
  void* r;
  void* g;
  void* b;
  int i;
  tmmes_t tm;
  FILE* fp;

  read_file("data/test.ppm", &ppm, NULL);

  r = malloc(WIDTH * HEIGHT);
  g = malloc(WIDTH * HEIGHT);
  b = malloc(WIDTH * HEIGHT);

  deintl3((uint8_t*)ppm + 15,
          WIDTH,
          HEIGHT,
          (uint8_t*)r,
          (uint8_t*)g,
          (uint8_t*)b);

  fp = fopen("test5-r.ppm", "wb");
  fprintf(fp, "P5\n#test\n%d %d\n255\n", WIDTH, HEIGHT);
  fwrite(r, WIDTH * HEIGHT, 1, fp);
  fclose(fp);

  fp = fopen("test5-g.ppm", "wb");
  fprintf(fp, "P5\n#test\n%d %d\n255\n", WIDTH, HEIGHT);
  fwrite(g, WIDTH * HEIGHT, 1, fp);
  fclose(fp);

  fp = fopen("test5-b.ppm", "wb");
  fprintf(fp, "P5\n#test\n%d %d\n255\n", WIDTH, HEIGHT);
  fwrite(b, WIDTH * HEIGHT, 1, fp);
  fclose(fp);

  free(ppm);
  free(r);
  free(g);
  free(b);

  return 0;
}
