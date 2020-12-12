/*
 *  Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#ifndef __UNICODE_UTF8_H__
#define __UNICODE_UTF8_H__

#include <stdio.h>
#include <stdint.h>
#include <uchar.h>

extern int utf8_len(char* src);
extern int utf8_dec(char* src, char32_t* dst);

#endif /* !defined(__UNICODE_UTF8_H__) */
