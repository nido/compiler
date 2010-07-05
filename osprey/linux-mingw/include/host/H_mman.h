
#ifndef H_MMAN_H
#define H_MMAN_H

#ifndef CROSS_COMPILE
#include <sys/mman.h>

#else /* !CROSS_COMPILE */

#include <sys/types.h>
/* For MINGW32, need to provide mmap/munmap function (defined in libSYS). */
#define NEED_MMAP
#define NEED_MUNMAP
#ifdef __cplusplus
extern "C" {
#endif
  extern void *mmap(void * x, size_t y, int z, int k, int l, off_t m);
  extern int munmap(void * x,  size_t y);
#ifdef __cplusplus
}
#endif

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2

#define MAP_FILE 0
#define MAP_SHARED 1
#define MAP_PRIVATE 2
#define MAP_TYPE 0xF

#define MAP_FAILED ((void *)-1)

#endif /* ! CROSS_COMPILE */

#endif /* H_MMAN_H */
