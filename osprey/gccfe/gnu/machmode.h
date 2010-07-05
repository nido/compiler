/* Machine mode definitions for GNU C-Compiler; included by rtl.h and tree.h.
   Copyright (C) 1991, 1993, 1994, 1996, 1998, 1999, 2000, 2001
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

#ifndef HAVE_MACHINE_MODES
#define HAVE_MACHINE_MODES

/* Make an enum class that gives all the machine modes.  */

#define DEF_MACHMODE(SYM, NAME, TYPE, BITSIZE, SIZE, UNIT, WIDER, INNER)  SYM,

#ifndef TARG_ST
//enum machine_mode {
//#include "machmode.def"
//MAX_MACHINE_MODE };
#else
enum machine_mode {
#include "machmode.def"
// STATIC_COUNT_MACHINE_MODE is the last machine mode staticaly known
STATIC_COUNT_MACHINE_MODE,
// MAX_LIMIT_MACHINE_MODE is the static limit of nb of machine mode
// (constraint must be on 7 bit because of tree bit field
// ENUM_BITFIELD(machine_mode) mode : 7;
MAX_LIMIT_MACHINE_MODE = 127
 };
// COUNT_MACHINE_MODE = dynamic count of the last machine mode
// COUNT_MACHINE_MODE cannot be > to MAX_LIMIT_MACHINE_MODE
typedef int machine_mode_t;
extern machine_mode_t COUNT_MACHINE_MODE;
/* We define the gcc MAX_MACHINE_MODE to be the counter.*/
#define MAX_MACHINE_MODE (COUNT_MACHINE_MODE)
#endif
#undef DEF_MACHMODE

#ifdef TARG_ST
// Last machine mode statically defined 
#define MACHINE_MODE_STATIC_LAST (STATIC_COUNT_MACHINE_MODE-1)

/* Nonzero if MODE has been dynamically defined */
#define IS_DYNAMIC_MACHINE_MODE(MODE) (MODE > MACHINE_MODE_STATIC_LAST)
#endif

#ifndef NUM_MACHINE_MODES
#ifdef TARG_ST
#define NUM_MACHINE_MODES (int) COUNT_MACHINE_MODE
#else
#define NUM_MACHINE_MODES (int) MAX_MACHINE_MODE
#endif
#endif

/* Get the name of mode MODE as a string.  */

extern const char *mode_name[MAX_LIMIT_MACHINE_MODE];
#define GET_MODE_NAME(MODE)		(mode_name[(int) (MODE)])

enum mode_class { MODE_RANDOM, MODE_INT, MODE_FLOAT, MODE_PARTIAL_INT, MODE_CC,
		  MODE_COMPLEX_INT, MODE_COMPLEX_FLOAT,
		  MODE_VECTOR_INT, MODE_VECTOR_FLOAT,
		  MAX_MODE_CLASS};

/* Get the general kind of object that mode MODE represents
   (integer, floating, complex, etc.)  */

extern  enum mode_class mode_class[MAX_LIMIT_MACHINE_MODE];
#define GET_MODE_CLASS(MODE)		(mode_class[(int) (MODE)])

/* Nonzero if MODE is an integral mode.  */
#define INTEGRAL_MODE_P(MODE)			\
  (GET_MODE_CLASS (MODE) == MODE_INT		\
   || GET_MODE_CLASS (MODE) == MODE_PARTIAL_INT \
   || GET_MODE_CLASS (MODE) == MODE_COMPLEX_INT \
   || GET_MODE_CLASS (MODE) == MODE_VECTOR_INT)

/* Nonzero if MODE is a floating-point mode.  */
#define FLOAT_MODE_P(MODE)		\
  (GET_MODE_CLASS (MODE) == MODE_FLOAT	\
   || GET_MODE_CLASS (MODE) == MODE_COMPLEX_FLOAT \
   || GET_MODE_CLASS (MODE) == MODE_VECTOR_FLOAT)

/* Nonzero if MODE is a complex mode.  */
#define COMPLEX_MODE_P(MODE)			\
  (GET_MODE_CLASS (MODE) == MODE_COMPLEX_INT	\
   || GET_MODE_CLASS (MODE) == MODE_COMPLEX_FLOAT)

/* Nonzero if MODE is a vector mode.  */
#define VECTOR_MODE_P(MODE)			\
  (GET_MODE_CLASS (MODE) == MODE_VECTOR_INT	\
   || GET_MODE_CLASS (MODE) == MODE_VECTOR_FLOAT)

/* Nonzero if MODE is a scalar integral mode.  */
#define SCALAR_INT_MODE_P(MODE)			\
  (GET_MODE_CLASS (MODE) == MODE_INT		\
   || GET_MODE_CLASS (MODE) == MODE_PARTIAL_INT)

/* Nonzero if MODE is a scalar floating point mode.  */
#define SCALAR_FLOAT_MODE_P(MODE)		\
  (GET_MODE_CLASS (MODE) == MODE_FLOAT)

/* Get the size in bytes of an object of mode MODE.  */

#ifdef TARG_ST
// [TTh] Support types wider than 255 bytes
extern  unsigned short mode_size[MAX_LIMIT_MACHINE_MODE];
#else
extern  unsigned char mode_size[MAX_LIMIT_MACHINE_MODE];
#endif
#define GET_MODE_SIZE(MODE)		(mode_size[(int) (MODE)])

/* Get the size in bytes of the basic parts of an object of mode MODE.  */

#ifdef TARG_ST
// [TTh] Support types wider than 255 bytes
extern  unsigned short mode_unit_size[MAX_LIMIT_MACHINE_MODE];
#else
extern  unsigned char mode_unit_size[MAX_LIMIT_MACHINE_MODE];
#endif
#define GET_MODE_UNIT_SIZE(MODE)	(mode_unit_size[(int) (MODE)])

/* Get the number of units in the object.  */

#define GET_MODE_NUNITS(MODE)  \
  ((GET_MODE_UNIT_SIZE ((MODE)) == 0) ? 0 \
   : (GET_MODE_SIZE ((MODE)) / GET_MODE_UNIT_SIZE ((MODE))))

/* Get the size in bits of an object of mode MODE.  */

extern  unsigned short mode_bitsize[MAX_LIMIT_MACHINE_MODE];
#define GET_MODE_BITSIZE(MODE)  (mode_bitsize[(int) (MODE)])

#endif /* not HAVE_MACHINE_MODES */

#if defined HOST_WIDE_INT && ! defined GET_MODE_MASK

/* Get a bitmask containing 1 for all bits in a word
   that fit within mode MODE.  */

extern  unsigned HOST_WIDE_INT mode_mask_array[MAX_LIMIT_MACHINE_MODE];

#define GET_MODE_MASK(MODE) mode_mask_array[(int) (MODE)]

extern  machine_mode_t inner_mode_array[MAX_LIMIT_MACHINE_MODE];

/* Return the mode of the inner elements in a vector.  */

#define GET_MODE_INNER(MODE) inner_mode_array[(int) (MODE)]

#endif /* defined (HOST_WIDE_INT) && ! defined GET_MODE_MASK */

#if ! defined GET_MODE_WIDER_MODE || ! defined GET_MODE_ALIGNMENT \
    || ! defined GET_CLASS_NARROWEST_MODE

/* Get the next wider natural mode (eg, QI -> HI -> SI -> DI -> TI).  */

extern  unsigned char mode_wider_mode[MAX_LIMIT_MACHINE_MODE];
#define GET_MODE_WIDER_MODE(MODE)	((machine_mode_t)mode_wider_mode[(int) (MODE)])

/* Return the mode for data of a given size SIZE and mode class CLASS.
   If LIMIT is nonzero, then don't use modes bigger than MAX_FIXED_MODE_SIZE.
   The value is BLKmode if no other mode is found.  */
extern machine_mode_t mode_for_size PARAMS ((unsigned int,
						enum mode_class, int));

/* Similar, but find the smallest mode for a given width.  */

extern machine_mode_t smallest_mode_for_size 
				PARAMS ((unsigned int, enum mode_class));


/* Return an integer mode of the exact same size as the input mode,
   or BLKmode on failure.  */

extern machine_mode_t int_mode_for_mode PARAMS ((machine_mode_t));

/* Find the best mode to use to access a bit field.  */

extern machine_mode_t get_best_mode PARAMS ((int, int, unsigned int,
						machine_mode_t, int));

/* Determine alignment, 1<=result<=BIGGEST_ALIGNMENT.  */

extern unsigned get_mode_alignment PARAMS ((machine_mode_t));

#define GET_MODE_ALIGNMENT(MODE) get_mode_alignment (MODE)

/* For each class, get the narrowest mode in that class.  */

extern const machine_mode_t class_narrowest_mode[(int) MAX_MODE_CLASS];
#define GET_CLASS_NARROWEST_MODE(CLASS) class_narrowest_mode[(int) (CLASS)]

/* Define the integer modes whose sizes are BITS_PER_UNIT and BITS_PER_WORD
   and the mode whose class is Pmode and whose size is POINTER_SIZE.  */

extern machine_mode_t byte_mode;
extern machine_mode_t word_mode;
extern machine_mode_t ptr_mode;

#endif /* ! defined GET_MODE_WIDER_MODE || ! defined GET_MODE_ALIGNMENT
	  || ! defined GET_CLASS_NARROWEST_MODE */
