#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "util.h"

#define DEFAULT_ERROR     __LINE__

int
read_file(char* path, void** _ptr, size_t* _size)
{
  int ret;
  int err;
  struct stat st;
  void* ptr;
  FILE* fp;
 
  /*
   * initialize
   */
  ptr = NULL;
  fp  = NULL;
 
  do {
    /*
     * entry process
     */
    ret = !0;
 
    /*
     * get file size
     */
    err = stat(path, &st);
    if (err < 0) break;
 
    /*
     * alloc memory
     */
    ptr = malloc(st.st_size);
    if (ptr == NULL) break;
 
    /*
     * open file
     */
    fp = fopen(path, "rb");
    if (fp == NULL) break;
 
    /*
     * read file
     */
    err = fread(ptr, st.st_size, 1, fp);
    if (err != 1) break;
 
    /*
     * set return parameter
     */
    *_ptr  = ptr;
    *_size = st.st_size;
 
    /*
     * mark success
     */
    ret = 0;
  } while(0);
 
  /*
   * post process
   */
  if (ret) {
    if (ptr != NULL) free(ptr);
  }
 
  if(fp != NULL) fclose(fp);
 
  return ret;
}

int
write_ppm(char* path, int width, int height, void* plane, char* comm)
{
  int ret;
  int err;
  FILE* fp;

  /*
   * initialize
   */
  ret = 0;
  fp  = NULL;

  /*
   * argument check
   */
  do {
    if (path == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (width < 16) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (height < 16) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (plane == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while(0);

  /*
   * open target file
   */
  if (!ret) {
    fp = fopen(path, "wb");
    if (fp == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * put data
   */
  if (!ret) do {
    if (comm == NULL) comm = "";

    err = fprintf(fp, "P6\n# %s\n%d %d\n255\n", comm, width, height);
    if (err < 0) {
      ret = DEFAULT_ERROR;
      break;
    }

    err = fwrite(plane, width * height * 3, 1, fp);
    if (err != 1) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * post process
   */
  if (fp) fclose(fp);

  return ret;
}
