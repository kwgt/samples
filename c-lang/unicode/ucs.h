/*
 * unicode utility
 *
 *  Copyright (C) 2021 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#ifndef __UNICODE_UTILITY_H__
#define __UNICODE_UTILITY_H__

#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>

/**
 * @brief
 *  UTF-8文字列中の文字数を数える
 *
 * @param [in] src  文字数を数える文字列のポインタ
 * @param [out] dst  文字数を格納する領域のアドレス
 *
 * @return
 *  処理に成功した場合は0を、失敗した場合は0以外を返す。
 *
 * @remark
 *  本関数は、バイト数ではなくunicode文字数のカウントを行う。
 */
size_t utf8_len(char* src, size_t* dst);

/**
 * @brief
 *  UTF-8文字列のUCS文字列への展開
 *
 * @param [in] src  UTF-8文字列のポインタ
 * @param [out] dst  UCS文字列を展開する配列のアドレス
 *
 * @return
 *  処理に成功した場合は0を、失敗した場合は0以外を返す。
 *
 * @attention
 *  本関数では出力領域の確保などは行わない。予め呼び出し側において、srcに含ま
 *  れる文字数をカウントし、必要領域を確保した上で本関数を呼び出すこと。
 */
int utf8_extract(char* src, char32_t* dst);

/**
 * @brief
 *  UTF-8文字列のunicodeのコードポイント配列への変換
 *
 * @param [in] src  UTF-8文字列のポインタ
 * @param [out] dst  UCS文字列を格納する領域のアドレス
 * @param [out] dsz  変換した文字数を書き込む領域 (NULL可)
 *
 * @return
 *  処理に成功した場合は0を、失敗した場合は0以外を返す。
 *
 * @remark
 *  本関数では引数srcで指定された領域にUTF-8文字列が格納されているものとみなし、
 *  その内容を UCS文字列に変換してその領域に変換結果を書き込み、その領域を引数
 *  dstで指定した領域返す。
 *
 * @attention
 *  本関数では出力領域の確保を行う。本関数が返した領域が不要になった場合はfree()
 *  を用いて開放すること。
 */
int utf8_to_ucs(char* src, char32_t** dst, size_t* dsz);

/**
 * @brief
 *  UTF-8文字列から一文字デコードし、unicodeコードポイントを返す
 *
 * @param [in] src  デコードするUTF-8文字列のアドレス
 * @param [out] dst  コードポイントを格納するアドレス
 * @param [out] dsz  コードポイントに対応するUTF-8文字列のバイト数
 *
 * @return
 *  処理に成功した場合は0を、失敗した場合は0以外を返す。
 *
 * @remark
 *  本関数は引数srcで指定したポインタをUTF-8文字列の先頭アドレスと見なし、その
 *  アドレスからunicode1文字分のデコードを行い、そのコードポイントを引数 dstで
 *  指定した領域に格納する。また、そのコードポイントをデコードするために解析し
 *  たバイト数を引数dszで指定した領域に格納する。
 */
int utf8_fetch(char* src, char32_t* dst, size_t* dsz);

#endif /* !defined(__UNICODE_UTILITY_H__) */
