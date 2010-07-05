
#ifndef H_LIMITS_H
#define H_LIMITS_H

#include <limits.h>

#ifdef CROSS_COMPILE

#define LONGLONG_MAX  LONG_LONG_MAX
#define LONGLONG_MIN  LONG_LONG_MIN
#define ULONGLONG_MAX ULONG_LONG_MAX

#ifndef PATH_MAX
#define PATH_MAX MAXPATHLEN
#endif

#else

#define LONGLONG_MAX LLONG_MAX
#define LONGLONG_MIN LLONG_MIN
#define ULONGLONG_MAX ULLONG_MAX

#endif /* CROSS_COMPILE */

#endif /* H_LIMITS_H */
