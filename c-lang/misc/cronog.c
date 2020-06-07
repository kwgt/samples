/*
 * Funcitons for time measurement
 *
 *  Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "cronog.h"

#define DEFAULT_ERROR         __LINE__
#define ALLOC(t)              ((t*)malloc(sizeof(t)))

#define F_STARTED             0x0001
#define F_STOPPED             0x0002

#define SET_FLAG(ptr, name)    ((ptr)->flags |= F_##name)
#define TEST_FLAG(ptr, name)   ((ptr)->flags & F_##name)
#define RESET_FLAG(ptr)        ((ptr)->flags = 0)

int
cronog_new(cronog_t** dst)
{
  int ret;
  cronog_t* obj;

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
   * alloc memory
   */
  if (!ret) {
    obj = ALLOC(cronog_t);
    if (obj == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * setup context
   */
  if (!ret) {
    RESET_FLAG(obj);
    memset(&obj->ts0, 0, sizeof(obj->ts0));
    memset(&obj->ts, 0, sizeof(obj->ts));
  }

  /*
   * put return parameter
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
cronog_destroy(cronog_t* ptr)
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
   * release memory
   */
  if (!ret) free(ptr);

  return ret;
}

int
cronog_start(cronog_t* ptr)
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
   * check object state
   */
  if (!ret) {
    if (ptr->flags != 0) ret = DEFAULT_ERROR;
  }

  /*
   * get star time
   */
  if (!ret) {
    SET_FLAG(ptr, STARTED);
    clock_gettime(CLOCK_REALTIME, &ptr->ts0);
  }

  return ret;
}

int
cronog_stop(cronog_t* ptr)
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
   * check object state
   */
  if (!ret) {
    if (!TEST_FLAG(ptr, STARTED)) ret = DEFAULT_ERROR;
  }

  /*
   * get end time
   */
  if (!ret) {
    SET_FLAG(ptr, STOPPED);
    clock_gettime(CLOCK_REALTIME, &ptr->ts);
  }

  return ret;
}

int
cronog_reset(cronog_t* ptr)
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
   * reset state 
   */
  if (!ret) RESET_FLAG(ptr);

  return ret;
}

int
cronog_result(cronog_t* ptr, int64_t* dst)
{
  int ret;

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
   * check object state
   */
  if (!ret) {
    if (!(TEST_FLAG(ptr, STARTED) && TEST_FLAG(ptr, STOPPED))) {
      ret = DEFAULT_ERROR;
    }
  }

  /*
   * put return paramter
   */
  if (!ret) {
    *dst = ((ptr->ts.tv_sec - ptr->ts0.tv_sec) * 1000) +
           ((ptr->ts.tv_nsec - ptr->ts0.tv_nsec) / 1000000);
  }

  return ret;
}
