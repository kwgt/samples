#include <stdio.h>
#include <string.h>

#include "slicer.h"

char* data = "abcdefg\nhijklnm\r\nopqrstu\n\rvwxyz";

int
main(int argc, char* argv[])
{
  slicer_t* sl;
  int err;
  char* s;
  size_t l;

  slicer_new(data, strlen(data), NULL, &sl);

  while (slicer_next(sl, &s, &l) != ERR_EMPTY) {
    printf("'%s' %lu\n", s, l);
    free(s);
  }

  slicer_destroy(sl);

  return 0;
}
