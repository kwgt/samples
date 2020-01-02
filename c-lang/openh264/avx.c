#include "avx.h"

#ifdef __APPLE_CC__
void*
aligned_malloc(size_t align, size_t size)
{
  int err;
  void* ret;

  err = posix_memalign(&ret, align, size);
  if (err) {
    ret = NULL;
  }

  return ret;
}

void
aligned_free(void* ptr)
{
  free(ptr);
}
#endif /* defined(__APPLE_CC__) */
