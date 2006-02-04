
#ifndef H_ERRNO_H
#define H_ERRNO_H

#ifdef CROSS_COMPILE

#include<errno.h>

#define sys_nerr _sys_nerr
#define sys_errlist _sys_errlist

#else
#include<errno.h>
#endif

#endif /* H_ERRNO_H */
