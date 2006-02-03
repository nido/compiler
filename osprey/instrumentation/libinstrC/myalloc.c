#include "myalloc.h"
#include <assert.h>
#ifndef EMBEDDED_DECOMP
/* For malloc */
#include <stdlib.h>
#include <stdio.h>

#endif
#ifdef EMBEDDED_DECOMP
#define ALIGN(addr,align) (((addr) + ~(-align)) & (-align))

void *malloc_embedded(size_t size, void *stack_ptr) {
  void* ptr; 
  ptr = *(void**)stack_ptr;

  size = ALIGN(size, __alignof__(long long)),
  *(void**)stack_ptr = (void*)(((unsigned char*)ptr) + (size));
/*   fprintf(stdout,"malloc_embedded: size allocated: %d\n", size); */

  return ptr;
}

void free_embedded(void *ptr, void *stack_ptr) {
}
#endif /*EMBEDDED_DECOMP */

#ifdef INSTRUMENT
static unsigned int allocated_size = 0;
static unsigned int max_allocated_size = 0;
void resetalloc()
{
  allocated_size = 0;
  max_allocated_size = 0;
}

void allocatedsize(unsigned int *allocsize, unsigned int *maxallocsize)
{
  *allocsize = allocated_size;
  *maxallocsize = max_allocated_size;
}

#endif /* INSTRUMENT */

#ifdef INSTRUMENT
#ifdef EMBEDDED_DECOMP
void *malloc_instrument_embedded(size_t size, void *stack_ptr) {
  size_t my_size = size + sizeof(size_t);
  size_t *ptr = (size_t *)malloc_embedded(my_size, stack_ptr);
  ptr[0] = size;
  allocated_size += size;
  max_allocated_size = max_allocated_size > allocated_size ? max_allocated_size : allocated_size;
/*   fprintf(stderr,"malloc_instrument_embedded: size allocated: %d, max_alloc:%d:\n", size, max_allocated_size); */
  return (void*) &ptr[1];
}

void free_instrument_embedded(void *ptr, void *stack_ptr) {
  size_t my_size;
  size_t *my_ptr = (size_t*)ptr;
  my_ptr--;
  my_size = my_ptr[0];
  allocated_size -= my_size;
  assert(allocated_size>=0);
  free_embedded(my_ptr, stack_ptr);
}
#else /* EMBEDDED_DECOMP */
void *malloc_instrument(size_t size) {
  size_t my_size = size + sizeof(size_t);
  size_t *ptr = (size_t *)malloc(my_size);
/*   static size_t print_size = 0; */
  if (ptr == NULL) {
    fprintf(stderr,"malloc error!\n");
    exit(1);
  }
  ptr[0] = size;
  allocated_size += size;
  max_allocated_size = max_allocated_size > allocated_size ? max_allocated_size : allocated_size;
/*   if (max_allocated_size > print_size) { */
/*     print_size = max_allocated_size + 1000; */
/*     fprintf(stdout,"malloc_instrument: size allocated: %d, max_allocated_size:%d\n", size, max_allocated_size); */
/*   } */
  return (void*) &ptr[1];
}
void free_instrument(void *ptr) {
  size_t my_size;
  size_t *my_ptr = (size_t*)ptr;
  my_ptr--;
  my_size = my_ptr[0];
  allocated_size -= my_size;
  assert(allocated_size>=0);
  free(my_ptr);
}
#endif
#endif /*INSTRUMENT*/

