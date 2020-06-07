/*
 * Funcitons for time measurement
 *
 *  Copyright (C) 2020 Hiroshi Kuwagata <kgt9221@gmail.com>
 */
#ifndef __CRONOG_T__
#define __CRONOG_T__

#include <stdint.h>
#include <time.h>

typedef struct {
  int flags;
  struct timespec ts0;
  struct timespec ts;
} cronog_t;

int cronog_new(cronog_t** dst);
int cronog_destroy(cronog_t* ptr);
int cronog_start(cronog_t* ptr);
int cronog_stop(cronog_t* ptr);
int cronog_reset(cronog_t* ptr);
int cronog_result(cronog_t* ptr, int64_t* dst);
#endif /* !defined(__CRONOG_T__) */
