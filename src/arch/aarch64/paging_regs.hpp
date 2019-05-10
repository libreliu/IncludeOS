#ifndef PAGING_REGS_HPP
#define PAGING_REGS_HPP
#include <kprint>



inline uint64_t ttbr0()
{
  uint64_t val=0;
  switch(cpu_current_el())
  {
    case 3:
      asm volatile("mrs %0,ttbr0_el3\r\n":"=r"(val)::"memory");
      break;
    case 2:
      asm volatile("mrs %0,ttbr0_el2\r\n":"=r"(val)::"memory");
      break;
    case 1:
      asm volatile("mrs %0,ttbr0_el1\r\n":"=r"(val)::"memory");
      break;
    //EL1 = todo!
  }
  return val;
}

inline uint64_t paging_capabs()
{
  uint64_t val;
  asm volatile ("mrs %0,ID_AA64MMFR0_EL1\r\n":"=r"(val)::"memory");
  return val;
}


//TODO move to somewhere central ?
//works for all regs up to 256bits
class BitRef {
public:
  //can do a first bit last bit version as well lol
//  BitRef(uint8_t bits,uint8_t shift) : m_bits(bits),m_shift(shift) {}
  uint8_t bits() {return m_bits; }
  uint8_t shift(){return m_shift; }
//private:
  uint8_t m_bits;
  uint8_t m_shift;
};

inline uint64_t extract_bits_64(uint64_t reg,BitRef br)
{
  uint64_t retval=reg>>br.shift()&((0x1<<br.bits())-1);
  return retval;
}

inline uint64_t set_bits_64(uint64_t reg,uint64_t val,BitRef br)
{
  uint64_t mask=(0x1<<br.bits()-1)<<br.shift();
  //clear mask
  reg&=reg&(~mask);
  //make sure we dont get unwanted bits from val
  reg|=((val<<br.shift())&mask);
  return reg;
}
//0x4 indicates 16TB no idea what 0x2 means
constexpr BitRef PARange={4,0};
//0x2 indicates 16 ASID bits supported
constexpr BitRef ASIDBits={4,4};
//0x1  indicates mixed endian support
constexpr BitRef BigEnd={4,8};
//0x1 indicates that the processor distinguishes between secure and non secure memory
constexpr BitRef SNSMem={4,12};
//not 0x0 indicates 16KB granule support
constexpr BitRef granule16KB={4,20};
//0x0 indicates 64KB granule support
constexpr BitRef granule64KB={4,24};
//0x0 indicates 4KB granule support
constexpr BitRef granule4KB={4,28};


#define PRINT_BITS(reg,name) \
  kprintf("%s=%zx\n",#name,extract_bits_64(reg,name));


//if not zero its supported
inline bool paging_supports16KB()
{
  if (extract_bits_64(paging_capabs(),granule16KB) != 0)
    return true;
  return false;
}

//if zero its supported
inline bool paging_supports64KB()
{
  if (extract_bits_64(paging_capabs(),granule64KB) == 0)
    return true;
  return false;
}

//if zero its supported
inline bool paging_supports4KB()
{
  if (extract_bits_64(paging_capabs(),granule4KB) == 0)
    return true;
  return false;
}

inline uint32_t paging_pa_bits()
{
  switch(extract_bits_64(paging_capabs(),PARange))
  {
    case 0x0: return 32; // 4GB
    case 0x1: return 36; // 64GB
    case 0x2: return 40; // 1TB
    case 0x3: return 42; // 4TB
    case 0x4: return 44; // 16TB
    case 0x5: return 48; // 256TB
    case 0x6: return 52; // 4PB
  }
}
/**
TCR ELX
000 4 GB 32 bits, PA[31:0]
001 64 GB 36 bits, PA[35:0]
010 1 TB 40 bits, PA[39:0]
011 4 TB 42 bits, PA[41:0]
100 16 TB 44 bits, PA[43:0]
101 256 TB 48 bits, PA[47:0]
110 4PB 52 bits, PA[51:0]a
*/

inline void print_paging_capabs()
{
  uint64_t val=paging_capabs();
  PRINT_BITS(val,PARange)
  PRINT_BITS(val,ASIDBits)
  PRINT_BITS(val,BigEnd)
  PRINT_BITS(val,SNSMem)
  PRINT_BITS(val,granule16KB)
  PRINT_BITS(val,granule64KB)
  PRINT_BITS(val,granule4KB)

  //printf("PARange=%x",extract_bits_64(val,PARange.shift,PARange.bits);
  //printf("ASIDbits=%x",extract_bits_64(val,PARange.shift,PARange.bits);

  //printf("Paging supported ")
}

inline uint32_t ttbcr()
{
  uint32_t val=0xCAFE;
//  asm volatile("mrs %0,ttbcr":"=r"(val)::"memory");
  return val;
}

inline uint64_t tcr()
{
  uint64_t val=0;
  switch(cpu_current_el())
  {
    case 3:
      asm volatile("mrs %0,tcr_el3\r\n":"=r"(val)::"memory");
      break;
    case 2:
      asm volatile("mrs %0,tcr_el2\r\n":"=r"(val)::"memory");
      break;
    case 1:
      asm volatile("mrs %0,tcr_el1\r\n":"=r"(val)::"memory");
      break;
    //EL1 = todo!
  }
  return val;
}
/*
0b00
Normal memory, Outer Non-cacheable.

0b01
Normal memory, Outer Write-Back Write-Allocate Cacheable.

0b10
Normal memory, Outer Write-Through Cacheable.

0b11
Normal memory, Outer Write-Back no Write-Allocate Cacheable.
*/
enum shareable
{
  none=0x0,
  outer=0x2,
  inner=0x3,
};

enum cache_type
{
  normal=0x0,
  write_back_allocate=0x1,
  write_through=0x2,
  write_back_no_allocate=0x3
};

//shared class for all ?
class tcr_el1
{
  enum class table
  {
    TTBR0,
    TTBR1
  };
public:
  //2^size => virtual address
  void setTOSZ(table t,uint32_t size)
  {
    uint64_t tosz;
    BitRef bits;
    if (size > 32)
      tosz=0;
    else
      tosz=(32-size);
    switch(t)
    {
      case table::TTBR0:
        bits={6,0};
        break;
      case table::TTBR1:
        bits={6,16};
        break;
    }

    tcr_el1=set_bits_64(tcr_el1,tosz,bits);
  }
private:
  uint64_t tcr_el1;
};


#endif //PAGING_REGS_HPP
