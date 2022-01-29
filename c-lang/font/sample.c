#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#include "utf8.h"
#include "font.h"
#include "png_file.h"

#define N(x)        (sizeof(x) / sizeof(*x))

#define WIDTH           200
#define HEIGHT          200

#define USE_FONT        font_k8x12

extern fontset_t font_h10r;
extern fontset_t font_f10r;
extern fontset_t font_h10b;
extern fontset_t font_f10b;
extern fontset_t font_h12r;
extern fontset_t font_f12r;
extern fontset_t font_h12b;
extern fontset_t font_f12b;
extern fontset_t font_shnm12;
extern fontset_t font_shnm12b;
extern fontset_t font_shnm14;
extern fontset_t font_shnm14b;
extern fontset_t font_shnm16;
extern fontset_t font_shnm16b;
extern fontset_t font_ayu18;
extern fontset_t font_ayu18b;
extern fontset_t font_ayu20;
extern fontset_t font_ayu20b;
extern fontset_t font_k8x12;
uint8_t bmap[WIDTH * HEIGHT];

void
measure_string(fontset_t* fs, char* cs, int* dd, int* da, int* dw)
{
  char32_t* us;
  glyph_t* gl;
  int i;
  int n;
  int w;

  n  = utf8_len((char*)cs);
  us = (char32_t*)malloc(sizeof(char32_t) * n);
  utf8_dec((char*)cs, us);

  w = 0;
  for (i = 0; i < n; i++) {
    find_glyph(fs, us[i], &gl);
    w += gl->wd;
  }

  if (dd != NULL) *dd = fs->decent;
  if (da != NULL) *da = fs->ascent;
  if (dw != NULL) *dw = w;

  free(us);
}

void
put_string(fontset_t* fs, int x, int y, char* cs)
{
  char32_t* us;
  glyph_t* gl;
  int i;
  int j;
  int k;
  int n;

  // UTF-8デコードしてunicodeの配列に変換
  n  = utf8_len((char*)cs);
  us = (char32_t*)malloc(sizeof(char32_t) * n);
  utf8_dec((char*)cs, us);

  // 参照用にベースラインに線を引いておく
  for (i = 0; i < WIDTH; i++) {
    bmap[(WIDTH * y) + i] = 0xc0;
  }

  // 一文字ずつレンダリング
  for (i = 0; i < n; i++) {
    uint8_t* dst;
    uint8_t* src;
    uint8_t msk;

    // グリフ情報の取得
    find_glyph(fs, us[i], &gl);

    // まずポインタをベースポイントに合わせる
    dst = (bmap + (WIDTH * y)) + x;

    // 次にバウンディングボックスのオフセット調整
    dst -= (WIDTH * gl->bbox.y);
    dst += gl->bbox.x;

    // 次にバウンディングボックスの高さ分を巻き戻す。
    // そこが描画開始位置
    dst -= (WIDTH * (gl->bbox.ht - 1));

    // ビットマップデータを展開
    src = (uint8_t*)gl->bmap;
    msk = 0x80;

    for (j = 0; j < gl->bbox.ht; j++) {
      for (k = 0; k < gl->bbox.wd; k++) {
#if SHOW_BOUNDINGBOX
        *dst++ = (*src & msk)? 0x00: 0xff;
#else /* defined(SHOW_BOUNDINGBOX) */
        if (*src & msk) *dst = 0x00;
        *dst++;
#endif /* defined(SHOW_BOUNDINGBOX) */

        if (!(msk >>= 1)) {
          msk = 0x80;
          src++;
        }
      }

      dst += (WIDTH - k);
    }

    // グリフ幅を足して次の文字のベースポイントに
    // 移動させる
    x += gl->wd;
  }

  free(us);
}

int
main(int argc, char* argv[])
{
  int d;
  int a;
  int w;

  memset(bmap, 0xe0, sizeof(bmap));

  put_string(&USE_FONT, 20, 35, "設定完了");
  put_string(&USE_FONT, 20, 54, "2021/9/8 15:53:08");

  measure_string(&USE_FONT, "電池残量 70%", &d, &a, &w);
  put_string(&USE_FONT, 200 - w, 200 - d, "電池残量 70%");

  write_png_file(bmap, WIDTH, HEIGHT, WIDTH, "afo.png");

  return 0;
}

