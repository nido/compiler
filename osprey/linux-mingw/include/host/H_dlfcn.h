
#ifndef H_DLFCN_H
#define H_DLFCN_H

#ifndef CROSS_COMPILE
#include <dlfcn.h>

#else /* !CROSS_COMPILE */

/* For MINGW32, need to provide dlxx functions (defined in libSYS). */
#define NEED_DLFCN
#ifdef __cplusplus
extern "C" {
#endif
  extern void *dlopen(const char * x, int y);
  extern int dlclose(void * x);
  extern void *dlsym(void * x, const char *n);
  extern char *dlerror(void);
  
#ifdef __cplusplus
}
#endif

/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY       0x00001 /* Lazy function call binding.  */
#define RTLD_NOW        0x00002 /* Immediate function call binding.  */
#define RTLD_BINDING_MASK   0x3 /* Mask of binding time value.  */
#define RTLD_NOLOAD     0x00004 /* Do not load the object.  */
 
/* If the following bit is set in the MODE argument to `dlopen',
   the symbols of the loaded object and its dependencies are made
   visible as if the object were linked directly into the program.  */
#define RTLD_GLOBAL     0x00100
 
/* Unix98 demands the following flag which is the inverse to RTLD_GLOBAL.
   The implementation does this by default and so we can define the
   value to zero.  */
#define RTLD_LOCAL      0
 
/* Do not delete object when closed.  */
#define RTLD_NODELETE   0x01000


#endif /* ! CROSS_COMPILE */


#endif /* H_DLFCN_H */
