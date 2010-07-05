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


/* -*-Mode: c++;-*- (Tell emacs to use c++ mode) */
/* ====================================================================
 * ====================================================================
 *
 * Module: inline_summarize.h
 *
 * Description:
 *	aux. data structures used by the inliner version of summary phase.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef inline_summarize_INCLUDED
#define inline_summarize_INCLUDED

// define dummy variables referenced by ipl_summarize_template.h that are
// never referenced by the inliner.  In fact, these references will be
// removed by the optimizer as dead code.  But if we build the inliner
// without optimization, we need them to statisfy the linker. 
WN_MAP Summary_Map;
WN_MAP Stmt_Map;
BOOL Trace_IPA = FALSE, Trace_Perf = FALSE;
static BOOL DoPreopt = FALSE;
static BOOL Do_Par = FALSE;
static BOOL Do_Common_Const = FALSE;
// end definitions of dummy variables

#include "ipl_summarize_template.h"	
#include "ipl_analyze_template.h"	


extern MEM_POOL* PU_pool;

//-----------------------------------------------------------
// summary procedure
//-----------------------------------------------------------
template <>
void 
SUMMARIZE<INLINER>::Process_procedure (WN *w)
{
    SUMMARY_PROCEDURE *proc = New_procedure ();
    SUMMARY_PROC_INFO *proc_info = New_proc_info ();
    WN* w2;
    BOOL Has_return_already = FALSE;
    BOOL Has_pdo_pragma = FALSE;
    BOOL Has_local_pragma = FALSE;

    static MEM_POOL Temp_pool;
    static BOOL Temp_pool_initialized = FALSE;

    if (!WHIRL_Return_Val_On)
	proc->Set_use_lowered_return_preg ();

    if (INLINE_Enable_Copy_Prop) {
	if (!Temp_pool_initialized) {
	    Temp_pool_initialized = TRUE;
	    MEM_POOL_Initialize(&Temp_pool, "temp_pool", 0);
	}
	MEM_POOL_Push(&Temp_pool);

	// init the global hash table;
	Global_hash_table = CXX_NEW (GLOBAL_HASH_TABLE (113, &Temp_pool),
				     &Temp_pool);
    };

    // Need to create Parent_Map for enabling copy_prop
    // Parent_Map needed even when we don't enable copy_prop
    // eg to check if a call is a fake call in a EH region

    Parent_Map = WN_MAP_Create (PU_pool);
    LWN_Parentize(w);

    // process all the file/pu scope pragmas
    if (WN_operator(w) == OPR_FUNC_ENTRY)
	Process_pragmas(w);

    const PU& pu = Get_Current_PU ();
    if (PU_mp_needs_lno (pu))
        proc->Set_has_mp_needs_lno ();

    ST* st = WN_st(w);

#ifdef TARG_ST
    // CL: why distinguish between C and C++?
    if ( (PU_src_lang(pu) == PU_CXX_LANG)
	 || (PU_src_lang(pu) == PU_C_LANG) ) {
#else
    if (PU_src_lang(pu) == PU_CXX_LANG) {
#endif
	if ((Opt_Level > 1) && !INLINE_Static_Set)
	    INLINE_Static = TRUE;
	if (!IPA_Enable_DFE_Set)
	    IPA_Enable_DFE_Set = TRUE;
    }

#if !defined( _LIGHTWEIGHT_INLINER ) || defined(TARG_ST) // [CL] enable check parameter number

    // set the formal count
    if (WN_num_formals(w)) {
	 if (WN_operator(w) == OPR_FUNC_ENTRY)
	     Process_formal (w, WN_num_formals(w), proc);
	 else
	     proc->Set_formal_count(WN_num_formals(w));
     }

#ifndef TARG_ST // [CL] only enable the above part
    if (INLINE_Enable_Subst_Copy_Prop)
      proc_info->Set_symbol_index(Get_symbol_index(st));
#endif

#endif // _LIGHTWEIGHT_INLINER

    // the index into the symbols array for the st to this procedure
    proc->Set_symbol_index(Get_symbol_index(st));

    if (PU_no_inline (pu) )
        proc->Set_no_inline();

    if (PU_no_delete (pu) )
        proc->Set_no_delete();

    if (TY_is_varargs (Ty_Table[ST_pu_type (st)]))
        proc->Set_is_varargs();
    else {

	// for C/C++ only those specified with the inline function
	// attribute

	if ((PU_src_lang(pu) == PU_C_LANG) ||
	    ((PU_src_lang(pu) == PU_CXX_LANG))) {
	    if (PU_is_inline_function (pu)) {
	      // [SC] Removed the following fix: 
	      // I see no reason not to inline functions marked weak inline:
	      // gcc does it, and I want to do it for C++ vague linkage functions.
#ifndef TARG_ST
	        // for 7.2 don't inline functions marked weak inline
		// [CL] include fix from 0.15
		if (ST_is_weak_symbol(st) && ST_export(st) != EXPORT_PROTECTED) {
		    DevWarn("Inliner encountered a function marked weak inline; NOT inlining it");
		    proc->Set_no_inline();
		} else
#endif
#ifdef TARG_ST
		{
		  // CL: force inlining of 'inline' functions
		  if (INLINE_All_Inline) {
		    proc->Set_must_inline();
		  } else {
		    proc->Set_may_inline();
		  }
		}
#else
		    proc->Set_may_inline();
#endif
	    } // if weak inline function
	    // Heuristic based inlining for "static" functions(marked XLOCAL) 
	    // whose address has not been taken passed or saved
#ifndef TARG_ST
	    if (INLINE_Static &&
			ST_is_export_local(st) &&
#else
		// [CL] allow inlining of internal, hidden and protected functions
	    if (( INLINE_Static || !INLINE_Only_Inline) &&
			(ST_is_export_local(st) ||
			 (ST_export(st) == EXPORT_INTERNAL) ||
			 (ST_export(st) == EXPORT_HIDDEN) ||
			 (ST_export(st) == EXPORT_PROTECTED)) &&
#endif
			(!PU_is_inline_function(pu)) &&
			(ST_addr_not_passed(st)) &&
			(ST_addr_not_saved(st)) &&
			(!ST_is_weak_symbol(st))) {
		proc->Set_may_inline();
		Inline_tlog("Heuristic Inlining",0,"candidate is Static Function: %s",ST_name(WN_entry_name(w)));
	      }
#ifdef TARG_ST
	    // CL: allow inlining of non-static (ie extern) functions
	    // in non Gen_PIC_Shared model
	    if (!Gen_PIC_Shared &&
		!INLINE_Only_Inline &&
		!ST_is_weak_symbol(st)) {
		proc->Set_may_inline();
		Inline_tlog("Heuristic Inlining",0,"candidate is extern non-weak function: %s",ST_name(WN_entry_name(w)));
	    }
#endif
	} else {
	    // for fortran based pu's use some sort of inlining heuristic
	    // only static functions and those explicitly specified
	    // "inline"
	    // For Fortran for now disable copy prop in standalone inliner
	    // fix for 418835; 
	    // long term: Else need to move init of Ipl_Common etc out of 
            // #ifdef IPA_SUMMARY in ipl_main.cxx
	    INLINE_Enable_Copy_Prop = FALSE;
	    if (PU_is_inline_function (pu) ||
			ST_is_export_local(st) ||
			ST_export(st) == EXPORT_INTERNAL)
		proc->Set_may_inline();
	}
    }


    Set_lang(proc);

    // A handle on the procedure wn is needed in summary computation
    // use the entry_point: which is otherwise used ONLY by IPL
    
    Set_entry_point(w);


    // walk the tree
    for (WN_TREE_ITER<PRE_ORDER, WN*> iter (w); iter.Wn () != NULL; ++iter) {

	w2 = iter.Wn ();
	
	switch (WN_operator(w2)) {
	      
	case OPR_CALL: {
	    if (INLINE_Enable_Subst_Copy_Prop)
	      proc_info -> Incr_call_count();
	  
	    // ignore fake call from exception handling block
	    if ((WN_opcode(w2) == OPC_VCALL) &&
		(WN_Fake_Call_EH_Region(w2, Parent_Map)))
	      break;

	    Process_callsite (w2, proc->Get_callsite_count (), 0);
	    SUMMARY_CALLSITE *callsite = Get_callsite(Get_callsite_idx());
	    callsite->Set_wn (w2);
	    proc->Incr_call_count ();
	    proc->Incr_callsite_count ();
	    } 
	    break;

#ifndef _LIGHTWEIGHT_INLINER

	    // record that its a ref
	case OPR_LDA:
	case OPR_LDID:
	case OPR_ILOAD:
	    if (INLINE_Enable_Copy_Prop) {
		Record_ref (w2);
	    }
	    break;

	    // record that its a mod
	case OPR_ISTORE:
	case OPR_MSTORE:
	    // for ISTORE and MSTORE also record proc_info._state
	    if (INLINE_Enable_Subst_Copy_Prop)
	      proc_info -> Set_has_istore();
	    
	case OPR_STID:
	    if (INLINE_Enable_Copy_Prop) {
		Record_mod (w2);
	    }
	    break;

	    // for param, update the addr_taken_passed  count
	case OPR_PARM: 
	    if (INLINE_Enable_Copy_Prop) {
		Update_Addr_Passed_Count(w2);
	    }
	    break;

#endif // _LIGHTWEIGHT_INLINER

	case OPR_ALTENTRY:
	    proc->Set_has_alt_entry();
	    proc->Set_no_inline();
	    break;	    

	// Exceptions now come as REGIONS (not as EXC_SCOPE_BEGINS)
	case OPR_EXC_SCOPE_BEGIN:
	    Fail_FmtAssertion ("Invalid opcode from old style Exception Processing\n");
	    break;

	case OPR_PRAGMA:
#ifndef _LIGHTWEIGHT_INLINER
	    if (WN_st(w2)) {
	      if ((ST_sclass(WN_st(w2)) == SCLASS_FORMAL ||
		   ST_sclass(WN_st(w2)) == SCLASS_FORMAL_REF)  &&
		  !(ST_is_value_parm(WN_st(w2))))
		proc->Set_has_formal_pragma();

                if (PU_has_alloca (Get_Current_PU ()) &&
                        is_variable_dim_array(ST_type(WN_st(w2))))
                    // Local VLAs with PRAGMA
                    proc->Set_has_formal_pragma();
	      
		// disable inlining in the case of MP formal pragmas
		// and or alloca
	      if ((ST_sclass(WN_st(w2)) == SCLASS_FORMAL) ||
                  PU_has_alloca (Get_Current_PU ()))
		{
		  switch (WN_pragma(w2))
		    {
		    case WN_PRAGMA_DISTRIBUTE:
		    case WN_PRAGMA_DISTRIBUTE_RESHAPE:
		    case WN_PRAGMA_DYNAMIC:
		    case WN_PRAGMA_REDISTRIBUTE:
		    case WN_PRAGMA_AFFINITY:
		    case WN_PRAGMA_DATA_AFFINITY:
		    case WN_PRAGMA_THREAD_AFFINITY:
		    case WN_PRAGMA_PAGE_PLACE:
		      proc->Set_has_formal_pragma();
		      break;

		    default:
		      break;
		    }
		}

                if (TY_is_non_pod(ST_type(WN_st(w2)))) {
                    switch (WN_pragma(w2)) {
                    case WN_PRAGMA_LOCAL:
                    case WN_PRAGMA_LASTLOCAL:
                    case WN_PRAGMA_FIRSTPRIVATE:
                        Has_local_pragma = TRUE;
                        break;

                    default:
                        break;
                    }
                }

	    }
#endif // _LIGHTWEIGHT_INLINER

	    // parallel pragmas can occur in 2 possible places
	    // 1. just before a loop (could have interveaning statements)
	    if (WN_pragma(w2) == WN_PRAGMA_SINGLE_PROCESS_BEGIN) {
		proc->Set_has_parallel_pragma();
	    }

            if (WN_pragma(w2) == WN_PRAGMA_PARALLEL_BEGIN) {
                proc->Set_has_parallel_region_pragma();
            }

#ifndef _LIGHTWEIGHT_INLINER
            if ((WN_pragma(w2) == WN_PRAGMA_DOACROSS) ||
                (WN_pragma(w2) == WN_PRAGMA_PARALLEL_DO) ||
                (WN_pragma(w2) == WN_PRAGMA_PARALLEL_BEGIN) ||
                (WN_pragma(w2) == WN_PRAGMA_PARALLEL_SECTIONS)) 
                proc->Set_has_noinline_parallel_pragma();

            if ((WN_pragma(w2) == WN_PRAGMA_PDO_BEGIN) ||
                (WN_pragma(w2) == WN_PRAGMA_PSECTION_BEGIN) ||
                (WN_pragma(w2) == WN_PRAGMA_SINGLE_PROCESS_BEGIN))
                Has_pdo_pragma = TRUE;

#endif // _LIGHTWEIGHT_INLINER
#ifdef TARG_ST // [CL] support pragma [no]inline
	    // now, inline pragmas are part of the function body
	    Process_pragma_node (w2);
#endif
	    break;

	case OPR_REGION:
	    if (REGION_is_mp(w2))
	      proc->Set_has_parallel_region_pragma();
#if !defined( _LIGHTWEIGHT_INLINER ) || defined(TARG_ST)
	    if (WN_region_is_EH(w2))
	      proc->Set_exc_inline();
	    if (WN_region_kind(w2)== REGION_KIND_TRY)
	      proc->Set_exc_try();
#endif // _LIGHTWEIGHT_INLINER
	    break;

	case OPR_RETURN:
        case OPR_RETURN_VAL:
	    if (!proc->Has_early_returns ()) {
		if (!Last_Node (iter))
		    proc->Set_has_early_returns ();
	    }
            break;


	default:
	    break;
	}

	// early check to see if there are any static variables
	// that would need to be promoted if inlined or cloned

        if (OPCODE_has_sym(WN_opcode(w2)) && WN_st_idx(w2) != 0) {
            ST* st2 = ST_st_idx (WN_st(w2)) == ST_base_idx (WN_st(w2)) ? WN_st(w2) : ST_base (WN_st(w2));
            if (ST_level (st2) == CURRENT_SYMTAB) {
                // local symtab
                if (ST_sclass (st2) == SCLASS_PSTATIC) 
                    proc->Set_has_pstatic ();
            } else if ((ST_sclass (st2) == SCLASS_FSTATIC) &&
		    	!ST_class (st2) == CLASS_CONST)
                proc->Set_has_fstatic ();

#ifndef _LIGHTWEIGHT_INLINER
	    if (is_variable_dim_array(ST_type(st2))) 
                proc->Set_has_var_dim_array();
#endif // _LIGHTWEIGHT_INLINER
	}

    }

    if (proc->Get_callsite_count () > 0)
	proc->Set_callsite_index (Get_callsite_idx () -
				  proc->Get_callsite_count () + 1);

    // final cleanup: record, local addr taken attr 

    if (INLINE_Enable_Copy_Prop)  {
	Set_local_addr_taken_attrib();
	MEM_POOL_Pop ( &Temp_pool );
    }

    else if (INLINE_Enable_Split_Common) 
      {

	Set_local_addr_taken_attrib();
	// MEM_POOL_Pop ( &Temp_pool );
      }

#ifndef _LIGHTWEIGHT_INLINER
    if ( Has_local_pragma && Has_pdo_pragma)
        proc->Set_has_pdo_pragma();
#endif // _LIGHTWEIGHT_INLINER

} // SUMMARIZE::Process_procedure 

#endif /* inline_summarize_INCLUDED */