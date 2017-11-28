#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <signal.h>
#include <fd_map.hpp>
#include <tcp_fd.hpp>

#define POLLIN		0x0001
#define POLLPRI		0x0002
#define POLLOUT		0x0004
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020

struct pollfd {
   int   fd;         /* file descriptor */
   short events;     /* requested events */
   short revents;    /* returned events */
};
typedef unsigned long nfds_t;

extern "C"
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
  return 0;
  printf("poll(%p, %lu, %d)\n", fds, nfds, timeout);

  if (fds == nullptr) return -1;
  if (timeout < 0) return -1;

  bool still_waiting = true;
  do {
    for (int i = 0; i < nfds; i++)
    {
      auto& pfd = fds[i];
      printf("polling %d for events %d\n", pfd.fd, pfd.events);
      /* code */

    }
    break;
  } while(still_waiting);
  return 0;
}
