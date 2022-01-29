#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/timerfd.h>

#include "cronog.h"

#define DEFAULT_ERROR         __LINE__

int to_exit = 0;

void
sig_hdr(int sig)
{
  to_exit = !0;
}

int
main(int argc, char* argv[])
{
  int ret;
  int err;
  int fd;
  cronog_t* cro;
  int64_t dura;
  struct itimerspec spec;

  ret = 0;
  fd  = -1;
  cro = NULL;

  do {
    signal(SIGINT, sig_hdr);
    signal(SIGTERM, sig_hdr);

    err = cronog_new(&cro); 
    if (err) {
      fprintf(stderr, "cronog_new() failed [err=%d]\n", err);
      break;
    }

    fd  = timerfd_create(CLOCK_REALTIME, 0);
    if (fd < 0) {
      fprintf(stderr, "%s\n", strerror(errno));
      break;
    }

    spec.it_value.tv_sec     = 5;
    spec.it_value.tv_nsec    = 0;
    spec.it_interval.tv_sec  = 2;
    spec.it_interval.tv_nsec = 0;

    err = timerfd_settime(fd, 0, &spec, NULL);
    if (err < 0) {
      fprintf(stderr, "%s\n", strerror(errno));
      break;
    }

    while (!to_exit) {
      uint64_t skip; 

      /*
      cronog_start(cro);
      read(fd, &skip, sizeof(skip));
      cronog_stop(cro);
      cronog_result(cro, &dura);

      printf("%llu %lld\n", skip, dura);

      cronog_reset(cro);
      */

      cronog_measure(cro, &dura) {
        read(fd, &skip, sizeof(skip));
      }

      printf("%llu %lld\n", skip, dura);
    }
  } while (0);

  if (cro != NULL) cronog_destroy(cro);
  if (fd >= 0) close(fd);

  return ret;
}
