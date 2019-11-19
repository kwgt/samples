#include <stdio.h>

/*
 * 最上位のセットビットの位置を求める
 * （一番左の1にセットされているビットの位置を求める） 
 */
int
msb(int x)
{
  register int r;
  register int w;
  register unsigned int m;

  r = 0;
  w = sizeof(int) * 4;  // half bit width of one-word data
  m = 0 - 1;            // full mask pattern

  do {
    m >>= w;
    if ((x >> r) & (m << w)) r += w;
  } while(w /= 2);

  return r;
}

int
main(int argc, char* argv[])
{
  printf("%d\n", msb(1));
  printf("%d\n", msb(3));
  printf("%d\n", msb(5));
  printf("%d\n", msb(0x8300));
  printf("%d\n", msb(0x28300));
  printf("%d\n", msb(0x40000000));
  printf("%d\n", msb(0xf0f00000));
  return 0;
}
