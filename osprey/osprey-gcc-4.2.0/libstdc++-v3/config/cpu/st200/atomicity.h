// Low-level functions for atomic operations: st200 version  -*- C++ -*-

// Copyright (C) 2009
// Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#include <ext/atomicity.h>

_GLIBCXX_BEGIN_NAMESPACE(__gnu_cxx)

_Atomic_word
__attribute__ ((__unused__))
__exchange_and_add (volatile _Atomic_word* __mem, int __val)
{
  register _Atomic_word __result;
  register int tmp;

#if defined(__st240__)
  __asm__
    ("1:\n"
     " ldwl %[result] = [%[__mem]]\n"
     ";;\n"
     " add %[tmp] = %[result],%[__val]\n"
     ";;\n"
     " stwl $b0, [%[__mem]] = %[tmp]\n"
     ";;\n"
     " brf  $b0, 1b\n"
     ";;\n"
     : [result] "=&r" (__result), [tmp] "=r" (tmp)
     : [__val] "r" (__val), [__mem] "r" (__mem)
     : "memory", "b0");
#else
  __asm__
    (" call $r63 = 1f\n"
     ";;\n"
     "1:\n"
     " mov $r62 = $r63\n"
     " or  $r12 = $r12, 1\n"
     " ldw %[result] = 0[%[__mem]]\n"
     ";;\n"
#  if defined(__st220__)
     " goto 1\n"
     ";;\n"
#  endif
     " add %[tmp] = %[result],%[__val]\n"
     ";;\n"
     " stw 0[%[__mem]] = %[tmp]\n"
     " and $r12 = $r12, ~1\n"
     : [result] "=&r" (__result), [tmp] "=r" (tmp)
     : [__val] "r" (__val), [__mem] "r" (__mem)
     : "memory", "r62", "r63");
#endif

  return __result;
}


void
__attribute__ ((__unused__))
__atomic_add (volatile _Atomic_word* __mem, int __val)
{
  register int tmp;

#if defined(__st240__)
  __asm__
    ("1:\n"
     " ldwl %[tmp] = [%[__mem]]\n"
     ";;\n"
     " add %[tmp] = %[tmp],%[__val]\n"
     ";;\n"
     " stwl $b0, [%[__mem]] = %[tmp]\n"
     ";;\n"
     " brf  $b0, 1b\n"
     ";;\n"
     : [tmp] "=&r" (tmp)
     : [__val] "r" (__val), [__mem] "r" (__mem)
     : "memory", "b0");
#else
  __asm__
    (" call $r63 = 1f\n"
     ";;\n"
     "1:\n"
     " mov $r62 = $r63\n"
     " or  $r12 = $r12, 1\n"
     " ldw %[tmp] = 0[%[__mem]]\n"
     ";;\n"
#  if defined(__st220__)
     " goto 1\n"
     ";;\n"
#  endif
     " add %[tmp] = %[tmp],%[__val]\n"
     ";;\n"
     " stw 0[%[__mem]] = %[tmp]\n"
     " and $r12 = $r12, ~1\n"
     : [tmp] "=&r" (tmp)
     : [__val] "r" (__val), [__mem] "r" (__mem)
     : "memory", "r62", "r63");
#endif

}

_GLIBCXX_END_NAMESPACE
