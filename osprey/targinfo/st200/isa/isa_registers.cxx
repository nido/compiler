/* -- This file is automatically generated -- */ 
/* 

  Copyright (C) 2002, 2004 ST Microelectronics, Inc.  All Rights Reserved. 

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

  Contact information:  ST Microelectronics, Inc., 
  , or: 

  http://www.st.com 

  For further information regarding this notice, see: 

  http: 
*/ 

// AUTOMATICALLY GENERATED FROM MDS DATA BASE !!! 

//  
//  Generate ISA registers information 
/////////////////////////////////////// 

#include <stddef.h>
#include "isa_registers_gen.h" 
#include "targ_isa_subset.h"

static const char *branch_reg_names[] = {
"$b0", "$b1", "$b2", "$b3", "$b4", "$b5", "$b6", "$b7"
};

static const char *integer_reg_names[] = {
"$r0", "$r1", "$r2", "$r3", "$r4", "$r5", "$r6", "$r7", "$r8", "$r9", "$r10", "$r11", "$r12", "$r13", "$r14", "$r15", "$r16", "$r17", "$r18", "$r19", "$r20", "$r21", "$r22", "$r23", "$r24", "$r25", "$r26", "$r27", "$r28", "$r29", "$r30", "$r31", "$r32", "$r33", "$r34", "$r35", "$r36", "$r37", "$r38", "$r39", "$r40", "$r41", "$r42", "$r43", "$r44", "$r45", "$r46", "$r47", "$r48", "$r49", "$r50", "$r51", "$r52", "$r53", "$r54", "$r55", "$r56", "$r57", "$r58", "$r59", "$r60", "$r61", "$r62", "$r63"
};

static const char *link_reg_names[] = {
"$r63"
};

static const char *nolink_reg_names[] = {
"$r0", "$r1", "$r2", "$r3", "$r4", "$r5", "$r6", "$r7", "$r8", "$r9", "$r10", "$r11", "$r12", "$r13", "$r14", "$r15", "$r16", "$r17", "$r18", "$r19", "$r20", "$r21", "$r22", "$r23", "$r24", "$r25", "$r26", "$r27", "$r28", "$r29", "$r30", "$r31", "$r32", "$r33", "$r34", "$r35", "$r36", "$r37", "$r38", "$r39", "$r40", "$r41", "$r42", "$r43", "$r44", "$r45", "$r46", "$r47", "$r48", "$r49", "$r50", "$r51", "$r52", "$r53", "$r54", "$r55", "$r56", "$r57", "$r58", "$r59", "$r60", "$r61", "$r62"
};

static const char *paired_reg_names[] = {
"$p0", "$p2", "$p4", "$p6", "$p8", "$p10", "$p12", "$p14", "$p16", "$p18", "$p20", "$p22", "$p24", "$p26", "$p28", "$p30", "$p32", "$p34", "$p36", "$p38", "$p40", "$p42", "$p44", "$p46", "$p48", "$p50", "$p52", "$p54", "$p56", "$p58", "$p60"
};

static const char *pairedfirst_reg_names[] = {
"$r0", "$r2", "$r4", "$r6", "$r8", "$r10", "$r12", "$r14", "$r16", "$r18", "$r20", "$r22", "$r24", "$r26", "$r28", "$r30", "$r32", "$r34", "$r36", "$r38", "$r40", "$r42", "$r44", "$r46", "$r48", "$r50", "$r52", "$r54", "$r56", "$r58", "$r60"
};

static const char *pairedsecond_reg_names[] = {
"$r0", "$r3", "$r5", "$r7", "$r9", "$r11", "$r13", "$r15", "$r17", "$r19", "$r21", "$r23", "$r25", "$r27", "$r29", "$r31", "$r33", "$r35", "$r37", "$r39", "$r41", "$r43", "$r45", "$r47", "$r49", "$r51", "$r53", "$r55", "$r57", "$r59", "$r61"
};

static const char *predicate_reg_names[] = {
"$b1", "$b2", "$b3", "$b4", "$b5", "$b6", "$b7"
};

static const int integer_link[] = {63};

static const int integer_nolink[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62};

static const int integer_paired[] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60};

static const int integer_pairedfirst[] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60};

static const int integer_pairedsecond[] = {0, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61};

static const int branch_predicate[] = {1, 2, 3, 4, 5, 6, 7};

main() 
{ 

  ISA_Registers_Begin("st200"); 

  ISA_REGISTER_CLASS rc_branch = ISA_Register_Class_Create("branch", 1, false, true, false);
  ISA_Register_Set(rc_branch, 0, 7, NULL, branch_reg_names,
    (1 << (int)ISA_SUBSET_st220) | (1 << (int)ISA_SUBSET_st231) | (1 << (int)ISA_SUBSET_st235));

  ISA_REGISTER_CLASS rc_integer = ISA_Register_Class_Create("integer", 32, true, true, false);
  ISA_Register_Set(rc_integer, 0, 63, NULL, integer_reg_names,
    (1 << (int)ISA_SUBSET_st220) | (1 << (int)ISA_SUBSET_st231) | (1 << (int)ISA_SUBSET_st235));

  ISA_Register_Subclass_Create("link", rc_integer, 1, integer_link, link_reg_names);

  ISA_Register_Subclass_Create("nolink", rc_integer, 63, integer_nolink, nolink_reg_names);

  ISA_Register_Subclass_Create("paired", rc_integer, 31, integer_paired, paired_reg_names);

  ISA_Register_Subclass_Create("pairedfirst", rc_integer, 31, integer_pairedfirst, paired_reg_names);

  ISA_Register_Subclass_Create("pairedsecond", rc_integer, 31, integer_pairedsecond, pairedsecond_reg_names);

  ISA_Register_Subclass_Create("predicate", rc_branch, 7, branch_predicate, predicate_reg_names);

  ISA_Registers_End(); 
  return 0; 
} 
