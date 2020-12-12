/*
 * bmp file data extractor
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdint.h>
#include "ruby.h"

#define ROW_PAD_SIZE(w)     ((4 - ((w) & 3)) & 3)

static VALUE klass;

static void
get_image_size(VALUE self, int* wd, int* ht)
{
  VALUE h;

  h = rb_iv_get(self, "@header");

  *wd = FIX2INT(rb_hash_aref(h, ID2SYM(rb_intern("img_width"))));
  *ht = FIX2INT(rb_hash_aref(h, ID2SYM(rb_intern("img_height"))));
}

static void
get_palette(VALUE self, VALUE* _dst, size_t* _n)
{
  VALUE src;
  VALUE dst;

  uint8_t* p;
  int i;
  size_t n;

  VALUE rgb;

  src = rb_hash_aref(rb_iv_get(self, "@header"), ID2SYM(rb_intern("palette")));
  n   = RARRAY_LEN(src);

  dst = rb_str_buf_new(3 * n);
  p   = (uint8_t*)RSTRING_PTR(dst);

  for (i = 0; i < (int)n; i++) {
    rgb  = RARRAY_AREF(src, i);

    p[0] = FIX2INT(RARRAY_AREF(rgb, 2)) & 0xff;
    p[1] = FIX2INT(RARRAY_AREF(rgb, 1)) & 0xff;
    p[2] = FIX2INT(RARRAY_AREF(rgb, 0)) & 0xff;

    p += 3;
  }

  *_dst = dst;
  *_n   = n;
}

static void
get_gs_coeff(VALUE self, int* r, int* g, int* b)
{
  VALUE gs;

  gs = rb_iv_get(self, "@grayscale");

  *r = FIX2INT(rb_hash_aref(gs, ID2SYM(rb_intern("r"))));
  *g = FIX2INT(rb_hash_aref(gs, ID2SYM(rb_intern("g"))));
  *b = FIX2INT(rb_hash_aref(gs, ID2SYM(rb_intern("b"))));
}

static void
get_palette_gs(VALUE self, VALUE* _dst, size_t* _n)
{
  VALUE src;
  VALUE dst;

  uint8_t* p;
  int i;
  size_t n;

  VALUE rgb;

  int cr; // as "Coefficient for Red"
  int cg; // as "Coefficient for Green"
  int cb; // as "Coefficient for Blue"
  int val;

  get_gs_coeff(self, &cr, &cg, &cb);

  src = rb_hash_aref(rb_iv_get(self, "@header"), ID2SYM(rb_intern("palette")));
  n   = RARRAY_LEN(src);

  dst = rb_str_buf_new(n);
  p   = (uint8_t*)RSTRING_PTR(dst);

  for (i = 0; i < (int)n; i++) {
    rgb  = RARRAY_AREF(src, i);
    val  = (FIX2INT(RARRAY_AREF(rgb, 2)) * cr) + 
           (FIX2INT(RARRAY_AREF(rgb, 1)) * cg) +
           (FIX2INT(RARRAY_AREF(rgb, 0)) * cb);

    *p++ = (val >> 10) & 0xff;
  }

  *_dst = dst;
  *_n   = n;
}

static VALUE
rb_bmp_extract_image1(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  VALUE _plt;
  uint8_t* plt;
  uint8_t* src;
  uint8_t* dst;
  size_t n;

  size_t rsz;
  VALUE _row;
  uint8_t* row;

  int x;
  int y;

  int i;
  int s;

  get_image_size(self, &wd, &ht);

  get_palette(self, &_plt, &n);
  plt  = (uint8_t*)RSTRING_PTR(_plt);

  ret  = rb_str_buf_new((wd * 3) * ht);
  rb_str_set_len(ret, (wd * 3) * ht);

  dst  = (uint8_t*)RSTRING_PTR(ret) + ((wd * 3) * (ht - 1));

  rsz  = (7 + wd) / 8;
  rsz += ROW_PAD_SIZE(rsz);
  _row = rb_str_buf_new(rsz);

  for (y = 0; y < ht; y++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), _row);

    row  = (uint8_t*)RSTRING_PTR(_row);

    for (x = 0; x < wd; x++) {
      i   = x / 8;
      s   = 7 - (x % 8);

      src = plt + (((row[i] >> s) & 0x01) * 3);

      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];

      dst += 3;
    }

    dst -= (wd * 3 * 2);
  }

  RB_GC_GUARD(_plt);
  RB_GC_GUARD(_row);

  return ret;
}

static VALUE
rb_bmp_extract_image4(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  VALUE _plt;
  uint8_t* plt;
  uint8_t* src;
  uint8_t* dst;
  size_t n;

  size_t rsz;
  VALUE _row;
  uint8_t* row;

  int x;
  int y;

  int i;
  int s;

  get_image_size(self, &wd, &ht);

  get_palette(self, &_plt, &n);
  plt  = (uint8_t*)RSTRING_PTR(_plt);

  ret  = rb_str_buf_new((wd * 3) * ht);
  rb_str_set_len(ret, (wd * 3) * ht);

  dst  = (uint8_t*)RSTRING_PTR(ret) + ((wd * 3) * (ht - 1));

  rsz  = (1 + wd) / 2;
  rsz += ROW_PAD_SIZE(rsz);
  _row = rb_str_buf_new(rsz);

  for (y = 0; y < ht; y++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), _row);

    row  = (uint8_t*)RSTRING_PTR(_row);

    for (x = 0; x < wd; x++) {
      i   = x / 2;
      s   = (x & 1)? 4: 0;

      src = plt + (((row[i] >> s) & 0x0f) * 3);

      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];

      dst += 3;
    }

    dst -= (wd * 3 * 2);
  }

  RB_GC_GUARD(_plt);
  RB_GC_GUARD(_row);

  return ret;
}

static VALUE
rb_bmp_extract_image8(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  VALUE _plt;
  uint8_t* plt;
  uint8_t* src;
  uint8_t* dst;
  size_t n;

  size_t rsz;
  VALUE _row;
  uint8_t* row;

  int x;
  int y;

  get_image_size(self, &wd, &ht);

  get_palette(self, &_plt, &n);
  plt  = (uint8_t*)RSTRING_PTR(_plt);

  ret  = rb_str_buf_new((wd * 3) * ht);
  rb_str_set_len(ret, (wd * 3) * ht);

  dst  = (uint8_t*)RSTRING_PTR(ret) + ((wd * 3) * (ht - 1));

  rsz  = wd + ROW_PAD_SIZE(wd);
  _row = rb_str_buf_new(rsz);

  for (y = 0; y < ht; y++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), _row);

    row  = (uint8_t*)RSTRING_PTR(_row);

    for (x = 0; x < wd; x++) {
      src = plt + (row[x] * 3);

      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];

      dst += 3;
    }

    dst -= (wd * 3 * 2);
  }

  RB_GC_GUARD(_plt);
  RB_GC_GUARD(_row);

  return ret;
}

static VALUE
rb_bmp_extract_image24(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  size_t rsz;
  VALUE row;

  uint8_t *src;
  uint8_t *dst;

  int x;
  int y;

  get_image_size(self, &wd, &ht);

  ret = rb_str_buf_new((wd * 3) * ht);
  rb_str_set_len(ret, (wd * 3) * ht);

  dst = (uint8_t*)RSTRING_PTR(ret) + ((wd * 3) * (ht - 1));

  rsz  = wd * 3;
  rsz += ROW_PAD_SIZE(rsz);
  row  = rb_str_buf_new(rsz);


  for (y = 0; y < ht; y ++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), row);

    src = (uint8_t*)RSTRING_PTR(row);

    for (x = 0; x < wd; x++){
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];

      dst += 3;
      src += 3;
    }

    dst -= (wd * 3 * 2);
  }

  return ret;
}

static VALUE
rb_bmp_extract_image32(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  size_t rsz;
  VALUE row;

  uint8_t *src;
  uint8_t *dst;

  int x;
  int y;

  get_image_size(self, &wd, &ht);

  ret  = rb_str_buf_new((wd * 3) * ht);
  rb_str_set_len(ret, (wd * 3) * ht);

  dst  = (uint8_t*)RSTRING_PTR(ret) + ((wd * 3) * (ht - 1));

  rsz  = wd * 4;
  rsz += ROW_PAD_SIZE(rsz);
  row  = rb_str_buf_new(rsz);

  for (y = 0; y < ht; y ++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), row);

    src = (uint8_t*)RSTRING_PTR(row);

    for (x = 0; x < wd; x++){
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];

      dst += 3;
      src += 4;
    }

    dst -= (wd * 3 * 2);
  }

  return ret;
}

static VALUE
rb_bmp_extract_grayscale1(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  VALUE _plt;
  uint8_t* plt;

  uint8_t* dst;
  size_t n;

  size_t rsz;
  VALUE _row;
  uint8_t* row;

  int x;
  int y;

  int i;
  int s;

  get_image_size(self, &wd, &ht);

  get_palette_gs(self, &_plt, &n);
  plt  = (uint8_t*)RSTRING_PTR(_plt);

  ret  = rb_str_buf_new(wd  * ht);
  rb_str_set_len(ret, wd * ht);

  dst  = (uint8_t*)RSTRING_PTR(ret) + (wd * (ht - 1));

  rsz  = (7 + wd) / 8;
  rsz += ROW_PAD_SIZE(rsz);
  _row = rb_str_buf_new(rsz);

  for (y = 0; y < ht; y++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), _row);

    row = (uint8_t*)RSTRING_PTR(_row);

    for (x = 0; x < wd; x++) {
      i   = x / 8;
      s   = 7 - (x % 8);

      dst[x] = plt[(row[i] >> s) & 0x01];
    }

    dst -= wd;
  }

  RB_GC_GUARD(_plt);
  RB_GC_GUARD(_row);

  return ret;
}

static VALUE
rb_bmp_extract_grayscale4(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  VALUE _plt;
  uint8_t* plt;

  uint8_t* dst;
  size_t n;

  size_t rsz;
  VALUE _row;
  uint8_t* row;

  int x;
  int y;

  int i;
  int s;

  get_image_size(self, &wd, &ht);

  get_palette_gs(self, &_plt, &n);
  plt  = (uint8_t*)RSTRING_PTR(_plt);

  ret  = rb_str_buf_new(wd  * ht);
  rb_str_set_len(ret, wd * ht);

  dst  = (uint8_t*)RSTRING_PTR(ret) + (wd * (ht - 1));

  rsz  = (1 + wd) / 2;
  rsz += ROW_PAD_SIZE(rsz);
  _row = rb_str_buf_new(rsz);

  for (y = 0; y < ht; y++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), _row);

    row = (uint8_t*)RSTRING_PTR(_row);

    for (x = 0; x < wd; x++) {
      i   = x / 2;
      s   = (x & 1)? 4: 0;

      dst[x] = plt[(row[i] >> s) & 0x0f];
    }

    dst -= wd;
  }

  RB_GC_GUARD(_plt);
  RB_GC_GUARD(_row);

  return ret;

}

static VALUE
rb_bmp_extract_grayscale8(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  VALUE _plt;
  uint8_t* plt;

  uint8_t* dst;
  size_t n;

  size_t rsz;
  VALUE _row;
  uint8_t* row;

  int x;
  int y;

  get_image_size(self, &wd, &ht);

  get_palette_gs(self, &_plt, &n);
  plt  = (uint8_t*)RSTRING_PTR(_plt);

  ret  = rb_str_buf_new(wd  * ht);
  rb_str_set_len(ret, wd * ht);

  dst  = (uint8_t*)RSTRING_PTR(ret) + (wd * (ht - 1));

  rsz  = wd + ROW_PAD_SIZE(wd);
  _row = rb_str_buf_new(rsz);

  for (y = 0; y < ht; y++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), _row);

    row = (uint8_t*)RSTRING_PTR(_row);

    for (x = 0; x < wd; x++) {
      dst[x] = plt[row[x]];
    }

    dst -= wd;
  }

  RB_GC_GUARD(_plt);
  RB_GC_GUARD(_row);

  return ret;
}

static VALUE
rb_bmp_extract_grayscale24(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  int cr;
  int cg;
  int cb;

  size_t rsz;
  VALUE row;

  uint8_t *src;
  uint8_t *dst;

  int x;
  int y;

  get_image_size(self, &wd, &ht);
  get_gs_coeff(self, &cr, &cg, &cb);

  ret  = rb_str_buf_new(wd * ht);
  rb_str_set_len(ret, wd * ht);

  dst  = (uint8_t*)RSTRING_PTR(ret) + (wd * (ht - 1));

  rsz  = wd * 3;
  rsz += ROW_PAD_SIZE(rsz);
  row  = rb_str_buf_new(rsz);

  for (y = 0; y < ht; y ++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), row);

    src = (uint8_t*)RSTRING_PTR(row);

    for (x = 0; x < wd; x++){
      dst[x] = (((src[2] * cr) + (src[1] * cg) + (src[0] * cb)) >> 10) & 0xff;

      src += 3;
    }

    dst -= wd;
  }

  return ret;
}


static VALUE
rb_bmp_extract_grayscale32(VALUE self, VALUE io)
{
  VALUE ret;
  int wd;
  int ht;

  int cr;
  int cg;
  int cb;

  size_t rsz;
  VALUE row;

  uint8_t *src;
  uint8_t *dst;

  int x;
  int y;

  get_image_size(self, &wd, &ht);
  get_gs_coeff(self, &cr, &cg, &cb);

  ret  = rb_str_buf_new(wd * ht);
  rb_str_set_len(ret, wd * ht);

  dst  = (uint8_t*)RSTRING_PTR(ret) + (wd * (ht - 1));

  rsz  = wd * 4;
  rsz += ROW_PAD_SIZE(rsz);
  row  = rb_str_buf_new(rsz);

  for (y = 0; y < ht; y ++) {
    rb_funcall(io, rb_intern("read"), 2, INT2FIX(rsz), row);

    src = (uint8_t*)RSTRING_PTR(row);

    for (x = 0; x < wd; x++){
      dst[x] = (((src[2] * cr) + (src[1] * cg) + (src[0] * cb)) >> 10) & 0xff;

      src += 4;
    }

    dst -= wd;
  }

  return ret;
}

void
Init_bmp() 
{
  klass = rb_define_class("BMP", rb_cObject);

  rb_define_private_method(klass, "extract_image1",
                           rb_bmp_extract_image1, 1);

  rb_define_private_method(klass, "extract_image4",
                           rb_bmp_extract_image4, 1);

  rb_define_private_method(klass, "extract_image8",
                           rb_bmp_extract_image8, 1);

  rb_define_private_method(klass, "extract_image24",
                           rb_bmp_extract_image24, 1);

  rb_define_private_method(klass, "extract_image32",
                           rb_bmp_extract_image32, 1);

  rb_define_private_method(klass, "extract_grayscale1",
                           rb_bmp_extract_grayscale1, 1);

  rb_define_private_method(klass, "extract_grayscale4",
                           rb_bmp_extract_grayscale4, 1);

  rb_define_private_method(klass, "extract_grayscale8",
                           rb_bmp_extract_grayscale8, 1);

  rb_define_private_method(klass, "extract_grayscale24",
                           rb_bmp_extract_grayscale24, 1);

  rb_define_private_method(klass, "extract_grayscale32",
                           rb_bmp_extract_grayscale32, 1);
}
