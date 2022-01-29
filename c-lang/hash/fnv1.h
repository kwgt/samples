#ifndef __FNV1_H__
#define __FNV1_H__
#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#undef API_SPEC
#ifdef DLL_EXPORT
#define API_SPEC                __declspec(dllexport)
#else /* defined(DLL_EXPORT) */
#define API_SPEC                __declspec(dllimport)
#endif /* defined(DLL_EXPORT) */
#else /* defined(WIN32) */
#define API_SPEC
#endif /* defined(WIN32) */

API_SPEC uint32_t fnv132(void* src, size_t size);
API_SPEC uint64_t fnv164(void* src, size_t size);

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */
#endif /* !defined(__FNV1_H__) */
