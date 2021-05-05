/*
 * Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#include <stdio.h>

char*
comma(int v, char* s)
{
  int i;
  int f;
  int m;

  if (v == 0) {
    s[0] = '0';
    s[1] = '\0';

  } else {
    if (v < 0) {
      v *= -1;
      f  = !0;

    } else {
      f  = 0;
    }

    for (m = 1, i = 0; v >= m; m *= 10, i++);
    i += ((i - 1) / 3);

    if (!f) i--;

    s += i;
    s[1] = '\0';

    i = 3;
    for (; v != 0; v/= 10, s--) {
      if (i-- == 0) {
        *s-- = ',';
        i = 2;
      }

      *s = "0123456789"[v % 10];
    }

    if (f) {
      *s = '-';
    } else {
      s++;
    }
  }

  return s;
}

int
main(int argc, char* argv[])
{
  char s[30];

  printf("%s\n", comma(10, s));
  printf("%s\n", comma(123, s));
  printf("%s\n", comma(1234, s));
  printf("%s\n", comma(12345, s));
  printf("%s\n", comma(123456, s));
  printf("%s\n", comma(1234567, s));
  printf("%s\n", comma(1234567890, s));
  printf("%s\n", comma(-1234567890, s));
}
