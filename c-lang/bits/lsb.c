#include <stdio.h>
#include <stdint.h>

/*
 * 最上位のセットビットの位置を求める
 * （一番左の1にセットされているビットの位置を求める） 
 */
int
lsb(uint32_t x)
{
  register uint32_t r;
  register int w;
  register uint32_t m;

  r = 0;
  w = sizeof(uint32_t) * 4;  // half bit width of one-word data
  m = ~(uint32_t)0;          // full mask pattern

  do {
    m >>= w;
    //printf("%08x %08x %3d %08x\n", x, m, r, m << r);
    if (!(x & (m << r))) r += w;
  } while(w /= 2);

  return r;
}

int
main(int argc, char* argv[])
{
#if 0
  printf("%d\n", lsb(1));
  printf("%d\n", lsb(3));
  printf("%d\n", lsb(5));
#endif

  printf("%d\n", lsb(0x8300));

#if 0
  printf("%d\n", lsb(0x28300));
  printf("%d\n", lsb(0x40000000));
  printf("%d\n", lsb(0xf0f00000));
#endif

#if 1
  {
    int i;
    uint32_t m;

    m = (uint32_t)0 - 1;

    for (i = 0; i < 32; i++) {
      printf("%08x %d\n", m << i, lsb(m << i));
    }
  }
#endif

  return 0;
}
