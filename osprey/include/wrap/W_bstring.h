
#ifndef W_BSTRING_H
#define W_BSTRING_H

#include <host/config.h>

#ifndef USE_H_BSTRING_H
/* use bstring.h provided with the system */
#include <bstring.h>
#else
/* use H_bstring.h wrapper */
#include "host/H_bstring.h"
#endif

#endif /* W_BSTRING_H */
