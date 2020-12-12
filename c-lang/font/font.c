#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>

#include "font.h"

#define DEFAULT_ERROR   __LINE__

int
find_glyph(fontset_t* fs, char32_t targ, glyph_t** dst)
{
  int ret;
  int head;
  int pos;
  int tail;
  char32_t code;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (fs == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (targ > 0xffff) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * search entry
   */
  if (!ret) {
    // 二分探査でコードマップ中の当該エントリを探査

    head = 0;
    tail = fs->n - 1;

    while(1) {
      pos  = (head + tail) / 2;
      code = fs->map[pos].code;

      if (head == tail) break;

      if (targ < code) {
        tail = pos - 1;
        continue;
      }

      if (targ > code) {
        head = pos + 1;
        continue;
      }

      /* 
       * ここに到達した場合は一致したエントリが見つかった
       * 事を意味する。
       */
      break;
    }
  }

  /*
   * put return parameter
   */
  if (!ret) {
    /*
     * head == tailの時に、一致した物が見つかる可能性
     * についてはここでフォローしている
     */
    *dst = ((targ == code)? fs->map[pos].glyph: fs->fallback);
  }

  return ret;
}
