#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

double
hausdorff(double a1[], size_t sz1, double a2[], size_t sz2)
{
  double ret;
  double dx;
  double dy;
  double dis;
  double min;
  int i;
  int j;

  ret = 0.0;

  for (i = 0; i < sz1; i += 2) {
    min = DBL_MAX;

    for (j = 0; j < sz2; j += 2) {
      dx  = a1[i+0] - a2[j+0];
      dy  = a1[i+1] - a2[j+1];
      dis = sqrt((dx * dx) + (dy * dy));

      if (dis < min) min = dis;
    }

    if (min > ret) ret = min;
  }

  return ret;
}
