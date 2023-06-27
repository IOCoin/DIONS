#ifndef SPH_BLAKE_H__
#define SPH_BLAKE_H__ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "sph_types.h"




#define SPH_SIZE_blake224 224




#define SPH_SIZE_blake256 256

#if SPH_64




#define SPH_SIZE_blake384 384




#define SPH_SIZE_blake512 512

#endif
# 81 "sph_blake.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[64];
  size_t ptr;
  sph_u32 H[8];
  sph_u32 S[4];
  sph_u32 T0, T1;
#endif
} sph_blake_small_context;





typedef sph_blake_small_context sph_blake224_context;





typedef sph_blake_small_context sph_blake256_context;

#if SPH_64
# 116 "sph_blake.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[128];
  size_t ptr;
  sph_u64 H[8];
  sph_u64 S[4];
  sph_u64 T0, T1;
#endif
} sph_blake_big_context;





typedef sph_blake_big_context sph_blake384_context;





typedef sph_blake_big_context sph_blake512_context;

#endif







void sph_blake224_init(void *cc);
# 157 "sph_blake.h"
void sph_blake224(void *cc, const void *data, size_t len);
# 168 "sph_blake.h"
void sph_blake224_close(void *cc, void *dst);
# 183 "sph_blake.h"
void sph_blake224_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_blake256_init(void *cc);
# 202 "sph_blake.h"
void sph_blake256(void *cc, const void *data, size_t len);
# 213 "sph_blake.h"
void sph_blake256_close(void *cc, void *dst);
# 228 "sph_blake.h"
void sph_blake256_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);

#if SPH_64







void sph_blake384_init(void *cc);
# 249 "sph_blake.h"
void sph_blake384(void *cc, const void *data, size_t len);
# 260 "sph_blake.h"
void sph_blake384_close(void *cc, void *dst);
# 275 "sph_blake.h"
void sph_blake384_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_blake512_init(void *cc);
# 294 "sph_blake.h"
void sph_blake512(void *cc, const void *data, size_t len);
# 305 "sph_blake.h"
void sph_blake512_close(void *cc, void *dst);
# 320 "sph_blake.h"
void sph_blake512_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);

#endif

#ifdef __cplusplus
}
#endif

#endif
