
#ifndef H_WAIT_H
#define H_WAIT_H

#ifdef CROSS_COMPILE
/* no wait.h ... use sys/wait.h instead */

#include <sys/wait.h>

#endif /* CROSS_COMPILE */

#endif /* H_WAIT_H */
