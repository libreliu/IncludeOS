#ifndef FDT_HPP
#include <cstdint>

extern "C" {
  #include <libfdt.h>
}

int fdt_interrupt_cells(const void *fdt,int nodeoffset);
uint64_t fdt_load_addr(const struct fdt_property *prop,int *offset,int addr_cells);
uint64_t fdt_load_size(const struct fdt_property *prop,int *offset,int size_cells);
#endif
