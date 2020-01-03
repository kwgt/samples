#ifndef __COMPAT_H__
#define __COMPAT_H__

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

#ifndef __cplusplus
#ifdef _MSC_VER
#define alignas(n)              __declspec(align(n))
#endif /* defined(_MSC_VER) */

#ifdef __GNUC__
#define alignas(n)              __attribute__((aligned(n)))
#endif /* defined(__GNUC__) */
#endif /* !defined(__cplusplus) */

#if defined(__MINGW32__) || defined(__MINGW64__)
#include <malloc.h>
#include <windows.h>
#include <intrin.h>
#define aligned_malloc          __mingw_aligned_malloc
#define aligned_free            __mingw_aligned_free
#endif /* defined(__MINGW) */

#ifdef __APPLE_CC__
#include <x86intrin.h>
#include <stdlib.h>
void* aligned_malloc(size_t align, size_t size);
void aligned_free(void* ptr);
#endif /* defined(__APPLE_CC__) */

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */
#endif /* !defined(__COMPAT_H__) */
