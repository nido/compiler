
#ifndef H_BSTRING_H
#define H_BSTRING_H

#include <strings.h>

/* For MINGW32, need to provide bxx functions (defined in libiberty). */
#ifdef __cplusplus
extern "C" {
#endif
  extern void bzero(void *s, size_t n);
  extern void bcopy(const void *src, void *dest, size_t n);
#ifdef __cplusplus
}
#endif


#endif /* H_BSTRING_H */
