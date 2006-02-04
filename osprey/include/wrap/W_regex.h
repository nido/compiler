
#ifndef W_REGEX_H
#define W_REGEX_H

#include <host/config.h>

#ifndef USE_H_REGEX_H
/* use regex.h provided with the system */
#include <regex.h>
#else
/* use H_regex.h wrapper */
#include "host/H_regex.h"
#endif

#endif /* W_STDINT_H */
