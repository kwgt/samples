#include <stdio.h>
#include <math.h>

#define DEFAULT_ERROR     __LINE__

int
is_valid_date(int y, int m, int d)
{
  int ret;

  ret = 0;

  do {
    if (y <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (m <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (d <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  if (!ret) {
    switch (m) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      if (d > 31) ret = DEFAULT_ERROR;
      break;

    case 4:
    case 6:
    case 9:
    case 11:
      if (d > 30) ret = DEFAULT_ERROR;
      break;

    case 2:
      if (y % 400 == 0) {
        // 400で割り切れる年は閏年
        if (d > 29) ret = DEFAULT_ERROR;

      } else if (y % 100 == 0) {
        // 100で割り切れる年は閏年
        if (d > 28) ret = DEFAULT_ERROR;

      } else if (y % 4 == 0) {
        // 4で割り切れる年は閏年
        if (d > 29) ret = DEFAULT_ERROR;

      } else {
        // 4で割り切れない年は平年
        if (d > 28) ret = DEFAULT_ERROR;
      }
      break;

    default:
      ret = DEFAULT_ERROR;
      break;
    }
  }

  return !ret;
}

int
gc2mjd(int y, int m, int d, long* dst)
{
  int ret;
  long yy;
  long mm;
  long mjd;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (y <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (m <= 0 || m > 12) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (!is_valid_date(y, m, d)) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * calc modified julian day
   */
  if (!ret) {
    if (m == 1 || m == 2) {
      y--;
      m += 12;
    }

    yy  = (long)(floor(365.25 * y) + floor(y / 400.0) - floor(y / 100.0));
    mm  = (long)floor(30.59 * (m - 2));
    mjd = yy + mm + d - 678912;
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = mjd;

  return ret;
}

int
mjd2gc(long mjd, int* dy, int* dm, int* dd)
{
  int ret;
  long n;
  long a;
  long b;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (mjd <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dy == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dm == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dd == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * gregorian calender's date
   */
  if (!ret) {
    n = mjd + 678881;
    a = (long)floor((3.0 * floor(((4.0 * (n + 1)) / 146097) + 1.0)) / 4.0);
    a = (4 * n) + 3 + (4 * a);
    b = 5 * floor((a % 1461) / 4.0) + 2;
  }

  /*
   * put return parameter
   */
  if (!ret) {
    *dy = (int)floor(a / 1461.0);
    *dm = (int)floor(b / 153.0) + 3;
    *dd = (int)floor((b % 153) / 5.0) + 1;
  }

  return ret;
}

#if 1
int
main(int argc, char* argv[])
{
  int err;
  long mjd;
  int y;
  int m;
  int d;

  err = gc2mjd(1970, 12, 29, &mjd);
  printf("%d %ld\n", err, mjd);

  err = mjd2gc(mjd, &y, &m, &d);
  printf("%d %d %d %d\n", err, y, m ,d);

  err = gc2mjd(2021, 9, 16, &mjd);
  printf("%d %ld\n", err, mjd);

  err = mjd2gc(mjd, &y, &m, &d);
  printf("%d %d %d %d\n", err, y, m ,d);

  return 0;
}
#endif
