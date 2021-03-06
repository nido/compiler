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



/* USMID:  "\n@(#)5.0_pl/headers/module.h	5.1	04/29/99 21:22:31\n" */


enum    tbl_type_values         {Null_Tbl,
                                 Name_Pool,
                                 Loc_Name_Tbl,
                                 Attr_Tbl,
                                 Bounds_Tbl,
                                 Const_Tbl,
                                 Const_Pool,
                                 Sec_Name_Tbl,
                                 Stor_Blk_Tbl,
                                 Type_Tbl,
                                 Ir_Tbl,
                                 Ir_List_Tbl,
                                 Sh_Tbl,
                                 Num_Of_Tbls = Sh_Tbl,
                                 Num_Of_Old_Tbls = Ir_List_Tbl };

typedef	enum	tbl_type_values		tbl_type_type;
typedef	union	mit_descriptor_entry	mit_descriptor_type;
typedef union   mit_header_entry	mit_header_type;
typedef struct  mit_inside_entry	mit_inside_type;
typedef struct  old_mit_inside_entry	old_mit_inside_type;

union	mit_descriptor_entry	{
				 struct	{tbl_type_type  tbl_type        :  8;
                                 	 Uint           unused	        : 24;
                                 	 Uint           num_entries     : 32;
                                	} fld;
				 long	wd;
				};

/***********************************\
|* MODULE INFORMATION TABLE HEADER *|
\***********************************/

struct	mit_inside_entry	{pdt_tbl_hdr_type       hdr;
				 Uint           	version_num     :  8;
				 Uint			pools_64_bits	:  1;
                        	 Uint		     	unused1		:  4;
                        	 boolean        	dalign          :  1;
                        	 boolean        	mod_has_errors  :  1;
                        	 boolean        	dp_hit_me       :  1;
                        	 Uint           	default_integer :  8;
                        	 Uint           	target          :  8;
                        	 Uint           	unused2         : 16;
                        	 Uint           	name_length     :  5;
                        	 boolean          	alternate_entry	:  1;
                        	 Uint           	float64		:  1;
                        	 Uint           	cf77types	:  1;
                        	 Uint           	module		:  1;
                        	 Uint           	default32	:  1;
                        	 Uint           	default64	:  1;
                        	 Uint           	pointer8        :  1;
                        	 Uint           	new_cn_tbl      :  1;
                        	 char                   name_char[32];
                        	};

struct	old_mit_inside_entry	{pdt_tbl_hdr_type       hdr;
				 Uint           	version_num     :  8;
                        	 Uint           	name_length     :  5;
                        	 boolean        	dalign          :  1;
                        	 boolean        	mod_has_errors  :  1;
                        	 boolean        	dp_hit_me       :  1;
                        	 Uint           	default_integer :  8;
                        	 Uint           	target          :  8;
                        	 boolean          	alternate_entry	:  1;
                        	 Uint           	unused2         : 24;
                        	 Uint           	float64		:  1;
                        	 Uint           	cf77types	:  1;
                        	 Uint           	module		:  1;
                        	 Uint           	default32	:  1;
                        	 Uint           	default64	:  1;
                        	 Uint           	pointer8        :  1;
                        	 Uint           	unused3         :  1;
                        	 char                   name_char[32];
                        	};

union	mit_header_entry        {mit_inside_type	tbl;
				 old_mit_inside_type	old_tbl;
				 long wd[sizeof(mit_inside_type)/sizeof(long)];
                       		};


	mit_descriptor_type	mit_descriptor[Num_Of_Tbls+1];
	mit_header_type		mit_header;

enum    md_3_linear_type_values {MD_3_Err_Res,
                                 MD_3_Type_Void = MD_3_Err_Res,
                                 MD_3_Short_Char_Const,
                                 MD_3_Short_Typeless_Const,
                                 MD_3_Typeless_4,
                                 MD_3_Typeless_8,
                                 MD_3_Long_Typeless,
                                 MD_3_Integer_1,
                                 MD_3_Integer_2,
                                 MD_3_Integer_4,
                                 MD_3_Integer_6,
                                 MD_3_Integer_8,
                                 MD_3_Real_4,
                                 MD_3_Real_8,
                                 MD_3_Real_16,
                                 MD_3_Complex_4,
                                 MD_3_Complex_8,
                                 MD_3_Complex_16,
                                 MD_3_CRI_Ptr_8,
                                 MD_3_Logical_1,
                                 MD_3_Logical_2,
                                 MD_3_Logical_4,
                                 MD_3_Logical_8,
                                 MD_3_Character_1,
                                 MD_3_Character_2,
                                 MD_3_Character_4,
                                 MD_3_CRI_Ch_Ptr_8,
                                 MD_3_Structure_Type,
                                 MD_3_CRI_Parcel_Ptr_8,
                                 MD_3_Num_Linear_Types
                                };


static	boolean	keep_module_procs	= TRUE;
static	char	mod_file_name[MAX_FILE_NAME_SIZE];

static	char   *tbl_type_str[]         = {
		" ",
		"Name_Pool",
		"Loc_Name_Tbl",
		"Attr_Tbl",
		"Bounds_Tbl",
		"Const_Tbl",
		"Const_Pool",
		"Sec_Name_Tbl",
		"Stor_Blk_Tbl",
		"Type_Tbl",
		"Ir_Tbl",
		"Ir_List_Tbl",
		"Sh_Tbl" };
