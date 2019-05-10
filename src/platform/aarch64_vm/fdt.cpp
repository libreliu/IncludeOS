#include "fdt.hpp"

int fdt_interrupt_cells(const void *fdt,int nodeoffset)
{
  const struct fdt_property *prop;
	int val;
	int len;
  const char *name="#interrupt-cells";
	prop = fdt_get_property(fdt, nodeoffset,name, &len);
	if (!prop)
		return len;
    //hmm

	if (len != (sizeof(int)))
		return -FDT_ERR_BADNCELLS;

  val=fdt32_ld((const fdt32_t *)((char *)prop->data));

  if (val == -FDT_ERR_NOTFOUND)
    return 1;

	if ((val <= 0) || (val > FDT_MAX_NCELLS))
		return -FDT_ERR_BADNCELLS;
	return val;
}


uint64_t fdt_load_addr(const struct fdt_property *prop,int *offset,int addr_cells)
{
  uint64_t addr=0;
  for (int j = 0; j < addr_cells; ++j) {
    addr |= (uint64_t)fdt32_ld((const fdt32_t *)((char *)prop->data + *offset)) <<
      ((addr_cells - j - 1) * 32);
      *offset+=sizeof(uint32_t);
  }
  return addr;
}


uint64_t fdt_load_size(const struct fdt_property *prop,int *offset,int size_cells)
{
  uint64_t size=0;
  for (int j = 0; j < size_cells; ++j) {
    size |= (uint64_t)fdt32_ld((const fdt32_t *)((char *)prop->data + *offset)) <<
      ((size_cells - j - 1) * 32);
      *offset+=sizeof(uint32_t);
  }
  return size;
}
