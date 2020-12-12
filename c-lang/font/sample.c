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

extern fontset_t font_h10r;
extern fontset_t font_f10r;
extern fontset_t font_h10b;
extern fontset_t font_f10b;
extern fontset_t font_h12r;
extern fontset_t font_f12r;
extern fontset_t font_h12b;
extern fontset_t font_f12b;
uint8_t bmap[WIDTH * HEIGHT];

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
    src = gl->bmap;
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
  memset(bmap, 0xe0, sizeof(bmap));

  put_string(&font_h12r, 3, 36, "abcdefghij 12345 あっぱれ");
  put_string(&font_h12r, 3, 70, "ABCDEFGHIJ 67890 天晴");
  put_string(&font_h12b, 3, 104, "abcdefghij 12345 残念");
  put_string(&font_h12r, 3, 138, "67890 東京特許許可局");
  put_string(&font_h12b, 3, 172, "67890 東京特許許可局");

  write_png_file(bmap, WIDTH, HEIGHT, WIDTH, "afo.png");

  return 0;
}

