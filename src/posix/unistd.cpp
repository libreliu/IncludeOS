// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fcntl.h>
#include <unistd.h>
#include <fd_map.hpp>
#include <kernel/os.hpp>

int open(const char*, int, ...)
{
  return -1;
}

int close(int fildes)
{
  try {
    auto& fd = FD_map::_get(fildes);
    // if close success
    if(fd.close() == 0)
      return FD_map::_close(fildes);
  }
  catch(const FD_not_found&) {
    errno = EBADF;
  }
  return -1;
}

int read(int, void*, size_t)
{
  return 0;
}
int write(int file, const void* ptr, size_t len)
{
  return OS::print((const char*) ptr, len);
}

int fsync(int fildes)
{
  try {
    auto& fd = FD_map::_get(fildes);
    // files should return 0, and others should not
    return 0;
  }
  catch(const FD_not_found&) {
    errno = EBADF;
    return -1;
  }
}

int fchown(int fd, uid_t owner, gid_t group)
{
  return 0;
}

#include <kernel/irq_manager.hpp>
#include <kernel/rtc.hpp>
unsigned int sleep(unsigned int seconds)
{
  int64_t now  = RTC::now();
  int64_t done = now + seconds;
  while (true) {
    if (now >= done) break;
    OS::halt();
    IRQ_manager::get().process_interrupts();
    now = RTC::now();
  }
  return 0;
}
