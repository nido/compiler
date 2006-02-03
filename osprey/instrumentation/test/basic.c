/*******************************************************
 * C file translated from WHIRL Mon Jul  7 15:24:33 2003
 *******************************************************/

/* Include file-level type and variable decls */
#include "basic.h"


_INT32 main()
{
  register _UINT32 pu_instrument_handle;
  
  __profile_init((_STRING *) anon, 0, 0);
  pu_instrument_handle = __profile_pu_init((_STRING *) anon0, (_STRING *) anon1, &main, 1);
  __profile_invoke_init(pu_instrument_handle, 1);
  __profile_invoke(pu_instrument_handle, 0);
  return 0;
} /* main */

