
#ifndef W_ALLOCA_H
#define W_ALLOCA_H

#include <host/config.h>

#ifndef USE_H_ALLOCA_H
/* use alloca.h provided with the system */
#include <alloca.h>
#else
/* use H_alloca.h wrapper */
#include "host/H_alloca.h"
#endif

#endif /* W_ALLOCA_H */
