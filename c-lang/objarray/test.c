#include <stdio.h>
#include <string.h>

#include "objarray.h"

int
main(int argc, char* argv[])
{
  int err;
  objary_t* a;
  char* s;
  int i;

#if 0
  printf("***\n");
  objary_new(&a);
  objary_aset(a, 0, "abc");
  objary_aset(a, 1, "def");

  objary_each(a, i, s) printf("%d %s\n", i, s);
  printf("\n");

  printf("***\n");
  objary_push(a, "efg");
  objary_push(a, "hijk");

  objary_each(a, i, s) printf("%d %s\n", i, s);
  printf("\n");

  objary_pop(a, (void**)&s);
  printf("pop %s\n", s);
  objary_each(a, i, s) printf("%d %s\n", i, s);

  objary_pop(a, NULL);
  printf("pop\n");
  objary_each(a, i, s) printf("%d %s\n", i, s);
  printf("\n");

  objary_aset(a, 40, "mark");
  objary_aset(a, 25, "mark");
  for (i = 0; i < 40; i++) {
    err = objary_pop(a, (void**)&s);
    printf("pop %d %s %d %d\n", err, s, a->size, a->capa);
  }

  objary_destroy(a);

  printf("***\n");
  objary_new(&a);
  objary_push(a, "abc");
  objary_push(a, "def");
  objary_push(a, "ghi");
  objary_each(a, i, s) printf("%d %s\n", i, s);

  printf("-\n");
  objary_remove(a, 1, (void**)&s);
  printf("- %s\n", s);
  objary_each(a, i, s) printf("%d %s\n", i, s);
  objary_destroy(a);
#endif

  printf("***\n");
  objary_new(&a);
  objary_push(a, "abc");
  objary_push(a, "def");
  objary_push(a, "ghi");
  objary_each(a, i, s) printf("%s ", s);
  printf("\n");

  printf("-\n");
  objary_shift(a, NULL);
  objary_each(a, i, s) printf("%s ", s);
  printf("\n");
  objary_shift(a, NULL);
  objary_each(a, i, s) printf("%s ", s);
  printf("\n");
  objary_shift(a, NULL);
  objary_each(a, i, s) printf("%s ", s);
  printf("\n");
  objary_destroy(a);

  return 0;
}
