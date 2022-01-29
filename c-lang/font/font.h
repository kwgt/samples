#ifndef __FONT_H__
#define __FONT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <uchar.h>

typedef struct {
  const char* name;
  const char* charset;
  int size;
  int decent;
  int ascent;

  struct {
    int wd;
    int ht;
    int x;
    int y;
  } bbox;
} font_t;

typedef struct {
  const font_t* font;
  uint8_t wd;

  struct {
    int wd;
    int ht;
    int x;
    int y;
  } bbox;

  const uint8_t bmap[];
} glyph_t;

typedef struct {
  char16_t code;
  const glyph_t* glyph;
} code_entry_t;

typedef struct {
  int size;
  int decent;
  int ascent;

  const font_t** fonts;
  const glyph_t* fallback;

  const code_entry_t* map;
  size_t n;
} fontset_t;

extern int find_glyph(fontset_t* fs, char32_t targ, glyph_t** dst);
#endif /* !defined(__FONT_H__) */

