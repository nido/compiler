/*
 * Copyright (C) 2005, 2006, 2007 PathScale Inc.  All Rights Reserved.
 */
/*

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation version 3

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "em64tintrin.h" /* bug 13252 */

#define EM64T_L1_CACHE_BYTES 16384
#define EM64T_L2_CACHE_BYTES 2097152

static size_t __memcpy_pathscale_em64t_sse2
  (uint8_t *d, const uint8_t *s, size_t n) __attribute__ ((always_inline));

static size_t __memcpy_pathscale_em64t_sse2
  (uint8_t *d, const uint8_t *s, size_t n)
{
  assert(n >= 16);
  /* align destination up to 16 bytes */
  size_t i;
  size_t align = (16 - (((uintptr_t) d) & 0xf)) & 0xf;
  if (align != 0) {
    for (i = 0; i < align; i++) {
      d[i] = s[i];
    }
    d += align;
    s += align;
    n -= align;
  }

  __m128i *dp = (__m128i *) d;
  __m128i const *sp = (__m128i const *) s;

  if ((((uintptr_t) sp) & 0xf) == 0x0) {
    /* source and destination are both 16 byte aligned */
    if (n <= (EM64T_L1_CACHE_BYTES >> 1)) {
      size_t count = n >> 6;
      for (i = 0; i < count; i++) {

	/* Make sure gcc -finline-limit is set large enough to inline all the
	   _mm_load_si128 calls.  Bug 10343. */

        __m128i tmp0 = _mm_load_si128(sp);
        __m128i tmp1 = _mm_load_si128(sp + 1);
        __m128i tmp2 = _mm_load_si128(sp + 2);
        __m128i tmp3 = _mm_load_si128(sp + 3);
        _mm_store_si128(dp, tmp0);
        _mm_store_si128(dp + 1, tmp1);
        _mm_store_si128(dp + 2, tmp2);
        _mm_store_si128(dp + 3, tmp3);
        sp += 4;
        dp += 4;
      }
      return align + (count << 6);
    }
    else if (n <= (EM64T_L2_CACHE_BYTES >> 1)) {
      size_t count = n >> 7;
      for (i = 0; i < count; i++) {
        _mm_prefetch(((const char *) sp) + 512, _MM_HINT_NTA);
        _mm_prefetch(((const char *) sp) + 576, _MM_HINT_NTA);
        __m128i tmp0 = _mm_load_si128(sp);
        __m128i tmp1 = _mm_load_si128(sp + 1);
        __m128i tmp2 = _mm_load_si128(sp + 2);
        __m128i tmp3 = _mm_load_si128(sp + 3);
        __m128i tmp4 = _mm_load_si128(sp + 4);
        __m128i tmp5 = _mm_load_si128(sp + 5);
        __m128i tmp6 = _mm_load_si128(sp + 6);
        __m128i tmp7 = _mm_load_si128(sp + 7);
        _mm_store_si128(dp, tmp0);
        _mm_store_si128(dp + 1, tmp1);
        _mm_store_si128(dp + 2, tmp2);
        _mm_store_si128(dp + 3, tmp3);
        _mm_store_si128(dp + 4, tmp4);
        _mm_store_si128(dp + 5, tmp5);
        _mm_store_si128(dp + 6, tmp6);
        _mm_store_si128(dp + 7, tmp7);
        sp += 8;
        dp += 8;
      }
      return align + (count << 7);
    }
    else {
      size_t count = n >> 7;
      for (i = 0; i < count; i++) {
        _mm_prefetch(((const char *) sp) + 768, _MM_HINT_NTA);
        _mm_prefetch(((const char *) sp) + 832, _MM_HINT_NTA);
        __m128i tmp0 = _mm_load_si128(sp);
        __m128i tmp1 = _mm_load_si128(sp + 1);
        __m128i tmp2 = _mm_load_si128(sp + 2);
        __m128i tmp3 = _mm_load_si128(sp + 3);
        __m128i tmp4 = _mm_load_si128(sp + 4);
        __m128i tmp5 = _mm_load_si128(sp + 5);
        __m128i tmp6 = _mm_load_si128(sp + 6);
        __m128i tmp7 = _mm_load_si128(sp + 7);
        _mm_stream_si128(dp, tmp0);
        _mm_stream_si128(dp + 1, tmp1);
        _mm_stream_si128(dp + 2, tmp2);
        _mm_stream_si128(dp + 3, tmp3);
        _mm_stream_si128(dp + 4, tmp4);
        _mm_stream_si128(dp + 5, tmp5);
        _mm_stream_si128(dp + 6, tmp6);
        _mm_stream_si128(dp + 7, tmp7);
        sp += 8;
        dp += 8;
      }
      _mm_sfence();	/* to impose order on non-temporal stores */
      return align + (count << 7);
    }
  }
  else {
    /* only destination is 16 byte aligned - use unaligned loads */
    if (n <= (EM64T_L1_CACHE_BYTES >> 1)) {
      size_t count = n >> 6;
      for (i = 0; i < count; i++) {
        __m128i tmp0 = _mm_loadu_si128(sp);
        __m128i tmp1 = _mm_loadu_si128(sp + 1);
        __m128i tmp2 = _mm_loadu_si128(sp + 2);
        __m128i tmp3 = _mm_loadu_si128(sp + 3);
        _mm_store_si128(dp, tmp0);
        _mm_store_si128(dp + 1, tmp1);
        _mm_store_si128(dp + 2, tmp2);
        _mm_store_si128(dp + 3, tmp3);
        sp += 4;
        dp += 4;
      }
      return align + (count << 6);
    }
    else if (n <= (EM64T_L2_CACHE_BYTES >> 1)) {
      size_t count = n >> 7;
      for (i = 0; i < count; i++) {
	/* 2 x 64 bytes of prefetch matches 8 x 16 bytes of load/store */
        /* The prefetch distance was tuned empirically */
        _mm_prefetch(((const char *) sp) + 512, _MM_HINT_NTA);
        _mm_prefetch(((const char *) sp) + 576, _MM_HINT_NTA);
        __m128i tmp0 = _mm_loadu_si128(sp);
        __m128i tmp1 = _mm_loadu_si128(sp + 1);
        __m128i tmp2 = _mm_loadu_si128(sp + 2);
        __m128i tmp3 = _mm_loadu_si128(sp + 3);
        __m128i tmp4 = _mm_loadu_si128(sp + 4);
        __m128i tmp5 = _mm_loadu_si128(sp + 5);
        __m128i tmp6 = _mm_loadu_si128(sp + 6);
        __m128i tmp7 = _mm_loadu_si128(sp + 7);
        _mm_store_si128(dp, tmp0);
        _mm_store_si128(dp + 1, tmp1);
        _mm_store_si128(dp + 2, tmp2);
        _mm_store_si128(dp + 3, tmp3);
        _mm_store_si128(dp + 4, tmp4);
        _mm_store_si128(dp + 5, tmp5);
        _mm_store_si128(dp + 6, tmp6);
        _mm_store_si128(dp + 7, tmp7);
        sp += 8;
        dp += 8;
      }
      return align + (count << 7);
    }
    else {
      size_t count = n >> 7;
      for (i = 0; i < count; i++) {
	/* 2 x 64 bytes of prefetch matches 8 x 16 bytes of load/store */
        /* The prefetch distance was tuned empirically */
        _mm_prefetch(((const char *) sp) + 768, _MM_HINT_NTA);
        _mm_prefetch(((const char *) sp) + 832, _MM_HINT_NTA);
        __m128i tmp0 = _mm_loadu_si128(sp);
        __m128i tmp1 = _mm_loadu_si128(sp + 1);
        __m128i tmp2 = _mm_loadu_si128(sp + 2);
        __m128i tmp3 = _mm_loadu_si128(sp + 3);
        __m128i tmp4 = _mm_loadu_si128(sp + 4);
        __m128i tmp5 = _mm_loadu_si128(sp + 5);
        __m128i tmp6 = _mm_loadu_si128(sp + 6);
        __m128i tmp7 = _mm_loadu_si128(sp + 7);
        _mm_stream_si128(dp, tmp0);
        _mm_stream_si128(dp + 1, tmp1);
        _mm_stream_si128(dp + 2, tmp2);
        _mm_stream_si128(dp + 3, tmp3);
        _mm_stream_si128(dp + 4, tmp4);
        _mm_stream_si128(dp + 5, tmp5);
        _mm_stream_si128(dp + 6, tmp6);
        _mm_stream_si128(dp + 7, tmp7);
        sp += 8;
        dp += 8;
      }
      _mm_sfence();	/* to impose order on non-temporal stores */
      return align + (count << 7);
    }
  }
  return 0;	/* unreachable */
}

void *__memcpy_pathscale_em64t (void *dst, const void *src, size_t n)
{
  uint8_t *d = (uint8_t *) dst;
  const uint8_t *s = (const uint8_t *) src;

  /* Smaller copies are detected and handled first since they are
   * the most latency sensitive. Larger copies can have residual
   * parts left at the end that are smaller than the unrolled loop.
   * I use an outer do-loop to allow these cases to loop around to
   * the smaller copy code. */

  do {
    if (n < 16) {
      if (n & 0x8) {
        * (uint64_t *) d = * (const uint64_t *) s;
        d += 8;
        s += 8;
      }
      if (n & 0x4) {
        * (uint32_t *) d = * (const uint32_t *) s;
        d += 4;
        s += 4;
      }
      if (n & 0x2) {
        * (uint16_t *) d = * (const uint16_t *) s;
        d += 2;
        s += 2;
      }
      if (n & 0x1) {
        * (uint8_t *) d = * (const uint8_t *) s;
      }
      return dst;
    }
    else if (n < 64) {
      /* align destination up to 8 bytes */
      size_t i;
      size_t a = 8 - (((uintptr_t) d) & 0x7);
      if (a != 8) {
        for (i = 0; i < a; i++) {
          d[i] = s[i];
        }
        d += a;
        s += a;
        n -= a;
      }
      uint64_t *dp = (uint64_t *) d;
      const uint64_t *sp = (const uint64_t *) s;
      size_t count = n >> 3;
      i = 0;
      while (i < count - 1) {
        uint64_t tmp0 = sp[i];
        uint64_t tmp1 = sp[i + 1];
        dp[i] = tmp0;
        dp[i + 1] = tmp1;
	i += 2;
        if (i >= count - 1) {
          break;
	}
        uint64_t tmp2 = sp[i];
        uint64_t tmp3 = sp[i + 1];
        dp[i] = tmp2;
        dp[i + 1] = tmp3;
	i += 2;
      }
      size_t bytes = i << 3;
      if (n == bytes) {
        return dst;	/* short-cut to return */
      }
      d += bytes;
      s += bytes;
      n -= bytes;
    }
    else {
      size_t bytes = __memcpy_pathscale_em64t_sse2(d, s, n);
      assert(bytes > 0);
      d += bytes;
      s += bytes;
      n -= bytes;
    }
  } while (n > 0);

  return dst;
}
