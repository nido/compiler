/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


#ifndef __STAMP_H__
#define __STAMP_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TARG_ST

    /*
     * IA64 Linux compiler
     */

#if defined(__linux)
#define	MS_STAMP 0
#define	LS_STAMP 9
#define INCLUDE_STAMP "0.01.0-13"
#endif

    /*
     * IA64 Sgi compiler
     */
#if defined(__mips)
#define	MS_STAMP 7
#define	LS_STAMP 40
#define INCLUDE_STAMP "7.40"
#endif

#else /* TARG_ST */

#ifndef RELEASE_DATE
#define RELEASE_DATE "Unknown"
#endif

    /*
     * ST Linux compiler
     */
#if defined(__linux)

#ifndef MS_STAMP
#define	MS_STAMP "1"
#endif
#ifndef LS_STAMP
#define	LS_STAMP "0-2-A"
#endif
#ifndef RELEASE_ID
#define RELEASE_ID MS_STAMP"."LS_STAMP
#endif

#ifndef RELEASE_PLATFORM
#define RELEASE_PLATFORM "(i386-linux-redhat-6.2)"
#endif

#ifndef TARG_ST
  /* Obsolete. */
#ifndef DEFAULT_TOOLROOT
#define DEFAULT_TOOLROOT "/apa/comp/Pro64-ST200/Linux/20020408"
#endif
#endif

#endif

/*
 * On Solaris ST compilers
 */
#if defined(__sun)

#ifndef MS_STAMP
#define	MS_STAMP "1"
#endif

#ifndef LS_STAMP
#define	LS_STAMP "0-5-A"
#endif

#ifndef RELEASE_ID
#define RELEASE_ID MS_STAMP"."LS_STAMP
#endif

#ifndef RELEASE_PLATFORM
#define RELEASE_PLATFORM "(sparc-sun-solaris-5.1)"
#endif

#ifndef TARG_ST
  /* Obsolete. */
#ifndef DEFAULT_TOOLROOT
#define DEFAULT_TOOLROOT "/apa/comp/Pro64-ST200/SunOS5/latest"
#endif
#endif

#endif

/*
 * On MingW ST compilers
 */
#if defined(__MINGW32__)

#ifndef MS_STAMP
#define	MS_STAMP "1"
#endif
#ifndef LS_STAMP
#define	LS_STAMP "0-0-A"
#endif
#ifndef RELEASE_ID
#define RELEASE_ID MS_STAMP"."LS_STAMP
#endif

#ifndef RELEASE_PLATFORM
#define RELEASE_PLATFORM "(i386-pc-mingw32msvc)"
#endif

#ifndef TARG_ST
  /* Obsolete. */
#ifndef DEFAULT_TOOLROOT
#define DEFAULT_TOOLROOT "/apa/comp/Pro64-ST200/Cingw/20020408/"
#endif
#endif

#endif

/*
 * On Cygwin ST compilers
 */
#if defined(__CYGWIN__)

#ifndef MS_STAMP
#define	MS_STAMP "1"
#endif
#ifndef LS_STAMP
#define	LS_STAMP "0-0-A"
#endif
#ifndef RELEASE_ID
#define RELEASE_ID MS_STAMP"."LS_STAMP
#endif

#ifndef RELEASE_PLATFORM
#define RELEASE_PLATFORM "(i686-pc-cygwin)"
#endif

#ifndef TARG_ST
  /* Obsolete. */
#ifndef DEFAULT_TOOLROOT
#define DEFAULT_TOOLROOT "/apa/comp/Pro64-ST200/Cygwin/20020408/"
#endif
#endif

#endif

#define INCLUDE_STAMP RELEASE_ID" "RELEASE_DATE" "RELEASE_PLATFORM

#ifdef TARG_ST200
/*
 * Added to support dircrimination between compiler releases
 * Driver uses this to emit
 * __ST200CC__ 			as the major release number
 * __ST200CC_MINOR__ 		as the minor release number
 * __ST200CC_PATCHLEVEL__ 	as the patch level
 * __ST200CC_DATE__ 		as the release date
 * __ST200CC_VERSION__ 		as a version string
 * This is similar to what is done in gcc (except that we cannot use
 * gcc defintions, of course, since they are not correlated to 
 * our versions, and except for the date that does not exist for
 * current gccs)
 */
#if defined(RELEASE_MAJOR)
#define ST200CC_MAJOR__ RELEASE_MAJOR
#else
#define ST200CC_MAJOR__ 0
#endif /* RELEASE_MAJOR */

#if defined(RELEASE_MINOR)
#define ST200CC_MINOR__ RELEASE_MINOR
#else
#define ST200CC_MINOR__ 0
#endif /* RELEASE_MINOR */

#if defined(RELEASE_PATCHLEVEL)
#define ST200CC_PATCHLEVEL__ RELEASE_PATCHLEVEL
#else
#define ST200CC_PATCHLEVEL__ 0
#endif /* RELEASE_PATCHLEVEL */

#if defined(RELEASE_IDATE)
#define ST200CC_DATE__ RELEASE_IDATE
#else
#define ST200CC_DATE__ 19700101
#endif /* RELEASE_DATE */

#if defined(RELEASE_ID) && defined(RELEASE_DATE)
#define ST200CC_VERSION__ RELEASE_ID" "RELEASE_DATE
#else
#define ST200CC_VERSION__ "Unknown"
#endif /*defined(RELEASE_ID) && defined(COMPLETE_DATE)*/

/* Some tokenization support */
#define ST200CC_STR__(x) #x
  /* [HK] the ## is unnecessary and throws a parse error with gcc-3.x */
  /*#define ST200CC_EXPAND__(s, t) #s ## "=" ST200CC_STR__(t)*/
#define ST200CC_EXPAND__(s, t) ST200CC_STR__(s) "=" ST200CC_STR__(t)

#endif /* TARG_ST200 */

#ifdef TARG_STxP70
/*
 * Added to support dircrimination between compiler releases
 * Driver uses this to emit
 * __STXP70CC__ 		as the major release number
 * __STXP70CC_MINOR__ 		as the minor release number
 * __STXP70CC_PATCHLEVEL__ 	as the patch level
 * __STXP70CC_DATE__ 		as the release date
 * __STXP70CC_VERSION__ 	as a version string
 * This is similar to what is done in gcc (except that we cannot use
 * gcc defintions, of course, since they are not correlated to 
 * our versions, and except for the date that does not exist for
 * current gccs)
 */
#if defined(RELEASE_MAJOR)
#define STXP70CC_MAJOR__ RELEASE_MAJOR
#else
#define STXP70CC_MAJOR__ 0
#endif /* RELEASE_MAJOR */

#if defined(RELEASE_MINOR)
#define STXP70CC_MINOR__ RELEASE_MINOR
#else
#define STXP70CC_MINOR__ 0
#endif /* RELEASE_MINOR */

#if defined(RELEASE_PATCHLEVEL)
#define STXP70CC_PATCHLEVEL__ RELEASE_PATCHLEVEL
#else
#define STXP70CC_PATCHLEVEL__ 0
#endif /* RELEASE_PATCHLEVEL */

#if defined(RELEASE_IDATE)
#define STXP70CC_DATE__ RELEASE_IDATE
#else
#define STXP70CC_DATE__ 19700101
#endif /* RELEASE_DATE */

#if defined(RELEASE_ID) && defined(RELEASE_DATE)
#define STXP70CC_VERSION__ RELEASE_ID" "RELEASE_DATE
#else
#define STXP70CC_VERSION__ "Unknown"
#endif /*defined(RELEASE_ID) && defined(COMPLETE_DATE)*/

/* Some tokenization support */
#define STXP70CC_STR__(x) #x
  /* [HK] the ## is unnecessary and throws a parse error with gcc-3.x */
  /*#define STXP70CC_EXPAND__(s, t) #s ## "=" STXP70CC_STR__(t)*/
#define STXP70CC_EXPAND__(s, t) STXP70CC_STR__(s) "=" STXP70CC_STR__(t)

#endif /* TARG_STxP70 */

#endif /* TARG_ST */

#ifdef __cplusplus
}
#endif

#endif  /* __STAMP_H__ */
