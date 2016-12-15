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

#include <tar>

#include <iostream> // strtol

using namespace tar;

// -------------------- Element --------------------

long int Element::size() {
  // Size of element's size in bytes
  char* pEnd;
  long int filesize;
  filesize = strtol(header_.size, &pEnd, 8); // header_.size is octal
  return filesize;
}

// ----------------------- Tar -----------------------

const Element Tar::element(const std::string& path) const {
  for (auto element : elements_) {
    if (element.name() == path) {
      return element;
    }
  }

  throw Tar_exception(std::string{"Path " + path + " doesn't exist. Folder names should have a trailing /"});
}

std::vector<std::string> Tar::element_names() const {
    std::vector<std::string> element_names;

    for (auto element : elements_)
      element_names.push_back(element.name());

    return element_names;
  }

// -------------------- Tar_reader --------------------

Tar& Tar_reader::read_uncompressed(const char* file, size_t size) {
  if (size == 0)
    throw Tar_exception("File is empty");

  if (size % SECTOR_SIZE not_eq 0)
    throw Tar_exception("Invalid size of tar file");

  // Go through the whole tar file block by block
  for (size_t i = 0; i < size; i += SECTOR_SIZE) {
    Tar_header* header = (Tar_header*) (file + i);
    Element element{*header};

    if (element.name().empty()) {
      // Empty header name - continue
      continue;
    }

    // Check if this is a directory or not (typeflag) (directories have no content)

    // If typeflag not set (as with m files) -> is not a directory and has content
    if (not element.typeflag_is_set() or not element.is_dir()) {
      // Get the size of this file in the tarball
      long int filesize = element.size(); // Gives the size in bytes (converts from octal to decimal)

      int num_content_blocks = 0;
      if (filesize % SECTOR_SIZE not_eq 0)
        num_content_blocks = (filesize / SECTOR_SIZE) + 1;
      else
        num_content_blocks = (filesize / SECTOR_SIZE);

      for (int j = 0; j < num_content_blocks; j++) {
        i += SECTOR_SIZE; // Go to next block with content

        Content_block* content_blk = (Content_block*) (file + i);
        element.add_content_block(content_blk);
      }
    }

    tar_.add_element(element);
  }

  return tar_;
}
