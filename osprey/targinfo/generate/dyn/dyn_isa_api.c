
/* 

  Copyright (C) 2006 ST Microelectronics, Inc.  All Rights Reserved. 

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

/**
 *
 * This file is only useful for dynamic extensions.
 *
 * It defines the structure that collects all dll function
 * pointers. This structure is used for communication between
 * loader and dll at connection time (see common/com/lai_loader.cxx).
 */
#include <stdio.h>                          /* NULL pointer */
#include "dyn_isa_topcode.h"
#include "dyn_isa_properties.h"
#include "dyn_isa_print.h"
#include "dyn_isa_lits.h"
#include "dyn_isa_subset.h"
#include "dyn_isa_registers.h"
#include "dyn_isa_api.h"
#include "dyn_isa_enums.h"
#include "dyn_isa_operands.h"
#include "dyn_isa_si.h"
#include "dyn_isa_injected.h"
#include "dyn_abi_properties.h"
#include "dyn_isa_bundle.h"
#include "dyn_isa_variants.h"
#include "dyn_isa_hazards.h"
#include "dyn_isa_parse.h"
#include "dyn_isa_pack.h"
#include "dyn_isa_decode.h"
#include "dyn_isa_binutils.h"

#ifndef DLL_EXPORTED
#define DLL_EXPORTED 
#endif
#ifdef __cplusplus
extern "C" {
#endif

/* Management of scheduling information */
static extension_si_t dyn_ISA_si[1] = {
  { dyn_get_processor_name,

    dyn_get_SI_resource_count,
    dyn_get_SI_resources,

    dyn_get_SI_RRW_initializer,
    dyn_get_SI_RRW_overuse_mask,

    dyn_get_SI_issue_slot_count,
    dyn_get_SI_issue_slots,

    dyn_get_SI_ID_si,
    dyn_get_SI_ID_count,

    dyn_get_SI_top_si,
  },
};

/* API routine */
static const extension_si_t* dyn_get_scheduling_info_tab(void) {
  return (dyn_ISA_si);
}

/* API routine */
static const mUINT32 dyn_get_scheduling_info_tab_sz(void) {
  return (1U);
}

/* Defining main API structure */
static ISA_EXT_Interface_t static_interface = {
   MAGIC_NUMBER_EXT_ISA_API,                 /* Magic number              */

   dyn_get_TOP_count,                        /* xxx_topcode.c             */
   dyn_get_TOP_name_tab,                     /* xxx_topcode.c             */
   dyn_belong_TOP_to_ext,                    /* xxx_topcode.c             */
   dyn_get_TOP_local,                        /* xxx_topcode.c             */
   dyn_set_TOP_base_offset,                  /* xxx_topcode.c             */
   dyn_get_TOP_AM_automod_variant,           /* xxx_isa_injected.c        */
   dyn_set_TOP_UNDEFINED,                    /* xxx_topcode.c             */

   dyn_get_TOP_prop_tab,                     /* xxx_targ_isa_properties.c */
   dyn_get_TOP_mem_bytes_tab,                /* xxx_targ_isa_properties.c */
   dyn_get_TOP_mem_align_tab,                /* xxx_targ_isa_properties.c */
   dyn_get_ISA_PROPERTIES_attribute_tab,     /* xxx_targ_isa_properties.c */
   dyn_get_ISA_PROPERTIES_attribute_tab_sz,  /* xxx_targ_isa_properties.c */
   
   dyn_get_ISA_PRINT_info_tab,               /* xxx_targ_isa_print.c      */
   dyn_get_ISA_PRINT_info_tab_sz,            /* xxx_targ_isa_print.c      */
   dyn_get_ISA_PRINT_info_index_tab,         /* xxx_targ_isa_print.c      */
   dyn_get_ISA_PRINT_name_tab,               /* xxx_targ_isa_print.c      */

   dyn_get_ISA_LIT_CLASS_info_tab,           /* xxx_targ_isa_lits.c       */
   dyn_get_ISA_LIT_CLASS_info_tab_sz,        /* xxx_targ_isa_lits.c       */
   dyn_get_ISA_LIT_CLASS_static_max,         /* xxx_targ_isa_lits.c       */

   dyn_get_ISA_OPERAND_operand_types_tab,    /* xxx_targ_isa_operands.c   */
   dyn_get_ISA_OPERAND_operand_types_tab_sz, /* xxx_targ_isa_operands.c   */
   dyn_get_ISA_OPERAND_info_tab,             /* xxx_targ_isa_operands.c   */
   dyn_get_ISA_OPERAND_info_tab_sz,          /* xxx_targ_isa_operands.c   */
   dyn_get_ISA_OPERAND_info_index_tab,       /* xxx_targ_isa_operands.c   */
   NULL, /* UNUSED relocs field, kept in order to simplify backward compat*/
   dyn_get_ISA_OPERAND_USE_attribute_tab,    /* xxx_targ_isa_operands.c   */
   dyn_get_ISA_OPERAND_USE_attribute_tab_sz, /* xxx_targ_isa_operands.c   */
   dyn_get_ISA_OPERAND_static_rclass_tab,    /* xxx_targ_isa_operands.c   */
   dyn_get_ISA_OPERAND_static_rclass_tab_sz, /* xxx_targ_isa_operands.c   */
   dyn_get_ISA_OPERAND_static_rsubclass_tab, /* xxx_targ_isa_operands.c   */
   dyn_get_ISA_OPERAND_static_rsubclass_tab_sz,/* xxx_targ_isa_operands.c */

   dyn_get_ISA_SUBSET_tab,                   /* xxx_targ_isa_subset.c     */ 
   dyn_get_ISA_SUBSET_tab_sz,                /* xxx_targ_isa_subset.c     */
   dyn_get_ISA_SUBSET_op_tab,                /* xxx_targ_isa_subset.c     */

   dyn_get_ISA_REGISTER_CLASS_tab,           /* xxx_targ_isa_registers.c  */
   dyn_get_ISA_REGISTER_CLASS_tab_sz,        /* xxx_targ_isa_registers.c  */
   dyn_get_ISA_REGISTER_CLASS_index_tab,     /* xxx_targ_isa_registers.c  */
   dyn_get_ISA_REGISTER_SUBCLASS_tab,        /* xxx_targ_isa_registers.c  */
   dyn_get_ISA_REGISTER_SUBCLASS_tab_sz,     /* xxx_targ_isa_registers.c  */

   dyn_get_ISA_injected_operation_tab,       /* xxx_isa_injected.c        */

   dyn_get_ABI_PROPERTIES_tab,               /* xxx_targ_abi_properties.c */
   dyn_get_ABI_PROPERTIES_attribute_tab,     /* xxx_targ_abi_properties.c */
   dyn_get_ABI_PROPERTIES_attribute_tab_sz,  /* xxx_targ_abi_properties.c */

   dyn_get_DW_DEBUG_EXT_name,                /* xxx_targ_abi_properties.c */
   dyn_get_DW_DEBUG_EXT_reloc_string,        /* xxx_targ_abi_properties.c */
   dyn_get_DW_DEBUG_EXT_reg_tab,             /* xxx_targ_abi_properties.c */

   dyn_get_scheduling_info_tab,              /* xxx_targ_isa_si.c         */
   dyn_get_scheduling_info_tab_sz,           /* xxx_targ_isa_si.c         */

   dyn_get_ISA_ENUM_CLASS_info_tab,          /* xxx_targ_isa_enums.c      */
   dyn_get_ISA_ENUM_CLASS_info_tab_sz,       /* xxx_targ_isa_enums.c      */
   dyn_get_ISA_ENUM_CLASS_VALUE_info_tab,    /* xxx_targ_isa_enums.c      */
   dyn_get_ISA_ENUM_CLASS_VALUE_info_tab_sz, /* xxx_targ_isa_enums.c      */

   dyn_get_ISA_EXEC_unit_prop_tab,           /* xxx_targ_isa_bundle.c     */

   dyn_get_ISA_VARIANT_INFO_tab,	     /* xxx_targ_isa_variants.c   */
   dyn_get_ISA_VARIANT_attribute_tab,	     /* xxx_targ_isa_variants.c   */
   dyn_get_ISA_VARIANT_attribute_tab_sz,     /* xxx_targ_isa_variants.c   */

   dyn_get_ISA_HAZARDS_index_tab,            /* xxx_targ_isa_hazards.c    */

   /* Put it for backward compatibility issue. */
   dyn_get_ISA_EXEC_unit_slots_tab,          /* xxx_targ_isa_bundle.c     */
   dyn_get_ISA_BUNDLE_slot_count_tab,        /* xxx_targ_isa_bundle.c     */

   /* Used by GNU binutils */
   dyn_get_ISA_PARSE_tab,                    /* xxx_targ_isa_parse.c      */ 
   dyn_get_ISA_PARSE_tab_sz,                 /* xxx_targ_isa_parse.c      */

   dyn_get_ISA_PACK_OPND_info_tab,           /* xxx_targ_isa_parse.c      */
   dyn_get_ISA_PACK_OPND_info_tab_sz,        /* xxx_targ_isa_parse.c      */
   dyn_get_ISA_PACK_OPND_info_index_tab,     /* xxx_targ_isa_parse.c      */
   dyn_get_ISA_PACK_info_tab,                /* xxx_targ_isa_parse.c      */
   dyn_get_ISA_PACK_inst_words_tab,          /* xxx_targ_isa_pack.c       */ 
   dyn_get_ISA_PACK_adj_info_tab,            /* xxx_targ_isa_parse.c      */
   dyn_get_ISA_PACK_adj_info_tab_sz,         /* xxx_targ_isa_parse.c      */
   dyn_get_ISA_PACK_adj_info_index_tab,      /* xxx_targ_isa_parse.c      */

   dyn_get_ISA_DECODE_decoding_tab,          /* xxx_targ_isa_decode.c     */
   dyn_get_ISA_DECODE_decoding_tab_sz,       /* xxx_targ_isa_decode.c     */

   dyn_get_ISA_RELOC_info_tab,               /* xxx_targ_isa_relocs.c     */
   dyn_get_ISA_RELOC_info_tab_sz,            /* xxx_targ_isa_relocs.c     */
   dyn_get_ISA_RELOC_SUBSET_info_tab,        /* xxx_targ_isa_relocs.c     */
   dyn_get_ISA_RELOC_SUBSET_info_tab_sz,     /* xxx_targ_isa_relocs.c     */
   dyn_get_ISA_RELOC_max_static_virtual_id_core_subset,/* xxx_targ_isa_relocs.c */
   dyn_set_ISA_RELOC_dynamic_reloc_offset,   /* xxx_targ_isa_relocs.c     */

   dyn_get_ISA_BINUTILS_info_tab,            /* xxx_targ_isa_binutils.c   */
   dyn_get_ISA_BINUTILS_info_tab_sz,         /* xxx_targ_isa_binutils.c   */

   dyn_set_ISA_REGISTER_CLASS_offset,        /* xxx_targ_isa_registers.c  */
   dyn_set_ISA_REGISTER_SUBCLASS_offset,     /* xxx_targ_isa_registers.c  */

   dyn_set_ISA_LIT_CLASS_offset,             /* xxx_targ_isa_lits.c       */

   dyn_get_ISA_RELOC_variant_info_tab,       /* xxx_targ_isa_relocs.c     */
   dyn_get_ISA_RELOC_variant_info_tab_sz,    /* xxx_targ_isa_relocs.c     */
};

/* Exporting global pointer on static internal structure.             */
DLL_EXPORTED const ISA_EXT_Interface_t *get_isa_extension_instance() {
  return &static_interface;
}

#define STRINGIFY(s)  #s
#define XSTRINGIFY(s) STRINGIFY(s)
/* Exporting function to access extension name.                       */
DLL_EXPORTED const char *get_extension_name() {
    return (XSTRINGIFY(EXTN));
}
#undef XSTRINGIFY
#undef STRINGIFY

#ifdef __cplusplus
}                                        /* End of "C" block          */
#endif
