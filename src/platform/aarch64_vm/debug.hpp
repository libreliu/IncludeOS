//extern "C"
inline uint8_t nibble2hex(uint8_t nibble)
{
  nibble=nibble&0xF;
  switch(nibble)
  {
    case 0x00: return '0';
    case 0x01: return '1';
    case 0x02: return '2';
    case 0x03: return '3';
    case 0x04: return '4';
    case 0x05: return '5';
    case 0x06: return '6';
    case 0x07: return '7';
    case 0x08: return '8';
    case 0x09: return '9';
    case 0x0A: return 'A';
    case 0x0B: return 'B';
    case 0x0C: return 'C';
    case 0x0D: return 'D';
    case 0x0E: return 'E';
    case 0x0F: return 'F';
    default: return 0x00; // unreachable
  }
  //unreachable
}

//extern "C"
inline void print_memory(const char *name,const char * mem,int size)
{
  kprint(name);
  kprint(":\n");
  int i;
  for (i=0;i<size;i++)
  {
    char buf[3];
    buf[0]=nibble2hex(mem[i]>>4);
    buf[1]=nibble2hex(mem[i]);
    buf[2]='\0';
    kprint(buf);
    //*((volatile unsigned int *) 0x09000000) =nibble2hex(mem[i]>>4);
    //*((volatile unsigned int *) 0x09000000) =nibble2hex(mem[i]);
    if (((i+1)%4)==0)
    {
      kprint(" ");
    }
    if (((i+1)%16)==0)
    {
      kprint("\n");
    }

  }
  kprint("\n");
}
