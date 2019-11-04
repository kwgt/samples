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
#endif

void i420_to_rgb(uint8_t* y, uint8_t* u, uint8_t* v,
                             int wd, int ht, uint8_t* rgb);

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
  void* y;
  void* u;
  void* v;
  void* rgb;
  int i;
  tmmes_t tm;
  FILE* fp;

  read_file("data/" DATABASE ".Y", &y, NULL);
  read_file("data/" DATABASE ".U", &u, NULL);
  read_file("data/" DATABASE ".V", &v, NULL);

  rgb = malloc(WIDTH * HEIGHT * 3);

  start_timer(&tm);
  for (i = 0; i < 200; i++) {
    i420_to_rgb(y, u, v, WIDTH, HEIGHT, rgb);
  }
  stop_timer(&tm);

  fprintf(stderr, "%10.2f msec\n", tm.tm / 1000000.0);

  fp = fopen("test.ppm", "wb");
  fprintf(fp, "P6\n#test\n%d %d\n255\n", WIDTH, HEIGHT);
  fwrite(rgb, WIDTH * HEIGHT * 3, 1, fp);
  fclose(fp);

  free(rgb);
  free(y);
  free(u);
  free(v);

  return 0;
}
