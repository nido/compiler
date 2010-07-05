
#ifndef W_AR_H
#define W_AR_H

#include <host/config.h>

#ifndef USE_H_AR_H
/* use ar.h provided with the system */
#include <ar.h>
#else
/* use H_ar.h wrapper */
#include "host/H_ar.h"
#endif

#endif /* W_AR_H */
