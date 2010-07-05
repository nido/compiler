
#ifndef W_SYS_WAIT_H
#define W_SYS_WAIT_H

#include <host/config.h>

#ifndef USE_H_SYS_WAIT_H
/* use sys/wait.h provided with the system */
#include <sys/wait.h>
#else
/* use sys/H_wait.h wrapper */
#include "host/sys/H_wait.h"
#endif

#endif /* W_SYS_WAIT_H */
