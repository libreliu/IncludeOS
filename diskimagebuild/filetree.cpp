#include "filetree.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <libgen.h>
#include <sys/stat.h>
#include <dirent.h>

File::File(const char* path)
{
  this->name = std::string(path);

  FILE* f = fopen(path, "rb");
  if (f == nullptr)
    throw std::runtime_error("diskbuilder: Could not open file " + std::string(path));

  fseek(f, 0, SEEK_END);
  this->size = ftell(f);
  rewind(f);

  this->data = std::unique_ptr<char[]> (new char[size], std::default_delete<char[]> ());
  size_t actual = fread(this->data.get(), this->size, 1, f);
  if (actual != 1) {
    throw std::runtime_error("diskbuilder: Could not read from file " + std::string(path));
  }
  fclose(f);
}
Dir::Dir(const char* path)
{
  this->name = std::string(path);

  /// ... ///
}

void Dir::print(int level) const
{
  for (const Dir& d : subs)
  {
    printf ("Dir%*s ", level * 2, "");
    printf("[%u entries] %s\n",
          d.sectors_used(),
          d.name.c_str());

    d.print(level + 1);
  }
  for (const File& f : files)
  {
    printf("File%*s ", level * 2, "");
    printf("[%08u b -> %06u sect] %s\n",
          f.size,
          f.sectors_used(),
          f.name.c_str());
  }
}

uint32_t Dir::sectors_used() const
{
  uint32_t cnt = this->size_helper;

  for (const auto& dir : subs)
      cnt += dir.sectors_used();

  for (const auto& file : files)
      cnt += file.sectors_used();

  return cnt;
}

void FileSys::print() const
{
  root.print(0);
}
void FileSys::add_dir(Dir& dvec)
{
  // push work dir
  char pwd_buffer[256];
  getcwd(pwd_buffer, sizeof(pwd_buffer));
  // go into directory
  char cwd_buffer[256];
  getcwd(cwd_buffer, sizeof(cwd_buffer));
  strcat(cwd_buffer, "/");
  strcat(cwd_buffer, dvec.name.c_str());

  //printf("*** Entering %s...\n", cwd_buffer);
  int res = chdir(cwd_buffer);
  // throw immediately when unable to read directory
  if (res < 0) {
    fprintf(stderr, "Unable to enter directory %s\n", cwd_buffer);
    throw std::runtime_error("Unable to enter directory " + std::string(cwd_buffer));
  }

  auto* dir = opendir(cwd_buffer);
  // throw immediately when unable to open directory
  if (dir == nullptr) {
    fprintf(stderr, "Unable to open directory %s\n", cwd_buffer);
    throw std::runtime_error("Unable to open directory " + std::string(cwd_buffer));
  }
  struct dirent* ent;
  while ((ent = readdir(dir)) != nullptr)
  {
    std::string name(ent->d_name);
    if (name == ".." || name == ".") continue;

    if (ent->d_type == DT_DIR) {
      auto& d = dvec.add_dir(ent->d_name);
      add_dir(d);
    }
    else {
      try {
          dvec.add_file(ent->d_name);
      } catch (std::exception& e) {
          fprintf(stderr, "%s\n", e.what());
      }
    }
  }
  // pop work dir
  res = chdir(pwd_buffer);
  if (res < 0) {
    throw std::runtime_error("diskbuilder: Failed to return back to parent directory");
  }
}

void FileSys::gather(const char* path)
{
  root = Dir(path);
  add_dir(root);
}
