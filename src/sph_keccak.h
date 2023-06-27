#ifndef SPH_KECCAK_H__
#define SPH_KECCAK_H__ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "sph_types.h"




#define SPH_SIZE_keccak224 224




#define SPH_SIZE_keccak256 256




#define SPH_SIZE_keccak384 384




#define SPH_SIZE_keccak512 512
# 76 "sph_keccak.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[144];
  size_t ptr, lim;
  union
  {
#if SPH_64
    sph_u64 wide[25];
#endif
    sph_u32 narrow[50];
  } u;
#endif
} sph_keccak_context;




typedef sph_keccak_context sph_keccak224_context;




typedef sph_keccak_context sph_keccak256_context;




typedef sph_keccak_context sph_keccak384_context;




typedef sph_keccak_context sph_keccak512_context;







void sph_keccak224_init(void *cc);
# 127 "sph_keccak.h"
void sph_keccak224(void *cc, const void *data, size_t len);
# 138 "sph_keccak.h"
void sph_keccak224_close(void *cc, void *dst);
# 153 "sph_keccak.h"
void sph_keccak224_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_keccak256_init(void *cc);
# 172 "sph_keccak.h"
void sph_keccak256(void *cc, const void *data, size_t len);
# 183 "sph_keccak.h"
void sph_keccak256_close(void *cc, void *dst);
# 198 "sph_keccak.h"
void sph_keccak256_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_keccak384_init(void *cc);
# 217 "sph_keccak.h"
void sph_keccak384(void *cc, const void *data, size_t len);
# 228 "sph_keccak.h"
void sph_keccak384_close(void *cc, void *dst);
# 243 "sph_keccak.h"
void sph_keccak384_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_keccak512_init(void *cc);
# 262 "sph_keccak.h"
void sph_keccak512(void *cc, const void *data, size_t len);
# 273 "sph_keccak.h"
void sph_keccak512_close(void *cc, void *dst);
# 288 "sph_keccak.h"
void sph_keccak512_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);

#ifdef __cplusplus
}
#endif

#endif
