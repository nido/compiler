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


/* ====================================================================
 * ====================================================================
 *
 *  Module: ti_bundle.h
 *
 *  Synopsis:
 *
 *      Bundle (or template) accounting package for various clients including
 *	the software pipeliner, local (BB) scheduler, disassembler and 
 *      linker. 
 *
 *  Interface Description:
 *
 *	Exported types:
 *
 *      Exported functions:
 *
 *	    BOOL TI_BUNDLE_Has_Property(
 *		TI_BUNDLE *bundle
 *		ISA_EXEC_UNIT_PROPERTY property
 *              INT *error
 *	    )
 *
 *		Returns TRUE if <bundle> contains an instance of <property>.
 *              If an error occurs, <error> is set to TI_RC_ERROR.
 *
 *	    BOOL TI_BUNDLE_Is_Full(
 *		TI_BUNDLE *bundle
 *              INT *error
 *	    )
 *
 *		Returns TRUE if <bundle> is fully packed with insts. If 
 *              an error occurs, <error> is set to TI_RC_ERROR.
 *
 *	    BOOL TI_BUNDLE_Is_Empty(
 *		TI_BUNDLE *bundle
 *              INT *error
 *	    )
 *
 *		Returns TRUE if <bundle> is empty. If an error occurs, 
 *		<error> is set to TI_RC_ERROR.
 *
 *          INT TI_BUNDLE_Return_Template(
 *              TI_BUNDLE *bundle
 *          )
 *              
 *              Returns the template encoding bit for the slot pattern in
 *              <bundle>.
 *              
 *          void TI_BUNDLE_Clear(
 *              TI_BUNDLE *bundle
 *          )
 *              
 *              Reset the contents of the bundle.
 *
 *	    BOOL TI_BUNDLE_Slot_Available(
 *		TI_BUNDLE  *bundle
 *		ISA_EXEC_UNIT_PROPERTY  property
 *		INT slot
 *	    )
 *
 *              Check to see if 'slot' for 'property' is available for
 *		the given 'bundle'. Returns TRUE if available; FALSE 
 *		otherwise.
 *
 *	    BOOL TI_BUNDLE_Stop_Bit_Available(
 *		TI_BUNDLE  *bundle
 *		INT slot
 *	    )
 *
 *              Check to see if a stop bit can be set for 'slot'
 *		in the given 'bundle'. Returns TRUE if available; FALSE 
 *		otherwise.  
 *
 *	    void TI_BUNDLE_Reserve_Slot(
 *		TI_BUNDLE  *bundle
 *		INT        slot
 *              ISA_EXEC_UNIT_PROPERTY property
 *	    )
 *
 *              Reserve 'slot' for 'property' at the given 'bundle'.
 *
 *	    void TI_BUNDLE_Reserve_Stop_Bit(
 *		TI_BUNDLE  *bundle
 *		INT        slot
 *	    )
 *
 *              Reserve 'slot' position for STOP bit at the given 'bundle'.
 *
 *	    void TI_BUNDLE_Unreserve_Stop_Bit(
 *		TI_BUNDLE  *bundle
 *		INT        slot
 *	    )
 *
 *              Unreserve 'slot' position for STOP bit at the given 'bundle'.
 *
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef ti_bundle_INCLUDED
#define ti_bundle_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "topcode.h"


typedef struct ti_bundle {
  ISA_BUNDLE_INFO *bundle_info; /* exported interface from targ_info */
  BOOL slot_filled[ISA_BUNDLE_MAX_SLOTS];  
} TI_BUNDLE;

/* TI_BUNDLE accessors:
 */

#define TI_BUNDLE_bundle_info(t)            ((t)->bundle_info)
#define TI_BUNDLE_slot_count(t)             ((t)->bundle_info->slot_count)
#define TI_BUNDLE_exec_property(t,i)        ((t)->bundle_info->slot[i])
#define TI_BUNDLE_stop_bit(t,i)             ((t)->bundle_info->stop[i])
#define TI_BUNDLE_slot_filled(t,i)          ((t)->slot_filled[i])
#define TI_BUNDLE_slot_mask(t)              ((t)->bundle_info->slot_mask)
#define TI_BUNDLE_stop_mask(t)              ((t)->bundle_info->stop_mask)
#define TI_BUNDLE_pack_code(t)	    	    ((t)->bundle_info->pack_code)

#define Set_TI_BUNDLE_exec_property(t, i, value) ((t)->bundle_info->slot[i] = \
						  (value))
#define Set_TI_BUNDLE_slot_mask(t, i)       ((t)->bundle_info->slot_mask = i)
#define Set_TI_BUNDLE_stop_mask(t, i)       ((t)->bundle_info->stop_mask = i)
#define Set_TI_BUNDLE_slot_count(t, i)      ((t)->bundle_info->slot_count = i)
#define Set_TI_BUNDLE_stop_bit(t, i, value) ((t)->bundle_info->stop[i] = \
						  (value))
#define Set_TI_BUNDLE_pack_code(t, value)   ((t)->bundle_info->pack_code = \
    	    	    	    	    	    	  (value))

/* Iterator Macros */

#define FOR_ALL_SLOT_MEMBERS(bundle, i) \
     for (i = 0; i < TI_BUNDLE_slot_count(bundle); ++i)

#ifdef TARG_ST
#define FOR_ALL_SLOT_MEMBERS_IN_REVERSE(bundle, i) \
     for (i = TI_BUNDLE_slot_count(bundle) - 1; i >= 0; --i)
#endif

inline BOOL
TI_BUNDLE_Stop_Bit_Present(TI_BUNDLE *bundle) {
  INT i;
  for (i = 0; i < TI_BUNDLE_slot_count(bundle) - 1; ++i) {
    if (TI_BUNDLE_stop_bit(bundle, i)) return TRUE;
  }
  return FALSE;
}

#ifdef TARG_ST
TARGINFO_EXPORTED extern void TI_BUNDLE_initialize(int max_slot);
#endif

TARGINFO_EXPORTED extern BOOL TI_BUNDLE_Has_Property(
  TI_BUNDLE *bundle,
  ISA_EXEC_UNIT_PROPERTY property,
  INT *error
);

TARGINFO_EXPORTED extern BOOL TI_BUNDLE_Is_Full(
  TI_BUNDLE *bundle,
  INT *error
);

TARGINFO_EXPORTED extern BOOL TI_BUNDLE_Is_Empty(
  TI_BUNDLE *bundle,
  INT *error
);

TARGINFO_EXPORTED extern INT TI_BUNDLE_Return_Template(
  TI_BUNDLE *bundle
);

TARGINFO_EXPORTED extern void TI_BUNDLE_Clear(
  TI_BUNDLE *bundle
);

TARGINFO_EXPORTED extern BOOL TI_BUNDLE_Slot_Available(
  TI_BUNDLE  *bundle,
  ISA_EXEC_UNIT_PROPERTY  property,
  INT        slot
);

TARGINFO_EXPORTED extern BOOL TI_BUNDLE_Stop_Bit_Available(
  TI_BUNDLE  *bundle,
  INT        slot
);

#ifdef TARG_ST
TARGINFO_EXPORTED extern ISA_EXEC_MASK TI_BUNDLE_Set_Slot_Mask_Property(
  ISA_EXEC_MASK slot_mask,
  INT slot,
  ISA_EXEC_UNIT_PROPERTY property
);

TARGINFO_EXPORTED extern ISA_EXEC_UNIT_PROPERTY TI_BUNDLE_Get_Slot_Mask_Property(
  ISA_EXEC_MASK slot_mask,
  INT slot
);
#endif

TARGINFO_EXPORTED extern void TI_BUNDLE_Reserve_Slot(
  TI_BUNDLE  *bundle,
  INT slot,
  ISA_EXEC_UNIT_PROPERTY property
);

TARGINFO_EXPORTED extern void TI_BUNDLE_Reserve_Stop_Bit(
  TI_BUNDLE  *bundle,
  INT slot
);

TARGINFO_EXPORTED extern void TI_BUNDLE_Unreserve_Stop_Bit(
  TI_BUNDLE  *bundle,
  INT slot
);

#ifdef __cplusplus
}
#endif
#endif /* ti_res_res_INCLUDED */

