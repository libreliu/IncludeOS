// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2016 Oslo and Akershus University College of Applied Sciences
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

#include <service>
#include <os>
#include <memdisk>
#include <fs/vfs.hpp>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <lest/lest.hpp>

fs::Disk_ptr& memdisk() {
  static auto disk = fs::new_shared_memdisk();

  if (not disk->fs_ready()) {
    printf("%s\n", disk->name().c_str());
    disk->init_fs([](fs::error_t err) {
        if (err) {
          printf("ERROR MOUNTING DISK\n");
          exit(127);
        }
      });
    }
  return disk;
}

const lest::test specification[] =
{
  {
    CASE("open() existing file for reading returns valid file descriptor")
    {
      int fd = open("/mnt/disk/file1", O_RDONLY);
      EXPECT(fd != -1);
      int res = close(fd);
      EXPECT(res != -1);
    }
  },
  {
    CASE("open() non-existing file for reading returns error, errno set to ENOENT")
    {
      int res = open("/mnt/disk/file666", O_RDONLY);
      EXPECT(res == -1);
      EXPECT(errno == ENOENT);
    }
  },
  {
    CASE("lseek() on open FD returns file offset")
    {
      int fd = open("/mnt/disk/file1", O_RDONLY);
      EXPECT(fd != -1);
      // get current offset
      off_t offset = lseek(fd, 0, SEEK_CUR);
      EXPECT(offset == 0);

      // seek from end
      struct stat sb;
      int res = stat("/mnt/disk/file1", &sb);
      EXPECT(res != -1);
      int size = sb.st_size;
      offset = lseek(fd, 0, SEEK_END);
      EXPECT(offset == size);

      // we can seek beyond end
      offset = lseek(fd, 100, SEEK_END);
      EXPECT(offset > size);

      // but not before beginning
      offset = lseek(fd, 0, SEEK_SET);
      EXPECT(offset == 0);
      offset = lseek(fd, -100, SEEK_CUR);
      EXPECT(offset == 0);

      res = close(fd);
      EXPECT(res != -1);
    }
  },
  {
    CASE("lseek on non-open FD returns -1, errno set to EBADF")
    {
      int fd = open("/mnt/disk/file666", O_RDONLY);
      EXPECT(fd == -1);
      off_t offset = lseek(fd, 0, SEEK_SET);
      EXPECT(offset == -1);
      EXPECT(errno == EBADF);
    }
  },
  {
    CASE("read() reads requested number of bytes from open fd")
    {
      int fd = open("/mnt/disk/file1", O_RDONLY);
      EXPECT(fd != -1);

      struct stat sb;
      int res = stat("/mnt/disk/file1", &sb);
      EXPECT(res != -1);
      int size = sb.st_size;

      char* buffer = (char*) malloc(1024);
      memset(buffer, 0, 1024);
      int bytes = read(fd, buffer, 1024);
      EXPECT(bytes == size);
      free(buffer);
    }
  },
  {
    CASE("close() closes file desctiptors")
    {
      int fd = open("/mnt/disk/file1", O_RDONLY);
      EXPECT(fd != -1);
      int res = close(fd);
      EXPECT(res != -1);

      // check that fd is closed
      char* buffer = (char*) malloc(1024);
      memset(buffer, 0, 1024);
      int bytes = read(fd, buffer, 1024);
      EXPECT(bytes == -1);
      EXPECT(errno == EBADF);
    }
  }
};

int main()
{
  INFO("POSIX file_fd", "Running tests for POSIX file_fd");

  // mount a disk with contents for testing
  auto root = memdisk()->fs().stat("/");
  fs::mount("/mnt/disk", root, "test root");

  auto failed = lest::run(specification, {"-p"});
  Expects(not failed);

  INFO("POSIX file_fd", "All done!");
  exit(0);
}
