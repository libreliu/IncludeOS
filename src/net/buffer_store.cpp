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

//#undef  NO_DEBUG
#define DEBUG
#if !defined(__MACH__)
#include <malloc.h>
#else
#include <cstddef>
extern void *memalign(size_t, size_t);
#endif
#include <net/buffer_store.hpp>
#include <kernel/syscalls.hpp>
#include <common>
#include <cassert>
#include <smp>
//#define DEBUG_RELEASE
//#define DEBUG_RETRIEVE

#define PAGE_SIZE     0x1000
#define ENABLE_BUFFERSTORE_CHAIN


#ifdef DEBUG_RELEASE
#define BSD_RELEASE(fmt, ...) printf(fmt, ##__VA_ARGS__);
#else
#define BSD_RELEASE(fmt, ...)  /** fmt **/
#endif

#ifdef DEBUG_RETRIEVE
#define BSD_GET(fmt, ...) printf(fmt, ##__VA_ARGS__);
#else
#define BSD_GET(fmt, ...)  /** fmt **/
#endif

namespace net {

  bool BufferStore::smp_enabled_ = false;

  BufferStore::BufferStore(size_t num, size_t bufsize) :
    poolsize_  {num * bufsize},
    bufsize_   {bufsize},
    next_(nullptr)
  {
    assert(num != 0);
    assert(bufsize != 0);
    const size_t DATA_SIZE  = poolsize_;

    this->pool_ = (uint8_t*) memalign(PAGE_SIZE, DATA_SIZE);
    assert(this->pool_);

    available_.reserve(num);
    for (uint8_t* b = pool_end()-bufsize; b >= pool_begin(); b -= bufsize) {
        available_.push_back(b);
    }
    assert(available_.capacity() == num);
    assert(available() == num);

#ifndef INCLUDEOS_SINGLE_THREADED
    // set CPU id this bufferstore was created for
    this->cpu = SMP::cpu_id();
    smp_enabled_ = (this->cpu != 0);
#else
    this->cpu = 0;
#endif

    static int bsidx = 0;
    this->index = ++bsidx;
  }

  BufferStore::~BufferStore() {
    delete this->next_;
    free(this->pool_);
  }

  size_t BufferStore::available() const noexcept
  {
    auto avail = this->available_.size();
#ifdef ENABLE_BUFFERSTORE_CHAIN
    auto* parent = this;
    while (parent->next_ != nullptr) {
        parent = parent->next_;
        avail += parent->available_.size();
    }
#endif
    return avail;
  }
  size_t BufferStore::total_buffers() const noexcept
  {
    size_t total = this->local_buffers();
#ifdef ENABLE_BUFFERSTORE_CHAIN
    auto* parent = this;
    while (parent->next_ != nullptr) {
        parent = parent->next_;
        total += parent->local_buffers();
    }
#endif
    return total;
  }

  bool BufferStore::is_from_pool(uint8_t* addr) const noexcept
  {
    auto* current = this;
    if (addr >= current->pool_begin() and
        addr < current->pool_end()) return true;
#ifdef ENABLE_BUFFERSTORE_CHAIN
    while (current->next_ != nullptr) {
        current = current->next_;
        if (addr >= current->pool_begin() and
            addr < current->pool_end()) return true;
    }
#endif
    return false;
  }

  BufferStore* BufferStore::get_next_bufstore()
  {
#ifdef ENABLE_BUFFERSTORE_CHAIN
    BufferStore* parent = this;
    while (parent->next_ != nullptr) {
        parent = parent->next_;
        if (!parent->available_.empty()) return parent;
    }
    INFO("BufferStore", "Allocating %u new packets", local_buffers());
    parent->next_ = new BufferStore(local_buffers(), bufsize());
    return parent->next_;
#else
    return nullptr;
#endif
  }

  BufferStore::buffer_t BufferStore::get_buffer_directly() noexcept
  {
    auto addr = available_.back();
    available_.pop_back();
    return { this, addr };
  }

  BufferStore::buffer_t BufferStore::get_buffer()
  {
#ifndef INCLUDEOS_SINGLE_THREADED
    bool is_locked = false;
    if (smp_enabled_) {
      lock(plock);
      is_locked = true;
    }
#endif

    if (UNLIKELY(available_.empty())) {
#ifndef INCLUDEOS_SINGLE_THREADED
      if (is_locked) unlock(plock);
#endif
#ifdef ENABLE_BUFFERSTORE_CHAIN
      auto* next = get_next_bufstore();
      if (next == nullptr)
          throw std::runtime_error("Unable to create new bufstore");

      // take buffer from external bufstore
      auto buffer = next->get_buffer_directly();
      BSD_GET("%d: Gave away EXTERN %p, %lu buffers remain\n",
              this->index, buffer.addr, available());
      return buffer;
#else
      panic("<BufferStore> Buffer pool empty! Not configured to increase pool size.\n");
#endif
    }

    auto buffer = get_buffer_directly();
    BSD_GET("%d: Gave away %p, %lu buffers remain\n",
            this->index, buffer.addr, available());

#ifndef INCLUDEOS_SINGLE_THREADED
    if (is_locked) unlock(plock);
#endif
    return buffer;
  }

  void BufferStore::release(void* addr)
  {
    auto* buff = (uint8_t*) addr;
    BSD_RELEASE("%d: Release %p -> ", this->index, buff);

#ifndef INCLUDEOS_SINGLE_THREADED
    bool is_locked = false;
    if (smp_enabled_) {
      lock(plock);
      is_locked = true;
    }
#endif
    // expensive: is_buffer(buff)
    if (LIKELY(is_from_pool(buff))) {
      available_.push_back(buff);
#ifndef INCLUDEOS_SINGLE_THREADED
      if (is_locked) unlock(plock);
#endif
      BSD_RELEASE("released (avail=%lu / %lu)\n",
                  available(), total_buffers());
      return;
    }
#ifdef ENABLE_BUFFERSTORE_CHAIN
    // try to release buffer on linked bufferstore
    BufferStore* ptr = next_;
    while (ptr != nullptr) {
      if (ptr->is_from_pool(buff)) {
        BSD_RELEASE("released on other bufferstore\n");
        ptr->release_directly(buff);
        return;
      }
      ptr = ptr->next_;
    }
#endif
#ifndef INCLUDEOS_SINGLE_THREADED
    if (is_locked) unlock(plock);
#endif
    throw std::runtime_error("Packet did not belong");
  }

  void BufferStore::release_directly(uint8_t* buffer)
  {
    BSD_GET("%d: Released EXTERN %p, %lu buffers remain\n",
            this->index, buffer, available());
    available_.push_back(buffer);
  }

  void BufferStore::move_to_this_cpu() noexcept
  {
    this->cpu = SMP::cpu_id();
    if (this->cpu != 0) smp_enabled_ = true;
  }

} //< namespace net
