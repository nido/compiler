
#ifndef H_LIMITS_H
#define H_LIMITS_H

#include <limits.h>

#define LONGLONG_MAX  LONG_LONG_MAX
#define LONGLONG_MIN  LONG_LONG_MIN
#define ULONGLONG_MAX ULONG_LONG_MAX

#ifndef PATH_MAX
#define PATH_MAX MAXPATHLEN
#endif

#endif /* H_LIMITS_H */
