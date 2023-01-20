/*
 * Hash map container
 *
 *  Copyright (C) 2021 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hmap.h"

#define DEFAULT_ERROR       (__LINE__)
#define ALLOC(t)            ((t*)malloc(sizeof(t)))
#define NALLOC(t,n)         ((t*)malloc(sizeof(t)*(n)))

#define ST_EMPTY            0
#define ST_USED             1
#define ST_REMOVED          2

/*
 * 構造体の定義
 */
struct key {
  void* data;
  size_t size;
};

struct bucket {
  int state;
  void* key;
  size_t ksz;
  void* val;
};

struct __hmap_t__ {
  struct bucket* bucket;
  size_t size;
  size_t mask;
  size_t used;

  struct bucket* pos;

  void (*fn)(char*, void*);
};

/*
 * 内部処理用の非公開関数の定義
 */
// implement by FNV1-32
static uint32_t
hash(uint8_t* data, size_t size)
{
  uint32_t ret;
  int i;

  ret = 0x811c9dc5;

  for (i = 0; i < (int)size; i++) {
    ret *= 0x01000193;
		/* 
     * 上記の乗算は以下の処理と同じ
     *
     *   ret += (ret <<  1) +
     *          (ret <<  4) +
     *          (ret <<  7) +
     *          (ret <<  8) +
     *          (ret << 24);
		 */
    ret ^= data[i];
  }

  return ret;
}

/**
 * @fn
 *  static void search_bucket(hmap_t* ptr,
 *                            void* key, 
 *                            size_t size, 
 *                            struct bucket** dst)
 *
 * @brief バケットの探査
 * @param [in] ptr  探査対象のハッシュマップオブジェクト
 * @param [in] key  探査キーのアドレス
 * @param [in] size  探査キーのサイズ
 * @param [out] dst  見つかったバケットの出力先
 *
 * @remark
 *  ptで返されるオブジェクトのパターンは以下の通り
 *  - NULLの場合
 *    空きが無い状態で同じキーのバケットが見つからなかった
 *
 *  - ST_USEDがマークされている場合
 *    指定されたキーに一致するバケット
 *
 *  - それ以外
 *    キーに一致するバケットが見つからず新規割り当てされたバケット
 */
static void
search_bucket(hmap_t* ptr, void* key, size_t size, struct bucket** dst)
{
  struct bucket* p0; // 探査開始位置
  struct bucket* pc; // 現探査位置
  struct bucket* pt; // 対象バケット

  /*
   * initialize
   */
  p0 = ptr->bucket + (hash(key, size) & ptr->mask);
  pc = p0;
  pt = NULL;

  /*
   * search bucket
   */
  do {
    if (pc->state == ST_USED) {
      if (pc->ksz == size && !memcmp(pc->key, key, size)) {
        // 使用中且つキーが一致する場合はそのバケットで確定
        pt = pc;
        break;
      }

    } else {
      // 最初の空き(削除済みを含む)のバケットが見つかったらとりあえず記録
      if (pt == NULL) pt = pc;

      // 探査を継続する必要が無い場合は確定させる
      if (pc->state == ST_EMPTY) break;
    }

    // 探査位置を次に進める。領域末端まで到達したら終端に巻き戻し
    if ((++pc - ptr->bucket) >= ptr->size) pc = ptr->bucket;
  } while (pc != p0);

  /*
   * put result
   *
   *  ptに対し、
   */
  *dst = pt;
}

/*
 * 外部公開関数の定義
 */

int
hmap_new(size_t size, hmap_t** dst)
{
  int ret;
  hmap_t* obj;
  struct bucket* bucket;

  /*
   * initialize
   */
  ret    = 0;
  obj    = NULL;
  bucket = NULL;

  /*
   * argument check
   */
  do {
    if (size < 16) {
      ret = HMAP_ERROR_OUT_OF_RANGE;
      break;
    }

    // 二の冪乗チェック
    if ((size & (size -1)) != 0) {
      ret = HMAP_ERROR_CONSTRAINT;
      break;
    }

    if (dst == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }
  } while (0);

  /*
   * memory allocate
   */
  if (!ret) do {
    obj = ALLOC(hmap_t);
    if (obj == NULL) {
      ret = HMAP_ERROR_NO_MEMORY;
      break;
    }

    bucket = NALLOC(struct bucket, size);
    if (obj == NULL) {
      ret = HMAP_ERROR_NO_MEMORY;
      break;
    }
  } while (0);

  /*
   * setup object
   */
  if (!ret) {
    memset(bucket, 0, sizeof(*bucket) * size);

    obj->bucket = bucket;
    obj->size   = size;
    obj->mask   = size - 1;
    obj->used   = 0;
    obj->pos    = bucket - 1;
    obj->fn     = NULL;
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = obj;

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
    if (bucket != NULL) free(bucket);
  }

  return ret;
}

int
hmap_destroy(hmap_t* ptr)
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
  if (ptr == NULL) ret = HMAP_ERROR_NULL_POINTER;

  /*
   * clear hashmap
   */
  if (!ret) ret = hmap_clear(ptr);

  /*
   * release memory
   */
  if (!ret) {
    if (ptr->bucket) free(ptr->bucket);
    free(ptr);
  }

  return ret;
}



int
hmap_store(hmap_t* ptr, char* _key, void* val)
{
  int ret;
  size_t len;
  char* key;
  struct bucket* item;

  /*
   * initialize
   */
  ret  = 0;
  key  = NULL;
  item = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }

    if (_key == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }
  } while (0);

  /*
   * state check
   */
  if (!ret) {
    if (ptr->used >= ptr->size) ret = HMAP_ERROR_FULL;
  }

  /*
   * search empty bucket
   */
  if (!ret) {
    len = strlen(_key);

    search_bucket(ptr, _key, len, &item);

    // 上段のstate checkで満杯チェックを行っているので
    // ここには引っかからないはず
    if (item == NULL) ret = HMAP_ERROR_FULL;
  }

  /*
   * duplicate key
   */
  if (!ret) {
    if (item->state != ST_USED) {
      key = strdup(_key);
      if (key == NULL) ret = HMAP_ERROR_NO_MEMORY;
    }
  }

  /*
   * update object
   */
  if (!ret) {
    if (item->state != ST_USED) {
      // 新規割り当ての場合は状態及びキー情報を書き替え、使用数を加算
      item->state = ST_USED;
      item->key   = key;
      item->ksz   = len;

      ptr->used++;

    } else {
      // 使用中領域の場合(値の上書)の場合、コールバック呼び出し。
      if (ptr->fn != NULL) ptr->fn(item->key, item->val);
    }

    item->val = val;
  }

  /*
   * post process
   */
  if (ret) {
    if (key != NULL) free(key);
  }

  return ret;
}

int
hmap_fetch(hmap_t* ptr, char* key, void** dst)
{
  int ret;
  struct bucket* item;

  /*
   * initialize
   */
  ret  = 0;
  item = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }

    if (key == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }

    if (dst == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }
  } while (0);

  /*
   * search entry
   */
  if (!ret) {
    search_bucket(ptr, key, strlen(key), &item);
    if (item == NULL || item->state != ST_USED) ret = HMAP_ERROR_NOT_FOUND;
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = item->val;

  return ret;
}

int
hmap_remove(hmap_t* ptr, char* key)
{
  int ret;
  struct bucket* item;
  struct bucket* pt;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }

    if (key == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }
  } while (0);

  /*
   * state check
   */
  if (!ret) {
    if (ptr->used == 0) ret = HMAP_ERROR_EMPTY;
  }

  /*
   * search entry
   */
  if (!ret) {
    search_bucket(ptr, key, strlen(key), &item);
    if (item == NULL || item->state != ST_USED) ret = HMAP_ERROR_NOT_FOUND;
  }

  /*
   * remove entry
   */
  if (!ret) {
    pt = item + 1;
    if ((pt - ptr->bucket) >= ptr->size) pt = ptr->bucket;

    if (pt->state != ST_EMPTY && ptr->used > 1) {
      // 対象エントリの次のエントリがST_EMPTYでない場合は、探索リンクを
      // 継続する必要があるのでST_REMOVEDに設定。
      item->state = ST_REMOVED;

    } else {
      // 対象リンクの次のエントリがST_EMPTYの場合、もしくはハッシュマップ中の
      // 最後の一個っだった場合は自身もST_EMPTYに設定。
      item->state = ST_EMPTY;

      // さらに前方に向かって連続しているST_REMOVEDを全てST_EMPTYに設定。
      for (pt = item - 1; pt != item; pt--) {
        if (pt < ptr->bucket) pt += ptr->size;
        if (pt->state != ST_REMOVED) break;

        pt->state = ST_EMPTY;
      }
    }

    // コールバックを呼び出す
    if (ptr->fn != NULL) ptr->fn(item->key, item->val);

    // キーの情報を削除
    free(item->key);

    item->key = NULL;
    item->ksz = 0;
    item->val = NULL;

    // サイズを減らす
    ptr->used--;
  }

  return ret;
}

int
hmap_clear(hmap_t* ptr)
{
  int ret;
  int i;
  struct bucket* item;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) ret = HMAP_ERROR_NULL_POINTER;

  /*
   * clear bucket table
   */
  if (!ret) {
    for (i = 0; i < (int)ptr->size; i++) {
      item = ptr->bucket + i;

      switch (item->state) {
      case ST_EMPTY:
        // ignore;
        break;

      case ST_USED:
        if (ptr->fn != NULL) ptr->fn(item->key, item->val);

        free(item->key);
        item->state = ST_EMPTY;
        item->key   = NULL;
        item->ksz   = 0;
        item->val   = NULL;
        break;

      case ST_REMOVED:
        item->state = ST_EMPTY;
        break;
      }
    }

    ptr->used = 0;
  }

  return ret;
}

int
hmap_size(hmap_t* ptr, size_t** dst)
{
  return -1;
}

int
hmap_set_callback(hmap_t* ptr, void (*fn)(char*, void*))
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * object update
   */
  if (!ret) ptr->fn = fn;

  return ret;
}

int
hmap_iter(hmap_t* ptr, char** dsk, void** dsv)
{
  int ret;
  int i;
  struct bucket* item;
  struct bucket* p;

  /*
   * initialize
   */
  ret  = 0;
  item = NULL;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }

    if (dsv == NULL) {
      ret = HMAP_ERROR_NULL_POINTER;
      break;
    }
  } while (0);

  /*
   * find next
   */
  if (!ret) {
    for (p = ptr->pos + 1; (p - ptr->bucket) < ptr->size; p++) {
      if (p->state == ST_USED) {
        item = p;
        break;
      }
    }

    if (item == NULL) ret = HMAP_ERROR_NOT_FOUND;
  }

  /*
   * update object
   */
  if (!ret) ptr->pos = item;

  /*
   * put return parameter
   */
  if (!ret) {
    if (dsk != NULL) *dsk = item->key;
    *dsv = item->val;
  }

  return ret;
}

int
hmap_rewind(hmap_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) ret = HMAP_ERROR_NULL_POINTER;

  /*
   * rewind find position
   */
  if (!ret) ptr->pos = ptr->bucket - 1;

  return ret;
}

#if 1
int
main(int args, char* argv)
{
  int err;
  hmap_t* map;
  char* key;
  void* val;

  hmap_new(16, &map);
  hmap_store(map, "1", (void*)"a");
  hmap_store(map, "2", (void*)"b");
  hmap_store(map, "3", (void*)"c");
  hmap_store(map, "4", (void*)"d");
  hmap_store(map, "5", (void*)"e");

  while (1) {
    err = hmap_iter(map, &key, &val);
    if (!err) {
      printf("*** %s %s\n", key, (char*)val);
    } else {
      break;
    }
  }

  hmap_store(map, "6", (void*)"f");
  hmap_store(map, "7", (void*)"g");
  hmap_store(map, "8", (void*)"h");
  hmap_store(map, "9", (void*)"i");
  hmap_store(map, "10", (void*)"j");
  hmap_store(map, "11", (void*)"k");
  hmap_store(map, "12", (void*)"l");
  hmap_store(map, "13", (void*)"m");
  hmap_store(map, "14", (void*)"n");
  hmap_store(map, "15", (void*)"o");
  hmap_store(map, "16", (void*)"p");

  printf("[rewind]\n");

  hmap_rewind(map);
  while (1) {
    err = hmap_iter(map, &key, &val);
    if (!err) {
      printf("*** %s %s\n", key, (char*)val);
    } else {
      break;
    }
  }

  hmap_remove(map, "1");
  hmap_remove(map, "2");
  hmap_remove(map, "3");
  hmap_remove(map, "4");

  printf("[rewind]\n");

  hmap_rewind(map);
  while (1) {
    err = hmap_iter(map, &key, &val);
    if (!err) {
      printf("*** %s %s\n", key, (char*)val);
    } else {
      break;
    }
  }

  hmap_remove(map, "5");
  hmap_remove(map, "6");
  hmap_remove(map, "7");
  hmap_remove(map, "8");
  hmap_remove(map, "9");
  hmap_remove(map, "10");
  hmap_remove(map, "11");
  hmap_remove(map, "12");
  hmap_remove(map, "13");
  hmap_remove(map, "14");
  hmap_remove(map, "15");

  hmap_remove(map, "16");


  hmap_destroy(map);

}
#endif
