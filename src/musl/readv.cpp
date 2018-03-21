#include "stub.hpp"
#include <errno.h>
#include <sys/uio.h>

static ssize_t sys_readv(int /*fd*/, const struct iovec*, int /*iovcnt*/)
{
  return -ENOSYS;
}

extern "C"
ssize_t syscall_SYS_readv(int fd, const struct iovec *iov, int iovcnt)
{
  return stubtrace(sys_readv, "readv", fd, iov, iovcnt);
}
