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


#ifdef _KEEP_RCS_ID
static const char source_file[] = __FILE__;
static const char rcs_id[] = "$Source$ $Revision$";
#endif /* _KEEP_RCS_ID */

#include "W_alloca.h"
#include <string.h>
#include <ctype.h>

#include "defs.h"
#include "targ_isa_subset.h"
#include "targ_isa_hazards.h"
#include "targ_isa_registers.h"
#include "targ_abi_properties.h"
#include "targ_proc.h"
#include "dso.h"

#include "ti_init.h"

#if defined(__MINGW32__) || defined(__CYGWIN__)
#define SO_EXT ".dll"
#else
#define SO_EXT ".so"
#endif

static void * targ_handler = NULL;

#ifdef TARG_ST

#include "ti_si.h"
#include <dlfcn.h>		    /* for sgidladd(), dlerror() */

/****************************************************************************
 ****************************************************************************/

typedef SI_RRW si_rrw_initial_t(void);
typedef SI_RRW si_rrw_has_overuse_t(SI_RRW);

SI_RRW SI_RRW_Initial(void)
{
  static si_rrw_initial_t* so_call = 0;
  if (!so_call && !(so_call = (si_rrw_initial_t*)
		    dlsym(targ_handler, "SI_RRW_Initial")))
    return 0;
		   
  return so_call();
}

SI_RRW SI_RRW_Has_Overuse(SI_RRW word_with_reservations)
{
  static si_rrw_has_overuse_t* so_call= 0;

  if (!so_call && !(so_call = (si_rrw_has_overuse_t*)
		    dlsym(targ_handler, "SI_RRW_Has_Overuse")))
    return 0;

  return so_call(word_with_reservations);
}

/****************************************************************************
 ****************************************************************************/
typedef SI_RESOURCE_ID_SET si_resource_id_set_universe_t(void);

SI_RESOURCE_ID_SET SI_RESOURCE_ID_SET_Universe(void)
{
  static si_resource_id_set_universe_t* so_call= 0;

  if (!so_call && !(so_call = (si_resource_id_set_universe_t*)
		    dlsym(targ_handler, "SI_RESOURCE_ID_SET_Universe")))
    return 0;

  return so_call();
}

/****************************************************************************
 ****************************************************************************/
typedef const char * si_issue_slot_name_t (SI_ISSUE_SLOT*);
typedef int si_issue_slot_skew_t(SI_ISSUE_SLOT*);
typedef int si_issue_slot_avail_per_cycle_t(SI_ISSUE_SLOT*);
typedef int si_issue_slot_count_t(void);
typedef SI_ISSUE_SLOT * si_ith_issue_slot_t(UINT);

const char* SI_ISSUE_SLOT_Name( SI_ISSUE_SLOT* slot )
{
  static si_issue_slot_name_t * so_call= 0;

  if (!so_call && !(so_call = (si_issue_slot_name_t*)
		    dlsym(targ_handler, "SI_ISSUE_SLOT_Name")))
    return NULL;

  return so_call(slot);
}

INT SI_ISSUE_SLOT_Skew( SI_ISSUE_SLOT* slot )
{
  static si_issue_slot_skew_t* so_call= 0;

  if (!so_call && !(so_call = (si_issue_slot_skew_t*)
		    dlsym(targ_handler, "SI_ISSUE_SLOT_Skew")))
    return 0;

  return so_call(slot);
}

INT SI_ISSUE_SLOT_Avail_Per_Cycle( SI_ISSUE_SLOT* slot )
{
  static si_issue_slot_avail_per_cycle_t* so_call= 0;

  if (!so_call && !(so_call = (si_issue_slot_avail_per_cycle_t*)
		    dlsym(targ_handler, "SI_ISSUE_SLOT_Avail_Per_Cycle")))
    return 0;

  return so_call(slot);
}

INT SI_ISSUE_SLOT_Count(void)
{
  static si_issue_slot_count_t* so_call= 0;

  if (!so_call && !(so_call = (si_issue_slot_count_t*)
		    dlsym(targ_handler, "SI_ISSUE_SLOT_Count")))
    return 0;

  return so_call();
}

SI_ISSUE_SLOT* SI_Ith_Issue_Slot( UINT i )
{
  static si_ith_issue_slot_t* so_call= 0;

  if (!so_call && !(so_call = (si_ith_issue_slot_t*)
		    dlsym(targ_handler, "SI_Ith_Issue_Slot")))
    return NULL;

  return so_call(i);
}

typedef SI_RESOURCE* si_resource_total_resource_t (SI_RESOURCE_TOTAL*);
typedef SI_RESOURCE_ID si_resource_total_resource_id_t (SI_RESOURCE_TOTAL*);
typedef UINT si_resource_total_avail_per_cycle_t(SI_RESOURCE_TOTAL*);
typedef INT si_resource_total_total_used_t(SI_RESOURCE_TOTAL*);

/****************************************************************************
 ****************************************************************************/

SI_RESOURCE*
SI_RESOURCE_TOTAL_Resource( SI_RESOURCE_TOTAL* pair )
{
  static si_resource_total_resource_t* so_call=0;

  if (!so_call && !(so_call = (si_resource_total_resource_t*)
		    dlsym(targ_handler, "SI_RESOURCE_TOTAL_Resource")))
    return NULL;

  return so_call(pair);
}

SI_RESOURCE_ID SI_RESOURCE_TOTAL_Resource_Id( SI_RESOURCE_TOTAL* pair )
{
  static si_resource_total_resource_id_t* so_call=0;

  if (!so_call && !(so_call = (si_resource_total_resource_id_t*)
		    dlsym(targ_handler, "SI_RESOURCE_TOTAL_Resource_Id")))
    return 0;

  return so_call(pair);
}

UINT SI_RESOURCE_TOTAL_Avail_Per_Cycle(SI_RESOURCE_TOTAL* pair)
{
  static si_resource_total_avail_per_cycle_t* so_call=0;

  if (!so_call && !(so_call = (si_resource_total_avail_per_cycle_t*)
		    dlsym(targ_handler, "SI_RESOURCE_TOTAL_Avail_Per_Cycle")))
    return 0;

  return so_call(pair);
}

INT SI_RESOURCE_TOTAL_Total_Used( SI_RESOURCE_TOTAL* pair )
{
  static si_resource_total_total_used_t* so_call=0;

  if (!so_call && !(so_call = (si_resource_total_total_used_t*)
		    dlsym(targ_handler, "SI_RESOURCE_TOTAL_Total_Used")))
    return 0;

  return so_call(pair);
}

/****************************************************************************
 ****************************************************************************/
typedef UINT si_rr_length_t (SI_RR);
typedef SI_RRW si_rr_cycle_rrw_t (SI_RR, UINT);

UINT SI_RR_Length( SI_RR req )
{
  static si_rr_length_t* so_call=0;

  if (!so_call && !(so_call = (si_rr_length_t*)
		    dlsym(targ_handler, "SI_RR_Length")))
    return 0;

  return so_call(req);
}

SI_RRW SI_RR_Cycle_RRW( SI_RR req, UINT cycle )
{
  static si_rr_cycle_rrw_t* so_call=0;

  if (!so_call && !(so_call = (si_rr_cycle_rrw_t*)
		    dlsym(targ_handler, "SI_RR_Cycle_RRW")))
    return 0;

  return so_call(req, cycle);
}

/****************************************************************************
 ****************************************************************************/
typedef SI_RR tsi_resource_requirement_t(TOP);
typedef SI_RR tsi_ii_resource_requirement_t(TOP, INT);
typedef const SI_RESOURCE_ID_SET* tsi_ii_cycle_resource_ids_used_t(TOP, INT);
typedef SI_BAD_II_SET tsi_bad_iis_t(TOP);
typedef SI_ID tsi_id_t(TOP);
typedef INT tsi_resource_total_vector_size_t(TOP);
typedef SI_RESOURCE_TOTAL * tsi_resource_total_vector_t(TOP);

SI_RR TSI_Resource_Requirement(TOP top)
{
  static tsi_resource_requirement_t* so_call=0;

  if (!so_call && !(so_call = (tsi_resource_requirement_t*)
		    dlsym(targ_handler, "TSI_Resource_Requirement")))
    return NULL;

  return so_call(top);
}

SI_RR TSI_II_Resource_Requirement(TOP top, INT ii)
{
  static tsi_ii_resource_requirement_t* so_call = 0;

  if (!so_call && !(so_call = (tsi_ii_resource_requirement_t*)
		    dlsym(targ_handler, "TSI_II_Resource_Requirement")))
    return NULL;

  return so_call(top, ii);
}

const SI_RESOURCE_ID_SET*
TSI_II_Cycle_Resource_Ids_Used( TOP opcode, INT ii )
{
  static tsi_ii_cycle_resource_ids_used_t* so_call=0;

  if (!so_call && !(so_call = (tsi_ii_cycle_resource_ids_used_t*)
		    dlsym(targ_handler, "TSI_II_Cycle_Resource_Ids_Used")))
    return NULL;

  return so_call (opcode, ii);
}

SI_BAD_II_SET TSI_Bad_IIs( TOP top )
{
  static tsi_bad_iis_t* so_call=0;

  if (!so_call)
    so_call = (tsi_bad_iis_t*) dlsym(targ_handler, "TSI_Bad_IIs");

  return so_call (top);
}

SI_ID TSI_Id( TOP top )
{
  static tsi_id_t* so_call=0;

  if (!so_call && !(so_call = (tsi_id_t*)
		    dlsym(targ_handler, "TSI_Id")))
    return 0;

  return so_call(top);
}

UINT TSI_Resource_Total_Vector_Size( TOP top )
{
  static tsi_resource_total_vector_size_t* so_call=0;

  if (!so_call && !(so_call = (tsi_resource_total_vector_size_t*)
		    dlsym(targ_handler, "TSI_Resource_Total_Vector_Size")))
    return 0;

  return so_call(top);
}

SI_RESOURCE_TOTAL* TSI_Resource_Total_Vector( TOP top )
{
  static tsi_resource_total_vector_t* so_call=0;

  if (!so_call && !(so_call = (tsi_resource_total_vector_t*)
		    dlsym(targ_handler, "TSI_Resource_Total_Vector")))
    return NULL;

  return so_call(top);
}

/****************************************************************************
 ****************************************************************************/

typedef const char* si_resource_name_t( SI_RESOURCE*);
typedef UINT si_resource_id_t( SI_RESOURCE*);
typedef UINT si_resource_avail_per_cycle_t( SI_RESOURCE*);
typedef UINT si_resource_word_index_t( SI_RESOURCE*);
typedef UINT si_resource_bit_index_t( SI_RESOURCE*);
typedef const char* si_resource_id_name_t( SI_RESOURCE_ID);
typedef UINT si_resource_id_avail_per_cycle_t( SI_RESOURCE_ID);


const char* SI_RESOURCE_Name( SI_RESOURCE* res )
{
  static si_resource_name_t* so_call = 0;
  if (! so_call && !(so_call = (si_resource_name_t*)
		     dlsym(targ_handler, "SI_RESOURCE_Name")))
    return NULL;

  return so_call (res);
}

UINT SI_RESOURCE_Id( SI_RESOURCE* res )
{
  static si_resource_id_t* so_call = 0;
  if (! so_call && !(so_call = (si_resource_id_t*)
		     dlsym(targ_handler, "SI_RESOURCE_Id")))
    return 0;

  return so_call (res);
}

UINT SI_RESOURCE_Avail_Per_Cycle( SI_RESOURCE* res )
{
  static si_resource_avail_per_cycle_t* so_call = 0;
  if (! so_call && !(so_call = (si_resource_avail_per_cycle_t*)
		     dlsym(targ_handler, "SI_RESOURCE_Avail_Per_Cycle")))
    return 0;

  return so_call (res);
}

UINT SI_RESOURCE_Word_Index( SI_RESOURCE* res )
{
  static si_resource_word_index_t* so_call = 0;
  if (! so_call && !(so_call = (si_resource_word_index_t*)
		     dlsym(targ_handler, "SI_RESOURCE_Word_Index")))
    return 0;

  return so_call (res);
}

UINT SI_RESOURCE_Bit_Index( SI_RESOURCE* res )
{
  static si_resource_bit_index_t* so_call = 0;
  if (! so_call && !(so_call = (si_resource_bit_index_t*)
		     dlsym(targ_handler, "SI_RESOURCE_Bit_Index")))
    return 0;

  return so_call (res);
}

const char* SI_RESOURCE_ID_Name( SI_RESOURCE_ID id )
{
  static si_resource_id_name_t* so_call = 0;
  if (! so_call && !(so_call = (si_resource_id_name_t*)
		     dlsym(targ_handler, "SI_RESOURCE_ID_Name")))
    return NULL;

  return  so_call (id);
}

UINT SI_RESOURCE_ID_Avail_Per_Cycle( SI_RESOURCE_ID id )
{
  static si_resource_id_avail_per_cycle_t* so_call = 0;
  if (! so_call && !(so_call = (si_resource_id_avail_per_cycle_t*)
		     dlsym(targ_handler, "SI_RESOURCE_ID_Avail_Per_Cycle")))
    return 0;

  return so_call (id);
}

/****************************************************************************
 ****************************************************************************/
INT TSI_Operand_Access_Time( TOP top, INT operand_index ) 
{
    typedef INT tsi_operand_access_time_t (TOP, INT)  ;
    static  tsi_operand_access_time_t* so_call = 0 ;
    if (!so_call && !(so_call = (tsi_operand_access_time_t*)
		      dlsym(targ_handler, "TSI_Operand_Access_Time")))
	return 0;
    
    return so_call(top, operand_index);
}

INT TSI_Result_Available_Time( TOP top, INT result_index )
{
    typedef INT tsi_result_available_time_t ( TOP , INT ) ;
    static tsi_result_available_time_t*  so_call = 0 ;
    if (!so_call && !(so_call = (tsi_result_available_time_t*)
		      dlsym(targ_handler, "TSI_Result_Available_Time")))
	return 0;
    
    return so_call(top, result_index);
}

INT TSI_Load_Access_Time( TOP top )
{
    typedef INT tsi_load_access_time_t  (TOP ) ;
    static  tsi_load_access_time_t* so_call = 0 ;
    if (!so_call && !(so_call = (tsi_load_access_time_t*)
		      dlsym(targ_handler, "TSI_Load_Access_Time")))
	return 0;
    
    return so_call(top);
}

INT TSI_Last_Issue_Cycle( TOP top ) 
{
    typedef INT tsi_last_issue_cycle_t (TOP ) ;
    static tsi_last_issue_cycle_t* so_call = 0 ;
    if (!so_call && !(so_call = (tsi_last_issue_cycle_t *)
		      dlsym(targ_handler, "TSI_Last_Issue_Cycle")))
	return 0;
    
    return so_call(top);
}

INT TSI_Store_Available_Time( TOP top )
{
    typedef INT tsi_store_available_time_t (TOP ) ;
    static tsi_store_available_time_t* so_call = 0 ;
    if (!so_call && !(so_call = (tsi_store_available_time_t *)
		      dlsym(targ_handler, "TSI_Store_Available_Time")))
	return 0;
    
    return so_call(top);
}

/****************************************************************************
 ****************************************************************************/

typedef INT si_id_count_t(void);
typedef const SI_RESOURCE_ID_SET* si_id_ii_cycle_resource_ids_used_t(SI_ID, INT);

INT SI_ID_Count(void)
{
  static si_id_count_t* so_call = 0;
  if (! so_call && !(so_call = (si_id_count_t*)
		     dlsym(targ_handler, "SI_ID_Count")))
    return 0;

  return so_call ();
}

const SI_RESOURCE_ID_SET*
SI_ID_II_Cycle_Resource_Ids_Used( SI_ID id, INT ii )
{
  static si_id_ii_cycle_resource_ids_used_t* so_call = 0;
  if (! so_call && !(so_call = (si_id_ii_cycle_resource_ids_used_t*)
		     dlsym(targ_handler, "SI_ID_II_Cycle_Resource_Ids_Used")))
    return NULL;

  return so_call (id, ii);
}

int SI_resource_count;

/****************************************************************************
 ****************************************************************************/
static void
SI_Resource_Count(void)
{
  int *ptr;
  ptr = (int*) dlsym(targ_handler, "SI_resource_count");
  if (ptr) {
    // Shadows the const declaration in ti_si.h. Expects warning here.
    SI_resource_count = *ptr;
  }
}
#endif

/* ====================================================================
 *
 *  TI_Initialize
 *
 *  See interface description
 *
 * ====================================================================
 */
void*
TI_Initialize(ABI_PROPERTIES_ABI tabi, ISA_SUBSET tisa, PROCESSOR tproc, char *tpath)
{
  static BOOL initialized;

  if ( !initialized ) {
    INT                i;
    const char        *targ_name     = PROCESSOR_Name(tproc);
    INT                targ_name_len = strlen(targ_name);
    char              *targ_so_name  = alloca(targ_name_len + sizeof(SO_EXT));

    for (i = 0; i < targ_name_len; i++) {
      targ_so_name[i] = tolower(targ_name[i]);
    }

    strcpy(targ_so_name + targ_name_len, SO_EXT);

    targ_handler = load_so(targ_so_name, tpath, FALSE /*verbose*/);

    ISA_SUBSET_Value = tisa;
    PROCESSOR_Value = tproc;
    ABI_PROPERTIES_ABI_Value = tabi;

    ABI_PROPERTIES_Initialize();
    ISA_HAZARD_Initialize();
    ISA_REGISTER_Initialize();

    initialized = TRUE;
    
    SI_Resource_Count();

    return targ_handler;
  }
}
