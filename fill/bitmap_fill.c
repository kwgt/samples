/*
 * bitmap fill on closed area
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ERROR       __LINE__

#define UNIT_SIZE       8
#define BASE_MASK       (1 << (UNIT_SIZE - 1))
#define INDEX(n)        ((n) / UNIT_SIZE)
#define MASK(n)         (BASE_MASK >> ((n) % UNIT_SIZE))
#define BTEST(p,n)      (((p)[INDEX(n)]) &  MASK(n))
#define BSET(p,n)       (((p)[INDEX(n)]) |= MASK(n))
                     
#define N(x)            (sizeof(x) / sizeof(*x))
#define ALLOC(t)        ((t*)malloc(sizeof(t)))
#define NALLOC(t, n)    ((t*)malloc(sizeof(t) * (n)))
#define NREALLOC(p, n)  (realloc(p, sizeof(*p) * (n)))
#define GROW(n)         ((n * 13) / 10)

typedef struct {
  int x;
  int y;
} coord_t;

typedef struct {
  coord_t* pt;
  size_t n;
} polygon_t;

typedef struct {
  int l;    /* X座標の未塗り領域左端 */
  int r;    /* X座標の未塗り領域右端 */
  int y;    /* ラインのY座標         */
  int py;   /* 親ラインのY座標       */
} seed_t;

typedef struct {
  seed_t* buf;
  size_t cap;
  int pos;
} seed_stack_t;

#define IS_STACK_EMPTY(p)     ((p)->pos == 0)

static int
seed_stack_new(size_t n, seed_stack_t** dst)
{
  int ret;
  seed_stack_t* obj;
  seed_t* buf;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;
  buf = NULL;

  /*
   * argument check
   */
  if (dst == NULL) {
    ret = ERROR;
  }

  /*
   * alloc memory
   */
  if (!ret) do {
    obj = ALLOC(seed_stack_t);
    if (obj == NULL) {
      ret = ERROR;
      break;
    }

    buf = NALLOC(seed_t, n);
    if (buf == NULL) {
      ret = ERROR;
      break;
    }

    memset(buf, 0, sizeof(seed_t) * n);
  } while (0);

  /*
   * set return parameter
   */
  if (!ret) {
    obj->buf = buf;
    obj->cap = n;
    obj->pos = 0;

    *dst = obj;
  }

  /*
   * post process
   */
  if (ret) {
    if (obj) free(obj);
    if (buf) free(buf);
  }

  return ret;
}

static int
seed_stack_destroy(seed_stack_t* ptr)
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
    ret = ERROR;
  }

  /*
   * release memory
   */
  if (!ret) {
    free(ptr->buf);
    free(ptr);
  }

  return ret;
}

static int
seed_stack_push(seed_stack_t* ptr, seed_t* s)
{
  int ret;
  size_t cap;
  seed_t* buf;

  /*
   * initialize
   */
  ret = 0;
  cap = 0;
  buf = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = ERROR;
      break;
    }

    if (s == NULL) {
      ret = ERROR;
      break;
    }
  } while (0);

  /*
   * expand buffer when it will full
   */
  if (!ret) do {
    if ((ptr->cap - ptr->pos) == 1) {
      cap = GROW(ptr->cap);
      buf = NREALLOC(ptr->buf, cap);

      if (buf == NULL) {
        ret = ERROR;
        break;
      }

      ptr->buf = buf;
      ptr->cap = cap;
    }
  } while (0);

  /*
   * push data
   */
  if (!ret) {
    ptr->buf[ptr->pos++] = *s;
  }

  /*
   * post process
   */
  if (!ret) {
    if (cap > 0 && buf != NULL) free(buf);
  }

  return ret;
}

static int
seed_stack_pop(seed_stack_t* ptr, seed_t* s)
{
  int ret;
  coord_t* c;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = ERROR;
      break;
    }

    if (s == NULL) {
      ret = ERROR;
      break;
    }
  } while (0);

  /*
   * pop data
   */
  if (!ret) {
    *s = ptr->buf[--ptr->pos];
  }

  return ret;
}

typedef struct {
  uint8_t* buf;
  int w;      // as width
  int s;      // as stride
  int h;      // as height

  seed_stack_t* stk;
} bmap_t;

static int
bmap_new(int w, int h, bmap_t** dst)
{
  int ret;
  int err;
  bmap_t* obj;
  uint8_t* buf;
  seed_stack_t* stk;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;
  buf = NULL;
  stk = NULL;

  /*
   * argument check
   */
  do {
    if (w < 64 || (w % UNIT_SIZE) != 0) {
      ret = ERROR;
      break;
    }

    if (h < 64) {
      ret = ERROR;
      break;
    }

    if (dst == NULL) {
      ret = ERROR;
      break;
    }
  } while (0);

  /*
   * alloc memory
   */
  if (!ret) do {
    obj = ALLOC(bmap_t);
    if (obj == NULL) {
      ret = ERROR;
      break;
    }

    buf = (uint8_t*)malloc((w / UNIT_SIZE) * h);
    if (buf == NULL) {
      ret = ERROR;
      break;
    }

    err = seed_stack_new(100, &stk);
    if (err) {
      ret = ERROR;
      break;
    }

    memset(buf, 0, (w / UNIT_SIZE) * h);
  } while (0);

  /*
   * put return parameter
   */
  if (!ret) {
    obj->buf = buf;
    obj->w   = w;
    obj->s   = w / 8;
    obj->h   = h;
    obj->stk = stk;

    *dst = obj;
  }

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
    if (buf != NULL) free(buf);
    if (stk != NULL) seed_stack_destroy(stk);
  }

  return ret;
}

static int
bmap_strip(bmap_t* ptr, void** dst)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = ERROR;
      break;
    }

    if (dst == NULL) {
      ret = ERROR;
      break;
    }
  } while (0);

  /*
   * set return parameter
   */
  if (!ret) {
    *dst = (void*)ptr->buf;
  }

  /*
   * release memory
   */
  if (!ret) {
    seed_stack_destroy(ptr->stk);
    free(ptr);
  }

  return ret;
}

//bmap_test(bmap_t* ptr, coord_t* p)
#define BMAP_TEST(ptr, x, y) \
      BTEST((((ptr)->buf) + ((ptr)->s * (y))), (x))

//bmap_set(bmap_t* ptr, coord_t* p)
#define BMAP_SET(ptr, x, y) \
      BSET((((ptr)->buf) + ((ptr)->s * (y))), (x))

static int
bmap_draw_line(bmap_t* ptr, coord_t* c1, coord_t* c2)
{
  int ret;
  int i;
  int dx;
  int dy;
  int sx;
  int sy;
  int e;
  int n;

  int x1;
  int y1;
  int x2;
  int y2;

  /*
   * initialize/
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = ERROR;
      break;
    }

    if (c1 == NULL) {
      ret = ERROR;
      break;
    }

    if (c1->x < 0 || c1->x >= ptr->w) {
      ret = ERROR;
      break;
    }

    if (c1->y < 0 || c1->y >= ptr->h) {
      ret = ERROR;
      break;
    }

    if (c2 == NULL) {
      ret = ERROR;
      break;
    }

    if (c2->x < 0 || c2->x >= ptr->w) {
      ret = ERROR;
      break;
    }

    if (c2->y < 0 || c2->y >= ptr->h) {
      ret = ERROR;
      break;
    }
  } while (0);

  /*
   * prepare
   */
  if (!ret) {
    x1 = c1->x;
    y1 = c1->y;
    x2 = c2->x;
    y2 = c2->y;

    if (x2 > x1) {
      dx = (x2 - x1);
      sx = +1;
    } else {
      dx = (x1 - x2);
      sx = -1;
    }

    if (y2 > y1) {
      dy = (y2 - y1);
      sy = +1;
    } else {
      dy = (y1 - y2);
      sy = -1;
    }
  }

  /*
   * draw line
   */
  if (!ret) {
    if (dx > dy) {
      e = -dx;
      n = (dx + 1) / 2;

      for (i = 0; i < n; i++) {
        BMAP_SET(ptr, x1, y1);
        BMAP_SET(ptr, x2, y2);

        x1 += sx;
        x2 -= sx;

        e += (2 * dy);

        if (e >= 0) {
          y1 += sy;
          y2 -= sy;

          e -= (2 * dx);
        }
      }

      BMAP_SET(ptr, x1, y1);

    } else{
      e = -dy;
      n = (dy + 1) / 2;

      for (i = 0; i < n; i++) {
        BMAP_SET(ptr, x1, y1);
        BMAP_SET(ptr, x2, y2);

        y1 += sy;
        y2 -= sy;

        e += (2 * dx);

        if (e >= 0) {
          x1 += sx;
          x2 -= sx;

          e -= (2 * dy);
        }
      }

      BMAP_SET(ptr, x1, y1);
    }
  }

  return ret;
}

static int
bmap_draw_polygon(bmap_t* ptr, coord_t coords[], size_t n)
{
  int ret;
  int i;

  /*
   * initialize 
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = ERROR;
      break;
    }

    if (coords == NULL) {
      ret = ERROR;
      break;
    }

    if (n <= 2) {
      ret = ERROR;
      break;
    }

    for (i = 0; i < (int)n; i++) {
      if (coords[i].x < 0 || coords[i].x >= ptr->w) {
        ret = ERROR;
        break;
      }

      if (coords[i].y < 0 || coords[i].y >= ptr->h) {
        ret = ERROR;
        break;
      }
    }
  } while(0);

  /*
   * draw line
   */
  if (!ret) {
    n--;

    for (i = 0; i < (int)n; i++) {
      bmap_draw_line(ptr, coords + i, coords + (i + 1));
    }

    bmap_draw_line(ptr, coords + 0, coords + n);
  }

  return ret;
}

static void
fill(uint8_t* p, int l, int r)
{
  int rem;
  uint8_t msk;
  int len;
  int n;
  int i;

  rem = l % UNIT_SIZE;
  len = (r - l) + 1;

  if (rem == len) {
    for (i = l; i <= r; i++) BSET(p, i);

  } else {
    if (rem > 0) {
      *p++ |= ((1 << rem) - 1);
      len  -= rem;
    }

    n = len / UNIT_SIZE; 
    if (n > 0) {
      memset(p, 0xff, n);
      p += n;
    }

    rem = len % UNIT_SIZE;
    if (rem != 0) {
      *p |= ~((1 << (UNIT_SIZE - rem)) - 1);
    }
  }
}

static int
scan_line(bmap_t* ptr, int l, int r, int y, int py)
{
  int ret;
  int err;
  uint8_t* lp;
  seed_t s;

  ret = 0;
  lp  = ptr->buf + (ptr->s * y);

  while (l <= r) {
    /* 左側について塗りつぶし済みの領域を飛ばす */
    while (l < r && BTEST(lp, l)) l++;
    if (l == r) break;
    
    s.l  = l;

    /* 右側について塗りつぶしてないの領域を飛ばす */
    while (l <= r && !BTEST(lp, l)) l++;

    s.r  = l - 1;
    s.y  = y;
    s.py = py;

    /* シードをスタックに積む */
    err = seed_stack_push(ptr->stk, &s);
    if (err) {
      ret = ERROR;
      break;
    }
  }

  return ret;
}

static int
bmap_fill_closed(bmap_t* ptr, int x, int y)
{
  int ret;
  int err;
  int i;
  seed_t s;
  uint8_t* lp;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = ERROR;
      break;
    }

    if (x < 0 || x >= ptr->w) {
      ret = ERROR;
      break;
    }

    if (y < 0 || y >= ptr->h) {
      ret = ERROR;
      break;
    }
  } while (0);

  /*
   * fill process
   */
  if (!ret) do {
    if (BMAP_TEST(ptr, x, y)) {
      /* 指定座標が既に塗りつぶし済みの場合は何もしない */
      break;
    }

    s.l  = x;
    s.r  = x;
    s.y  = y;
    s.py = y;

    err = seed_stack_push(ptr->stk, &s);
    if (err) {
      ret = ERROR;
      break;
    }

    while (!ret && !IS_STACK_EMPTY(ptr->stk)) {
      int ls;
      int rs;
      int ny;

      seed_stack_pop(ptr->stk, &s);

      lp = ptr->buf + (ptr->s * s.y);

      /* 処理済みのシードの場合はスキップ */
      if (BTEST(lp, s.l)) continue;

      ls = s.l - 1;
      rs = s.r + 1;

      /* 左側の境界を探査 */
      while (s.l >= 0 && !BTEST(lp, s.l - 1)) s.l--; 

      /* 右側の境界を探査 */
      while (s.r < ptr->w && !BTEST(lp, s.r + 1)) s.r++; 

      /* 線分を描画 */
      //for (i = s.l; i <= s.r; i++) BSET(lp, i);
      fill(lp, s.l, s.r);

      /* 真上をスキャン */
      ny = s.y - 1;
      if (ny >= 0) {
        if (ny == s.py) {
          err = scan_line(ptr, s.l,  ls, ny, s.y);
          if (err) {
            ret = ERROR;
            break;
          }

          err = scan_line(ptr,  rs, s.r, ny, s.y);
          if (err) {
            ret = ERROR;
            break;
          }

        } else {
          err = scan_line(ptr, s.l, s.r, ny, s.y);
          if (err) {
            ret = ERROR;
            break;
          }
        }
      }

      /* 真下をスキャン */
      ny = s.y + 1;
      if (ny < ptr->h) {
        if (ny == s.py) {
          err = scan_line(ptr, s.l,  ls, ny, s.y);
          if (err) {
            ret = ERROR;
            break;
          }

          err = scan_line(ptr,  rs, s.r, ny, s.y);
          if (err) {
            ret = ERROR;
            break;
          }

        } else {
          err = scan_line(ptr, s.l, s.r, ny, s.y);
          if (err) {
            ret = ERROR;
            break;
          }
        }
      }
    }
  } while (0);

  return ret;
}

int
main(int argc, char* argv[])
{
  int err;
  bmap_t* bm;
  void* msk;

  const uint8_t header[] = {
    0x42, 0x4d,               // bfType
    0x20, 0x00, 0x02, 0x00,   // bfSize
    0x00, 0x00,               // bfReserved1
    0x00, 0x00,               // bfReserved2
    0x20, 0x00, 0x00, 0x00,   // bfOffBits

    0x0c, 0x00, 0x00, 0x00,   // bcSize
    0x00, 0x04,               // bcWidth
    0x00, 0x04,               // bcHeight
    0x01, 0x00,               // bcPlanes
    0x01, 0x00,               // bcBitCount

    // color pallet
    0x00, 0x00, 0x00,
    0xff, 0xff, 0xff,
  };

  const coord_t polygon[] = {
    {  3,   0},
    {131, 128},
    {  3, 128}
  };
    
  bm  = NULL;
  msk = NULL;

  err = bmap_new(1024, 1024, &bm);
  printf("%d %p\n", err, bm);

  //err = bmap_draw_line(bm, polygon + 0, polygon + 1);
  //printf("%d\n", err);
  err = bmap_draw_polygon(bm, polygon, N(polygon));
  printf("%d\n", err);

  err = bmap_fill_closed(bm, 23, 64);
  printf("%d %p\n", err, msk);

  err = bmap_strip(bm, &msk);
  printf("%d %p\n", err, msk);

  {
    FILE* fp;

    fp = fopen("afo.bmp", "wb");
    fwrite(header, sizeof(header), 1, fp);
    fwrite(msk, 128 * 1024, 1, fp);
    fclose(fp);
  }

  free(msk);
}
