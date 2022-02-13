#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DEFAULT_ERROR     (__LINE__)

int
encode64(void* src, size_t sz, char* prefix, char** dst)
{
  int ret;
  char* str;
  uint8_t* ps;  // as "Pointer to Source side"
  char* pd;     // as "Pointer to Destination side"
  uint32_t v24;

  size_t pl;    // as "Prefix Length"

  static const char tbl[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
  };

  /*
   * initialize
   */
  ret = 0;
  str = NULL;

  /*
   * argument check
   */
  do {
    if (src == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (sz <= 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * memory allocation
   */
  if (!ret) {
    pl  = (prefix == NULL)? 0: strlen(prefix);
    str = malloc(pl + (((sz / 3) * 4) + 4) + 1);
    if (str == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * put prefix
   */
  if (!ret) {
    ps = src;
    pd = str;

    if (prefix != NULL) {
      strcpy(pd, prefix);
      pd += pl;
    }
  }

  /*
   * do encode
   */
  if (!ret) {
    while (sz >= 3) {
      v24 = (ps[0] << 16) | (ps[1] << 8) | (ps[2] << 0);

      pd[0] = tbl[(v24 >> 18) & 0x3f];
      pd[1] = tbl[(v24 >> 12) & 0x3f];
      pd[2] = tbl[(v24 >>  6) & 0x3f];
      pd[3] = tbl[(v24 >>  0) & 0x3f];

      ps += 3;
      pd += 4;
      sz -= 3;
    }

    switch (sz) {
    case 2:
      v24 = (ps[0] << 16) | (ps[1] << 8);
      pd[0] = tbl[(v24 >> 18) & 0x3f];
      pd[1] = tbl[(v24 >> 12) & 0x3f];
      pd[2] = tbl[(v24 >>  6) & 0x3f];
      pd[3] = '=';
      pd += 4;
      break;

    case 1:
      v24 = (ps[0] << 16);
      pd[0] = tbl[(v24 >> 18) & 0x3f];
      pd[1] = tbl[(v24 >> 12) & 0x3f];
      pd[2] = '=';
      pd[3] = '=';
      pd += 4;
      break;
    }

    *pd = '\0';
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = str;

  /*
   * post process
   */
  if (ret) {
    if (str != NULL) free(str);
  }

  return ret;
}

#if 1
int
main(int argc, char* argv[])
{
  char* s;
  int err;

  s   = NULL;
  err = encode64("ABCDEFG", 7, NULL, &s);
  printf("err = %d, %s\n", err, s);

  if (s != NULL) free(s);

  s   = NULL;
  err = encode64("ABCDEFG", 7, "data:image/jpeg;", &s);
  printf("err = %d, %s\n", err, s);

  if (s != NULL) free(s);

  return 0;
}
#endif
