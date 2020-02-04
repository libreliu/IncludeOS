#include <util/memstream.h>
#if defined(ARCH_x86_64) || defined(ARCH_x86)
#include <x86intrin.h>
#endif

void* streamcpy(void* dest, const void* srce, size_t n)
{
#if defined(ARCH_x86_64) || defined(ARCH_x86)
  char* dst       = (char*) dest;
  const char* src = (const char*) srce;

  // copy up to 15 bytes until SSE-aligned
  while (((intptr_t) dst & (SSE_SIZE-1)) && n)
  {
    *dst++ = *src++; n--;
  }
  // copy SSE-aligned
  while (n >= SSE_SIZE)
  {
    __m128i data = _mm_load_si128((__m128i*) src);
    _mm_stream_si128((__m128i*) dst, data);

    dst += SSE_SIZE;
    src += SSE_SIZE;

    n -= SSE_SIZE;
  }
  // copy remainder
  while (n--)
  {
    *dst++ = *src++;
  }
  return dst;
#else
  // a safer but inefficient implementation
  return streamucpy(dest, srce, n);
#endif
}
void* streamucpy(void* dest, const void* usrc, size_t n)
{
#if defined(ARCH_x86_64) || defined(ARCH_x86)
  char* dst       = (char*) dest;
  const char* src = (const char*) usrc;

  // copy up to 15 bytes until SSE-aligned
  while (((intptr_t) dst & (SSE_SIZE-1)) && n)
  {
    *dst++ = *src++; n--;
  }
  // copy SSE-aligned
  while (n >= SSE_SIZE)
  {
    __m128i data = _mm_loadu_si128((__m128i*) src);
    _mm_stream_si128((__m128i*) dst, data);

    dst  += SSE_SIZE;
    src += SSE_SIZE;

    n -= SSE_SIZE;
  }
  // copy remainder
  while (n--)
  {
    *dst++ = *src++;
  }
  return dst;
#else
# warning "Generic version used, performance and correctness not guaranteed."
  char *dst = (char *)dest;
  const char *src = (const char *)usrc;

  for (size_t i = 0; i < n; i++) {
    dst[i] = src[i];
  }
  return dst + n;
#endif
}

#if defined(ARCH_x86_64) || defined(ARCH_x86)
static inline char* stream_fill(char* dst, size_t* n, const __m128i data)
{
  while (*n >= SSE_SIZE)
  {
    _mm_stream_si128((__m128i*) dst, data);

    dst += SSE_SIZE;
    *n  -= SSE_SIZE;
  }
  return dst;
}
#endif 

void* streamset8(void* dest, int8_t value, size_t n)
{
#if defined(ARCH_x86_64) || defined(ARCH_x86)
  char* dst = dest;

  // memset up to 15 bytes until SSE-aligned
  while (((intptr_t) dst & (SSE_SIZE-1)) && n)
  {
    *dst++ = value; n--;
  }
  // memset SSE-aligned
  const __m128i data = _mm_set1_epi8(value);
  dst = stream_fill(dst, &n, data);
  // memset remainder
  while (n--)
  {
    *dst++ = value;
  }
  return dst;
#else
# warning "Generic version used, performance and correctness not guaranteed."
  char *dst = dest;
  for (size_t i = 0; i < n; i++) {
    dst[i] = value;
  }
  return dst + n;
#endif
}
void* streamset16(void* dest, int16_t value, size_t n)
{
#if defined(ARCH_x86_64) || defined(ARCH_x86)
  // memset SSE-aligned
  const __m128i data = _mm_set1_epi16(value);
  return stream_fill((char*) dest, &n, data);
#else
# warning "Generic version used, performance and correctness not guaranteed."
  int16_t *dst = dest;
  size_t times = n >> 1;
  for (size_t i = 0; i < times; i++) {
    dst[i] = value;
  }
  return dst + n;
#endif
}
void* streamset32(void* dest, int32_t value, size_t n)
{
#if defined(ARCH_x86_64) || defined(ARCH_x86)
  // memset SSE-aligned
  const __m128i data = _mm_set1_epi32(value);
  return stream_fill((char*) dest, &n, data);
#else
  #warning "Generic version used, performance and correctness not guaranteed."
  int32_t *dst = dest;
  size_t times = n >> 2;
  for (size_t i = 0; i < times; i++) {
    dst[i] = value;
  }
  return dst + n;
#endif
}
