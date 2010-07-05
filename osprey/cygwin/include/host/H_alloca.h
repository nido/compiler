
#ifndef H_ALLOCA_H
#define H_ALLOCA_H

#ifdef __GNUC__
/* no alloca available in cygwin. we use the gcc builtin */
#define alloca(size) __builtin_alloca(size)
#endif /* __GNUC__ */

#endif /* H_ALLOCA_H */
