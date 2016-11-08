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
#include <memdisk>
#define __SPU__
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <ftw.h>

void print_stat(struct stat buffer);
int display_info(const char *fpath, const struct stat *sb, int flag, struct FTW *ftwbuf);

int main()
{
  int res;
  char* nullbuf = nullptr;
  char shortbuf[4];
  char buf[1024];
  struct stat buffer;

  res = stat("/FOLDER1", &buffer);
  printf("stat result: %d\n", res);
  if (res == -1)
  {
    printf("stat error: %s\n", strerror(errno));
  }
  else {
    print_stat(buffer);
  }

  res = chdir("FILE2");
  printf("chdir result (not a folder): %d\n", res);
  if (res == -1)
  {
    printf("chdir error: %s\n", strerror(errno));
  }

  res = chdir("FOLDER1");
  printf("chdir result (existing folder): %d\n", res);
  if (res == -1)
  {
    printf("chdir error: %s\n", strerror(errno));
  }

  res = chdir("/FOLDER1");
  printf("chdir result (existing folder, absolute): %d\n", res);
  if (res == -1)
  {
    printf("chdir error: %s\n", strerror(errno));
  }

  char* nullcwd = getcwd(nullptr, 0);
  printf("getcwd result (nullptr, size 0): %s\n", nullcwd == nullptr ? "NULL" : nullcwd);
  if (nullcwd == nullptr)
  {
    printf("getcwd error: %s\n", strerror(errno));
  }

  nullcwd = getcwd(nullptr, 1024);
  printf("getcwd result (nullptr): %s\n", nullcwd == nullptr ? "NULL" : nullcwd);
  if (nullcwd == nullptr)
  {
    printf("getcwd error: %s\n", strerror(errno));
  }

  char* shortcwd = getcwd(shortbuf, 4);
  printf("getcwd result (small buffer): %s\n", shortcwd == nullptr ? "NULL" : shortcwd);
  if (shortcwd == nullptr)
  {
    printf("getcwd error: %s\n", strerror(errno));
  }

  char* cwd = getcwd(buf, 1024);
  printf("getcwd result (adequate buffer): %s\n", cwd);
  if (cwd == nullptr)
  {
    printf("getcwd error: %s\n", strerror(errno));
  }

  res = chmod("/dev/null", S_IRUSR);
  printf("chmod result: %d\n", res);
  if (res == -1)
  {
    printf("chmod error: %s\n", strerror(errno));
  }

  int fd = STDOUT_FILENO;
  res = fchmod(fd, S_IWUSR);
  printf("fchmod result: %d\n", res);
  if (res == -1)
  {
    printf("fchmod error: %s\n", strerror(errno));
  }

  res = fchmodat(fd, "test", S_IRUSR, AT_SYMLINK_NOFOLLOW);
  printf("fchmodat result: %d\n", res);
  if (res == -1)
  {
    printf("fchmodat error: %s\n", strerror(errno));
  }

  res = fstat(fd, &buffer);
  printf("fstat result: %d\n", res);
  if (res == -1)
  {
    printf("fstat error: %s\n", strerror(errno));
  }

  res = fstatat(fd, "test", &buffer, AT_SYMLINK_NOFOLLOW);
  printf("fstatat result: %d\n", res);
  if (res == -1)
  {
    printf("fstatat error: %s\n", strerror(errno));
  }

  res = futimens(fd, nullptr);
  printf("futimens result: %d\n", res);
  if (res == -1)
  {
    printf("futimens error: %s\n", strerror(errno));
  }

  res = utimensat(fd, "test", nullptr, AT_SYMLINK_NOFOLLOW);
  printf("utimensat result: %d\n", res);
  if (res == -1)
  {
    printf("utimensat error: %s\n", strerror(errno));
  }

/*
  res = lstat("/", &buffer);
  printf("lstat result: %d\n", res);
  if (res == -1)
  {
    printf("lstat error: %s\n", strerror(errno));
  }
*/

  res = mkdir("/dev/sda1/root",  S_IWUSR);
  printf("mkdir result: %d\n", res);
  if (res == -1)
  {
    printf("mkdir error: %s\n", strerror(errno));
  }

  res = mkdirat(fd, "root",  S_IWUSR);
  printf("mkdirat result: %d\n", res);
  if (res == -1)
  {
    printf("mkdirat error: %s\n", strerror(errno));
  }

  res = mkfifo("/dev/null",  S_IWUSR);
  printf("mkfifo result: %d\n", res);
  if (res == -1)
  {
    printf("mkfifo error: %s\n", strerror(errno));
  }

  res = mkfifoat(AT_FDCWD, "test",  S_IWUSR);
  printf("mkfifoat result: %d\n", res);
  if (res == -1)
  {
    printf("mkfifoat error: %s\n", strerror(errno));
  }

/*
  res = mknod("/dev/null",  S_IWUSR, 0);
  printf("mknod result: %d\n", res);
  if (res == -1) {
    printf("mknod error: %s\n", strerror(errno));
  }
*/

  res = mknodat(AT_FDCWD, "test",  S_IWUSR, 0);
  printf("mknodat result: %d\n", res);
  if (res == -1) {
    printf("mknodat error: %s\n", strerror(errno));
  }

  mode_t old_umask = umask(0);
  printf("Old umask: %d\n", old_umask);

  res = nftw("/FOLDER3", display_info, 20, FTW_PHYS);
  printf("nftw result: %d\n", res);
  if (res == -1)
  {
    printf("nftw error: %s\n", strerror(errno));
  }

  printf("All done!\n");
  exit(0);
}

void print_stat(struct stat buffer)
{
    printf("st_dev: %d\n", buffer.st_dev);
    printf("st_ino: %hu\n", buffer.st_ino);
    printf("st_mode: %d\n", buffer.st_mode);
    printf("st_nlink: %d\n", buffer.st_nlink);
    printf("st_uid %d\n", buffer.st_uid);
    printf("st_gid: %d\n", buffer.st_gid);
    printf("st_rdev: %d\n", buffer.st_rdev);
    printf("st_size: %ld\n", buffer.st_size);
    printf("st_atime: %ld\n", buffer.st_atime);
    printf("st_ctime: %ld\n", buffer.st_ctime);
    printf("st_mtime: %ld\n", buffer.st_mtime);
    printf("st_blksize: %ld\n", buffer.st_blksize);
    printf("st_blocks: %ld\n", buffer.st_blocks);
}

int display_info(const char *fpath, const struct stat *sb, int flag, struct FTW *ftwbuf)
{
  printf("%ld\t%s (%d)\n", sb->st_size, fpath, flag);
  return 0;
}
