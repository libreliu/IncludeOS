#include "common.hpp"

#include <posix/fd_map.hpp>

static ssize_t sys_readv(int fd, const struct iovec* iov, int iovcnt)
{
  try {
    auto& fildes = FD_map::_get(fd);
    return fildes.readv(iov, iovcnt);
  }
  catch(const FD_not_found&) {
    return -EBADF;
  }
}

extern "C"
ssize_t syscall_SYS_readv(int fd, const struct iovec *iov, int iovcnt)
{
  return strace(sys_readv, "readv", fd, iov, iovcnt);
}
