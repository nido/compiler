/* Output variables, constants and external declarations, for GNU compiler.
   Copyright (C) 1987, 1988, 1989, 1992, 1993, 1994, 1995, 1996, 1997,
   1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef TARG_ST
#include "gccfe_targinfo_interface.h"
#define FIRST_PSEUDO_REGISTER GCCTARG_Initial_Number_Of_Registers()
#define REGISTER_NAMES {}
#else
#ifdef TARG_MIPS
#include "mips.h"
#else
#include "i386.h"
#endif
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static const char *reg_names[] = REGISTER_NAMES;

/* Given NAME, a putative register name, discard any customary prefixes.  */

static const char *
strip_reg_name (const char *name)
{
  if (name[0] == '%' || name[0] == '#')
    name++;
  return name;
}

/* Decode an `asm' spec for a declaration as a register name.
   Return the register number, or -1 if nothing specified,
   or -2 if the ASMSPEC is not `cc' or `memory' and is not recognized,
   or -3 if ASMSPEC is `cc' and is not recognized,
   or -4 if ASMSPEC is `memory' and is not recognized.
   Accept an exact spelling or a decimal number.
   Prefixes such as % are optional.  */

#ifdef TARG_ST
// [TTh] Use constants to avoid hardcoded immediates, at least in ST modified code..
#define ASMSPEC_EMPTY              (-1)
#define ASMSPEC_NOT_RECOGNIZED     (-2)
#define ASMSPEC_CC_RECOGNIZED      (-3)
#define ASMSPEC_MEM_RECOGNIZED     (-4)
#endif

int
gs_decode_reg_name (const char *asmspec)
{
  if (asmspec != 0)
    {
      int i;

      /* Get rid of confusing prefixes.  */
      asmspec = strip_reg_name (asmspec);

      /* Allow a decimal number as a "register name".  */
      for (i = strlen (asmspec) - 1; i >= 0; i--)
	if (! isdigit (asmspec[i]))
	  break;
      if (asmspec[0] != 0 && i < 0)
	{
	  i = atoi (asmspec);
	  if (i < FIRST_PSEUDO_REGISTER && i >= 0)
	    return i;
	  else
	    return -2;
	}

#ifndef TARG_ST
      for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
	if (reg_names[i][0]
	    && ! strcmp (asmspec, strip_reg_name (reg_names[i])))
	  return i;

#ifdef ADDITIONAL_REGISTER_NAMES
      {
	static const struct { const char *const name; const int number; } table[]
	  = ADDITIONAL_REGISTER_NAMES;

	for (i = 0; i < (int) ARRAY_SIZE (table); i++)
	  if (! strcmp (asmspec, table[i].name))
	    return table[i].number;
      }
#endif /* ADDITIONAL_REGISTER_NAMES */
#else
//TB: for extension, make ADDITIONAL_REGISTER_NAMES a real array
      int add_size = GCCTARG_Additional_Register_Names_Size ();
      gcc_register_map_t *add_names = GCCTARG_Additional_Register_Names ();
      for (i = 0; i < add_size; i++)
	  if (asmspec[0] != '%') {
	    char *whole_name = add_names[i].name;
	      while (strchr(whole_name, '%')) {
		whole_name++;
	      }
	    if (! strcasecmp (asmspec, whole_name))
	      if (!add_names[i].disabled) {
		return add_names[i].number;
	      } else {
		return (ASMSPEC_NOT_RECOGNIZED);
	      }
#ifdef TARG_ST200
	    // [CG]: We want to have the '$' optional in '$r12' for ST200
	    // when parsing the register name.
	    // TODO: handle this in targinfo parsing description.
	    const char *ignored = "$";
	    while(*ignored != '\0') {
	      if (asmspec[0] == *ignored) {
		if (!strcasecmp (&asmspec[1], whole_name))
		  return add_names[i].number;
	      }
	      if (whole_name[0] == *ignored) {
		if (!strcasecmp (asmspec, &whole_name[1]))
		  return add_names[i].number;
	      }
	      ignored++;
	    }
#endif
	  } else {
	    if (! strcasecmp (asmspec, add_names[i].name))
	      if (!add_names[i].disabled) {
		return add_names[i].number;
	      } else {
		return (ASMSPEC_NOT_RECOGNIZED);
	      }
	  }
#endif//TARG_ST

      if (!strcmp (asmspec, "memory"))
	return -4;

      if (!strcmp (asmspec, "cc"))
	return -3;

      return -2;
    }

  return -1;
}
