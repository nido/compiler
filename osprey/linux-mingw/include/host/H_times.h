
#ifndef H_TIMES_H
#define H_TIMES_H

#ifndef CROSS_COMPILE
#include <sys/times.h>

#else /* !CROSS_COMPILE */

/* For MINGW32, need to provide dlxx functions (defined in libSYS). */
#define NEED_TIMES

#include <sys/types.h>
#include <time.h>

/* Structure describing CPU time used by a process and its children.  */
struct tms  {
  clock_t tms_utime;		/* User CPU time.  */
  clock_t tms_stime;		/* System CPU time.  */
  
  clock_t tms_cutime;		/* User CPU time of dead children.  */
  clock_t tms_cstime;		/* System CPU time of dead children.  */
};


#ifdef __cplusplus
extern "C" {
#endif
  extern clock_t times(struct tms *t);
  
#ifdef __cplusplus
}
#endif

#ifndef HZ
#define HZ CLOCKS_PER_SEC
#endif

#endif /* ! CROSS_COMPILE */

#endif /* H_TIMES_H */
