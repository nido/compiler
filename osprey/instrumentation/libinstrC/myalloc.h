#undef INSTRUMENT
#undef EMBEDDED_DECOMP
#ifndef MYALLOC_H
#define MYALLOC_H

#ifdef INSTRUMENT
#ifdef EMBEDDED_DECOMP
#define MYMALLOC(size,stack_ptr) malloc_instrument_embedded(size,stack_ptr)
#define MYFREE(ptr,stack_ptr) free_instrument_embedded(ptr,stack_ptr)
#ifdef __cplusplus
extern "C" {
#endif
  extern void *malloc_instrument_embedded(size_t size, void *stack_ptr);
  extern void free_instrument_embedded(void *ptr, void *stack_ptr);
#ifdef __cplusplus
}
#endif
#else /* EMBEDDED_DECOMP */
#define MYMALLOC(size) malloc_instrument(size)
#define MYFREE(ptr) free_instrument(ptr)
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
  extern void *malloc_instrument(size_t size);
  extern void free_instrument(void *ptr);
#ifdef __cplusplus
}
#endif
#endif
#else  /* INSTRUMENT */
#ifdef EMBEDDED_DECOMP
#define MYMALLOC(size,stack_ptr) malloc_embedded(size,stack_ptr)
#define MYFREE(ptr,stack_ptr) free_embedded(ptr,stack_ptr)
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
  extern void *malloc_embedded(size_t size, void *stack_ptr);
  extern void free_embedded(void *ptr, void *stack_ptr);
#ifdef __cplusplus
}
#endif
#else
#include <stdlib.h>
#define MYMALLOC(size) malloc(size)
#define MYFREE(ptr) free(ptr)
#endif
#endif


#ifdef INSTRUMENT
#ifdef __cplusplus
extern "C" {
#endif
  extern void resetalloc(void);
  extern void allocatedsize(unsigned int *allocsize, unsigned int *maxallocsize);
#ifdef __cplusplus
}
#endif
#endif /* INSTRUMENT */
#endif /* MYALLOC_H */
