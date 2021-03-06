/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

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

   Special thanks goes to SGI for their continued support to open source

*/


/* USMID:  "\n@(#)5.0_pl/macros/i_cvrt.m	5.2	06/10/99 14:15:34\n" */

/*****************\
|* MISCELLANEOUS *|
\*****************/

# define NUM_PDG_TYP_WDS	(sizeof(pdg_type_tbl_type)/HOST_BYTES_PER_WORD)

/*****************************************************************************/
/*  The following are for the frontend use and do not exist in the shared    */
/*  module.                                                                  */
/*****************************************************************************/


# define SIGN_EXTEND(VALU)						       \
{   VALU = VALU << 32;           					       \
    VALU = VALU >> 32;   }						       \



# ifdef _DEBUG



# define PDG_DBG_PRINT_START						       \
if (dump_flags.pdgcs) {


# define PDG_DBG_PRINT_END						       \
}


# define PDG_DBG_PRINT_C(NAME)						       \
fprintf(debug_file, "i_cvrt(%5d): %s()\n", __LINE__, NAME);

# define PDG_DBG_PRINT_S(STR1, VAR1) 	 				       \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30s (string)\n", __LINE__, STR1, VAR1);

# define PDG_DBG_PRINT_LLD(STR1, VAR1)                                         \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30lld (decimal)\n", __LINE__, STR1, (long long) VAR1);

# define PDG_DBG_PRINT_LLO(STR1, VAR1)					       \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30llo (octal)\n", __LINE__, STR1, (long long) VAR1);


# if defined(_HOST32) && defined(_TARGET64)

# define PDG_DBG_PRINT_LVD(STR1, VAR1)                                          \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30ld (decimal)\n", __LINE__, STR1, VAR1);

# define PDG_DBG_PRINT_VD(STR1, VAR1)                                          \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30lld (decimal)\n", __LINE__, STR1, VAR1);

# define PDG_DBG_PRINT_VO(STR1, VAR1)					       \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30llo (octal)\n", __LINE__, STR1, VAR1);

# else

# define PDG_DBG_PRINT_LVD(STR1, VAR1)                                          \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30ld (decimal)\n", __LINE__, STR1, VAR1);

# define PDG_DBG_PRINT_VD(STR1, VAR1)                                          \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30d (decimal)\n", __LINE__, STR1, VAR1);

# define PDG_DBG_PRINT_VO(STR1, VAR1)					       \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30o (octal)\n", __LINE__, STR1, VAR1);

# endif


# define PDG_DBG_PRINT_D(STR1, VAR1)                                           \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30d (decimal)\n", __LINE__, STR1, VAR1);

# define PDG_DBG_PRINT_LD(STR1, VAR1)                                           \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30ld (decimal)\n", __LINE__, STR1, VAR1);

# define PDG_DBG_PRINT_O(STR1, VAR1)					       \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30o (octal)\n", __LINE__, STR1, VAR1);

# define PDG_DBG_PRINT_LO(STR1, VAR1)					       \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %-30lo (octal)\n", __LINE__, STR1, VAR1);




# if (defined(_TARGET_OS_IRIX) || defined(_TARGET_OS_LINUX) || defined(_TARGET_OS_DARWIN)) || defined(_TARGET_MONGOOSE)
# define PDG_DBG_PRINT_T(STR1, VAR1)					       \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %1o %1o %1o %1o %1o %1o %1o %1o " \
		"%d %d %d %1o %d\n", __LINE__, STR1,			       \
	        VAR1.const_flag, VAR1.volatile_flag, 			       \
		VAR1.signed_flag, VAR1.automatic, VAR1.restricted, 	       \
		VAR1.short_flag, VAR1.long_flag, VAR1.bitfield,		       \
		VAR1.table_type, VAR1.basic_type, VAR1.aux_info,	       \
		VAR1.shrd_pointee, VAR1.table_index);
# else
# define PDG_DBG_PRINT_T(STR1, VAR1)					       \
fprintf(debug_file, "i_cvrt(%5d):     %-20s= %1o %1o %1o %1o %1o %1o %1o "     \
		"%d %d %d %d\n", __LINE__, STR1,			       \
	        VAR1.const_flag, VAR1.volatile_flag, 			       \
		VAR1.signed_flag, VAR1.restricted, 	                       \
		VAR1.short_flag, VAR1.long_flag, VAR1.bitfield,		       \
		VAR1.table_type, VAR1.basic_type, VAR1.aux_info,	       \
		VAR1.table_index);
# endif



# else



# define PDG_DBG_PRINT_START
# define PDG_DBG_PRINT_END
# define PDG_DBG_PRINT_C(NAME)
# define PDG_DBG_PRINT_S(STR1, VAR1)
# define PDG_DBG_PRINT_D(STR1, VAR1)
# define PDG_DBG_PRINT_LD(STR1, VAR1)
# define PDG_DBG_PRINT_LLD(STR1, VAR1)
# define PDG_DBG_PRINT_T(STR1, VAR1)
# define PDG_DBG_PRINT_O(STR1, VAR1)
# define PDG_DBG_PRINT_LO(STR1, VAR1)
# define PDG_DBG_PRINT_LLO(STR1, VAR1)
# define PDG_DBG_PRINT_VD(STR1, VAR1)
# define PDG_DBG_PRINT_LVD(STR1, VAR1)
# define PDG_DBG_PRINT_VO(STR1, VAR1)


# endif
