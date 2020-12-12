#ifndef __FONT_H__
#define __FONT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <uchar.h>

typedef struct {
  char* name;
  char* charset;
  int size;
  int ascent;
  int decent;

  struct {
    int wd;
    int ht;
    int x;
    int y;
  } bbox;
} font_t;

typedef struct {
  font_t* font;
  uint8_t wd;

  struct {
    int wd;
    int ht;
    int x;
    int y;
  } bbox;

  uint8_t bmap[];
} glyph_t;

typedef struct {
  char16_t code;
  glyph_t* glyph;
} code_entry_t;

typedef struct {
  int size;
  int ascent;
  int decent;

  font_t** fonts;
  glyph_t* fallback;

  code_entry_t* map;
  size_t n;
} fontset_t;

extern int find_glyph(fontset_t* fs, char32_t targ, glyph_t** dst);

#endif /* !defined(__FONT_H__) */

