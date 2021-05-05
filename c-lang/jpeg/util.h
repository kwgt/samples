#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int read_file(char* path, void** _ptr, size_t* _size);
int write_ppm(char* path, int width, int height, void* plane, char* comm);
int write_png(char* path, int width, int height, void* plane);

#endif /* !defined(__UTIL_H__) */
