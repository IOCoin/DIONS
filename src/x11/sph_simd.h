#ifndef SPH_SIMD_H__
#define SPH_SIMD_H__ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "sph_types.h"




#define SPH_SIZE_simd224 224




#define SPH_SIZE_simd256 256




#define SPH_SIZE_simd384 384




#define SPH_SIZE_simd512 512
# 77 "sph_simd.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[64];
  size_t ptr;
  sph_u32 state[16];
  sph_u32 count_low, count_high;
#endif
} sph_simd_small_context;
# 98 "sph_simd.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[128];
  size_t ptr;
  sph_u32 state[32];
  sph_u32 count_low, count_high;
#endif
} sph_simd_big_context;




typedef sph_simd_small_context sph_simd224_context;




typedef sph_simd_small_context sph_simd256_context;




typedef sph_simd_big_context sph_simd384_context;




typedef sph_simd_big_context sph_simd512_context;







void sph_simd224_init(void *cc);
# 144 "sph_simd.h"
void sph_simd224(void *cc, const void *data, size_t len);
# 155 "sph_simd.h"
void sph_simd224_close(void *cc, void *dst);
# 170 "sph_simd.h"
void sph_simd224_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_simd256_init(void *cc);
# 189 "sph_simd.h"
void sph_simd256(void *cc, const void *data, size_t len);
# 200 "sph_simd.h"
void sph_simd256_close(void *cc, void *dst);
# 215 "sph_simd.h"
void sph_simd256_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_simd384_init(void *cc);
# 234 "sph_simd.h"
void sph_simd384(void *cc, const void *data, size_t len);
# 245 "sph_simd.h"
void sph_simd384_close(void *cc, void *dst);
# 260 "sph_simd.h"
void sph_simd384_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_simd512_init(void *cc);
# 279 "sph_simd.h"
void sph_simd512(void *cc, const void *data, size_t len);
# 290 "sph_simd.h"
void sph_simd512_close(void *cc, void *dst);
# 305 "sph_simd.h"
void sph_simd512_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);
#ifdef __cplusplus
}
#endif

#endif
