#include "bcm2835_gpio.hpp"
#include <stdio.h>

/*
https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf page 89:

There are 54 general-purpose I/O (GPIO) lines split into two banks. All GPIO pins have at
least two alternative functions within BCM. The alternate functions are usually peripheral IO
and a single peripheral may appear in each bank to allow flexibility on the choice of IO
voltage. Details of alternative functions are given in section 6.2. Alternative Function
*/

#include <cstdint>

using namespace hw;

//someday http://blog.bitwigglers.org/using-enum-classes-as-type-safe-bitmasks/
//TODO expose
enum class Function : uint32_t
{
  INPUT=0b000,
  OUTPUT=0b001,
  ALT0=0b100,
  ALT1=0b101,
  ALT2=0b110,
  ALT3=0b110,
  ALT4=0b011,
  ALT5=0b010,
};

/*

FSEL9 - Function Select 9
000 = GPIO Pin 9 is an input
001 = GPIO Pin 9 is an output
100 = GPIO Pin 9 takes alternate function 0
101 = GPIO Pin 9 takes alternate function 1
110 = GPIO Pin 9 takes alternate function 2
111 = GPIO Pin 9 takes alternate function 3
011 = GPIO Pin 9 takes alternate function 4
010 = GPIO Pin 9 takes alternate function 5
*/

//use std array instead ?
struct bcm2835_gpio_regs
{
  volatile uint32_t fsel[6]; //GPIO Function Select 0-5 32 R/W
  volatile uint32_t reserved1;
  volatile uint32_t set[2]; //GPIO Pin Output Set 0 32 W
  volatile uint32_t reserved2;
  volatile uint32_t clear[2]; //GPIO Pin Output Clear 0 32 W
  volatile uint32_t reserved3;
  volatile uint32_t level[2]; //GPIO Pin Level 0 32 R
  volatile uint32_t reserved4;
  volatile uint32_t status[2]; // GPIO Pin Event Detect Status 0 32 R/W
  volatile uint32_t reserved5;
  volatile uint32_t rising_edge_detect[2]; //GPIO Pin Rising Edge Detect Enable 0 32 R/W
  volatile uint32_t reserved6;
  volatile uint32_t falling_edge_detect[2]; //GPIO Pin Falling Edge Detect Enable 0 32 R/W
  volatile uint32_t reserved7;
  volatile uint32_t level_high_detect[2]; //GPIO Pin High Detect Enable 0 32 R/W
  volatile uint32_t reserved8;
  volatile uint32_t level_low_detect[2]; //GPIO Pin Low Detect Enable 0 32 R/W
  volatile uint32_t reserved9;
  volatile uint32_t async_rising_edge_detect[2]; //GPIO Pin Async. Rising Edge Detect 0 32 R/W
  volatile uint32_t reserved10;
  volatile uint32_t async_falling_edge_detect[2]; //GPIO Pin Async. Falling Edge Detect 0 32 R/W
  volatile uint32_t reserved11;
  volatile uint32_t pull_up_down_enable; //GPPUD GPIO Pin Pull-up/down Enable 32 R/W
  volatile uint32_t pull_up_down_enable_clock[2]; //GPIO Pin Pull-up/down Enable Clock 0 32 R/W
  volatile uint32_t reserved12[4]; //0xA0-0xB0
  volatile uint32_t test;
};

//TODO get this from fdt!!
static struct bcm2835_gpio_regs *gpio=(struct bcm2835_gpio_regs *)0x3F200000;


void GPIOInterface::setDirection(uint32_t gpio_pin,GPIO_DIR dir)
{
  if (gpio_pin > 54) return;
  uint32_t control_reg = gpio_pin/10; //3 and 3 bits totalling 10 per reg and one unused
  uint32_t pin=gpio_pin-(control_reg*10);
  printf("Setting direction for pin %d reg %d  subreg %d\n",gpio_pin,control_reg,pin);

  printf("using control reg %08x\n",&gpio->fsel[control_reg]);

  volatile uint32_t reg=gpio->fsel[control_reg];

  printf("pre modify reg %d = %08x\n",control_reg,reg);
  //clear any pending setting
  reg&= ~(0x7<<(pin*3));
  uint32_t val;
  if (dir == GPIO_DIR::OUTPUT)
    val = static_cast<uint32_t>(Function::OUTPUT);
  else
    val = static_cast<uint32_t>(Function::INPUT);
  reg|= (val&0x7)<<(pin*3);
  printf("post modify reg %d = %08x\n",control_reg,reg);
  gpio->fsel[control_reg]=reg;

  reg=gpio->fsel[control_reg];

  printf("readback reg %d = %08x\n",control_reg,reg);

}

void GPIOInterface::setLevel(uint32_t gpio_pin,GPIO_LEVEL level)
{
  //for this impl this is enough
  GPIOInterface::set(gpio_pin,level);
  /*if (gpio_pin > 54) return;

*/
}

void GPIOInterface::set(uint32_t gpio_pin,GPIO_LEVEL level)
{
  if (gpio_pin > 54) return;
  int reg=gpio_pin/32;
  int pin=0x1<<(gpio_pin&0x1F);
  static int first_high=10;
  static int first_low=10;


  switch(level)
  {
    case GPIO_LEVEL::HIGH:
      gpio->set[reg]=pin;
      if (first_high)
      {
        printf("set reg %d addr %08x pin %08x\n",reg,&gpio->set[reg],pin);
        first_high--;
      }

      break;
    case GPIO_LEVEL::LOW:
      gpio->clear[reg]=pin;
      if (first_low)
      {
        printf("clr reg %d addr %08x pin %08x\n",reg,&gpio->clear[reg],pin);
        first_low--;
      }

      break;
  }
}

GPIO_LEVEL GPIOInterface::get(uint32_t gpio_pin)
{
  //very painful to detect errors
  if (gpio_pin > 54) return GPIO_LEVEL::LOW;
  int reg=gpio_pin>>0x1F;
  int pin=0x1<<(gpio_pin&0x1F);

  if (gpio->level[reg]&(pin))
    return GPIO_LEVEL::HIGH;
  return GPIO_LEVEL::LOW;
}
