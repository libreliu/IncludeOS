// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2018 IncludeOS AS, Oslo, Norway
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

#define DEBUG_UNIT

#include <common.cxx>
#include <valarray>
#include <util/alloc_buddy.hpp>



struct Pool {
  using Alloc = mem::buddy::Alloc<true>;

  Pool(size_t s) : size{s} {
    auto sz  = Alloc::required_size(s);
    printf("Pool: wanted size 0x%zx, required 0x%zx\n", s, sz);
    auto res = posix_memalign(&addr, Alloc::min_size, sz);
    Expects(res == 0);
    alloc = Alloc::create(addr, sz);
  }

  ~Pool() {
    free((void*)addr);
  }

  size_t size;
  Alloc* alloc = nullptr;
  void* addr = nullptr;
};

CASE("mem::buddy init allocator"){
  using namespace util;
  Pool pool(64_KiB);
  auto& alloc = *pool.alloc;

  #ifdef DEBUG_UNIT
  std::cout << "Allocator root @ 0x"
            << std::hex << alloc.root().addr()
            << " size: "
            << std::dec << alloc.root().size()
            << "\n";
  #endif

  EXPECT(bool(alloc.root()));

  EXPECT(alloc.root().height() == 1);

  EXPECT(alloc.root().is_parent());
  EXPECT(alloc.root().is_free());
  EXPECT(alloc.root().left().height() == 2);
  EXPECT(alloc.bytes_used() == 0);
  auto addr = alloc.allocate(4_KiB);
  EXPECT(addr);
  EXPECT(alloc.bytes_used() == 4_KiB);
  alloc.deallocate(addr, 4_KiB);
  EXPECT(alloc.bytes_used() == 0);
}

CASE("mem::buddy basic allocation / deallocation"){
  using namespace util;

  Pool pool(64_KiB);
  auto& alloc = *pool.alloc;
  auto node = alloc.root();

  // Verify heights, leftmost addresses etc.
  for (int i = 1; i < alloc.tree_height(); i++) {
    EXPECT(node.height() == i);
    if (node.is_parent()) {
      EXPECT(node.left().height() == i + 1);
      EXPECT(node.right().height() == i + 1);
    } else {
      EXPECT(node.left().height() == 0);
    }
    EXPECT(node.addr() == alloc.addr_begin());
    node = node.left();
  }

  EXPECT(alloc.root().addr() == alloc.addr_begin());
  EXPECT(not alloc.root().deallocate(0x10, 0x10000));

  auto sum = 0;
  std::vector<void*> addresses;

  // Allocate all allowable sizes
  for (auto sz = alloc.min_size; sz < pool.size; sz *= 2) {
    auto addr = alloc.allocate(sz);
    EXPECT(addr);
    EXPECT(alloc.in_range(addr));
    addresses.push_back(addr);
    sum += sz;
    EXPECT(alloc.bytes_used() == sum);

    #ifdef DEBUG_UNIT
    std::cout << alloc.draw_tree();
    #endif
  }

  // Deallocate
  auto sz = alloc.min_size;
  for (auto addr : addresses) {
    sum -= sz;
    alloc.free((void*)addr);
    EXPECT(alloc.bytes_used() == sum);
    sz *= 2;
  }

  EXPECT(alloc.bytes_used() == 0);

}


CASE("mem::buddy random ordered allocation then deallocation"){
  using namespace util;
  #ifdef DEBUG_UNIT
  std::cout << "mem::buddy random ordered\n";
  #endif

  Pool pool(32_MiB);
  auto& alloc = *pool.alloc;

  EXPECT(bool(alloc.root()));

  auto sum = 0;
  std::vector<void*> addresses;
  std::vector<size_t> sizes;

  // Allocate all allowable sizes
  for (auto rnd : test::random_1k) {

    if (not alloc.bytes_free())
      break;

    const auto sz = alloc.chunksize(rnd % std::max(size_t(32_KiB), pool.size / 1024));

    #ifdef DEBUG_UNIT
    std::cout << "Alloc " << Byte_r(sz) << "\n";
    #endif
    auto avail = alloc.bytes_free();
    if (sz > avail or sz == 0) {
      if (alloc.full())
        break;
      continue;
    }
    auto addr  = alloc.allocate(sz);
    EXPECT(addr);
    EXPECT(alloc.in_range(addr));
    addresses.push_back(addr);
    sizes.push_back(sz);
    sum += sz;
    EXPECT(alloc.bytes_used() == sum);
    #ifdef DEBUG_UNIT
    std::cout << alloc.summary();
    #endif
  }

  auto dashes = std::string(80, '=');

  #ifdef DEBUG_UNIT
  std::cout << "\nAlloc done. Now dealloc \n" << dashes << "\n";
  if (pool.size <= 256_KiB)
    std::cout << alloc.draw_tree();
  #endif

  // Deallocate
  for (int i = 0; i < addresses.size(); i++) {
    auto addr = addresses.at(i);
    auto sz   = sizes.at(i);
    if (addr) EXPECT(sz) ;
    sum -= sz;
    alloc.free((void*)addr);
    EXPECT(alloc.bytes_used() == sum);
  }

  #ifdef DEBUG_UNIT
  std::cout << "Completed " << addresses.size() << " random ordered\n";
  std::cout << alloc.summary();
  #endif

  EXPECT(alloc.bytes_used() == 0);
}

struct Allocation {
  using Alloc  = Pool::Alloc;
  using Addr_t = typename Alloc::Addr_t;
  using Size_t = typename Alloc::Size_t;

  Addr_t addr = 0;
  Size_t size = 0;
  char data = 0;
  Alloc* alloc = nullptr;

  Allocation(Alloc* a) : alloc{a} {}

  auto addr_begin(){
    return addr;
  }

  auto addr_end() {
    return addr + size;
  }

  bool overlaps(Addr_t other) {
    return other >= addr_begin() and other < addr_end();
  }

  bool overlaps(Allocation other){
    return overlaps(other.addr_begin())
      or overlaps(other.addr_end());
  }

  bool verify_addr() {
    return alloc->in_range((void*)addr);
  }

  bool verify_all() {
    if (not verify_addr())
      return false;

    auto buf = std::make_unique<char[]>(size);
    memset(buf.get(), data, size);
    if (memcmp(buf.get(), (void*)addr, size) == 0)
      return true;
    return false;
  }
};


CASE("mem::buddy random chaos with data verification"){
  using namespace util;

  Pool pool(1_GiB);
  auto& alloc = *pool.alloc;

  EXPECT(bool(alloc.root()));
  EXPECT(alloc.bytes_free() == alloc.pool_size());

  std::vector<Allocation> allocs;

  for (auto rnd : test::random_1k) {
    auto sz = rnd % alloc.pool_size_ / 1024;
    EXPECT(sz);

    if (not alloc.full()) {
      Allocation a{&alloc};
      a.size = sz;
      a.addr = (uintptr_t)alloc.allocate(sz);
      a.data = 'A' + (rnd % ('Z' - 'A'));
      EXPECT(a.addr);
      EXPECT(a.verify_addr());
      EXPECT(a.overlaps(a));
      auto overlap =
        std::find_if(allocs.begin(), allocs.end(), [&a](Allocation& other) {
            return other.overlaps(a);
          });
      EXPECT(overlap == allocs.end());
      allocs.emplace_back(std::move(a));
      memset((void*)a.addr, a.data, a.size);
    }

    EXPECT(not alloc.empty());

    // Delete a random allocation
    if (rnd % 3 or alloc.full()) {
      auto a = allocs.begin() + rnd % allocs.size();
      a->verify_all();
      auto use_pre = alloc.bytes_used();
      alloc.deallocate((void*)a->addr, a->size);
      EXPECT(alloc.bytes_used() == use_pre - alloc.chunksize(a->size));
      allocs.erase(a);
    }
  }

  #ifdef DEBUG_UNIT
  std::cout << "mem::buddy random chaos complete \n";
  std::cout << alloc.summary();
  #endif

  for (auto a : allocs) {
    alloc.deallocate((void*)a.addr, a.size);
  }

  #ifdef DEBUG_UNIT
  std::cout << "mem::buddy random chaos cleaned up \n";
  #endif

  EXPECT(alloc.empty());
}
