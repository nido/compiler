
#ifndef W_MMAN_H
#define W_MMAN_H

#include <host/config.h>

#ifndef USE_H_MMAN_H
/* use wait.h provided with the system */
#include <sys/mman.h>
#else
/* use H_wait.h wrapper */
#include "host/H_mman.h"
#endif

#endif /* W_WAIT_H */
