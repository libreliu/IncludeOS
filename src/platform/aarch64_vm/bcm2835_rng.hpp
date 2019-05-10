#ifndef BCM2835_RNG_HPP
#define BCM2835_RNG_HPP
#include <cstdint>


void bcm2835_rng_init(uintptr_t base/*todo pass the base addr*/);

void bcm2835_rng_data(uint64_t *data);

#endif //BCM2835_RNG_HPP
