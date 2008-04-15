/* Modified by STMicroelectronics 2005 */
/* Generic threads compatibility routines for libgcc2 and libobjc. */
/* Compile this one with gcc.  */
/* Copyright (C) 1997, 1999, 2000, 2002 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* As a special exception, if you link this library with other files,
   some of which are compiled with GCC, to produce an executable,
   this library does not by itself cause the resulting executable
   to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

#ifndef __gthr_generic_h
#define __gthr_generic_h

#define __GTHREADS 1

#define __GTHREAD_ONCE_INIT 0
#define __GTHREAD_MUTEX_INIT_FUNCTION __gthread_mutex_init_function
#define __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION __gthread_recursive_mutex_init_function

#ifdef __cplusplus
extern "C" {
#endif

/* avoid depedency on specific headers */
typedef void *__gthread_key_t;
typedef void *__gthread_once_t;
typedef struct __gthread_mutex_s { void *p; } __gthread_mutex_t;
typedef struct __gthread_recursive_mutex_s { void *p; } __gthread_recursive_mutex_t;

/* We should always link with at least one definition, so we want strong
   references.  The stub definitions are weak so that they can be overriden.  */
#ifndef __GTHR_WEAK
#define __GTHR_WEAK
#endif

extern int __generic_gxx_active_p (void) __GTHR_WEAK;
extern int __generic_gxx_once (__gthread_once_t *, void (*)(void)) __GTHR_WEAK;
extern int __generic_gxx_key_create (__gthread_key_t *,
				     void (*)(void *)) __GTHR_WEAK;
extern int __generic_gxx_key_dtor (__gthread_key_t, void *) __GTHR_WEAK;
extern int __generic_gxx_key_delete (__gthread_key_t key) __GTHR_WEAK;
extern void *__generic_gxx_getspecific (__gthread_key_t key) __GTHR_WEAK;
extern int __generic_gxx_setspecific (__gthread_key_t, const void *) __GTHR_WEAK;
extern void __generic_gxx_mutex_init_function (__gthread_mutex_t *) __GTHR_WEAK;
extern int __generic_gxx_mutex_lock (__gthread_mutex_t *) __GTHR_WEAK;
extern int __generic_gxx_mutex_trylock (__gthread_mutex_t *) __GTHR_WEAK;
extern int __generic_gxx_mutex_unlock (__gthread_mutex_t *) __GTHR_WEAK;
extern void __generic_gxx_recursive_mutex_init_function (__gthread_recursive_mutex_t *) __GTHR_WEAK;
extern int __generic_gxx_recursive_mutex_lock (__gthread_recursive_mutex_t *) __GTHR_WEAK;
extern int __generic_gxx_recursive_mutex_trylock (__gthread_recursive_mutex_t *) __GTHR_WEAK;
extern int __generic_gxx_recursive_mutex_unlock (__gthread_recursive_mutex_t *) __GTHR_WEAK;

#ifdef __cplusplus
}
#endif

#ifdef _LIBOBJC

#error "Objective-C not supported by generic thread support package"

#else /* !_LIBOBJC */

static inline int
__gthread_active_p (void)
{
  return __generic_gxx_active_p ();
}

static inline int
__gthread_once (__gthread_once_t *once, void (*func)(void))
{
  return __generic_gxx_once (once, func);
}

static inline int
__gthread_key_create (__gthread_key_t *key, void (*dtor)(void *))
{
  return __generic_gxx_key_create (key, dtor);
}

static inline int
__gthread_key_dtor (__gthread_key_t key, void *ptr)
{
  return __generic_gxx_key_dtor (key, ptr);
}

static inline int
__gthread_key_delete (__gthread_key_t key)
{
  return __generic_gxx_key_delete (key);
}

static inline void *
__gthread_getspecific (__gthread_key_t key)
{
  return __generic_gxx_getspecific (key);
}

static inline int
__gthread_setspecific (__gthread_key_t key, const void *ptr)
{
  return __generic_gxx_setspecific (key, ptr);
}

static inline void
__gthread_mutex_init_function (__gthread_mutex_t *mutex)
{
  __generic_gxx_mutex_init_function (mutex);
}

static inline int
__gthread_mutex_lock (__gthread_mutex_t * mutex)
{
  return __generic_gxx_mutex_lock (mutex);
}

static inline int
__gthread_mutex_trylock (__gthread_mutex_t * mutex)
{
  return __generic_gxx_mutex_trylock (mutex);
}

static inline int
__gthread_mutex_unlock (__gthread_mutex_t * mutex)
{
  return __generic_gxx_mutex_unlock (mutex);
}

static inline void
__gthread_recursive_mutex_init_function (__gthread_recursive_mutex_t *mutex)
{
  __generic_gxx_recursive_mutex_init_function (mutex);
}

static inline int
__gthread_recursive_mutex_lock (__gthread_recursive_mutex_t * mutex)
{
  return __generic_gxx_recursive_mutex_lock (mutex);
}

static inline int
__gthread_recursive_mutex_trylock (__gthread_recursive_mutex_t * mutex)
{
  return __generic_gxx_recursive_mutex_trylock (mutex);
}

static inline int
__gthread_recursive_mutex_unlock (__gthread_recursive_mutex_t * mutex)
{
  return __generic_gxx_recursive_mutex_unlock (mutex);
}

#endif /* _LIBOBJC */

#endif /* __gthr_generic_h */
