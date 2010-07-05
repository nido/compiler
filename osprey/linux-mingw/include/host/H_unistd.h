
#ifndef H_UNISTD_H
#define H_UNISTD_H

#include <unistd.h>

/* For MINGW32, need to provide pipe/fork functions (defined in libSYS). */
#define NEED_PIPE
#define NEED_FORK
#define NEED_SBRK
#define NEED_SYMLINK
#define NEED_LINK
#define NEED_GETRUSAGE
#define NEED_GETRLIMIT
#ifdef __cplusplus
extern "C" {
#endif
  extern pid_t fork(void);
  extern int pipe(int filedes[2]);
  extern void *sbrk(ptrdiff_t inc);
  extern int symlink(const char *oldpath, const char *newpath);
  extern int link(const char *oldpath, const char *newpath);
  struct rusage;
  extern int getrusage(int who, struct rusage *usage);
  struct rlimit;
  extern int getrlimit(int resource, struct rlimit *rlim);

#ifdef __cplusplus
}
#endif

#define RUSAGE_SELF 0
#define RUSAGE_CHILDREN -1
#define RUSAGE_BOTH -2


#endif /* H_UNISTD_H */
