#ifndef SPH_SKEIN_H__
#define SPH_SKEIN_H__ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "sph_types.h"

#if SPH_64




#define SPH_SIZE_skein224 224




#define SPH_SIZE_skein256 256




#define SPH_SIZE_skein384 384




#define SPH_SIZE_skein512 512
# 83 "sph_skein.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[64];
  size_t ptr;
  sph_u64 h0, h1, h2, h3, h4, h5, h6, h7;
  sph_u64 bcount;
#endif
} sph_skein_big_context;




typedef sph_skein_big_context sph_skein224_context;




typedef sph_skein_big_context sph_skein256_context;




typedef sph_skein_big_context sph_skein384_context;




typedef sph_skein_big_context sph_skein512_context;







void sph_skein224_init(void *cc);
# 129 "sph_skein.h"
void sph_skein224(void *cc, const void *data, size_t len);
# 140 "sph_skein.h"
void sph_skein224_close(void *cc, void *dst);
# 155 "sph_skein.h"
void sph_skein224_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_skein256_init(void *cc);
# 174 "sph_skein.h"
void sph_skein256(void *cc, const void *data, size_t len);
# 185 "sph_skein.h"
void sph_skein256_close(void *cc, void *dst);
# 200 "sph_skein.h"
void sph_skein256_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_skein384_init(void *cc);
# 219 "sph_skein.h"
void sph_skein384(void *cc, const void *data, size_t len);
# 230 "sph_skein.h"
void sph_skein384_close(void *cc, void *dst);
# 245 "sph_skein.h"
void sph_skein384_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_skein512_init(void *cc);
# 264 "sph_skein.h"
void sph_skein512(void *cc, const void *data, size_t len);
# 275 "sph_skein.h"
void sph_skein512_close(void *cc, void *dst);
# 290 "sph_skein.h"
void sph_skein512_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);

#endif

#ifdef __cplusplus
}
#endif

#endif
