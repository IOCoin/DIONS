#ifndef SPH_JH_H__
#define SPH_JH_H__ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "sph_types.h"




#define SPH_SIZE_jh224 224




#define SPH_SIZE_jh256 256




#define SPH_SIZE_jh384 384




#define SPH_SIZE_jh512 512
# 76 "sph_jh.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[64];
  size_t ptr;
  union
  {
#if SPH_64
    sph_u64 wide[16];
#endif
    sph_u32 narrow[32];
  } H;
#if SPH_64
  sph_u64 block_count;
#else
  sph_u32 block_count_high, block_count_low;
#endif
#endif
} sph_jh_context;




typedef sph_jh_context sph_jh224_context;




typedef sph_jh_context sph_jh256_context;




typedef sph_jh_context sph_jh384_context;




typedef sph_jh_context sph_jh512_context;







void sph_jh224_init(void *cc);
# 132 "sph_jh.h"
void sph_jh224(void *cc, const void *data, size_t len);
# 143 "sph_jh.h"
void sph_jh224_close(void *cc, void *dst);
# 158 "sph_jh.h"
void sph_jh224_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_jh256_init(void *cc);
# 177 "sph_jh.h"
void sph_jh256(void *cc, const void *data, size_t len);
# 188 "sph_jh.h"
void sph_jh256_close(void *cc, void *dst);
# 203 "sph_jh.h"
void sph_jh256_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_jh384_init(void *cc);
# 222 "sph_jh.h"
void sph_jh384(void *cc, const void *data, size_t len);
# 233 "sph_jh.h"
void sph_jh384_close(void *cc, void *dst);
# 248 "sph_jh.h"
void sph_jh384_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_jh512_init(void *cc);
# 267 "sph_jh.h"
void sph_jh512(void *cc, const void *data, size_t len);
# 278 "sph_jh.h"
void sph_jh512_close(void *cc, void *dst);
# 293 "sph_jh.h"
void sph_jh512_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);

#ifdef __cplusplus
}
#endif

#endif
