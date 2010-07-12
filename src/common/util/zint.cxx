/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Copyright (C) STMicroelectronics All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/

/* ====================================================================
 * ====================================================================
 *
 * Module: zint.cxx
 *
 * Description:
 *
 * Implementation of unbounded integer.
 *
 *  The most positive value is treated as +infinity,
 *  the most negative value is treated as -infinity.
 *
 * ====================================================================
 * ====================================================================
 */

#include <limits.h>
#include "defs.h"
#include "errors.h"
#include "zint.h"

static const INT ZINT_BITS = 64;

static const INT64 CARRY = ((INT64)1 << (ZINT_BITS-1));

static const ZInt PlusInfValue = ZInt (ZINT_MAX);
static const ZInt MinusInfValue = ZInt (ZINT_MIN);

static INT64
add_zint (INT64 a, INT64 b)
{
  if (a != ZINT_MAX
      && a != ZINT_MIN)
    { INT64 temp = a; a = b; b = temp; }

  if (a == ZINT_MIN || a == ZINT_MAX)
    return a;
  else if (b == ZINT_MIN || b == ZINT_MAX)
    return b;
  else {
    INT64 res = a + b;
    if (((a ^ b) & CARRY) == 0) {
      if (((res ^ a) & CARRY) != 0) {
	res = (a < 0) ? ZINT_MIN : ZINT_MAX;
      }
    }
    return res;
  }
}

static INT64
sub_zint (INT64 a, INT64 b)
{
  if (a == ZINT_MIN || a == ZINT_MAX)
    return a;
  else if (b == ZINT_MIN)
    return ZINT_MAX;
  else if (b == ZINT_MAX)
    return ZINT_MIN;
  else {
    INT64 res = a - b;
    if (((a ^ b) & CARRY) != 0) {
      if (((res ^ a) & CARRY) != 0) {
	res = a < 0 ? ZINT_MIN : ZINT_MAX;
      }
    }
    return res;
  }
}

static INT64
div_zint (INT64 a, INT64 b)
{
  if (a == ZINT_MIN)
    return (b < 0) ? ZINT_MAX : ZINT_MIN;
  else if (a == ZINT_MAX)
    return (b < 0) ? ZINT_MIN : ZINT_MAX;
  else if (b == ZINT_MIN)
    return 0;
  else if (b == ZINT_MAX)
    return 0;
  else {
    INT64 res = a / b;
    return res;
  }
}

static INT64
mod_zint (INT64 a, INT64 b)
{
  if (a == ZINT_MIN || a == ZINT_MAX)
    return a;
  else if (b == ZINT_MIN)
    return -a;
  else if (b == ZINT_MAX)
    return a;
  else {
    INT64 res = a % b;
    return res;
  }
}

static INT64
bitand_zint (INT64 a, INT64 b)
{
  if ((a == ZINT_MIN || a == ZINT_MAX) && b)
    return a;
  else if ((b == ZINT_MIN || b == ZINT_MAX) && a)
    return b;
  else {
    INT64 res = a & b;
    return res;
  }
}

static INT64
bitor_zint (INT64 a, INT64 b)
{
  if ((a == ZINT_MIN || a == ZINT_MAX) && b != (INT64)-1)
    return a;
  else if ((b == ZINT_MIN || b == ZINT_MAX) && a != (INT64)-1)
    return b;
  else {
    INT64 res = a | b;
    return res;
  }
}

static INT64
neg_zint (INT64 a)
{
  return a == ZINT_MIN ? ZINT_MAX : -a;
}

static INT64
bitnot_zint (INT64 a)
{
  return (a == ZINT_MIN || a == ZINT_MAX) ? a : ~a;
}

static INT64
abs_zint (INT64 a)
{
  return a == ZINT_MIN ? ZINT_MAX : (a < 0 ? -a : a);
}

static INT64
max_zint (INT64 a, INT64 b)
{
  if (a == ZINT_MAX || b == ZINT_MIN)
    return a;
  else if (a == ZINT_MIN || b == ZINT_MAX)
    return b;
  else 
    return (a < b ? b : a);
}

static INT64
min_zint (INT64 a, INT64 b)
{
  if (a == ZINT_MAX || b == ZINT_MIN)
    return b;
  else if (a == ZINT_MIN || b == ZINT_MAX)
    return a;
  else 
    return (a < b ? a : b);
}

static INT64
mul_zint(INT64 a, INT64 b)
{
  int sign = 0;
  INT64 res;
  UINT64 a1, a2, b1, b2;
  UINT64 p1, p2, p3, p4, p5;
  
  if (a >= 1 && b == ZINT_MIN) return ZINT_MIN;
  if (b >= 1 && a == ZINT_MIN) return ZINT_MIN;

  if ((a ^ b) & CARRY) sign = 1;

  a = abs_zint(a);
  b = abs_zint(b);

  a1 = a & 0xFFFFFFFF;
  a2 = a >> 32;
  b1 = b & 0xFFFFFFFF;
  b2 = b >> 32;

  p1 = a1 * b1;
  p2 = a1 * b2;
  p2 = p2 & 0xFFFFFFFF00000000ull ? UINT64_MAX : p2 << 32;
  p3 = a2 * b1;
  p3 = p3 & 0xFFFFFFFF00000000ull ? UINT64_MAX : p3 << 32;
  p4 = a2*b2;
  p4 = p4 > 0 ? UINT64_MAX : 0;
  p5 = p1 | p2 | p3 | p4;

  if (sign)
    res = p5 > ZINT_MAX ? ZINT_MIN : INT64 (-p5);
  else
    res = p5 > ZINT_MAX ? ZINT_MAX : INT64 (p5);
  return res;
}

static INT64
shl_zint(INT64 a, INT64 n)
{
  INT64 res;
  FmtAssert (n >= 0, ("Negative shift in shl_zint"));
  if (n >= (8 * sizeof(a))) {
    // Either 0 or saturate.
    return (a == 0) ? 0 : ((a < 0) ? ZINT_MIN : ZINT_MAX);
  } else {
    // Saturate if we do not have enough sign bits.
    const INT bits = sizeof(a) * 8;
    if ((a >> ((bits - 1) - n)) != (a >> (bits - 1))) {
      return (a < 0) ? ZINT_MIN : ZINT_MAX;
    } else
      return a << n;
  }
}

static INT64
shr_zint(INT64 a, INT64 n)
{
  // Arithmetic shift right.
  FmtAssert (n >= 0, ("Negative shift in shr_zint"));

  // Infinities are unchanged.
  if (a == ZINT_MIN || a == ZINT_MAX)
    return a;
  else  if (n >= (8 * sizeof(a))) {
    // Either 0 or -1;
    return (a >= 0) ? 0 : -1;
  }
  else
    return a >> n;
}

ZInt::ZInt()
{
}

ZInt::ZInt(INT v)
{
  value = (INT64)v;
}

ZInt::ZInt(INT64 v)
{
  if (v < ZINT_MIN)
    value = ZINT_MIN;
  else if (v > ZINT_MAX)
    value = ZINT_MAX;
  else
    value = v;
}

ZInt::ZInt(UINT64 v)
{
  if (v > ZINT_MAX)
    value = ZINT_MAX;
  else
    value = v;
}

ZInt::ZInt(const ZInt& v)
{
  value = v.value;
}

const ZInt operator+(const ZInt& a, const ZInt& b)
{
  return ZInt (add_zint (a.value, b.value));
}

ZInt& operator+=(ZInt &a, const ZInt &b)
{
  a.value = add_zint (a.value, b.value);
  return a;
}

const ZInt operator-(const ZInt& a, const ZInt& b)
{
  return ZInt (sub_zint (a.value, b.value));
}

ZInt& operator-=(ZInt &a, const ZInt &b)
{
  a.value = sub_zint (a.value, b.value);
  return a;
}

const ZInt operator-(const ZInt& a)
{
  return ZInt (neg_zint (a.value));
}

const ZInt operator*(const ZInt& a, const ZInt& b)
{
  return ZInt (mul_zint (a.value, b.value));
}

const ZInt operator/(const ZInt& a, const ZInt& b)
{
  return ZInt (div_zint (a.value, b.value));
}

const ZInt operator%(const ZInt& a, const ZInt& b)
{
  return ZInt (mod_zint (a.value, b.value));
}

const ZInt operator|(const ZInt& a, const ZInt& b)
{
  return ZInt (bitor_zint (a.value, b.value));
}

const ZInt operator&(const ZInt& a, const ZInt& b)
{
  return ZInt (bitand_zint (a.value, b.value));
}

const ZInt operator~(const ZInt& a)
{
  return ZInt (bitnot_zint (a.value));
}

const ZInt operator<<(const ZInt &a, const ZInt &b)
{
  return ZInt (shl_zint (a.value, b.value));
}

const ZInt operator>>(const ZInt& a, const ZInt &b)
{
  return ZInt (shr_zint (a.value, b.value));
}

const BOOL operator>(const ZInt &a, const ZInt &b)
{
  return a.value > b.value;
}

const BOOL operator>=(const ZInt &a, const ZInt &b)
{
  return a.value >= b.value;
}

const BOOL operator<(const ZInt &a, const ZInt &b)
{
  return a.value < b.value;
}

const BOOL operator<=(const ZInt &a, const ZInt &b)
{
  return a.value <= b.value;
}

const BOOL operator==(const ZInt &a, const ZInt &b)
{
  return a.value == b.value;
}

const BOOL operator!=(const ZInt &a, const ZInt &b)
{
  return a.value != b.value;
}

const ZInt abs(const ZInt& a)
{
  return ZInt (abs_zint (a.value));
}

const ZInt Min(const ZInt& a, const ZInt& b)
{
  return ZInt (min_zint (a.value, b.value));
}

const ZInt Max(const ZInt& a, const ZInt& b)
{
  return ZInt (max_zint (a.value, b.value));
}


INT64  const
ZInt::to_INT64 () const
{
  FmtAssert (isFinite(), ("Attempt to convert non-finite ZInt to INT64"));
  return value;
}

INT  const
ZInt::to_INT () const
{
  FmtAssert (isFinite(), ("Attempt to convert non-finite ZInt to INT64"));
  return (INT)value;
}

const ZInt
ZInt::PlusInf ()
{
  return PlusInfValue;
}

const ZInt
ZInt::MinusInf ()
{
  return MinusInfValue;
}

const INT
ZInt::bits () const
{
  if (! isFinite ())
    return INT_MAX;

  INT msb = 0;
  INT64 v = value;
  if (v <= 0) {
    msb = 1;
  }
  if (v < 0) {
    v = ~v;
  }
  while (v) {
    v >>= 1;
    msb++;
  }
  return msb;
}

const INT
ZInt::highest_set_bit () const
{
  if (value < 0) {
    return INT_MAX;
  } else {
    return bits () - 1;
  }
}

void
ZInt::Print (FILE *f) const
{
  if (isMinusInf())
    fputs ("-Inf", f);
  else if (isPlusInf())
    fputs ("+Inf", f);
  else
    fprintf (f, "%"SCNd64"", (INT64)value);
}