
#ifndef H_WAIT_H
#define H_WAIT_H

#include <signal.h>

/* For MINGW32, need to provide wait/waitpid functions (defined in libSYS). */
#define NEED_WAIT
#define NEED_WAITPID
#ifdef __cplusplus
extern "C" {
#endif
  extern pid_t wait(int *status);
  extern pid_t waitpid(pid_t pid, int *status, int options);
#ifdef __cplusplus
}
#endif

#define WNOHANG         1       /* Don't block waiting.  */
#define WUNTRACED       2       /* Report status of stopped children.  */


#endif /* H_WAIT_H */
