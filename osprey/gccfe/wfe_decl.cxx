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


// translate gnu decl trees to whirl

#include "W_values.h"
#include <sys/types.h>
#include <elf.h>

#include "gnu_config.h"
extern "C" {
#include "gnu/flags.h"
#include "gnu/system.h"
#include "gnu/tree.h"
#include "gnu/c-common.h"
#include "gnu/toplev.h"
#ifdef TARG_ST
  /* (cbr) fix includes */
#include "gnu/real.h"
#endif
#include "function.h"
#include "c-pragma.h"
}
#ifdef TARG_IA32
// the definition in gnu/config/i386/i386.h causes problem
// with the enumeration in common/com/ia32/config_targ.h
#undef TARGET_PENTIUM
#endif /* TARG_IA32 */

#include "defs.h"
#include "errors.h"
#include "glob.h"
#include "wn.h"
#include "wn_util.h"
#include "symtab.h"
#include "const.h"
#include "pu_info.h"
#include "ir_bwrite.h"
#include "ir_reader.h"
#include "tree_symtab.h"
#include "wfe_decl.h"
#include "wfe_misc.h"
#include "wfe_dst.h"
#include "wfe_expr.h"
#include "wfe_stmt.h"
#ifdef TARG_ST
#include "wfe_pragmas.h"
#include "wfe_loader.h" //[TB] For Map_Reg_To_Preg
#endif
// #define WFE_DEBUG

#ifdef TARG_ST
// [CG] Handle volatile field accesses. See comments
// in the implementation
static void function_update_volatile_accesses(WN *func_wn);
#endif

extern FILE *tree_dump_file; // for debugging only

extern PU_Info *PU_Tree_Root;
static PU_Info *PU_Info_Table     [258] = {0};
static ST      *Return_Address_ST [258] = {0};
static INT32    Save_Expr_Table   [258] = {0};
extern INT32    wfe_save_expr_stack_last;
static BOOL map_mempool_initialized = FALSE;
static MEM_POOL Map_Mem_Pool;
ST* WFE_Vararg_Start_ST;

// Because we build inito's piecemeal via calls into wfe for each piece,
// need to keep track of current inito and last initv that we append to.
static INITO_IDX aggregate_inito = 0;
static INITV_IDX last_aggregate_initv = 0;	
static BOOL not_at_root = FALSE;

static int __ctors = 0;
static int __dtors = 0;

#ifdef TARG_ST
// [SC] For compatibility with g++fe, provide this
// function for use by code common to gccfe and g++fe
// (in particular, the builtins expansion).
tree Current_Function_Decl(void) {return current_function_decl;}
#endif

extern "C" tree lookup_name (tree);

/* Generate WHIRL representing an asm at file scope (between
  functions). This is an awful hack. */
void
WFE_Assemble_Asm(const char *asm_string)
{
  ST *asm_st = New_ST(GLOBAL_SYMTAB);
  ST_Init(asm_st,
	  Str_To_Index (Save_Str (asm_string),
			Global_Strtab),
	  CLASS_NAME,
	  SCLASS_UNKNOWN,
	  EXPORT_LOCAL,
	  (TY_IDX) 0);

  Set_ST_asm_function_st(*asm_st);

  WN *func_wn = WN_CreateEntry(0,
			       asm_st,
			       WN_CreateBlock(),
			       NULL,
			       NULL);

  /* Not sure how much setup of WN_MAP mechanism, etc. we need to do.
   * Pretty certainly we need to set up some PU_INFO stuff just to get
   * this crazy hack of a FUNC_ENTRY node written out to the .B file.
   */

  /* This code patterned after "wfe_decl.cxx":WFE_Start_Function, and
     specialized for the application at hand. */

#ifdef ASM_NEEDS_WN_MAP
    /* deallocate the old map table */
    if (Current_Map_Tab) {
        WN_MAP_TAB_Delete(Current_Map_Tab);
        Current_Map_Tab = NULL;
    }

    /* set up the mem pool for the map table and predefined mappings */
    if (!map_mempool_initialized) {
        MEM_POOL_Initialize(&Map_Mem_Pool,"Map_Mem_Pool",FALSE);
        map_mempool_initialized = TRUE;
    } else {
        MEM_POOL_Pop(&Map_Mem_Pool);
    }

    MEM_POOL_Push(&Map_Mem_Pool);

    /* create the map table for the next PU */
    (void)WN_MAP_TAB_Create(&Map_Mem_Pool);
#endif

    // This non-PU really doesn't need a symbol table and the other
    // trappings of a local scope, but if we create one, we can keep
    // all the ir_bread/ir_bwrite routines much more blissfully
    // ignorant of the supreme evil that's afoot here.

    FmtAssert(CURRENT_SYMTAB == GLOBAL_SYMTAB,
	      ("file-scope asm must be at global symtab scope."));

    New_Scope (CURRENT_SYMTAB + 1, Malloc_Mem_Pool, TRUE);

    if (Show_Progress) {
      fprintf (stderr, "Asm(%s)\n", ST_name (asm_st));
      fflush (stderr);
    }
    PU_Info *pu_info;
    /* allocate a new PU_Info and add it to the list */
    pu_info = TYPE_MEM_POOL_ALLOC(PU_Info, Malloc_Mem_Pool);
    PU_Info_init(pu_info);

    Set_PU_Info_tree_ptr (pu_info, func_wn);
    PU_Info_maptab (pu_info) = Current_Map_Tab;
    PU_Info_proc_sym (pu_info) = ST_st_idx(asm_st);
    PU_Info_pu_dst (pu_info) = DST_Create_Subprogram (asm_st,/*tree=*/0);
    PU_Info_cu_dst (pu_info) = DST_Get_Comp_Unit ();

    Set_PU_Info_state(pu_info, WT_SYMTAB, Subsect_InMem);
    Set_PU_Info_state(pu_info, WT_TREE, Subsect_InMem);
    Set_PU_Info_state(pu_info, WT_PROC_SYM, Subsect_InMem);

    Set_PU_Info_flags(pu_info, PU_IS_COMPILER_GENERATED);

    if (PU_Info_Table [CURRENT_SYMTAB])
      PU_Info_next (PU_Info_Table [CURRENT_SYMTAB]) = pu_info;
    else
      PU_Tree_Root = pu_info;

    PU_Info_Table [CURRENT_SYMTAB] = pu_info;

  /* This code patterned after "wfe_decl.cxx":WFE_Finish_Function, and
     specialized for the application at hand. */

    // write out all the PU information
    pu_info = PU_Info_Table [CURRENT_SYMTAB];

    /* deallocate the old map table */
    if (Current_Map_Tab) {
        WN_MAP_TAB_Delete(Current_Map_Tab);
        Current_Map_Tab = NULL;
    }

    PU_IDX pu_idx;
    PU &pu = New_PU(pu_idx);
    PU_Init(pu, (TY_IDX) 0, CURRENT_SYMTAB);
    Set_PU_no_inline(pu);
    Set_PU_no_delete(pu);
    Set_ST_pu(*asm_st, pu_idx);

    Write_PU_Info (pu_info);

    // What does the following line do?
    PU_Info_Table [CURRENT_SYMTAB+1] = NULL;

    Delete_Scope(CURRENT_SYMTAB);
    --CURRENT_SYMTAB;
}


#ifdef KEY
// Contents of the array set up below: 
// exc_ptr ST_IDX, filter ST_IDX, typeinfo INITO_IDX, eh_spec INITO_IDX
static void
Setup_Entry_For_EH (void)
{
    const int lbnd = 0;
    const int hbnd = 3;

    ARB_HANDLE arb = New_ARB();
    ARB_Init (arb, lbnd, hbnd, 4);
    Set_ARB_flags (arb, ARB_flags(arb) | ARB_FIRST_DIMEN | ARB_LAST_DIMEN);
    STR_IDX str = Save_Str ("__EH_INFO_PER_PU__");
    TY_IDX ty;
    TY_Init (New_TY(ty), (hbnd+1)/* # of entries */ * 4 /* sizeof */, KIND_ARRAY, MTYPE_M, str);
    Set_TY_arb (ty, arb);
    Set_TY_etype (ty, MTYPE_TO_TY_array[MTYPE_U4]);
    ST * etable = New_ST (CURRENT_SYMTAB);
    ST_Init (etable, str, CLASS_VAR, SCLASS_EH_REGION_SUPP, EXPORT_LOCAL, ty);
    Set_ST_is_initialized (*etable);
    Set_ST_one_per_pu (etable);

    ST  * exc_ptr_st = New_ST (CURRENT_SYMTAB);
    ST_Init (exc_ptr_st, Save_Str ("__Exc_Ptr__"), CLASS_VAR, SCLASS_AUTO,
			EXPORT_LOCAL, MTYPE_To_TY(Pointer_Mtype));
    Set_ST_one_per_pu (exc_ptr_st);

#ifdef TARG_ST
#if 0
    /* (cbr) valid if EH_RETURN_DATA_REGNO live in a callee registers.
       if not must lower_landing_pad_entry to force their saving on handler's entry */
 {
#ifndef TARG_ST
	extern PREG_NUM Map_Reg_To_Preg [];
#else
	//TB now Map_Reg_To_Preg is defined.
#endif
  TY_IDX ty_idx = ST_type (exc_ptr_st);
  Set_TY_is_volatile (ty_idx);
  Set_ST_type (exc_ptr_st, ty_idx);
  Set_ST_assigned_to_dedicated_preg (exc_ptr_st);
  ST_ATTR_IDX st_attr_idx;
  ST_ATTR&    st_attr = New_ST_ATTR (CURRENT_SYMTAB, st_attr_idx);
  ST_ATTR_Init (st_attr, ST_st_idx (exc_ptr_st), ST_ATTR_DEDICATED_REGISTER,
                Map_Reg_To_Preg[EH_RETURN_DATA_REGNO(0)]);
 }
#endif
#endif

    INITV_IDX exc_ptr_iv = New_INITV();
    INITV_Set_VAL (Initv_Table[exc_ptr_iv], Enter_tcon (Host_To_Targ (MTYPE_U4,
                                ST_st_idx (exc_ptr_st))), 1);

    ST  * filter_st = New_ST (CURRENT_SYMTAB);
    ST_Init (filter_st, Save_Str ("__Exc_Filter__"), CLASS_VAR, SCLASS_AUTO,
	                EXPORT_LOCAL, MTYPE_To_TY(TARGET_64BIT ? MTYPE_U8 : MTYPE_U4));
    Set_ST_one_per_pu (filter_st);

#ifdef TARG_ST
#if 0
    /* (cbr) valid if EH_RETURN_DATA_REGNO live in a callee registers.
       if not must lower_landing_pad_entry to force their saving on handler's entry */
 {
#ifndef TARG_ST
	extern PREG_NUM Map_Reg_To_Preg [];
#else
	//TB now Map_Reg_To_Preg is defined
#endif
  TY_IDX ty_idx = ST_type (filter_st);
  Set_TY_is_volatile (ty_idx);
  Set_ST_type (filter_st, ty_idx);
  Set_ST_assigned_to_dedicated_preg (filter_st);
  ST_ATTR_IDX st_attr_idx;
  ST_ATTR&    st_attr = New_ST_ATTR (CURRENT_SYMTAB, st_attr_idx);
  ST_ATTR_Init (st_attr, ST_st_idx (filter_st), ST_ATTR_DEDICATED_REGISTER,
                Map_Reg_To_Preg[EH_RETURN_DATA_REGNO(1)]);
 }
#endif
#endif

    INITV_IDX filter_iv = New_INITV();
    INITV_Set_VAL (Initv_Table[filter_iv], Enter_tcon (Host_To_Targ (MTYPE_U4,
                                ST_st_idx (filter_st))), 1);
    Set_INITV_next (exc_ptr_iv, filter_iv);
    // this will be filled in later if there are type-filter entries
    INITV_IDX tinfo = New_INITV();
    INITV_Set_VAL (Initv_Table[tinfo], Enter_tcon (Host_To_Targ (MTYPE_U4,
                                0)), 1);
    Set_INITV_next (filter_iv, tinfo);
    // this will be filled in later if there are exception specifications
    INITV_IDX eh_spec = New_INITV();
    INITV_Set_VAL (Initv_Table[eh_spec], Enter_tcon (Host_To_Targ (MTYPE_U4,
                                0)), 1);
    Set_INITV_next (tinfo, eh_spec);

    Get_Current_PU().unused = New_INITO (ST_st_idx (etable), exc_ptr_iv);
}
#endif

extern void
WFE_Start_Function (tree fndecl)
{
#ifdef TARG_ST
  // (cbr) emit ident directives if any
  if (idents_strs) {
    WFE_Idents();
  }
#endif

#ifdef WFE_DEBUG
    fprintf (stdout, "Start Function:\n");
    print_tree (stdout, fndecl);
#endif

    if (CURRENT_SYMTAB != GLOBAL_SYMTAB) {

      Set_PU_uplevel (Get_Current_PU ());
#ifndef TARG_ST
      // [SC] We should be allowed to inline nested functions,
      // and code in glibc requires it.
      Set_PU_no_inline (Get_Current_PU ());
#endif
    }
#ifdef TARG_ST
    else {
      // [CL] clean function scope inline pragmas
      // only if not entering a nested function
      Clear_Function_Pragma_List(DISCARD);
    }
    Clear_Callsite_Pragma_List(DISCARD);
#endif

    /* deallocate the old map table */
    if (Current_Map_Tab) {
        WN_MAP_TAB_Delete(Current_Map_Tab);
        Current_Map_Tab = NULL;
    }

    /* set up the mem pool for the map table and predefined mappings */
    if (!map_mempool_initialized) {
        MEM_POOL_Initialize(&Map_Mem_Pool,"Map_Mem_Pool",FALSE);
        map_mempool_initialized = TRUE;
    } else {
        MEM_POOL_Pop(&Map_Mem_Pool);
    }

    MEM_POOL_Push(&Map_Mem_Pool);

    /* create the map table for the next PU */
    (void)WN_MAP_TAB_Create(&Map_Mem_Pool);

    New_Scope (CURRENT_SYMTAB + 1, Malloc_Mem_Pool, TRUE);

    // handle VLAs in the declaration
    WN *vla_block = WN_CreateBlock ();
    WFE_Stmt_Push (vla_block, wfe_stmk_func_body, Get_Srcpos());

    ST        *func_st;
    ST_EXPORT  eclass;
#ifdef TARG_ST
    eclass = Get_Export_Class_For_Tree (fndecl, CLASS_FUNC, SCLASS_TEXT);
#else
    eclass = TREE_PUBLIC(fndecl) ? EXPORT_PREEMPTIBLE
      : EXPORT_LOCAL;
#endif

#ifndef TARG_ST
    /* (cbr) use MUST_DELETE flag instead */
    if (DECL_INLINE (fndecl) && TREE_PUBLIC (fndecl)) {
      if (DECL_EXTERNAL (fndecl) && DECL_ST2 (fndecl) == 0) {
        // encountered first extern inline definition
        ST *oldst = DECL_ST (fndecl);
        DECL_ST (fndecl) = 0;
        func_st =  Get_ST (fndecl);
        DECL_ST (fndecl) = oldst;
        DECL_ST2 (fndecl) = func_st;
        eclass = EXPORT_LOCAL;
      }
      else {
        // encountered second definition, the earlier one was extern inline
        func_st = Get_ST (fndecl);
        DECL_ST2 (fndecl) = 0;
      }
    }
    else
#endif

#ifdef TARG_ST
      /* (cbr) redefinition of extern inline function */
        if (!DECL_EXTERNAL (fndecl) && TREE_PUBLIC (fndecl)) {
          if ((func_st = DECL_ST (fndecl)) &&
              PU_must_delete (Pu_Table [ST_pu (func_st)])) {
            DECL_ST (fndecl) = 0;
          }
      }
#endif

    func_st = Get_ST (fndecl);
    Set_ST_sclass (func_st, SCLASS_TEXT);
    Set_PU_lexical_level (Pu_Table [ST_pu (func_st)], CURRENT_SYMTAB);
    Set_PU_c_lang (Pu_Table [ST_pu (func_st)]);

    if (DECL_INLINE(fndecl)) {
      Set_PU_is_inline_function (Pu_Table [ST_pu (func_st)]);
#ifdef TARG_ST
      /* (cbr) handle extern inline extension */
      if (DECL_EXTERNAL (fndecl) && TREE_PUBLIC (fndecl)) {
        Set_PU_must_delete (Pu_Table [ST_pu (func_st)]);        
        /* (cbr) correct export scope set in Create_ST_From_Tree
        eclass = EXPORT_LOCAL;
        */
      }
#endif
      wfe_invoke_inliner = TRUE;
    }

#ifdef TARG_ST
    // [CL] noinline and always_inline attributes
    if (DECL_UNINLINABLE(fndecl)) {
      Set_PU_no_inline (Pu_Table [ST_pu (func_st)]);
    }
    if (DECL_ALWAYS_INLINE(fndecl)) {
      Set_PU_must_inline (Pu_Table [ST_pu (func_st)]);
      wfe_invoke_inliner = TRUE;
    }
    /* (cbr) const/pure attribute */
    /* Warning: 
       gcc pure maps to open64 no_side_effect
       gcc const maps to open64 pure
    */
    if (TREE_READONLY (fndecl)) {
      Set_PU_is_pure (Pu_Table [ST_pu (func_st)]);
    }
    if (DECL_IS_PURE (fndecl)) {
      Set_PU_no_side_effects (Pu_Table [ST_pu (func_st)]);
    }

    // [CR] interrupt attributes
    if (lookup_attribute("interrupt", DECL_ATTRIBUTES(fndecl))) {
      Set_PU_is_interrupt(Pu_Table [ST_pu (func_st)]);
    }
    if (lookup_attribute("interrupt_nostkaln", DECL_ATTRIBUTES(fndecl))) {
      Set_PU_is_interrupt_nostkaln(Pu_Table [ST_pu (func_st)]);
    }

    // [VB] Stack alignment attribute for more than 64bit types
      {
	tree attr = lookup_attribute("aligned_stack", DECL_ATTRIBUTES(fndecl));
	if (attr) {
	  attr = TREE_VALUE (TREE_VALUE (attr));
	  FmtAssert (TREE_CODE(attr) == INTEGER_CST, 
		     ("Malformed stack alignment attribute - arg not an integer"));
	  UINT64 h = TREE_INT_CST_HIGH (attr);
	  UINT64 l = TREE_INT_CST_LOW (attr);
	  l = l << 32 >> 32;	
	  h = h << 32;
	  UINT64 val = h | l;
	  FmtAssert (((val > 8) && (val <= 256) && ((val & (val-1)) == 0)),
		     ("Malformed stack alignment attribute - value must be a power of 2 strictly greater than 8 and less than or equal to 256"));
	  Set_PU_aligned_stack(Pu_Table [ST_pu (func_st)], val);
	}
      }

#endif
    Set_ST_export (func_st, eclass);

#ifdef TARG_ST
    /* Support for function attributes. */
    if (DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT(fndecl)) {
      Set_PU_no_instrument_function(Pu_Table [ST_pu (func_st)]);
    }
#endif

    if (Show_Progress) {
      fprintf (stderr, "Compiling %s \n", ST_name (func_st));
      fflush (stderr);
    }

    Scope_tab [Current_scope].st = func_st;

#ifdef TARG_ST
    if (flag_exceptions)
      Setup_Entry_For_EH ();
#endif

    INT num_args = 0;
    tree pdecl;
    for (pdecl = DECL_ARGUMENTS (fndecl); pdecl; pdecl = TREE_CHAIN (pdecl)) {
      TY_IDX arg_ty_idx = Get_TY(TREE_TYPE(pdecl));
      if (!WFE_Keep_Zero_Length_Structs   &&
          TY_mtype (arg_ty_idx) == MTYPE_M &&
          TY_size (arg_ty_idx) == 0) {
        // zero length struct parameter
      }
      else
	++num_args;
    }

#ifdef TARG_ST
    // [TTh] Check that dynamically added mtypes are not used
    //       for return value and arguments of function.
    {
      // Check argument type
      INT pi;
      for (pi=1, pdecl = DECL_ARGUMENTS (fndecl);
	   pdecl;
	   pi++, pdecl = TREE_CHAIN (pdecl)) {
	TY_IDX arg_ty_idx = Get_TY(TREE_TYPE(pdecl));
	if (MTYPE_is_dynamic(TY_mtype(arg_ty_idx))) {
	  error ("forbidden type `%s' for parameter %d of `%s'",
		 MTYPE_name(TY_mtype(arg_ty_idx)),
		 pi,
		 (DECL_NAME (fndecl)) ? IDENTIFIER_POINTER (DECL_NAME (fndecl)) : "<unknown>");
	}
      }
      // Check return value
      if (DECL_RESULT(fndecl)) {
	TY_IDX ret_ty_idx = Get_TY(TREE_TYPE(DECL_RESULT(fndecl)));
	if (MTYPE_is_dynamic(TY_mtype(ret_ty_idx))) {
	  error ("forbidden return type `%s' for `%s'",
		 MTYPE_name(TY_mtype(ret_ty_idx)),
		 (DECL_NAME (fndecl)) ? IDENTIFIER_POINTER (DECL_NAME (fndecl)) : "<unknown>");
	}
      }
    }
#endif

    WN *body, *wn;
    body = WN_CreateBlock ( );
    wn = WN_CreateEntry ( num_args, func_st, body, NULL, NULL );

    /* from 1..nkids, create idname args */
    INT i = 0;
    for (pdecl = DECL_ARGUMENTS (fndecl); pdecl; pdecl = TREE_CHAIN (pdecl) )
    {
      TY_IDX arg_ty_idx = Get_TY(TREE_TYPE(pdecl));
      ST *st = Get_ST(pdecl);

#ifdef TARG_ST
      /* (cbr) parameters always passed as values */
      Set_ST_is_value_parm(st);
#endif

      if (!WFE_Keep_Zero_Length_Structs   &&
          TY_mtype (arg_ty_idx) == MTYPE_M &&
          TY_size (arg_ty_idx) == 0) {
        // zero length struct parameter
      }
      else {
        if (TY_mtype (arg_ty_idx) == MTYPE_F4 &&
            !TY_has_prototype (ST_pu_type (func_st)) &&
	    !flag_short_double /* disable promotion if -fshort-double.  */)
          Set_ST_promote_parm (st);
	WN_kid(wn,i) = WN_CreateIdname ( 0, ST_st_idx(st) );
	++i;
      }
    }

    PU_Info *pu_info;
    /* allocate a new PU_Info and add it to the list */
    pu_info = TYPE_MEM_POOL_ALLOC(PU_Info, Malloc_Mem_Pool);
    PU_Info_init(pu_info);

    Set_PU_Info_tree_ptr (pu_info, wn);
    PU_Info_maptab (pu_info) = Current_Map_Tab;
    PU_Info_proc_sym (pu_info) = ST_st_idx(func_st);
    PU_Info_pu_dst (pu_info) = DST_Create_Subprogram (func_st,fndecl);
    PU_Info_cu_dst (pu_info) = DST_Get_Comp_Unit ();

    Set_PU_Info_state(pu_info, WT_SYMTAB, Subsect_InMem);
    Set_PU_Info_state(pu_info, WT_TREE, Subsect_InMem);
    Set_PU_Info_state(pu_info, WT_PROC_SYM, Subsect_InMem);

    Set_PU_Info_flags(pu_info, PU_IS_COMPILER_GENERATED);

    if (PU_Info_Table [CURRENT_SYMTAB])
      PU_Info_next (PU_Info_Table [CURRENT_SYMTAB]) = pu_info;

    else
    if (CURRENT_SYMTAB == GLOBAL_SYMTAB + 1)
      PU_Tree_Root = pu_info;

    else
      PU_Info_child (PU_Info_Table [CURRENT_SYMTAB -1]) = pu_info;

    PU_Info_Table [CURRENT_SYMTAB] = pu_info;
    Save_Expr_Table [CURRENT_SYMTAB] = wfe_save_expr_stack_last;

    WFE_Stmt_Pop (wfe_stmk_func_body);

    WFE_Stmt_Push (wn, wfe_stmk_func_entry, Get_Srcpos());
    WFE_Stmt_Push (body, wfe_stmk_func_body, Get_Srcpos());

    wn = WN_CreatePragma (WN_PRAGMA_PREAMBLE_END, (ST_IDX) NULL, 0, 0);
    WFE_Stmt_Append (wn, Get_Srcpos());
    WFE_Stmt_Append (vla_block, Get_Srcpos());

    WFE_Vararg_Start_ST = NULL;

#if !defined (TARG_ST) || (GNU_FRONT_END!=33)
    if (current_function_varargs) {
      // the function uses varargs.h
      // throw off the old type declaration as it did not 
      // take into account any arguments
      PU& pu = Pu_Table[ST_pu (func_st)];
      TY_IDX ty_idx;
      TY &ty = New_TY (ty_idx);
      TY_Init (ty, 0, KIND_FUNCTION, MTYPE_UNKNOWN, STR_IDX_ZERO);
      Set_TY_align (ty_idx, 1);
      TYLIST tylist_idx;
      Set_TYLIST_type (New_TYLIST (tylist_idx),
                       Get_TY(TREE_TYPE(TREE_TYPE(fndecl))));
      Set_TY_tylist (ty, tylist_idx);
      for (pdecl = DECL_ARGUMENTS (fndecl); pdecl; pdecl = TREE_CHAIN (pdecl) ) {
	WFE_Vararg_Start_ST = Get_ST(pdecl);
        Set_TYLIST_type (New_TYLIST (tylist_idx), ST_type(WFE_Vararg_Start_ST));
      }
      Set_TYLIST_type (New_TYLIST (tylist_idx), 0);
      Set_TY_is_varargs (ty_idx);
      Set_PU_prototype (pu, ty_idx);
    }
#endif
}

extern void
WFE_Finish_Function (void)
{
    WFE_Check_Undefined_Labels ();
    PU_Info *pu_info = PU_Info_Table [CURRENT_SYMTAB];
    wfe_save_expr_stack_last = Save_Expr_Table [CURRENT_SYMTAB];

    if (CURRENT_SYMTAB > GLOBAL_SYMTAB + 1) {

#ifndef TARG_ST
      // [SC]: Nested functions are now supported.
      DevWarn ("Encountered nested function");
#endif
      Set_PU_is_nested_func (Get_Current_PU ());
#ifndef TARG_ST
      // [SC] We should be allowed to inline nested functions,
      // and code in glibc requires it.
      Set_PU_no_inline (Get_Current_PU ());
#endif
    }

#ifdef TARG_ST
    extern void Do_Cleanups_For_EH (void);
    extern bool need_manual_unwinding;

    // Insert a RETURN if it does not exist
    {
      WN * wn = WN_last (WFE_Stmt_Top ());
#ifdef TARG_ST 
    /* GNU C interprets a `volatile void' return type to indicate
       that the function does not return. 
       function definition: no need to emit a return sequence */
    if (! (TREE_THIS_VOLATILE (current_function_decl) &&
           TREE_TYPE (TREE_TYPE (current_function_decl))))
#endif
      if (wn == NULL || WN_operator (wn) != OPR_RETURN &&
          WN_operator (wn) != OPR_RETURN_VAL) {
        WFE_Stmt_Append (WN_CreateReturn (), Get_Srcpos ());
      }
    }

    if (flag_exceptions && need_manual_unwinding) {
      Do_Cleanups_For_EH();
      need_manual_unwinding=false;
    }
    else
      Get_Current_PU().unused = 0;
#endif

#ifdef TARG_ST
     WFE_Stmt_Pop (wfe_stmk_func_body);
#else
    // write out all the PU information
    WN *wn = WFE_Stmt_Pop (wfe_stmk_func_body);
      // Insert a RETURN at the end if it does not exist
      if (WN_last (wn) == NULL ||
          (WN_operator (WN_last (wn)) != OPR_RETURN &&
           WN_operator (WN_last (wn)) != OPR_RETURN_VAL)) {
        WN_INSERT_BlockLast (wn, WN_CreateReturn ());
#ifdef TARG_ST
	// [CL] force line number for closing brace
	WN_Set_Linenum(WN_last(wn), Get_Srcpos());
#endif
      }
#endif

    WN *func_wn = WFE_Stmt_Pop (wfe_stmk_func_entry);
    if (PU_has_syscall_linkage (Get_Current_PU ())) {
      Set_PU_no_inline (Get_Current_PU ());
/*
      WN     *pragmas_wn = WN_func_pragmas (func_wn);
      TCON    tcon       = Host_To_Targ_String (MTYPE_STRING, "-O0", 3);
      TY_IDX  ty_idx     = Be_Type_Tbl(MTYPE_I1);
      ST     *st         = Gen_String_Sym (&tcon, ty_idx, FALSE);
      wn = WN_CreatePragma (WN_PRAGMA_OPTIONS, st, 0, 0);
      WN_INSERT_BlockLast (pragmas_wn, wn);
      fdump_tree(stderr, func_wn);
*/
    }

#ifdef TARG_ST
    // [CG] Handle volatile field accesses.
    function_update_volatile_accesses(func_wn);
#endif

#ifdef WFE_DEBUG
    fprintf(stdout, "================= Dump function ================\n");
    fdump_tree(stdout, func_wn);
#endif

    /* deallocate the old map table */
    if (Current_Map_Tab) {
        WN_MAP_TAB_Delete(Current_Map_Tab);
        Current_Map_Tab = NULL;
    }

    Write_PU_Info (pu_info);

    PU_Info_Table [CURRENT_SYMTAB+1] = NULL;

    if (Return_Address_ST [CURRENT_SYMTAB]) {
      Set_PU_has_return_address (Get_Current_PU ());
      Set_PU_no_inline (Get_Current_PU ());
      Return_Address_ST [CURRENT_SYMTAB] = NULL;
    }

    Delete_Scope (CURRENT_SYMTAB);
    --CURRENT_SYMTAB;
//  if (CURRENT_SYMTAB > GLOBAL_SYMTAB)
//    Current_pu = &Pu_Table[ST_pu (Scope_tab[CURRENT_SYMTAB].st)];

#ifdef TARG_ST
    // (cbr) emit ident directives if any
    if (idents_strs) {
      WFE_Idents();
    }

    // [CL] clean function scope inline pragmas
    Clear_Callsite_Pragma_List(DISCARD);
    Clear_Function_Pragma_List(WARN);
#endif
}


void
WFE_Start_Aggregate_Init (tree decl)
{
  if (TREE_STATIC(decl)) {

#ifdef WFE_DEBUG
    fprintf(stdout,"====================================================\n");
    fprintf(stdout,"             WFE_Start_Aggregate_Init \n");
    fprintf(stdout,"====================================================\n");
    print_tree(stdout, decl);
#endif

	ST *st = Get_ST(decl);
	Set_ST_is_initialized(st);
	if (ST_sclass(st) == SCLASS_UGLOBAL ||
	    ST_sclass(st) == SCLASS_EXTERN  ||
	    ST_sclass(st) == SCLASS_COMMON)
		Set_ST_sclass(st, SCLASS_DGLOBAL);
	aggregate_inito = New_INITO (st);
	not_at_root = FALSE;
	last_aggregate_initv = 0;
  }
}

void
WFE_Add_Aggregate_Init_Padding (INT size)
{
  if (aggregate_inito == 0) return;
  if (size < 0) return;	// actually happens from assemble_zeroes
  INITV_IDX inv = New_INITV();
  INITV_Init_Pad (inv, size);
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
}

void
WFE_Add_Aggregate_Init_Integer (INT64 val, INT size)
{
  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();
  TYPE_ID mtype;
  if (size == 1) mtype = MTYPE_I1;
  else if (size == 2) mtype = MTYPE_I2;
  else if (size == 4) mtype = MTYPE_I4;
  else if (size == 8) mtype = MTYPE_I8;
  else FmtAssert(FALSE, ("WFE_Add_Aggregate_Init_Integer unexpected size"));

  INITV_Init_Integer (inv, mtype, val);
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
}

static void
WFE_Add_Init_Block(void)
{
  if (aggregate_inito == 0) return;
  INITV_IDX inv_blk = New_INITV();
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv_blk);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv_blk);
  last_aggregate_initv = inv_blk;
}

#if defined (TARG_ST) && (GNU_FRONT_END==33)
/* (cbr) gcc 3.3 upgrade */
double
real_value_to_double (REAL_VALUE_TYPE val)
{
  union { long d[2] ;  double df ; } d_ ;
  REAL_VALUE_TO_TARGET_DOUBLE (val, d_.d);
  if (HOST_FLOAT_WORDS_BIG_ENDIAN != FLOAT_WORDS_BIG_ENDIAN) {
#if __GNUC__ >= 3
      std::swap (d_.d[0], d_.d[1]);
#else
    swap (d_.d[0], d_.d[1]);
#endif // __GNUC__ >= 3
  }
  return d_.df ;
}
#endif

/* The rbuf parameter represents a double using 2 32-bits integers. We
   swap the two words when required to accomodate with different
   Host/Target endianess */
#define TARGET_F8_TO_HOST_F8(rbuf)				        \
  if (HOST_FLOAT_WORDS_BIG_ENDIAN != FLOAT_WORDS_BIG_ENDIAN) {		\
    HOST_WIDE_INT tmp;                                                  \
    tmp = rbuf[0];                                                      \
    rbuf[0] = rbuf[1];                                                  \
    rbuf[1] = tmp;                                                      \
  }

void 
WFE_Add_Aggregate_Init_Real (REAL_VALUE_TYPE real, INT size)
{
  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();
  TCON    tc;
  int     t1;
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  long     buffer [4];
#else
  int     buffer [4];
#endif

  switch (size) {
    case 4:
      REAL_VALUE_TO_TARGET_SINGLE (real, t1);
      tc = Host_To_Targ_Float_4 (MTYPE_F4, *(float *) &t1);
      break;
    case 8:
      REAL_VALUE_TO_TARGET_DOUBLE (real, buffer);
#if defined(REAL_ARITHMETIC)
            /* rbuf is made of two 32-bit pieces of the result, in the
               order they would appear in memory on the target */
            /* swap 32-bits words if host and target endianess differ */
            TARGET_F8_TO_HOST_F8(buffer);
            /* create a TCON from host double constant */
            tc = Host_To_Targ_Float (MTYPE_F8,*(double *) &buffer);
#else
            /* rbuf is made of two 32-bit pieces of the result, in the
               order they would appear in memory on the host */
            tc = Host_To_Targ_Float (MTYPE_F8, *(double *) &buffer);
#endif
      break;
    default:
      FmtAssert(FALSE, ("WFE_Add_Aggregate_Init_Real unexpected size"));
      break;
  }
  INITV_Set_VAL (Initv_Table[inv], Enter_tcon(tc), 1);
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
} /* WGE_Add_Aggregate_Init_Real */

void 
WFE_Add_Aggregate_Init_Complex (REAL_VALUE_TYPE rval, REAL_VALUE_TYPE ival, INT size)
{
  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();
  TCON    rtc;
  TCON    itc;
  int     t1;
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  long     buffer [4];
#else
  int     buffer [4];
#endif

  switch (size) {
    case 8:
      REAL_VALUE_TO_TARGET_SINGLE (rval, t1);
      rtc = Host_To_Targ_Float_4 (MTYPE_F4, *(float *) &t1);
      REAL_VALUE_TO_TARGET_SINGLE (ival, t1);
      itc = Host_To_Targ_Float_4 (MTYPE_F4, *(float *) &t1);
      break;
    case 16:
      REAL_VALUE_TO_TARGET_DOUBLE (rval, buffer);
      rtc = Host_To_Targ_Float (MTYPE_F8, *(double *) &buffer);
      REAL_VALUE_TO_TARGET_DOUBLE (ival, buffer);
      itc = Host_To_Targ_Float (MTYPE_F8, *(double *) &buffer);
      break;
    default:
      FmtAssert(FALSE, ("WFE_Add_Aggregate_Init_Complex unexpected size"));
      break;
  }
  INITV_Set_VAL (Initv_Table[inv], Enter_tcon(rtc), 1);
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
  inv = New_INITV();
  INITV_Set_VAL (Initv_Table[inv], Enter_tcon(itc), 1);
  Set_INITV_next(last_aggregate_initv, inv);
  last_aggregate_initv = inv;
}

void 
WFE_Add_Aggregate_Init_String (const char *s, INT size)
{
  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();
  INITV_Init_String (inv, (char *) s, size);
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
}

void
#ifdef TARG_ST
/* (cbr) support for half address relocation */
WFE_Add_Aggregate_Init_Symbol (ST *st, WN_OFFSET offset = 0, BOOL halfword = 0)
#else
WFE_Add_Aggregate_Init_Symbol (ST *st, WN_OFFSET offset = 0)
#endif
{
#ifdef WFE_DEBUG
    fprintf(stdout,"====================================================\n");
    fprintf(stdout,"      WFE_Add_Aggregate_Init_Symbol %s \n\n", ST_name(st));
    fprintf(stdout,"  last_aggregate_initv = %d\n", last_aggregate_initv);
    fprintf(stdout,"====================================================\n");
#endif

  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();
#ifdef TARG_ST
/* (cbr) support for half address relocation */
  INITV_Init_Symoff (inv, st, offset, 1, halfword);
#else
  INITV_Init_Symoff (inv, st, offset);
#endif

#ifdef TARG_ST
  // [CG]: it seems that alias analysis fails if this is not set
  Set_ST_addr_saved (st);
#endif
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
}

#ifdef TARG_ST
/* (cbr) DDTSst24451. add support for label diffs initializers */
void
WFE_Add_Aggregate_Init_Labeldiff (LABEL_IDX lab1, LABEL_IDX lab2) 
{
#ifdef WFE_DEBUG
    fprintf(stdout,"====================================================\n");
    fprintf(stdout,"      WFE_Add_Aggregate_Init_Labeldiff %d %d \n\n", (int)lab1, (int)lab2);
    fprintf(stdout,"  last_aggregate_initv = %d\n", last_aggregate_initv);
    fprintf(stdout,"====================================================\n");
#endif

  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();

  INITV_Init_Labdiff (inv, lab1, lab2);

  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;

  Set_LABEL_addr_saved (lab1);
  Set_LABEL_addr_saved (lab2);
}
#endif

void
WFE_Add_Aggregate_Init_Label (LABEL_IDX lab)
{
#ifdef TARG_ST
  // [CG] Implemented in same scope, no devwarn.
#else
  DevWarn ("taking address of a label at line %d", lineno);
#endif

  Set_PU_no_inline (Get_Current_PU ());
  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();
  INITV_Init_Label (inv, lab, 1);
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
  Set_LABEL_addr_saved (lab);
}

UINT
Traverse_Aggregate_Constructor (
  ST   *st, tree init_list, tree type, BOOL gen_initv,
  UINT current_offset, UINT array_elem_offset, UINT field_id);

void
WFE_Add_Aggregate_Init_Address (tree init)
{
#ifdef WFE_DEBUG
    fprintf(stdout,"====================================================\n");
    fprintf(stdout,"             WFE_Add_Aggregate_Init_Address \n");
    fprintf(stdout,"====================================================\n");
    print_tree(stdout, init);
#endif

  switch (TREE_CODE (init)) {

  case VAR_DECL:
  case FUNCTION_DECL:
	WFE_Add_Aggregate_Init_Symbol (Get_ST (init));
	break;

  case STRING_CST:
	{
	TCON tcon = Host_To_Targ_String (MTYPE_STRING,
				       TREE_STRING_POINTER(init),
				       TREE_STRING_LENGTH(init));
	ST *const_st = New_Const_Sym (Enter_tcon (tcon), 
		Get_TY(TREE_TYPE(init)));
      	WFE_Add_Aggregate_Init_Symbol (const_st);
	}
    	break;

  case PLUS_EXPR:
	if ( TREE_CODE(TREE_OPERAND(init,0)) == ADDR_EXPR
	  && TREE_CODE(TREE_OPERAND(init,1)) == INTEGER_CST)
	{
		tree addr_kid = TREE_OPERAND(TREE_OPERAND(init,0),0);
		FmtAssert(TREE_CODE(addr_kid) == VAR_DECL
			|| TREE_CODE(addr_kid) == FUNCTION_DECL,
			("expected decl under plus_expr"));
		WFE_Add_Aggregate_Init_Symbol ( Get_ST (addr_kid),
			Get_Integer_Value(TREE_OPERAND(init,1)) );
	}
	else
	{
		WN *init_wn = WFE_Expand_Expr (init);
		FmtAssert (WN_operator (init_wn) == OPR_LDA,
				("expected decl under plus_expr"));
		WFE_Add_Aggregate_Init_Symbol (WN_st (init_wn),
					       WN_offset (init_wn));
		WN_Delete (init_wn);
	}
	break;

  case INTEGER_CST:
	WFE_Add_Aggregate_Init_Integer (Get_Integer_Value (init), Pointer_Size);
	break;

  case LABEL_DECL:
	{
	 	LABEL_IDX label_idx = WFE_Get_LABEL (init, FALSE);
#ifdef TARG_ST
		// [CG] Assert we are at same scope
		FmtAssert (DECL_CONTEXT (init) == current_function_decl,
			   ("line %d: taking address of a label not defined in current function currently not implemented", lineno));
#endif
		WFE_Add_Aggregate_Init_Label (label_idx);
	}
	break;

#ifdef TARG_ST
  case CONSTRUCTOR:
    {
      // (cbr) Can't use WFE_Start/Finish_Aggregate_Init and WFE_Expand_Expr
      // here. Handle the constructor's symbol, the initv and the initos.
      char *name = "";
      INITO_IDX save_aggregate_inito = aggregate_inito;
      INITV_IDX save_last_aggregate_initv = last_aggregate_initv;
      TY_IDX ty_idx = Get_TY(TREE_TYPE(init));
      ST *temp = New_ST (GLOBAL_SYMTAB);
      ST_Init (temp, 0, CLASS_VAR, SCLASS_FSTATIC, EXPORT_LOCAL, ty_idx);
      Set_ST_is_initialized(temp);
      aggregate_inito = New_INITO (temp);

      not_at_root = FALSE;
      last_aggregate_initv = 0;

      Traverse_Aggregate_Constructor (temp, init, TREE_TYPE(init),
                                      TRUE /*gen_initv*/, 0, 0, 0);

      not_at_root = FALSE;
      aggregate_inito = save_aggregate_inito;
      last_aggregate_initv = save_last_aggregate_initv;

      WFE_Add_Aggregate_Init_Symbol (temp);
    }
    break;
    /* (cbr) first implementation. pro-release-1-7-0-B/11) */
  case COMPOUND_LITERAL_EXPR:
    {
      tree decl = COMPOUND_LITERAL_EXPR_DECL (init);
      if (TREE_CODE (decl) == VAR_DECL) {
        tree val = DECL_INITIAL(decl);
        if (val && TREE_CODE (val) == INTEGER_CST) {
          WFE_Add_Aggregate_Init_Integer (Get_Integer_Value (val), Pointer_Size);
          break;
        }
        if (val && TREE_CODE (val) == CONSTRUCTOR) {
          char *name = "";
          INITO_IDX save_aggregate_inito = aggregate_inito;
          INITV_IDX save_last_aggregate_initv = last_aggregate_initv;
          TY_IDX ty_idx = Get_TY(TREE_TYPE(init));
          ST *temp = New_ST (GLOBAL_SYMTAB);
          ST_Init (temp, 0, CLASS_VAR, SCLASS_FSTATIC, EXPORT_LOCAL, ty_idx);
          Set_ST_is_initialized(temp);
          aggregate_inito = New_INITO (temp);
          
          not_at_root = FALSE;
          last_aggregate_initv = 0;

          Traverse_Aggregate_Constructor (temp, init, TREE_TYPE(init),
                                          TRUE /*gen_initv*/, 0, 0, 0);

          not_at_root = FALSE;
          aggregate_inito = save_aggregate_inito;
          last_aggregate_initv = save_last_aggregate_initv;

          WFE_Add_Aggregate_Init_Symbol (temp);
          break;
        }
      }
    }

#endif
  default:
	{
		WN *init_wn = WFE_Expand_Expr (init);
		FmtAssert (WN_operator (init_wn) == OPR_LDA,
				("expected operator encountered"));
		WFE_Add_Aggregate_Init_Symbol (WN_st (init_wn),
					       WN_offset (init_wn));
		WN_Delete (init_wn);
	}
      	break;
  }
} /* WFE_Add_Aggregate_Init_Address */

void
WFE_Finish_Aggregate_Init (void)
{
  if (aggregate_inito == 0) return;
  ST *st = INITO_st(aggregate_inito);
  TY_IDX ty = ST_type(st);
  if (TY_size(ty) == 0 ||
      (TY_kind(ty) == KIND_ARRAY &&
       !ARB_const_ubnd (TY_arb(ty)) &&
       TY_size(ty) <= Get_INITO_Size(aggregate_inito))) {
	// e.g. array whose size is determined by init;
	// fill in with initv size
#ifdef TARG_ST
    // [CG] For multidim arrays, we must use the stride var.
    Set_TY_size(ty, Get_INITO_Size(aggregate_inito));
    // (chr) /pro-release-1-5-0-B/25,26 only stride if type as as size 
    if (TY_kind(ty) == KIND_ARRAY && TY_size(ty)) {
      int dim = ARB_dimension(TY_arb(ty));
      Set_ARB_const_ubnd (TY_arb(ty)[dim-1]);
      Set_ARB_ubnd_val (TY_arb(ty)[dim-1], 
			(TY_size(ty) / ARB_stride_val(TY_arb(ty)[dim-1])) - 1 );
    }
#else
	Set_TY_size(ty, Get_INITO_Size(aggregate_inito));
	if (TY_kind(ty) == KIND_ARRAY) {
		Set_ARB_const_ubnd (TY_arb(ty));
		Set_ARB_ubnd_val (TY_arb(ty), 
			(TY_size(ty) / TY_size(TY_etype(ty))) - 1 );
	}
#endif
  }
  if (last_aggregate_initv == 0) {
    WFE_Add_Aggregate_Init_Padding (0);
  }
  aggregate_inito = 0;
  not_at_root = FALSE;
}


static BOOL
Has_Non_Constant_Init_Value (tree init)
{
  if (init == NULL) {
	return FALSE;
  }
  switch (TREE_CODE(init)) {
  case CONSTRUCTOR:
	return Has_Non_Constant_Init_Value (CONSTRUCTOR_ELTS(init));
  case TREE_LIST:
	{
	tree p;
	for (p = init; p != NULL; p = TREE_CHAIN(p)) {
		if (Has_Non_Constant_Init_Value (TREE_VALUE(p))) {
			return TRUE;
		}
/*
		if (TREE_CODE(TREE_PURPOSE(p)) == FIELD_DECL
			&& DECL_BIT_FIELD(TREE_PURPOSE(p)))
		{
			// if bitfield, then do each element separately
			// rather than combine into initv field.
			return TRUE;
		}
*/
	}
	return FALSE;
	}
  case INTEGER_CST:
  case REAL_CST:
  case STRING_CST:
	return FALSE;
  case NOP_EXPR:
	if (TREE_CODE(TREE_OPERAND(init,0)) == ADDR_EXPR
    	    && TREE_CODE(TREE_OPERAND(TREE_OPERAND(init,0),0)) == STRING_CST) 
		return FALSE;
	else
		return TRUE;
  default:
	return TRUE;
  }
}

// For a dynamic initialization, we can either
// do a series of moves for each element,
// or we can create a static INITO and structure copy that value.
// GCC allows non-constant initial values, 
// so if any of those exist, we need to assign each element.
// Also, if the init is small we can optimize better
// if we make each element assignment be explicit.
// But otherwise, we create the static INITO since that saves code space.
static BOOL
Use_Static_Init_For_Aggregate (ST *st, tree init)
{
#ifdef TARG_ST
  /* (cbr) fix for DDTS 23379 */
	if (CURRENT_SYMTAB == GLOBAL_SYMTAB)
	  return TRUE;
#endif
	if (TY_size(ST_type(st)) <= (2*MTYPE_byte_size(Spill_Int_Mtype))) {
		return FALSE;
	}
	else if (Has_Non_Constant_Init_Value(init)) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

#ifdef TARG_ST
static WN *
skip_tas (WN *wn_tree)
{
  while (WN_operator (wn_tree) == OPR_TAS) {
    wn_tree = WN_kid0 (wn_tree);
  }
  return wn_tree;
}
#endif

static void
Add_Initv_For_Tree (tree val, UINT size)
{
	WN *init_block;
	WN * init_wn;

#ifdef WFE_DEBUG
  fprintf(stdout, "=============== Add_Initv_For_Tree ==============\n");
  fprintf(stdout, "size: %d, val tree:\n", size);
  print_tree(stdout, val);
#endif

	switch (TREE_CODE(val)) {
	case INTEGER_CST:
		WFE_Add_Aggregate_Init_Integer (
			Get_Integer_Value(val), size);
		break;
	case REAL_CST:
		WFE_Add_Aggregate_Init_Real (
			TREE_REAL_CST(val), size);
		break;
	case STRING_CST:
		WFE_Add_Aggregate_Init_String (
			TREE_STRING_POINTER(val), size);
		break;
#if 0
	case PLUS_EXPR:
		if ( TREE_CODE(TREE_OPERAND(val,0)) == ADDR_EXPR
		     && TREE_CODE(TREE_OPERAND(val,1)) == INTEGER_CST)
		{
			tree addr_kid = TREE_OPERAND(TREE_OPERAND(val,0),0);
			FmtAssert(TREE_CODE(addr_kid) == VAR_DECL
				  || TREE_CODE(addr_kid) == FUNCTION_DECL,
				("expected decl under plus_expr"));
			WFE_Add_Aggregate_Init_Symbol ( Get_ST (addr_kid),
			Get_Integer_Value(TREE_OPERAND(val,1)) );
		}
		else
			FmtAssert(FALSE, ("unexpected tree code %s", 
				tree_code_name[TREE_CODE(val)]));
		break;
#endif
	case NOP_EXPR:
		tree kid;
		kid = TREE_OPERAND(val,0);
		if (TREE_CODE(kid) == ADDR_EXPR
	    		&& TREE_CODE(TREE_OPERAND(kid,0)) == STRING_CST) 
		{
			kid = TREE_OPERAND(kid,0);
			WFE_Add_Aggregate_Init_Address (kid);
			break;
		}
#ifdef TARG_ST
                /* (cbr) first implementation. pro-release-1-7-0-B/11) */
                /* couldn't we factorize with Traverse_Aggregate_Struct ? */
                else if (TREE_CODE (kid) == ADDR_EXPR
                         && TREE_CODE(TREE_OPERAND(kid,0)) == (enum tree_code)COMPOUND_LITERAL_EXPR) 
                  {
                    tree comp = TREE_OPERAND(kid,0);
                    tree decl = COMPOUND_LITERAL_EXPR_DECL (comp);
                    if (TREE_CODE (decl) == VAR_DECL) {
                      tree val = DECL_INITIAL(decl);
                      if (val && TREE_CODE (val) == INTEGER_CST) {
                        WFE_Add_Aggregate_Init_Integer (Get_Integer_Value (val), Pointer_Size);
                        break;
                      }
                      else if (val && TREE_CODE (val) == CONSTRUCTOR) {
                        TY_IDX ty = Get_TY(TREE_TYPE (val));
                        if (TY_kind (ty) == KIND_STRUCT) {
                          tree init;
                          for (init = CONSTRUCTOR_ELTS(val);
                               init;
                               init = TREE_CHAIN(init)) {
                            if (TREE_PURPOSE(init) && TREE_CODE(TREE_PURPOSE(init)) == FIELD_DECL) {
                              tree val = TREE_VALUE(init);
                              TY_IDX fld_ty = Get_TY(TREE_TYPE(val));
                              Add_Initv_For_Tree (val, TY_size(fld_ty));
                            }
                          }
                        }
                        break;
                      }
                    }
                  }
#endif
		// fallthru
	default:
		{
		init_block = WN_CreateBlock ();
                WFE_Stmt_Push (init_block, wfe_stmk_func_body, Get_Srcpos());
		init_wn = WFE_Expand_Expr (val);
#ifdef TARG_ST
                // Skip OPR_TAS since it does not change the representation.
                init_wn = skip_tas (init_wn);
#endif
                WFE_Stmt_Pop (wfe_stmk_func_body);

		if ((WN_opcode (init_wn) == OPC_I4U4CVT &&
		     WN_opcode (WN_kid0 (init_wn)) == OPC_U4LDA) ||
		    (WN_opcode (init_wn) == OPC_I8U8CVT &&
		     WN_opcode (WN_kid0 (init_wn)) == OPC_U8LDA)) {
			WFE_Add_Aggregate_Init_Symbol (WN_st (WN_kid0 (init_wn)),
						       WN_offset (WN_kid0 (init_wn)));
			WN_DELETE_Tree (init_wn);
			break;
		}

		if (WN_operator (init_wn) == OPR_LDA) {
			WFE_Add_Aggregate_Init_Symbol (WN_st (init_wn),
						       WN_offset (init_wn));
			WN_DELETE_Tree (init_wn);
			break;
		}
		else if (WN_operator(init_wn) == OPR_INTCONST) {
			WFE_Add_Aggregate_Init_Integer (
				WN_const_val(init_wn), size);
			break;
		}
		// following cases for ADD and SUB are needed because the
		// simplifier may be unable to fold due to overflow in the
		// 32-bit offset field
		else if (WN_operator(init_wn) == OPR_ADD) {
			WN *kid0 = WN_kid0(init_wn);
			WN *kid1 = WN_kid1(init_wn);

#ifdef TARG_ST
                        // Skip OPR_TAS since it does not change the representation.            
                        kid0 = skip_tas(kid0);
                        kid1 = skip_tas(kid1);
#endif

		 	if (WN_operator(kid0) == OPR_LDA &&
			    WN_operator(kid1) == OPR_INTCONST) {
			  WFE_Add_Aggregate_Init_Symbol (WN_st (kid0),
				     WN_offset(kid0) + WN_const_val(kid1));
			  WN_DELETE_Tree (init_wn);
			  break;
			}
		 	else if (WN_operator(kid1) == OPR_LDA &&
			    WN_operator(kid0) == OPR_INTCONST) {
			  WFE_Add_Aggregate_Init_Symbol (WN_st (kid1),
				     WN_offset(kid1) + WN_const_val(kid0));
			  WN_DELETE_Tree (init_wn);
			  break;
			}
		}
		else if (WN_operator(init_wn) == OPR_SUB) {
			WN *kid0 = WN_kid0(init_wn);
			WN *kid1 = WN_kid1(init_wn);
#ifdef TARG_ST
                        // Skip OPR_TAS since it does not change the representation.            
                        kid0 = skip_tas(kid0);
                        kid1 = skip_tas(kid1);
#endif
		 	if (WN_operator(kid0) == OPR_LDA &&
			    WN_operator(kid1) == OPR_INTCONST) {
			  WFE_Add_Aggregate_Init_Symbol (WN_st (kid0),
				     WN_offset(kid0) - WN_const_val(kid1));
			  WN_DELETE_Tree (init_wn);
			  break;
                        }
#if defined (TARG_ST) && (GNU_FRONT_END==33)
                        /* (cbr) DDTSst24451. add support for label diffs initializers */
		 	if (WN_operator(kid0) == OPR_LDA_LABEL &&
			    WN_operator(kid1) == OPR_LDA_LABEL) {

                          if (TREE_CODE (val) == NOP_EXPR)
                            val = TREE_OPERAND(val, 0);

                          tree arg = TREE_OPERAND(val,0);
                          tree label0 = NULL;
                          tree label1 = NULL;

                          if (arg && TREE_CODE (arg)== CONVERT_EXPR) {
                            arg = TREE_OPERAND(arg,0);
                            if (arg && TREE_CODE (arg)== ADDR_EXPR) {
                              label0 = TREE_OPERAND(arg,0);
                            }
                          }
                              
                          arg = TREE_OPERAND(val,1);

                          if (arg && TREE_CODE (arg)== CONVERT_EXPR) {
                            arg = TREE_OPERAND(arg,0);
                            if (arg && TREE_CODE (arg)== ADDR_EXPR) {
                              label1 = TREE_OPERAND(arg,0);
                            }
                          }

                          if (label0 && label1) {
                            LABEL_IDX label_idx0 = WFE_Get_LABEL (label0, FALSE);
                            LABEL_IDX label_idx1 = WFE_Get_LABEL (label1, FALSE);

                            WFE_Add_Aggregate_Init_Labeldiff (label_idx0, label_idx1);
                            WN_DELETE_Tree (init_wn);
                            break;
                          }
                        }
#endif
		}
#if defined (TARG_ST) && (GNU_FRONT_END==33)
                /* (cbr) add support for label initializers */
		else if (WN_operator(init_wn) == OPR_LDA_LABEL) {
                  tree label;
                  if (TREE_CODE (val) == NOP_EXPR) {
                    val = TREE_OPERAND(val, 0);
                  }
                  if (TREE_CODE (val) == ADDR_EXPR) 
                    label = TREE_OPERAND(val,0);
                  else
                    FmtAssert(FALSE, ("unexpected tree code %s", 
                                  tree_code_name[TREE_CODE(val)]));                    
                  LABEL_IDX label_idx = WFE_Get_LABEL (label, FALSE);
                  WFE_Add_Aggregate_Init_Label (label_idx);
                  WN_DELETE_Tree (init_wn);
                  break;
                }
#endif
		FmtAssert(FALSE, ("unexpected tree code %s", 
			tree_code_name[TREE_CODE(val)]));
		}
	}
}

// buffer for simulating the initialized memory unit; it is managed independent
// of host's endianness
class INITBUF { 
public:
  UINT64 ival;

  INITBUF(void) {}
  INITBUF(UINT64 i): ival(i) {}
  ~INITBUF(void) {}
  mUINT8 Nth_byte(INT i) { // i must be from 0 to 7
		      INT rshft_amt = (Target_Byte_Sex == BIG_ENDIAN) ? 7-i : i;
		      return (ival >> (rshft_amt * 8)) & 0xff;
		    }
};

// at entry, assumes that in the current struct, initv for "bytes" bytes have 
// been generated; at exit, "bytes" will be updated with the additional
// bytes that this invocation generates.
static void
Add_Bitfield_Initv_For_Tree (tree val, FLD_HANDLE fld, INT &bytes)
{
  INT bofst = FLD_bofst(fld);
  INT bsize = FLD_bsize(fld);
  if (bsize == 0)
    return;

#ifdef TARG_ST
  /* (cbr) handle conversion initializer with labels */
  if (TREE_CODE (val) == CONVERT_EXPR &&
      TREE_CODE (TREE_TYPE (val)) == INTEGER_TYPE){ 

    WN *init_wn = WFE_Expand_Expr (val);
#ifdef TARG_ST
  // Skip OPR_TAS since it does not change the representation.
    init_wn = skip_tas (init_wn);
#endif
    FmtAssert (WN_operator (init_wn) == OPR_LDA,
               ("expected operator encountered"));

    if (bsize/8 == 2) {
      INITV_Init_Symoff (last_aggregate_initv, WN_st(init_wn), WN_offset(init_wn), 1, 1);
      INT num_of_bytes = ((bofst + bsize - 1) >> 3) + 1;
      INT bytes_out = bytes - FLD_ofst(fld);
      bytes += num_of_bytes - bytes_out;
      return; 
    }
    else {
      error ("invalid initial value for member `%s'", ST_name (WN_st (init_wn)));
      return;
    }
  }
#endif

  FmtAssert(TREE_CODE(val) == INTEGER_CST,
	    ("initialization value of bitfield expected to be integer, not %s",
	     tree_code_name[TREE_CODE(val)]));

  INITBUF ib(Get_Integer_Value(val));
  // truncate ival according to the bitfield size and leave it left-justified
  ib.ival = ib.ival << (64 - bsize);
  // shift the value back right to the precise position within INITBUF
  if (Target_Byte_Sex == BIG_ENDIAN) 
    ib.ival = ib.ival >> bofst;
  else ib.ival = ib.ival >> (64 - bofst - bsize);

  // find number of bytes to output
  INT num_of_bytes = ((bofst + bsize - 1) >> 3) + 1;
  // find number of bytes that have been output with previous bitfields
  INT bytes_out = bytes - FLD_ofst(fld);
  INT i;
  if (bytes_out > 0) {
    // verify that, other than the last output byte, the earlier bytes in 
    // ib are all 0
    for (i = 0; i < bytes_out - 1; i++)
      FmtAssert(ib.Nth_byte(i) == 0, 
		("processing error in Add_Bitfield_Initv_For_Tree"));
    if (ib.Nth_byte(bytes_out-1) != 0) {// merge and change last_aggregate_initv
      if (INITV_kind(last_aggregate_initv) == INITVKIND_VAL) {
        TCON &tc = INITV_tc_val(last_aggregate_initv);
        mUINT8 last_ival = TCON_k0(tc);
        tc.vals.k0 = last_ival | ib.Nth_byte(bytes_out-1);
      }
      else { // need to create a new TCON
        if (INITV_kind(last_aggregate_initv) == INITVKIND_ONE) 
	  INITV_Init_Integer(last_aggregate_initv, MTYPE_I1, 
			     1 | ib.Nth_byte(bytes_out-1));
	else {
	  FmtAssert(INITV_kind(last_aggregate_initv) == INITVKIND_ZERO,
		    ("processing error in static bit field initialization"));
	  INITV_Init_Integer(last_aggregate_initv, MTYPE_I1, 
			     ib.Nth_byte(bytes_out-1));
	}
      }
    }
  }
  // output the remaining bytes
  for (i = bytes_out; i < num_of_bytes; i++)
    WFE_Add_Aggregate_Init_Integer(ib.Nth_byte(i), 1);
  bytes += num_of_bytes - bytes_out;
}

// "bytes" will be updated with the additional bytes that this invocation
// generates stores into
static void
Gen_Assign_Of_Init_Val (ST *st, tree init, UINT offset, UINT array_elem_offset,
	TY_IDX ty, BOOL is_bit_field, UINT field_id, FLD_HANDLE fld, INT &bytes)
{
    WN *init_wn = WFE_Expand_Expr (init);

#ifdef WFE_DEBUG
  fprintf(stdout, "    Gen_Assign_Of_Init_Val %s:",
	  ST_name(st));
  print_node_brief (stdout, " ", init, 0);
  fprintf(stdout, " -- %s\n", is_bit_field ? "bit_field" : "");
  fdump_tree(stdout, init_wn);
#endif

    if (TREE_CODE(init) == STRING_CST && TY_kind(ty) == KIND_ARRAY)
    {
	// have to store string into address,
	// rather than directy copy assignment,
	// so need special code.
#ifdef TARG_ST
      // [CG] Handle overflow and padding with string initialization
      UINT elt_size = TY_size(ty);
      UINT str_size = TREE_STRING_LENGTH(init);
      UINT size = elt_size <= str_size ? elt_size : str_size;
      UINT pad =  elt_size - size;
      TY_IDX ptr_ty = Make_Pointer_Type(ty);
      WN *load_wn = WN_CreateMload (0, ptr_ty, init_wn,
				    WN_Intconst(MTYPE_I4, size));
      WN *addr_wn = WN_Lda(Pointer_Mtype, 0, st);
      WFE_Stmt_Append(
		      WN_CreateMstore (offset, ptr_ty,
				       load_wn,
				       addr_wn,
				       WN_Intconst(MTYPE_I4,size)),
		      Get_Srcpos());
      if (pad > 0) {
	load_wn = WN_Intconst(MTYPE_U4, 0);
	addr_wn = WN_Lda(Pointer_Mtype, 0, st);
	WFE_Stmt_Append(
			WN_CreateMstore (offset+size, ptr_ty,
					 load_wn,
					 addr_wn,
					 WN_Intconst(MTYPE_I4,pad)),
			Get_Srcpos());
	}
      bytes += elt_size;
#else
	UINT size = TY_size(ty);
	TY_IDX ptr_ty = Make_Pointer_Type(ty);
	WN *load_wn = WN_CreateMload (0, ptr_ty, init_wn,
				      WN_Intconst(MTYPE_I4, size));
	WN *addr_wn = WN_Lda(Pointer_Mtype, 0, st);
	WFE_Stmt_Append(
		WN_CreateMstore (offset, ptr_ty,
				 load_wn,
				 addr_wn,
				 WN_Intconst(MTYPE_I4,size)),
		Get_Srcpos());
	bytes += size;
#endif
    }
    else {
	TYPE_ID mtype = is_bit_field ? MTYPE_BS : TY_mtype(ty);
	if (is_bit_field) { 
	    offset = array_elem_offset;	// uses array element offset instead
	} else
	    field_id = 0;	// uses offset instead
	WFE_Set_ST_Addr_Saved (init_wn);
	WN *wn = WN_Stid (mtype, ST_ofst(st) + offset, st,
		ty, init_wn, field_id);
	WFE_Stmt_Append(wn, Get_Srcpos());
	if (! is_bit_field) 
	  bytes += TY_size(ty);
	else {
	  INT bofst = FLD_bofst(fld);
	  INT bsize = FLD_bsize(fld);
	  // find number of bytes to output
	  INT num_of_bytes = ((bofst + bsize - 1) >> 3) + 1;
	  // find number of bytes that have been output with previous bitfields
	  INT bytes_out = bytes - FLD_ofst(fld);
	  bytes += num_of_bytes - bytes_out;
	}
    }
    // [CL]
    Clear_Callsite_Pragma_List(WARN);
}


UINT
Traverse_Aggregate_Struct (
  ST   *st, tree init_list, tree type, BOOL gen_initv,
  UINT current_offset, UINT array_elem_offset, UINT field_id);

// For the specified symbol, generate padding at the offset specified.
// If gen_initv is TRUE build an initv, otherwise generate a sequence
// of stores.

void
Traverse_Aggregate_Pad (
  ST     *st,
  BOOL   gen_initv,
  UINT   pad,
  UINT   current_offset)
{
  if (gen_initv) {
     WFE_Add_Aggregate_Init_Padding (pad);
  }
  else {
    WN *zero_wn = WN_Intconst(MTYPE_U4, 0);
    WN *pad_wn = WN_Intconst(MTYPE_U4, pad);
    WN *addr_wn = WN_Lda(Pointer_Mtype, 0, st);
    TY_IDX mstore_ty = Make_Pointer_Type(MTYPE_To_TY(MTYPE_U1)); // char *
    WFE_Stmt_Append (WN_CreateMstore (current_offset, mstore_ty,
                                      zero_wn, addr_wn, pad_wn),
                     Get_Srcpos());
  }
} /* Traverse_Aggregate_Pad */

// The aggregate element for the specified symbol at the current_offset
// is an array having the gcc tree type 'type'.
// If gen_initv is TRUE build an initv, otherwise generate a sequence
// of stores.

void
Traverse_Aggregate_Array (
  ST   *st,            // symbol being initialized
  tree init_list,      // list of initializers for each array element
  tree type,           // type of array
  BOOL gen_initv,      // TRUE if initializing with INITV, FALSE for statements
  UINT current_offset) // offset of array from start of symbol
{
  INT    emitted_bytes = 0;
  INT    pad;
  TY_IDX ty            = Get_TY(type);
  TY_IDX ety           = TY_etype (ty);
  UINT   esize         = TY_size (ety);
  tree   init;

  for (init = CONSTRUCTOR_ELTS(init_list);
       init;
       init = TREE_CHAIN(init)) {
    // loop through each array element
    if (TREE_CODE(TREE_VALUE (init)) == CONSTRUCTOR) {
      // recursively process nested ARRAYs and STRUCTs
      // update array_elem_offset to current_offset to
      // keep track of where each array element starts
#ifdef TARG_ST
        /* (cbr) handle indexed initialization */
        if (TREE_PURPOSE(init)) {
          INT pos = Get_Integer_Value(TREE_PURPOSE(init))*TY_size(Get_TY(TREE_TYPE(type)));
	  // FdF 20050916: changed to take into account fix for bug 180B/22
          if (pos > emitted_bytes) {
            pad = pos - emitted_bytes;
            Traverse_Aggregate_Pad (st, gen_initv, pad, current_offset);
            emitted_bytes += pad;
            current_offset += pad;
          }
        }
#endif

      Traverse_Aggregate_Constructor (st, TREE_VALUE(init), TREE_TYPE(type),
                                      gen_initv, current_offset, current_offset,
                                      0);

      emitted_bytes += TY_size(Get_TY(TREE_TYPE(TREE_VALUE(init))));
      current_offset += TY_size(Get_TY(TREE_TYPE(TREE_VALUE(init))));
    }
 
#ifdef TARG_ST
    // [CG] Treat special case of string constant.
    // In this case we must handle the string size and the required padding
    else if (TREE_CODE (TREE_VALUE (init)) == STRING_CST) {
      if (gen_initv) {
	UINT elt_size, size, str_size, pad;
	elt_size = Get_Integer_Value(TYPE_SIZE(TREE_TYPE(TREE_VALUE(init))))/BITSPERBYTE;
	str_size = TREE_STRING_LENGTH(TREE_VALUE(init));
	size = elt_size <= str_size ? elt_size : str_size;
	pad = elt_size - size;
	Add_Initv_For_Tree (TREE_VALUE(init), size);
	if (pad > 0)
	  WFE_Add_Aggregate_Init_Padding (pad);
	emitted_bytes += elt_size;
	current_offset += elt_size;
      } else {
	// For this case we pass the array type instead of the element type
	// such that the initialization is seen as an array initialization.
	TY_IDX array_ty = Get_TY(TREE_TYPE(TREE_VALUE(init)));
        Gen_Assign_Of_Init_Val (st, TREE_VALUE(init), current_offset, 0,
                                array_ty, FALSE, 0, FLD_HANDLE (), emitted_bytes);
	current_offset += TY_size(array_ty);
      }
    }
#endif

    else {
      // initialize SCALARs and POINTERs
      // note that we should not be encountering bit fields
      if (gen_initv) {
#ifdef TARG_ST
        /* (cbr) handle indexed initialization */
        if (TREE_PURPOSE(init)) {
          INT pos = Get_Integer_Value(TREE_PURPOSE(init))*TY_size(Get_TY(TREE_TYPE(type)));
	  // FdF 20050916: changed to take into account fix for bug 180B/22
          if (pos > emitted_bytes) {
            pad = pos - emitted_bytes;
            Traverse_Aggregate_Pad (st, gen_initv, pad, current_offset);
            emitted_bytes += pad;
            current_offset += pad;
          }
        }
#endif

        Add_Initv_For_Tree (TREE_VALUE(init), esize);
        emitted_bytes += esize;
      }
      else
        Gen_Assign_Of_Init_Val (st, TREE_VALUE(init), current_offset, 0,
                                ety, FALSE, 0, FLD_HANDLE (), emitted_bytes);

      current_offset += esize;
    }
  }

  // If the entire array has not been initialized, pad till the end
  pad = TY_size (ty) - emitted_bytes;

  if (pad > 0)
    Traverse_Aggregate_Pad (st, gen_initv, pad, current_offset);

  return;
} /* Traverse_Aggregate_Array */

// The aggregate element for the specified symbol at the current_offset
// is a struct/class/union having the gcc tree type 'type'.
// If gen_initv is TRUE build an initv, otherwise generate a sequence
// of stores.
// It accepts the field_id of the struct, and returns the field_id
// of the last element in the struct if it has elements, otherwise
// it returns the field_id passed in for empty structs

UINT
Traverse_Aggregate_Struct (
  ST   *st,               // symbol being initialized
  tree init_list,         // list of initializers for elements in STRUCT
  tree type,              // type of struct
  BOOL gen_initv,         // TRUE if initializing with INITV, FALSE for statements
  UINT current_offset,    // offset from start of symbol for current struct
  UINT array_elem_offset, // if struct is with an array, then it is the
                          //   offset of the outermost struct from the
                          //   array enclosing the struct
                          // if struct is not within an array, it is zero
                          // this is needed when field_id is used to generate
                          //   stores for initialization
  UINT field_id)          // field_id of struct
{
  TY_IDX     ty    = Get_TY(type);       // get WHIRL type
  tree       field = TYPE_FIELDS(type);  // get first field in gcc
  FLD_HANDLE fld   = TY_fld (ty);        // get first field in WHIRL

  INT        emitted_bytes = 0;          // keep track of # of bytes initialize;
  INT        current_offset_base = current_offset;
  INT        pad;
  BOOL       is_bit_field;
  tree       init;
  TY_IDX     fld_ty;

#ifdef WFE_DEBUG
  fprintf(stdout, "=========== Traverse_Aggregate_Struct %s ==========\n",
	  ST_name(st));
  print_node_brief (stdout, "  init_list: ", init_list, 0);
  fprintf(stdout, "\n");
  print_node_brief (stdout, "  tree type: ", type, 0);
  fprintf(stdout, "\n%s\n", gen_initv ? "  -- gen_initv" : "  -- nada");
  fprintf(stdout, "  current_offset = %d\n", current_offset);
#endif

  for (init = CONSTRUCTOR_ELTS(init_list);
       init;
       init = TREE_CHAIN(init)) {
    // loop through each initializer specified

    ++field_id; // compute field_id for current field

#ifdef WFE_DEBUG
    fprintf(stdout, "    field_id %d --> ", field_id);
    print_node_brief (stdout, " init: ", init, 0);
    fprintf(stdout, "\n");
#endif

#if 0
    // (cbr) wrong. (DDTS 2270, 22544)
#ifdef TARG_ST
    // (cbr) skip empty initialization aggregate
    tree stype = TREE_TYPE (TREE_VALUE(init));
    if (AGGREGATE_TYPE_P(stype) && (!TYPE_SIZE(stype) || !Get_Integer_Value (TYPE_SIZE (stype)))) {
      continue;
    }
#endif
#endif

    // if the initialization is not for the current field,
    // advance the fields till we find it
    if (field && TREE_PURPOSE(init) && TREE_CODE(TREE_PURPOSE(init)) == FIELD_DECL) {
#ifndef TARG_ST
      // Arthur: seems to work allright
      DevWarn ("Encountered FIELD_DECL during initialization");
#endif
      for (;;) {
        if (field == TREE_PURPOSE(init)) {
          break;
        }
        ++field_id;
        field = TREE_CHAIN(field);
        fld = FLD_next(fld);
      }
    }

    // check if we need to pad upto the offset of the field
    pad = FLD_ofst (fld) - emitted_bytes;

    if (pad > 0) {
      Traverse_Aggregate_Pad (st, gen_initv, pad, current_offset);
      current_offset += pad;
      emitted_bytes  += pad;
    }

    fld_ty = FLD_type(fld);
    if (TREE_CODE(TREE_VALUE(init)) == CONSTRUCTOR) {
      // recursively process nested ARRAYs and STRUCTs
      tree element_type;
      element_type = TREE_TYPE(field);
      field_id = Traverse_Aggregate_Constructor (st, TREE_VALUE(init),
                                                 element_type, gen_initv,
                                                 current_offset,
#ifdef TARG_ST
   // Arthur: looks like a bug, start traversing a struct from field_id 1
                                                 array_elem_offset, 0);
#else
                                                 array_elem_offset, field_id);
#endif
      emitted_bytes += TY_size(fld_ty);
    }
    else {
      // initialize SCALARs and POINTERs
      is_bit_field = FLD_is_bit_field(fld);
      if (gen_initv) {
        if (! is_bit_field) {
          Add_Initv_For_Tree (TREE_VALUE(init), TY_size(fld_ty));
          emitted_bytes += TY_size(fld_ty);
        }
        else { // do 1 byte a time
          Add_Bitfield_Initv_For_Tree (TREE_VALUE(init), fld, emitted_bytes);
          // emitted_bytes updated by the call as reference parameter
        }
      }
      else {
        Gen_Assign_Of_Init_Val (st, TREE_VALUE(init),
                                current_offset, array_elem_offset,
                                is_bit_field ? ty : fld_ty,
                                is_bit_field, field_id, fld, emitted_bytes);
        // emitted_bytes updated by the call as reference parameter
      }
    }

    // advance ot next field
    current_offset = current_offset_base + emitted_bytes;
    field = TREE_CHAIN(field);
    fld = FLD_next(fld);
  }

  // if not all fields have been initialized, then loop through
  // the remaining fields to update field_id
  // Also check to see if any bit fields need to be initialized 
  // to zero to handle the case where the bit field shares the
  // same byte as last bit field which was initialized.
  while ( ! fld.Is_Null()) {
    ++field_id;
    if (!gen_initv && FLD_is_bit_field(fld)) {
#ifdef TARG_ST
      // [CG 2004/11/22] The original code is wrong, we must initialize
      // to zero bitfields that start before the current offset.
      // The current bitfiled may also span multiple bytes.
      // In this case it is initialized fully even if the trailling
      // bytes will also be initialized by the byte padding below.

      // find number of bytes that have been output with previous bitfields
      INT bytes_out = current_offset - FLD_ofst(fld);
      if (bytes_out > 0) {
	TY_IDX fld_ty = FLD_type(fld);
	WN *init_wn = WN_Intconst (TY_mtype (fld_ty), 0);
	WN *wn = WN_Stid (MTYPE_BS, ST_ofst(st) + array_elem_offset, st,
			  ty, init_wn, field_id);
	WFE_Stmt_Append(wn, Get_Srcpos());
      }
#else
      INT bofst = FLD_bofst(fld);
      INT bsize = FLD_bsize(fld);
      // find number of bytes to output
      INT num_of_bytes = ((bofst + bsize - 1) >> 3) + 1;
      // find number of bytes that have been output with previous bitfields
      INT bytes_out = current_offset - FLD_ofst(fld);
      if (num_of_bytes == bytes_out) {
	TY_IDX fld_ty = FLD_type(fld);
	WN *init_wn = WN_Intconst (TY_mtype (fld_ty), 0);
	WN *wn = WN_Stid (MTYPE_BS, ST_ofst(st) + array_elem_offset, st,
			  ty, init_wn, field_id);
	WFE_Stmt_Append(wn, Get_Srcpos());
      }
#endif
    }
    field = TREE_CHAIN(field);
    fld = FLD_next(fld);
  }

  // if not all fields have been initilaized, then check if
  // padding is needed to the end of struct
  pad = TY_size (ty) - emitted_bytes;

  if (pad > 0)
    Traverse_Aggregate_Pad (st, gen_initv, pad, current_offset);

  return field_id;
} /* Traverse_Aggregate_Struct */

// The aggregate element for the specified symbol at the current_offset
// is either an array or  struct/class/union having the gcc tree type 'type'.
// If gen_initv is TRUE build an initv, otherwise generate a sequence
// of stores.
// It accepts the field_id of the element in the enclosing struct
// used for computing field_ids (0 if no such struct exists)
// If the aggregate element is non-array, it returns the field_id of 
// last field within the aggregate element.
// If the aggregate element is array, then it returns the field_id passed in
UINT
Traverse_Aggregate_Constructor (
  ST   *st,               // symbol being initialized
  tree init_list,         // list of initilaizers for this aggregate
  tree type,              // type of aggregate being initialized
  BOOL gen_initv,         // TRUE  if initializing with INITV,
                          // FALSE if initializing with statements
  UINT current_offset,    // offset from start of symbol for this aggregate
  UINT array_elem_offset,
  UINT field_id)
{
  TY_IDX ty = Get_TY(type);

#ifdef TARG_ST
  INITV_IDX block_body_initv;

  /* (cbr) if forward declaration reset the proper ty */
  if (ST_type (st) && ST_type(st) != ty && !TY_size(ST_type(st))) {
    Set_ST_type (st, ty);
  }
#endif    

  INITV_IDX last_aggregate_initv_save;

#ifdef WFE_DEBUG
  fprintf(stdout, "========= Traverse_Aggregate_Constructor %s ========\n",
	  ST_name(st));
  print_node_brief (stdout, "  init_list: ", init_list, 0);
  fprintf(stdout, "\n");
  print_node_brief (stdout, "  tree type: ", type, 0);
  fprintf(stdout, "\n%s\n", gen_initv ? "  -- gen_initv" : "  -- nada");
#endif

  if (gen_initv) {

    WFE_Add_Init_Block();
    INITV_Init_Block(last_aggregate_initv, INITV_Next_Idx());
#ifdef TARG_ST
    block_body_initv = INITV_Next_Idx ();
#endif
    not_at_root = TRUE;
    last_aggregate_initv_save = last_aggregate_initv;
    last_aggregate_initv = 0;
  }

  if (TY_kind (ty) == KIND_STRUCT) {

    field_id = Traverse_Aggregate_Struct (st, init_list, type, gen_initv,
                                          current_offset, array_elem_offset,
                                          field_id);
  }

  else
  if (TY_kind (ty) == KIND_ARRAY) {

// FdF 20050916: Fix for bug 180B/22 requires calling with current_offset. 
#if 0//def TARG_ST
/* (cbr) element positions are given relatively to the start of the array,
   not of the symbol */
    Traverse_Aggregate_Array (st, init_list, type, gen_initv, 0);
#else
    Traverse_Aggregate_Array (st, init_list, type, gen_initv, current_offset);
#endif
  }

  else
    Fail_FmtAssertion ("Traverse_Aggregate_Constructor: non STRUCT/ARRAY");

#ifdef TARG_ST
/* (cbr) ddts 24482 can have 0 size and initializer */
/* [SC] Rework the test for whether we actually created the block we
   inserted a reference to above.  Note: we can have a zero-sized struct
   with several (zero-sized) fields, so field_id == 0 is not a good test
   here.  Similarly, we can have a multi-dimensional array such that we
   get here with last_aggregate_initv == 0 even if we generated initv
   records. */
  if (block_body_initv != 0 && block_body_initv == INITV_Next_Idx ()) 
    // Overwrite the reference, since it turned out we did not create
    // any initializers for the block.
    INITV_Init_Block(last_aggregate_initv_save, INITV_IDX_ZERO);
#endif

  // restore current level's last_aggregate_initv and return
  last_aggregate_initv = last_aggregate_initv_save;

  return field_id;
} /* Traverse_Aggregate_Constructor */


static void
Add_Inito_For_Tree (tree init, tree decl, ST *st)
{
  tree kid;

#ifdef WFE_DEBUG
  fprintf(stdout, "=============== Add_Inito_For_Tree ==============\n");
  print_tree(stdout, decl);
#endif

  last_aggregate_initv = 0;
  switch (TREE_CODE(init)) {
  case INTEGER_CST:
	UINT64 val;
	val = Get_Integer_Value (init);
	// if section-attribute, keep as dglobal inito
	if (
#ifdef TARG_ST
            (Zeroinit_in_bss && val == 0)
#else
            (val == 0)
#endif
            && ! DECL_SECTION_NAME (decl)) {
		Set_ST_init_value_zero(st);
		if (ST_sclass(st) == SCLASS_DGLOBAL)
			Set_ST_sclass(st, SCLASS_UGLOBAL);
		return;
	}
	aggregate_inito = New_INITO (st);
	not_at_root = FALSE;
	WFE_Add_Aggregate_Init_Integer (val, TY_size(ST_type(st)));
	return;
  case REAL_CST:
	aggregate_inito = New_INITO (st);
	not_at_root = FALSE;
	WFE_Add_Aggregate_Init_Real (TREE_REAL_CST(init), 
		TY_size(ST_type(st)));
	return;
  case COMPLEX_CST:
	aggregate_inito = New_INITO (st);
	not_at_root = FALSE;
	WFE_Add_Aggregate_Init_Complex (TREE_REAL_CST(TREE_REALPART(init)), 
					TREE_REAL_CST(TREE_IMAGPART(init)), 
					TY_size(ST_type(st)));
	return;
  case STRING_CST:
#ifdef TARG_ST
    {
      // [CG] we must pass the type size if  type size <= string length.
      // We must pass the String size and add padding if type size
      // > string_length
      int size = TY_size (ST_type(st)) <= TREE_STRING_LENGTH(init) ? 
	TY_size (ST_type(st)) : TREE_STRING_LENGTH(init);
      int pad = TY_size (ST_type(st)) - size;
      aggregate_inito = New_INITO (st);
      not_at_root = FALSE;
      WFE_Add_Aggregate_Init_String (TREE_STRING_POINTER(init), 
				     size);
      if (pad > 0)
	WFE_Add_Aggregate_Init_Padding (pad);
    }
#else
	aggregate_inito = New_INITO (st);
	not_at_root = FALSE;
	WFE_Add_Aggregate_Init_String (TREE_STRING_POINTER(init), 
                                       TREE_STRING_LENGTH(init));
	if (TY_size (ST_type(st)) > TREE_STRING_LENGTH(init))
		WFE_Add_Aggregate_Init_Padding ( TY_size (ST_type(st)) -
						 TREE_STRING_LENGTH(init));
#endif
	return;
  case NOP_EXPR:
	Add_Inito_For_Tree (TREE_OPERAND(init,0), decl, st);
	return;
  case ADDR_EXPR:
	kid = TREE_OPERAND(init,0);
#ifdef TARG_ST
	if (TREE_CODE(kid) == VAR_DECL ||
	    TREE_CODE(kid) == FUNCTION_DECL ||
	    TREE_CODE(kid) == STRING_CST ||
	    // [CG]: Add support for initialization with label values
	    TREE_CODE(kid) == LABEL_DECL)
#else
	if (TREE_CODE(kid) == VAR_DECL ||
	    TREE_CODE(kid) == FUNCTION_DECL ||
	    TREE_CODE(kid) == STRING_CST)
#endif
	  {
		aggregate_inito = New_INITO (st);
		not_at_root = FALSE;
		WFE_Add_Aggregate_Init_Address (kid);
		return;
	}
  case PLUS_EXPR:
	kid = TREE_OPERAND(init,0);
	if (TREE_CODE(kid) == ADDR_EXPR) {
		// symbol+offset
		Add_Inito_For_Tree (kid, decl, st);
		kid = TREE_OPERAND(init,1);
		if (INITV_kind(last_aggregate_initv) == INITVKIND_SYMOFF
			&& TREE_CODE(kid) == INTEGER_CST)
		{
			Set_INITV_ofst (last_aggregate_initv,
				Get_Integer_Value(kid));
			return;
		}
	}
	break;
  case MINUS_EXPR:
	kid = TREE_OPERAND(init,0);
	if (TREE_CODE(kid) == ADDR_EXPR) {
		// symbol-offset
		Add_Inito_For_Tree (kid, decl, st);
		kid = TREE_OPERAND(init,1);
		if (INITV_kind(last_aggregate_initv) == INITVKIND_SYMOFF
			&& TREE_CODE(kid) == INTEGER_CST)
		{
			Set_INITV_ofst (last_aggregate_initv,
				-Get_Integer_Value(kid));
			return;
		}
	}
	break;
  case CONSTRUCTOR:
	aggregate_inito = New_INITO (st);
	not_at_root = FALSE;
	last_aggregate_initv = 0;
	Traverse_Aggregate_Constructor (st, init, TREE_TYPE(init),
                                        TRUE /*gen_initv*/, 0, 0, 0);
	return;
  }

  // not recognized, so try to simplify
  WN *init_wn = WFE_Expand_Expr (init);
#ifdef TARG_ST
  // [SC] Skip OPR_TAS since it does not change the representation.
  init_wn = skip_tas (init_wn);
#endif
  if (WN_operator(init_wn) == OPR_INTCONST) {
	aggregate_inito = New_INITO (st);
	not_at_root = FALSE;
	WFE_Add_Aggregate_Init_Integer (
		WN_const_val(init_wn), TY_size(ST_type(st)));
	return;
  }
  else 
  if (WN_operator(init_wn) == OPR_LDA) {
	aggregate_inito = New_INITO (st);
	not_at_root = FALSE;
	WFE_Add_Aggregate_Init_Symbol (WN_st (init_wn), WN_offset (init_wn));
	return;
  }
  else
  if (WN_operator(init_wn) == OPR_ADD) {
#ifdef TARG_ST
    WN *kid0 = skip_tas (WN_kid0(init_wn));
    WN *kid1 = skip_tas (WN_kid1(init_wn));
    if (WN_operator(kid0) == OPR_LDA &&
	WN_operator(kid1) == OPR_INTCONST) {
      aggregate_inito = New_INITO (st);
      not_at_root = FALSE;
      WFE_Add_Aggregate_Init_Symbol (WN_st(kid0),
		WN_offset(kid0) + WN_const_val(kid1));
#else
    if (WN_operator(WN_kid0(init_wn)) == OPR_LDA &&
        WN_operator(WN_kid1(init_wn)) == OPR_INTCONST) {
      aggregate_inito = New_INITO (st);
      not_at_root = FALSE;
      WFE_Add_Aggregate_Init_Symbol (WN_st(WN_kid0(init_wn)),
		WN_offset(WN_kid0(init_wn)) + WN_const_val(WN_kid1(init_wn)));
#endif
      return;
    }
  }
  else
  if (WN_operator(init_wn) == OPR_SUB) {
#ifdef TARG_ST
    WN *kid0 = skip_tas (WN_kid0(init_wn));
    WN *kid1 = skip_tas (WN_kid1(init_wn));
    if (WN_operator(kid0) == OPR_LDA &&
	WN_operator(kid1) == OPR_INTCONST) {
      aggregate_inito = New_INITO (st);
      not_at_root = FALSE;
      WFE_Add_Aggregate_Init_Symbol (WN_st(kid0),
		WN_offset(kid0) - WN_const_val(kid1));
#else
    if (WN_operator(WN_kid0(init_wn)) == OPR_LDA &&
        WN_operator(WN_kid1(init_wn)) == OPR_INTCONST) {
      aggregate_inito = New_INITO (st);
      not_at_root = FALSE;
      WFE_Add_Aggregate_Init_Symbol (WN_st(WN_kid0(init_wn)),
		WN_offset(WN_kid0(init_wn)) - WN_const_val(WN_kid1(init_wn)));
#endif
      return;
    }
#ifdef TARG_ST
/* (cbr) DDTSst24451. add support for label diffs initializers */
    if (WN_operator(kid0) == OPR_LDA_LABEL &&
        WN_operator(kid1) == OPR_LDA_LABEL) {
 
      aggregate_inito = New_INITO (st);
      not_at_root = FALSE;

      kid = TREE_OPERAND(init, 0);

      tree label0=NULL;
      tree label1=NULL;

      if (TREE_CODE (kid)== CONVERT_EXPR) {
        kid = TREE_OPERAND(kid,0);
        if (TREE_CODE (kid)== ADDR_EXPR) {
          label0 = TREE_OPERAND(kid,0);
        }
      }

      kid = TREE_OPERAND(init, 1);
      if (TREE_CODE (kid)== CONVERT_EXPR) {
        kid = TREE_OPERAND(kid,0);
        if (TREE_CODE (kid)== ADDR_EXPR) {
          label1 = TREE_OPERAND(kid,0);
        }
      }
      
      if (label0 && label1) {
        LABEL_IDX label_idx0 = WFE_Get_LABEL (label0, FALSE);
        LABEL_IDX label_idx1 = WFE_Get_LABEL (label1, FALSE);

        WFE_Add_Aggregate_Init_Labeldiff (label_idx0, label_idx1);
        return;
      }
    }
#endif
  }
  Fail_FmtAssertion ("unexpected static init tree for %s", ST_name(st));
}


extern ST *
WFE_Generate_Temp_For_Initialized_Aggregate (tree init, char * name)
{
  TY_IDX ty_idx = Get_TY(TREE_TYPE(init));
  ST *temp = New_ST (CURRENT_SYMTAB);
  ST_Init (temp,
	Save_Str2 (name, ".init"),
	CLASS_VAR, SCLASS_PSTATIC, EXPORT_LOCAL,
	ty_idx );
  if (TREE_CODE(init) == CONSTRUCTOR
	&& ! Use_Static_Init_For_Aggregate (temp, init)) 
  {
	// do sequence of stores to temp
	Set_ST_sclass(temp, SCLASS_AUTO);	// put on stack
	Traverse_Aggregate_Constructor (temp, init, TREE_TYPE(init),
                                        FALSE /*gen_initv*/, 0, 0, 0);
  }
  else {
	// setup inito for temp
	Set_ST_is_initialized(temp);
#ifdef TARG_ST
        /* (cbr) DDTs23379 */
        INITO_IDX save_aggregate_inito = aggregate_inito;
        INITV_IDX save_last_aggregate_initv = last_aggregate_initv;
#endif
	aggregate_inito = New_INITO (temp);
	not_at_root = FALSE;
	last_aggregate_initv = 0;
	Traverse_Aggregate_Constructor (temp, init, TREE_TYPE(init),
                                        TRUE /*gen_initv*/, 0, 0, 0);
	WFE_Finish_Aggregate_Init ();
#ifdef TARG_ST
        /* (cbr) DDTs23379 */
        aggregate_inito = save_aggregate_inito;
        last_aggregate_initv = save_last_aggregate_initv;
#endif
  }
  return temp;
}

extern void
WFE_Initialize_Decl (tree decl)
{
  if (DECL_IGNORED_P(decl)) {
  	// ignore initialization unless really used
	// e.g. FUNCTION and PRETTY_FUNCTION
	return;
  }
  ST *st = Get_ST(decl);

#ifdef WFE_DEBUG
  fprintf(stdout, "==================================================\n"
	          "\t WFE_Initialize_Decl: %s \n"
	          "==================================================\n",
	          ST_name(st));
  print_tree (stdout, decl);
  fprintf(stdout, "==================================================\n");
#endif

  tree init = DECL_INITIAL(decl);

  if (TREE_STATIC(decl) || DECL_CONTEXT(decl) == NULL) 
  {
	// static or global context, so needs INITO
	if ((ST_sclass(st) == SCLASS_UGLOBAL &&
             !ST_init_value_zero(st)) ||
	    ST_sclass(st) == SCLASS_EXTERN  ||
	    ST_sclass(st) == SCLASS_COMMON)
		Set_ST_sclass(st, SCLASS_DGLOBAL);
	if (!ST_is_initialized(st)) {
		Set_ST_is_initialized(st);
		Add_Inito_For_Tree (init, decl, st);
		WFE_Finish_Aggregate_Init ();
	}
	if (TREE_READONLY(decl))
		Set_ST_is_const_var (st);
  }
  else {
	// mimic an assign
	if (TREE_CODE(init) == CONSTRUCTOR) {
		// is aggregate
		if (Use_Static_Init_For_Aggregate (st, init)) {
			// create inito for initial copy
			// and store that into decl
			ST *copy = WFE_Generate_Temp_For_Initialized_Aggregate(
					init, ST_name(st));
			WN *init_wn = WN_CreateLdid (OPR_LDID, MTYPE_M, MTYPE_M,
				0, copy, ST_type(copy));
			WFE_Stmt_Append(
				WN_CreateStid (OPR_STID, MTYPE_V, MTYPE_M,
					0, st, ST_type(st), init_wn),
				Get_Srcpos());
		}
		else {
			// do sequence of stores for each element
			Traverse_Aggregate_Constructor (st, init, TREE_TYPE(init),
                                FALSE /*gen_initv*/, 0, 0, 0);
		}
		return;
	}
	else {
		INT emitted_bytes;
		Gen_Assign_Of_Init_Val (st, init, 
			0 /*offset*/, 0 /*array_elem_offset*/,
			ST_type(st), FALSE, 0 /*field_id*/,
			FLD_HANDLE(), emitted_bytes);
	}
  }
}

// Called for declarations without initializers.
// Necessary to do Get_ST so things like
// int errno (at global level) get ST
// entries so a variable is emitted in whirl (and thus .o).
//
void
WFE_Decl (tree decl)
{
  if (DECL_INITIAL (decl) != 0) return; // already processed
  if (DECL_IGNORED_P(decl)) return;
#ifdef TARG_ST
  // [TTh] Check that dynamically added mtypes are not used
  //       for return value and arguments of function.
  if ((TREE_CODE (decl) == FUNCTION_DECL) && (DECL_FUNCTION_CODE(decl) == 0)) {
    // ...Current node is a function and not a builtin

    // Check argument types
    INT pi;
    tree list;
    for (pi=1, list = TYPE_ARG_TYPES (TREE_TYPE (decl));
	 list;
	 pi++, list = TREE_CHAIN (list)) {
      TY_IDX arg_ty_idx = Get_TY (TREE_VALUE (list));
      if (MTYPE_is_dynamic (TY_mtype (arg_ty_idx))) {
	error ("forbidden type `%s' for parameter %d of `%s'",
	       MTYPE_name(TY_mtype(arg_ty_idx)),
	       pi,
	       (DECL_NAME (decl)) ? IDENTIFIER_POINTER (DECL_NAME (decl)) : "<unknown>");
      }
    }
  
    // Check return value
    TY_IDX ret_ty_idx = Get_TY(TREE_TYPE(TREE_TYPE(decl)));
    if (MTYPE_is_dynamic(TY_mtype(ret_ty_idx))) {
      error ("forbidden return type `%s' for `%s'",
	     MTYPE_name(TY_mtype(ret_ty_idx)),
	     (DECL_NAME (decl)) ? IDENTIFIER_POINTER (DECL_NAME (decl)) : "<unknown>");
    }
  }
#endif
  if (TREE_CODE(decl) != VAR_DECL) return;
  if (DECL_CONTEXT(decl) != 0) return;  // local
#ifndef TARG_ST
  /* (cbr) needed if static comes after extern decl. */
  if ( ! TREE_PUBLIC(decl)) return;     // local
#endif
  if ( ! TREE_STATIC(decl)) return;     // extern
  // is something that we want to put in symtab
  // (a global var defined in this file).
  (void) Get_ST(decl);
  return;
}

#ifdef TARG_ST
/* (cbr) for alias support. need to wait the end of the compilation
   for the attributes to be all set */
struct alias_syms
{
  struct alias_syms * next;
  char * bname;
  char * name;
};

struct alias_syms * alias_decls;

static int
add_alias (char *name, char *bname)
{
  struct alias_syms *alias;

  alias = (struct alias_syms *) xmalloc (sizeof (struct alias_syms));

  if (alias == NULL) 
    return 0;

  alias->next = alias_decls;
  alias->name = xstrdup(name);
  alias->bname = xstrdup(bname);
  alias_decls = alias;

  return 1;
}

static void
WFE_Set_Alias (tree decl, tree base_decl)
{
  ST *base_st = Get_ST (base_decl);

#ifdef TARG_ST
  // [CG]: Aliases on common data do not work in ELF.
  // gcc silently fails. We generate a warning.
  if (ST_class(base_st) == CLASS_VAR && ST_sclass(base_st) == SCLASS_COMMON) {
    warning_with_decl (decl,
		     "alias attribute cannot be specified on common data");
    return;
  }
#endif

  ST *st = Get_ST (decl);

  // [CL] remember symbol is base of alias
  Set_ST_is_alias_base(base_st);

  if (ST_is_weak_symbol(st)) {
#ifdef TARG_ST
    /* (cbr) */
    Set_ST_sclass (st, SCLASS_EXTERN);
#endif
    Set_ST_strong_idx (*st, ST_st_idx (base_st));
    Set_ST_sclass (st, SCLASS_EXTERN);
  }
  else {
    Set_ST_base_idx (st, ST_st_idx (base_st));
    Set_ST_emit_symbol(st);	// for cg
#ifdef TARG_ST
    // [CG]: Don't set sclass to the target sclass for variables.
    // An alias on a variable is always extern.
    if (ST_class(base_st) == CLASS_VAR) {
      Set_ST_sclass (st, SCLASS_EXTERN);
    } else if (ST_class(base_st) == CLASS_FUNC) {
      Set_ST_sclass (st, ST_sclass (base_st));
    } 

    if (ST_is_initialized (base_st))
      Set_ST_is_initialized (st);

#endif
  }
  /*
    if (ST_is_initialized (base_st)) {
    Set_ST_is_initialized (st);
    if (ST_init_value_zero (base_st))
    Set_ST_init_value_zero (st);
    }
  */
}

void
WFE_Assemble_Alias (tree decl, tree target)
{
  tree base_decl = lookup_name (target);

#ifdef TARG_ST
  /* (cbr) don't create symbol now */
  if (!base_decl || !DECL_ST(base_decl) || !DECL_ST(decl)) {
#else
  if (!base_decl || !DECL_ST(base_decl)) {
#endif
    add_alias (IDENTIFIER_POINTER(DECL_NAME(decl)), IDENTIFIER_POINTER (target));
    return;
  }

  WFE_Set_Alias (decl, base_decl);

} /* WFE_Assemble_Alias */

void
WFE_Alias_Finish ()
{
  while (alias_decls) {
    if (alias_decls->bname) {
      tree base_decl = lookup_name (get_identifier (alias_decls->bname));
      if (!base_decl) {
        /* (cbr) Synthetize a function decl */
        tree fnname = get_identifier (alias_decls->bname);
        base_decl = build_decl (FUNCTION_DECL, fnname,
                                build_function_type (void_type_node, NULL_TREE));
      }
      WFE_Set_Alias (lookup_name(get_identifier(alias_decls->name)), base_decl);
    }
    alias_decls = alias_decls->next;
  }
}
#else /* !TARG_ST */

void
WFE_Assemble_Alias (tree decl, tree target)
{
  DevWarn ("__attribute alias encountered at line %d", lineno);
  tree base_decl = lookup_name (target);
#if 0
  FmtAssert (base_decl != NULL,
             ("undeclared base symbol %s not yet declared in __attribute__ alias is not currently implemented",
              IDENTIFIER_POINTER (target)));
#endif
  if (!base_decl)
    base_decl =  lookup_name (get_identifier (IDENTIFIER_POINTER (target)));

  ST *base_st = Get_ST (base_decl);
  ST *st = Get_ST (decl);
  if (ST_is_weak_symbol(st)) {
    Set_ST_strong_idx (*st, ST_st_idx (base_st));
    Set_ST_sclass (st, SCLASS_EXTERN);
  }
  else {
    Set_ST_base_idx (st, ST_st_idx (base_st));
    Set_ST_emit_symbol(st);	// for cg
    Set_ST_sclass (st, ST_sclass (base_st));
    if (ST_is_initialized (base_st))
      Set_ST_is_initialized (st);
  }
/*
  if (ST_is_initialized (base_st)) {
    Set_ST_is_initialized (st);
    if (ST_init_value_zero (base_st))
      Set_ST_init_value_zero (st);
  }
*/
} /* WFE_Assemble_Alias */

#endif /* !TARG_ST */

void
WFE_Assemble_Constructor (const char *name)
{
#if 0
  DevWarn ("__attribute__ ((constructor)) encountered at line %d", lineno);
#endif
  tree func_decl = lookup_name (get_identifier (name));
  ST *func_st = Get_ST (func_decl);
  INITV_IDX initv = New_INITV ();
  INITV_Init_Symoff (initv, func_st, 0, 1);
  ST *init_st = New_ST (GLOBAL_SYMTAB);
  ST_Init (init_st, Save_Str2i ("__ctors", "_", ++__ctors),
           CLASS_VAR, SCLASS_FSTATIC,
           EXPORT_LOCAL, Make_Pointer_Type (ST_pu_type (func_st), FALSE));
  Set_ST_is_initialized (init_st);
  INITO_IDX inito = New_INITO (init_st, initv);
  ST_ATTR_IDX st_attr_idx;
  ST_ATTR&    st_attr = New_ST_ATTR (GLOBAL_SYMTAB, st_attr_idx);
  ST_ATTR_Init (st_attr, ST_st_idx (init_st), ST_ATTR_SECTION_NAME,
                Save_Str (".ctors"));
}

void
WFE_Assemble_Destructor (const char *name)
{
#if 0
  DevWarn ("__attribute__ ((destructor)) encountered at line %d", lineno);
#endif
  tree func_decl = lookup_name (get_identifier (name));
  ST *func_st = Get_ST (func_decl);
  INITV_IDX initv = New_INITV ();
  INITV_Init_Symoff (initv, func_st, 0, 1);
  ST *init_st = New_ST (GLOBAL_SYMTAB);
  ST_Init (init_st, Save_Str2i ("__dtors", "_", ++__dtors),
           CLASS_VAR, SCLASS_FSTATIC,
           EXPORT_LOCAL, Make_Pointer_Type (ST_pu_type (func_st), FALSE));
  Set_ST_is_initialized (init_st);
  INITO_IDX inito = New_INITO (init_st, initv);
  ST_ATTR_IDX st_attr_idx;
  ST_ATTR&    st_attr = New_ST_ATTR (GLOBAL_SYMTAB, st_attr_idx);
  ST_ATTR_Init (st_attr, ST_st_idx (init_st), ST_ATTR_SECTION_NAME,
                Save_Str (".dtors"));
  Set_PU_no_inline (Pu_Table [ST_pu (func_st)]);
  Set_PU_no_delete (Pu_Table [ST_pu (func_st)]);
  Set_ST_addr_saved (func_st);
}

ST *
WFE_Get_Return_Address_ST (int level)
{
#ifdef TARG_ST
  // [CG] Works only for level 0.
  // Use the be function Create_Special_Return_Address_Symbol() to
  // create the symbol.
  // The ST_is_return_var flag must not be set, it is used for
  // other purpose.
  FmtAssert(level == 0,
	    ("file-scope asm must be at global symtab scope."));
  ST *return_address_st = Return_Address_ST [CURRENT_SYMTAB];
  if (return_address_st == NULL) {
    return_address_st = New_ST (CURRENT_SYMTAB - level);
    ST_Init (return_address_st, Save_Str ("__return_address"), CLASS_VAR,
             SCLASS_AUTO, EXPORT_LOCAL, 
             Make_Pointer_Type (Be_Type_Tbl (MTYPE_V), FALSE));
    Return_Address_ST [CURRENT_SYMTAB] = return_address_st;
  }
  return return_address_st;
#else
  ST *return_address_st = Return_Address_ST [CURRENT_SYMTAB - level];
  if (return_address_st == NULL) {
    return_address_st = New_ST (CURRENT_SYMTAB - level);
    ST_Init (return_address_st, Save_Str ("__return_address"), CLASS_VAR,
             SCLASS_AUTO, EXPORT_LOCAL, 
             Make_Pointer_Type (Be_Type_Tbl (MTYPE_V), FALSE));
    Set_ST_is_return_var (return_address_st);
    Return_Address_ST [CURRENT_SYMTAB - level] = return_address_st;
  }
  return return_address_st;
#endif
} /* WFE_Get_Return_Address_ST */

ST *
WFE_Alloca_0 (void)
{
  WN *wn;
  TY_IDX ty_idx = Make_Pointer_Type (Be_Type_Tbl (MTYPE_V), FALSE);
  ST* alloca_st = Gen_Temp_Symbol (ty_idx, "__alloca");
  wn = WN_CreateAlloca (WN_CreateIntconst (OPC_I4INTCONST, 0));
  wn = WN_Stid (Pointer_Mtype, 0, alloca_st, ty_idx, wn);
  WFE_Stmt_Append (wn, Get_Srcpos());
  Set_PU_has_alloca (Get_Current_PU ());
  return alloca_st;
} /* WFE_Alloca_0 */

ST *
WFE_Alloca_ST (tree decl)
{
  ST *st = Create_ST_For_Tree (decl);
  ST *alloca_st = New_ST (CURRENT_SYMTAB);
  ST_Init (alloca_st, Save_Str (ST_name (st)),
           CLASS_VAR, SCLASS_AUTO, EXPORT_LOCAL,
           Make_Pointer_Type (ST_type (st), FALSE));
  Set_ST_is_temp_var (alloca_st);
  Set_ST_pt_to_unique_mem (alloca_st);
  Set_ST_base_idx (st, ST_st_idx (alloca_st));
  WN *swn = WFE_Expand_Expr (TYPE_SIZE(TREE_TYPE(decl)));
#ifdef TARG_ST
  // [SC] swn contains the size in bits.  Convert this to bytes.
  TYPE_ID mtype = TY_mtype (WN_object_ty (swn));
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  // (cbr) swn was not a save_expr
  if (!mtype) {
    TY_IDX ty_idx = Get_TY (TREE_TYPE(TYPE_SIZE(TREE_TYPE(decl))));
    mtype  = TY_mtype (ty_idx);
  }
#endif
  swn = WN_Div (mtype, swn, WN_Intconst (mtype, BITSPERBYTE));
#endif
  WN *wn  = WN_CreateAlloca (swn);
  wn = WN_Stid (Pointer_Mtype, 0, alloca_st, ST_type (alloca_st), wn);
  WFE_Stmt_Append (wn, Get_Srcpos());
#ifdef TARG_ST
  /* (cbr) should be set */
  Set_PU_has_alloca (Get_Current_PU ());
#endif
  return st;
} /* WFE_Alloca_ST */

void
WFE_Dealloca (ST *alloca_st, tree vars)
{
  int  nkids = 0;
  tree decl;
  WN   *wn;
  ST   *st;
  ST   *base_st;

  for (decl =vars; decl; decl = TREE_CHAIN (decl))
    if (TREE_CODE (decl) == VAR_DECL && DECL_ST (decl)) {
      st = DECL_ST (decl);
      base_st = ST_base (st);
      if (st != base_st)
        ++nkids;
  }

  wn = WN_CreateDealloca (nkids+1);
  WN_kid0 (wn) = WN_Ldid (Pointer_Mtype, 0, alloca_st, ST_type (alloca_st));
  nkids = 0;

  for (decl =vars; decl; decl = TREE_CHAIN (decl))
    if (TREE_CODE (decl) == VAR_DECL && DECL_ST (decl)) {
      st = DECL_ST (decl);
      base_st = ST_base (st);
      if (st != base_st)
        WN_kid (wn, ++nkids) = WN_Ldid (Pointer_Mtype, 0, base_st, ST_type (base_st));
  }

  WFE_Stmt_Append (wn, Get_Srcpos());
} /* WFE_Dealloca */

void
WFE_Record_Asmspec_For_ST (tree decl, const char *asmspec, int reg)
{
#ifndef TARG_ST
	extern PREG_NUM Map_Reg_To_Preg [];
#else
	//TB now Map_Reg_To_Preg is defined
#endif
  PREG_NUM preg = Map_Reg_To_Preg [reg];
  FmtAssert (preg >= 0,
             ("mapping register %d to preg failed\n", reg));
  ST *st = Get_ST (decl);
  TY_IDX ty_idx = ST_type (st);
  Set_TY_is_volatile (ty_idx);
  Set_ST_type (st, ty_idx);
  Set_ST_assigned_to_dedicated_preg (st);
  ST_ATTR_IDX st_attr_idx;
  ST_ATTR&    st_attr = New_ST_ATTR (CURRENT_SYMTAB, st_attr_idx);
  ST_ATTR_Init (st_attr, ST_st_idx (st), ST_ATTR_DEDICATED_REGISTER, preg);
} /* WFE_Record_Asmspec_For_ST */

void
WFE_Resolve_Duplicate_Decls (tree olddecl, tree newdecl)
{
  ST     *st      = DECL_ST(olddecl);
  tree    newtype = TREE_TYPE(newdecl);
  tree    newsize = TYPE_SIZE(newtype);
  TY_IDX  ty      = ST_type (st);

  if (TREE_STATIC(olddecl) == FALSE &&
      TREE_STATIC(newdecl) == TRUE  &&
      TREE_PUBLIC(olddecl) == TRUE  &&
      TREE_PUBLIC(newdecl) == FALSE) {
    Set_ST_sclass (st, SCLASS_FSTATIC);
    Set_ST_export (st, EXPORT_LOCAL);
  }

  if (newsize                           &&
      TREE_CODE(newsize) == INTEGER_CST &&
      TY_size (ty) <= Get_Integer_Value (newsize) / BITSPERBYTE) {
    UINT64 size = Get_Integer_Value (newsize) / BITSPERBYTE;
    Set_TY_size (ty, size);
#ifdef TARG_ST
    // [CG] We directly get the domain ubnd as for
    // multidim arrays, the ubnd is NOT equal to size / element size.
    if (TY_kind (ty) == KIND_ARRAY &&
	TREE_CODE(TYPE_MAX_VALUE(TYPE_DOMAIN(newtype))) == INTEGER_CST) {
      int dim = ARB_dimension(TY_arb(ty));
      Set_ARB_const_ubnd (TY_arb(ty)[dim-1]);
      Set_ARB_ubnd_val (TY_arb(ty)[dim-1], 
			Get_Integer_Value(TYPE_MAX_VALUE(TYPE_DOMAIN(newtype))));
    }
#else
    if (TY_kind (ty) == KIND_ARRAY) {
      Set_ARB_const_ubnd (TY_arb(ty));
      Set_ARB_ubnd_val (TY_arb(ty), (size / TY_size(TY_etype(ty))) - 1);
    }
#endif
  } 
} /* WFE_Resolve_Duplicate_Decls */


void
WFE_Add_Weak (tree decl)
{
  if (decl) {
    ST *st = DECL_ST (decl);
    if (st)
      Set_ST_is_weak_symbol (st);
  }
} /* WFE_Add_Weak */


#if defined (TARG_ST) && (GNU_FRONT_END==33)
extern tree weak_decls;
/* (cbr) gcc 3.3 upgrade */
void
WFE_Gen_Weak (tree decl, const char *name)
{
  ST *st = Get_ST (decl);

  Set_ST_is_weak_symbol (st);

#if 0
  ST *base_st = DECL_ST (base_decl);
  if (base_st)
    Set_ST_strong_idx (*st, ST_st_idx (base_st));

#endif
}
#else
void
WFE_Weak_Finish ()
{
  for (t = weak_decls; t; t = t->next) {
    const char *name = t->name;

    if (name) {
      tree decl = lookup_name (get_identifier (name));
      if (!decl) 
        warning ("did not find declaration `%s' for used in #pragma weak", name);
      else {
#ifdef TARG_ST
        /* (cbr) */
        ST *st = Get_ST (decl);
#else
        ST *st = DECL_ST (decl);
	if (st == NULL && t->value) {
	  st = Get_ST (decl);
	}
#endif
        if (st) {
          Set_ST_is_weak_symbol (st);

          if (t->value) {
            tree base_decl = lookup_name (get_identifier (t->value));
            if (!base_decl)
               warning ("did not find declaration for `%s' used in #pragma weak", t->value);
            else {
              ST *base_st = DECL_ST (base_decl);
              if (base_st)
                Set_ST_strong_idx (*st, ST_st_idx (base_st));
            }
          }
        }
      }
    }
  }
} /* WFE_Weak_Finish */
#endif

#ifdef TARG_ST
#include "wn_tree_util.h"
// [CG]
// function_update_volatile_accesses(WN *func_wn)
//
// Update missing volatile accesses that could not
// be generated by the translator.
//
// Refer to the comment in tree_symtab.cxx, function
// fixup_volatility() for the rationale.

// This is function object passed to the tree walk below.
class WN_update_volatile {
public:
  void  operator()(WN* wn) {
    OPCODE opc = WN_opcode(wn);
    // Get nodes that access structure fields.
    if (OPCODE_is_load (opc) || OPCODE_is_store (opc)) {
      if (OPCODE_has_field_id(opc) && WN_field_id(wn)) {
	TY_IDX type = 0;
	// Get the structure type.
	if (OPCODE_has_1ty(opc)) {
	  if (OPCODE_operator(opc) == OPR_ISTORE ||
	      OPCODE_operator(opc) == OPR_MSTORE ||
	      OPCODE_operator(opc) == OPR_MLOAD ||
	      OPCODE_operator(opc) == OPR_ISTBITS) {
	    type = TY_pointed (WN_ty (wn));
	  } else {
	    type = WN_ty (wn);
	  }
	} else if (OPCODE_has_2ty(opc)) {
	  type = TY_pointed (WN_load_addr_ty(wn));
	}
	// Get the field and set structure type volatile if
	// the field is volatile
	TY_IDX new_type = type;
	UINT tmp = 0;
	FLD_HANDLE fld = FLD_get_to_field (type, WN_field_id(wn), tmp);
	if (TY_is_volatile(FLD_type(fld))) Set_TY_is_volatile(new_type);
	// Update the structure type in the node.
	if (new_type != type) {
	  if (OPCODE_has_1ty(opc)) {
	    if (OPCODE_operator(opc) == OPR_ISTORE ||
		OPCODE_operator(opc) == OPR_MSTORE ||
		OPCODE_operator(opc) == OPR_MLOAD ||
		OPCODE_operator(opc) == OPR_ISTBITS) {
	      WN_set_ty(wn, Make_Pointer_Type (new_type));
	    } else {
	      WN_set_ty(wn, new_type);
	    }
	  } else if (OPCODE_has_2ty(opc)) {
	    // [CG]: We must dereference the second type.
	    WN_set_load_addr_ty(wn, Make_Pointer_Type (new_type));
	  }
	}
      }
    }
  }
};

static void
function_update_volatile_accesses(WN *func_wn)
{
  WN_update_volatile update;
  WN_TREE_walk_pre_order (func_wn, update);
}
#endif

