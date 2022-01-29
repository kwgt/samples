#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DEFAULT_ERROR   __LINE__

int
comma_string(long v, char** dst)
{
  int ret;
  int n;
  char* s;
  char* p;
  int i;

  /*
   * initialize
   */
  ret = 0;
  s   = NULL;

  /*
   * argument check
   */
  do {
    if (v < 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * memory allocation
   */
  if (!ret) {
    n = 1 + floor(log10((double)v));
    n += ((n - 1) / 3);

    s = (char*)malloc(n + 1);
    if (s == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * render string
   */
  if (!ret) {
    p = s + n;
    i = 0;

    *p-- = '\0';

    do {
      *p-- = "0123456789"[v % 10];
      v /= 10;
      if (v == 0) break;
      if (++i % 3 == 0) *p-- = ',';
    } while (1);
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = s;

  /*
   * post process
   */
  if (ret) {
    if (s != NULL) free(s);
  }

  return ret;
}

void
test(int v)
{
  int err;
  char* str;

  str = NULL;
  err = comma_string(v, &str);

  printf("(%d) %d %s\n", err, v, str);

  if (str != NULL) free(str);
}
int
main(int argc, char* argv)
{
  test(0);
  test(1);
  test(12);
  test(123);
  test(1234);
  test(12345);
  test(123456);
  test(1234567);

  return 0;
}
