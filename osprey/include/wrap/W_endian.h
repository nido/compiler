
#ifndef W_ENDIAN_H
#define W_ENDIAN_H

#include <host/config.h>

#ifndef USE_H_ENDIAN_H
/* use endian.h provided with the system */
#include <endian.h>
#else
/* use H_endian.h wrapper */
#include "host/H_endian.h"
#endif

#endif /* W_ENDIAN_H */
