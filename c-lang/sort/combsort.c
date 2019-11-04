#include <stdio.h>
#include <stdlib.h>

#define SHRINK(n)       ((n * 10) / 13)
#define SWAP(a,b)       do {double t; t = b; b = a; a = t;} while(0)


void
combsort11(double* a, size_t n)
{
  int h;
  int f;
  int i;

  /*
   * sort by ascending order
   */

  h = n;

  do {
    if (h > 1) {
      h = SHRINK(h);
    } else if (!f) {
      break;
    }

    f = 0;

    if (h == 9 || h == 10) h = 11;

    for (i = 0; i < ((int)n - h); i++) {
      if (a[i] > a[i + h]) {
        SWAP(a[i], a[i + h]);
        f = !0;
      }
    }
  } while (1);
}

