
#ifndef W_LIMITS_H
#define W_LIMITS_H

#include <host/config.h>

#ifndef USE_H_LIMITS_H
/* use limits.h provided with the system */
#include <limits.h>
#else
/* use H_limits.h wrapper */
#include "host/H_limits.h"
#endif

#endif /* W_LIMITS_H */
