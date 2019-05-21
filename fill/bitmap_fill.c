/*
 * bitmap fill on closed area
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DEFAULT_ERROR   __LINE__

// 単位サイズはビッグエンディアンの場合のみ変更可
#define unit_t          uint8_t
#define UNIT_BYTES      sizeof(unit_t)
#define UNIT_BITS       (UNIT_BYTES * 8)

#define MAX_COORD       100
#define BASE_MASK       (1 << (UNIT_BITS - 1))
#define INDEX(n)        ((n) / UNIT_BITS)
#define MASK(n)         (BASE_MASK >> ((n) % UNIT_BITS))
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
    ret = DEFAULT_ERROR;
  }

  /*
   * alloc memory
   */
  if (!ret) do {
    obj = ALLOC(seed_stack_t);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    buf = NALLOC(seed_t, n);
    if (buf == NULL) {
      ret = DEFAULT_ERROR;
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
    ret = DEFAULT_ERROR;
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
      ret = DEFAULT_ERROR;
      break;
    }

    if (s == NULL) {
      ret = DEFAULT_ERROR;
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
        ret = DEFAULT_ERROR;
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
      ret = DEFAULT_ERROR;
      break;
    }

    if (s == NULL) {
      ret = DEFAULT_ERROR;
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
  unit_t* buf;
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
  int bs;
  bmap_t* obj;
  unit_t* buf;
  seed_stack_t* stk;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;
  buf = NULL;
  stk = NULL;
  bs  = ((w * UNIT_BYTES) / UNIT_BITS) * h;

  /*
   * argument check
   */
  do {
    if (w < 64 || (w % UNIT_BITS) != 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (h < 64) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * alloc memory
   */
  if (!ret) do {
    obj = ALLOC(bmap_t);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    buf = (unit_t*)malloc(bs);
    if (buf == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    err = seed_stack_new(100, &stk);
    if (err) {
      ret = DEFAULT_ERROR;
      break;
    }

    memset(buf, 0, bs * UNIT_BYTES);
  } while (0);

  /*
   * put return parameter
   */
  if (!ret) {
    obj->buf = buf;
    obj->w   = w;
    obj->s   = w / UNIT_BITS;
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
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
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
      ret = DEFAULT_ERROR;
      break;
    }

    if (c1 == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (c1->x < 0 || c1->x >= ptr->w) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (c1->y < 0 || c1->y >= ptr->h) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (c2 == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (c2->x < 0 || c2->x >= ptr->w) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (c2->y < 0 || c2->y >= ptr->h) {
      ret = DEFAULT_ERROR;
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
bmap_draw_polygon(bmap_t* ptr, coord_t va[], size_t n)
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
      ret = DEFAULT_ERROR;
      break;
    }

    if (va == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (n <= 2) {
      ret = DEFAULT_ERROR;
      break;
    }

    for (i = 0; i < (int)n; i++) {
      if (va[i].x < 0 || va[i].x >= ptr->w) {
        ret = DEFAULT_ERROR;
        break;
      }

      if (va[i].y < 0 || va[i].y >= ptr->h) {
        ret = DEFAULT_ERROR;
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
      bmap_draw_line(ptr, va + i, va + (i + 1));
    }

    bmap_draw_line(ptr, va + 0, va + n);
  }

  return ret;
}

#define SHRINK(n)       ((n * 10) / 13)
#define SWAP(a,b)       do {int t; t = b; b = a; a = t;} while(0)

static void
sort(int* a, size_t n)
{
  int h;
  int f;
  int i;

  /*
   * sort by ascending order
   */

  h = n;

  while (h > 1 || f) {
    f = 0;
    h = SHRINK(h);

    if (h == 9 || h == 10) h = 11;

    for (i = 0; i < ((int)n - h); i++) {
      if (a[i] > a[i + h]) {
        SWAP(a[i], a[i + h]);
        f = !0;
      }
    }
  }
}

static void
fill(unit_t* p, int l, int r)
{
  int rem;
  unit_t msk;
  int len;
  int n;
  int i;

  p  += (l / UNIT_BITS);
  rem = UNIT_BITS - (l % UNIT_BITS);
  len = (r - l) + 1;

  /* 左側のフラグメントの処理 */
  if (rem > 0) {
    msk  = (1 << rem) - 1;
    if (rem > len) {
      msk &= ~((1 << (rem - len)) - 1);
      len  = 0;

    } else {
      len -= rem;
    }

    *p++ |= msk;
  }

  /* 中央のバイトコンプリートな部分の処理 */
  n = len / UNIT_BITS; 
  if (n > 0) {
    memset(p, 0xff, n);
    p += n;
  }

  /* 右側のフラグメントの処理 */
  rem = len % UNIT_BITS;
  if (rem != 0) {
    *p |= ~((1 << (UNIT_BITS - rem)) - 1);
  }
}

static int
bmap_fill_polygon(bmap_t* ptr, coord_t va[], size_t n)
{
  int ret;
  int* buf;
  int i;
  int j;
  int x;
  int y;
  int min_y;
  int max_y;
  int d1;
  int d2;

  coord_t* p1;
  coord_t* p2;
  coord_t* pt;

  /*
   * initialize 
   */
  ret = 0;
  buf = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (va == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (n <= 2 || n > MAX_COORD) {
      ret = DEFAULT_ERROR;
      break;
    }

    for (i = 0; i < (int)n; i++) {
      if (va[i].x < 0 || va[i].x >= ptr->w) {
        ret = DEFAULT_ERROR;
        break;
      }

      if (va[i].y < 0 || va[i].y >= ptr->h) {
        ret = DEFAULT_ERROR;
        break;
      }
    }
  } while(0);

  /*
   * alloc work buffer
   */
  if (!ret) {
    buf = NALLOC(int, MAX_COORD);
    if (buf == NULL) {
      ret = DEFAULT_ERROR;
    }
  }

  /*
   * do solid scan
   */
  if (!ret) {
    min_y = ptr->h;
    max_y = 0;

    for (i = 0; i < (int)n; i++) {
      if (va[i].y < min_y) min_y = va[i].y;
      if (va[i].y > max_y) max_y = va[i].y;
    }
  }

  if (!ret && min_y != max_y) {
    for (y = min_y; y <= max_y; y++) {
      j = 0;

      for (i = 0; i < n; i++) {
        p1 = va + i;
        p2 = va + ((i + 1) % n);

        // 水平線になる場合は除外
        if (p1->y == p2->y) continue;

        // 奇数交点を避ける
        if (p2->y == y) {
          pt = va + ((i + 2) % n); 
          d1 = p2->y - p1->y;
          d2 = pt->y - p2->y;

          if (((d1 < 0) && (d2 < 0)) || ((d1 > 0) && (d2 > 0))) {
            continue;
          }
        }

        // スキャンラインが範囲外の場合は除外
        if (p1->y > p2->y) {
          pt = p1;
          p1 = p2;
          p2 = pt;
        }

        if (y < p1->y || y > p2->y) continue;

        // スキャンラインとの交点を計算
        x = (((p2->x - p1->x) * (y - p1->y)) / (p2->y - p1->y)) + p1->x;
        buf[j++] = x;
      }

      sort(buf, j);

      for (i = 0; i < j; i += 2) {
        fill(ptr->buf + (ptr->s * y), buf[i], buf[i + 1]);
      }
    }
  }

  /*
   * post process
   */
  if (buf) free(buf);

  return ret;
}

static int
scan_line(bmap_t* ptr, int l, int r, int y, int py)
{
  int ret;
  int err;
  unit_t* lp;
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
      ret = DEFAULT_ERROR;
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
  unit_t* lp;

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

    if (x < 0 || x >= ptr->w) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (y < 0 || y >= ptr->h) {
      ret = DEFAULT_ERROR;
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
      ret = DEFAULT_ERROR;
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
            ret = DEFAULT_ERROR;
            break;
          }

          err = scan_line(ptr,  rs, s.r, ny, s.y);
          if (err) {
            ret = DEFAULT_ERROR;
            break;
          }

        } else {
          err = scan_line(ptr, s.l, s.r, ny, s.y);
          if (err) {
            ret = DEFAULT_ERROR;
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
            ret = DEFAULT_ERROR;
            break;
          }

          err = scan_line(ptr,  rs, s.r, ny, s.y);
          if (err) {
            ret = DEFAULT_ERROR;
            break;
          }

        } else {
          err = scan_line(ptr, s.l, s.r, ny, s.y);
          if (err) {
            ret = DEFAULT_ERROR;
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
    { 20,   0},
    { 300,  150},
    {  0,  300}
  };
    
  bm  = NULL;
  msk = NULL;

  err = bmap_new(1024, 1024, &bm);
  printf("%d %p\n", err, bm);

  //err = bmap_draw_line(bm, polygon + 0, polygon + 1);
  //printf("%d\n", err);
#if 0
  err = bmap_draw_polygon(bm, polygon, N(polygon));
  printf("%d\n", err);

  err = bmap_fill_closed(bm, 23, 64);
  printf("%d %p\n", err, msk);
#endif

  err = bmap_fill_polygon(bm, polygon, N(polygon));
  printf("%d\n", err);

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
