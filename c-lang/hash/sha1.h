/*
 * SHA1 sample
 *
 *  Copyright (C) 2023 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __SHA1_H__

#include <stddef.h>
#include <stdint.h>

#define SHA1_DIGEST_SIZE  20

/**
 * @brief
 *  SHA1の算出処理コンテキストを抽象化した型
 *
 * @see
 *  struct __sha1_t__
 */
typedef struct __sha1_t__ sha1_t;

/**
 * @brief
 *  SHA1の出力結果を格納するデータタイプを抽象化した型
 */
typedef uint8_t sha1_output_t[SHA1_DIGEST_SIZE];

/**
 * @brief
 *  SHA1算出コンテキストの生成
 *
 * @param [out] dst  生成したオブジェクトの格納先
 *
 * @return
 *  処理に成功した場合は0を、失敗した場合は0以外の値を返す。
 */
int sha1_new(sha1_t** dst);

/**
 * @brief
 *  SHA1算出コンテキストの破棄
 *
 * @param [int] ptr  破棄対象オブジェクトのポインタ
 *
 * @return
 *  処理に成功した場合は0を、失敗した場合は0以外の値を返す。
 */
int sha1_destroy(sha1_t* ptr);

/**
 * @brief
 *  SHA1算出コンテキストの更新
 *
 * @param [int] ptr  対象オブジェクトのポインタ
 * @param [int] data  入力データへのポインタ
 * @param [int] size  入力データのサイズ
 *
 * @return
 *  処理に成功した場合は0を、失敗した場合は0以外の値を返す。
 */
int sha1_update(sha1_t* ptr, void* data, size_t size);

/**
 * @brief
 *  SHA1算出コンテキストのファイナライズ
 *
 * @param [int] ptr  対象オブジェクトのポインタ
 * @param [int] data  入力データへのポインタ
 * @param [int] size  入力データのサイズ
 *
 * @return
 *  処理に成功した場合は0を、失敗した場合は0以外の値を返す。
 */
int sha1_finalize(sha1_t* ptr, sha1_output_t dst);

/**
 * @brief
 *  SHA1算出関数
 *
 * @param [in] data  入力データへのポインタ
 * @param [in] size  入力データのサイズ
 * @param [out] dst  算出結果の格納先
 *
 */
int sha1(void* data, size_t size, sha1_output_t dst);

#endif /* !defined(__SHA1_H__) */
