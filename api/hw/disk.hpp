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

#ifndef HW_DISK_HPP
#define HW_DISK_HPP

#include "pci_device.hpp"
#include "block_device.hpp"

namespace hw {

  /**
   * Template version of Block_device, one (template) instance per device driver
   **/
  template <typename DRIVER>
  class Disk : public Block_device {
  public:
    /** optimal block size for this device */
    virtual block_t block_size() const noexcept override
    { return driver.block_size(); }

    /** Human readable name */
    const char* driver_name() const noexcept override
    {
      return driver.name();
    }

    static const char* device_type()
    { return "Disk"; }

    void
    read(block_t blk, on_read_func del) override {
      driver.read(blk, del);
    }
    void
    read(block_t blk, size_t count, on_read_func del) override {
      driver.read(blk, count, del);
    }

    buffer_t read_sync(block_t blk) override {
      return driver.read_sync(blk);
    }
    buffer_t read_sync(block_t blk, size_t cnt) override {
      return driver.read_sync(blk, cnt);
    }

    block_t size() const noexcept override
    {
      return driver.size();
    }

    virtual ~Disk() = default;

    /**
     *  Just a wrapper around the driver constructor
     *  @note The Dev-class is a friend and will call this
     */
    template <typename... Args>
    explicit Disk(PCI_Device& d, Args&&... args):
      driver{d, std::forward<Args>(args)... } {}

  private:
    DRIVER driver;

  }; //< class Disk

} //< namespace hw

#endif //< HW_DISK_HPP
