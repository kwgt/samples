/*
 * universal buffer
 *
 *  Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "ubuf.h"

#define DEFAULT_ERROR     __LINE__
#define ALLOC(t)          ((t*)malloc(sizeof(t)))

#define ROUND_UNIT        8192
#define ROUND_UP(n)       ((n - 1) + (ROUND_UNIT - ((n - 1) % ROUND_UNIT)))
#define GROW(n)           ((n * 13) / 10)

/**
 * バッファ容量の拡張
 *
 * @param [in,out] ptr  対象となるバッファオブジェクト
 * @param [in] req 必要とするサイズ(増分を指定)
 */
static int
extend(ubuf_t* ptr, size_t req)
{
  int ret;
  size_t capa;
  void* data;

  do {
    ret = 0;

    // 増分が0なら何もしない
    if (req <= 0) break;

    // 現容量で足りるなら何もしない
    capa = ptr->size + req;
    if (capa < ptr->capa) break;

    // 単位サイズに切り上げてバッファを拡張
    capa = ROUND_UP(capa);
    data = realloc(ptr->data, capa);
    if (data == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    // バッファオブジェクトを更新
    ptr->capa = capa;
    ptr->data = data;
  } while (0);

  return ret;
}

/**
 * バッファオブジェクトの生成
 *
 * @param [in] capa  バッファの初期容量(0指定可能)
 * @param [out] dst  生成したオブジェクトの格納先
 *
 * @retval 0 成功
 * @retval 0以外 失敗
 */
int
ubuf_new(size_t capa, ubuf_t** dst)
{
  int ret;
  ubuf_t* obj;
  void* data;

  /*
   * initialize
   */
  ret  = 0;
  obj  = NULL;
  data = NULL;

  /*
   * argument check
   */
  if (dst == NULL) ret = DEFAULT_ERROR;

  /*
   * capacity roundup
   */
  if (!ret) capa = ROUND_UP(capa);

  /*
   * memory allocate
   */
  if (!ret) do {
    obj = ALLOC(ubuf_t);
    if (obj == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (capa > 0) {
      data = malloc(capa);
      if (data == NULL) {
        ret = DEFAULT_ERROR;
        break;
      }
    }
  } while (0);

  /*
   * setup object
   */
  if (!ret) {
    obj->data = data;
    obj->capa = capa;
    obj->size = 0;
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
    if (data != NULL) free(data);
  }

  return ret;
}

/**
 * バッファオブジェクトの生成2(初期化データ指定有り)
 *
 * @param [in] data  バッファに書き込むデータ
 * @param [in] size  バッファに書き込むデータのサイズ
 * @param [out] dst  生成したオブジェクトの格納先
 *
 * @retval 0 成功
 * @retval 0以外 失敗
 */
int
ubuf_new2(void* data, size_t size, ubuf_t** dst)
{
  int ret;
  ubuf_t* obj;

  /*
   * initialize
   */
  ret  = 0;
  obj  = NULL;

  /*
   * argument check
   */
  do {
    if (data == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * create buffer object
   */
  if (!ret) ret = ubuf_new(size, &obj);

  /*
   * copy data
   */
  if (!ret) {
    memcpy(obj->data, data, size);
    obj->size = size;
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = obj;

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) ubuf_destroy(obj);
  }

  return ret;
}

/**
 * バッファオブジェクトの生成3(文字列による初期化指定有り)
 *
 * @param [in] s  バッファに書き込む文字列データ
 * @param [out] dst  生成したオブジェクトの格納先
 *
 *  @note
 *    文字列終端のNUL文字は追加されません。
 *
 * @retval 0 成功
 * @retval 0以外 失敗
 */
int
ubuf_new3(char* s, ubuf_t** dst)
{
  int ret;

  /*
   * initialize
   */
  ret  = 0;

  /*
   * argument check
   */
  do {
    if (s == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * call ubuf_new2()
   */
  if (!ret) ret = ubuf_new2((void*)s, strlen(s), dst);

  return ret;

}

/**
 * バッファオブジェクトの破棄
 *
 * @param [in] ptr  破棄対象のバッファオブジェクト
 *
 * @retval 0 成功
 * @retval 0以外 失敗
 */
int
ubuf_destroy(ubuf_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * check argument
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * release memory
   */
  if (!ret) {
    if (ptr->data != NULL) {
      memset(ptr->data, 0, ptr->size);
      free(ptr->data);
    }
    free(ptr);
  }

  return ret;
}

/**
 *  バッファオブジェクトへのデータの追加
 * 
 *  @param [in,out] ptr 対象のバッファオブジェクト
 *  @param [in] data  追加するデータのアドレス
 *  @param [in] size  追加するデータのサイズ
 *
 *  @retval 0 成功 
 *  @retval 0以外 失敗
 */
int
ubuf_append(ubuf_t* ptr, void* data, size_t size)
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

    if (data == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * append specified data to buffer object
   */
  if (!ret) do {
    // 指定サイズが0の場合は何もせず終了
    if (size == 0) break;

    // 必要に応じてバッファ容量を拡張
    ret = extend(ptr, size);
    if (ret) break;

    // データをコピー
    memcpy(((uint8_t*)ptr->data) + ptr->size, data, size);

    // 使用量を更新
    ptr->size += size;
  } while (0);


  return ret;
}

/**
 *  バッファオブジェクトへの文字列の追加
 * 
 *  @param [in,out] ptr 対象のバッファオブジェクト
 *  @param [in] s  追加する文字列データのアドレス
 *
 *  @note
 *    文字列終端のNUL文字は追加されません。
 *
 *  @retval 0 成功 
 *  @retval 0以外 失敗
 */
int
ubuf_strcat(ubuf_t* ptr, char* s)
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

    if (s == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * call ubuf_append()
   */
  if (!ret) ret = ubuf_append(ptr, (void*)s, strlen(s));

  return ret;
}

/**
 *  バッファオブジェクト先頭からのデータのスライス
 * 
 *  @param [in,out] ptr 対象のバッファオブジェクト
 *  @param [in] size  スライスするデータのサイズ
 *  @param [out] dst  スライスしたデータのコピー先(NULL可)
 *  @param [out] dsz  実際にスライスできたバイト数(NULL可)
 *
 *  @retval 0 成功 
 *  @retval 0以外 失敗
 */
int
ubuf_slice(ubuf_t* ptr, size_t size, void* dst, size_t* dsz)
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
   * do slice
   */
  if (!ret) do {
    // 実際にスライスできるサイズ（実効スライスサイズ）を計算
    if (size > ptr->size) size = ptr->size;

    // 実効スライスサイズのコピーを要求されている場合はコピー
    if (dsz != NULL) *dsz = size;

    // 実効スライスサイズが0の場合は以降の処理はスキップ
    if (size == 0) break;

    // スライスされるデータのコピーを要求されている場合はコピー
    if (dst != NULL) memcpy(dst, ptr->data, size);

    // 当該領域をバッファから除去
    ptr->size -= size;
    memmove(ptr->data, (uint8_t*)ptr->data + size, ptr->size);
    memset((uint8_t*)ptr->data + size, 0, size);
  } while (0);

  return ret;
}

/**
 *  バッファオブジェクトからバッファ内容の抜き出し
 * 
 *  @param [in,out] ptr 対象のバッファオブジェクト
 *  @param [out] dst  データの書き込み先(NULL可)
 *  @param [out] dsz  データサイズの書き込み先(NULL可)
 *
 *  @retval 0 成功 
 *  @retval 0以外 失敗
 *
 *  @note
 *    本関数を呼び出すと、バッファ内容が空になります。
 *
 *  @note
 *    本関数からデータを受け取った場合、データが不要になったらfree()関数で開
 *    放してください。
 */
int
ubuf_eject(ubuf_t* ptr, void** dst, size_t* dsz)
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
   * do slice
   */
  if (!ret) do {
    // データの引き渡しを要求されている場合はポインタをコピー
    // そうでない場合は、本関数内で開放する。
    if (dst != NULL) {
      *dst = ptr->data;
    } else {
      memset(ptr->data, 0, ptr->size);
      free(ptr->data);
    }

    // データサイズのコピーを要求されている場合はコピー
    if (dsz != NULL) *dsz = ptr->size;

    // バッファ内容をリセット
    ptr->data = NULL;
    ptr->size = 0;
    ptr->capa = 0;
  } while (0);

  return ret;
}

/**
 * バッファ内容の削除
 * 
 *  @param [in,out] ptr 対象のバッファオブジェクト
 *
 *  @retval 0 成功 
 *  @retval 0以外 失敗
 *
 *  @note
 *    本関数を呼び出すと、バッファ内容が空になります。
 *
 *  @note
 *    ubuf_eject(ptr, NULL, NULL)と同様の処理を行います。
 */
int
ubuf_clear(ubuf_t* ptr)
{
  return ubuf_eject(ptr, NULL, NULL);
}

/**
 * バッファサイズの変更
 * 
 *  @param [in,out] ptr 対象のバッファオブジェクト
 *
 *  @retval 0 成功 
 *  @retval 0以外 失敗
 *
 *  @note
 *    本関数を呼び出すと、バッファの使用量が変更されます。拡張方向に
 *    リサイズされた場合、拡張部分の内容は不定です。
 */
int
ubuf_resize(ubuf_t* ptr, size_t size)
{
  int ret;
  int err;

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

    if (size <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * do resize
   */
  if (!ret) do {
    // extend()でバッファの容量を拡張するがextend()には使用量からの
    // 増分を指定する必要があるのでこのようなコードになっている
    if (ptr->size < size) {
      err = extend(ptr, size - ptr->size);
      if (err) {
        ret = DEFAULT_ERROR;
        break;
      }
    }

    ptr->size = size;
  } while (0);
    
  return ret;
}
