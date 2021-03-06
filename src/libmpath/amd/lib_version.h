

#ifndef LIB_VERSION_H_INCLUDED
#define LIB_VERSION_H_INCLUDED


#include <math.h>


#if defined(__linux)

#define PATH64_LIB_VERSION_SVID (_LIB_VERSION == _SVID_)
#define PATH64_LIB_VERSION_POSIX (_LIB_VERSION == _POSIX_)

#elif defined(__sun)

#define PATH64_LIB_VERSION_SVID 0
#define PATH64_LIB_VERSION_POSIX 0

#else

#error "You should define lib version macros for your platform"

#endif

#endif // LIB_VERSION_H_INCLUDED

