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
#define DEBUG_RELEASE

#define PAGE_SIZE     0x1000
#define ENABLE_BUFFERSTORE_CHAIN
#define BS_CHAIN_ALLOC_PACKETS   256


#ifdef DEBUG_RELEASE
#define BSD_RELEASE(fmt, ...) printf(fmt, ##__VA_ARGS__);
#else
#define BSD_RELEASE(fmt, ...)  /** fmt **/
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
    this->total_avail = num;
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

  BufferStore* BufferStore::get_next_bufstore()
  {
#ifdef ENABLE_BUFFERSTORE_CHAIN
    BufferStore* parent = this;
    while (parent->next_ != nullptr) {
        parent = parent->next_;
        if (parent->available() != 0) return parent;
    }
    INFO("BufferStore", "Allocating %u new packets", BS_CHAIN_ALLOC_PACKETS);
    parent->next_ = new BufferStore(BS_CHAIN_ALLOC_PACKETS, bufsize());
    total_avail += BS_CHAIN_ALLOC_PACKETS;
    return parent->next_;
#else
    return nullptr;
#endif
  }

  BufferStore::buffer_t BufferStore::get_buffer_directly() noexcept
  {
    auto addr = available_.back();
    available_.pop_back();
    this->total_avail --;
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
      if (next == nullptr) return {this, nullptr};

      // take buffer from external bufstore
      this->total_avail--;
      return next->get_buffer_directly();
#else
      panic("<BufferStore> Buffer pool empty! Not configured to increase pool size.\n");
#endif
    }

    auto buffer = get_buffer_directly();
    debug("Gave away %p, %lu buffers remain\n",
            (void*) addr, available_.size());

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
      this->total_avail++;
#ifndef INCLUDEOS_SINGLE_THREADED
      if (is_locked) unlock(plock);
#endif
      BSD_RELEASE("released (avail=%lu / %lu)\n",
                  available_.size(), total_buffers());
      return;
    }
#ifdef ENABLE_BUFFERSTORE_CHAIN
    // try to release buffer on linked bufferstore
    BufferStore* ptr = next_;
    while (ptr != nullptr) {
      if (ptr->is_from_pool(buff)) {
        BSD_RELEASE("released on other bufferstore\n");
        ptr->release_directly(buff);
        this->total_avail++;
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
    available_.push_back(buffer);
    this->total_avail ++;
  }

  void BufferStore::move_to_this_cpu() noexcept
  {
    this->cpu = SMP::cpu_id();
    if (this->cpu != 0) smp_enabled_ = true;
  }

} //< namespace net
