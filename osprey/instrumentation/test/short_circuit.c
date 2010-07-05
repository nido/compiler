/*******************************************************
 * C file translated from WHIRL Tue Jul  8 15:51:02 2003
 *******************************************************/

/* Include file-level type and variable decls */
#include "short_circuit.h"


_INT32 main()
{
  register _UINT32 pu_instrument_handle;
  register _INT32 __circuit_cond;
  register _INT32 __circuit_cond0;
  register _INT32 __cand_cior;
  register _INT32 __branch_cond1;
  register _INT32 __circuit_cond1;
  register _INT32 __circuit_cond2;
  register _INT32 __cand_cior0;
  register _INT32 __branch_cond0;
  register _INT32 __circuit_cond3;
  register _INT32 __circuit_cond4;
  register _INT32 __cand_cior1;
  register _INT32 __branch_cond;
  _INT32 i;
  _INT32 i1;
  _INT32 i3;
  _INT32 i4;
  _INT32 i6;
  
  __profile_init((_STRING *) anon, 0, 0);
  pu_instrument_handle = __profile_pu_init((_STRING *) anon0, (_STRING *) anon1, &main, 11);
  __profile_invoke_init(pu_instrument_handle, 1);
  __profile_branch_init(pu_instrument_handle, 3);
  __profile_loop_init(pu_instrument_handle, 1);
  __profile_short_circuit_init(pu_instrument_handle, 6);
  __profile_invoke(pu_instrument_handle, 0);
  i = 0;
  __profile_loop(pu_instrument_handle, 0);
  while(i <= 999)
  {
    __profile_loop_iter(pu_instrument_handle, 0);
    if(!(((_UINT32)(i) & 1U) != 0U))
      goto _3;
    __circuit_cond = (i % 3) != 0;
    __profile_short_circuit(pu_instrument_handle, 0, __circuit_cond == 0);
    if(!(__circuit_cond))
      goto _3;
    __circuit_cond0 = (i % 6) != 0;
    __profile_short_circuit(pu_instrument_handle, 1, __circuit_cond0 == 0);
    if(!(__circuit_cond0))
      goto _3;
    __cand_cior = 1;
    goto _6;
    _3 :;
    __cand_cior = 0;
    _6 :;
    __branch_cond1 = __cand_cior;
    __profile_branch(pu_instrument_handle, 2, __branch_cond1 != 0);
    if(__branch_cond1)
    {
      i6 = i;
      glob = i6 + glob;
    }
    else
    {
      if(!((i % 3) != 0))
        goto _9;
      __circuit_cond1 = ((_UINT32)(i) & 3U) != 0U;
      __profile_short_circuit(pu_instrument_handle, 2, __circuit_cond1 == 0);
      if(!(__circuit_cond1))
        goto _9;
      goto _7;
      _9 :;
      __circuit_cond2 = (i % 7) != 0;
      __profile_short_circuit(pu_instrument_handle, 3, __circuit_cond2 != 0);
      if(!(__circuit_cond2))
        goto _8;
      _7 :;
      __cand_cior0 = 1;
      goto _11;
      _8 :;
      __cand_cior0 = 0;
      _11 :;
      __branch_cond0 = __cand_cior0;
      __profile_branch(pu_instrument_handle, 1, __branch_cond0 != 0);
      if(__branch_cond0)
      {
        i4 = i;
        glob = glob + (i4 * 2);
      }
      else
      {
        if((i % 3) != 0)
          goto _12;
        __circuit_cond3 = ((_UINT32)(i) & 3U) != 0U;
        __profile_short_circuit(pu_instrument_handle, 4, __circuit_cond3 != 0);
        if(!(__circuit_cond3))
          goto _14;
        goto _12;
        _14 :;
        __circuit_cond4 = (i % 7) != 0;
        __profile_short_circuit(pu_instrument_handle, 5, __circuit_cond4 != 0);
        if(!(__circuit_cond4))
          goto _13;
        _12 :;
        __cand_cior1 = 1;
        goto _16;
        _13 :;
        __cand_cior1 = 0;
        _16 :;
        __branch_cond = __cand_cior1;
        __profile_branch(pu_instrument_handle, 0, __branch_cond != 0);
        if(__branch_cond)
        {
          i3 = i;
          glob = glob + (i3 * 2);
        }
        else
        {
          i1 = i;
          glob = glob - i1;
        }
      }
    }
    i = i + 5;
  }
  return 0;
} /* main */

