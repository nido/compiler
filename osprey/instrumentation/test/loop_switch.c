/*******************************************************
 * C file translated from WHIRL Fri Jul  4 10:51:53 2003
 *******************************************************/

/* Include file-level type and variable decls */
#include "loop_switch.h"


_INT32 branch_exple(
  _INT32 aaa)
{
  register _UINT32 pu_instrument_handle;
  register _INT32 __branch_cond;
  register _UINT32 reg17;
  
  __profile_init((_STRING *) anon, 0, 1);
  pu_instrument_handle = __profile_pu_init((_STRING *) anon0, (_STRING *) anon1, &branch_exple, 2);
  __profile_invoke_init(pu_instrument_handle, 1);
  __profile_branch_init(pu_instrument_handle, 1);
  __profile_invoke(pu_instrument_handle, 0);
  __branch_cond = aaa != 0;
  __profile_branch(pu_instrument_handle, 0, __branch_cond != 0);
  if(__branch_cond)
  {
    return 1;
  }
  else
  {
    return 0;
  }
  return reg17;
} /* branch_exple */


_INT32 switch_exple(
  _INT32 aaa)
{
  register _UINT32 pu_instrument_handle;
  register _INT32 __switch_cond;
  _INT32 res;
  static _INT32 switch_num_targets[1LL];
  static _INT64 switch_case_values[6LL];
  
  __profile_init((_STRING *) anon, 0, 1);
  pu_instrument_handle = __profile_pu_init((_STRING *) anon0, (_STRING *) anon2, &switch_exple, 2);
  __profile_invoke_init(pu_instrument_handle, 1);
  switch_num_targets[0] = 5;
  switch_case_values[0] = 0LL;
  switch_case_values[1] = 1LL;
  switch_case_values[2] = (2LL);
  switch_case_values[3] = (3LL);
  switch_case_values[4] = (4LL);
  switch_case_values[5] = (5LL);
  __profile_switch_init(pu_instrument_handle, 1, &switch_num_targets, 6, &switch_case_values);
  __profile_invoke(pu_instrument_handle, 0);
  __switch_cond = aaa;
  __profile_switch(pu_instrument_handle, 0, __switch_cond, 5);
  if(__switch_cond == 0)
    goto _1;
  if(__switch_cond == 1)
    goto _3;
  if(__switch_cond == 2)
    goto _4;
  if(__switch_cond == 3)
    goto _5;
  if(__switch_cond == 4)
    goto _6;
  if(__switch_cond == 5)
    goto _7;
  goto _8;
  _1 :;
  res = 9;
  goto _2;
  _3 :;
  res = 7;
  goto _2;
  _4 :;
  res = 5;
  goto _2;
  _5 :;
  res = 8;
  goto _2;
  _6 :;
  res = 54;
  goto _2;
  _7 :;
  res = 2;
  goto _2;
  _8 :;
  res = 0;
  goto _2;
  _2 :;
  return res;
} /* switch_exple */

_INT32 a;

_INT32 main()
{
  register _UINT32 pu_instrument_handle;
  register _INT32 __call_comma;
  register _INT32 __call_comma0;
  _INT32 j;
  _INT32 i;
  
  __profile_init((_STRING *) anon, 0, 0);
  pu_instrument_handle = __profile_pu_init((_STRING *) anon0, (_STRING *) anon3, &main, 5);
  __profile_invoke_init(pu_instrument_handle, 1);
  __profile_loop_init(pu_instrument_handle, 2);
  __profile_call_init(pu_instrument_handle, 2);
  __profile_invoke(pu_instrument_handle, 0);
  a = 1;
  j = 1;
  __profile_loop(pu_instrument_handle, 1);
  while(j <= 2)
  {
    __profile_loop_iter(pu_instrument_handle, 1);
    i = 1;
    __profile_loop(pu_instrument_handle, 0);
    while(i <= 887)
    {
      __profile_loop_iter(pu_instrument_handle, 0);
      __profile_call_entry(pu_instrument_handle, 0, (_STRING *) anon1);
      __call_comma = branch_exple(i % 3);
      __profile_call_exit(pu_instrument_handle, 0, (_STRING *) anon1);
      a = a + __call_comma;
      __profile_call_entry(pu_instrument_handle, 1, (_STRING *) anon2);
      __call_comma0 = switch_exple(i % 3);
      __profile_call_exit(pu_instrument_handle, 1, (_STRING *) anon2);
      a = a + __call_comma0;
      i = i + 1;
    }
    j = j + 1;
  }
  return 0;
} /* main */

