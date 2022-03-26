
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define DEFAULT_ERROR __LINE__

static uint8_t
code_table[] = {
  /* 0x00 - 0x74 */
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80,

  /* '+' */
  0x3e,

  /* 0x76 - 0x78 */
  0x80, 0x80, 0x80,

  /* '/' */
  0x3f,

  /* '0' - '9' */
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
  0x3c, 0x3d,

  /* 0x3A - 0x40 */
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,

  /* 'A' - 'Z' */
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19,

  /* 0x5B - 0x60 */
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80,

  /* 'a' - 'z' */
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
  0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
  0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31,
  0x32, 0x33,

  /* 0x7B - 0xFF */
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80,
};

static int
count64(char* str, size_t len, size_t* dst)
{
  int ret;
  int st;
  char ch;
  int i;
  size_t l1;
  size_t l2;

  /*
   * initialize
   */
  ret = 0;
  st  = 0;
  l1  = 0;
  l2  = 0;

  /*
   * argument check
   */
  do {
    if (str == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * count base64 charactors
   */
  if (!ret) {
    for (i = 0; i < (int)len; i++) {
      ch = str[i];

      switch (st) {
      case 0:
        if (!(code_table[(int)ch] & 0x80)) {
          l1++;

        } else if (isspace(ch)) {
          // 空白文字の場合は無視
 
        } else if (ch == '=') {
          // 末尾のパディングを検出
          st = 1;
          l2++;

        } else {
          // Base64に使用できない文字が見つかった場合
          ret = DEFAULT_ERROR;
          goto loop_out;
        }
        break;

      case 1:
        if (ch == '=') {
          if (++l2 > 3) {
            // 末尾のパディングが三文字を超える場合
            ret = DEFAULT_ERROR;
            break;
          }

        } else if (isspace(ch)) {
          // 空白文字の場合は無視
 
        } else {
          // 末尾のパディングに余計な文字が入った場合
          ret = DEFAULT_ERROR;
          goto loop_out;
        }
        break;
      }
    }
  }
  loop_out:

  if (!ret) {
    // Base64構成文字数が4の倍数でなかった場合
    if ((l1 + l2) % 4 != 0) ret = DEFAULT_ERROR;
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = l1;

  /*
   * post process
   */
  // nothing

  return ret;
}


int
decode64(char* src, size_t ssz, void** _dst, size_t* _dsz)
{
  int ret;
  int err;
  void* dst;
  size_t dsz;
  size_t rem;

  int i;
  int j;
  uint8_t* p;
  char ch;
  uint8_t v;

  /*
   * initialize
   */
  ret = 0;
  dst = NULL;
  rem = 0;

  /*
   * argument check
   */
  do {
    if (src == NULL) {
      ret = DEFAULT_ERROR; 
      break;
    }

    if (_dst == NULL) {
      ret = DEFAULT_ERROR; 
      break;
    }
  } while (0);

  /*
   * check input data
   */
  if (!ret) do {
    if (ssz == 0) ssz = strlen(src);

    if (ssz < 4) {
      ret = DEFAULT_ERROR; 
      break;
    }

    err = count64(src, ssz, &rem);
    if (err) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * memory allocaiton
   */
  if (!ret) {
    dsz  = (rem * 3) / 4;
    dst  = malloc(dsz);

    if (dst == NULL) {
      ret = DEFAULT_ERROR; 
    }
  }

  /*
   * try convert
   */
  if (!ret) {
    memset(dst, 0, dsz);

    for (p = dst, i = 0, j = 0; j < rem; i++) {
      ch = (uint8_t)src[i];

      if (isspace(ch)) continue;

      v  = code_table[(int)ch];

      switch (j++ % 4) {
      case 0:
        p[0] |= (v << 2) & 0xfc;
        break;

      case 1:
        p[0] |= (v >> 4) & 0x03;
        p[1] |= (v << 4) & 0xf0;
        break;

      case 2:
        p[1] |= (v >> 2) & 0x0f;
        p[2] |= (v << 6) & 0xc0;
        break;

      case 3:
        p[2] |= (v << 0) & 0x3f;
        p += 3;
        break;
      }
    }
  }

  /*
   * put return parameter
   */
  if (!ret) {
    *_dst = dst;
    if (_dsz) *_dsz = dsz;
  }

  /*
   * post process
   */
  if (ret) {
    if (dst != NULL) free(dst);
  }

  return ret;
}

#if 0
void
test(char *src)
{
  void* buf;
  size_t len;

  decode64(src, 0, &buf, &len);
  printf("%s => ", src);
  fwrite(buf, len, 1, stdout);
  fprintf(stdout, "\n");

  free(buf);
}


int
main(int argc, char* argv[])
{
  printf("%zu\n", sizeof(code_table));
  test("QQ==");
  test("QUE=");
  test("YWJj");
  test("YWJjZA==");
  test("Y WJ  jZ\nA= =");

  return 0;
}
#endif
