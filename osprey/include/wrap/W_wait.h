
#ifndef W_WAIT_H
#define W_WAIT_H

#include <host/config.h>

#ifndef USE_H_WAIT_H
/* use wait.h provided with the system */
#include <wait.h>
#else
/* use H_wait.h wrapper */
#include "host/H_wait.h"
#endif

#endif /* W_WAIT_H */
