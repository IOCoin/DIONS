#ifndef SPH_LUFFA_H__
#define SPH_LUFFA_H__ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "sph_types.h"




#define SPH_SIZE_luffa224 224




#define SPH_SIZE_luffa256 256




#define SPH_SIZE_luffa384 384




#define SPH_SIZE_luffa512 512
# 76 "sph_luffa.h"
typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[32];
  size_t ptr;
  sph_u32 V[3][8];
#endif
} sph_luffa224_context;





typedef sph_luffa224_context sph_luffa256_context;




typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[32];
  size_t ptr;
  sph_u32 V[4][8];
#endif
} sph_luffa384_context;




typedef struct
{
#ifndef DOXYGEN_IGNORE
  unsigned char buf[32];
  size_t ptr;
  sph_u32 V[5][8];
#endif
} sph_luffa512_context;







void sph_luffa224_init(void *cc);
# 131 "sph_luffa.h"
void sph_luffa224(void *cc, const void *data, size_t len);
# 142 "sph_luffa.h"
void sph_luffa224_close(void *cc, void *dst);
# 157 "sph_luffa.h"
void sph_luffa224_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_luffa256_init(void *cc);
# 176 "sph_luffa.h"
void sph_luffa256(void *cc, const void *data, size_t len);
# 187 "sph_luffa.h"
void sph_luffa256_close(void *cc, void *dst);
# 202 "sph_luffa.h"
void sph_luffa256_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_luffa384_init(void *cc);
# 221 "sph_luffa.h"
void sph_luffa384(void *cc, const void *data, size_t len);
# 232 "sph_luffa.h"
void sph_luffa384_close(void *cc, void *dst);
# 247 "sph_luffa.h"
void sph_luffa384_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);







void sph_luffa512_init(void *cc);
# 266 "sph_luffa.h"
void sph_luffa512(void *cc, const void *data, size_t len);
# 277 "sph_luffa.h"
void sph_luffa512_close(void *cc, void *dst);
# 292 "sph_luffa.h"
void sph_luffa512_addbits_and_close(
  void *cc, unsigned ub, unsigned n, void *dst);

#ifdef __cplusplus
}
#endif

#endif
