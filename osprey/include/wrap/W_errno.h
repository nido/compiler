
#ifndef W_ERRNO_H
#define W_ERRNO_H

#include <host/config.h>

#ifndef USE_H_ERRNO_H
/* use errno.h provided with the system */
#include <errno.h>
#else
/* use H_errno.h wrapper */
#include "host/H_errno.h"
#endif

#endif /* W_ERRNO_H */
