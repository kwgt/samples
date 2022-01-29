/*
 * Hash map container
 *
 *  Copyright (C) 2021 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__
#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

#include <stddef.h>

typedef struct __hmap_t__ hmap_t;

#define HMAP_ERROR_OUT_OF_RANGE     (-1)
#define HMAP_ERROR_CONSTRAINT       (-2)
#define HMAP_ERROR_NULL_POINTER     (-3)
#define HMAP_ERROR_NO_MEMORY        (-4)
#define HMAP_ERROR_FULL             (-5)
#define HMAP_ERROR_EMPTY            (-6)
#define HMAP_ERROR_NOT_FOUND        (-7)

/*
 * @fn
 *   int hmap_new(size_t size, hmap_t** dst);
 *
 * @brief  ハッシュマップオブジェクトの生成
 *
 * @param [in] size  ハッシュマップのサイズ
 * @param [out] dst  生成したオブジェクトの格納先
 *
 * @return 正常に処理できた場合は0、失敗した場合はそれ以外の値を返す。
 *
 * @retval HMAP_ERROR_OUT_OF_RANGE
 *   引数sizeで指定されたハッシュマップのサイズが16未満の場合に返す。
 *
 * @retval HMAP_ERROR_CONSTRAINT
 *   引数sizeで指定されたハッシュマップのサイズが2の冪乗数でない場合に返す。
 *
 * @retval HMAP_ERROR_NULL_POINTER
 *   NULLが許容されないポインタ引数にNULLが指定された場合に返す。
 *
 * @retval HMAP_ERROR_NO_MEMORY
 *   メモリの確保に失敗した場合に返す。
 *
 * @remark
 *   ハッシュマップサイズは16以上、かつ2の冪乗数(16,32,64,128 ...)のみが指定
 *   可能
 *
 * @remark
 *   本関数で生成されるハッシュマップは満杯になっても自動的に拡張されない(リ
 *   ハッシュは行わない)。
 */
extern int hmap_new(size_t size, hmap_t** dst);

/*
 * @fn
 *   int hmap_destroy(hmap_t** dst);
 *
 * @brief  ハッシュマップオブジェクトの破棄
 *
 * @param [in] ptr  破棄対象のハッシュマップオブジェクト
 *
 * @return 正常に処理できた場合は0、失敗した場合はそれ以外の値を返す。
 *
 * @retval HMAP_ERROR_NULL_POINTER
 *   NULLが許容されないポインタ引数にNULLが指定された場合に返す。
 *
 * @remark
 *   本関数呼び出した場合、ハッシュマップのクリア処理を行う。これに伴いコー
 *   ルバック関数が登録済みの場合、コールバック呼び出しを行う場合がある。
 */
extern int hmap_destroy(hmap_t* dst);

/*
 * @fn
 *   int hmap_store(hmap_t* ptr, char* key, void* value);
 *
 * @brief  ハッシュマップへの保存（値の保存）
 *
 * @param [in] ptr  対象のハッシュマップオブジェクト
 * @param [in] key  データのキー(NUL終端文字列)
 * @param [in] value  データの値(任意のオブジェクトのポインタ, NULL可)
 *
 * @return 正常に処理できた場合は0、失敗した場合はそれ以外の値を返す。
 *
 * @retval HMAP_ERROR_NULL_POINTER
 *   NULLが許容されないポインタ引数にNULLが指定された場合に返す。
 *
 * @retval HMAP_ERROR_FULL
 *   ハッシュマップが満杯(空きがない場合)に返す。
 *
 * @retval HMAP_ERROR_NO_MEMORY
 *   メモリ確保に失敗した場合に返す。
 *
 * @remark
 *   引数 keyで指定する文字列は関数内部でコピーを作成する。このため、関数終了
 *   後は開放・書き換えなどの処理を行っても問題ない。
 *
 * @remark
 *   本関数で指定したキーがすでに保存済みの場合、値の上書きを行う。コールバッ
 *   ク関数の登録を行っている場合は、上書き発生時にコールバック呼び出しが行わ
 *   れる。
 */
extern int hmap_store(hmap_t* ptr, char* key, void* value);

/**
 * @fn
 *   int hmap_fetch(hmap_t* ptr, char* key, void** dst);
 *
 * @brief  ハッシュマップの読み出し
 *
 * @param [in] ptr  対象のハッシュマップオブジェクト
 * @param [in] key  読み出し対象のキー
 * @param [out] dst  読み出した値の書き込み先
 *
 * @return 正常に処理できた場合は0、失敗した場合はそれ以外の値を返す。
 *
 * @retval HMAP_ERROR_NULL_POINTER
 *   NULLが許容されないポインタ引数にNULLが指定された場合に返す。
 *
 * @retval HMAP_ERROR_NOT_FOUND
 *   キーに対応するデータが見つからなかった場合に返す。
 */
extern int hmap_fetch(hmap_t* ptr, char* key, void** dst);

/**
 * @fn
 *   int hmap_remove(hmap_t* ptr, char* key);
 *
 * @brief  ハッシュマップからの削除
 *
 * @param [in] ptr  対象のハッシュマップオブジェクト
 * @param [in] key  削除対象のキー
 *
 * @return 正常に処理できた場合は0、失敗した場合はそれ以外の値を返す。
 *
 * @retval HMAP_ERROR_NULL_POINTER
 *   NULLが許容されないポインタ引数にNULLが指定された場合に返す。
 *
 * @retval HMAP_ERROR_EMPTY
 *   登録が行われていないオブジェクト(空のオブジェクト)に対して呼び出した場
 *   合に返す。
 *
 * @retval HMAP_ERROR_NOT_FOUND
 *   キーに対応するエントリが見つからなかった(キーが登録されていない)場合に
 *   返す。
 *
 * @remark
 *   エントリの削除が行われる場合 (指定されたキーに対応するデータが登録され
 *   ている場合)、 コールバック関数の登録が行われていればコールバック呼び出
 *   しが行われる。
 */
extern int hmap_remove(hmap_t* ptr, char* key);

/**
 * @fn
 *   int hmap_clear(hmap_t* ptr);
 *
 * @brief  ハッシュマップのクリア
 *
 * @param [in] ptr  クリア対象のハッシュマップオブジェクト
 *
 * @return 正常に処理できた場合は0、失敗した場合はそれ以外の値を返す。
 *
 * @retval HMAP_ERROR_NULL_POINTER
 *   NULLが許容されないポインタ引数にNULLが指定された場合に返す。
 *
 * @remark
 *   ハッシュマップのクリアに伴い、エントリの削除が行われた場合、コールバッ
 *   ク関数の登録が行われていれば、コールバック呼び出しが行われる。
 */
extern int hmap_clear(hmap_t* ptr);

/**
 * @fn
 *   int hmap_size(hmap_t* ptr, size_t* dst);
 *
 * @brief  登録済みエントリ数の取得
 *
 * @param [in] ptr  対象のハッシュマップオブジェクト
 * @param [out] dst  登録されているエントリの数の書き込み先
 *
 * @return 正常に処理できた場合は0、失敗した場合はそれ以外の値を返す。
 *
 * @retval HMAP_ERROR_NULL_POINTER
 *   NULLが許容されないポインタ引数にNULLが指定された場合に返す。
 */
extern int hmap_size(hmap_t* ptr, size_t** dst);

/**
 * @fn
 *   int hmap_set_callback(hmap_t* ptr, void(*fn)(char*, void*));
 *
 * @brief  コールバック関数の登録
 *
 * @param [in] ptr  対象のハッシュマップオブジェクト
 * @param [in] fn  登録するコールバック関数のポインタ(NULL可)
 *
 * @return 正常に処理できた場合は0、失敗した場合はそれ以外の値を返す。
 *
 * @retval HMAP_ERROR_NULL_POINTER
 *   NULLが許容されないポインタ引数にNULLが指定された場合に返す。
 *
 * @remark
 *   本関数で登録したコールバック関数は、何らかの理由で登録されたデータがハ
 *   ッシュマップの管理下から除外される場合に、その通知として呼び出される。
 *   コールバック関数は二つの引数を取り、第一引数にキーを、第二引数に値を指
 *   定して呼び出しを行う。このコールバック関数の引数で渡される値は、ハッシ
 *   ュマップの管理から除外されるので開放などの破壊的処理を行っても問題ない。
 */
extern int hmap_set_callback(hmap_t* ptr, void(*fn)(char*, void*));

extern int hmap_iter(hmap_t* ptr, char** dsk, void** dsv);
extern int hmap_rewind(hmap_t* ptr);

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */
#endif /* !defined(__HASH_MAP_H__) */
