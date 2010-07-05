
#ifndef H_SIGNAL_H
#define H_SIGNAL_H

#include <signal.h>

/* For MINGW32, need to provide kill function (defined in libSYS). */
#define NEED_KILL
#ifdef __cplusplus
extern "C" {
#endif
  extern int kill(pid_t pid, int sig);
#ifdef __cplusplus
}
#endif

/* signal functions .*/
#ifndef WIFSIGNALED
#define WIFSIGNALED(S) (((S) & 0xff) != 0 && ((S) & 0xff) != 0x7f)
#endif
#ifndef WTERMSIG
#define WTERMSIG(S) ((S) & 0x7f)
#endif
#ifndef WIFEXITED
#define WIFEXITED(S) (((S) & 0xff) == 0)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(S) (((S) & 0xff00) >> 8)
#endif

#endif /* H_SIGNAL_H */
