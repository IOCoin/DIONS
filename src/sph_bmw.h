#ifndef SPH_BMW_H__
#define SPH_BMW_H__ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "sph_types.h"




#define SPH_SIZE_bmw224 224




#define SPH_SIZE_bmw256 256

#if SPH_64




#define SPH_SIZE_bmw384 384




#define SPH_SIZE_bmw512 512

#endif
# 80 "sph_bmw.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[64];
  size_t ptr;
  sph_u32 H[16];
#if SPH_64
  sph_u64 bit_count;
#else
  sph_u32 bit_count_high, bit_count_low;
#endif
#endif
} sph_bmw_small_context;





typedef sph_bmw_small_context sph_bmw224_context;





typedef sph_bmw_small_context sph_bmw256_context;

#if SPH_64
# 118 "sph_bmw.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[128];
  size_t ptr;
  sph_u64 H[16];
  sph_u64 bit_count;
#endif
} sph_bmw_big_context;





typedef sph_bmw_big_context sph_bmw384_context;





typedef sph_bmw_big_context sph_bmw512_context;

#endif







void sph_bmw224_init(void *cc);
# 158 "sph_bmw.h"
void sph_bmw224(void *cc, const void *data, size_t len);
# 169 "sph_bmw.h"
void sph_bmw224_close(void *cc, void *dst);
# 184 "sph_bmw.h"
void sph_bmw224_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_bmw256_init(void *cc);
# 203 "sph_bmw.h"
void sph_bmw256(void *cc, const void *data, size_t len);
# 214 "sph_bmw.h"
void sph_bmw256_close(void *cc, void *dst);
# 229 "sph_bmw.h"
void sph_bmw256_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);

#if SPH_64







void sph_bmw384_init(void *cc);
# 250 "sph_bmw.h"
void sph_bmw384(void *cc, const void *data, size_t len);
# 261 "sph_bmw.h"
void sph_bmw384_close(void *cc, void *dst);
# 276 "sph_bmw.h"
void sph_bmw384_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_bmw512_init(void *cc);
# 295 "sph_bmw.h"
void sph_bmw512(void *cc, const void *data, size_t len);
# 306 "sph_bmw.h"
void sph_bmw512_close(void *cc, void *dst);
# 321 "sph_bmw.h"
void sph_bmw512_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);

#endif

#ifdef __cplusplus
}
#endif

#endif
