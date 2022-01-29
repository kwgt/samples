#include <stddef.h>
#include <stdint.h>
#include "fnv1.h"

#define FNV132_INIT       0x811c9dc5L
#define FNV164_INIT       0xcbf29ce484222325LL

// 以下のマジックナンバーを使えば5個のシフト結果の加算を
// 1個の乗算に置き換えられる
#define FNV132_PRIME      0x01000193;

static uint64_t
fnv(uint8_t* src, size_t size, uint64_t seed)
{
  int i;

  for (i = 0; i < (int)size; i++) {
    seed += (seed <<  1) +
            (seed <<  4) +
            (seed <<  7) +
            (seed <<  8) +
            (seed << 24);
    // seed *= FNV132_PRIME;
    seed ^= src[i];
  }

  return seed;
}

uint32_t
fnv132(void* src, size_t size)
{
  return (uint32_t)(fnv(src, size, FNV132_INIT) & 0x00000000ffffffff);
}

uint64_t
fnv164(void* src, size_t size)
{
  return fnv(src, size, FNV164_INIT);
}
