
#include "bcm2835_rng.hpp"

struct bcm2835_rng {
  volatile uint32_t ctrl;
  volatile uint32_t status;
  volatile uint32_t data;
  volatile uint32_t mask;
};

//this could be an inherited class
/*
class BCM2835_RNG
{
public:
  BCM2835_RNG(uintptr_t base);
  uint64_t data();
private:
  wait();
  struct bcm2835_rng *rng;
};
*/
static struct bcm2835_rng *rng=nullptr;

void bcm2835_rng_init(uintptr_t base)
{
  rng=(struct bcm2835_rng *)0x3F104000;
  rng->status=0x40000;
  rng->mask|=1; //enable
  rng->ctrl|=1; //enable rng source
  //wait for entropy
  while(!((rng->status)>>24)) asm volatile("nop");
}

void bcm2835_rng_data(uint64_t *data)
{
  if (rng == nullptr) {
    *data=0xD05EDA7ADEADBEEF; //unsafe
    return;
  }
  while(!((rng->status)>>24)) asm volatile("nop");
  uint64_t high=rng->data;
  while(!((rng->status)>>24)) asm volatile("nop");
  uint64_t low=rng->data;
  *data=high<<32|low;
}
