#ifndef BCM2836_IRQ_HPP
#define BCM2836_IRQ_HPP
#include <cstdint>

void bcm2836_irq_init_fdt(const char * fdt,uint32_t gic_offset);

int bcm2835_irq_decode();
void bcm2835_irq_enable(uint32_t irq);
void bcm2835_irq_disable(uint32_t irq);
#endif //BCM2836_IRQ_HPP
