/* Configuration for GCC for hosting on Windows32.
   using GNU tools and the Windows32 API Library.
   Copyright (C) 1997, 1998, 1999, 2001, 2002, 2003, 2004, 2007
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef USG
#define USG 1
#endif

#ifndef ONLY_INT_FIELD
#define ONLY_INT_FIELDS 1
#endif

#ifndef USE_PROTOTYPES
#define USE_PROTOTYPES 1
#endif

#define environ _environ

/* Even though we support "/", allow "\" since everybody tests both.  */
#define DIR_SEPARATOR '\\'
#define DIR_SEPARATOR_2 '/'

/* Mingw32 does not try to hide the underlying DOS-based file system
   like Cygwin does.  */
#define HAVE_DOS_BASED_FILE_SYSTEM

#define EXECUTABLE_SUFFIX ".exe"

#define HOST_EXECUTABLE_SUFFIX ".exe"

#undef PATH_SEPARATOR
#define PATH_SEPARATOR ';'

/* This is the name of the null device on windows.  */
#define HOST_BIT_BUCKET "nul"

/*  The st_ino field of struct stat is always 0.  */
#define HOST_LACKS_INODE_NUMBERS

/* MSVCRT does not support the "ll" format specifier for printing
   "long long" values.  Instead, we use "I64".  */
#define HOST_LONG_LONG_FORMAT "I64"
