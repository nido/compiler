
#ifndef H_BSTRING_H
#define H_BSTRING_H

#if !defined(__open64_) && !defined(__ST200)
#include <strings.h>
#else
/* [TB] When compiling with st200cc include string.h instead */
#include <string.h>
#endif
#endif /* H_BSTRING_H */
