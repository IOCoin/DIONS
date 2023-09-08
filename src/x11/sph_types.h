#ifndef SPH_TYPES_H__
#define SPH_TYPES_H__ 

#include <limits.h>





#if CHAR_BIT != 8
#error This code requires 8-bit bytes
#endif



#ifdef DOXYGEN_IGNORE
# 384 "sph_types.h"
typedef __arch_dependant__ sph_u32;





typedef __arch_dependant__ sph_s32;
# 402 "sph_types.h"
typedef __arch_dependant__ sph_u64;





typedef __arch_dependant__ sph_s64;
# 418 "sph_types.h"
#define SPH_C32(x) 







#define SPH_T32(x) 
# 438 "sph_types.h"
#define SPH_ROTL32(x,n) 
# 450 "sph_types.h"
#define SPH_ROTR32(x,n) 





#define SPH_64 





#define SPH_64_TRUE 
# 473 "sph_types.h"
#define SPH_C64(x) 
# 482 "sph_types.h"
#define SPH_T64(x) 
# 495 "sph_types.h"
#define SPH_ROTL64(x,n) 
# 508 "sph_types.h"
#define SPH_ROTR64(x,n) 
# 517 "sph_types.h"
#define SPH_INLINE 







#define SPH_LITTLE_ENDIAN 







#define SPH_BIG_ENDIAN 
# 542 "sph_types.h"
#define SPH_LITTLE_FAST 
# 551 "sph_types.h"
#define SPH_BIG_FAST 







#define SPH_UPTR 






#define SPH_UNALIGNED 
# 576 "sph_types.h"
static inline sph_u32 sph_bswap32(sph_u32 x);
# 587 "sph_types.h"
static inline sph_u64 sph_bswap64(sph_u64 x);
# 596 "sph_types.h"
static inline unsigned sph_dec16le(const void *src);
# 605 "sph_types.h"
static inline void sph_enc16le(void *dst, unsigned val);
# 614 "sph_types.h"
static inline unsigned sph_dec16be(const void *src);
# 623 "sph_types.h"
static inline void sph_enc16be(void *dst, unsigned val);
# 632 "sph_types.h"
static inline sph_u32 sph_dec32le(const void *src);
# 644 "sph_types.h"
static inline sph_u32 sph_dec32le_aligned(const void *src);
# 653 "sph_types.h"
static inline void sph_enc32le(void *dst, sph_u32 val);
# 665 "sph_types.h"
static inline void sph_enc32le_aligned(void *dst, sph_u32 val);
# 674 "sph_types.h"
static inline sph_u32 sph_dec32be(const void *src);
# 686 "sph_types.h"
static inline sph_u32 sph_dec32be_aligned(const void *src);
# 695 "sph_types.h"
static inline void sph_enc32be(void *dst, sph_u32 val);
# 707 "sph_types.h"
static inline void sph_enc32be_aligned(void *dst, sph_u32 val);
# 717 "sph_types.h"
static inline sph_u64 sph_dec64le(const void *src);
# 730 "sph_types.h"
static inline sph_u64 sph_dec64le_aligned(const void *src);
# 740 "sph_types.h"
static inline void sph_enc64le(void *dst, sph_u64 val);
# 754 "sph_types.h"
static inline void sph_enc64le_aligned(void *dst, sph_u64 val);
# 764 "sph_types.h"
static inline sph_u64 sph_dec64be(const void *src);
# 777 "sph_types.h"
static inline sph_u64 sph_dec64be_aligned(const void *src);
# 787 "sph_types.h"
static inline void sph_enc64be(void *dst, sph_u64 val);
# 801 "sph_types.h"
static inline void sph_enc64be_aligned(void *dst, sph_u64 val);

#endif



#ifndef DOXYGEN_IGNORE
# 816 "sph_types.h"
#undef SPH_64
#undef SPH_64_TRUE

#if defined __STDC__ && __STDC_VERSION__ >= 199901L







#include <stdint.h>

#ifdef UINT32_MAX
typedef uint32_t sph_u32;
typedef int32_t sph_s32;
#else
typedef uint_fast32_t sph_u32;
typedef int_fast32_t sph_s32;
#endif
#if !SPH_NO_64
#ifdef UINT64_MAX
typedef uint64_t sph_u64;
typedef int64_t sph_s64;
#else
typedef uint_fast64_t sph_u64;
typedef int_fast64_t sph_s64;
#endif
#endif

#define SPH_C32(x) ((sph_u32)(x))
#if !SPH_NO_64
#define SPH_C64(x) ((sph_u64)(x))
#define SPH_64 1
#endif

#else
# 861 "sph_types.h"
#if ((UINT_MAX >> 11) >> 11) >= 0x3FF

typedef unsigned int sph_u32;
typedef int sph_s32;

#define SPH_C32(x) ((sph_u32)(x ## U))

#else

typedef unsigned long sph_u32;
typedef long sph_s32;

#define SPH_C32(x) ((sph_u32)(x ## UL))

#endif

#if !SPH_NO_64
# 887 "sph_types.h"
#if ((ULONG_MAX >> 31) >> 31) >= 3

typedef unsigned long sph_u64;
typedef long sph_s64;

#define SPH_C64(x) ((sph_u64)(x ## UL))

#define SPH_64 1

#elif ((ULLONG_MAX >> 31) >> 31) >= 3 || defined __GNUC__

typedef unsigned long long sph_u64;
typedef long long sph_s64;

#define SPH_C64(x) ((sph_u64)(x ## ULL))

#define SPH_64 1

#else





#endif

#endif

#endif






#if SPH_64 && (((ULONG_MAX >> 31) >> 31) >= 3 || defined _M_X64)
#define SPH_64_TRUE 1
#endif







#define SPH_T32(x) ((x) & SPH_C32(0xFFFFFFFF))
#define SPH_ROTL32(x,n) SPH_T32(((x) << (n)) | ((x) >> (32 - (n))))
#define SPH_ROTR32(x,n) SPH_ROTL32(x, (32 - (n)))

#if SPH_64

#define SPH_T64(x) ((x) & SPH_C64(0xFFFFFFFFFFFFFFFF))
#define SPH_ROTL64(x,n) SPH_T64(((x) << (n)) | ((x) >> (64 - (n))))
#define SPH_ROTR64(x,n) SPH_ROTL64(x, (64 - (n)))

#endif

#ifndef DOXYGEN_IGNORE




#if (defined __STDC__ && __STDC_VERSION__ >= 199901L) || defined __GNUC__
#define SPH_INLINE inline
#elif defined _MSC_VER
#define SPH_INLINE __inline
#else
#define SPH_INLINE 
#endif
#endif
# 1000 "sph_types.h"
#if defined __i386__ || defined _M_IX86

#define SPH_DETECT_UNALIGNED 1
#define SPH_DETECT_LITTLE_ENDIAN 1
#define SPH_DETECT_UPTR sph_u32
#ifdef __GNUC__
#define SPH_DETECT_I386_GCC 1
#endif
#ifdef _MSC_VER
#define SPH_DETECT_I386_MSVC 1
#endif




#elif defined __x86_64 || defined _M_X64

#define SPH_DETECT_UNALIGNED 1
#define SPH_DETECT_LITTLE_ENDIAN 1
#define SPH_DETECT_UPTR sph_u64
#ifdef __GNUC__
#define SPH_DETECT_AMD64_GCC 1
#endif
#ifdef _MSC_VER
#define SPH_DETECT_AMD64_MSVC 1
#endif




#elif ((defined __sparc__ || defined __sparc) && defined __arch64__) \
 || defined __sparcv9

#define SPH_DETECT_BIG_ENDIAN 1
#define SPH_DETECT_UPTR sph_u64
#ifdef __GNUC__
#define SPH_DETECT_SPARCV9_GCC_64 1
#define SPH_DETECT_LITTLE_FAST 1
#endif




#elif (defined __sparc__ || defined __sparc) \
 && !(defined __sparcv9 || defined __arch64__)

#define SPH_DETECT_BIG_ENDIAN 1
#define SPH_DETECT_UPTR sph_u32
#if defined __GNUC__ && defined __sparc_v9__
#define SPH_DETECT_SPARCV9_GCC_32 1
#define SPH_DETECT_LITTLE_FAST 1
#endif




#elif defined __arm__ && __ARMEL__

#define SPH_DETECT_LITTLE_ENDIAN 1




#elif MIPSEL || _MIPSEL || __MIPSEL || __MIPSEL__

#define SPH_DETECT_LITTLE_ENDIAN 1




#elif MIPSEB || _MIPSEB || __MIPSEB || __MIPSEB__

#define SPH_DETECT_BIG_ENDIAN 1




#elif defined __powerpc__ || defined __POWERPC__ || defined __ppc__ \
 || defined _ARCH_PPC







#if defined __GNUC__
#if SPH_64_TRUE
#define SPH_DETECT_PPC64_GCC 1
#else
#define SPH_DETECT_PPC32_GCC 1
#endif
#endif

#if defined __BIG_ENDIAN__ || defined _BIG_ENDIAN
#define SPH_DETECT_BIG_ENDIAN 1
#elif defined __LITTLE_ENDIAN__ || defined _LITTLE_ENDIAN
#define SPH_DETECT_LITTLE_ENDIAN 1
#endif




#elif defined __ia64 || defined __ia64__ \
 || defined __itanium__ || defined _M_IA64

#if defined __BIG_ENDIAN__ || defined _BIG_ENDIAN
#define SPH_DETECT_BIG_ENDIAN 1
#else
#define SPH_DETECT_LITTLE_ENDIAN 1
#endif
#if defined __LP64__ || defined _LP64
#define SPH_DETECT_UPTR sph_u64
#else
#define SPH_DETECT_UPTR sph_u32
#endif

#endif

#if defined SPH_DETECT_SPARCV9_GCC_32 || defined SPH_DETECT_SPARCV9_GCC_64
#define SPH_DETECT_SPARCV9_GCC 1
#endif

#if defined SPH_DETECT_UNALIGNED && !defined SPH_UNALIGNED
#define SPH_UNALIGNED SPH_DETECT_UNALIGNED
#endif
#if defined SPH_DETECT_UPTR && !defined SPH_UPTR
#define SPH_UPTR SPH_DETECT_UPTR
#endif
#if defined SPH_DETECT_LITTLE_ENDIAN && !defined SPH_LITTLE_ENDIAN
#define SPH_LITTLE_ENDIAN SPH_DETECT_LITTLE_ENDIAN
#endif
#if defined SPH_DETECT_BIG_ENDIAN && !defined SPH_BIG_ENDIAN
#define SPH_BIG_ENDIAN SPH_DETECT_BIG_ENDIAN
#endif
#if defined SPH_DETECT_LITTLE_FAST && !defined SPH_LITTLE_FAST
#define SPH_LITTLE_FAST SPH_DETECT_LITTLE_FAST
#endif
#if defined SPH_DETECT_BIG_FAST && !defined SPH_BIG_FAST
#define SPH_BIG_FAST SPH_DETECT_BIG_FAST
#endif
#if defined SPH_DETECT_SPARCV9_GCC_32 && !defined SPH_SPARCV9_GCC_32
#define SPH_SPARCV9_GCC_32 SPH_DETECT_SPARCV9_GCC_32
#endif
#if defined SPH_DETECT_SPARCV9_GCC_64 && !defined SPH_SPARCV9_GCC_64
#define SPH_SPARCV9_GCC_64 SPH_DETECT_SPARCV9_GCC_64
#endif
#if defined SPH_DETECT_SPARCV9_GCC && !defined SPH_SPARCV9_GCC
#define SPH_SPARCV9_GCC SPH_DETECT_SPARCV9_GCC
#endif
#if defined SPH_DETECT_I386_GCC && !defined SPH_I386_GCC
#define SPH_I386_GCC SPH_DETECT_I386_GCC
#endif
#if defined SPH_DETECT_I386_MSVC && !defined SPH_I386_MSVC
#define SPH_I386_MSVC SPH_DETECT_I386_MSVC
#endif
#if defined SPH_DETECT_AMD64_GCC && !defined SPH_AMD64_GCC
#define SPH_AMD64_GCC SPH_DETECT_AMD64_GCC
#endif
#if defined SPH_DETECT_AMD64_MSVC && !defined SPH_AMD64_MSVC
#define SPH_AMD64_MSVC SPH_DETECT_AMD64_MSVC
#endif
#if defined SPH_DETECT_PPC32_GCC && !defined SPH_PPC32_GCC
#define SPH_PPC32_GCC SPH_DETECT_PPC32_GCC
#endif
#if defined SPH_DETECT_PPC64_GCC && !defined SPH_PPC64_GCC
#define SPH_PPC64_GCC SPH_DETECT_PPC64_GCC
#endif

#if SPH_LITTLE_ENDIAN && !defined SPH_LITTLE_FAST
#define SPH_LITTLE_FAST 1
#endif
#if SPH_BIG_ENDIAN && !defined SPH_BIG_FAST
#define SPH_BIG_FAST 1
#endif

#if defined SPH_UPTR && !(SPH_LITTLE_ENDIAN || SPH_BIG_ENDIAN)
#error SPH_UPTR defined, but endianness is not known.
#endif

#if SPH_I386_GCC && !SPH_NO_ASM






static SPH_INLINE sph_u32
sph_bswap32(sph_u32 x)
{
  __asm__ __volatile__ ("bswapl %0" : "=r" (x) : "0" (x));
  return x;
}

#if SPH_64

static SPH_INLINE sph_u64
sph_bswap64(sph_u64 x)
{
  return ((sph_u64)sph_bswap32((sph_u32)x) << 32)
         | (sph_u64)sph_bswap32((sph_u32)(x >> 32));
}

#endif

#elif SPH_AMD64_GCC && !SPH_NO_ASM






static SPH_INLINE sph_u32
sph_bswap32(sph_u32 x)
{
  __asm__ __volatile__ ("bswapl %0" : "=r" (x) : "0" (x));
  return x;
}

#if SPH_64

static SPH_INLINE sph_u64
sph_bswap64(sph_u64 x)
{
  __asm__ __volatile__ ("bswapq %0" : "=r" (x) : "0" (x));
  return x;
}

#endif
# 1263 "sph_types.h"
#else

static SPH_INLINE sph_u32
sph_bswap32(sph_u32 x)
{
  x = SPH_T32((x << 16) | (x >> 16));
  x = ((x & SPH_C32(0xFF00FF00)) >> 8)
      | ((x & SPH_C32(0x00FF00FF)) << 8);
  return x;
}

#if SPH_64







static SPH_INLINE sph_u64
sph_bswap64(sph_u64 x)
{
  x = SPH_T64((x << 32) | (x >> 32));
  x = ((x & SPH_C64(0xFFFF0000FFFF0000)) >> 16)
      | ((x & SPH_C64(0x0000FFFF0000FFFF)) << 16);
  x = ((x & SPH_C64(0xFF00FF00FF00FF00)) >> 8)
      | ((x & SPH_C64(0x00FF00FF00FF00FF)) << 8);
  return x;
}

#endif

#endif

#if SPH_SPARCV9_GCC && !SPH_NO_ASM
# 1315 "sph_types.h"
#define SPH_SPARCV9_SET_ASI \
 sph_u32 sph_sparcv9_asi; \
 __asm__ __volatile__ ( \
  "rd %%asi,%0\n\twr %%g0,0x88,%%asi" : "=r" (sph_sparcv9_asi));

#define SPH_SPARCV9_RESET_ASI \
 __asm__ __volatile__ ("wr %%g0,%0,%%asi" : : "r" (sph_sparcv9_asi));

#define SPH_SPARCV9_DEC32LE(base,idx) ({ \
  sph_u32 sph_sparcv9_tmp; \
  __asm__ __volatile__ ("lda [%1+" #idx "*4]%%asi,%0" \
   : "=r" (sph_sparcv9_tmp) : "r" (base)); \
  sph_sparcv9_tmp; \
 })

#endif

static SPH_INLINE void
sph_enc16be(void *dst, unsigned val)
{
  ((unsigned char *)dst)[0] = (val >> 8);
  ((unsigned char *)dst)[1] = val;
}

static SPH_INLINE unsigned
sph_dec16be(const void *src)
{
  return ((unsigned)(((const unsigned char *)src)[0]) << 8)
         | (unsigned)(((const unsigned char *)src)[1]);
}

static SPH_INLINE void
sph_enc16le(void *dst, unsigned val)
{
  ((unsigned char *)dst)[0] = val;
  ((unsigned char *)dst)[1] = val >> 8;
}

static SPH_INLINE unsigned
sph_dec16le(const void *src)
{
  return (unsigned)(((const unsigned char *)src)[0])
         | ((unsigned)(((const unsigned char *)src)[1]) << 8);
}







static SPH_INLINE void
sph_enc32be(void *dst, sph_u32 val)
{
#if defined SPH_UPTR
#if SPH_UNALIGNED
#if SPH_LITTLE_ENDIAN
  val = sph_bswap32(val);
#endif
  *(sph_u32 *)dst = val;
#else
  if (((SPH_UPTR)dst & 3) == 0)
  {
#if SPH_LITTLE_ENDIAN
    val = sph_bswap32(val);
#endif
    *(sph_u32 *)dst = val;
  }
  else
  {
    ((unsigned char *)dst)[0] = (val >> 24);
    ((unsigned char *)dst)[1] = (val >> 16);
    ((unsigned char *)dst)[2] = (val >> 8);
    ((unsigned char *)dst)[3] = val;
  }
#endif
#else
  ((unsigned char *)dst)[0] = (val >> 24);
  ((unsigned char *)dst)[1] = (val >> 16);
  ((unsigned char *)dst)[2] = (val >> 8);
  ((unsigned char *)dst)[3] = val;
#endif
}
# 1406 "sph_types.h"
static SPH_INLINE void
sph_enc32be_aligned(void *dst, sph_u32 val)
{
#if SPH_LITTLE_ENDIAN
  *(sph_u32 *)dst = sph_bswap32(val);
#elif SPH_BIG_ENDIAN
  *(sph_u32 *)dst = val;
#else
  ((unsigned char *)dst)[0] = (val >> 24);
  ((unsigned char *)dst)[1] = (val >> 16);
  ((unsigned char *)dst)[2] = (val >> 8);
  ((unsigned char *)dst)[3] = val;
#endif
}







static SPH_INLINE sph_u32
sph_dec32be(const void *src)
{
#if defined SPH_UPTR
#if SPH_UNALIGNED
#if SPH_LITTLE_ENDIAN
  return sph_bswap32(*(const sph_u32 *)src);
#else
  return *(const sph_u32 *)src;
#endif
#else
  if (((SPH_UPTR)src & 3) == 0)
  {
#if SPH_LITTLE_ENDIAN
    return sph_bswap32(*(const sph_u32 *)src);
#else
    return *(const sph_u32 *)src;
#endif
  }
  else
  {
    return ((sph_u32)(((const unsigned char *)src)[0]) << 24)
           | ((sph_u32)(((const unsigned char *)src)[1]) << 16)
           | ((sph_u32)(((const unsigned char *)src)[2]) << 8)
           | (sph_u32)(((const unsigned char *)src)[3]);
  }
#endif
#else
  return ((sph_u32)(((const unsigned char *)src)[0]) << 24)
         | ((sph_u32)(((const unsigned char *)src)[1]) << 16)
         | ((sph_u32)(((const unsigned char *)src)[2]) << 8)
         | (sph_u32)(((const unsigned char *)src)[3]);
#endif
}
# 1469 "sph_types.h"
static SPH_INLINE sph_u32
sph_dec32be_aligned(const void *src)
{
#if SPH_LITTLE_ENDIAN
  return sph_bswap32(*(const sph_u32 *)src);
#elif SPH_BIG_ENDIAN
  return *(const sph_u32 *)src;
#else
  return ((sph_u32)(((const unsigned char *)src)[0]) << 24)
         | ((sph_u32)(((const unsigned char *)src)[1]) << 16)
         | ((sph_u32)(((const unsigned char *)src)[2]) << 8)
         | (sph_u32)(((const unsigned char *)src)[3]);
#endif
}







static SPH_INLINE void
sph_enc32le(void *dst, sph_u32 val)
{
#if defined SPH_UPTR
#if SPH_UNALIGNED
#if SPH_BIG_ENDIAN
  val = sph_bswap32(val);
#endif
  *(sph_u32 *)dst = val;
#else
  if (((SPH_UPTR)dst & 3) == 0)
  {
#if SPH_BIG_ENDIAN
    val = sph_bswap32(val);
#endif
    *(sph_u32 *)dst = val;
  }
  else
  {
    ((unsigned char *)dst)[0] = val;
    ((unsigned char *)dst)[1] = (val >> 8);
    ((unsigned char *)dst)[2] = (val >> 16);
    ((unsigned char *)dst)[3] = (val >> 24);
  }
#endif
#else
  ((unsigned char *)dst)[0] = val;
  ((unsigned char *)dst)[1] = (val >> 8);
  ((unsigned char *)dst)[2] = (val >> 16);
  ((unsigned char *)dst)[3] = (val >> 24);
#endif
}
# 1530 "sph_types.h"
static SPH_INLINE void
sph_enc32le_aligned(void *dst, sph_u32 val)
{
#if SPH_LITTLE_ENDIAN
  *(sph_u32 *)dst = val;
#elif SPH_BIG_ENDIAN
  *(sph_u32 *)dst = sph_bswap32(val);
#else
  ((unsigned char *)dst)[0] = val;
  ((unsigned char *)dst)[1] = (val >> 8);
  ((unsigned char *)dst)[2] = (val >> 16);
  ((unsigned char *)dst)[3] = (val >> 24);
#endif
}







static SPH_INLINE sph_u32
sph_dec32le(const void *src)
{
#if defined SPH_UPTR
#if SPH_UNALIGNED
#if SPH_BIG_ENDIAN
  return sph_bswap32(*(const sph_u32 *)src);
#else
  return *(const sph_u32 *)src;
#endif
#else
  if (((SPH_UPTR)src & 3) == 0)
  {
#if SPH_BIG_ENDIAN
#if SPH_SPARCV9_GCC && !SPH_NO_ASM
    sph_u32 tmp;







    __asm__ __volatile__ (
      "lda [%1]0x88,%0" : "=r" (tmp) : "r" (src));
    return tmp;
# 1596 "sph_types.h"
#else
    return sph_bswap32(*(const sph_u32 *)src);
#endif
#else
    return *(const sph_u32 *)src;
#endif
    }
    else
    {
    return (sph_u32)(((const unsigned char *)src)[0])
    | ((sph_u32)(((const unsigned char *)src)[1]) << 8)
    | ((sph_u32)(((const unsigned char *)src)[2]) << 16)
    | ((sph_u32)(((const unsigned char *)src)[3]) << 24);
    }
#endif
#else
  return (sph_u32)(((const unsigned char *)src)[0])
         | ((sph_u32)(((const unsigned char *)src)[1]) << 8)
         | ((sph_u32)(((const unsigned char *)src)[2]) << 16)
         | ((sph_u32)(((const unsigned char *)src)[3]) << 24);
#endif
}
# 1626 "sph_types.h"
static SPH_INLINE sph_u32
sph_dec32le_aligned(const void *src)
{
#if SPH_LITTLE_ENDIAN
  return *(const sph_u32 *)src;
#elif SPH_BIG_ENDIAN
#if SPH_SPARCV9_GCC && !SPH_NO_ASM
  sph_u32 tmp;

  __asm__ __volatile__ ("lda [%1]0x88,%0" : "=r" (tmp) : "r" (src));
  return tmp;
# 1646 "sph_types.h"
#else
  return sph_bswap32(*(const sph_u32 *)src);
#endif
#else
  return (sph_u32)(((const unsigned char *)src)[0])
         | ((sph_u32)(((const unsigned char *)src)[1]) << 8)
         | ((sph_u32)(((const unsigned char *)src)[2]) << 16)
         | ((sph_u32)(((const unsigned char *)src)[3]) << 24);
#endif
}

#if SPH_64







static SPH_INLINE void
sph_enc64be(void *dst, sph_u64 val)
{
#if defined SPH_UPTR
#if SPH_UNALIGNED
#if SPH_LITTLE_ENDIAN
  val = sph_bswap64(val);
#endif
  *(sph_u64 *)dst = val;
#else
  if (((SPH_UPTR)dst & 7) == 0)
  {
#if SPH_LITTLE_ENDIAN
    val = sph_bswap64(val);
#endif
    *(sph_u64 *)dst = val;
  }
  else
  {
    ((unsigned char *)dst)[0] = (val >> 56);
    ((unsigned char *)dst)[1] = (val >> 48);
    ((unsigned char *)dst)[2] = (val >> 40);
    ((unsigned char *)dst)[3] = (val >> 32);
    ((unsigned char *)dst)[4] = (val >> 24);
    ((unsigned char *)dst)[5] = (val >> 16);
    ((unsigned char *)dst)[6] = (val >> 8);
    ((unsigned char *)dst)[7] = val;
  }
#endif
#else
  ((unsigned char *)dst)[0] = (val >> 56);
  ((unsigned char *)dst)[1] = (val >> 48);
  ((unsigned char *)dst)[2] = (val >> 40);
  ((unsigned char *)dst)[3] = (val >> 32);
  ((unsigned char *)dst)[4] = (val >> 24);
  ((unsigned char *)dst)[5] = (val >> 16);
  ((unsigned char *)dst)[6] = (val >> 8);
  ((unsigned char *)dst)[7] = val;
#endif
}
# 1713 "sph_types.h"
static SPH_INLINE void
sph_enc64be_aligned(void *dst, sph_u64 val)
{
#if SPH_LITTLE_ENDIAN
  *(sph_u64 *)dst = sph_bswap64(val);
#elif SPH_BIG_ENDIAN
  *(sph_u64 *)dst = val;
#else
  ((unsigned char *)dst)[0] = (val >> 56);
  ((unsigned char *)dst)[1] = (val >> 48);
  ((unsigned char *)dst)[2] = (val >> 40);
  ((unsigned char *)dst)[3] = (val >> 32);
  ((unsigned char *)dst)[4] = (val >> 24);
  ((unsigned char *)dst)[5] = (val >> 16);
  ((unsigned char *)dst)[6] = (val >> 8);
  ((unsigned char *)dst)[7] = val;
#endif
}







static SPH_INLINE sph_u64
sph_dec64be(const void *src)
{
#if defined SPH_UPTR
#if SPH_UNALIGNED
#if SPH_LITTLE_ENDIAN
  return sph_bswap64(*(const sph_u64 *)src);
#else
  return *(const sph_u64 *)src;
#endif
#else
  if (((SPH_UPTR)src & 7) == 0)
  {
#if SPH_LITTLE_ENDIAN
    return sph_bswap64(*(const sph_u64 *)src);
#else
    return *(const sph_u64 *)src;
#endif
  }
  else
  {
    return ((sph_u64)(((const unsigned char *)src)[0]) << 56)
           | ((sph_u64)(((const unsigned char *)src)[1]) << 48)
           | ((sph_u64)(((const unsigned char *)src)[2]) << 40)
           | ((sph_u64)(((const unsigned char *)src)[3]) << 32)
           | ((sph_u64)(((const unsigned char *)src)[4]) << 24)
           | ((sph_u64)(((const unsigned char *)src)[5]) << 16)
           | ((sph_u64)(((const unsigned char *)src)[6]) << 8)
           | (sph_u64)(((const unsigned char *)src)[7]);
  }
#endif
#else
  return ((sph_u64)(((const unsigned char *)src)[0]) << 56)
         | ((sph_u64)(((const unsigned char *)src)[1]) << 48)
         | ((sph_u64)(((const unsigned char *)src)[2]) << 40)
         | ((sph_u64)(((const unsigned char *)src)[3]) << 32)
         | ((sph_u64)(((const unsigned char *)src)[4]) << 24)
         | ((sph_u64)(((const unsigned char *)src)[5]) << 16)
         | ((sph_u64)(((const unsigned char *)src)[6]) << 8)
         | (sph_u64)(((const unsigned char *)src)[7]);
#endif
}
# 1788 "sph_types.h"
static SPH_INLINE sph_u64
sph_dec64be_aligned(const void *src)
{
#if SPH_LITTLE_ENDIAN
  return sph_bswap64(*(const sph_u64 *)src);
#elif SPH_BIG_ENDIAN
  return *(const sph_u64 *)src;
#else
  return ((sph_u64)(((const unsigned char *)src)[0]) << 56)
         | ((sph_u64)(((const unsigned char *)src)[1]) << 48)
         | ((sph_u64)(((const unsigned char *)src)[2]) << 40)
         | ((sph_u64)(((const unsigned char *)src)[3]) << 32)
         | ((sph_u64)(((const unsigned char *)src)[4]) << 24)
         | ((sph_u64)(((const unsigned char *)src)[5]) << 16)
         | ((sph_u64)(((const unsigned char *)src)[6]) << 8)
         | (sph_u64)(((const unsigned char *)src)[7]);
#endif
}







static SPH_INLINE void
sph_enc64le(void *dst, sph_u64 val)
{
#if defined SPH_UPTR
#if SPH_UNALIGNED
#if SPH_BIG_ENDIAN
  val = sph_bswap64(val);
#endif
  *(sph_u64 *)dst = val;
#else
  if (((SPH_UPTR)dst & 7) == 0)
  {
#if SPH_BIG_ENDIAN
    val = sph_bswap64(val);
#endif
    *(sph_u64 *)dst = val;
  }
  else
  {
    ((unsigned char *)dst)[0] = val;
    ((unsigned char *)dst)[1] = (val >> 8);
    ((unsigned char *)dst)[2] = (val >> 16);
    ((unsigned char *)dst)[3] = (val >> 24);
    ((unsigned char *)dst)[4] = (val >> 32);
    ((unsigned char *)dst)[5] = (val >> 40);
    ((unsigned char *)dst)[6] = (val >> 48);
    ((unsigned char *)dst)[7] = (val >> 56);
  }
#endif
#else
  ((unsigned char *)dst)[0] = val;
  ((unsigned char *)dst)[1] = (val >> 8);
  ((unsigned char *)dst)[2] = (val >> 16);
  ((unsigned char *)dst)[3] = (val >> 24);
  ((unsigned char *)dst)[4] = (val >> 32);
  ((unsigned char *)dst)[5] = (val >> 40);
  ((unsigned char *)dst)[6] = (val >> 48);
  ((unsigned char *)dst)[7] = (val >> 56);
#endif
}
# 1861 "sph_types.h"
static SPH_INLINE void
sph_enc64le_aligned(void *dst, sph_u64 val)
{
#if SPH_LITTLE_ENDIAN
  *(sph_u64 *)dst = val;
#elif SPH_BIG_ENDIAN
  *(sph_u64 *)dst = sph_bswap64(val);
#else
  ((unsigned char *)dst)[0] = val;
  ((unsigned char *)dst)[1] = (val >> 8);
  ((unsigned char *)dst)[2] = (val >> 16);
  ((unsigned char *)dst)[3] = (val >> 24);
  ((unsigned char *)dst)[4] = (val >> 32);
  ((unsigned char *)dst)[5] = (val >> 40);
  ((unsigned char *)dst)[6] = (val >> 48);
  ((unsigned char *)dst)[7] = (val >> 56);
#endif
}







static SPH_INLINE sph_u64
sph_dec64le(const void *src)
{
#if defined SPH_UPTR
#if SPH_UNALIGNED
#if SPH_BIG_ENDIAN
  return sph_bswap64(*(const sph_u64 *)src);
#else
  return *(const sph_u64 *)src;
#endif
#else
  if (((SPH_UPTR)src & 7) == 0)
  {
#if SPH_BIG_ENDIAN
#if SPH_SPARCV9_GCC_64 && !SPH_NO_ASM
    sph_u64 tmp;

    __asm__ __volatile__ (
      "ldxa [%1]0x88,%0" : "=r" (tmp) : "r" (src));
    return tmp;
# 1920 "sph_types.h"
#else
    return sph_bswap64(*(const sph_u64 *)src);
#endif
#else
    return *(const sph_u64 *)src;
#endif
    }
    else
    {
    return (sph_u64)(((const unsigned char *)src)[0])
    | ((sph_u64)(((const unsigned char *)src)[1]) << 8)
    | ((sph_u64)(((const unsigned char *)src)[2]) << 16)
    | ((sph_u64)(((const unsigned char *)src)[3]) << 24)
    | ((sph_u64)(((const unsigned char *)src)[4]) << 32)
    | ((sph_u64)(((const unsigned char *)src)[5]) << 40)
    | ((sph_u64)(((const unsigned char *)src)[6]) << 48)
    | ((sph_u64)(((const unsigned char *)src)[7]) << 56);
    }
#endif
#else
  return (sph_u64)(((const unsigned char *)src)[0])
         | ((sph_u64)(((const unsigned char *)src)[1]) << 8)
         | ((sph_u64)(((const unsigned char *)src)[2]) << 16)
         | ((sph_u64)(((const unsigned char *)src)[3]) << 24)
         | ((sph_u64)(((const unsigned char *)src)[4]) << 32)
         | ((sph_u64)(((const unsigned char *)src)[5]) << 40)
         | ((sph_u64)(((const unsigned char *)src)[6]) << 48)
         | ((sph_u64)(((const unsigned char *)src)[7]) << 56);
#endif
}
# 1958 "sph_types.h"
static SPH_INLINE sph_u64
sph_dec64le_aligned(const void *src)
{
#if SPH_LITTLE_ENDIAN
  return *(const sph_u64 *)src;
#elif SPH_BIG_ENDIAN
#if SPH_SPARCV9_GCC_64 && !SPH_NO_ASM
  sph_u64 tmp;

  __asm__ __volatile__ ("ldxa [%1]0x88,%0" : "=r" (tmp) : "r" (src));
  return tmp;
# 1981 "sph_types.h"
#else
  return sph_bswap64(*(const sph_u64 *)src);
#endif
#else
  return (sph_u64)(((const unsigned char *)src)[0])
         | ((sph_u64)(((const unsigned char *)src)[1]) << 8)
         | ((sph_u64)(((const unsigned char *)src)[2]) << 16)
         | ((sph_u64)(((const unsigned char *)src)[3]) << 24)
         | ((sph_u64)(((const unsigned char *)src)[4]) << 32)
         | ((sph_u64)(((const unsigned char *)src)[5]) << 40)
         | ((sph_u64)(((const unsigned char *)src)[6]) << 48)
         | ((sph_u64)(((const unsigned char *)src)[7]) << 56);
#endif
}

#endif

#endif

#endif
