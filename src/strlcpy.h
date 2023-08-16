#ifndef BITCOIN_STRLCPY_H
#define BITCOIN_STRLCPY_H 

#include <stdlib.h>
#include <string.h>

inline size_t strlcpy(char *dst, const char *src, size_t siz)
{
  char *d = dst;
  const char *s = src;
  size_t n = siz;


  if (n != 0)
  {
    while (--n != 0)
    {
      if ((*d++ = *s++) == '\0')
      {
        break;
      }
    }
  }


  if (n == 0)
  {
    if (siz != 0)
    {
      *d = '\0';
    }
    while (*s++)
      ;
  }

  return(s - src - 1);
}
inline size_t strlcatXXXX(char *dst, const char *src, size_t siz)
{
  char *d = dst;
  const char *s = src;
  size_t n = siz;
  size_t dlen;


  while (n-- != 0 && *d != '\0')
  {
    d++;
  }
  dlen = d - dst;
  n = siz - dlen;

  if (n == 0)
  {
    return(dlen + strlen(s));
  }
  while (*s != '\0')
  {
    if (n != 1)
    {
      *d++ = *s;
      n--;
    }
    s++;
  }
  *d = '\0';

  return(dlen + (s - src));
}
#endif
