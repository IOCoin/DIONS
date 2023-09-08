#ifndef SPH_SHAVITE_H__
#define SPH_SHAVITE_H__ 

#include <stddef.h>
#include "sph_types.h"

#ifdef __cplusplus
extern "C" {
#endif




#define SPH_SIZE_shavite224 224




#define SPH_SIZE_shavite256 256




#define SPH_SIZE_shavite384 384




#define SPH_SIZE_shavite512 512
# 78 "sph_shavite.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[64];
  size_t ptr;
  sph_u32 h[8];
  sph_u32 count0, count1;
#endif
} sph_shavite_small_context;





typedef sph_shavite_small_context sph_shavite224_context;





typedef sph_shavite_small_context sph_shavite256_context;
# 110 "sph_shavite.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[128];
  size_t ptr;
  sph_u32 h[16];
  sph_u32 count0, count1, count2, count3;
#endif
} sph_shavite_big_context;





typedef sph_shavite_big_context sph_shavite384_context;





typedef sph_shavite_big_context sph_shavite512_context;







void sph_shavite224_init(void *cc);
# 148 "sph_shavite.h"
void sph_shavite224(void *cc, const void *data, size_t len);
# 159 "sph_shavite.h"
void sph_shavite224_close(void *cc, void *dst);
# 174 "sph_shavite.h"
void sph_shavite224_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_shavite256_init(void *cc);
# 193 "sph_shavite.h"
void sph_shavite256(void *cc, const void *data, size_t len);
# 204 "sph_shavite.h"
void sph_shavite256_close(void *cc, void *dst);
# 219 "sph_shavite.h"
void sph_shavite256_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_shavite384_init(void *cc);
# 238 "sph_shavite.h"
void sph_shavite384(void *cc, const void *data, size_t len);
# 249 "sph_shavite.h"
void sph_shavite384_close(void *cc, void *dst);
# 264 "sph_shavite.h"
void sph_shavite384_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_shavite512_init(void *cc);
# 283 "sph_shavite.h"
void sph_shavite512(void *cc, const void *data, size_t len);
# 294 "sph_shavite.h"
void sph_shavite512_close(void *cc, void *dst);
# 309 "sph_shavite.h"
void sph_shavite512_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);

#ifdef __cplusplus
}
#endif

#endif
