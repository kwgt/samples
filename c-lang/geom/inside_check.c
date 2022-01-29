#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
  double x;
  double y;
} coord_t;

int
is_inside(coord_t points[], size_t n, double x, double y)
{
  int ret;
  int i;
  coord_t *p1;
  coord_t *p2;

  for (i = 0; i < n; i++) {
    p1 = points[i];
    p2 = points[(i + 1) % n];
  }
}
