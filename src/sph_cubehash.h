#ifndef SPH_CUBEHASH_H__
#define SPH_CUBEHASH_H__ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "sph_types.h"




#define SPH_SIZE_cubehash224 224




#define SPH_SIZE_cubehash256 256




#define SPH_SIZE_cubehash384 384




#define SPH_SIZE_cubehash512 512
# 77 "sph_cubehash.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[32];
  size_t ptr;
  sph_u32 state[32];
#endif
} sph_cubehash_context;




typedef sph_cubehash_context sph_cubehash224_context;




typedef sph_cubehash_context sph_cubehash256_context;




typedef sph_cubehash_context sph_cubehash384_context;




typedef sph_cubehash_context sph_cubehash512_context;
# 113 "sph_cubehash.h"
void sph_cubehash224_init(void *cc);
# 123 "sph_cubehash.h"
void sph_cubehash224(void *cc, const void *data, size_t len);
# 134 "sph_cubehash.h"
void sph_cubehash224_close(void *cc, void *dst);
# 149 "sph_cubehash.h"
void sph_cubehash224_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);
# 159 "sph_cubehash.h"
void sph_cubehash256_init(void *cc);
# 169 "sph_cubehash.h"
void sph_cubehash256(void *cc, const void *data, size_t len);
# 180 "sph_cubehash.h"
void sph_cubehash256_close(void *cc, void *dst);
# 195 "sph_cubehash.h"
void sph_cubehash256_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);
# 205 "sph_cubehash.h"
void sph_cubehash384_init(void *cc);
# 215 "sph_cubehash.h"
void sph_cubehash384(void *cc, const void *data, size_t len);
# 226 "sph_cubehash.h"
void sph_cubehash384_close(void *cc, void *dst);
# 241 "sph_cubehash.h"
void sph_cubehash384_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);
# 251 "sph_cubehash.h"
void sph_cubehash512_init(void *cc);
# 261 "sph_cubehash.h"
void sph_cubehash512(void *cc, const void *data, size_t len);
# 272 "sph_cubehash.h"
void sph_cubehash512_close(void *cc, void *dst);
# 287 "sph_cubehash.h"
void sph_cubehash512_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);
#ifdef __cplusplus
}
#endif

#endif
