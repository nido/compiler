#ifndef BUILTINS_H
#define BUILTINS_H


/* Codes that identify the various built in functions
   so that expand_call can identify them quickly.  */

#define DEF_BUILTIN(ENUM, N, C, T, LT, B, F, NA, AT) ENUM,
enum built_in_function
{
#include "builtins.def"

  /* Upper bound on non-language-specific builtins.  */
  END_BUILTINS
};
// BUILT_IN_STATIC_COUNT is the count of staticaly defined builtins
#define BUILT_IN_STATIC_COUNT END_BUILTINS
// BUILT_IN_STATIC_LAST is the last builtin staticaly known
#define BUILT_IN_STATIC_LAST (END_BUILTINS-1)
extern enum built_in_function BUILT_IN_COUNT; 

#undef DEF_BUILTIN

#endif
