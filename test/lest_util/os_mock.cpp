// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2017 Oslo and Akershus University College of Applied Sciences
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

#ifdef __MACH__
#include <stdlib.h>
#include <stddef.h>
#include <gsl/gsl_assert>
void* memalign(size_t alignment, size_t size) {
  void* ptr {nullptr};
  int res = posix_memalign(&ptr, alignment, size);
  Ensures(res == 0);
  return ptr;
}
#endif

#include <os>
#include <assert.h>

// mocked commandline arguments
const std::string& OS::cmdline_args() noexcept
{
  static const std::string args("binary.bin command line arguments passed here");
  return args;
}

extern "C" {

  void panic(const char* why){

    assert(0 && "Panic: " && why);
  }

}
