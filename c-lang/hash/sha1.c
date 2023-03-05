/*
 * SHA1 sample
 *
 *  Copyright (C) 2023 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>

#include "sha1.h"

#define DEFAULT_ERROR   (__LINE__)
#define N(x)            (sizeof(x) / sizeof(*x))

#define ALLOC(t)        ((t*)malloc(sizeof(t)))

#define R_MASK(n)       (((1 << (n)) - 1) & 0xffffffff)
#define L_MASK(n)       (~R_MASK(n))
#if 0
#define ROTL(v,n) \
    ((((v) << (n)) & L_MASK(n)) | (((v) >> (32 - (n))) & R_MASK(n)))
#else
#define ROTL(v,n)       (((v) << (n)) | ((v) >> (32 - (n))))
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define BLK0(i)         block->l[i]
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define BLK0(i) \
    (block.l[i] = (ROTL(block.l[i],24) & 0xff00ff00)|\
                  (ROTL(block.l[i], 8) & 0x00ff00ff))
#else
#error "not supported byte order"
#endif

#define I13(i)         (((i) + 13) & 0x0f)
#define I8(i)          (((i) +  8) & 0x0f)
#define I2(i)          (((i) +  2) & 0x0f)
#define I0(i)          (((i) +  0) & 0x0f)

#define BLK_L13(i)     (block.l[I13(i)])
#define BLK_L8(i)      (block.l[I8(i)])
#define BLK_L2(i)      (block.l[I2(i)])
#define BLK_L0(i)      (block.l[I0(i)])

#define BLK(i) \
    (BLK_L0(i) = ROTL(BLK_L13(i) ^ BLK_L8(i) ^ BLK_L2(i) ^ BLK_L0(i), 1))

#define R0(v,w,x,y,z,i) {\
    z += ((w & (x ^ y)) ^ y) + BLK0(i) + 0x5a827999 + ROTL(v, 5); \
    w = ROTL(w, 30); \
}

#define R1(v,w,x,y,z,i) {\
    z += ((w & (x ^ y)) ^ y) + BLK(i) + 0x5a827999 + ROTL(v, 5); \
    w = ROTL(w, 30); \
}

#define R2(v,w,x,y,z,i) {\
    z += (w ^ x ^ y) + BLK(i) + 0x6ed9eba1 + ROTL(v, 5); \
    w = ROTL(w, 30); \
}

#define R3(v,w,x,y,z,i) { \
    z += (((w | x) & y) | (w & x)) + BLK(i) + 0x8f1bbcdc + ROTL(v, 5); \
    w = ROTL(w, 30); \
}

#define R4(v,w,x,y,z,i) { \
    z += (w ^ x ^ y) + BLK(i) + 0xca62c1d6 +ROTL(v, 5); \
    w = ROTL(w, 30); \
}

/**
 * @brief
 *  sha1_tの実体
 */
struct __sha1_t__ {
  uint32_t state[5];
  uint32_t count[2];
  uint8_t buf[64];
};

/*
 * declar internal functions
 */

static void
reset(sha1_t* ptr)
{
  ptr->state[0] = 0x67452301;
  ptr->state[1] = 0xefcdab89;
  ptr->state[2] = 0x98badcfe;
  ptr->state[3] = 0x10325476;
  ptr->state[4] = 0xc3d2e1f0;

  ptr->count[0] = 0;
  ptr->count[1] = 0;

  memset(ptr->buf, 0, sizeof(ptr->buf));
}

void
transform(uint32_t state[5], const uint8_t buf[64])
{
  uint32_t a, b, c, d, e;

  union {
    uint8_t c[64];
    uint32_t l[16];
  } block;

  memcpy(&block, buf, 64);

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];

  R0(a, b, c, d, e,  0);
  R0(e, a, b, c, d,  1);
  R0(d, e, a, b, c,  2);
  R0(c, d, e, a, b,  3);
                    
  R0(b, c, d, e, a,  4);
  R0(a, b, c, d, e,  5);
  R0(e, a, b, c, d,  6);
  R0(d, e, a, b, c,  7);

  R0(c, d, e, a, b,  8);
  R0(b, c, d, e, a,  9);
  R0(a, b, c, d, e, 10);
  R0(e, a, b, c, d, 11);

  R0(d, e, a, b, c, 12);
  R0(c, d, e, a, b, 13);
  R0(b, c, d, e, a, 14);
  R0(a, b, c, d, e, 15);

  R1(e, a, b, c, d, 16);
  R1(d, e, a, b, c, 17);
  R1(c, d, e, a, b, 18);
  R1(b, c, d, e, a, 19);

  R2(a, b, c, d, e, 20);
  R2(e, a, b, c, d, 21);
  R2(d, e, a, b, c, 22);
  R2(c, d, e, a, b, 23);

  R2(b, c, d, e, a, 24);
  R2(a, b, c, d, e, 25);
  R2(e, a, b, c, d, 26);
  R2(d, e, a, b, c, 27);

  R2(c, d, e, a, b, 28);
  R2(b, c, d, e, a, 29);
  R2(a, b, c, d, e, 30);
  R2(e, a, b, c, d, 31);

  R2(d, e, a, b, c, 32);
  R2(c, d, e, a, b, 33);
  R2(b, c, d, e, a, 34);
  R2(a, b, c, d, e, 35);

  R2(e, a, b, c, d, 36);
  R2(d, e, a, b, c, 37);
  R2(c, d, e, a, b, 38);
  R2(b, c, d, e, a, 39);

  R3(a, b, c, d, e, 40);
  R3(e, a, b, c, d, 41);
  R3(d, e, a, b, c, 42);
  R3(c, d, e, a, b, 43);

  R3(b, c, d, e, a, 44);
  R3(a, b, c, d, e, 45);
  R3(e, a, b, c, d, 46);
  R3(d, e, a, b, c, 47);

  R3(c, d, e, a, b, 48);
  R3(b, c, d, e, a, 49);
  R3(a, b, c, d, e, 50);
  R3(e, a, b, c, d, 51);

  R3(d, e, a, b, c, 52);
  R3(c, d, e, a, b, 53);
  R3(b, c, d, e, a, 54);
  R3(a, b, c, d, e, 55);

  R3(e, a, b, c, d, 56);
  R3(d, e, a, b, c, 57);
  R3(c, d, e, a, b, 58);
  R3(b, c, d, e, a, 59);

  R4(a, b, c, d, e, 60);
  R4(e, a, b, c, d, 61);
  R4(d, e, a, b, c, 62);
  R4(c, d, e, a, b, 63);

  R4(b, c, d, e, a, 64);
  R4(a, b, c, d, e, 65);
  R4(e, a, b, c, d, 66);
  R4(d, e, a, b, c, 67);

  R4(c, d, e, a, b, 68);
  R4(b, c, d, e, a, 69);
  R4(a, b, c, d, e, 70);
  R4(e, a, b, c, d, 71);

  R4(d, e, a, b, c, 72);
  R4(c, d, e, a, b, 73);
  R4(b, c, d, e, a, 74);
  R4(a, b, c, d, e, 75);

  R4(e, a, b, c, d, 76);
  R4(d, e, a, b, c, 77);
  R4(c, d, e, a, b, 78);
  R4(b, c, d, e, a, 79);

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;

  // a = b = c = d = e =0;
}

/*
 * declar global functions
 */

int
sha1_new(sha1_t** dst)
{
  int ret;
  sha1_t* obj;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;

  /*
   * argument check
   */
  if (dst == NULL) ret = DEFAULT_ERROR;

  /*
   * memory allocate
   */
  if (!ret) {
    obj = ALLOC(sha1_t);
    if (obj == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * setup object
   */
  if (!ret) reset(obj);

  /*
   * put return paramter
   */
  if (!ret) *dst = obj;
  
  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
  }

  return ret;
}

int
sha1_destroy(sha1_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * release resources
   */
  if (!ret) free(ptr);

  /*
   * post process
   */
  // nothing

  return ret;
}

int
sha1_update(sha1_t* ptr, void* data, size_t size)
{
  int ret;
  int i;
  int j;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (data == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * update process
   */
  if (!ret) {
    j = (ptr->count[0] >> 3) & 63;

    ptr->count[0] += (size << 3);
    ptr->count[1] += (size >> 29);

    if (ptr->count[0] < (size << 3)) ptr->count[1]++;

    if ((j + size) > 63) {
      i = 64 - j;

      memcpy(ptr->buf + j, data, i);
      transform(ptr->state, ptr->buf);

      for (; (i + 63) < size; i += 64) transform(ptr->state, data + i);

      j = 0;

    } else {
      i = 0;
    }

    memcpy(ptr->buf + j, data + i, size - i);
  }

  /*
   * post process
   */
  // nothing

  return ret;
}

int
sha1_final(sha1_t* ptr, sha1_output_t dst)
{
  int ret;
  int i;
  int j;
  int k;
  uint8_t fc[8];  // as Final Count

  const uint8_t c80[1] = {0x80};
  const uint8_t c00[1] = {0x00};

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * do padding
   */
  if (!ret) {
    for (i = 0; i < 8; i++) {
      j = (i >= 4)? 0: 1;
      k = (3 - (i & 3)) * 8;

      fc[i] = (uint8_t)((ptr->count[j] >> k) & 0xff);
    }

    sha1_update(ptr, (void*)c80, 1);

    while ((ptr->count[0] & 0x1f8) != 0x1c0) {
      sha1_update(ptr, (void*)c00, 1);
    }

    sha1_update(ptr, fc, 8);
  }

  /*
   * put return parameter
   */
  if (!ret) {
    for (i = 0; i < SHA1_DIGEST_SIZE; i++) {
      j = i >> 2;
      k = (3 - (i & 3)) * 8;

      dst[i] = (uint8_t)((ptr->state[j] >> k) & 0xff);
    }
  }

  /*
   * post process
   */
  if (!ret) reset(ptr);

  return ret;
}

int
sha1(void* data, size_t size, sha1_output_t dst)
{
  int ret;
  int err;
  sha1_t* dig;

  /*
   * initialize
   */
  ret = 0;
  dig = NULL;

  /*
   * argument check
   */
  if (data == NULL) ret = DEFAULT_ERROR;

  /*
   * create calculate context
   */
  if (!ret) {
    err = sha1_new(&dig);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * input data stream
   */
  if (!ret) {
    err = sha1_update(dig, data, size);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * put return parameter
   */
  if (!ret) {
    err = sha1_final(dig, dst);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * post process
   */
  if (dig != NULL) sha1_destroy(dig);

  return ret;
}

#if 0
#include <stdio.h>

void
print_digest(sha1_output_t src)
{
  int i;

  for (i = 0; i < SHA1_DIGEST_SIZE; i++) printf("%02x", src[i]);
  printf("\n");
}

int
main(int argc, char* argv[])
{
  sha1_t* dig;
  int i;
  sha1_output_t hash;

  /*
   * test vector (from FIPS PUB 180-1)
   *
   * "abc" 
   *   -> a9993e364706816aba3e25717850c26c9cd0d89d
   *
   * "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
   *   -> 84983e441c3bd26ebaae4aa1f95129e5e54670f1
   *
   * million repetitions of "a"
   *   -> 34aa973cd4c4daa4f61eeb2bdbad27316534016f
   */

  sha1("abc", 3, hash);
  print_digest(hash);

  sha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56, hash);
  print_digest(hash);

  sha1_new(&dig);
  for (i = 0; i < 1000000; i++) sha1_update(dig, (void*)"a", 1);
  sha1_final(dig, hash);
  sha1_destroy(dig);
  print_digest(hash);
}
#endif
