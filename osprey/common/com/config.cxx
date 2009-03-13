
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
 * Module: config.c
 *
 * Revision history:
 *  06-Jun-90 -	Original Version (moved	from cdriver.c)
 *  01-Feb-91 -	Copied for TP/Muse
 *  15-Jun-91 -	Restructured and integrated Josie
 *  05-May-96 -	Added -WOPT group.
 *
 * Description:
 *
 * Configuration data and routines to set up configuration.
 *
 * Refer to the	discussion in config.h for the distribution of such
 * data	and processing among the various configuration files.
 *
 * ====================================================================
 * ====================================================================
 */

#ifdef _KEEP_RCS_ID
/*REFERENCED*/
#endif /* _KEEP_RCS_ID */

#ifdef FRONT_END	/* For setting fullwarn, woff in front end */
#ifndef FRONT_F90
#ifdef EDGSRC
# include "basics.h"
# include "cmd_line.h"
# include "error.h"
#endif /* EDGSRC */
#endif /* ~FRONT_F90 */
#endif /*  FRONT_END */
#include <ctype.h>	/* For isdigit */
#include <elf.h>
#include "W_bstring.h"

#define USE_STANDARD_TYPES 1
#include "defs.h"
#include "em_elf.h"
#include "config.h"
#include "config_platform.h"
#ifdef TARG_ST 
#include "config_target.h"
#else
#include "config_targ.h"
#endif
#include "erglob.h"
#include "flags.h"
#include "tracing.h"
#include "glob.h"
#include "symtab.h"
#include "wn.h"

#ifndef BACK_END
static INT32 Ignore_Int;
#endif

/* The following contains the phase-specific option groups and their
 * associated variable definitions:
 */
#include "config_TARG.cxx"
#include "config_debug.cxx"
#include "config_ipa.cxx"
#include "config_list.cxx"
#include "config_opt.cxx"
#include "config_wopt.cxx"
#include "config_vho.cxx"
#include "config_flist.cxx"
#include "config_clist.cxx"
#include "config_purple.cxx"
#include "config_promp.cxx"

#ifdef BACK_END
# include "config_lno.cxx"
# include "instr_reader.h"
#endif


//TB:
#include "register_preg.h" // ISA_REGISTER_CLASS
/* IR builder sometimes	needs to know whether we're in front end: */
#ifdef SINGLE_PROCESS
INT16 In_Front_End = TRUE;	/* Start out there */
#endif

/* ====================================================================
 *
 *   Target-specific controls: 
 *
 *   Moved declarations common to all targets here from 
 *   ia64/config_target.cxx
 *
 * ====================================================================
 */

/* We need to know the machine type of the integer and floating point
 * registers for purposes of subprogram save/restore:
 */
CLASS_INDEX Spill_Int_Mtype = 0;
CLASS_INDEX Spill_Ptr_Mtype = MTYPE_UNKNOWN;
CLASS_INDEX Spill_Float_Mtype = 0;

/* The maximum integer machine type corresponding directly to the
 * machine registers, and the default integer machine type:
 */
CLASS_INDEX Max_Int_Mtype = 0;
CLASS_INDEX Max_Uint_Mtype = 0;
CLASS_INDEX Def_Int_Mtype = 0;
CLASS_INDEX Def_Uint_Mtype = 0;

/* On MIPS III targets, should we use 32-bit pointers? */
BOOL Use_32_Bit_Pointers = FALSE;

/* For various targets, what are the characteristics of pointers */
INT		Pointer_Size; 
#ifdef TARG_ST
ISA_REGISTER_CLASS Pointer_Register_Class;
#endif
CLASS_INDEX	Pointer_Mtype;
CLASS_INDEX	Pointer_Mtype2;

/* What are pointers and booleans aliased to in WHIRL */
TYPE_ID Pointer_type;
TYPE_ID Pointer_type2;
TYPE_ID Boolean_type;
TYPE_ID Boolean_type2;
TYPE_ID Integer_type;

/* For various targets, what is the comparison result type? */
INT		Comparison_Result_Size;		/* in bytes */
CLASS_INDEX	Comparison_Result_Mtype;

/* ====================================================================
 *
 * Target debugging options
 * Arhtur: this comes from ia64/config_targ.cxx
 * ====================================================================
 */

/* Symbolic Debug mode as specified on command line.  (The mode can
 * change from PU to PU because, for example, we encounter a call to
 * the routine 'alloca()' -- we must restore the mode to the value as
 * it was specified on the command line, so we remember it.)
 */
INT16 Symbolic_Debug_Mode;
INT16 Max_Symbolic_Debug_Mode;	/* Maximum for any PU */

/* ====================================================================
 *
 * Global option flags
 *
 * ====================================================================
 */

/***** General optimization control *****/
BOOL Enable_LOH = FALSE;		/* Do loop overhead processing? */
BOOL Enable_LOH_overridden = FALSE;	/* ... option seen? */
BOOL CSE_Elim_Enabled = FALSE;		/* Is CSE-elim on? -- this does
					 * not control it, it just
					 * shadows the opt. level
					 */
BOOL Enable_LAI = FALSE;               /* Generate Lai_Code ? */

#ifdef BACK_END
# define DEF_DEBUG_LEVEL        0
INT8 Debug_Level = DEF_DEBUG_LEVEL;     /* -gn: debug level */
#endif


/***** Alignment (misaligned memory reference) control *****/
BOOL	UseAlignedCopyForStructs = FALSE;	/* control aggregrate copy */
INT32	MinStructCopyLoopSize =    16;		/* 0 = always expand */
INT32	MinStructCopyMemIntrSize=  0;		/* generate bcopy */
BOOL MinStructCopyMemIntrSize_Set = FALSE;
INT32	Aggregate_Alignment = -1;		/* This alignment for aggregate layout */
#ifdef TARG_ST
static BOOL	UseMemcpy = FALSE;			/* Use memcpy instead of inlined copy */
static BOOL	UseMemcpy_Set = FALSE;
INT32   Scalar_Struct_Limit = -1;
BOOL UnrollLoops = TRUE;
BOOL UnrollLoops_Set = FALSE;
INT32	MinStructCopyParallel     =    0;	/* 0 or 1 = do not generate parallel moves for struct copies */
BOOL    MinStructCopyParallel_Set =    FALSE;
#endif

INT32 iolist_reuse_limit = 100;

/***** Pointer optimizations, such as treating pointers as arrays *****/
BOOL Ptr_Opt_Allowed = FALSE;

/***** Put all-zero initialized file-level data in the BSS section? *****/
BOOL Zeroinit_in_bss = TRUE;

/* don't make strings gp-relative (to save gp space) */
BOOL Strings_Not_Gprelative = FALSE;

/***** IEEE 754 options *****/
IEEE_LEVEL IEEE_Arithmetic = IEEE_ACCURATE; /* IEEE arithmetic? */
BOOL IEEE_Arith_Set = FALSE;	/* ... option seen? */

/***** Speculation eagerness options *****/
EAGER_LEVEL Eager_Level = EAGER_SAFE;	/* Eagerness to use: -Xn */
static BOOL Eager_Level_Set = FALSE;	/* ... option seen? */

/***** Endianness options *****/
static BOOL Is_Little_Endian_Set = FALSE;
static BOOL Is_Big_Endian_Set = FALSE;

/***** Constant folding and WHIRL simplifier options *****/
ROUNDOFF Roundoff_Level = ROUNDOFF_NONE;/* -OPT_roundoff=n value */
BOOL Roundoff_Set = FALSE;		/* ... option seen? */
BOOL Fast_Complex_Allowed = FALSE;	/* Fast c_div and c_abs? */
BOOL Fast_Complex_Set = FALSE;		/* ... option seen? */
BOOL Fast_Bit_Allowed = FALSE;		/* Fast inlined bit intrinsics? */
BOOL Fast_Bit_Set = FALSE;		/* ... option seen? */
BOOL Fast_NINT_Allowed = FALSE;		/* Fast NINT and ANINT? */
BOOL Fast_NINT_Set = FALSE;		/* ... option seen? */
BOOL Fast_trunc_Allowed = FALSE;	/* Fast truncs for NINT/ANINT/AINT/AMOD? */
BOOL Fast_trunc_Set = FALSE;		/* ... option seen? */
BOOL Inline_Intrinsics_Allowed = TRUE;	/* Inline intrinsics? Or lib calls? */
BOOL Inline_Intrinsics_Set = FALSE;	/* ... option seen? */
BOOL Regions_Around_Inner_Loops = FALSE;/* Put REGIONs around inner loops? */
BOOL Region_Boundary_Info = FALSE;	/* calc boundary info for regions */
BOOL Simp_Multiply_To_Shift=FALSE;      /* Convert multiplies to shifts */
BOOL Enable_NaryExpr= FALSE;		/* allow nary expression in the lowerer */
BOOL Enable_NaryExpr_Set = FALSE;	/* ... option seen? */

/***** LANGuage group options *****/
static char *Language_Name = NULL;	/* Source language name */
LANGUAGE Language = LANG_UNKNOWN;	/* See language.h */
BOOL CXX_Bool_On = TRUE;
BOOL CXX_Bool_Set = FALSE;
BOOL CXX_Exceptions_On = TRUE;
BOOL CXX_Exceptions_Set = FALSE;
BOOL CXX_Alias_Const=FALSE;
BOOL CXX_Alias_Const_Set=FALSE;
BOOL LANG_Recursive = FALSE;
BOOL LANG_Recursive_Set = FALSE;
BOOL CXX_Wchar_On = TRUE;
BOOL CXX_Wchar_Set = FALSE;
BOOL CXX_Namespaces_On = TRUE; 
BOOL CXX_Namespaces_Set = FALSE;
BOOL CXX_Ansi_For_Init_Scope_On = FALSE;
BOOL CXX_Ansi_For_Init_Scope_Set = FALSE;
BOOL CXX_Standard_C_Plus_Plus_On = FALSE;
BOOL CXX_Standard_C_Plus_Plus_Set = FALSE;
BOOL C_Restrict_On = FALSE;
BOOL C_Restrict_Set = FALSE;
char *C_Auto_Restrict = NULL;
BOOL C_Auto_Restrict_Set = FALSE;
BOOL FTN_Short_Circuit_On = FALSE;
BOOL FTN_Short_Circuit_Set = FALSE;
BOOL Macro_Expand_Pragmas_On = FALSE;
BOOL Macro_Expand_Pragmas_Set = FALSE;
BOOL C_VLA_On = FALSE;
BOOL C_VLA_Set = FALSE;
BOOL CXX_Typename_On = TRUE; 
BOOL CXX_Typename_Set = FALSE;
BOOL CXX_Explicit_On = TRUE; 
BOOL CXX_Explicit_Set = FALSE;
BOOL CXX_Mutable_On = TRUE; 
BOOL CXX_Mutable_Set = FALSE;
BOOL CXX_Packed_On = FALSE; 
BOOL CXX_Packed_Set = FALSE;
BOOL LANG_Symtab_Verify_On = TRUE;
BOOL LANG_Symtab_Verify_Set = TRUE;
BOOL LANG_Ansi_Setjmp_On = TRUE; 
BOOL LANG_Ansi_Setjmp_Set = FALSE;
BOOL LANG_Ignore_Carriage_Return_On = TRUE; 
BOOL LANG_Ignore_Carriage_Return_Set = FALSE;

BOOL LANG_Pch;
BOOL LANG_Pch_Set;
char *LANG_Create_Pch;
BOOL LANG_Create_Pch_Set;
char *LANG_Use_Pch;
BOOL LANG_Use_Pch_Set;
char *LANG_Pchdir;
char *LANG_cxx_dialect;
BOOL LANG_Pchdir_Set;
BOOL LANG_cxx_dialect_Set;
BOOL LANG_Microsoft_Mode = FALSE;
BOOL LANG_Microsoft_Mode_Set = FALSE;

/***** INTERNAL group options *****/

BOOL WHIRL_Merge_Types_On = FALSE; 
BOOL WHIRL_Merge_Types_Set = FALSE;
BOOL WHIRL_Comma_Rcomma_On = TRUE;
BOOL WHIRL_Comma_Rcomma_Set = FALSE;
#ifdef TARG_ST100
BOOL WHIRL_Mtype_A_On = TRUE;
#else
BOOL WHIRL_Mtype_A_On = FALSE;
#endif
#ifdef TARG_IA64
BOOL WHIRL_Mtype_B_On = TRUE;
#else
#ifdef TARG_ST100
BOOL WHIRL_Mtype_B_On = TRUE;
#else
BOOL WHIRL_Mtype_B_On = FALSE;
#endif
#endif
BOOL WHIRL_Mtype_BS_On = FALSE;
BOOL WHIRL_Flatten_Field_On = FALSE;
BOOL WHIRL_Vfcall_On = FALSE;
BOOL WHIRL_Addr_Passed_On = FALSE;
BOOL WHIRL_Addr_Saved_For_Passed_On = FALSE;
BOOL WHIRL_Addr_Saved_On = TRUE;
BOOL WHIRL_Keep_Cvt_On = DEFAULT_KEEP_CVT;

BOOL Global_Pragmas_In_Dummy_PU_On = TRUE;
BOOL Malloc_Free_On     = TRUE;
BOOL Alloca_Dealloca_On = TRUE;
BOOL Barrier_Lvalues_On = TRUE;

/***** F90 Heap/stack allocation threshold */
INT32 Heap_Allocation_Threshold=-1;      /* Allocate objects > this on the heap 
					 * (-1 means always use stack), 0 always use heap
					 * default is -1
					 */

/***** Miscellaneous code generation options *****/
INT32 Short_Data = DEF_SDATA_ELT_SIZE;	/* Objects of this size in .sdata */
static BOOL Short_Data_Set = FALSE;	/* ... option seen? */
INT32 Short_Lits = DEF_SDATA_ELT_SIZE; /* Literals of this size in .litX */
static BOOL Short_Lits_Set = FALSE;	/* ... option seen? */
INT32 Max_Sdata_Elt_Size = DEF_SDATA_ELT_SIZE;	/* -Gn: sdata size */
INT32 Max_Srdata_Elt_Size = DEF_SRDATA_ELT_SIZE;
static BOOL Max_Srdata_Elt_Size_Set = FALSE;

BOOL Constant_GP = FALSE;		/* gp never changes? */

/* ====================================================================
 *
 * Option groups (see flags.h)
 *
 * When defining a new option group, remember to not only add an
 * option descriptor list (e.g. Options_TENV), but also to add an
 * entry describing the group in Common_Option_Groups, below.
 *
 * ====================================================================
 */

/* Temporary variables used for holding GOT size options during option
 * processing until Guaranteed_Small_GOT can be set properly:
 */
static BOOL Use_Small_GOT = FALSE;
static BOOL Use_Large_GOT = FALSE;
INT32 Gspace_Available = DEFAULT_GSPACE;

/* Always force EH Region offsets to be long */
BOOL Force_Long_EH_Range_Offsets = FALSE;
/* Force stack frame to use large model */
BOOL Force_Large_Stack_Model = FALSE;
BOOL Force_GP_Prolog;	/* force usage of gp prolog */
#ifdef TARG_ST
BOOL Auto_align_stack = FALSE;        /* Auto align stack */
#endif

OPTION_LIST *Registers_Not_Allocatable = NULL;
#ifdef TARG_ST
// [TTh] List of disabled registers (neither allocatable
// nor usable in asm stmt clobber list or variable decl
// with register keyword)
OPTION_LIST *Disabled_Registers = NULL;
#endif

/* Unique ident from IPA */
INT32 Ipa_Ident_Number = 0;
#ifdef TARG_ST
// [CL] unique label suffix to ensure that two ipa
// link phases don't generate the same symbol names
// (causes problems when linking together two files
// generated in relocatable mode under IPA)
char *Ipa_Label_Suffix = "";
char *Ipa_Exec_Name = NULL;
#endif

#ifdef TARG_ST
// FdF: builtin_prefetch in the code will be ignored if the option
// -fno-builtin-prefetch is used
BOOL Ignore_Builtin_Prefetch = FALSE;
INT32 Prefetch_Optimize = 3;
#endif

#ifdef TARG_ST
// [CG]: Enable emulation for all floating point ops
// If this is set it disable any floating point op mapping, whatever the target dependent flags.
// Thus it guarantees that any floating point operation is emulated.
// This flag should be always FALSE by default, and used only to force emulation when the target has some support for floating point.
BOOL Emulate_FloatingPoint_Ops;
BOOL Emulate_FloatingPoint_Ops_Set;
// [CG]: Enable Single Float Emulation
// If this is set, the default for 'float' operations is to be emulated unless a target dependent flag forces a specific
// operator to not be emulated.
// Thus, this flag is not a guarantee that any 'float' operation is emulated. It is an optimization hint only.
// This flag should be FALSE by default for target with IEEE float support, TRUE otherwise. (must be set in config_target.cxx).
BOOL Emulate_Single_Float_Type;
BOOL Emulate_Single_Float_Type_Set;
// [CG]: Enable Double Float Emulation
// If this is set, the default for 'double' operations is to be emulated unless a target dependent flag forces a specific
// operator to not be emulated.
// Thus, this flag is not a guarantee that any 'double' operation is emulated. It is an optimization hint only.
// This flag should be FALSE by default for target with IEEE double support, TRUE otherwise. (must be set in config_target.cxx).
BOOL Emulate_Double_Float_Type;
BOOL Emulate_Double_Float_Type_Set;
// [CM]: Enable Division or Remainder Integer Emulation
// Optimization hint that Div/rem are available on the target.
// This flag should be FALSE by default for target with div/rem support, TRUE otherwise. (must be set in config_target.cxx).
BOOL Emulate_DivRem_Integer_Ops;
BOOL Emulate_DivRem_Integer_Ops_Set;
#endif

#ifdef TARG_ST
// [CG]: Enable transformation of builtins. 
// For instance: strlen("toto") -> 4
// Normally activated at Opt_Level >= 1.
// This flag does not disable builtin mapping to intrinsics, 
// use front-end flag -fno-builtins for this
BOOL Enable_Expand_Builtin = FALSE;
BOOL Enable_Expand_Builtin_Set = FALSE;
char *Extension_Names = NULL;
BOOL Extension_Is_Present = FALSE;
INT32 Enable_Extension_Native_Support = EXTENSION_NATIVE_SUPPORT_DEFAULT;
BOOL Enable_Extension_Native_Support_Set = FALSE;
char *Disabled_Native_Extensions = NULL;
BOOL Disabled_Native_Extensions_Set = FALSE;

// extra options enabling to activate/block the
// Enable_Extension_Native_Support mask bit per bit.
INT32 Activate_Extension_Native_Support_Bits = 0;
BOOL Activate_Extension_Native_Support_Bits_Set = FALSE;
INT32 Block_Extension_Native_Support_Bits = 0;
BOOL Block_Extension_Native_Support_Bits_Set = FALSE;

BOOL Meta_Instruction_Threshold_Set = FALSE;
INT32 Meta_Instruction_Threshold = INT_MAX;
BOOL Meta_Instruction_By_Size_Set = FALSE;
BOOL Meta_Instruction_By_Size = FALSE;
#endif

BOOL Indexed_Loads_Allowed = FALSE;

/* Target environment options: */
static OPTION_DESC Options_TENV[] = {
  { OVK_INT32,	OV_VISIBLE,	FALSE, "align_aggregates",	"align_ag",
    -1, 0, 16,	&Aggregate_Alignment, NULL,
    "Minimum alignment to use for aggregates (structs/arrays)" },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "aligned_copy",		NULL,
    0, 0, 0,	&UseAlignedCopyForStructs, NULL },
  { OVK_BOOL,   OV_SHY,		FALSE, "call_mcount",		NULL,
    0, 0, 0,    &Call_Mcount, NULL },
  { OVK_BOOL,   OV_SHY,		FALSE, "constant_gp",		NULL,
    0, 0, 0,    &Constant_GP, NULL },
  { OVK_BOOL,	OV_SHY,		FALSE, "cpic",			"cp",
#ifdef TARG_ST
    0, 0, 0, &Gen_PIC_Call_Shared, &Gen_PIC_Call_Shared_Set,
#else
    0, 0, 0, &Gen_PIC_Call_Shared, NULL,
#endif
    "Generate code for executable programs which may call DSOs" },
#ifdef TARG_ST
  { OVK_BOOL,	OV_SHY,		FALSE, "no_shared_warning",	NULL,
    0, 0, 0, &No_Shared_Warning, NULL,
    "Disable warning when mixinig modules compiled for different code generation models" },
#endif
#ifdef TARG_ST
  { OVK_NAME,	OV_VISIBLE,	FALSE, "visibility",	NULL,
    0, 0, 0, &ENV_Symbol_Visibility_String, NULL,
    "Specify default symbol visibility (default is STV_DEFAULT)" },
  { OVK_NAME,	OV_VISIBLE,	FALSE, "visibility-decl",	NULL,
    0, 0, 0, &ENV_Symbol_Visibility_Spec_Filename, NULL,
    "Use the specified visibility declaration file" },
#endif

  { OVK_BOOL,	OV_VISIBLE,	FALSE, "fixed_addresses",	"fi",
    0, 0, 0,	&PIC_Fixed_Addresses, NULL },
  { OVK_INT32,	OV_SHY,		FALSE, "Gspace",		NULL,
    DEFAULT_GSPACE,0,INT32_MAX,	&Gspace_Available, NULL,
    "Maximum GP-relative space available" },
  { OVK_UINT32,	OV_INTERNAL,	FALSE, "ipa_ident",		NULL, 
    0, 0, UINT32_MAX, &Ipa_Ident_Number, NULL,
    "Specify IPA timestamp number" },
#ifdef TARG_ST
  { OVK_NAME,	OV_INTERNAL,	FALSE, "ipa_suffix",		NULL, 
    0, 0, 0, &Ipa_Label_Suffix, NULL,
    "Specify IPA unique suffix" },
  { OVK_NAME,	OV_INTERNAL,	FALSE, "ipa_exec_name",		NULL, 
    0, 0, 0, &Ipa_Exec_Name, NULL,
    "Specify executable name when running IPA phase" },
#endif
  { OVK_BOOL,	OV_VISIBLE,	FALSE, "kernel",		NULL,
    0, 0, 0,	&Kernel_Code,	NULL,
    "Generate code for kernel use" },
  { OVK_BOOL,	OV_VISIBLE,	FALSE, "large_GOT",		"",
    0, 0, 0,	&Use_Large_GOT, NULL,
    "Assume GOT is larger than 64K bytes" },
  { OVK_NAME,	OV_SHY,		FALSE, "io_library",		NULL,
    0, 0, 0,	&Library_Name, NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "large_stack",		NULL,
    0, 0, 0,	&Force_Large_Stack_Model, NULL,
    "Generate code assuming >32KB stack frame" },
#ifdef TARG_ST
  { OVK_BOOL,	OV_INTERNAL, TRUE, "auto_align_stack", "",
    TRUE, 0, 0, &Auto_align_stack, NULL },
#endif
  { OVK_BOOL,	OV_VISIBLE,	FALSE, "local_names",		"",
    0, 0, 0,	&PIC_Local_Names, NULL },
  { OVK_BOOL,	OV_SHY,		FALSE, "long_eh_offsets",	"long_eh", 
    0, 0, 0,	&Force_Long_EH_Range_Offsets, NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "non_volatile_GOT",	"non_v",
    0, 0, 0,	&Non_Volatile_GOT, NULL,
    "Assume GOT is non-volatile" },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "no_page_offset",	"no_p",
    0, 0, 0,	&PIC_No_Page_Offset, NULL,
    "Don't use GOT page/offset addressing" },
  { OVK_BOOL,	OV_SHY,		FALSE, "pic2",			"pi",
#ifdef TARG_ST
    0, 0, 0, &Gen_PIC_Shared, &Gen_PIC_Shared_Set,
#else
    0, 0, 0, &Gen_PIC_Shared, NULL,
#endif
    "Generate position-independent code suitable for DSOs" },
  { OVK_BOOL,	OV_SHY,		FALSE, "pic1",			NULL,
#ifdef TARG_ST
    0, 0, 0, &Gen_PIC_Call_Shared, &Gen_PIC_Call_Shared_Set,
#else
    0, 0, 0, &Gen_PIC_Call_Shared, NULL,
#endif
    "Generate code for executable programs which may call DSOs" },
  { OVK_BOOL,   OV_SHY,		FALSE, "profile_call",		"prof",
    0, 0, 0,    &Gen_Profile, NULL },
  { OVK_NAME,	OV_SHY,		FALSE, "profile_name",		"",
    0, 0, 0, &Gen_Profile_Name, NULL },
  { OVK_BOOL,	OV_VISIBLE,	FALSE, "protected_names",	"",
    0, 0, 0,	&PIC_Protected_Names, NULL },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "short_data",		"short_d",
    0, 0, 4096,	&Short_Data,	&Short_Data_Set,
    "Maximum size of data to allocate GP-relative" },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "short_literals",	"short_l",
    0, 0, 1024,	&Short_Lits,	&Short_Lits_Set,
    "Maximum size of literals to allocate GP-relative" },
  { OVK_BOOL,	OV_VISIBLE,	FALSE, "small_GOT",		"sm",
    0, 0, 0,	&Use_Small_GOT, NULL,
    "Assume GOT is smaller than 64K bytes" },
  { OVK_INT32,	OV_VISIBLE,	FALSE, "short_rodata",	        "short_r",
    8, 0, 0x7ffffff,	&Max_Srdata_Elt_Size,	&Max_Srdata_Elt_Size_Set,
    "Maximum size of const data to allocate GP-relative" },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "struct_copy_loop_size", "struct_copy_loop",
    -1, 0, 4096,	&MinStructCopyLoopSize, NULL },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "struct_copy_mem_intr_size", "struct_copy_mem",
    -1, 0, 4096,	&MinStructCopyMemIntrSize, &MinStructCopyMemIntrSize_Set },
#ifdef TARG_ST
  { OVK_INT32,  OV_INTERNAL,    FALSE, "struct_copy_parallel", NULL,
    -1, 0, 4096,        &MinStructCopyParallel, &MinStructCopyParallel_Set },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "use_memcpy", 	"",
    0, 0, 0,	&UseMemcpy, &UseMemcpy_Set },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "scalar_struct_limit", NULL,
    4, 0, 4096,	&Scalar_Struct_Limit, NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "unroll_loops", 	"",
    1, 0, 0,	&UnrollLoops, &UnrollLoops_Set },
#endif
  { OVK_INT32,	OV_VISIBLE,	FALSE, "X",			NULL,
    1, 0, 4,	&Eager_Level,	&Eager_Level_Set,
    "Exception-enable level" },
  { OVK_BOOL,   OV_VISIBLE,	FALSE, "zeroinit_in_bss",	NULL,
    0, 0, 0,    &Zeroinit_in_bss, NULL,
    "Place zero-initialized data in .bss section" },
  { OVK_BOOL,   OV_SHY,		FALSE, "strings_not_gprelative",	"strings_not_gprel",
    0, 0, 0,    &Strings_Not_Gprelative, NULL,
    "Do not put any strings in gp-relative sections" },
  { OVK_NAME,	OV_SHY,		FALSE, "emit_global_data",	"emit_global",
    0, 0, 0, &Emit_Global_Data, NULL,
    "only process the global data" },
  { OVK_NAME,	OV_SHY,		FALSE, "read_global_data",	"read_global",
    0, 0, 0, &Read_Global_Data, NULL,
    "only read the already-processed global data" },
  { OVK_BOOL,	OV_SHY,		FALSE, "force_gp_prolog",	"force_gp",
    0, 0, 0, &Force_GP_Prolog, NULL,
    "force gp_prolog to always be setup" },
  { OVK_LIST,	OV_VISIBLE,	FALSE, "registers_not_allocatable",	NULL,
    0, 0, 0, &Registers_Not_Allocatable, NULL,
    "list of registers that are reserved and not available for allocation" },
#ifdef TARG_ST
  { OVK_LIST,	OV_VISIBLE,	FALSE, "disabled_registers",	NULL,
    0, 0, 0, &Disabled_Registers, NULL,
    "list of registers that are neither available for allocation nor for manual usage in asm stmt clobber list or variable decl with register keyword" },
#endif
  { OVK_BOOL,	OV_SHY,		FALSE, "GPREL",	NULL,
    0, 0, 0, &Gen_GP_Relative, NULL,
    "do not generate GP-relative memory accesses" },
  { OVK_BOOL,	OV_INTERNAL, FALSE, "little_endian", NULL,
    0, 0, 0, &Is_Little_Endian_Set, NULL,
    "is the target little endian" },
  { OVK_BOOL,	OV_INTERNAL, FALSE, "big_endian", NULL,
    0, 0, 0, &Is_Big_Endian_Set, NULL,
    "is the target big endian" },
#ifdef TARG_ST
  { OVK_BOOL,	OV_INTERNAL, FALSE, "no_builtin_prefetch", NULL,
    0, 0, 0, &Ignore_Builtin_Prefetch, NULL,
    "Allow or ignore buit-in prefetch" },
  // FdF 20050203: This flag is used to control the optimization of prefetch. 
  // - bit 0x1 will activate the improved association between user
  // - prefetch and memory operations, and compute a stride on user
  // - prefetchs
  // - bit 0x2 will activate the scheduling of user and automatic
  // - prefetch in the code generator
   { OVK_INT32, OV_INTERNAL, FALSE, "prefetch_optimize", NULL,
     3, 0, 3,   &Prefetch_Optimize, NULL,
     "Fine tuning of prefetch optimization" },
#endif

  /***** Options moved elsewhere -- retained for compatibility: *****/
  /* See -DEBUG:div_check */
  { OVK_INT32,   OV_INTERNAL,	FALSE, "check_div",	"check_div",
    1, 0, 3, &Initial_DEBUG.div_check, &Initial_DEBUG.div_check_set },
  /* See -DEBUG:trap_uninitialized */
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "trapuv",	"",
    0, 0, 0, &Initial_DEBUG.trap_uv, &Initial_DEBUG.trap_uv_set },
  /* See -DEBUG:trapuv_right_justify */
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "trapuv_right_justify",	"trapuv_right",
    0, 0, 0, &Initial_DEBUG.trap_uv_rjustify,
	     &Initial_DEBUG.trap_uv_rjustify_set },

  /***** Options moved elsewhere -- replaced: *****/
  /* See -DEBUG:varargs_prototypes */
  { OVK_REPLACED, OV_INTERNAL,	FALSE, "varargs_prototypes",	"varargs_p",
    0, 0, 0,
    const_cast<char*>("-DEBUG:varargs_prototypes"), NULL },

  /***** Obsolete options: *****/
  { /* OVK_INT32, */
    OVK_OBSOLETE, OV_INTERNAL,	FALSE, "align_extern",		NULL,
    0, 0, 16,	NULL, NULL,
    "Assume this alignment for unknown objects" },
  { /* OVK_BOOL, */
    OVK_OBSOLETE, OV_SHY,	FALSE, "aligned",		NULL,
    0, 0, 0,	NULL, NULL,
    "Assume unknown objects are properly aligned" },
  { /* OVK_INT32, */
    OVK_OBSOLETE, OV_INTERNAL,	FALSE, "misalignment",		NULL,
    3, 0, 3,	NULL, NULL },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "iolist_reuse",	"iolist_reuse",
    100, 1, INT32_MAX,	&iolist_reuse_limit, 			NULL,
    "Maximum number of iolists which will share stack space" },

#ifdef TARG_ST
  { OVK_BOOL,   OV_SHY,		FALSE, "instrument_functions",		NULL,
    0, 0, 0,    &Instrument_Functions_Enabled, NULL },
  { OVK_BOOL,   OV_SHY,		FALSE, "instrument_functions_for_pg",		NULL,
    0, 0, 0,    &Instrument_Functions_Enabled_For_PG, NULL },
  { OVK_BOOL,   OV_SHY,		FALSE, "profile_arcs",		NULL,
    0, 0, 0,    &Profile_Arcs_Enabled, NULL },
  { OVK_BOOL,   OV_SHY,		FALSE, "test_coverage",		NULL,
    0, 0, 0,    &Test_Coverage_Enabled, NULL },
  { OVK_BOOL,   OV_SHY,		FALSE, "profile_arcs_cgir",		NULL,
    0, 0, 0,    &Profile_Arcs_Enabled_Cgir, NULL },
  { OVK_BOOL,   OV_SHY,		FALSE, "coverage_counter64",		NULL,
    0, 0, 0,    &Coverage_Counter64, NULL },
  { OVK_BOOL,   OV_SHY,		FALSE, "branch_probabilities",		NULL,
    0, 0, 0,    &Branch_Probabilities, NULL },
#endif

#ifdef TARG_ST
  // [CG]: Floating point options
  { OVK_BOOL,   OV_VISIBLE,    FALSE, "emulate_fp", "",
    0, 0, 0,    &Emulate_FloatingPoint_Ops, &Emulate_FloatingPoint_Ops_Set,
    "Enable emulation for single and double precision floating point" },

  { OVK_BOOL,   OV_VISIBLE,    FALSE, "emulate_single", "",
    0, 0, 0,    &Emulate_Single_Float_Type, &Emulate_Single_Float_Type_Set,
    "Enable emulation for single precision floating point" },

  { OVK_BOOL,   OV_VISIBLE,    FALSE, "emulate_double", "",
    0, 0, 0,    &Emulate_Double_Float_Type, &Emulate_Double_Float_Type_Set,
    "Enable emulation for single precision floating point" },

  { OVK_BOOL,   OV_VISIBLE,    FALSE, "emulate_divrem", "",
    0, 0, 0,    &Emulate_DivRem_Integer_Ops, &Emulate_DivRem_Integer_Ops_Set,
    "Enable emulation for integer division and remainder" },
#endif

#ifdef TARG_ST
  { OVK_BOOL,   OV_VISIBLE,    FALSE, "expand_builtin", "",
    0, 0, 0,    &Enable_Expand_Builtin, &Enable_Expand_Builtin_Set,
    "Enable expansion of builtin functions into specialized code" },
#endif

#ifdef TARG_ST
  { OVK_NAME,   OV_INTERNAL,    FALSE, "extension", NULL,
    0, 0, 0,    &Extension_Names, &Extension_Is_Present,
    "List of extension names to be used" },
  { OVK_INT32,   OV_INTERNAL,    FALSE, "extension_native_support", NULL,
    EXTENSION_NATIVE_SUPPORT_DEFAULT, 0, 0x3f, &Enable_Extension_Native_Support,
    &Enable_Extension_Native_Support_Set,
    "Enable support of automatic codegen for compatible extension" },

  { OVK_INT32,   OV_INTERNAL,    FALSE, "activate_extension_native_support_bits",
    NULL, 0, 0, 0xff, &Activate_Extension_Native_Support_Bits,
    &Activate_Extension_Native_Support_Bits_Set,
    "Activate support of automatic codegen for compatible extension (per bits)" },
  { OVK_INT32,   OV_INTERNAL,    FALSE, "block_extension_native_support_bits",
    NULL, 0, 0, 0xff, &Block_Extension_Native_Support_Bits,
    &Block_Extension_Native_Support_Bits_Set,
    "Block support of automatic codegen for compatible extension (per bits)" },

  { OVK_INT32,   OV_INTERNAL,    FALSE, "meta_instruction_threshold", NULL,
    INT_MAX, 0, INT_MAX,    &Meta_Instruction_Threshold,
    &Meta_Instruction_Threshold_Set,
    "Threshold on the accepatable cost for selecting a meta instruction" },
  { OVK_BOOL,   OV_INTERNAL,    FALSE, "meta_instruction_by_size", NULL,
    0, 0, 0,    &Meta_Instruction_By_Size,
    &Meta_Instruction_By_Size_Set,
    "Selects meta instruction by size" },
  { OVK_NAME,   OV_INTERNAL,    FALSE, "disabled_native_extensions", NULL,
    0, 0, 0,    &Disabled_Native_Extensions, &Disabled_Native_Extensions_Set,
    "List of extension names with disabled native support" },
#endif

  { OVK_COUNT }		/* List terminator -- must be last */
};

#ifdef BACK_END

/* Phase selection options: */
static OPTION_DESC Options_PHASE[] = {
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "lno",	  "l",	 0, 0, 0,
      &Run_lno,	NULL},
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "wopt",	  "w",	 0, 0, 0,
      &Run_wopt,	NULL},
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "preopt", "p",	 0, 0, 0,
      &Run_preopt,	NULL},
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "cg",	  "c",	 0, 0, 0,
      &Run_cg,	NULL},
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "clist",  NULL,	 0, 0, 0,
      &Run_w2c,	NULL},
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "flist",  NULL,	 0, 0, 0,
      &Run_w2f,	NULL},
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "mplist", NULL,	 0, 0, 0,
      &Run_w2fc_early,	NULL},
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "purple", "", 0, 0, 0,
      &Run_purple,	NULL},
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "ipl",    "i",	 0, 0, 0,
      &Run_ipl,	NULL},
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "prompf", NULL,	 0, 0, 0,
      &Run_prompf,	NULL},
    { OVK_NAME,	OV_INTERNAL,	FALSE, "lpath",  "",	 0, 0, 0,
      &LNO_Path,	NULL},
    { OVK_NAME,	OV_INTERNAL,	FALSE, "wpath",  "",	 0, 0, 0,
      &WOPT_Path,	NULL},
    { OVK_NAME,	OV_INTERNAL,	FALSE, "cpath",  "",	 0, 0, 0,
      &CG_Path,	NULL},
    { OVK_NAME,	OV_INTERNAL,	FALSE, "w2cpath", "", 0, 0, 0,
      &W2C_Path,	NULL},
    { OVK_NAME,	OV_INTERNAL,	FALSE, "w2fpath", "", 0, 0, 0,
      &W2F_Path,	NULL},
    { OVK_NAME,	OV_INTERNAL,	FALSE, "purpath", "", 0, 0, 0,
      &Purple_Path,	NULL},
    { OVK_NAME,	OV_INTERNAL,	FALSE, "ipath",   "", 0, 0, 0,
      &Ipl_Path,	NULL},
    { OVK_NAME,	OV_INTERNAL,	FALSE, "tpath",   "", 0, 0, 0,
      &Targ_Path,	NULL},
    { OVK_NAME,	OV_INTERNAL,	FALSE, "prompf_anl_path", "", 0, 0, 0,
      &Prompf_Anl_Path, NULL},
#ifdef TARG_ST
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "extension_check_only", "", 0, 0, 0,
      &Run_extension_check_only, NULL},
#endif
    { OVK_COUNT}
};
#elif defined(QIKKI_BE)
static OPTION_DESC Options_PHASE[] = {
    { OVK_NAME, OV_INTERNAL,	FALSE, "tpath",   "t", 0, 0, 0,
      &Targ_Path,	NULL},
    { OVK_COUNT}
};
#endif /* BACK_END */

static OPTION_DESC Options_LANG[] = {
    { OVK_NAME, OV_INTERNAL,	FALSE, "",			NULL,
      0, 0, 0,	&Language_Name,		NULL,
      "Language being compiled, from front end" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "bool",			"",
      0, 0, 0,	&CXX_Bool_On,		&CXX_Bool_Set,
      "C++: enable builtin type 'bool'" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "exceptions",		"",
      0, 0, 0,	&CXX_Exceptions_On,	&CXX_Exceptions_Set,
      "C++: enable exceptions" },
#if 0 // remove it till we have a robust design 
    { OVK_BOOL, OV_SHY,		FALSE, "alias_const",		"",
      0, 0, 0,  &CXX_Alias_Const,       &CXX_Alias_Const_Set },
#endif
    { OVK_BOOL, OV_VISIBLE,	FALSE, "recursive",		"",
      0, 0, 0,	&LANG_Recursive,	&LANG_Recursive_Set,
      "FORTRAN: program contains recursion" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "wchar_t",		"",
      0, 0, 0,	&CXX_Wchar_On,		&CXX_Wchar_Set,
      "C++: enable builtin type 'wchar_t'" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "namespaces",		"namespace",
      0, 0, 0,  &CXX_Namespaces_On,	&CXX_Namespaces_Set,
      "C++: enable namespaces" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "ansi-for-init-scope",	"",
      0, 0, 0,  &CXX_Ansi_For_Init_Scope_On,	&CXX_Ansi_For_Init_Scope_Set},
    { OVK_BOOL, OV_VISIBLE,	FALSE, "std",	"",
      0, 0, 0,  &CXX_Standard_C_Plus_Plus_On,	&CXX_Standard_C_Plus_Plus_Set},
    { OVK_BOOL, OV_SHY,		FALSE, "restrict",		"",
      0, 0, 0,	&C_Restrict_On,		&C_Restrict_Set },
    { OVK_NAME, OV_VISIBLE,	FALSE, "autorestrict",		NULL,
      0, 0, 0,	&C_Auto_Restrict,		&C_Auto_Restrict_Set,
      "Automatically set the \"restrict\" qualifier on some or all pointers" },
    { OVK_BOOL, OV_INTERNAL,	FALSE, "scalar_formal_ref", "",
      0, 0, 0,  &Scalar_Formal_Ref,     NULL },
    { OVK_BOOL, OV_INTERNAL,	FALSE, "non_scalar_formal_ref", "",
      0, 0, 0,  &Non_Scalar_Formal_Ref, NULL },
    { OVK_BOOL, OV_INTERNAL,	FALSE, "short_circuit_conditionals", "",
      0, 0, 0,  &FTN_Short_Circuit_On, &FTN_Short_Circuit_Set },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "vla",			"",
      0, 0, 0,	&C_VLA_On,		&C_VLA_Set,
      "C/C++: enable variable length arrays" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "explicit",		"",
      0, 0, 0,  &CXX_Explicit_On,	&CXX_Explicit_Set,
      "C++: enable explicit keyword" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "typename",		"",
      0, 0, 0,  &CXX_Typename_On,	&CXX_Typename_Set,
      "C++: enable typename keyword" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "mutable",		"",
      0, 0, 0,  &CXX_Mutable_On,	&CXX_Mutable_Set,
      "C++: enable mutable keyword" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "macro_expand_pragmas",		"",
      0, 0, 0,	&Macro_Expand_Pragmas_On, &Macro_Expand_Pragmas_Set,
      "C/C++: enable macro expansion in pragmas" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "packed",		"",
      0, 0, 0,  &CXX_Packed_On,	&CXX_Packed_Set,
      "C++: enable pragma pack" },
    { OVK_BOOL, OV_INTERNAL,	FALSE, "symtab_verify",		"",
      0, 0, 0,	&LANG_Symtab_Verify_On,	&LANG_Symtab_Verify_Set },
    { OVK_BOOL, OV_VISIBLE,     FALSE, "pch",          NULL,
      0, 0, 0,  &LANG_Pch, &LANG_Pch_Set,
      "Create a precompiled header for this compilation unit" },
    { OVK_NAME, OV_VISIBLE,     FALSE, "create_pch",          NULL,
      0, 0, 0,  &LANG_Create_Pch, &LANG_Create_Pch_Set,
      "Create a precompiled header file named by this option" },
    { OVK_NAME, OV_VISIBLE,      FALSE, "use_pch",            NULL,
      0, 0, 0,  &LANG_Use_Pch, &LANG_Use_Pch_Set,
      "Use the precompiled header file named by this option" },
    { OVK_NAME, OV_VISIBLE,      FALSE, "pch_dir",            NULL,
      0, 0, 0,  &LANG_Pchdir, &LANG_Pchdir_Set,
      "Create/Use from the directory named by this option" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "ansi_setjmp",		"",
      0, 0, 0,  &LANG_Ansi_Setjmp_On,	&LANG_Ansi_Setjmp_Set,
      "C/C++: enable optimization of functions with calls to setjmp" },
    { OVK_BOOL, OV_VISIBLE,	FALSE, "microsoft_extensions",	"microsoft_extension",
      0, 0, 0,  &LANG_Microsoft_Mode,	&LANG_Microsoft_Mode_Set},

    { OVK_INT32,OV_VISIBLE,	TRUE, "heap_allocation_threshold", "heap_a",
      -1, -1, INT32_MAX, &Heap_Allocation_Threshold,	NULL,
      "Size threshold for switching from stack to heap allocation" },
    { OVK_NAME, OV_VISIBLE, 	FALSE, "cxx_dialect", NULL,
      0, 0, 0,  &LANG_cxx_dialect,   &LANG_cxx_dialect_Set},
    { OVK_BOOL, OV_VISIBLE,	FALSE, "ignore_carriage_return",	"",
      0, 0, 0,  &LANG_Ignore_Carriage_Return_On, &LANG_Ignore_Carriage_Return_Set,
      "C/C++: ignore carriage returns in source" },


    { OVK_COUNT }		    /* List terminator -- must be last */
};

static OPTION_DESC Options_INTERNAL[] = {

    { OVK_BOOL, OV_INTERNAL,	FALSE, "comma_rcomma",		"",
      0, 0, 0,	&WHIRL_Comma_Rcomma_On,	&WHIRL_Comma_Rcomma_Set },
    { OVK_BOOL, OV_INTERNAL,	FALSE, "merge_types",		"",
      0, 0, 0,	&WHIRL_Merge_Types_On,	&WHIRL_Merge_Types_Set },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "mtype_a",		NULL,
      0, 0, 0,	&WHIRL_Mtype_A_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "mtype_b",		NULL,
      0, 0, 0,	&WHIRL_Mtype_B_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "mtype_bs",		NULL,
      0, 0, 0,	&WHIRL_Mtype_BS_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "return_val",		NULL,
      0, 0, 0,	&WHIRL_Return_Val_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "flatten_field",		NULL,
      0, 0, 0,	&WHIRL_Flatten_Field_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "mldid_mstid",		NULL,
      0, 0, 0,	&WHIRL_Mldid_Mstid_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "vfcall",		NULL,
      0, 0, 0,	&WHIRL_Vfcall_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "addr_passed",		NULL,
      0, 0, 0,	&WHIRL_Addr_Passed_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "addr_saved_for_passed",	NULL,
      0, 0, 0,	&WHIRL_Addr_Saved_For_Passed_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "addr_saved",		NULL,
      0, 0, 0,	&WHIRL_Addr_Saved_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "keep_cvt",	NULL,
      0, 0, 0,	&WHIRL_Keep_Cvt_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "return_info",	NULL,
      0, 0, 0,	&WHIRL_Return_Info_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "global_pragmas",	NULL,
      0, 0, 0,	&Global_Pragmas_In_Dummy_PU_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "malloc_free",	NULL,
      0, 0, 0,	&Malloc_Free_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "alloca_dealloca",	NULL,
      0, 0, 0,	&Alloca_Dealloca_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "barrier_lvalues",	NULL,
      0, 0, 0,	&Barrier_Lvalues_On, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "mask_shift_counts",	NULL,
      0, 0, 0,	&ARCH_mask_shift_counts, NULL },
    { OVK_BOOL,	OV_INTERNAL,	FALSE, "generate_nor",	NULL,
      0, 0, 0,	&ARCH_generate_nor, NULL },
    { OVK_COUNT }		    /* List terminator -- must be last */
};

OPTION_GROUP Common_Option_Groups[] = {
  { "DEBUG",	':', '=', Options_DEBUG, NULL,
    "Options to assist debugging" },
  { "INLINE",	':', '=', Options_INLINE, NULL,
    "Options to control subprogram inlining" },
  { "INTERNAL",	':', '=', Options_INTERNAL, NULL,
    "Options to control internal flags for testing" },
  { "IPA",	':', '=', Options_IPA, NULL,
    "Options to control interprocedural analysis and optimization" },
  { "LANG",	':', '=', Options_LANG, NULL,
    "Options to control source language interpretation" },
  { "LIST",	':', '=', Options_LIST, NULL,
    "Options to control the listing file" },
  { "OPT",	':', '=', Options_OPT, NULL,
    "Options to control general optimization" },
#ifdef BACK_END
  { "LNO",	':', '=', Options_LNO, NULL,
    "Options to control loop nest optimization" },
#endif /* BACK_END */
#if defined(BACK_END) || defined(QIKKI_BE)
  { "PHASE",	':', '=', Options_PHASE, NULL,
    "Options to control phase invocation and locations" },
#endif /* defined(BACK_END) || defined(QIKKI_BE) */
  { "TARG",	':', '=', Options_TARG, NULL,
    "Options to specify the target machine characteristics" },
  { "TENV",	':', '=', Options_TENV, NULL,
    "Options to set or assert target environment characteristics" },
  { "WOPT",	':', '=', Options_WOPT, NULL,
    "Options to control internal WHIRL optimization" },
  { "VHO",	':', '=', Options_VHO, NULL,
    "Options to control internal VH WHIRL optimization" },
  { "FLIST", ':', '=', Options_FLIST, NULL,
       "Options to control listing of transformed f77 source" },
  { "CLIST", ':', '=', Options_CLIST, NULL,
       "Options to control listing of transformed C source" },
  { "PURPLE", ':', '=', Options_PURPLE, NULL,
       "Options to control program region extraction process" },
  { "PROMP", ':', '=', Options_PROMP, NULL,
       "Options to control listing mp transformations" },
  { NULL }		/* List terminator -- must be last */
};

/* ====================================================================
 *
 * Miscellaneous data declarations and initialization
 *
 * ====================================================================
 */

#ifdef TARG_ST
BOOL FE_Cvtl_Opt = TRUE;                /* Keep CVTs for STOREs ? */
#endif

/* What is the model to be used for logical values in Fortran? */
BOOL Use_C_Like_Logicals = TRUE;

/* Is exception-handling enabled in C++? */
BOOL Allow_Exceptions = TRUE;

/***** Compiler	debug/trace options *****/
BOOL Tracing_Enabled = FALSE;		/* Any trace options set? */

/***** Miscellaneous optimization options *****/
/* Should idict commute operands in seeking match? */
BOOL Idict_Commutable_Match = FALSE;
BOOL Scalar_Formal_Ref = TRUE;		/* for fortran scalar formal refs */
BOOL Non_Scalar_Formal_Ref = FALSE;	/* for fortran non_scalar formal refs */

BOOL CG_mem_intrinsics = TRUE;		/* for memory intrinsic expansion */
INT32 CG_memmove_inst_count = 16;	/* for intrinsic expansion of bzero etc */
BOOL CG_memmove_inst_count_overridden = FALSE;
BOOL CG_bcopy_cannot_overlap = FALSE;	/* for intrinsic expansion of bcopy */
BOOL CG_memcpy_cannot_overlap = FALSE;	/* for intrinsic expansion of memcpy */
BOOL CG_memmove_cannot_overlap = FALSE;	/* for intrinsic expansion of memmove */
BOOL CG_memmove_nonconst = FALSE;	/* expand mem intrinsics unknown size */
BOOL CG_floating_const_in_memory = TRUE; /* keep fp constants in memory */

/***** Miscellaneous GOPT options *****/
INT32 Opt_Level = DEF_OPT_LEVEL;
INT32 OPT_unroll_times = 4;		/* but see Configure_Target() */
BOOL OPT_unroll_times_overridden = FALSE;
INT32 OPT_unroll_size = 40;		/* but see Configure_CG_Options() */
BOOL OPT_unroll_size_overridden = FALSE;
INT32 Const_Copy_TN_CNT = DEF_CONST_COPY_TN_CNT;
BOOL  Enable_BB_Splitting = TRUE;
INT32 Split_BB_Length = DEF_BBLENGTH;
BOOL Enable_SWP = FALSE;		/* but see cgdriver.c */
BOOL Enable_SWP_overridden = FALSE;

/***** What is the byte	sex of the host	and target? *****/
UINT8 Host_Byte_Sex = BIG_ENDIAN;	/* Set in config_host.c	*/
UINT8 Target_Byte_Sex =	BIG_ENDIAN;	/* Set in config_targ.c	*/
BOOL  Same_Byte_Sex = TRUE;		/* Set in config_targ.c	*/

/***** Miscellaneous code generation options *****/
BOOL Use_Base_Ptrs = TRUE;	/* Explicit ptrs to .DATA./.RDATA? */
BOOL Gen_PIC_Call_Shared = FALSE; /* CPIC */
BOOL Gen_PIC_Shared = FALSE;	/* PIC */
#ifdef TARG_ST
BOOL Gen_PIC_Call_Shared_Set = FALSE; /* CPIC */
BOOL Gen_PIC_Shared_Set = FALSE;	/* PIC */
BOOL No_Shared_Warning = FALSE;
#endif
#ifdef TARG_ST
/* [CG]: Options for symbol visibility. */
INT32 ENV_Symbol_Visibility = 0; /* Default visibility value.
				    (see STV_... in elf.h).
				    Default value is STV_DEFAULT (0).  */
char *ENV_Symbol_Visibility_String;	/* Visibility string. */
char *ENV_Symbol_Visibility_Spec_Filename; /* Visibility spec. file. */
#endif
BOOL Gen_PIC_Calls = FALSE;	/* PIC calls */
BOOL Guaranteed_Small_GOT = TRUE; /* GOT < 64kB? */
BOOL Non_Volatile_GOT = FALSE;	/* GOT entries volatile? */
BOOL PIC_Local_Names = FALSE;	/* Names local by default? */
BOOL PIC_Protected_Names = FALSE; /* Names protected by default? */
BOOL PIC_Fixed_Addresses = FALSE; /* Names fixed by default? */
BOOL PIC_No_Page_Offset = FALSE;  /* Don't use page/offset addressing? */
BOOL Force_Mem_Formals = FALSE;	/* Always force formals to memory? */
BOOL Kernel_Code = FALSE;	/* Compiling OS kernel? */
BOOL Varargs_Prototypes = TRUE;	/* Varargs have prototypes for FP? */
BOOL Gen_Profile = FALSE;	/* Generate a profile call for each user call */
char *Gen_Profile_Name = "__profile_call"; 
BOOL Call_Mcount = FALSE;	/* generate a call to mcount in pu entry */

#ifdef TARG_ST
BOOL Instrument_Functions_Enabled = FALSE; /* generate calls to instrumentation for function entries and exits. */
BOOL Instrument_Functions_Enabled_For_PG = FALSE; /* generate calls to instrumentation for profiling function entries and exits (gprof method used for stxp70). */
BOOL Profile_Arcs_Enabled = FALSE; /* Create data files for the `gcov' code-coverage utility and instrument code. */
BOOL Test_Coverage_Enabled = FALSE; /* Create data files for the `gcov' code-coverage utility and instrument code. */
BOOL Profile_Arcs_Enabled_Cgir = FALSE; /* Create data files for the `gcov' code-coverage utility and instrument code in the cgir. */
BOOL Coverage_Counter64 = FALSE; /* Use 64 bits counters instead of 32. */
BOOL Branch_Probabilities = FALSE; /* Use .gcda file as feedback. */
#endif

BOOL GP_Is_Preserved = FALSE;	/* GP is neither caller or callee-save */
BOOL Gen_GP_Relative = FALSE;   /* generate GP-relative addressing ? */

BOOL Only_32_Bit_Ops;           /* only 32-bit instructions available */
#ifndef TARG_ST
// [CG] Defined above to be used by -TENV option
BOOL Emulate_FloatingPoint_Ops; /* fp arithmetic is not supported on target */
#endif

char *Emit_Global_Data = NULL;	/* only emit global data */
char *Read_Global_Data = NULL;	/* only read global data */

char *Library_Name = NULL;      /* -TENV:io_library=xxx */
INT  target_io_library;

BOOL Meld_Schedule = FALSE;	/* Attempt to meld basic blocks	*/
BOOL Gap_Schedule = FALSE;	/* Attempt to perform gap scheduling */
BOOL Attempt_Bypass = FALSE;	/* Attempt to use bypass registers */
BOOL Isolate_Lines = FALSE;	/* Don't overlap source	lines */
BOOL Fill_Delay_Slots = FALSE;  /* Attempt to fill branch delay slots */
BOOL Enable_GDSE = FALSE;       /* Allow global dead store elimination */
#ifndef TARG_ST
/* moved this to cg_flags[h,cxx]
BOOL Enable_CG_Peephole =FALSE;	/* Enable peephole optimization in cgprep */
#endif

#ifdef BACK_END
/* back end phases options */
BE_EXPORTED BOOL Run_lno = FALSE;   /* run loop-nest optimizer */
BOOL Run_wopt = FALSE;		    /* run WHIRL global optimizer */
BOOL Run_preopt = FALSE;	    /* run WHIRL preopt optimizer */
BOOL Run_ipl = FALSE;		    /* run procedure summary phase  */
BOOL Run_cg = FALSE;		    /* run code generator */
BOOL Run_w2c = FALSE;		    /* run whirl2c */
BOOL Run_w2f = FALSE;		    /* run whirl2f */
BOOL Run_w2fc_early = FALSE;	    /* run whirl2fc after LNO auto par*/
BOOL Run_purple = FALSE;	    /* run purple code instrumenter */
BOOL Run_prompf = FALSE;	    /* run to generate prompf analysis file */
#ifdef TARG_ST
BOOL Run_extension_check_only = FALSE; /* run extension compatibility check only */
#endif
char *LNO_Path = 0;		    /* path to lno.so */
char *WOPT_Path = 0;		    /* path to wopt.so */
char *CG_Path = 0;		    /* path to cg.so */
char *Ipl_Path = 0;		    /* path to ipl.so */
char *W2C_Path = 0;		    /* path to whirl2c.so */
char *W2F_Path = 0;		    /* path to whirl2f.so */
char *Purple_Path = 0;		    /* path to purple.so */
char *Prompf_Anl_Path = 0;	    /* path to prompf_anl.so */
BE_EXPORTED WN_MAP Prompf_Id_Map = WN_MAP_UNDEFINED; 
			/* Maps WN constructs to unique identifiers */
#endif /* BACK_END */
char *Inline_Path = 0;                    /* path to inline.so */
#if defined(BACK_END) || defined(QIKKI_BE)
char *Targ_Path = 0;		    /* path to targ_info .so */
#endif /* defined(BACK_END) || defined(QIKKI_BE) */

/* ====================================================================
 *
 * Preconfigure
 *
 * Configure compiler options prior to flag processing.
 *
 * ====================================================================
 */

BE_EXPORTED void
Preconfigure (void)
{
  OPTION_GROUP *og;

  /* Perform host-specific and target-specific configuration: */
  Preconfigure_Host ();
  Preconfigure_Target ();

  /* Initialize the option groups: */
  Initialize_Option_Groups ( Common_Option_Groups );

  og = Get_Command_Line_Group ( Common_Option_Groups, "TARG" );
  Set_Option_Internal ( og, "fp_regs" /* don't admit to this one */ );
  Set_Option_Internal ( og, "mips1" /* duplicates isa=mips1 */ );
  Set_Option_Internal ( og, "mips2" /* duplicates isa=mips2 */ );
  Set_Option_Internal ( og, "mips3" /* duplicates isa=mips3 */ );
  Set_Option_Internal ( og, "mips4" /* duplicates isa=mips4 */ );
  Set_Option_Internal ( og, "mips5" /* duplicates isa=mips5 */ );
  Set_Option_Internal ( og, "mips6" /* duplicates isa=mips6 */ );

  og = Get_Command_Line_Group ( Common_Option_Groups, "TENV" );
  Set_Option_Internal ( og, "pic1" /* same as -TENV:cpic */ );
  Set_Option_Internal ( og, "pic2" /* same as -TENV:pic */ );

#ifdef BACK_END
  /* -PHASE is just for driver and internal use -- don't expose it */
  og = Get_Command_Line_Group ( Common_Option_Groups, "PHASE" );
  Set_Option_Internal ( og, NULL );

  /* -INLINE and -IPA aren't passed properly to back end, so don't
   * confuse the poor users by printing the defaults (PV 645705)
   */
  og = Get_Command_Line_Group ( Common_Option_Groups, "INLINE" );
  Set_Option_Internal ( og, NULL );
  og = Get_Command_Line_Group ( Common_Option_Groups, "IPA" );
  Set_Option_Internal ( og, NULL );

#endif /* BACK_END */
}

#ifdef TARG_ST
/* ====================================================================
 *
 * Save_Default_Options
 *
 * Save the current values for common options.
 *
 * ====================================================================
 */

BE_EXPORTED void
Save_Default_Options(void)
{
  Save_Option_Groups(Common_Option_Groups);
}
/* ====================================================================
 *
 * Reset_Default_Options
 *
 * Reset the common default options.
 *
 * ====================================================================
 */

BE_EXPORTED void
Reset_Default_Options(void)
{
  Reset_Option_Groups(Common_Option_Groups);
}

/* ====================================================================
 *
 * Apply_Opt_Level_For_Common
 *
 * Set options for optimization level .
 *
 * ====================================================================
 */

BE_EXPORTED void
Apply_Opt_Level_For_Common(UINT32 level)
{
  if (!OPT_Mul_by_cst_threshold_Set)
    OPT_Mul_by_cst_threshold = level;

}
/* ====================================================================
 *
 * Apply_Opt_Size_For_Common
 *
 * Set options for code size .
 *
 * ====================================================================
 */

BE_EXPORTED void
Apply_Opt_Size_For_Common(UINT32 level)
{
  //level = 0 means no size opt
  if (level == PU_OPTLEVEL_0 || level == PU_OPTLEVEL_UNDEF) return;
  
  FmtAssert(level == PU_OPTLEVEL_1,
	    ("Apply_Opt_Size_For_Common: only level 1 is implemented (asked was %d)",level));

  if (!CG_memmove_inst_count_overridden)
    CG_memmove_inst_count = 8;
  if (! OPT_unroll_size_overridden)
    OPT_unroll_size = 20;
    /* reduce caller+callee "size" limit for inlining */
  if (!INLINE_Max_Pu_Size_Set)
    INLINE_Max_Pu_Size=1000;
#if 0 /* not ready for this yet. */
  /* don't inline divide expansions */
  if (!OPT_Inline_Divide_Set) OPT_Inline_Divide = FALSE;
#endif
  
#if 0 //def BACK_END
  /* LNO options to be turned off for SPACE */
  LNO_Outer_Unroll = 1;
  LNO_Split_Tiles = FALSE;
#endif /* BACK_END */

  if (!WOPT_Enable_CFG_Opt_Limit_Set)
    WOPT_Enable_CFG_Opt_Limit = 5;

  if (!WOPT_Enable_While_Loop_Set)
    WOPT_Enable_While_Loop = FALSE;

  if (!UseMemcpy_Set) {
    /* In -Os, do not inline copies, unless -mno-mempcy is passed. */
    if (!MinStructCopyMemIntrSize_Set)
      MinStructCopyMemIntrSize = 7;
  }

  if (!OPT_Mul_by_cst_threshold_Set)
    OPT_Mul_by_cst_threshold = 0;

  if (!OPT_Lower_While_Do_For_Space_Set)
    OPT_Lower_While_Do_For_Space = TRUE;

  if (!OPT_Expand_Switch_For_Space_Set)
    OPT_Expand_Switch_For_Space = TRUE;

}
#endif


/* ====================================================================
 *
 * Configure_Platform
 *
 * Process a platform name, setting Platform and Processor_Name, the
 * latter to be processed by Configure_Target.  The platform name may
 * come from either -OPT:Ofast=name or -TARG:platform=name.
 *
 * ====================================================================
 */

static void
Configure_Platform ( char *platform_name )
{
  PLATFORM_OPTIONS *popts;

  /* If we've already set the platform, and the new name is empty,
   * don't re-default it:
   */
  if ( Platform != IP0
    && ( platform_name == NULL || *platform_name == 0 ) )
  {
    return;
  }

  /* Get platform and its associated options: */
  popts = Get_Platform_Options ( platform_name );
  Platform = POPTS_id(popts);
  
  /* Set the per-IP options: */
  if ( Processor_Name == NULL ) {
    Processor_Name = POPTS_pname(popts);
  }
}

/* ====================================================================
 *
 * Configure_Ofast
 *
 * Configure option defaults which depend on -Ofast (the baseline SPEC
 * optimizaiton option).  These currently include:
 *
 *   -OPT:Olimit=0 -- no limit on optimization region size.
 *   -OPT:roundoff=3 -- do any mathematically valid rearrangement.
 *   -OPT:div_split=ON -- allow splitting a/b => a*recip(b).
 *   -OPT:speculative_null_ptr_deref=ON -- allow speculation past the null
 *				   	   ptr test. assumes page zero as 
 *					   readable.
 *   -OPT:alias=typed -- pointers to different types don't alias.
 *   -WOPT:copy_ops=OFF -- don't copy-propagate operations that the IVR
 *		can't handle (OFF by default now, but just in case...).
 *   -WOPT:estr_fb_injury=ON -- SSAPRE strength reduction uses
 *                              feedback frequency rather than loop
 *                              nesting to decide whether each IV
 *                              update should be viewed as injury or
 *                              kill.
 *
 * This must be done before abi/isa/processor configuration.
 *
 * ====================================================================
 */

static void
Configure_Ofast ( void )
{
  /* We assume that the driver has defaulted Opt_Level properly. */
  /* First set the options that are common to all targets: */
  if ( ! Olimit_Set ) {
    Olimit = 0;
    Olimit_Set = TRUE;
  }
  if ( ! Roundoff_Set ) {
    Roundoff_Level = ROUNDOFF_ANY;
    Roundoff_Set = TRUE;
  }
  if ( ! Div_Split_Set ) {
    Div_Split_Allowed = TRUE;
    Div_Split_Set = TRUE;
  }
/* #645549: There exists an OS bug which gets triggered by NULL ptr
   speculation. Disable NULL ptr speculation for Ofast (base flags).
   They will however continue to be turned ON for SPEC peak flags.

  if ( ! GCM_Eager_Null_Ptr_Deref_Set ) {
    GCM_Eager_Null_Ptr_Deref = TRUE;
    GCM_Eager_Null_Ptr_Deref_Set = TRUE;
  }
*/
  if ( ! Alias_Pointer_Types_Set ) {
    Alias_Pointer_Types = TRUE;
    Alias_Pointer_Types_Set = TRUE;
  }
  if ( ! WOPT_Enable_Copy_Prop_Bad_Ops_Set ) {
    WOPT_Enable_Copy_Prop_Bad_Ops = FALSE;
    WOPT_Enable_Copy_Prop_Bad_Ops_Set = TRUE;
  }
  if ( ! WOPT_Enable_Estr_FB_Injury_Set ) {
    WOPT_Enable_Estr_FB_Injury = TRUE;
    WOPT_Enable_Estr_FB_Injury_Set = TRUE;
  }

  /* Get platform and its associated options: */
  Configure_Platform ( Ofast );
}

/* ====================================================================
 *
 * Configure
 *
 * Configure compiler options based on flag values.
 *
 * ====================================================================
 */

#ifdef KEEP_WHIRLSTATS
/* defined in wn.c */
extern void whirlstats();
#endif


extern BOOL IR_set_dump_order (BOOL prefix); /* in ir_reader.c */

BE_EXPORTED void
Configure (void)
{
  static BOOL dev_warn_toggled = FALSE;

#if !defined(SGI_FRONT_END_CPP)
  /* See if trees should be dumped in prefix order */
  if (Get_Trace(TKIND_INFO, TINFO_PREFIXDUMP)) {
     IR_set_dump_order(TRUE);
  }  
#endif

  if ( ! dev_warn_toggled ) {
    if ( Get_Trace( TP_MISC, 0x40 ) ) {
      dev_warn_toggled = TRUE;
      DevWarn_Toggle();
    }
  }

#ifdef KEEP_WHIRLSTATS
  atexit(whirlstats);
#endif

  /* Configure the alias options first so the list is processed and
   * we can tell for -OPT:Ofast below what overrides have occurred:
   */
  Configure_Alias_Options ( Alias_Option );

  /* Check the -TARG:platform option (subordinate to Ofast): */
  if ( Platform_Name != NULL && *Platform_Name != 0 ) {
    Configure_Platform ( Platform_Name );
  }

  /* First, if -OPT:Ofast (a.k.a. SPEC) is set, configure defaults: */
  if ( Ofast != NULL ) {
    Configure_Ofast ();
  }


  /* Perform host-specific and target-specific configuration: */
  Configure_Host ();

#ifdef TARG_ST
  /* 
   * Set the target endianness. It could have been specified as
   * a -TENV:little_endian or -TENV:big_endian  option
   *
   * NOTE: this are always set by the driver anyway.
   *
   */
  if (Is_Little_Endian_Set) {
    Target_Byte_Sex = LITTLE_ENDIAN;
  }
  else if (Is_Big_Endian_Set) {
    Target_Byte_Sex = BIG_ENDIAN;
  } // else leave it as currently defined (by the front-end).

  Same_Byte_Sex = ( Target_Byte_Sex == Host_Byte_Sex );

#endif

  Configure_Target ();

  /* What size GOT to use?  Configure_Target sets it to small for
   * 32-bit pointers, large for 64-bit pointers.  Override it here
   * if TENV was used to specify it:
   */
  if ( Use_Large_GOT && Use_Small_GOT ) {
    /* Make up your mind! */
    ErrMsg ( EC_GOT_Size, Guaranteed_Small_GOT ? "small" : "large" );
  } else if ( Use_Large_GOT ) {
    Guaranteed_Small_GOT = FALSE;
  } else if ( Use_Small_GOT ) {
    Guaranteed_Small_GOT = TRUE;
  }

  if (Emit_Global_Data && Read_Global_Data) {
    /* Make up your mind! */
    FmtAssert (FALSE, ("can't specify options to both emit and read global data"));
  }
  else if (Emit_Global_Data) Global_File_Name = Emit_Global_Data;
  else if (Read_Global_Data) Global_File_Name = Read_Global_Data;

  /* Configure the treatment of short literals and data.  We give
   * precedence to the -TENV:short_lits=nnn:short_data=mmm options,
   * but -Gn can override either of them if not specified:
   */
  if ( ! Short_Lits_Set ) {
    Short_Lits = Max_Sdata_Elt_Size;
  }
  if ( ! Short_Data_Set ) {
    Short_Data = Max_Sdata_Elt_Size;
  }

  /* Turn on -OPT:Reorg_Common by default at -O3: */
  if ( ! OPT_Reorg_Common_Set && Opt_Level > 2 ) {
    OPT_Reorg_Common = TRUE;
  }

  if (Force_GP_Prolog) Force_Jalr = TRUE;
}

/* ====================================================================
 *
 * Configure_Source
 *
 * Configure compiler options for each source file.
 *
 * Note	that we	set the	various	optimization and code generation
 * options at this level to ultimately allow per-source	respecification
 * with	pragmas.
 *
 * ====================================================================
 */

BE_EXPORTED void
Configure_Source ( char	*filename )
  /**  NOTE: filename CAN BE NULL */
{
  /* Identify the source language: */
  if ( Language_Name != NULL ) {
    if ( strcasecmp ( Language_Name, "KR_C" ) == 0 ) {
      Language = LANG_KR_C;
    } else if ( strcasecmp ( Language_Name, "ANSI_C" ) == 0 ) {
      Language = LANG_ANSI_C;
    } else if ( strcasecmp ( Language_Name, "CPLUS" ) == 0 ) {
      Language = LANG_CPLUS;
    } else if ( strcasecmp ( Language_Name, "DELTA" ) == 0 ) {
      Language = LANG_DELTA;
    } else if ( strcasecmp ( Language_Name, "F77" ) == 0 ) {
      Language = LANG_F77;
    } else if ( strcasecmp ( Language_Name, "F90" ) == 0 ) {
      Language = LANG_F90;
    }
  }

  /* Configure the -DEBUG and -LIST groups: */
  DEBUG_Configure ();
  LIST_Configure ();

  /* Determine whether to use the CRAY or MIPS IO library */
  if (Library_Name != NULL) {
    if (strcasecmp ( Library_Name,"cray") == 0 ) {
       target_io_library = IOLIB_CRAY;
    } else if (strcasecmp ( Library_Name,"mips") == 0 ) {
       target_io_library = IOLIB_MIPS;
    }
  } else {
     /* For F90, use the CRAY libraries by default */
     if (Language == LANG_F90) {
	target_io_library = IOLIB_CRAY;
     /* For F77, use the MIPS libraries by default */
     } else {
	target_io_library = IOLIB_MIPS;
     }
  }

#ifdef BACK_END

  /* If we're invoking CITE, turn on whirl2c/f: */
  if ( List_Cite ) {
    if ( Language == LANG_F77 || Language == LANG_F90 ) {
      Run_w2f = TRUE;
    } else if ( Language == LANG_KR_C || Language == LANG_ANSI_C
	     || Language == LANG_CPLUS || Language == LANG_DELTA )
    {
      Run_w2c = TRUE;
    }
  }

  /* If we're invoking CITE, turn on whirl2c/f: */
  if (Run_w2fc_early) {
    if ( Language == LANG_F77 || Language == LANG_F90 ) {
      Run_w2f = TRUE;
    } else if ( Language == LANG_KR_C || Language == LANG_ANSI_C
	     || Language == LANG_CPLUS || Language == LANG_DELTA )
    {
      Run_w2c = TRUE;
    }
  }
#endif /* BACK_END */

  if ( Use_Large_GOT )	Guaranteed_Small_GOT = FALSE;

  /* if we get both TENV:CPIC and TENV:PIC, use only TENV:CPIC */
  if (Gen_PIC_Call_Shared && Gen_PIC_Shared) Gen_PIC_Shared = FALSE;

#ifdef TARG_ST
  /* Interpret -TENV:visibility=<string> option. */
  if (ENV_Symbol_Visibility_String != NULL) {
    if (strcmp(ENV_Symbol_Visibility_String, "default") == 0)
      ENV_Symbol_Visibility = 0; /* STV_DEFAULT. */
    else if (strcmp(ENV_Symbol_Visibility_String, "internal") == 0)
      ENV_Symbol_Visibility = 1; /* STV_INTERNAL. */
    else if (strcmp(ENV_Symbol_Visibility_String, "hidden") == 0)
      ENV_Symbol_Visibility = 2; /* STV_HIDDEN. */
    else if (strcmp(ENV_Symbol_Visibility_String, "protected") == 0)
      ENV_Symbol_Visibility = 3; /* STV_PROTECTED. */
    else {
      ErrMsg(EC_Misc_String, "visibility argument", ENV_Symbol_Visibility_String);
    }
  }
#endif

  /* Select optimization options: */

  /* Are we skipping any PUs for optimization? */
  Optimization_Skip_List = Build_Skiplist ( Opt_Skip );
  /* Are we skipping any regions for optimization? */
  Region_Skip_List = Build_Skiplist ( Region_Skip );

  /* F90 is a recursive language, so this needs to be set */
  if (!LANG_Recursive_Set && Language == LANG_F90)
     LANG_Recursive = TRUE;

  /* Since there seems to be little compile time reason not to be aggressive, 
   * make the folder aggressive by default
   */
  if ( ! Cfold_Aggr_Set )
    Enable_Cfold_Aggressive = TRUE;

  if (!Enable_CVT_Opt_Set)
    Enable_CVT_Opt = ( Opt_Level > 0);

  CSE_Elim_Enabled = Opt_Level > 0;

  /* Perform the %call16 -> %got_disp relocation change? */
  if ( ! Enable_GOT_Call_overridden )
    Enable_GOT_Call_Conversion = Opt_Level > 2;

  /* Force formal parameters to memory? */
  Force_Mem_Formals = ( Opt_Level < 1 );

  // TB: now done in a specific function apply_opt_space(level).
  // I leave here the code to fix some eventual issues
  /* Optimize for space */
//   if ( OPT_Space ) {

//     /* TODO:  Other space optimizations to force? */
//     if (!CG_memmove_inst_count_overridden)
//       CG_memmove_inst_count = 8;
//     if (! OPT_unroll_size_overridden)
//       OPT_unroll_size = 20;
//     /* reduce caller+callee "size" limit for inlining */
//     INLINE_Max_Pu_Size=1000;
// #if 0 /* not ready for this yet. */
//     /* don't inline divide expansions */
//     if (!OPT_Inline_Divide_Set) OPT_Inline_Divide = FALSE;
// #endif

// #ifdef BACK_END
//     /* LNO options to be turned off for SPACE */
//     LNO_Outer_Unroll = 1;
//     LNO_Split_Tiles = FALSE;
// #endif /* BACK_END */

// #ifdef TARG_ST
//     // FdF 06/10/2004: Reduce the code duplication in
//     // CFG_Transformation (be/opt/opt_cfg_trans.cxx)
//     if (!WOPT_Enable_CFG_Opt_Limit_Set)
//       WOPT_Enable_CFG_Opt_Limit = 5;

//     // FdF 20050411: Reduce loop body duplication
//     WOPT_Enable_While_Loop = FALSE;
// #endif
// #if 0 
//   } else {
//     //TB: Set default value
//     /* TODO:  Other space optimizations to force? */
//     if (!CG_memmove_inst_count_overridden)
//       CG_memmove_inst_count = 16;
//     if (! OPT_unroll_size_overridden)
//      OPT_unroll_size = 64;
//     /* reduce caller+callee "size" limit for inlining */
//     INLINE_Max_Pu_Size=DEFAULT_INLINE_Max_Pu_Size;
// +    // FdF 06/10/2004: Reduce the code duplication in
//     // CFG_Transformation (be/opt/opt_cfg_trans.cxx)
//     if (!WOPT_Enable_CFG_Opt_Limit_Set)
//       WOPT_Enable_CFG_Opt_Limit = 10;

//     // FdF 20050411: Reduce loop body duplication
//     WOPT_Enable_While_Loop = TRUE;
// #endif
//   }

#ifdef TARG_ST
  /* -mmemcpy forces to call mempcy, -mno-memcopy forces to always
  inline copies (the default). */

  if (UseMemcpy_Set) {
    if (UseMemcpy) {
      if (!MinStructCopyMemIntrSize_Set)
	MinStructCopyMemIntrSize = 7;
    }
    else
      MinStructCopyMemIntrSize = 0;
  }
//   else if (OPT_Space) {
//     /* In -Os, do not inline copies, unless -mno-mempcy is passed. */
//     if (!MinStructCopyMemIntrSize)
//       MinStructCopyMemIntrSize = 7;
//   } 
  // else
  //  MinStructCopyMemIntrSize = 0;
  if (Opt_Level > 2) {
    MinStructCopyLoopSize = 32;
    if(!MinStructCopyParallel_Set) MinStructCopyParallel = 3;
  }    
#endif


  /* symbolic debug stuff */
  Symbolic_Debug_Mode = SDM_NONE;
  if (Debug_Level > 0 && Debug_Level <= 2)
    Symbolic_Debug_Mode |= SDM_SEQLINE;
  if (Debug_Level > 0) {
    Symbolic_Debug_Mode |= SDM_SYMBOL;
    Symbolic_Debug_Mode |= SDM_LINE;
  }


  /* Disabling splitting of long basic blocks: */
  Enable_BB_Splitting = ! Get_Trace ( TP_FLOWOPT, 0x080 );

  if (Opt_Level > 2 && ! Olimit_Set)
	Olimit = DEFAULT_O3_OLIMIT;
  if (Olimit == 0) {
	/* 0 Olimit means no limit or infinite limit */
	Olimit = MAX_OLIMIT;
  }
  else if (Olimit < 10) {
	/* olimit too small to work properly */
	DevWarn("Olimit < 10 is too small; resetting to Olimit=10");
	Olimit = 10;
  }
  if (Opt_Level == 0 && ! Olimit_opt_Set)
	Olimit_opt = FALSE;

#if !defined(SGI_FRONT_END_CPP) && !defined(QIKKI_BE)
  /* -OPT:rail or -OPT:rbi implies -OPT:compile_by_region
   * unless -OPT:compile_by_region=no is specified.
   */
  if (Regions_Around_Inner_Loops || Region_Boundary_Info)
    Set_PU_has_region (Get_Current_PU ());
#endif /* !defined(SGI_FRONT_END_CPP) && !defined(QIKKI_BE) */

#ifndef TARG_ST
  /* Enable IEEE_arithmetic options */
  if (Opt_Level > 2 && !IEEE_Arith_Set) {
     IEEE_Arithmetic = IEEE_INEXACT;
  }
#endif

  Recip_Allowed = ARCH_recip_is_exact;
#ifdef TARG_ST200
  Rsqrt_Allowed = ARCH_rsqrt_is_exact;
#endif
  /* IEEE arithmetic options: */
  if ( IEEE_Arithmetic > IEEE_ACCURATE ) {
    /* Minor roundoff differences for inexact results: */
    if ( ! Recip_Set )
      Recip_Allowed = IEEE_Arithmetic >= IEEE_INEXACT;
    if ( ! Rsqrt_Set )
      Rsqrt_Allowed = IEEE_Arithmetic >= IEEE_INEXACT;
    /* Potential non-IEEE results for exact operations: */
    if ( ! Div_Split_Set )
      Div_Split_Allowed = IEEE_Arithmetic >= IEEE_ANY;
  }

  /* Constant folding options: */
  if ( ! Roundoff_Set && Opt_Level > 2 ) {
    Roundoff_Level = ROUNDOFF_ASSOC;
  }

#ifdef TARG_ST
  if ( ! No_Math_Errno_Set )
      No_Math_Errno = IEEE_Arithmetic >= IEEE_ACCURATE;
  if ( ! Finite_Math_Set ){
      Finite_Math = IEEE_Arithmetic >= IEEE_INEXACT;
      Force_IEEE_Comparisons = !(Finite_Math);
  }
  if ( ! No_Rounding_Set )
      No_Rounding = IEEE_Arithmetic >= IEEE_ACCURATE;
  if ( ! No_Trapping_Set )
      No_Trapping = IEEE_Arithmetic >= IEEE_ACCURATE;
  if ( ! Unsafe_Math_Set )
      Unsafe_Math = IEEE_Arithmetic >= IEEE_ANY;
  if ( ! Fused_FP_Set ) {
      Fused_FP = IEEE_Arithmetic >= IEEE_ACCURATE;
  }
  Rsqrt_Allowed &= Fused_FP;
  if ( ! Fused_Madd_Set )
      Fused_Madd = IEEE_Arithmetic >= IEEE_ACCURATE;
  if ( ! No_Denormals_Set )
      No_Denormals = IEEE_Arithmetic >= IEEE_ACCURATE;
  if ( ! Reassoc_Set )
       Reassoc_Level = (REASSOC)(IEEE_Arithmetic - (IEEE_Arithmetic >= IEEE_ACCURATE));
  if ( Reassoc_Set )
      Roundoff_Level = (ROUNDOFF) Reassoc_Level;
#endif

  // [HK] make a clear separation between FP and non-FP related transformations
  if ( Roundoff_Level > ROUNDOFF_NONE ) {

    /* The following allows folding of intrinsics with constant arguments: */
    if ( ! Cfold_Intrinsics_Set )
      Enable_Cfold_Intrinsics = Roundoff_Level >= ROUNDOFF_SIMPLE;

    /* reassociate in the lowerer to find MADD oportunities */
    if ( ! Enable_NaryExpr_Set )
      Enable_NaryExpr = Roundoff_Level >= ROUNDOFF_ASSOC;


    if (!Fast_Complex_Set)
      Fast_Complex_Allowed = Roundoff_Level >= ROUNDOFF_ANY;
    if (!Fast_NINT_Set)
      Fast_NINT_Allowed = Roundoff_Level >= ROUNDOFF_ANY;
    if (!Fast_trunc_Set)
      Fast_trunc_Allowed = Roundoff_Level >= ROUNDOFF_SIMPLE;

// [HK] 20070531 fix for codex bug #28633
// what's the point of activating the CIS if the roundoff level is sufficient ?
#ifndef TARG_ST
    if ( ! CIS_Set )
      CIS_Allowed |= Roundoff_Level >= ROUNDOFF_SIMPLE;
#endif

  }

  if ( Reassoc_Level > REASSOC_NONE ) {

    /* The following allow minor roundoff differences: */
    if ( ! Fast_Exp_Set )
      Fast_Exp_Allowed = Reassoc_Level >= REASSOC_SIMPLE;
    /* The following allows arbitrary reassociation: */
    if ( ! Cfold_Reassoc_Set )
      Enable_Cfold_Reassociate = Reassoc_Level >= REASSOC_ASSOC;
  }


#if 0
  /* Set the relational operator folding in simplifier based on the optimizer
     setting of Allow_wrap_around_opt */
  if (!Simp_Unsafe_Relops_Set && Allow_wrap_around_opt_Set) {
     Simp_Unsafe_Relops = Allow_wrap_around_opt;
  }
  if (!Allow_wrap_around_opt_Set && Simp_Unsafe_Relops_Set ) {
     Allow_wrap_around_opt = Simp_Unsafe_Relops;
  }
#endif
#ifndef TARG_ST200
  if (!Simp_Unsafe_Relops_Set && Opt_Level > 2) {
     Simp_Unsafe_Relops = TRUE;
  }
#endif
  
  Enable_GDSE	 = ((Opt_Level > 1) &&
		    (!Get_Trace(TP_GLOBOPT, 4096))
		    );
  /* The lowerer will do a simple treeheight reduction for
   * binary commutative ops
   */
  if (!OPT_Lower_Treeheight_Set)
     OPT_Lower_Treeheight = (Opt_Level > 1);

  /* Perform host-specific and target-specific configuration: */

  /**  NOTE: filename CAN BE NULL */
  
  Configure_Source_Host ( filename );
  Configure_Source_Target ( filename );

  /* Set eagerness-level-based information.  This must come after the
   * call to Configure_Source_Target, since that routine sets the
   * FP exception enable masks.
   */
  if ( ! Eager_Level_Set && Opt_Level > 2 ) {
    Eager_Level = EAGER_ARITH;
  }
  if ( Eager_Level >= EAGER_ARITH ) {
    FP_Exception_Enable_Max &= ~(FPX_I|FPX_U|FPX_O|FPX_V);
  }
  if ( Eager_Level >= EAGER_DIVIDE ) {
    FP_Exception_Enable_Max &= ~FPX_Z;
  }
  if ( Eager_Level >= EAGER_MEMORY ) {
    Force_Memory_Dismiss = TRUE;
  }

#ifdef BACK_END
  /* Configure the -LNO group: */
  LNO_Configure ();
#endif /* BACK_END */

  /* Trace options: */
  if ( Get_Trace ( TP_MISC, 128 ) ) {
    Trace_Option_Groups ( TFile, Common_Option_Groups, TRUE );
  } else if ( Get_Trace ( TP_MISC, 32 ) ) {
    Trace_Option_Groups ( TFile, Common_Option_Groups, FALSE );
  }

#ifdef TARG_ST
  /* [CG]: Enable expansion of builtins at Opt_Level > 0. */
  if (Opt_Level > 0 && !Enable_Expand_Builtin_Set) 
    Enable_Expand_Builtin = TRUE;
#endif

#ifdef TARG_ST
  /* Unless C++ language , exceptions must be off by default. */
  if (Language != LANG_CPLUS && !CXX_Exceptions_Set) {
    CXX_Exceptions_On = FALSE;
  }
#endif
}

/* ====================================================================
 *
 * Configure_Alias_Options
 *
 * Configure the options related to alias analysis.
 *
 * ====================================================================
 */

BE_EXPORTED void
Configure_Alias_Options( OPTION_LIST *olist )
{
  OPTION_LIST *ol;
  for (ol = olist; ol != NULL; ol = OLIST_next(ol)) {
    char *val = OLIST_val(ol);
    INT len = strlen (val);
    if (strncasecmp( val, "any", len) == 0) {
      Alias_Pointer_Parms = TRUE;	/* observed by Fortran programs */
      Alias_Pointer_Cray = FALSE;	/* observed by Fortran programs */
      Alias_Pointer_Types = TRUE;	/* observed by C and C++ programs */
      Alias_Not_In_Union  = TRUE;	/* observed by C++ programs only */
      Alias_Pointer_Strongly_Typed = FALSE;	/* observed by C and C++ programs */
      Alias_Pointer_Types_Set = TRUE;
      Alias_Not_In_Union_Set  = TRUE;	/* observed by C++ programs only */
      Alias_Pointer_Named_Data = FALSE;	/* observed by C and C++ programs */
      Alias_Pointer_Restricted = FALSE;	/* observed by C and C++ programs */
      Alias_Pointer_Disjoint   = FALSE;
    } else if (strncasecmp( val, "parm", len) == 0) {
      Alias_Pointer_Parms = TRUE;
    } else if (strncasecmp( val, "typed", len) == 0) {
      Alias_Pointer_Types = TRUE;
      Alias_Pointer_Types_Set = TRUE;
    } else if (strncasecmp( val, "unnamed", len) == 0) {
      Alias_Pointer_Named_Data = TRUE;
    } else if (strncasecmp( val, "nounion",len) == 0) {
      Alias_Not_In_Union  = TRUE;	/* observed by C++ programs only */
      Alias_Not_In_Union_Set  = TRUE;	/* observed by C++ programs only */
    } else if (strncasecmp( val, "restricted", len) == 0) {
      Alias_Pointer_Restricted = TRUE;
      Alias_Pointer_Named_Data = TRUE;
    } else if (strncasecmp( val, "disjoint", len) == 0) {
      Alias_Pointer_Disjoint = TRUE;
      Alias_Pointer_Restricted = TRUE;
      Alias_Pointer_Named_Data = TRUE;
    } else if (strncasecmp( val, "cray_pointer", len) == 0) {
      Alias_Pointer_Cray = TRUE;
    } else if (strncasecmp( val, "strongly_typed", len) == 0) {
      Alias_Pointer_Strongly_Typed = TRUE;
    } else if (strncasecmp( val, "no_parm", len) == 0) {
      Alias_Pointer_Parms = FALSE;
    } else if (strncasecmp( val, "no_typed", len) == 0) {
      Alias_Pointer_Types = FALSE;
      Alias_Pointer_Types_Set = TRUE;
    } else if (strncasecmp( val, "no_unnamed", len) == 0) {
      Alias_Pointer_Named_Data = FALSE;
    } else if (strncasecmp( val, "no_restricted", len) == 0) {
      Alias_Pointer_Restricted = FALSE;
      Alias_Pointer_Named_Data = FALSE;
    } else if (strncasecmp( val, "no_disjoint", len) == 0) {
      Alias_Pointer_Disjoint = FALSE;
      Alias_Pointer_Named_Data = FALSE;
    } else if (strncasecmp( val, "no_cray_pointer", len) == 0) {
      Alias_Pointer_Cray = FALSE;
    } else if (strncasecmp( val, "no_strongly_typed", len) == 0) {
      Alias_Pointer_Strongly_Typed = FALSE;
    } else if (strncasecmp( val, "cckr_default", len) == 0) {
      Alias_Pointer_Cckr = TRUE;
    } else if (strncasecmp( val, "common_scalar", len) == 0) {
      Alias_Common_Scalar = TRUE;
    } else if (strncasecmp( val, "no_common_scalar", len) == 0) {
      Alias_Common_Scalar = FALSE;
    } else if (strncasecmp( val, "no_f90_pointer_alias", len) == 0) {
      Alias_F90_Pointer_Unaliased = TRUE;
    } else if (strncasecmp( val, "f90_pointer_alias", len) == 0) {
      Alias_F90_Pointer_Unaliased = FALSE;
    } else {
      ErrMsg ( EC_Inv_OPT, "alias", val );
    }
  }

  /* If we didn't explicitly set alias=typed, and this is a -cckr
   * compilation, turn off Alias_Pointer_Types:
   */
  if ( ! Alias_Pointer_Types_Set && Alias_Pointer_Cckr ) {
    Alias_Pointer_Types = FALSE;
  }
}

/* ====================================================================
 *
 * SKIPLIST
 *
 * Support for lists of PU numbers to skip (e.g. for optimization)
 * based on options in a command line group.  A typedef for SKIPLIST
 * is defined in config.h, but the type is opaque outside; for now,
 * Build_Skiplist and Query_Skiplist are the only visible interface.
 *
 * This interface is sufficiently generic that it could go into flags.*
 * in common/util.  It isn't there because that could end up breaking
 * the principle that flags.h should not be widely included (i.e. other
 * than by command line processing).
 *
 * ====================================================================
 */

typedef enum {
  SK_NONE,	/* End of list */
  SK_AFTER,	/* Values after this one */
  SK_BEFORE,	/* Values before this one */
  SK_EQUAL	/* Just this one */
} SKIPKIND;

struct skiplist {
  mINT32 size;	/* Number of elements */
  mINT8 *kind;	/* Array of kinds */
  mINT32 *val;	/* Array of values */
};

#define SKIPLIST_size(sl)		((sl)->size)
#define SKIPLIST_kind_vec(sl)		((sl)->kind)
#define SKIPLIST_kind(sl,i)		((SKIPKIND)((sl)->kind[i]))
#define Set_SKIPLIST_kind(sl,i,v)	(((sl)->kind[i]) = (mINT8)(v))
#define SKIPLIST_val_vec(sl)		((sl)->val)
#define SKIPLIST_val(sl,i)		((sl)->val[i])

/* ====================================================================
 *
 * Print_Skiplist
 *
 * Print a skiplist.  
 *
 * ====================================================================
 */

static void
Print_Skiplist ( FILE *tf, SKIPLIST *skip, char *lab )
{
  INT32 i;

  if ( skip == NULL ) {
    fprintf ( tf, "SKIPLIST %s empty\n", lab );
    return;
  }
  fprintf ( tf, "SKIPLIST %s:\n", lab );

  for ( i = 0; SKIPLIST_kind(skip,i) != SK_NONE; i++ ) {
    switch ( SKIPLIST_kind(skip,i) ) {
      case SK_EQUAL:
	fprintf ( tf, "  equal %d\n", SKIPLIST_val(skip,i) );
	break;
      case SK_AFTER:
	fprintf ( tf, "  after %d\n", SKIPLIST_val(skip,i) );
	break;
      case SK_BEFORE:
	fprintf ( tf, "  before %d\n", SKIPLIST_val(skip,i) );
	break;
    }
  }
  fprintf ( tf, "SKIPLIST %s end\n\n", lab );
}

/* ====================================================================
 *
 * Build_Skiplist
 *
 * Build a skiplist from a group option list.  For now, we assume that
 * the only choices are skip_a* (after) skip_b* (before), and skip_e*
 * (equal).  See Query_Skiplist below for the list semantics.
 *
 * Note that we interpret skip_equal=1,3-5,7-10,12,35-39 as you might
 * hope.
 *
 * WARNING:  This routine does no error checking.  This option is for
 * internal use, and if the syntax is wrong, strange (non-fatal) things
 * may happen (typically ignoring the rest of the option).
 *
 * ====================================================================
 */

SKIPLIST *
Build_Skiplist ( OPTION_LIST *olist )
{
  INT32 count = 0;
  OPTION_LIST *ol;
  SKIPLIST *sl;
  BOOL list_found = FALSE;
  char *p;

  /* Count the elements: */
  if ( olist == NULL ) return NULL;
  for ( ol = olist; ol != NULL; ol = OLIST_next(ol) ) {

    /* At least one entry: */
    ++count;
    
    /* Check for commas and ranges: */
    p = OLIST_val(ol);
    while ( *p != ':' && *p != 0 ) {
      if ( *p == ',' || *p == '-' ) {
	++count;
	list_found = TRUE;
      }
      ++p;
    }
  }

  /* Allocate the skiplist: */
  sl = (SKIPLIST *) malloc ( sizeof(SKIPLIST) );
  SKIPLIST_size(sl) = count+1;
  SKIPLIST_kind_vec(sl) = (mINT8 *) calloc ( sizeof(mINT8), count+1 );
  SKIPLIST_val_vec(sl) = (mINT32 *) calloc ( sizeof(mINT32), count+1 );

  /* Fill the skiplist: */
  for ( count = 0, ol = olist;
	ol != NULL;
	++count, ol = OLIST_next(ol) )
  {
    if ( !strncmp ( "skip_a", OLIST_opt(ol), 6 ) ||
	 !strncmp ( "region_skip_a", OLIST_opt(ol), 13 ) ) {
      Set_SKIPLIST_kind ( sl, count, SK_AFTER );
    } else if ( !strncmp ( "skip_b", OLIST_opt(ol), 6 ) ||
	        !strncmp ( "region_skip_b", OLIST_opt(ol), 13 ) ) {
      Set_SKIPLIST_kind ( sl, count, SK_BEFORE );
    } else {
      Set_SKIPLIST_kind ( sl, count, SK_EQUAL );
    }
    SKIPLIST_val(sl,count) = atoi ( OLIST_val(ol) );

    /* If this is skip_equal, look for a list... */
    if ( list_found && SKIPLIST_kind(sl,count) == SK_EQUAL ) {
      p = OLIST_val(ol);
      while ( *p >= '0' && *p <= '9' ) ++p;
      if ( *p == '-' ) {
	Set_SKIPLIST_kind ( sl, count, SK_AFTER );
	--SKIPLIST_val(sl,count);
	++p;
	++count;
	Set_SKIPLIST_kind ( sl, count, SK_BEFORE );
	SKIPLIST_val(sl,count) = 1 + atoi ( p );
	while ( *p >= '0' && *p <= '9' ) ++p;
      }
      while ( *p++ == ',' ) {
	++count;
	Set_SKIPLIST_kind ( sl, count, SK_EQUAL );
	SKIPLIST_val(sl,count) = atoi ( p );
	while ( *p >= '0' && *p <= '9' ) ++p;
	if ( *p == '-' ) {
	  Set_SKIPLIST_kind ( sl, count, SK_AFTER );
	  --SKIPLIST_val(sl,count);
	  ++p;
	  ++count;
	  Set_SKIPLIST_kind ( sl, count, SK_BEFORE );
	  SKIPLIST_val(sl,count) = 1 + atoi ( p );
	  while ( *p >= '0' && *p <= '9' ) ++p;
	}
      }
    }
  }
  Set_SKIPLIST_kind ( sl, count, SK_NONE );

  if ( Get_Trace ( TP_MISC, 0x80 ) ) {
    Print_Skiplist ( TFile, sl, "Build_Skiplist" );
  }

  return sl;
}

/* ====================================================================
 *
 * Query_Skiplist
 *
 * Query a skiplist.  A TRUE result means that the element passed is in
 * the skiplist.  The semantics of the list is as follows:  Return TRUE
 * if elmt is equal to an SK_EQUAL element of the list.  Return TRUE if
 * elmt is greater than an SK_AFTER element AND it is less than an
 * immediately following SK_BEFORE element; otherwise skip over the
 * following SK_BEFORE element.  Return TRUE if elmt is smaller than an
 * SK_BEFORE which does not immediately follow an SK_AFTER.  If nothing
 * on the list produces a TRUE result, return FALSE.  That is, a list
 * consists of SK_EQUAL elements, SK_AFTER/SK_BEFORE pairs in that
 * order, or SK_AFTER and SK_BEFORE elements that aren't in such pairs.
 * Any match of one of these tests causes a skip.
 *
 * ====================================================================
 */

BOOL
Query_Skiplist ( SKIPLIST *skip, INT32 elmt )
{
  INT32 i;
  BOOL ok;

  if ( skip == NULL ) return FALSE;

  for ( i = 0; SKIPLIST_kind(skip,i) != SK_NONE; i++ ) {
    switch ( SKIPLIST_kind(skip,i) ) {
      case SK_EQUAL:
	/* printf ( "<skip> equal %d ? %d\n", SKIPLIST_val(skip,i), elmt ); */
	if ( SKIPLIST_val(skip,i) == elmt ) return TRUE;
	break;
      case SK_AFTER:
	ok = ( SKIPLIST_val(skip,i) < elmt );
	/* printf ( "<skip> after %d ? %d\n", SKIPLIST_val(skip,i), elmt ); */
	if ( SKIPLIST_kind(skip,i+1) == SK_BEFORE 
	  && SKIPLIST_val(skip,i+1) > SKIPLIST_val(skip,i)) 
	{
	  if ( SKIPLIST_val(skip,++i) <= elmt ) ok = FALSE;
	}
	if ( ok ) return TRUE;
	break;
      case SK_BEFORE:
	/* printf ( "<skip> before %d ? %d\n", SKIPLIST_val(skip,i), elmt ); */
	if ( SKIPLIST_val(skip,i) > elmt ) return TRUE;
	break;
    }
  }
  return FALSE;
}

/* ====================================================================
 *
 * Process_Trace_Option
 *
 * Given a trace option -t..., process it.  The caller should already
 * have determined that it cannot be anything else.  The option should
 * be the full option string, with leading -t... (for error messages).
 * such options may be catenated, except for those which take name
 * operands which can't be delimited, which must be last.
 *
 * ====================================================================
 */

BE_EXPORTED BOOL
Process_Trace_Option ( char *option )
{
  char *cp = option+1;
  INT32 phase;

  Tracing_Enabled = TRUE;

  while ( cp && *cp == 't' ) {
    cp += 2;

    switch ( *(cp-1) ) {
    case 'a':
	Set_Trace (TKIND_ALLOC,
		   Get_Trace_Phase_Number ( &cp, option ) );
	break;

    case 'b':
	Set_Trace (TKIND_BB,
		   Get_Numeric_Flag (&cp, 0, 32767, 0, option));
	break;

    case 'c':
	Set_Trace (TKIND_CTRL,
		   Get_Numeric_Flag (&cp, 0, 32767, 0, option));
	break;

    case 'd':
	Set_Trace (TKIND_DEBUG,
		   Get_Numeric_Flag (&cp, 1, 32767, 0, option));
	break;

    /* ex: -Wb,-tf0 for function 0 in the file */
    case 'f':
	if ( isdigit (*cp) ) {
	  Set_Trace_Pu_Number (
	      Get_Numeric_Flag (&cp, 0, 0x7fffffff, 0, option) );
	} else {
	  Set_Trace_Pu ( cp );
	  cp = 0;		/* Done with this flag */
	}
	break;

    /* ex: -Wb,-tg1 for region 1 in each function (best when used with -tf) */
    case 'g':
	if ( isdigit (*cp) ) {
	  Set_Trace_Region_Number (
	      Get_Numeric_Flag (&cp, 0, 0x7fffffff, 0, option) );
	} else
	  Is_True(FALSE,("Process_Trace_Option: regions don't have names"));
	break;

    case 'i':
	Set_Trace (TKIND_INFO,
		   Get_Numeric_Flag (&cp, 1, 32767, 0, option));
	break;

    case 'n':
	Set_Trace (TKIND_TN,
		   Get_Trace_Phase_Number ( &cp, option ) );
	break;
    
    case 'p':
	phase = Get_Trace_Phase_Number ( &cp, option );
	if ( phase != 0 && (*cp == ',' || *cp == '\0'))
	  Set_Trace (TKIND_XPHASE, phase);
	break;
    
    case 'r':
	Set_Trace (TKIND_IR,
		   Get_Trace_Phase_Number ( &cp, option ) );
	break;

    case 's':
	Set_Trace (TKIND_SYMTAB,
		   Get_Trace_Phase_Number ( &cp, option ) );
	Symbol_Table_Out = TRUE;
	break;

    case 't':
	phase = Get_Trace_Phase_Number ( &cp, option );
	if ( *cp != ',' && *cp != ':' ) {
	    ErrMsg ( EC_Trace_Flag, *cp, option );
	    break;
	}
	cp++;	    /* skip separator */
	Set_Trace (phase,
		   Get_Numeric_Flag (&cp, 1, 0xffffffff, 0,
				     option));
	break;

#ifdef FRONT_END
    case 'v':
	{
	  /* Used by the EDG front ends to control tracing verbosity: */
	  extern BOOL trace_verbose;	/* Type must match a_boolean */
	  trace_verbose = TRUE;
	}
	break;
#endif

    case 0:   
	ErrMsg ( EC_Trace_Flag, '?', option );
	return FALSE;

    default:
	--cp;
	ErrMsg ( EC_Trace_Flag, *cp, option );
	return FALSE;
    }

  }

  if ( cp && *cp != 0 ) {
    ErrMsg ( EC_Trace_Flag, '?', option );
    return FALSE;
  }
  return TRUE;

}

/* ====================================================================
 *
 * List_Compile_Options
 *
 * List options to the given file.  This is primarily an interface to
 * the flags.c routine Print_Option_Groups, but also prints a number of
 * non-group options.  The "internal" flag indicates whether to list
 * internal-only options.  The "full" flag indicates whether to list
 * all options if "internal" is set; otherwise option group listing
 * is controlled by List_Options and List_All_Options.
 *
 * ====================================================================
 */

BE_EXPORTED void
List_Compile_Options (
  FILE *f,
  char *pfx,
  BOOL internal,
  BOOL full,
  BOOL update )
{
  char *bar = SBar+12;	/* Shorten it a bit */


  /* [VCdV] Dump full command line to assembly file */
#ifdef TARG_STxP70
  if (Cmd_Line!=NULL) {
    FILE* fcmdline = fopen(Cmd_Line, "r");
    const int bufsize = 100;
    char buf[bufsize];
    
    if (fcmdline!=NULL)
      {
        int s=bufsize;
        fprintf(f , "\n%s%s%s Command-line: ", pfx, bar,
                pfx);
        while (s==bufsize)
          {
            s=fread((char*)buf, sizeof(char), bufsize, fcmdline);
            fwrite((char*)buf, sizeof(char), s, f);
          }

        fclose(fcmdline);
        fprintf(f , "\n%s%s\n", pfx,  bar);
      }
  }
#endif

  fprintf ( f, "\n%s%s%s Compiling %s (%s)\n%s%s",
	    pfx, bar, pfx, Src_File_Name, Irb_File_Name, pfx, bar ); 
  fprintf ( f, "\n%s%s%s Options:\n%s%s", pfx, bar, pfx, pfx, bar );

  fprintf ( f, "%s  Target:%s, ISA:%s, Pointer Size:%d\n",
	    pfx, Targ_Name (Target), Isa_Name (Target_ISA),
	    (Use_32_Bit_Pointers ? 32 : 64) );
#ifdef TARG_ST
  if (OPTION_Space)
    fprintf ( f, "%s  -Os\t(Optimization level: %d)\n", pfx, OPTION_Space);
  else
#endif
  fprintf ( f, "%s  -O%d\t(Optimization level)\n", pfx, Opt_Level );
  fprintf ( f, "%s  -g%d\t(Debug level)\n", pfx, Debug_Level );
  
  if ( Min_Error_Severity == ES_ADVISORY )
      fprintf ( f, "%s  -m2\t(Report advisories)\n", pfx );
  else if ( Min_Error_Severity == ES_WARNING )
      fprintf ( f, "%s  -m1\t(Report warnings)\n", pfx );
  else
      fprintf ( f, "%s  -m0\t(Report errors only)\n", pfx );
  
  fprintf ( f, "%s%s\n", pfx, bar );

  if ( internal || List_Options ) {
    fprintf ( f, "%s Group options are marked with '#' if changed,\n"
		 "%s or with '=' if explicitly set to default value.\n",
	      pfx, pfx );
    Print_Option_Groups ( f, Common_Option_Groups, pfx, internal,
			  internal ? full : List_All_Options , update );
  }
#ifdef TARG_STxP70
  fprintf(f, "\n\t.double%d\n", Double_Is_Short? 32: 64);
#endif
}

// MAPPING between register class and preg

typedef struct {
  PREG_NUM min;
  PREG_NUM max;
} preg_range_t;
static preg_range_t *Rclass_To_Preg_array;
//TB:Initilaize rclass to preg mapping
static void Initialize_RegisterClass_To_Preg(void)
{
  int preg_index = 1; //First PREG must not be 0
  ISA_REGISTER_CLASS rclass;
  Rclass_To_Preg_array = TYPE_MEM_POOL_ALLOC_N(preg_range_t, Malloc_Mem_Pool,
					       ISA_REGISTER_CLASS_MAX+1);

  FOR_ALL_ISA_REGISTER_CLASS( rclass ) {
    const ISA_REGISTER_CLASS_INFO *icinfo = ISA_REGISTER_CLASS_Info(rclass);
    INT first_isa_reg  = ISA_REGISTER_CLASS_INFO_First_Reg(icinfo);
    INT last_isa_reg   = ISA_REGISTER_CLASS_INFO_Last_Reg(icinfo);
    INT register_count = last_isa_reg - first_isa_reg + 1;
    Rclass_To_Preg_array[rclass].min = preg_index;
    Rclass_To_Preg_array[rclass].max = preg_index + register_count - 1;
    // Next index
    preg_index = preg_index + register_count;
  }
}

//TB:Reset rclass to preg mapping This to has to be done when the
//targinfo has loaded the extension to rebuild Rclass_To_Preg_array
//with the new extended targinfo
void Reset_RegisterClass_To_Preg(void)
{
  MEM_POOL_FREE(Malloc_Mem_Pool, Rclass_To_Preg_array);
  Rclass_To_Preg_array = NULL;
}

PREG_NUM CGTARG_Regclass_Preg_Min(  ISA_REGISTER_CLASS rclass)
{
  if (Rclass_To_Preg_array == NULL)
    Initialize_RegisterClass_To_Preg();    
  return Rclass_To_Preg_array[rclass].min;
}

PREG_NUM CGTARG_Regclass_Preg_Max(  ISA_REGISTER_CLASS rclass)
{
  if (Rclass_To_Preg_array == NULL)
    Initialize_RegisterClass_To_Preg();    
  return Rclass_To_Preg_array[rclass].max;
}
