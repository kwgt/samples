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

size_t utf8_len(char* src, size_t* dst);
int utf8_extract(char* src, char32_t* dst);
int utf8_to_ucs(char* src, char32_t** dst, size_t* dsz);

#endif /* !defined(__UNICODE_UTILITY_H__) */
