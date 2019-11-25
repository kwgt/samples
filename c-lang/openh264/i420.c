#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "i420.h"

#define ALLOC(t)        ((t*)malloc(sizeof(t)))
#define CLIP(v)         (((v) < 0)? 0: ((v) > 255)? 255: (v))
#define DEFAULT_ERROR   __LINE__

int
i420_new(i420_t** dst)
{
  int ret;
  i420_t* obj;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;

  /*
   * argument check
   */
  if (dst == NULL) {
    ret = DEFAULT_ERROR;
  }

  /*
   * alloc memory
   */
  if (!ret) {
    obj = ALLOC(i420_t);
    if (obj == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * put return parameter
   */
  if (!ret) {
    obj->width     = -1;
    obj->height    = -1;
    obj->stride    = -1;
    obj->size      = -1;
    obj->plane     = NULL;

    obj->y_stride  = -1;
    obj->uv_stride = -1;

    *dst = obj;
  }

  /*
   * post process
   */
  if (ret) {
    if (obj) free(obj);
  }
  
  return ret;
}

int
i420_destroy(i420_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) {
    ret = DEFAULT_ERROR;
  }

  /*
   * release memory
   */
  if (!ret) {
    if (ptr->plane) free(ptr->plane);
    free(ptr);
  }

  return ret;
}

int
i420_update(i420_t* ptr, int width, int height, int y_stride, int uv_stride)
{
  int ret;
  int size;
  void* plane;

  /*
   * initialize
   */
  ret   = 0;
  plane = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (width <= 0 || width & 1) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (height <= 0 || height & 1) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (y_stride < width ) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (uv_stride < (width / 2)) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while(0);

  /*
   * alloc memory
   */
  if (!ret) {
    size  = width * height * 3;

    if (ptr->size == size) {
      plane = ptr->plane;

    } else {
      plane = (ptr->plane == NULL)? malloc(size): realloc(ptr->plane, size);
      if (plane == NULL) ret = DEFAULT_ERROR;
    }
  }

  /*
   * update context
   */
  if (!ret) {
    ptr->width     = width;
    ptr->height    = height;
    ptr->stride    = width * 3;
    ptr->size      = size;
    ptr->plane     = plane;

    ptr->y_stride  = y_stride;
    ptr->uv_stride = uv_stride;
  }

  /*
   * post process
   */
  if (ret) {
    if (plane) free(plane);
  }

  return ret;
}

int
i420_conv(i420_t* ptr, uint8_t* src_y, uint8_t* src_u, uint8_t* src_v)
{
  int ret;
  uint8_t* dst1;    // destination pointer for even line
  uint8_t* dst2;    // destination pointer for odd line

  uint8_t* yp1;     // y-plane pointer for even line
  uint8_t* yp2;     // y-plane pointer for odd line
  uint8_t* up;      // u-plane pointer
  uint8_t* vp;      // v-plane pointer

  int i;
  int j;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src_y == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src_u == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (src_v == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * do convert
   */
  if (!ret) {
    dst1 = ptr->plane;
    dst2 = dst1 + ptr->stride;

    for (i = 0; i < ptr->height; i += 2) {
      yp1 = src_y;
      yp2 = src_y + ptr->y_stride;
      up  = src_u;
      vp  = src_v;

      for (j = 0; j < ptr->width; j += 2) {
        int y;
        int u;
        int v;

        int r0;
        int g0;
        int b0;

        u  = (int)*up - 128;
        v  = (int)*vp - 128;

        r0 = (v * 1634);
        g0 = (v * 833) + (u * 400);
        b0 = (u * 2066);

        /* [0,0] */
        y = ((int)yp1[0] - 16) * 1192;

        *dst1++ = CLIP((y + r0) >> 10);
        *dst1++ = CLIP((y - g0) >> 10);
        *dst1++ = CLIP((y + b0) >> 10);

        /* [0,1] */
        y = ((int)yp2[0] - 16) * 1192;

        *dst2++ = CLIP((y + r0) >> 10);
        *dst2++ = CLIP((y - g0) >> 10);
        *dst2++ = CLIP((y + b0) >> 10);

        /* [1,0] */
        y = ((int)yp1[1] - 16) * 1192;

        *dst1++ = CLIP((y + r0) >> 10);
        *dst1++ = CLIP((y - g0) >> 10);
        *dst1++ = CLIP((y + b0) >> 10);

        /* [1,1] */
        y = ((int)yp2[1] - 16) * 1192;

        *dst2++ = CLIP((y + r0) >> 10);
        *dst2++ = CLIP((y - g0) >> 10);
        *dst2++ = CLIP((y + b0) >> 10);

        yp1 += 2;
        yp2 += 2;
        up++;
        vp++;
      }

      dst1  += ptr->stride;
      dst2  += ptr->stride;

      src_y += (ptr->y_stride * 2);
      src_u += ptr->uv_stride;
      src_v += ptr->uv_stride;
    }
  }

  return ret;
}

