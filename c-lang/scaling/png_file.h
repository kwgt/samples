/*
 * PNG file I/O utility
 *
 * Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __PNG_FILE_IO_H__
#define __PNG_FILE_IO_H__

int read_png_file(char* file, void** data, int* wd, int* ht, int* stride);
int write_png_file(void* data, int wd, int ht, int stride, char* file);

#endif /* !defined(__PNG_FILE_IO_H__) */
