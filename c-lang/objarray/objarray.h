/*
 * Object array
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#ifndef __OBJECT_ARRAY_H__
#define __OBJECT_ARRAY_H__

typedef struct {
  void** t;
  int capa;
  int size;
}  objary_t;

int objary_new(objary_t** dst);
int objary_new2(int capa, objary_t** dst);
int objary_destroy(objary_t* ptr);

int objary_aset(objary_t* ptr, int i, void* obj);
int objary_aref(objary_t* ptr, int i, void** dst);
int objary_push(objary_t* ptr, void* obj);
int objary_pop(objary_t* ptr, void** dst);
int objary_shift(objary_t* ptr, void** dst);
int objary_remove(objary_t* ptr, int i, void** dst);

#define objary_len(a)       ((a)->size)

#define objary_each(a,i,o) \
  for ((i) = 0, (o) = (a)->t[0];(i) < (a)->size; o = (a)->t[++(i)])  

#endif /* !defined(__OBJECT_ARRAY_H__) */
