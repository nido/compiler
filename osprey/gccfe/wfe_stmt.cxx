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

extern "C" {
#include "gnu_config.h"
#include "gnu/system.h"

#include "gnu/flags.h"
#include "gnu/tree.h"

#ifdef TARG_ST
  /* (cbr) fix includes */
#include "gnu/cp/cp-tree.h"
#include "gnu/real.h"
#include "gnu/errors.h"		// To access warning() and error()
#include "tm_p.h"
#endif

#include "insn-config.h"	// MAX_RECOG_OPERANDS
}

#include "defs.h"
#include "glob.h"
#include "config.h"
#include "wn.h"
#include "wn_util.h"
#include "srcpos.h"
#include "erfe.h"

#include "ir_reader.h"
#include "tree_symtab.h"
#include "wfe_misc.h"
#include "wfe_expr.h"
#include "wfe_stmt.h"
#include "targ_sim.h"
#include <ctype.h>
#include "wfe_pragmas.h"
#ifdef TARG_ST
#include "wfe_loader.h" // For EXTENSION_Get_Mtype_For_Preg()
#include "ext_info.h"   // For EXTENSION_Are_Equivalent_Mtype()
#include "config_target.h"
#endif
extern "C" int decode_reg_name (char*);

#define ENLARGE(x) (x + (x >> 1))

static BOOL  *if_else_info_stack;
static INT32  if_else_info_i;
static INT32  if_else_info_max;

#ifdef TARG_ST
/* (cbr) for branch prediction builtin_expect */
INT32 if_else_hint;
#endif

enum LOOP_CONTINUE_INFO {
  CONTINUE_NONE,
  CONTINUE_ELSEWHERE,      // while 
  CONTINUE_HERE,
  CONTINUE_ELSEWHERE_HERE, // for
  CONTINUE_HERE_ELSEWHERE  // do while
};
  
typedef struct loop_info_t {
  struct nesting     *whichloop;
  LOOP_CONTINUE_INFO  continue_info;
  LABEL_IDX           continue_label_idx;
  LABEL_IDX           exit_label_idx;
  BOOL                exit_loop_if_false;
  BOOL                continue_here;
} LOOP_INFO;

static LOOP_INFO *loop_info_stack;
static INT32      loop_info_i;
static INT32      loop_info_max;

typedef struct case_info_t {
  INT64     case_lower_bound_value;
  INT64     case_upper_bound_value;
  LABEL_IDX case_label_idx;
} CASE_INFO;

typedef struct switch_info_t {
  WN        *index;
  TYPE_ID    index_mtype;
  INT32      start_case_index;
  LABEL_IDX  default_label_idx;
  LABEL_IDX  exit_label_idx;
} SWITCH_INFO;

static CASE_INFO   *case_info_stack;
static INT32        case_info_i;
static INT32        case_info_max;

static SWITCH_INFO *switch_info_stack;
static INT32        switch_info_i;
static INT32        switch_info_max;

typedef struct label_info_t {
  LABEL_IDX         label_idx;
  unsigned char     symtab_idx;
  unsigned char     defined;
} LABEL_INFO;

static LABEL_INFO  *undefined_labels_stack;
static INT32        undefined_labels_i;
static INT32        undefined_labels_max;

#ifdef TARG_ST
#include <vector>
#include <list>

extern const int WFE_CPlusPlus_Translator = 0;

bool need_manual_unwinding = false;

typedef struct eh_cleanup_entry {
  vector<tree>	     *cleanups;	// emit
  LABEL_IDX	     pad;	// emit
  LABEL_IDX	     start;	// emit after pad and before cleanups
  LABEL_IDX	     goto_idx;  // emit a goto
} EH_CLEANUP_ENTRY;

static std::list<EH_CLEANUP_ENTRY> cleanup_list_for_eh;

typedef struct scope_cleanup_info_t {
  tree		    stmt;
  LABEL_IDX	    label_idx;
} SCOPE_CLEANUP_INFO;

static SCOPE_CLEANUP_INFO *scope_cleanup_stack;
static INT32	    	   scope_cleanup_i;
static INT32	    	   scope_cleanup_max;

static tree	   *scope_stack;
static INT32	    scope_i;
static INT32	    scope_max;

void
Push_Scope_Cleanup (tree t)
{
  // Don't push a cleanup without a scope
  if (scope_cleanup_i == -1 && TREE_CODE(t) == (enum tree_code)CLEANUP_STMT)
    return;

  if (++scope_cleanup_i == scope_cleanup_max) {
    scope_cleanup_max = ENLARGE (scope_cleanup_max);
    scope_cleanup_stack =
      (SCOPE_CLEANUP_INFO *) xrealloc (scope_cleanup_stack,
               scope_cleanup_max * sizeof (SCOPE_CLEANUP_INFO));
  }

  scope_cleanup_stack [scope_cleanup_i].stmt = t;
  if (TREE_CODE(t) == (enum tree_code)CLEANUP_STMT)
    New_LABEL (CURRENT_SYMTAB, 
	       scope_cleanup_stack [scope_cleanup_i].label_idx);
  else
    scope_cleanup_stack [scope_cleanup_i].label_idx = 0;
}

LABEL_IDX
New_eh_cleanup_entry (tree t, vector<tree> *v, LABEL_IDX goto_idx)
{
  EH_CLEANUP_ENTRY e;

  e.cleanups = v;
  e.goto_idx = goto_idx;
  LABEL_IDX pad;
  New_LABEL (CURRENT_SYMTAB, pad);
  Label_Table[pad].kind = LKIND_BEGIN_HANDLER;
  e.pad = pad;
  New_LABEL (CURRENT_SYMTAB, e.start);
  cleanup_list_for_eh.push_back (e);
  return pad;
}

LABEL_IDX
lookup_cleanups (INITV_IDX& iv)
{
  tree t=0;
  iv = 0;
  vector<tree> *cleanups = new vector<tree>();

  if (scope_cleanup_i == -1) 
  {
	iv = New_INITV();
	INITV_Set_ZERO (Initv_Table[iv], MTYPE_U4, 1);
	return 0;
  }

  int scope_index;
  LABEL_IDX goto_idx=0;
  for (scope_index=scope_cleanup_i; scope_index>=0; scope_index--)
  {
    t = scope_cleanup_stack[scope_index].stmt;
    if (TREE_CODE(t) == (enum tree_code)CLEANUP_STMT)
      cleanups->push_back (t);
  }

  return New_eh_cleanup_entry (0, cleanups, goto_idx);
}

static void
Emit_Cleanup(tree cleanup)
{
  if (TREE_CODE(cleanup) == (enum tree_code)IF_STMT) {
    // Mimick WFE_Expand_If but don't call it, because WFE_Expand_If calls
    // WFE_Expand_Stmt which creates temp cleanups.  This leads to infinite
    // loop.
    FmtAssert(THEN_CLAUSE(cleanup) != NULL_TREE,
	      ("Do_Temp_Cleanups: then clause should be non-null"));
    FmtAssert(ELSE_CLAUSE(cleanup) == NULL_TREE,
	      ("Do_Temp_Cleanups: else clause should be null"));
    WN *test = WFE_Expand_Expr_With_Sequence_Point (IF_COND(cleanup),
						    Boolean_type);
    WN *then_block = WN_CreateBlock();
    WN *else_block = WN_CreateBlock();
    WN *if_stmt = WN_CreateIf (test, then_block, else_block);
    WFE_Stmt_Append (if_stmt, Get_Srcpos());
    WFE_Stmt_Push (then_block, wfe_stmk_if_then, Get_Srcpos());
    tree then_clause = THEN_CLAUSE(cleanup);
    if (TREE_CODE(then_clause) == EXPR_WITH_FILE_LOCATION)
      then_clause = EXPR_WFL_NODE(then_clause);
    else if (TREE_CODE(then_clause) == (enum tree_code)CLEANUP_STMT)
      then_clause = CLEANUP_EXPR(then_clause);
    WFE_One_Stmt_Cleanup(then_clause);
    WFE_Stmt_Pop(wfe_stmk_if_then);
  } else {
    if (TREE_CODE(cleanup) == EXPR_WITH_FILE_LOCATION)
      cleanup = EXPR_WFL_NODE(cleanup);
    else if (TREE_CODE(cleanup) == (enum tree_code)CLEANUP_STMT)
      cleanup = CLEANUP_EXPR(cleanup);
    WFE_One_Stmt_Cleanup (cleanup);
  }
}

// If non-zero, don't use label indexes less than or equal to this.
LABEL_IDX WFE_unusable_label_idx;

// The last label index allocated.
LABEL_IDX WFE_last_label_idx;

static TY_IDX
Type_For_Function_Returning_Void (void)
{
  static TY_IDX result = 0;
  if (result == 0) {
    TY &ty = New_TY (result);
 #ifdef TARG_ST
    /* (cbr) shut up warnings */
   TY_Init (ty, 0, KIND_FUNCTION, MTYPE_UNKNOWN, STR_IDX_ZERO);
#else
   TY_Init (ty, 0, KIND_FUNCTION, MTYPE_UNKNOWN, NULL);
#endif

   TYLIST_IDX tylist_idx;
    Set_TYLIST_type (New_TYLIST (tylist_idx), Void_Type);
    Set_TY_tylist (ty, tylist_idx);
#ifdef TARG_ST
    // (cbr) last tylist expects a 0
    Set_TYLIST_type (New_TYLIST (tylist_idx), 0);
#endif
  }
  
  return result;
}

static ST *
Function_ST_For_String (const char * s)
{
  ST * st = New_ST (GLOBAL_SYMTAB);
  PU_IDX pu_idx;
  PU & pu = New_PU (pu_idx);
  PU_Init (pu, Type_For_Function_Returning_Void (), GLOBAL_SYMTAB + 1);
  ST_Init (st, Save_Str(s),
           CLASS_FUNC, SCLASS_EXTERN, EXPORT_PREEMPTIBLE, TY_IDX(pu_idx));
  return st;
}
    
static void
Generate_unwind_resume (void)
{
  ST_IDX exc_ptr_param = TCON_uval (INITV_tc_val (INITO_val (PU_misc_info (Get_Current_PU()))));
  ST exc_st = St_Table[exc_ptr_param];
  WN* parm_node = WN_Ldid (Pointer_Mtype, 0, &exc_st, ST_type (exc_st));

  TY_IDX idx;
  TY &ptr_ty = New_TY (idx);
  TY_Init (ptr_ty, Pointer_Size, KIND_POINTER, Pointer_Mtype,
                        Save_Str ("anon_ptr."));
                                                                                
  ptr_ty.Set_pointed (ST_type(exc_st));
                                                                                
  WN * arg0 = WN_CreateParm (Pointer_Mtype, parm_node, idx, WN_PARM_BY_VALUE);
                                                                                
  ST * st = Function_ST_For_String("_Unwind_Resume");
  WN * call_wn = WN_Create (OPR_CALL, Pointer_Mtype, MTYPE_V, 1);
  WN_kid0 (call_wn) = arg0;
  WN_st_idx (call_wn) = ST_st_idx (st);

#ifdef TARG_ST
  /* (cbr) end of eh_region. never return */
  // FdF 20090929: Mark this function Never_Return
  WN_Set_Call_Never_Return(call_wn);
#endif  

  WFE_Stmt_Append (call_wn, Get_Srcpos());
}

static void
Cleanup_To_Scope(tree scope)
{
  INT32 i = scope_cleanup_i;
  INT32 j = -1;
  Is_True(i != -1, ("Cleanup_To_Scope: scope_cleanup_stack empty"));
  while (scope_cleanup_stack [i].stmt != scope) {
    if (TREE_CODE(scope_cleanup_stack [i].stmt) == (enum tree_code)SCOPE_STMT)
      j = i;
    --i;
  }

  if (j != -1) {
    i = scope_cleanup_i;
    while (i != j) {
      if (TREE_CODE(scope_cleanup_stack [i].stmt) == (enum tree_code)CLEANUP_STMT)
        WFE_One_Stmt_Cleanup (CLEANUP_EXPR(scope_cleanup_stack [i].stmt));
      --i;
    }
  }
}
 
// This should ultimately replace Do_EH_Cleanups(), at present the latter
// seems redundant.
//
// Emit all cleanups, and emit a goto after each set of cleanups to the handler.
void
Do_Cleanups_For_EH (void)
{

  for (std::list<EH_CLEANUP_ENTRY>::iterator i = cleanup_list_for_eh.begin();
		i != cleanup_list_for_eh.end(); ++i) {
    EH_CLEANUP_ENTRY e = *i;

    WN *pad_wn = WN_CreateLabel ((ST_IDX) 0, e.pad, 0, NULL);
    WN_Set_Label_Is_Handler_Begin (pad_wn);
    WFE_Stmt_Append (pad_wn, Get_Srcpos());

    WFE_Stmt_Append (WN_CreateLabel ((ST_IDX) 0, e.start, 0, NULL), 
    		     Get_Srcpos());

    for (vector<tree>::iterator j=e.cleanups->begin();
		j!=e.cleanups->end();++j)
    {
    	tree cleanup = *j;
        Emit_Cleanup(cleanup);
    }
    if (e.goto_idx)
	WFE_Stmt_Append (WN_CreateGoto ((ST_IDX) 0, e.goto_idx), Get_Srcpos());
    else {
      Generate_unwind_resume();
    }
  }
  cleanup_list_for_eh.clear();
}

void
Pop_Scope_And_Do_Cleanups (void)
{
  Is_True(scope_cleanup_i != -1,
	  ("Pop_Scope_And_Do_Cleanups: scope_cleanup-stack is empty"));

  while (true) {
    tree t = scope_cleanup_stack [scope_cleanup_i].stmt;
    if (TREE_CODE(t) != (enum tree_code)CLEANUP_STMT) {
      if (TREE_CODE(t) == (enum tree_code)SCOPE_STMT)
        {
 	--scope_cleanup_i;
        }
      break;
    }
    Is_True(scope_cleanup_i != -1,
	    ("Pop_Scope_And_Do_Cleanups: no scope_stmt on stack"));

    INT j = scope_cleanup_i - 1;
    LABEL_IDX goto_idx = 0;
    while (j != -1 && TREE_CODE(scope_cleanup_stack [j].stmt) != (enum tree_code)CLEANUP_STMT) {
      --j;
    }
    --scope_cleanup_i;
    need_manual_unwinding=true;
    WFE_One_Stmt_Cleanup (CLEANUP_EXPR(scope_cleanup_stack [scope_cleanup_i+1].stmt));
  }

}       

// [CL] support lexical blocks
static LEXICAL_BLOCK_INFO *current_lexical_block;
static int lexical_block_id;

static BOOL
nested_lexical_block_p (const LEXICAL_BLOCK_INFO *lexical_block)
{
  /*  [SC] at function scope, we will have different fndecl from
      parent (remember nested functions).
  */
  return (lexical_block->parent
	  && lexical_block->fndecl == lexical_block->parent->fndecl);
}

LEXICAL_BLOCK_INFO*
Push_Lexical_Block ()
{
  extern struct mongoose_gcc_DST_IDX DST_Create_Lexical_Block(LEXICAL_BLOCK_INFO*);
  LEXICAL_BLOCK_INFO* lexical_block;

  // [HK] malloc is poisoned, use xmalloc instead
  lexical_block = (LEXICAL_BLOCK_INFO*) xmalloc(sizeof(LEXICAL_BLOCK_INFO));
  lexical_block->id    = lexical_block_id++;
  lexical_block->parent = current_lexical_block;
  lexical_block->lexical_block_start_idx = LABEL_IDX_ZERO;
  lexical_block->lexical_block_end_idx = LABEL_IDX_ZERO;
  lexical_block->fndecl = current_function_decl;
  
  if(Debug_Level >= 2) {
    /* [CL] Function scope does not need
       an additional lexical block (a function is a scope itself)
    */
    if (nested_lexical_block_p (lexical_block)) {
      lexical_block->dst = DST_Create_Lexical_Block(lexical_block);
    }
  }

#if 0
  fprintf(stderr, "Creating lexical block id:%d parent:%d fn:%s dst:(%d:%d)\n",
	  lexical_block->id, lexical_block->parent ? lexical_block->parent->id : 0,
	  IDENTIFIER_POINTER (DECL_NAME (current_function_decl)),
	  lexical_block->dst.block, lexical_block->dst.offset);
#endif
  
  current_lexical_block = lexical_block;
  
  return lexical_block;
}

LEXICAL_BLOCK_INFO*
Pop_Lexical_Block ()
{
  LEXICAL_BLOCK_INFO* lexical_block = current_lexical_block;
  current_lexical_block = current_lexical_block->parent;

  return lexical_block;
}

void Set_Current_Scope_DST(tree x)
{
  // [CL] global scope variables get NULL scope here, so as to be
  // identified as "compilation unit" in the DST generator
  // [SC] Similarly, fn scope decls get NULL scope here,
  // so only decls in nested lexical blocks get non-NULL.
  if (current_lexical_block
      && nested_lexical_block_p (current_lexical_block)) {
    x->common.scope = current_lexical_block;
  } else {
    x->common.scope = NULL;
  }
}

void Start_Lexical_Block(LEXICAL_BLOCK_INFO* lexical_block)
{
  current_lexical_block = lexical_block;

  // [CL] create a label only for inner scopes
  if (nested_lexical_block_p (lexical_block)) {
    New_LABEL (CURRENT_SYMTAB,
	       lexical_block->lexical_block_start_idx);
    
    New_LABEL (CURRENT_SYMTAB,
	       lexical_block->lexical_block_end_idx);
    WFE_Stmt_Append(
		    WN_CreateLabel(lexical_block->lexical_block_start_idx,
				   0, NULL),
		    Get_Srcpos());

    if(Debug_Level >= 2) {
      if (! nested_lexical_block_p (lexical_block->parent)) {
	lexical_block->parent->dst = DECL_DST_IDX(lexical_block->parent->fndecl);
#if 0
	fprintf(stderr, "Lexical block id:%d dst:(%d:%d)\n",
		lexical_block->parent->id, lexical_block->parent->dst.block,
		lexical_block->parent->dst.offset);
#endif
      }
#if 0
      fprintf(stderr, "Linking block id:%d to parent id:%d\n",
	      lexical_block->id, lexical_block->parent->id);
#endif
      extern void DST_Link_Lexical_Block(LEXICAL_BLOCK_INFO*, LEXICAL_BLOCK_INFO*);
      DST_Link_Lexical_Block(lexical_block->parent, lexical_block);
    }
  }
}

void End_Lexical_Block(LEXICAL_BLOCK_INFO* lexical_block)
{
  // [CL] create a label only for inner scopes
  if (nested_lexical_block_p (lexical_block)) {
    WFE_Stmt_Append(
		    WN_CreateLabel(lexical_block->lexical_block_end_idx,
				   0, NULL),
		    Get_Srcpos());
  }
  current_lexical_block = lexical_block->parent;

  // [CL] end of life for this lexical block
  free(lexical_block);
}
#endif

void
WFE_Stmt_Init (void)
{
  if_else_info_max   = 32;
  if_else_info_i     = -1;
  // [HK] malloc is poisoned, use xmalloc instead
  if_else_info_stack = (BOOL *) xmalloc (sizeof (BOOL) * if_else_info_max);
  loop_info_max      = 32;
  loop_info_i        = -1;
  // [HK] malloc is poisoned, use xmalloc instead
  loop_info_stack    = (LOOP_INFO *) xmalloc (sizeof (LOOP_INFO) * loop_info_max);
  switch_info_max    = 32;
  switch_info_i      = -1;
  // [HK] malloc is poisoned, use xmalloc instead
  switch_info_stack  = (SWITCH_INFO *) xmalloc (sizeof (SWITCH_INFO) * switch_info_max);
  case_info_max      = 32;
  case_info_i        = -1;
  // [HK] malloc is poisoned, use xmalloc instead
  case_info_stack    = (CASE_INFO *) xmalloc (sizeof (CASE_INFO) * case_info_max);
  undefined_labels_max   = 32;
  undefined_labels_i     = -1;
  // [HK] malloc is poisoned, use xmalloc instead
  undefined_labels_stack = (LABEL_INFO *) xmalloc (sizeof (LABEL_INFO) * undefined_labels_max);

#ifdef TARG_ST
  scope_cleanup_max      = 32;
  scope_cleanup_i  	 = -1;
  scope_cleanup_stack    = (SCOPE_CLEANUP_INFO *) xmalloc (sizeof (SCOPE_CLEANUP_INFO) * scope_cleanup_max);

  // [CL] support for lexical blocks
  current_lexical_block = NULL;
  lexical_block_id = 0;
#endif

} /* WFE_Stmt_Init */

void
WFE_Expand_Start_Cond (tree cond, int exitflag)
{
  WN* if_stmt;
  WN* test;
  WN* then_block;
  WN* else_block;

  if (++if_else_info_i == if_else_info_max) {

    if_else_info_max   = ENLARGE(if_else_info_max);
  // [HK] realloc is poisoned, use xrealloc instead
    if_else_info_stack = (BOOL *) xrealloc (if_else_info_stack,
                                           if_else_info_max * sizeof (BOOL));
  }

  if_else_info_stack [if_else_info_i] = FALSE;
  test       = WFE_Expand_Expr_With_Sequence_Point (cond, Boolean_type);
  then_block = WN_CreateBlock ();
  else_block = WN_CreateBlock ();
  if_stmt    = WN_CreateIf (test, then_block, else_block);

#ifdef TARG_ST
  /* (cbr) for builtin expect */
  if (if_else_hint) {
    WN *pwn = WN_CreatePragma(WN_PRAGMA_MIPS_FREQUENCY_HINT, (ST_IDX) NULL,
                              if_else_hint, 0);
    WN_INSERT_BlockAfter (then_block, WN_last(then_block), pwn);
    if_else_hint = 0;
  }
#endif

  WFE_Stmt_Append (if_stmt, Get_Srcpos());
  WFE_Stmt_Push (else_block, wfe_stmk_if_else, Get_Srcpos());
  WFE_Stmt_Push (then_block, wfe_stmk_if_then, Get_Srcpos());
} /* WFE_Expand_Start_Cond */

void
WFE_Expand_Start_Else (void)
{
  FmtAssert (if_else_info_i >= 0,
             ("WFE_Expand_Start_Else: no ifs"));
  if_else_info_stack [if_else_info_i] = TRUE;
  WFE_Stmt_Pop (wfe_stmk_if_then);

#ifdef TARG_ST
  // [CL]
  WN_Set_Linenum(WFE_Stmt_Top(), Get_Srcpos());
#endif
} /* WFE_Expand_Start_Else */

void
WFE_Expand_End_Cond (void)
{
  FmtAssert (if_else_info_i >= 0,
             ("WFE_Expand_End_Cond: no ifs"));
  if (if_else_info_stack [if_else_info_i] == FALSE)
    WFE_Stmt_Pop (wfe_stmk_if_then);
  WFE_Stmt_Pop (wfe_stmk_if_else);
  --if_else_info_i;
} /* WFE_Expand_End_Cond */

void
WFE_Expand_Start_Loop (int exitflag, struct nesting *whichloop)
{
  WN* while_body;

  if (++loop_info_i == loop_info_max) {

    loop_info_max   = ENLARGE(loop_info_max);
  // [HK] realloc is poisoned, use xrealloc instead
    loop_info_stack = (LOOP_INFO *) xrealloc (loop_info_stack,
                                             loop_info_max * sizeof (LOOP_INFO));
  }

  loop_info_stack [loop_info_i].whichloop          = whichloop;
  loop_info_stack [loop_info_i].continue_info      = CONTINUE_NONE;
  loop_info_stack [loop_info_i].continue_label_idx = 0;
  loop_info_stack [loop_info_i].exit_label_idx     = 0;
  loop_info_stack [loop_info_i].exit_loop_if_false = FALSE;
  loop_info_stack [loop_info_i].continue_here      = FALSE;

  while_body = WN_CreateBlock ();
  WFE_Stmt_Push (while_body, wfe_stmk_while_body, Get_Srcpos());
} /* WFE_Expand_Start_Loop */

void
WFE_Expand_Start_Loop_Continue_Elsewhere (int exitflag, struct nesting *whichloop)
{
  FmtAssert (loop_info_i >= 0,
             ("WFE_Expand_Start_Loop_Continue_Elsewhere: no loops"));
  FmtAssert (loop_info_stack [loop_info_i].whichloop == whichloop,
             ("WFE_Expand_Start_Loop_Continue_Elsewhere: loop mismatch"));
  if (loop_info_stack [loop_info_i].continue_info == CONTINUE_NONE)
    loop_info_stack [loop_info_i].continue_info = CONTINUE_ELSEWHERE;
  else
  if (loop_info_stack [loop_info_i].continue_info == CONTINUE_HERE)
    loop_info_stack [loop_info_i].continue_info = CONTINUE_HERE_ELSEWHERE;
  else
    Fail_FmtAssertion ("WFE_Expand_Start_Loop_Continue_Elsewhere: unexpected state"); 
  LABEL_IDX continue_label_idx;
  New_LABEL (CURRENT_SYMTAB, continue_label_idx);
  loop_info_stack [loop_info_i].continue_label_idx = continue_label_idx;
} /* WFE_Expand_Start_Loop_Continue_Elsewhere */

void
WFE_Expand_Loop_Continue_Here (void)
{
  FmtAssert (loop_info_i >= 0,
             ("WFE_Expand_Loop_Continue_Here: no loops"));
  if (loop_info_stack [loop_info_i].continue_info == CONTINUE_NONE)
    loop_info_stack [loop_info_i].continue_info = CONTINUE_HERE;
  else
  if (loop_info_stack [loop_info_i].continue_info == CONTINUE_ELSEWHERE)
    loop_info_stack [loop_info_i].continue_info = CONTINUE_ELSEWHERE_HERE;
  else
    Fail_FmtAssertion ("WFE_Expand_Loop_Continue_Here: unexpected state"); 
  WFE_Stmt_Append (
    WN_CreateLabel ((ST_IDX) 0,
                    loop_info_stack [loop_info_i].continue_label_idx,
                    0, NULL),
    Get_Srcpos());
  loop_info_stack [loop_info_i].continue_here = TRUE;
} /* WFE_Expand_Loop_Continue_Here */

void
WFE_Expand_End_Loop (void)
{
  LABEL_IDX exit_label_idx;
  FmtAssert (loop_info_i >= 0,
             ("WFE_Expand_End_Loop: no loops"));
  if (!loop_info_stack [loop_info_i].exit_loop_if_false) {
    WN *while_body;
    WN *while_stmt;
    WN *test = WN_Intconst (MTYPE_I4, 1);
    while_body = WFE_Stmt_Pop (wfe_stmk_while_body);
    switch (loop_info_stack [loop_info_i].continue_info) {
      case CONTINUE_NONE:
      case CONTINUE_ELSEWHERE:
        while_stmt = WN_CreateWhileDo (test, while_body);
        break;
      case CONTINUE_ELSEWHERE_HERE:
        while_stmt = WN_CreateDoWhile (test, while_body);
        break;
      default:
        Fail_FmtAssertion ("WFE_Expand_Exit_Loop_If_False: unexpected state");
        break;
    }
    WFE_Stmt_Append (while_stmt, Get_Srcpos());
    WFE_Stmt_Push (while_body, wfe_stmk_while_body, Get_Srcpos());
  }
  if (loop_info_stack [loop_info_i].continue_label_idx &&
      loop_info_stack [loop_info_i].continue_here == FALSE) {
    WFE_Stmt_Append (
      WN_CreateLabel ((ST_IDX) 0,
                      loop_info_stack [loop_info_i].continue_label_idx,
                      0, NULL),
      Get_Srcpos());
  }
  WFE_Stmt_Pop (wfe_stmk_while_body);
  exit_label_idx = loop_info_stack [loop_info_i].exit_label_idx;
  if (exit_label_idx) {
    WFE_Stmt_Append (
      WN_CreateLabel ((ST_IDX) 0, exit_label_idx, 0, NULL),
      Get_Srcpos());
  }
  --loop_info_i;
} /* WFE_Expand_End_Loop */

void
WFE_Expand_Continue_Loop (struct nesting *whichloop)
{
  FmtAssert (loop_info_i >= 0,
             ("WFE_Expand_Continue_Loop: no loops"));
  LABEL_IDX continue_label_idx = loop_info_stack [loop_info_i].continue_label_idx;

  if (continue_label_idx == 0) {
    New_LABEL (CURRENT_SYMTAB, continue_label_idx);
    loop_info_stack [loop_info_i].continue_label_idx = continue_label_idx;
  }

  WN* wn = WN_CreateGoto ((ST_IDX) NULL, continue_label_idx);
  WFE_Stmt_Append (wn, Get_Srcpos());
} /* WFE_Expand_Continue_Loop */

void
WFE_Expand_Exit_Loop (struct nesting *whichloop)
{
  Fail_FmtAssertion ("WFE_Expand_Exit_Loop: unexpected state"); 
} /* WFE_Expand_Exit_Loop */

void
WFE_Expand_Exit_Loop_If_False (struct nesting *whichloop, tree cond)
{
  WN* while_stmt;
  WN* test;
  WN* while_body;

  FmtAssert (loop_info_i >= 0,
             ("WFE_Expand_Exit_Loop_If_False: no loops"));
  FmtAssert (loop_info_stack [loop_info_i].whichloop == whichloop,
             ("WFE_Expand_Exit_Loop_If_False: loop mismatch"));
  loop_info_stack [loop_info_i].exit_loop_if_false = TRUE;
#ifdef TARG_ST
  /* (cbr) ddts 24469. infinite loop */
  if (!cond)
    test       = WN_Intconst (Boolean_type, 1);
  else
#endif
  test       = WFE_Expand_Expr_With_Sequence_Point (cond, Boolean_type);
  while_body = WFE_Stmt_Pop (wfe_stmk_while_body);
  switch (loop_info_stack [loop_info_i].continue_info) {
    case CONTINUE_NONE:
    case CONTINUE_ELSEWHERE:
      while_stmt = WN_CreateWhileDo (test, while_body);
      break;
    case CONTINUE_ELSEWHERE_HERE:
      while_stmt = WN_CreateDoWhile (test, while_body);
      break;
    default:
      Fail_FmtAssertion ("WFE_Expand_Exit_Loop_If_False: unexpected state");
      break;
  }
  WFE_Stmt_Append (while_stmt, Get_Srcpos());
  WFE_Stmt_Push (while_body, wfe_stmk_while_body, Get_Srcpos());
} /* WFE_Expand_Exit_Loop_If_False */

void
WFE_Expand_Start_Case (int exit_flag, tree expr, tree type, char *printname)
{
  TYPE_ID index_mtype = Mtype_comparison (TY_mtype (Get_TY (TREE_TYPE (expr))));
  WN *switch_block    = WN_CreateBlock ();
  WN *index           = WFE_Expand_Expr_With_Sequence_Point (expr, index_mtype);
  WFE_Stmt_Push (switch_block, wfe_stmk_switch, Get_Srcpos());
  if (++switch_info_i == switch_info_max) {

    switch_info_max   = ENLARGE(switch_info_max);
  // [HK] realloc is poisoned, use xrealloc instead
    switch_info_stack = (SWITCH_INFO *) xrealloc (switch_info_stack,
                                             switch_info_max * sizeof (SWITCH_INFO));
  }
  switch_info_stack [switch_info_i].index             = index;
  switch_info_stack [switch_info_i].index_mtype       = index_mtype;
  switch_info_stack [switch_info_i].start_case_index  = case_info_i + 1;
  switch_info_stack [switch_info_i].default_label_idx = 0;
  switch_info_stack [switch_info_i].exit_label_idx    = 0;
} /* WFE_Expand_Start_Case */

void
WFE_Expand_Start_Case_Dummy (void)
{
} /* WFE_Expand_Start_Case_Dummy */

void
WFE_Add_Case_Node (tree low, tree high, tree label)
{
  WN        *wn;
  LABEL_IDX  case_label_idx;
  WN *lower_bound = WFE_Expand_Expr (low);
  WN *upper_bound = WFE_Expand_Expr (high);
  if (WN_const_val (lower_bound) != WN_const_val (upper_bound))
    DevWarn ("ecncountered case range at line %d", lineno);
  if (++case_info_i == case_info_max) {

    case_info_max   = ENLARGE(case_info_max);
  // [HK] realloc is poisoned, use xrealloc instead
    case_info_stack = (CASE_INFO *) xrealloc (case_info_stack,
                                             case_info_max * sizeof (CASE_INFO));
  }
  case_info_stack [case_info_i].case_lower_bound_value = WN_const_val (lower_bound);
  case_info_stack [case_info_i].case_upper_bound_value = WN_const_val (upper_bound);
#ifdef KEY
  if (label->decl.sgi_u1.label_idx != (LABEL_IDX) 0)
    case_label_idx = label->decl.sgi_u1.label_idx;
  else {
#endif
  FmtAssert (label->decl.sgi_u1.label_idx == (LABEL_IDX) 0,
             ("WFE_Add_Case_Node: label already defined"));
  New_LABEL (CURRENT_SYMTAB, case_label_idx);
#ifdef KEY
  }
#endif
  label->decl.sgi_u1.label_idx = case_label_idx;
  label->decl.label_defined = TRUE;
  case_info_stack [case_info_i].case_label_idx = case_label_idx;
  wn = WN_CreateLabel ((ST_IDX) 0, case_label_idx, 0, NULL);
  WFE_Stmt_Append (wn, Get_Srcpos ());
} /* WFE_Add_Case_Node */

void
WFE_Emit_Case_Nodes (void)
{
} /* WFE_Emit_Case_Nodes */

void
WFE_Expand_End_Case_Dummy (void)
{
} /* WFE_Expand_End_Case_Dummy */

void
WFE_Expand_End_Case ()
{
  INT32  i;
  INT32  n;
  WN    *switch_wn;
  WN    *switch_block;
  WN    *case_block;
  WN    *case_entry;
  WN    *def_goto;
  WN    *wn;
  TYPE_ID index_mtype = switch_info_stack [switch_info_i].index_mtype;

#ifdef TARG_ST
  // [CG]: fixed computation of num_entries
  n = 0;
#else
  n = case_info_i - switch_info_stack [switch_info_i].start_case_index;
#endif
  if (switch_info_stack [switch_info_i].exit_label_idx == 0) {
    New_LABEL (CURRENT_SYMTAB, switch_info_stack [switch_info_i].exit_label_idx);
  }
  if (switch_info_stack [switch_info_i].default_label_idx)
    def_goto = WN_CreateGoto (switch_info_stack [switch_info_i].default_label_idx);
  else
    def_goto = WN_CreateGoto (switch_info_stack [switch_info_i].exit_label_idx);
  case_block = WN_CreateBlock ();
  for (i = switch_info_stack [switch_info_i].start_case_index;
       i <= case_info_i;
       i++) {
    LABEL_IDX case_label_idx = case_info_stack [i].case_label_idx;
    if (case_info_stack [i].case_lower_bound_value ==
        case_info_stack [i].case_upper_bound_value) {

      INT64 case_value = case_info_stack [i].case_lower_bound_value;
      case_entry = WN_CreateCasegoto (case_value, case_label_idx);
      WN_INSERT_BlockLast (case_block, case_entry);
#ifdef TARG_ST
      n++;
#endif
    }
    else {
      if (MTYPE_is_signed (index_mtype)) {
        INT64 case_value;
        for (case_value  = case_info_stack [i].case_lower_bound_value;
             case_value <= case_info_stack [i].case_upper_bound_value;
             case_value++) {

          case_entry = WN_CreateCasegoto (case_value, case_label_idx);
          WN_INSERT_BlockLast (case_block, case_entry);
#ifdef TARG_ST
	  n++;
#endif
        }
      }
      else {
        UINT64 case_value;
        for (case_value  = (UINT64) case_info_stack [i].case_lower_bound_value;
             case_value <= (UINT64) case_info_stack [i].case_upper_bound_value;
             case_value++) {

          case_entry = WN_CreateCasegoto (case_value, case_label_idx);
          WN_INSERT_BlockLast (case_block, case_entry);
#ifdef TARG_ST
	  n++;
#endif
        }
      }
    }
  }
  switch_wn = WN_CreateSwitch (n,
                               switch_info_stack [switch_info_i].index,
                               case_block,
                               def_goto,
                               switch_info_stack [switch_info_i].exit_label_idx);
  switch_block = WFE_Stmt_Pop (wfe_stmk_switch);
#ifdef TARG_ST
  // [CL] use line number of switch() statement in user's source code
  WFE_Stmt_Append (switch_wn, WN_linenum(switch_block));
  WFE_Stmt_Append (switch_block, 0);
#else
  WFE_Stmt_Append (switch_wn, Get_Srcpos ());
  WFE_Stmt_Append (switch_block, Get_Srcpos ());
#endif
  wn = WN_CreateLabel ((ST_IDX) 0,
                       switch_info_stack [switch_info_i].exit_label_idx,
                       0, NULL);
#ifdef TARG_ST
  // [CL] use line number of switch() statement in user's source code
  WFE_Stmt_Append (wn, WN_linenum(switch_block));
#else
  WFE_Stmt_Append (wn, Get_Srcpos ());
#endif
  case_info_i = switch_info_stack [switch_info_i].start_case_index - 1;
  --switch_info_i;
} /* WFE_Expand_End_Case */

void
WFE_Record_Switch_Default_Label (tree label)
{
  LABEL_IDX  default_label_idx;
  WN        *wn;

#ifdef KEY
// Fix bug 951. If we have already formed the label (during expansion
// of STMT_EXPR), don't do it again (when called from GNU code)
  if (label->decl.sgi_u1.label_idx != (LABEL_IDX) 0)
      default_label_idx = label->decl.sgi_u1.label_idx;
  else
#else
  FmtAssert (label->decl.sgi_u1.label_idx == (LABEL_IDX) 0,
             ("WFE_Record_Switch_Default_Label: label already defined"));
#endif
  New_LABEL (CURRENT_SYMTAB, default_label_idx);
  label->decl.sgi_u1.label_idx = default_label_idx;
  label->decl.label_defined = TRUE;
  switch_info_stack [switch_info_i].default_label_idx = default_label_idx;
  wn = WN_CreateLabel ((ST_IDX) 0, default_label_idx, 0, NULL);
  WFE_Stmt_Append (wn, Get_Srcpos ());
} /* WFE_Record_Switch_Default_Label */

void
WFE_Expand_Exit_Something (struct nesting *n,
                           struct nesting *cond_stack,
                           struct nesting *loop_stack,
                           struct nesting *case_stack,
                           LABEL_IDX      *label_idx)
{
  LABEL_IDX  exit_label_idx = *label_idx;
  WN        *wn;
  if (n == case_stack) {
    if (exit_label_idx == 0) {
      New_LABEL (CURRENT_SYMTAB, exit_label_idx);
      *label_idx = exit_label_idx;
      switch_info_stack [switch_info_i].exit_label_idx = exit_label_idx;
    }
    wn = WN_CreateGoto (exit_label_idx);
    WFE_Stmt_Append (wn, Get_Srcpos ());
  }
  else
  if (n == loop_stack) {
    if (n == loop_info_stack [loop_info_i].whichloop) {
      if (exit_label_idx == 0) {
        New_LABEL (CURRENT_SYMTAB, exit_label_idx);
        *label_idx = exit_label_idx;
        loop_info_stack [loop_info_i].exit_label_idx = exit_label_idx;
      }
#ifdef TARG_ST
      if (scope_cleanup_i != -1) {
        tree t = scope_cleanup_stack [scope_cleanup_i].stmt;
        if (t && TREE_CODE (t) == (enum tree_code)CLEANUP_STMT) {
          WFE_One_Stmt_Cleanup (CLEANUP_EXPR(t));
        }
      }
#endif
      wn = WN_CreateGoto (exit_label_idx);
      WFE_Stmt_Append (wn, Get_Srcpos ());
    }
  }
} /* WFE_Expand_Exit_Something */

LABEL_IDX
WFE_Get_LABEL (tree label, int def)
{
  LABEL_IDX label_idx  = label->decl.sgi_u1.label_idx;
#ifndef TARG_ST
  // [SC] Unused.
  LABEL_IDX symtab_idx = label->decl.symtab_idx;
#endif

  if (label_idx == 0) {
    LABEL_Init (New_LABEL (CURRENT_SYMTAB, label_idx), 0, LKIND_DEFAULT);
    label->decl.sgi_u1.label_idx = label_idx;
#ifndef TARG_ST
    label->decl.symtab_idx   = CURRENT_SYMTAB;
#endif
    if (!def) {
      if (++undefined_labels_i == undefined_labels_max) {
        undefined_labels_max   = ENLARGE(undefined_labels_max);
        undefined_labels_stack =
  // [HK] realloc is poisoned, use xrealloc instead
          (LABEL_INFO *) xrealloc (undefined_labels_stack,
                                  undefined_labels_max * sizeof (LABEL_INFO));
      }
      undefined_labels_stack [undefined_labels_i].label_idx  = label_idx;
      undefined_labels_stack [undefined_labels_i].symtab_idx = CURRENT_SYMTAB;
      undefined_labels_stack [undefined_labels_i].defined    = FALSE;
    }
  }
  else {
    if (def) {
      for (int i = undefined_labels_i; i >= 0; --i) {
        if (undefined_labels_stack [i].label_idx  == label_idx &&
            undefined_labels_stack [i].symtab_idx == CURRENT_SYMTAB) {
          undefined_labels_stack [i].defined = TRUE;
          break;
        }
      }
    }
/*
    else {
      if (label->decl.label_defined)
#ifdef TARG_ST
        FmtAssert (DECL_CONTEXT (label) == current_function_decl,
	           ("jumping to a label not defined in current function"));
#else
        FmtAssert (label->decl.symtab_idx == CURRENT_SYMTAB,
                   ("jumping to a label not defined in current function"));
#endif
    }
*/
  }

  return label_idx;
} /* WFE_Get_LABEL */

void
WFE_Declare_Nonlocal_Label (tree label)
{
  LABEL_IDX label_idx = WFE_Get_LABEL (label, FALSE);
#ifdef TARG_ST
  // [CG]: set this flag only at the goto outer block generation instead
#else
  Set_LABEL_target_of_goto_outer_block (label_idx);
#endif
} /* WFE_Expand_Label */

void
WFE_Expand_Label (tree label)
{
  LABEL_IDX label_idx = WFE_Get_LABEL (label, TRUE);
#ifndef TARG_ST
  label->decl.symtab_idx = CURRENT_SYMTAB;
#endif
//fprintf (stderr, "\n");
  if (!label->decl.label_defined) {
    WN *wn;
    label->decl.label_defined = TRUE;
    wn = WN_CreateLabel ((ST_IDX) 0, label_idx, 0, NULL);
    WFE_Stmt_Append (wn, Get_Srcpos ());
  }
} /* WFE_Expand_Label */

void
WFE_Expand_Goto (tree label)
{
  WN *wn;
  LABEL_IDX label_idx = WFE_Get_LABEL (label, FALSE);
#ifdef TARG_ST
  // [CG]: no support for inter scope goto
  FmtAssert (DECL_CONTEXT (label) == current_function_decl,
	     ("line %d: jump to a label not defined in current function currently not implemented", lineno));
#else
  if ((CURRENT_SYMTAB > GLOBAL_SYMTAB + 1) &&
      (label->decl.symtab_idx < CURRENT_SYMTAB)) {
#ifdef TARG_ST
    // [CG] we set the flag only when necessary
    Set_LABEL_target_of_goto_outer_block (label_idx);
#endif
    wn = WN_CreateGotoOuterBlock (label_idx, label->decl.symtab_idx);
  }  else
#endif
    wn = WN_CreateGoto ((ST_IDX) NULL, label_idx);
  WFE_Stmt_Append (wn, Get_Srcpos());
} /* WFE_Expand_Goto */

void
WFE_Expand_Computed_Goto (tree exp)
{
#ifdef TARG_ST
  // [CG]: computed goto are implemented at function scope
#else
  DevWarn ("encountered indirect jump at line %d\n", lineno);
#endif
  Set_PU_no_inline (Get_Current_PU ());
  WN *addr = WFE_Expand_Expr (exp);
  WN *wn   = WN_CreateAgoto (addr);
  WFE_Stmt_Append (wn, Get_Srcpos());
} /* WFE_Expand_Computed_Goto */

void
WFE_Expand_Return (tree retval)
{
  WN *wn;

#if defined (TARG_ST) && (GNU_FRONT_END==33)
  if (!retval || TREE_CODE (TREE_TYPE (TREE_TYPE (current_function_decl))) == VOID_TYPE) {
#else
  if (TREE_CODE (TREE_TYPE (TREE_TYPE (current_function_decl))) == VOID_TYPE) {
#endif
    if (retval) {
      /* If function wants no value, give it none.  */
	(void) WFE_Expand_Expr_With_Sequence_Point (retval, MTYPE_V);
    }
    wn = WN_CreateReturn ();
  }
  else {
    WN *rhs_wn;
    TY_IDX ret_ty_idx = Get_TY(TREE_TYPE(TREE_TYPE(current_function_decl)));
#ifdef TARG_ST
    // [TTh] Check that dynamically added mtypes are not used
    //       as return type of a function call.
    if (MTYPE_is_dynamic(TY_mtype(ret_ty_idx))) {
      error ("forbidden type `%s' for return value",
	     MTYPE_name(TY_mtype(ret_ty_idx)));
      return;
    }
#endif
    rhs_wn = WFE_Expand_Expr_With_Sequence_Point (
		TREE_OPERAND (retval, 1),
		TY_mtype (ret_ty_idx));
    if (!WFE_Keep_Zero_Length_Structs    &&
        TY_mtype (ret_ty_idx) == MTYPE_M &&
        TY_size (ret_ty_idx) == 0) {
      // function returning zero length struct
#ifdef TARG_ST
      //TB: Sometime rhs_wn is NULL. For instance return f() with f
      // returning a zer0 zize struct
      if (rhs_wn && WN_has_side_effects (rhs_wn)) {
#else
      if (WN_has_side_effects (rhs_wn)) {
#endif
        rhs_wn = WN_CreateEval (rhs_wn);  
        WFE_Stmt_Append(rhs_wn, Get_Srcpos());
      }
      wn = WN_CreateReturn ();
#ifdef TARG_ST
      //TB: Set the is_return_val_lowered flag to TRUE to specify to the code
      //generator that this return is already lowered and to not emit a
      //warning that control reaches end of non-void function
      WN_is_return_val_lowered(wn) = TRUE;
#endif
    }
    else {
      if (WN_opcode (rhs_wn) == OPC_MMLDID &&
          WN_offset (rhs_wn) == 0          &&
          WN_field_id (rhs_wn) == 0        &&
          TY_align (ret_ty_idx) < MTYPE_align_best(Spill_Int_Mtype)) {
        ST *st = WN_st (rhs_wn);
        TY_IDX ty_idx = ST_type (st);
#ifdef TARG_ST
	// [CG] Optimize alignment only if the returned symbol is local,
	// it is invalid to do it for FORMAL or EXTERN at least. 
	if (ST_sclass(st) == SCLASS_AUTO) 
#endif
	  if (ty_idx == ret_ty_idx) {
	    Set_TY_align (ty_idx, MTYPE_align_best(Spill_Int_Mtype));
	    Set_ST_type (st, ty_idx);
	  }
      }
      WFE_Set_ST_Addr_Saved (rhs_wn);
      if (DECL_WIDEN_RETVAL(current_function_decl)) {
	TYPE_ID old_rtype = WN_rtype(rhs_wn);
	TYPE_ID new_rtype = MTYPE_is_signed (old_rtype) ? MTYPE_I8 : MTYPE_U8;
	rhs_wn = WN_Cvt (old_rtype, new_rtype, rhs_wn);
        wn = WN_CreateReturn_Val(OPR_RETURN_VAL, new_rtype, MTYPE_V, rhs_wn);
      }
      else
        wn = WN_CreateReturn_Val(OPR_RETURN_VAL, TY_mtype(ret_ty_idx), MTYPE_V, rhs_wn);
    }
  }
  WFE_Stmt_Append(wn, Get_Srcpos());
} /* WFE_Expand_Return */


/* Generate WHIRL for an asm statement with arguments.
   For now, we don't do all the stuff done by expand_asm_operands;
   instead, we leave much of that stuff until asm lowering time.
   Here, we just build the OPR_ASM node that records the relevant
   information about the asm statement. */

static WN *
idname_from_regnum (int gcc_reg)
{
  if (gcc_reg < 0) {
	DevWarn("unrecognized register name in asm");
  	return NULL;
  }
  else {

#ifndef TARG_ST
	extern PREG_NUM Map_Reg_To_Preg [];
#else
	//TB now Map_Reg_To_Preg is defined
#endif
	PREG_NUM preg = Map_Reg_To_Preg [gcc_reg];
	if (preg < 0) {
		DevWarn("couldn't map asm regname to preg");
		return NULL;
	}
	ST *st;
	if (Preg_Offset_Is_Int(preg))
		st = Int_Preg;
	else if (Preg_Offset_Is_Float(preg))
		st = Float_Preg;
#ifdef TARG_ST200
	else if (preg >= Branch_Preg_Min_Offset && 
		                 preg <= Branch_Preg_Max_Offset) {
	  st = MTYPE_To_PREG (MTYPE_B);
	}
	//TB: add preg to register class mapping for extension
	else if(preg > Get_Static_Last_Dedicated_Preg_Offset() &&
	  preg <= Last_Dedicated_Preg_Offset) {
	  TYPE_ID type = EXTENSION_Get_Mtype_For_Preg(preg);
	  st = MTYPE_To_PREG (type);
	}
#endif
	else {
#ifdef TARG_ST
	  //TB: Return specific PREG to handle think like non general register in
	  //clobber asm list
	  st = Untyped_Preg();
#else
	  FmtAssert (FALSE, ("unexpected preg %d", preg));
#endif
	}
  	return WN_CreateIdname((WN_OFFSET) preg, st);
  }
}

char *
remove_plus_modifier(char *s)
{
#define MAX_NON_PLUS_CONSTRAINT_CHARS 128
  static char out[MAX_NON_PLUS_CONSTRAINT_CHARS + 1];
  int i = 0;
  while (i <= MAX_NON_PLUS_CONSTRAINT_CHARS)
    {
      while (*s == '+')
	{
	  ++s;
	}
      out[i++] = *s;
      if (*s == '\0')
	{
	  return out;
	}
      else
	{
	  ++s;
	}
    }
  Fail_FmtAssertion("Constraint string too long");
  /*NOTREACHED*/
}
//TB: no more need. Now defined in config_target.cxx in the targinfo
#ifndef TARG_ST
BOOL
constraint_supported (const char *s)
{
  while (*s != 0) {
    if (*s != 'r' &&
	*s != 'f' &&
	*s != 'm' &&
	*s != '+' &&
	*s != ',' &&
	*s != '=' &&
	(*s < '0' ||
	 *s > '9')) {
      return FALSE;
    }
    ++s;
  }
  return TRUE;
}
#endif
ST *
st_of_new_temp_for_expr(const WN *expr)
{
  static unsigned int temp_count = 0;

  static char temp_name[64];

  sprintf(temp_name, "asm.by.address.temp_%u", temp_count++);

  ST *retval = New_ST(CURRENT_SYMTAB);
  
  ST_Init (retval,
	   Save_Str (temp_name),
	   CLASS_VAR,
	   SCLASS_AUTO,
	   EXPORT_LOCAL,
	   MTYPE_To_TY(WN_rtype(expr)));
  return retval;
}

// need to keep track of what kind of constraint a numeric constraint
// refers to (by address or not).  So keep list of constraints.

static char *operand_constraint_array[MAX_RECOG_OPERANDS];

static BOOL
constraint_by_address (const char *s)
{
#ifdef TARG_ST
  static const char modifiers[] = "=&%+";
  /* (cbr) in case of error */
  if (!s)
    return FALSE;
  while (*s != '\0' && strchr(modifiers, *s)) {
    s++;
  }
  //TB: Dynamic register files: scan first because the register
  //nickname might contain any character with a specific meaning
  if (strlen(s) > 1) {
    return FALSE;
  }
#endif

  if (strchr (s, 'm')) {
    return TRUE;
  }
  else if (isdigit(*s)) {
    return constraint_by_address (operand_constraint_array[*s - '0']);
  }
  else {
    return FALSE;
  }
}

static WN *
add_offset(WN_OFFSET  ofst,
	   WN        *address)	// not const; some simplification may occur.
{
  return WN_Binary(OPR_ADD, Pointer_Mtype,
		   WN_Intconst(MTYPE_I8, ofst),
		   address);
}

static WN *
address_of (const WN *wn)
{
  if (WN_operator(wn) == OPR_ILOAD ||
      WN_operator(wn) == OPR_MLOAD) {
    return add_offset(WN_offset(wn), WN_kid0(wn));
  }
  else if ((WN_operator(wn) == OPR_LDID) &&
	   (ST_sclass(WN_st(wn)) != SCLASS_REG)) {
    return WN_Lda (Pointer_Mtype,
		   WN_offset(wn),
		   WN_st(wn),
		   (UINT) 0);
  }
  // No address for this object. This expression is not an lvalue.
  return NULL;
}

/* What OPR_ASM looks like:
 *
 *   Kids: 0 is a block of IDNAMEs referring to
 *         registers that get clobbered. Clobbering of memory and
 *         condition codes is encoded in WN_Asm_Clobbers_Cc() and
 *         WN_Asm_Clobbers_Mem().
 *       1 is a block of PRAGMA or XPRAGMA nodes giving information
 *         about copy-out output operands and their constraints.
 *       2 .. WN_kid_count() - 1 are OPR_ASM_INPUT nodes, each of
 *         which gives a constraint and an rvalue for the
 *         corresponding input to the asm statement.
 *
 * Inputs originate either as input operands to the ASM, or as output
 * operands that are passed by address.
 */

static PREG_NUM asm_neg_preg = -2;

void
Wfe_Expand_Asm_Operands (tree  string,
			 tree  outputs,
			 tree  inputs,
			 tree  clobbers,
			 int   vol,
			 const char *filename,
			 int   line)
{
  // filename and line are ignored for now; eventually maybe they
  // should be used to generate SRCPOS information on the OPR_ASM_STMT
  // WN.
  //
  // I don't know yet why filename and line are passed for
  // expand_asm_operands but not for other expand_* routines in
  // gnu/stmt.c.

  int ninputs = list_length (inputs);

#ifdef TARG_ST
  /* (cbr) ASM with no outputs needs to be treated as volatile */
  if (!vol && !outputs)
    vol = 1;
#endif

  tree tail;
  char *constraint_string;

  // Keep list of output operand constraints so that we know
  // what a numeric constraint refers to.
  int i = 0;
  // Store the constraint strings
  for (tail = outputs; tail; tail = TREE_CHAIN (tail)) {
    constraint_string = TREE_STRING_POINTER (TREE_PURPOSE (tail));
    operand_constraint_array[i] = constraint_string;
    ++i;
  }
  FmtAssert(i < MAX_RECOG_OPERANDS, ("Too many asm operands"));
  for ( ; i < MAX_RECOG_OPERANDS; ++i) {
    operand_constraint_array[i] = NULL;
  }
  
  // Each occurrence of the "+" constraint modifier is converted to a
  // numeric matching constraint on a new input. In the following
  // loop, we count the number of "+" constraint modifiers so we know
  // how many inputs there will be.
  //
  // Also for the time being we discard the entire ASM construct if
  // there is a constraint we don't recognize. This is so we can
  // test-compile code containing ASM statements that apply to targets
  // we don't support. At the moment, we support only "r", "f", and
  // "m" constraints for IA-64, so those are the only ones on which we
  // don't barf. Ideally we would check with some target-specific
  // routine to see which constraints are valid, but we don't want to
  // link gccfe with targ_info or other similar stuff for now.
  for (tail = outputs;
       tail;
       tail = TREE_CHAIN (tail))
    {
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) fix */
      constraint_string = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (tail)));
#else
      constraint_string = TREE_STRING_POINTER (TREE_PURPOSE (tail));
#endif

      if (strchr (constraint_string, '+') ||
	  constraint_by_address (constraint_string))
	{
	  ++ninputs;
	}
#if !defined (TARG_ST) || (GNU_FRONT_END!=33)
  /* (cbr) gcc 3.3 upgrade */
      if (flag_bad_asm_constraint_kills_stmt &&
	  !constraint_supported (constraint_string)) {
	DevWarn ("Unrecognized constraint %s; "
		 "asm statement at line %d discarded",
		 constraint_string, lineno);
	return;
      }
#endif
    }

  WN *asm_wn = WN_CreateAsm_Stmt (ninputs + 2,
				  TREE_STRING_POINTER (string));

  WN *clobber_block = WN_CreateBlock ();

  WN_kid0(asm_wn) = clobber_block;

  for (tail = clobbers; tail; tail = TREE_CHAIN (tail))
    {
      char *clobber_string =
	TREE_STRING_POINTER (TREE_VALUE (tail));

      WN *clobber_pragma = NULL;

      int gcc_reg = decode_reg_name(clobber_string);
      if (gcc_reg == -3)
	WN_Set_Asm_Clobbers_Cc(asm_wn);
      else if (gcc_reg == -4)
	WN_Set_Asm_Clobbers_Mem(asm_wn);
      else {
	WN *clobbered_idname = idname_from_regnum (gcc_reg);

      	if (clobbered_idname) {
	  // This is a clobbered register that can be expressed as a
	  // WHIRL dedicated PREG.

	  ST *clobber_st = New_ST(CURRENT_SYMTAB);
	  ST_Init(clobber_st,
		Str_To_Index (Save_Str (clobber_string),
			      Current_Strtab),
		CLASS_NAME,
		SCLASS_UNKNOWN,
		EXPORT_LOCAL,
		(TY_IDX) 0);

	  clobber_pragma = WN_CreateXpragma (WN_PRAGMA_ASM_CLOBBER,
			    ST_st_idx(clobber_st),
			    1);
	  WN_kid0 (clobber_pragma) = clobbered_idname;
      	}
      	else {
	  // This is a clobbered register that cannot be expressed as a
	  // WHIRL dedicated PREG. Make the "asm" volatile because it
	  // clobbers something WHIRL can't see.

	  ST *clobber_st = New_ST(CURRENT_SYMTAB);
	  ST_Init(clobber_st,
		Str_To_Index (Save_Str (clobber_string),
			      Current_Strtab),
		CLASS_NAME,
		SCLASS_UNKNOWN,
		EXPORT_LOCAL,
		(TY_IDX) 0);

	  clobber_pragma = WN_CreatePragma (WN_PRAGMA_ASM_CLOBBER,
			   ST_st_idx(clobber_st),
			   (INT32) 0,
			   (INT32) 0);

	  WN_Set_Asm_Volatile (asm_wn);
        }
      }

      if (clobber_pragma != NULL)
      	WN_INSERT_BlockAfter (clobber_block,
			    WN_last (clobber_block),
			    clobber_pragma);
    }

  WN *output_constraint_block = WN_CreateBlock ();

  WN_kid1(asm_wn) = output_constraint_block;

  i = 2;

  // Expand the by-address output operands before appending the
  // ASM_STMT node so side effects of these operands occur in the
  // right place.
  UINT32 opnd_num = 0;

  for (tail = outputs;
       tail;
       tail = TREE_CHAIN (tail))
    {
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) fix */
      constraint_string = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (tail)));
#else
      constraint_string = TREE_STRING_POINTER (TREE_PURPOSE (tail));
#endif

      if (constraint_by_address(constraint_string)) {
	// This operand is by address, and gets represented as an
	// ASM_INPUT even though the user told us it's an output.
	WN *lhs_rvalue = WFE_Expand_Expr(TREE_OPERAND(tail, 0));
	WN *addr_of_lvalue = address_of(lhs_rvalue);
	FmtAssert(addr_of_lvalue != NULL,
		  ("WFE_Expand_Asm_Operands: output operand must be lvalue"));
#ifdef TARG_ST
	// [CG] May be an expression that requires address saved flag.
	WFE_Set_ST_Addr_Saved (addr_of_lvalue);
#endif
	WN_kid (asm_wn, i) =
	  WN_CreateAsm_Input (constraint_string, opnd_num, addr_of_lvalue);
	++i;
      }
      ++opnd_num;
    }

#ifdef TARG_ST
  UINT32 input_num = 0;
#endif
  for (tail = inputs;
       tail;
       tail = TREE_CHAIN (tail)
#ifdef TARG_ST
         , ++input_num
#endif
       )
    {
      if (TREE_PURPOSE (tail) == NULL_TREE)
	{
	  Fail_FmtAssertion ("hard register `%s' listed as "
			     "input operand to `asm'",
			     TREE_STRING_POINTER (TREE_VALUE (tail)) );
	  return;
	}

#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) fix */
      constraint_string = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (tail)));
#else
      constraint_string = TREE_STRING_POINTER (TREE_PURPOSE (tail));
#endif
      
#if !defined (TARG_ST) || (GNU_FRONT_END!=33)
      /* (cbr) gcc 3.3 upgrade */
      if (flag_bad_asm_constraint_kills_stmt &&
	  !constraint_supported (constraint_string)) {
	DevWarn ("Unrecognized constraint %s; "
		 "asm statement at line %d discarded",
		 constraint_string, lineno);
	return;
      }
#endif

      WN *input_rvalue = WFE_Expand_Expr (TREE_VALUE (tail));
#ifdef TARG_ST
      // [TTh] Add missing conversion in case of equivalent types
      TYPE_ID constraint_type = Get_Extension_MTYPE_From_Asm_Constraints(constraint_string);
      if (constraint_type != MTYPE_UNKNOWN &&
	  constraint_type != WN_rtype(input_rvalue)) {
	if (EXTENSION_Are_Equivalent_Mtype(constraint_type, WN_rtype(input_rvalue))) {
	  input_rvalue = WN_Cvt (WN_rtype(input_rvalue), constraint_type, input_rvalue);
	}
	else {
	  error ("Mismatch between the asm statement constraint '%s' and the argument type for operand %d",
		 constraint_string, input_num+1);
	  return;
	}
      }

      //[TB]: Add dynamic mtype checking
      if (!Check_Asm_Constraints(constraint_string, WN_rtype(input_rvalue))) {
	error ("Unrecognized asm statement constraint '%s' for operand %d",
	       constraint_string, input_num+1);
	return;
      }
#endif
      if (constraint_by_address(constraint_string)) {
	WN *addr_of_rvalue = address_of(input_rvalue);
	if (addr_of_rvalue != NULL) {
	  // Pass the address of the input rvalue, because the
	  // constraint says we pass the operand by its address.
	  input_rvalue = addr_of_rvalue;
	}
	else {
	  // Create a temporary to hold the value of the expression,
	  // and pass the address of that temporary.
	  ST *temp_st = st_of_new_temp_for_expr(input_rvalue);
	  WN *store_wn = WN_Stid(WN_rtype(input_rvalue),
				 (WN_OFFSET) 0,
				 temp_st,
				 // We may want to get high-level type
				 // of the RHS in the cases where that
				 // information exists, but for now,
				 // just put the low-level type on the
				 // store.
				 MTYPE_To_TY(WN_rtype(input_rvalue)),
				 input_rvalue);
	  WFE_Stmt_Append (store_wn, Get_Srcpos ());
	  input_rvalue = WN_Lda (Pointer_Mtype,
				 (WN_OFFSET) 0,
				 temp_st,
				 (UINT) 0);
	}
      }

#ifdef TARG_ST
      // [CG] May be an expression that requires address saved flag.
      WFE_Set_ST_Addr_Saved (input_rvalue);
#endif
      WN_kid (asm_wn, i) =
	WN_CreateAsm_Input (constraint_string, opnd_num, input_rvalue);
      ++i;
      ++opnd_num;
    }

  // Is Get_Srcpos the right thing to use?
  WFE_Stmt_Append (asm_wn, Get_Srcpos ());

  // Side effects of copy-out operands occur after the asm. Kind of
  // weird, but that's what GCC does.
  opnd_num = 0;
  for (tail = outputs;
       tail;
       tail = TREE_CHAIN (tail), ++opnd_num)
    {
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) fix */
      constraint_string = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (tail)));
#else
      constraint_string = TREE_STRING_POINTER (TREE_PURPOSE (tail));
#endif

      if (!constraint_by_address(constraint_string)) {
	// This operand is copy-in/copy-out.

	BOOL plus_modifier = (strchr (constraint_string, '+') != NULL);

	char input_opnd_constraint[8];

	if (plus_modifier)
	  {
	    // de-plus the output operand's constraint string.
	    constraint_string = remove_plus_modifier(constraint_string);

	    // Make up a numeric matching constraint string for the
	    // input operand we're going to add.
	    sprintf(input_opnd_constraint, "%d", opnd_num);
	  }

#ifdef TARG_ST
	// [TTh] (Code moved from few lines below)
	// Compute the ST used as the base for the negative PREG
	// reference in the output operand. This duplicates work done in
	// WFE_Lhs_Of_Modify_Expr.
	TYPE_ID desc = TY_mtype (Get_TY (TREE_TYPE (TREE_VALUE (tail))));

	// [TTh] Add missing conversion in case of equivalent types
	TY_IDX component_type = (TY_IDX)0;
	{
	  TYPE_ID constraint_type = Get_Extension_MTYPE_From_Asm_Constraints(constraint_string);
	  if (constraint_type != MTYPE_UNKNOWN &&
	      constraint_type != desc) {
	    if (EXTENSION_Are_Equivalent_Mtype(constraint_type, desc)) {
	      component_type = MTYPE_To_TY(constraint_type);
	      desc = constraint_type;
	    }
	    else {
	      error ("Mismatch between the asm statement constraint '%s' and the argument type for result %d",
		     constraint_string, opnd_num+1);
	      return;
	    }
	  }
	}
#endif

	WN *output_rvalue_wn = WFE_Lhs_Of_Modify_Expr (MODIFY_EXPR,
						       TREE_VALUE (tail),
						       plus_modifier,
#ifdef TARG_ST
						       (TY_IDX) component_type, // component type
#else
						       (TY_IDX) 0, // component type
#endif
						       (INT64) 0,  // component offset
						       (UINT32) 0, // field ID
						       FALSE,      // is bit field?
						       NULL,       // dummy rhs kid
						       asm_neg_preg, // preg num
						       FALSE,      // is realpart
						       FALSE);     // is imagpart

	if (plus_modifier)
	  {
#ifdef TARG_ST
	    // [CG] May be an expression that requires address saved flag.
	    WFE_Set_ST_Addr_Saved (output_rvalue_wn);
#endif

	    WN_kid (asm_wn, i) =
	      WN_CreateAsm_Input (input_opnd_constraint,
				  opnd_num,
				  output_rvalue_wn);
	    ++i;
	  }

#ifdef TARG_ST
	//[TB]: Add dynamic mtype checking
	if (!Check_Asm_Constraints(constraint_string, desc)) {
	  error ("Unrecognized asm statement constraint '%s' for result %d",
		 constraint_string,  opnd_num+1);
	  return;
	}
#else
	// Compute the ST used as the base for the negative PREG
	// reference in the output operand. This duplicates work done in
	// WFE_Lhs_Of_Modify_Expr.
	TYPE_ID desc = TY_mtype (Get_TY (TREE_TYPE (TREE_VALUE (tail))));
#endif
	ST *preg_st = MTYPE_To_PREG(desc);

	ST *constraint_st = New_ST(CURRENT_SYMTAB);
	ST_Init(constraint_st,
		Str_To_Index (Save_Str (constraint_string),
			      Current_Strtab),
		CLASS_NAME,
		SCLASS_UNKNOWN,
		EXPORT_LOCAL,
		(TY_IDX) 0);

	WN *constraint_pragma =
	  WN_CreatePragma (WN_PRAGMA_ASM_CONSTRAINT,
			   (ST_IDX) ST_st_idx(preg_st),
			   (INT32) ST_st_idx(constraint_st),
			   asm_neg_preg,
			   opnd_num);

	WN_INSERT_BlockAfter (output_constraint_block,
			      WN_last (output_constraint_block),
			      constraint_pragma);
	--asm_neg_preg;
      }
    }

  if (vol)
    {
      WN_Set_Asm_Volatile (asm_wn);
    }
}

void
WFE_Check_Undefined_Labels (void)
{
  INT32 i;
  for (i = undefined_labels_i; i >= 0; --i) {
    LABEL_IDX  label_idx  = undefined_labels_stack [undefined_labels_i].label_idx;
    SYMTAB_IDX symtab_idx = undefined_labels_stack [undefined_labels_i].symtab_idx;
//  fprintf (stderr, "WFE_Check_Undefined_Labels: %d idx = %8x [%d]\n", i, label_idx, symtab_idx);
    if (symtab_idx < CURRENT_SYMTAB)
      break;
    FmtAssert (undefined_labels_stack [undefined_labels_i].defined,
               ("label not defined within current function scope"));
  }
  undefined_labels_i = i;
} /* WFE_Check_Undefined_Labels */

