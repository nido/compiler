/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

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


//  
//  Generate ISA registers information
///////////////////////////////////////


//  $Revision$
//  $Date$
//  $Author$
//  $Source$

#include <stddef.h>
#include "isa_registers_gen.h"
#include "targ_isa_subset.h"


static int ISA_Mask(ISA_SUBSET isa)
{
  return 1 << (int)isa;
}


static int All_ISA_Mask(void)
{
  int i;
  int mask = 0;
  for (i = ISA_SUBSET_MIN; i <= ISA_SUBSET_MAX; ++i) {
    mask |= 1 << i;
  }
  return mask;
}


static int Range_ISA_Mask(ISA_SUBSET min_isa, ISA_SUBSET max_isa)
{
  int i;
  int mask = 0;
  for (i = (int)min_isa; i <= (int)max_isa; ++i) {
    mask |= 1 << i;
  }
  return mask;
}


static const char *guard_reg_names[] = {
  NULL,        NULL,        NULL,          NULL,		// 0-3
  NULL,        NULL,        NULL,          NULL,		// 4-7
  NULL,        NULL,        NULL,          NULL,		// 8-11
  NULL,        NULL,        NULL,          "g15",		// 12-15
};

static const char *loop_reg_names[] = {
  "lcr0",        "lcr1",        "lcr2",               		// 0-2
};

static const int au_3[] = {3};
static const int au_11[] = {11};
static const int au_13[] = {13};
static const int au_15[] = {15};
static const int au_lset[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static const int au_hset[] = { 8, 9, 10, 11, 12, 13, 14, 15};

static const int du_3[] = {3};
static const int du_lset[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static const int du_hset[] = { 8, 9, 10, 11, 12, 13, 14, 15};

static const int cr_8[] = {8};
static const int cr_9[] = {9};
static const int cr_29[] = {29};
static const int crl_set[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
static const int crh_set[] = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };

static const int g_0[] = { 0 };

static const int lr_0[] = {0};
static const int lr_1[] = {1};
static const int lr_2[] = {2};

#define NELEMS(a) (sizeof(a) / sizeof(*(a)))

main()
{
  ISA_REGISTER_CLASS
    rc_du,
    rc_au,
    rc_guard,
    rc_control,
    rc_loop;

  ISA_Registers_Begin("st100");

  rc_du = ISA_Register_Class_Create("du", 40, false, true, false);
  rc_au = ISA_Register_Class_Create("au", 32, true, true, false);
  rc_guard = ISA_Register_Class_Create("guard", 1, false, true, true);
  rc_control = ISA_Register_Class_Create("control", 1, false, false, false);
  rc_loop = ISA_Register_Class_Create("loop", 1, false, false, false);

  // DU and AU registers are named in the ABI
  ISA_Register_Set(rc_du, 0, 15, "r%d", NULL, All_ISA_Mask());
  ISA_Register_Subclass_Create("r3", rc_du, 
			       NELEMS(du_3), du_3, NULL);
  ISA_Register_Subclass_Create("drl", rc_du, 
			       NELEMS(du_lset), du_lset, NULL);
  ISA_Register_Subclass_Create("drh", rc_du, 
			       NELEMS(du_hset), du_hset, NULL);

  ISA_Register_Set(rc_au, 0, 15, "p%d", NULL, All_ISA_Mask());
  ISA_Register_Subclass_Create("p3", rc_au, 
			       NELEMS(au_3), au_3, NULL);
  ISA_Register_Subclass_Create("p11", rc_au, 
			       NELEMS(au_11), au_11, NULL);
  ISA_Register_Subclass_Create("p13", rc_au, 
			       NELEMS(au_13), au_13, NULL);
  ISA_Register_Subclass_Create("p15", rc_au, 
			       NELEMS(au_15), au_15, NULL);
  ISA_Register_Subclass_Create("arl", rc_au, 
			       NELEMS(au_lset), au_lset, NULL);
  ISA_Register_Subclass_Create("arh", rc_au, 
			       NELEMS(au_hset), au_hset, NULL);

  ISA_Register_Set(rc_guard, 0, 15, "g%d", NULL, All_ISA_Mask());
  ISA_Register_Subclass_Create("g0", rc_guard, 
			       NELEMS(g_0), g_0, NULL);

  ISA_Register_Set(rc_control, 0, 31, "c%d", NULL, All_ISA_Mask());
  ISA_Register_Subclass_Create("cr8", rc_control, 
			       NELEMS(cr_8), cr_8, NULL);
  ISA_Register_Subclass_Create("cr9", rc_control, 
			       NELEMS(cr_9), cr_9, NULL);
  ISA_Register_Subclass_Create("cr29", rc_control, 
			       NELEMS(cr_29), cr_29, NULL);
  ISA_Register_Subclass_Create("crl", rc_control, 
			       NELEMS(crl_set), crl_set, NULL);
  ISA_Register_Subclass_Create("crh", rc_control, 
			       NELEMS(crh_set), crh_set, NULL);


  ISA_Register_Set(rc_loop, 0, 2, "lcr%d", loop_reg_names,
		   All_ISA_Mask());
  ISA_Register_Subclass_Create("lr0", rc_loop, 
			       NELEMS(lr_0), lr_0, NULL);
  ISA_Register_Subclass_Create("lr1", rc_loop, 
			       NELEMS(lr_1), lr_1, NULL);
  ISA_Register_Subclass_Create("lr2", rc_loop, 
			       NELEMS(lr_2), lr_2, NULL);


  ISA_Registers_End();
  return 0;
}
