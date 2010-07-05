
#ifndef H_REGEX_H
#define H_REGEX_H

#ifndef CROSS_COMPILE

#include <regex.h>

#else /* !CROSS_COMPILE */

/* Include file from libiberty. */
#include "libiberty/xregex.h"

#endif /* !CROSS_COMPILE */

#endif /* H_REGEX_H */
