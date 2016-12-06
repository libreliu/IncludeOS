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

#pragma once
#ifndef TAR_READER_HPP
#define TAR_READER_HPP

#include <posix/tar.h>  // our posix header has the Tar_header struct, which the newlib tar.h does not

#include <string>
#include <vector>
#include <stdexcept>

#include <util/tinf.h>

extern char _binary_input_bin_start;
extern uintptr_t _binary_input_bin_size;

const int SECTOR_SIZE = 512;

class Tar_exception : public std::runtime_error {
  using runtime_error::runtime_error;
};

struct Content_block {
  char block[SECTOR_SIZE];
};

// Element/file/folder in tarball
class Element {

public:
  Element(Tar_header& header)
    : header_{header} {}

  Element(Tar_header& header, const std::vector<Content_block*>& content)
    : header_{header}, content_{content} {}

  const Tar_header& header() const { return header_; }
  void set_header(const Tar_header& header) { header_ = header; }

  const std::vector<Content_block*>& content() const { return content_; }
  void add_content_block(Content_block* content_block) {
    content_.push_back(content_block);
  }

  std::string name() const { return std::string{header_.name}; }
  std::string mode() const { return std::string{header_.mode}; }
  std::string uid() const { return std::string{header_.uid}; }
  std::string gid() const { return std::string{header_.gid}; }
  long int size();
  std::string mod_time() const { return std::string{header_.mod_time, LENGTH_MTIME}; }
  std::string checksum() const { return std::string{header_.checksum}; }
  char typeflag() const { return header_.typeflag; }
  std::string linkname() { return std::string{header_.linkname}; }
  std::string magic() const { return std::string{header_.magic}; }
  std::string version() const { return std::string{header_.version, LENGTH_VERSION}; }
  std::string uname() const { return std::string{header_.uname}; }
  std::string gname() const { return std::string{header_.gname}; }
  std::string devmajor() const { return std::string{header_.devmajor}; }
  std::string devminor() const { return std::string{header_.devminor}; }
  std::string prefix() const { return std::string{header_.prefix}; }
  std::string pad() const { return std::string{header_.pad}; }

  bool is_ustar() const { return header_.magic == TMAGIC; }

  bool is_dir() const { return header_.typeflag == DIRTYPE; }

  bool typeflag_is_set() const { return header_.typeflag == ' '; }

  bool is_empty() const { return content_.size() == 0; }

  int num_content_blocks() const { return content_.size(); }

private:
  Tar_header& header_;
  std::vector<Content_block*> content_;
};

class Tar {

public:

  Tar() = default;

  int num_elements() const { return elements_.size(); }

  void add_element(const Element& element) { elements_.push_back(element); }
  const Element element(const std::string& path) const;
  const std::vector<Element>& elements() const { return elements_; }
  std::vector<std::string> element_names() const;

private:
  std::vector<Element> elements_;

};  // class Tar

class Tar_reader {

public:

  Tar& read_binary_tar() {
    // tar and mender

    const char* bin_content  = &_binary_input_bin_start;
    const int   bin_size     = (intptr_t) &_binary_input_bin_size;

    return read(bin_content, bin_size);
  }

  Tar& read(const char* file, size_t size);

  Tar& decompress_binary_tar_gz() {
    // tar.gz

    const char* bin_content  = &_binary_input_bin_start;
    const int   bin_size     = (intptr_t) &_binary_input_bin_size;

    return decompress(bin_content, bin_size);
  }

  Tar& decompress(const char* file, size_t size) {


    return tar_;
  }

private:
  Tar tar_;

};  // < class Tar_reader

#endif
