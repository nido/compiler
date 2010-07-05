
#ifndef H_VALUES_H
#define H_VALUES_H

#ifdef CROSS_COMPILE

/*
  There is no values.h on Cygwin ... we need to define BITSPERBYTE 
*/

/* for CHAR_BIT */
#include <limits.h>

#define BITSPERBYTE (sizeof(char)*CHAR_BIT)

#endif /* CROSS_COMPILE */

#endif /* H_VALUES_H */
