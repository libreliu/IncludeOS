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

#ifndef HW_GPIO_HPP
#define HW_GPIO_HPP
#include <delegate>
#include <cstdint>

namespace hw {
  enum class GPIO_DIR
  {
    OUTPUT,
    INPUT
  };

  enum class GPIO_LEVEL
  {
    HIGH=1,
    LOW=0
  };

  struct GPIO
  {
  using handler    = delegate<void(size_t pin)>;

  public:
    GPIO(uint32_t gpio_pin,GPIO_DIR dir=GPIO_DIR::INPUT,GPIO_LEVEL level=GPIO_LEVEL::LOW);
    void setHandler(handler);
    void set();
    void clear();
    void toggle();
    GPIO_LEVEL get();
  private:
    uint32_t gpio_pin;
    GPIO_LEVEL level;
    GPIO_DIR dir;
  };

/** LowLevel implementation that must exist to be able to use gpio*/
  //this must be implemented at platorm level to be able to use gpio
  class GPIOInterface
  {
  public:
    static void setDirection(uint32_t gpio_pin,GPIO_DIR direction);
    static void setLevel(uint32_t gpip_pin,GPIO_LEVEL level);
    static void set(uint32_t gpio_pin,GPIO_LEVEL level);
    //if pin is not input throw exception ?
    static GPIO_LEVEL get(uint32_t gio_pin);
    static void setTriggerLevel(uint32_t gpio_pin,GPIO_LEVEL level);
    static void setIrqHandler(uint32_t gpio_pin,GPIO::handler handler);
  };

}
#endif //HW_GPIO_HPP
