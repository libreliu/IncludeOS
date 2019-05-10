#include <hw/gpio.hpp>

using namespace hw;
GPIO::GPIO(uint32_t id,GPIO_DIR dir,GPIO_LEVEL level) :
  gpio_pin(id),dir(dir)/*,level(level)*/
  {
    if (dir==GPIO_DIR::OUTPUT)
    {
      //set the initial level before enabling the output
      GPIOInterface::setLevel(gpio_pin,level);
    }

    //enable the gpio pin
    GPIOInterface::setDirection(gpio_pin,dir);
  }

void GPIO::set()
{
  GPIOInterface::set(gpio_pin,GPIO_LEVEL::HIGH);
}

void GPIO::clear()
{
  GPIOInterface::set(gpio_pin,GPIO_LEVEL::LOW);
}

//if pin is in tristate we might need to keep internal state for it
void GPIO::toggle()
{
  switch(get())
  {
    case GPIO_LEVEL::HIGH:
      GPIOInterface::set(gpio_pin,GPIO_LEVEL::LOW);
      break;
    case GPIO_LEVEL::LOW:
      GPIOInterface::set(gpio_pin,GPIO_LEVEL::LOW);
      break;
  }
}

GPIO_LEVEL GPIO::get()
{
  return GPIOInterface::get(gpio_pin);
}
