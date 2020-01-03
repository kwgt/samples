#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "h264dec.h"
#include "i420.h"
#include "util.h"

#define ALLOC(t)        ((t*)malloc(sizeof(t)))
#define N(x)            (sizeof(x) / sizeof(*x))

int
main(int argc, char* argv[])
{
  int err;
  h264dec_t* dec;
  i420_t* csc;      // as Color Space Converter

  int i;
  char path[64];

  void* data;
  size_t size;
  int state;

  h264dec_new(&dec);
  i420_new(&csc);

  for (i = 1; i < 300; i++) {
    sprintf(path, "data/%04d.nal", i);

    read_file(path, &data, &size);
    state = 0;

    err = h264dec_decode(dec, data, size, &state);

    if (!err) {
      printf("%d %d %zu\n", i, state, size);

      if (state == 1) {
        i420_update(csc, dec->width, dec->height,
                    dec->y_stride, dec->uv_stride);

#ifdef BENCHMARK
        {
          int j;

          for (j = 0; j < 10000; j++) {
            i420_conv(csc, dec->y, dec->u, dec->v);
          }
        }

        exit(0);

#else /* defined(BENCHMARK) */
        i420_conv(csc, dec->y, dec->u, dec->v);
        sprintf(path, "ppm/%04d.ppm", i);
        write_ppm(path, csc->width, csc->height, csc->plane, "test");
#endif /* defined(BENCHMARK) */
      }

    } else {
      fprintf(stderr, "error occured on frame %d\n", i);
    }

    free(data);
  }

  h264dec_destroy(dec);

  return 0;
}
