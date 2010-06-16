#ifndef BUILTINS_H
#define BUILTINS_H


/* Codes that identify the various built in functions
   so that expand_call can identify them quickly.  */

#define DEF_BUILTIN(ENUM, N, C, T, LT, B, F, NA, AT, IM, COND) ENUM,
enum built_in_function
{
#include "builtins.def"


  /* Complex division routines in libgcc.  These are done via builtins
     because emit_library_call_value can't handle complex values.  */
  BUILT_IN_COMPLEX_MUL_MIN,
  BUILT_IN_COMPLEX_MUL_MAX
    = BUILT_IN_COMPLEX_MUL_MIN
      + MAX_MODE_COMPLEX_FLOAT
      - MIN_MODE_COMPLEX_FLOAT,

  BUILT_IN_COMPLEX_DIV_MIN,
  BUILT_IN_COMPLEX_DIV_MAX
    = BUILT_IN_COMPLEX_DIV_MIN
      + MAX_MODE_COMPLEX_FLOAT
      - MIN_MODE_COMPLEX_FLOAT,

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
