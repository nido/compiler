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
#include "defs.h"
#include "errors.h"
#include "gnu_config.h"
  // [HK]
#if __GNUC__ >=3
#include "wn.h"
#endif // __GNUC__ >=3
extern "C" {
#include "gnu/flags.h"
#include "gnu/system.h"
#include "gnu/toplev.h"
#include "gnu/tree.h"
#ifdef TARG_ST
  /* (cbr) fix includes */
#include "gnu/cp/cp-tree.h"
#include "gnu/real.h"
#include "tm_p.h"
#else
#include "cp-tree.h"
#endif
#include "c-pragma.h"
}
#undef TARGET_PENTIUM // hack around macro definition in gnu
#include "glob.h"
  // [HK]
#if __GNUC__ < 3
#include "wn.h"
#endif // __GNUC__ <3
#include "wn_util.h"
#include "symtab.h"
#include "const.h"
#include "pu_info.h"
#include "ir_bwrite.h"
#include "ir_reader.h"
#include "wfe_decl.h"
#include "wfe_misc.h"
#include "wfe_dst.h"
#include "tree_symtab.h"
#include "wfe_expr.h"
#include "wfe_stmt.h"
#ifdef KEY
#include "wfe_dst.h" // DST_enter_member_function
#endif

#ifdef TARG_ST
#include <set>
#include "wfe_pragmas.h"
#include "wfe_loader.h" //[TB] For Map_Reg_To_Preg
extern "C" {
#include "gnu/tree-inline.h"
}
#endif

//#define WFE_DEBUG

#ifdef TARG_ST
// [CG] Handle volatile field accesses. See comments
// in the implementation
static void function_update_volatile_accesses(WN *func_wn);
#endif

extern "C" void check_gnu_errors (int *, int *);

static tree *deferred_function_stack;
static INT32 deferred_function_i;
static INT32 deferred_function_max;

static void 
Init_Deferred_Function_Stack()
{
  deferred_function_max   = 32;
  deferred_function_i     = -1;
  deferred_function_stack =
      // [HK]
#if __GNUC__ >= 3
    (tree *) xmalloc (sizeof (tree) * deferred_function_max);
#else
    (tree *) malloc (sizeof (tree) * deferred_function_max);
#endif // __GNUC__ >= 3
}

void
Push_Deferred_Function (tree decl)
{
  //fprintf(stderr, "Push_Deferred_Function 1: %p %s\n", decl, IDENTIFIER_POINTER(DECL_NAME(decl)));
#ifdef TARG_ST
  /* [SC] Find cases where we come here before init. deferred_function_stack */
  FmtAssert (deferred_function_stack, ("deferred_function_stack uninitialized"));
  /* (cbr) Init_Deferred_Function_Stack has not be called in case of error */
  if (!deferred_function_stack) { 
    return;
  }
#endif

  for (INT32 i = deferred_function_i; i != -1; --i)
    if (deferred_function_stack [i] == decl)
      return;
  //fprintf(stderr, "Push_Deferred_Function 2: %p %s\n", decl, IDENTIFIER_POINTER(DECL_NAME(decl)));
//fprintf(stderr, "Push_Deferred_Function 2: %s\n", IDENTIFIER_POINTER(DECL_NAME(decl)));
  if (++deferred_function_i == deferred_function_max) {
    deferred_function_max = 2 * deferred_function_max;
    deferred_function_stack =
      // [HK]
#if __GNUC__ >= 3
      (tree *) xrealloc (deferred_function_stack,
#else
      (tree *) realloc (deferred_function_stack,
#endif // __GNUC__ >= 3
			deferred_function_max * sizeof(tree));
  }

  deferred_function_stack[deferred_function_i] = decl;
}

static tree 
Pop_Deferred_Function (void)
{
  tree decl;
  decl = deferred_function_stack[deferred_function_i--];
//fprintf(stderr, "Pop_Deferred_Function: %p %s\n", decl, IDENTIFIER_POINTER(DECL_NAME(decl)));
  return decl;
}

static tree *deferred_decl_init_stack;
static INT32 deferred_decl_init_i;
static INT32 deferred_decl_init_max;

static void 
Init_Deferred_Decl_Init_Stack ()
{
  deferred_decl_init_max   = 32;
  deferred_decl_init_i     = -1;
  deferred_decl_init_stack =
      // [HK]
#if __GNUC__ >= 3
    (tree *) xmalloc (sizeof (tree) * deferred_decl_init_max);
#else
    (tree *) malloc (sizeof (tree) * deferred_decl_init_max);
#endif // __GNUC__ >= 3
} /* Init_Deferred_Decl_Init_Stack */

void
Push_Deferred_Decl_Init (tree decl)
{
  if (++deferred_decl_init_i == deferred_decl_init_max) {
    deferred_decl_init_max = 2 * deferred_decl_init_max;
    deferred_decl_init_stack =
      // [HK]
#if __GNUC__ >= 3
      (tree *) xrealloc (deferred_decl_init_stack,
#else
      (tree *) realloc (deferred_decl_init_stack,
#endif // __GNUC__ >= 3
			deferred_decl_init_max * sizeof(tree));
  }

  deferred_decl_init_stack[deferred_decl_init_i] = decl;
} /* Push_Deferred_Decl_Init */

static tree
Pop_Deferred_Decl_Init (void)
{
  tree decl;
  decl = deferred_decl_init_stack[deferred_decl_init_i--];
  return decl;
} /* Pop_Deferred_Decl_Init */

extern PU_Info *PU_Tree_Root;
static PU_Info *PU_Info_Table     [258] = {0};
static ST      *Return_Address_ST [258] = {0};
static BOOL map_mempool_initialized = FALSE;
static MEM_POOL Map_Mem_Pool;

static tree curr_func_decl = NULL_TREE;

static int __ctors = 0;
static int __dtors = 0;

static void Set_Current_Function_Decl(tree decl)
{
  curr_func_decl = decl;
}

tree Current_Function_Decl(void) {return curr_func_decl;}

// void (*back_end_hook) (tree) = &WFE_Expand_Decl;

#ifdef KEY
// A stack of entry WN's.  The current function's entry WN is at the top of
// stack.
static std::vector<WN*> curr_entry_wn;
static void Push_Current_Entry_WN(WN *wn) { curr_entry_wn.push_back(wn); }
static void Pop_Current_Entry_WN() { curr_entry_wn.pop_back(); }
WN *Current_Entry_WN(void) { return curr_entry_wn.back(); }

// Catch-all for all the functions that g++ turns into assembler, so that we
// won't miss any while translating into WHIRL.
std::vector<tree> gxx_emitted_decls;
// Any typeinfo symbols that we have determined we should emit
std::vector<tree> emit_typeinfos;
// Any var_decls or type_decls that we want to expand last.
std::vector<tree> emit_decls;
// Any asm statements at file scope
std::vector<char *> gxx_emitted_asm;
// Struct fields whose type we want to set last.
std::vector<std::pair<tree, FLD_HANDLE> > defer_fields;

void
gxx_emits_decl(tree t) {
  const char *n = IDENTIFIER_POINTER(DECL_NAME(t));
#if 0
  fprintf (stderr, "gxx_emits_decl: %s", n);
  if (DECL_NAME(t) && DECL_ASSEMBLER_NAME_SET_P(t)) {
    fprintf (stderr, "(%s)", IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(t)));
  }
  fprintf (stderr, "\n");
#endif
  gxx_emitted_decls.push_back(t);
}

void
gxx_emits_typeinfos (tree t) {
  const char *n = IDENTIFIER_POINTER(DECL_NAME(t));
  //  fprintf (stderr, "gxx_emits_typeinfos: %s\n", n);
  emit_typeinfos.push_back (t);
}

void
defer_decl (tree t) {
  emit_decls.push_back (t);
}

void
gxx_emits_asm (char *str) {
  gxx_emitted_asm.push_back (str);
}

void
defer_field (tree t, FLD_HANDLE fld) {
  defer_fields.push_back (std::make_pair(t, fld));
}

// Support defer creating DST info until all types are created.
typedef struct {
  union {
    struct {		// for is_member_function = 1
      tree context;
      tree fndecl;
    } member_func;
  } u1;
  int is_member_function : 1;
} DST_defer_misc_info;

// List of things that we need to defer creating DST info for.
std::vector<DST_defer_misc_info *> defer_DST_misc;

typedef struct {
  tree t;
  TY_IDX ttidx;
  TY_IDX idx;
} DST_defer_type_info;

// List of types that we defer creating DST info for.
std::vector<DST_defer_type_info *> defer_DST_types;

void
defer_DST_type (tree t, TY_IDX ttidx, TY_IDX idx)
{
  DST_defer_type_info *p =
    (DST_defer_type_info *) calloc(1, sizeof(DST_defer_type_info));
  p->t = t;
  p->ttidx = ttidx;
  p->idx = idx;
  defer_DST_types.push_back(p);
}
#endif

#ifdef TARG_ST
static void WFE_Generate_Thunk (tree decl);  /* forward */
#endif

#ifdef TARG_ST
static tree
take_address (tree t)
{
  if (TREE_CODE (t) == COMPONENT_REF) {
    tree base = take_address (TREE_OPERAND (t, 0));
    tree field = TREE_OPERAND (t, 1);
    tree offset = DECL_FIELD_OFFSET (field);
    return build (PLUS_EXPR, build_pointer_type(TREE_TYPE(t)), base, offset);
  } else {
    return build1 (ADDR_EXPR,
		   build_pointer_type(TREE_TYPE(t)), t);
  }
}

 class Target_Expr_Item {
 private:
   const tree target;
   bool stabilized;
   bool stabilize_change;
   bool written;
   std::set<tree> target_aliases;
 public:
   Target_Expr_Item (tree t) : target(t), stabilized(false),
			       stabilize_change(false), written(false) {}
   tree Use () {
     FmtAssert (Target_Set_p (), ("Target_Expr_Info.Use() with no target set"));
     if (! stabilized) {
       tree stabilized_target = stabilize_reference (target);
       stabilize_change = (stabilized_target != target);
       stabilized = true;
     }
     return target;
   }
   bool Target_Set_p () {
     return target != NULL_TREE;
   }
   void Add_Alias (tree slot) {
     target_aliases.insert(slot);
   }
   void Remove_Alias (tree slot) {
     target_aliases.erase(slot);
   }
   bool Is_Alias_p (tree slot) {
     return target_aliases.find(slot) != target_aliases.end();
   }
   bool Stabilize_Change_p () {
     return stabilize_change;
   }
   bool Written_p () {
     return written;
   }
   void Set_Written (bool v) {
     written = v;
   }
   tree Target () {
     FmtAssert (Target_Set_p (), ("Target_Expr_Item.Target() with no target set"));
     return target;
   }
 };   

 class Target_Expr_Info {
 private:
   std::set<tree> visited_call_exprs;
   // target_item_stack is basically a stack of target items, where the
   // top of the stack is the current target item.  We sometimes want to
   // iterate over all the target items on the stack, so we use
   // std::vector rather than std:stack.
   std::vector<Target_Expr_Item *> target_item_stack;
 public:
   Target_Expr_Info () {
     visited_call_exprs.clear();
     Push_Target (NULL_TREE);
   }
   bool Visited_Call_Expr_p (tree t) {
     return visited_call_exprs.find(t) != visited_call_exprs.end();
   }
   void Add_Visited_Call_Expr (tree t) {
     visited_call_exprs.insert(t);
   }
   bool Target_Set_p () {
     return target_item_stack.back()->Target_Set_p();
   }
   bool Written_p () {
     return target_item_stack.back()->Written_p();
   }
   void Set_Written (bool v) {
     target_item_stack.back()->Set_Written (v);
   }
   bool Stabilize_Change_p () {
     return target_item_stack.back()->Stabilize_Change_p();
   }
   tree Target () {
     return target_item_stack.back()->Target();
   }
   void Add_Alias (tree slot) {
     target_item_stack.back()->Add_Alias(slot);
   }
   void Remove_Alias (tree slot) {
     target_item_stack.back()->Remove_Alias(slot);
   }
   void Push_Target (tree t) {
     target_item_stack.push_back (new Target_Expr_Item (t));
   }
   void Pop_Target () {
     delete target_item_stack.back();
     target_item_stack.pop_back ();
   }
   tree Use () {
     return target_item_stack.back()->Use();
   }
   tree Substitute (tree t) {
     std::vector<Target_Expr_Item *>::reverse_iterator it;
     for (it = target_item_stack.rbegin (); it != target_item_stack.rend (); it++) {
       Target_Expr_Item *item = *it;
       if (item->Is_Alias_p (t)) {
	 return item->Use ();
       }
     }
     return t;
   }
   void Write (tree t) {
     std::vector<Target_Expr_Item *>::reverse_iterator it;
     for (it = target_item_stack.rbegin (); it != target_item_stack.rend (); it++) {
       Target_Expr_Item *item = *it;
       if (item->Target_Set_p() && item->Target() == t) {
	 item->Set_Written (true);
	 break;
       }
     }
   }
 };
   
static void simplify_target_exprs (tree *tp, Target_Expr_Info &target_expr_info);

static tree
simplify_target_exprs_r (tree *tp,
			 int *walk_subtrees,
			 void *data) {
  Target_Expr_Info *target_expr_info = (Target_Expr_Info *)data;
  tree t = *tp;
  if (TYPE_P (t)) {
    *walk_subtrees = 0;
    return NULL_TREE;
  }

  tree_code code = TREE_CODE (t);
  switch (code) {
  case TARGET_EXPR:
    {
      *walk_subtrees = 0;
      /* Here it is indeterminate whether the expression is operand 1 or
	 operand 3 (because gcc rtl generation may have moved it),
	 so normalize it to operand 1. */
      if (TREE_OPERAND (t, 1) == NULL_TREE) {
	TREE_OPERAND (t, 1) = TREE_OPERAND (t, 3);
	TREE_OPERAND (t, 3) = NULL_TREE;
      }
      tree slot = TREE_OPERAND (t, 0);
      if (target_expr_info->Target_Set_p ()) {
	/* This is a "normal" TARGET_EXPR.  We should treat SLOT as
	   an alias of the real target.  No cleanup is required. */
	target_expr_info->Add_Alias (slot);
	/* Here assert that the types of target_expr_info->target and
	   TREE_TYPE (t) are compatible. */
	tree target = target_expr_info->Target ();
	FmtAssert (TYPE_MAIN_VARIANT (TREE_TYPE (t))
		   == TYPE_MAIN_VARIANT (TREE_TYPE (target)),
		   ("Type mismatch in simplify_target_exprs_r"));
	bool old_written = target_expr_info->Written_p ();
	target_expr_info->Set_Written (false);
	simplify_target_exprs (& TREE_OPERAND (*tp, 1), *target_expr_info);	
	if (target_expr_info->Written_p ()) {
	  /* Target is written in the rhs, so we can remove this
	     TARGET_EXPR node completely. Remember no cleanup is required
	     for a "normal" target expr. */
	  *tp = TREE_OPERAND (t, 1);
	} else {
	  /* Rewrite this node to be a MODIFY_EXPR of the true target */
	  *tp = build (MODIFY_EXPR, TREE_TYPE (t),
		       target,
		       TREE_OPERAND (t, 1));
	  target_expr_info->Set_Written (old_written);
	}
	target_expr_info->Remove_Alias (slot);
      } else {
	/* This is an "orphaned" TARGET_EXPR.  SLOT is a real variable,
	   not a temporary.  SLOT becomes the new target for the expression
	   and the cleanup. */
	target_expr_info->Push_Target (slot);
	simplify_target_exprs (& TREE_OPERAND (t, 1), *target_expr_info);
	if (target_expr_info->Written_p ()) {
	  /* Put the expression as 3rd operand, as an indicator that the
	     target is written in the expression, so we do not have to assign
	     it when we evaluate this TARGET_EXPR node. */
	  TREE_OPERAND (t, 3) = TREE_OPERAND (t, 1);
	  TREE_OPERAND (t, 1) = NULL_TREE;
	}
	if (TREE_OPERAND(t, 2)) {
	  simplify_target_exprs (& TREE_OPERAND (t, 2), *target_expr_info);
	}
	target_expr_info->Pop_Target ();
      }
    }
    break;
  case INIT_EXPR:
  case MODIFY_EXPR:
  case PREDECREMENT_EXPR:
  case PREINCREMENT_EXPR:
  case POSTDECREMENT_EXPR:
  case POSTINCREMENT_EXPR:
    {
      *walk_subtrees = 0;
      simplify_target_exprs (& TREE_OPERAND (t, 0), *target_expr_info);

      target_expr_info->Push_Target (TREE_OPERAND (t, 0));
      simplify_target_exprs (& TREE_OPERAND (t, 1), *target_expr_info);
      if (target_expr_info->Written_p ()) {
	/* If we can guarantee the target is written in the rhs, then
	   there is no need to write to the target here, so here just evaluate
	   for side-effects.
	   This is used to avoid targets with copy constructors, where we do
	   not want to assign multiple times. */
	if (target_expr_info->Stabilize_Change_p ()
	    && TREE_SIDE_EFFECTS(target_expr_info->Target ())) {
	      *tp = build (COMPOUND_EXPR, TREE_TYPE (t),
			   target_expr_info->Target(),
			   TREE_OPERAND (t, 1));
	} else {
	  *tp = TREE_OPERAND (t, 1);
	}
      } else {
	if (target_expr_info->Stabilize_Change_p ()
	    && TREE_SIDE_EFFECTS(target_expr_info->Target ())) {
	  /* We used the target somewhere inside the rhs, so we should precompute
	     the stabilized form. */
	  *tp = build (COMPOUND_EXPR, TREE_TYPE (t),
		       target_expr_info->Target(),
		       build (code, TREE_TYPE (t),
			      target_expr_info->Target(),
			      TREE_OPERAND (t, 1)));
	}
      }
      target_expr_info->Pop_Target ();
    }
    break;
  case VAR_DECL:
    /* This could be an alias introduced by a TARGET_EXPR.  If so, substitute
       the true target. */
    *tp = target_expr_info->Substitute (*tp);
    break;

  case CALL_EXPR:
    {
      *walk_subtrees = 0;
      simplify_target_exprs (& TREE_OPERAND (t, 0), *target_expr_info);

      /* An outer INIT/MODIFY target does not propagate down into TARGET_EXPRs
	 in the arglist.  However, the arglist may have references to
	 a variable defined in an outer TARGET_EXPR. */
      target_expr_info->Push_Target (NULL_TREE);
      simplify_target_exprs (& TREE_OPERAND (t, 1), *target_expr_info);
      target_expr_info->Pop_Target ();
      
      /* Set the written flag if this is a constructor of our target.
         Do this after transforming the args so that we do not have to check
	 for aliases. */
      tree callee_fndecl = get_callee_fndecl (t);
      tree first_arg = TREE_OPERAND(t, 1) ? TREE_VALUE(TREE_OPERAND(t, 1)) : NULL_TREE;
      if (callee_fndecl != NULL_TREE
	  && DECL_CONSTRUCTOR_P(callee_fndecl)
	  && first_arg != NULL_TREE
	  && TREE_CODE (first_arg) == ADDR_EXPR) {
	tree constructor_dest = TREE_OPERAND(first_arg, 0);
	target_expr_info->Write(constructor_dest);
      }

      /* Insert the return value address at the head of the args list. */
      if (RETURN_IN_MEMORY (TREE_TYPE(t)) ||
	  (TYPE_LANG_SPECIFIC(TREE_TYPE(t)) &&
	   TREE_ADDRESSABLE (TREE_TYPE(t)) && 
	   CLASSTYPE_NON_POD_P (TREE_TYPE(t)))) {  
	if (! target_expr_info->Visited_Call_Expr_p (t)) {
	  TREE_OPERAND (t, 1) = tree_cons (NULL_TREE,
					   take_address(target_expr_info->Use ()),
					   TREE_OPERAND(t, 1));
	}
	target_expr_info->Set_Written(true);
      }
      target_expr_info->Add_Visited_Call_Expr(t);
    }
    break;

  case COMPOUND_EXPR:
    /* target is not propagated into arg0 of COMPOUND_EXPR. */
    {
      *walk_subtrees = 0;
      target_expr_info->Push_Target (NULL_TREE);
      simplify_target_exprs (& TREE_OPERAND (t, 0), *target_expr_info);
      target_expr_info->Pop_Target ();
      simplify_target_exprs (& TREE_OPERAND (t, 1), *target_expr_info);
    }
    break;

  case SAVE_EXPR:
  case INDIRECT_REF:
  case COMPONENT_REF:
  case BIT_FIELD_REF:
  case ARRAY_RANGE_REF:
      /* target is not propagated into arg of these. */
    {
      *walk_subtrees = 0;
      target_expr_info->Push_Target (NULL_TREE);
      simplify_target_exprs (& TREE_OPERAND (t, 0), *target_expr_info);
      target_expr_info->Pop_Target ();
    }
    break;

  case ADDR_EXPR:
      /* target is not propagated into arg of this. */
    {
      tree old_operand = TREE_OPERAND (t, 0);
      *walk_subtrees = 0;
      target_expr_info->Push_Target (NULL_TREE);
      simplify_target_exprs (& TREE_OPERAND (t, 0), *target_expr_info);
      target_expr_info->Pop_Target ();
      tree new_operand = TREE_OPERAND (t, 0);
      if (new_operand != old_operand
	  && TREE_CODE (new_operand) == COMPONENT_REF) {
	*tp = take_address (new_operand);
      }
    }
    break;


  case ARRAY_REF:
      /* target is not propagated into arg of these. */
    {
      *walk_subtrees = 0;
      target_expr_info->Push_Target (NULL_TREE);
      simplify_target_exprs (& TREE_OPERAND (t, 0), *target_expr_info);
      simplify_target_exprs (& TREE_OPERAND (t, 1), *target_expr_info);
      target_expr_info->Pop_Target ();
    }
    break;
    
  case CONSTRUCTOR:
    {
      *walk_subtrees = 0;
      tree elt;
      for (elt = CONSTRUCTOR_ELTS (t); elt; elt = TREE_CHAIN (elt)) {
	tree field = TREE_PURPOSE (elt);
	tree value = TREE_VALUE (elt);
	tree component = (target_expr_info->Target_Set_p ()
			  ? build (COMPONENT_REF, TREE_TYPE(field),
				   target_expr_info->Target (),
				   field)
			  : NULL_TREE);
	target_expr_info->Push_Target (component);
        simplify_target_exprs (& TREE_VALUE (elt), *target_expr_info);
	target_expr_info->Pop_Target ();
      }
    }
    break;

  case COND_EXPR:
    {
      *walk_subtrees = 0;
      /* Take care with written flag here:
	 target is definitely written only if written in condition,
	 or BOTH conditional operands.
      */
      simplify_target_exprs (& TREE_OPERAND (t, 0), *target_expr_info);
      bool old_written = target_expr_info->Written_p ();
      target_expr_info->Set_Written (false);
      simplify_target_exprs (& TREE_OPERAND (t, 1), *target_expr_info);
      bool lhs_written = target_expr_info->Written_p ();
      target_expr_info->Set_Written (false);
      simplify_target_exprs (& TREE_OPERAND (t, 2), *target_expr_info);
      bool rhs_written = target_expr_info->Written_p ();
      target_expr_info->Set_Written (old_written
				     || (lhs_written && rhs_written));
    }
    break;

  case TRUTH_ANDIF_EXPR:
  case TRUTH_ORIF_EXPR:
    {
      *walk_subtrees = 0;
      /* Take care with written flag here:
	 target cannot be assumed written if only written in the
	 conditional operand. */
      simplify_target_exprs (& TREE_OPERAND (t, 0), *target_expr_info);
      bool old_written = target_expr_info->Written_p ();
      simplify_target_exprs (& TREE_OPERAND (t, 1), *target_expr_info);
      target_expr_info->Set_Written (old_written);
    }
    break;

  default:
    break;
  }
  return NULL_TREE;
}

// simplify_target_exprs transforms the TARGET_EXPR nodes in *TP so that
// they are easier to handle by the whirl generator.
// Ref. the gcc documentation for definition of "normal" and "orphaned"
// TARGET_EXPRs.
//
// This function removes all "normal" target exprs by substituting the
// real target for the target variable in the target expression, and
// transforming the TARGET_EXPR to a MODIFY_EXPR that assigns the real target.
// The target variable is completely removed.  In the case where we
// can determine that the real target is already assigned in the
// target expression, then no MODIFY_EXPR is necessary, and the target
// expression can be evaluated just for its side-effects.
//
// "Orphaned" target exprs are unchanged, except to indicate if the
// target is assigned in the target expression: the target expression is
// placed in operand 3 in that case, otherwise operand 1.  If the whirl
// generator finds the target expression in operand 3, then the
// target expression can be evaluated just for its side-effects, otherwise
// the target expression value should be assigned to the target variable.
//
// This function also transforms calls to return-in-memory, by adding
// a pointer to the result destination to the head of the argument list.
// Since nodes can be visited more than once, and the argument list should
// be transformed only once, it is necessary to remember which CALL_EXPR
// nodes have been visited.
static void
simplify_target_exprs (tree *tp, Target_Expr_Info &target_expr_info)
{
  walk_tree (tp, simplify_target_exprs_r, &target_expr_info, NULL);
}

#endif

// Return 1 if we actually expand decl.
static int
WFE_Expand_Function_Body (tree decl)
{
  tree body;

#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) don't emit functions if we don't need them */
  if (DECL_IGNORED_P (decl))
    return 0;
#endif

#ifdef KEY
  if (expanded_decl(decl) == TRUE)
    return 0;

  expanded_decl(decl) = TRUE;
#endif
  //  fprintf(stderr, "FUNCTION_DECL: %s\n", IDENTIFIER_POINTER(DECL_NAME(decl)));

  (void) WFE_Start_Function(decl);

  Set_Current_Function_Decl(decl);

#ifdef TARG_ST
  {
    Target_Expr_Info target_expr_info;
    simplify_target_exprs (& DECL_SAVED_TREE(decl), target_expr_info);
  }
#endif

  for (body = DECL_SAVED_TREE(decl); body; body = TREE_CHAIN(body))
    Mark_Scopes_And_Labels (body);

  for (body = DECL_SAVED_TREE(decl); body; body = TREE_CHAIN(body))
    WFE_Expand_Stmt(body);

  WFE_Finish_Function();

#ifdef TARG_ST
  /* (cbr) expand needed thunks */
  if (DECL_VIRTUAL_P (decl))
    {
      tree thunk;
      for (thunk = DECL_THUNKS (decl); thunk; thunk = TREE_CHAIN (thunk))
        WFE_Generate_Thunk (thunk);
    }
#endif

#ifdef TARG_ST
  /* (cbr) handle attribute */
  if (DECL_STATIC_CONSTRUCTOR (decl))
    WFE_Assemble_Constructor (decl);
  if (DECL_STATIC_DESTRUCTOR (decl))
    WFE_Assemble_Destructor (decl);
#endif

  return 1;
}

/*
 * WFE_Expand_Decl is called with the root of the g++ tree (always a
 * NAMESPACE_DECL) as argument and is thus the top-level routine in
 * tree-to-whirl tranlsation.  WFE_Expand_Decl is not called for
 * every declaration, but only for FUNCTION_DECL, NAMESPACE_DECL, 
 * TYPE_DECL, and VAR_DECL:  that is, for those declarations that
 * can appear in namespace scope.
 */

void WFE_Finish_Function(void);

static void
WFE_Generate_Thunk (tree decl)
{
#ifdef KEY
  if (expanded_decl(decl) == TRUE)
    return;
  expanded_decl(decl) = TRUE;
#endif

  Is_True(decl != NULL &&
          TREE_CODE(decl) == FUNCTION_DECL &&
          DECL_THUNK_P(decl) &&
          TREE_CODE(CP_DECL_CONTEXT(decl)) != NAMESPACE_DECL,
          ("Argument to WFE_Generate_Thunk isn't a thunk"));

  Is_True(DECL_INITIAL(decl) != NULL,
          ("Argument to WFE_Generate_Thunk has null DECL_INITIAL"));

#ifdef TARG_ST
  /* (cbr) Say we need to output it. */
  tree function = TREE_OPERAND (DECL_INITIAL(decl), 0);
  TREE_ADDRESSABLE (decl) = TREE_ADDRESSABLE (function) = 1;
  TREE_USED (decl) = TREE_USED (function) = 1;
  DECL_EXTERNAL (decl) = 0;
  DECL_COMDAT(decl) = DECL_COMDAT(function);
#endif

  ST      *thunk_st  = Get_ST (decl);
  ST      *func_st   = Get_ST (TREE_OPERAND (DECL_INITIAL(decl), 0));
#ifdef TARG_ST
  /* (cbr) set correct return type */
  TY_IDX ret_idx = TY_ret_type (ST_pu_type (func_st));
  TYPE_ID  ret_mtype = TY_mtype (ret_idx);
#else
  TYPE_ID  ret_mtype = TY_mtype (TY_ret_type (ST_pu_type (func_st)));
#endif
  WN      *entry_wn  = WFE_Start_Function (decl);
  INT32    nargs     = WN_kid_count (entry_wn) - 3;
  INT32    i;
  ST      *arg_st;
  TY_IDX   arg_ty;
  TYPE_ID  arg_mtype;
  WN      *arg_wn;
  WN      *wn;
  WN      *call_wn;

  // modify this parameter by the delta
#ifdef TARG_ST
  /* (cbr) returned in hidden param */
  tree type_tree = TREE_TYPE (function);
  if (RETURN_IN_MEMORY (TREE_TYPE(type_tree)) ||
      (TYPE_LANG_SPECIFIC(TREE_TYPE(type_tree)) &&
       TREE_ADDRESSABLE (TREE_TYPE(type_tree)) && 
       CLASSTYPE_NON_POD_P (TREE_TYPE(type_tree)))) {
    arg_st = WN_st (WN_kid1 (entry_wn));
    ret_mtype = MTYPE_V;
  }
  else
#endif
    arg_st = WN_st (WN_kid0 (entry_wn));

  arg_ty = ST_type (arg_st);
  arg_mtype = TY_mtype (arg_ty);

  // Pseudocode:
  //     this += delta;
  //     if (vcall_offset != 0)
  //       this += (*((ptrdiff_t **) this))[vcall_offset];
  wn = WN_Binary (OPR_ADD, arg_mtype,
                  WN_Ldid (arg_mtype, 0, arg_st, arg_ty),
                  WN_Intconst (arg_mtype, THUNK_DELTA(decl)));
  if (THUNK_VCALL_OFFSET(decl) != 0) {
    DevWarn ("Generating thunk with vcall adjustment at line %d\n", lineno);

    TY_IDX pdiff    = Get_TY(ptrdiff_type_node);
    TY_IDX p_pdiff  = Make_Pointer_Type(pdiff, FALSE);
    TY_IDX pp_pdiff = Make_Pointer_Type(p_pdiff, FALSE);

#if defined (TARG_ST) && (GNU_FRONT_END==33)
    /* (cbr) compute 'this' with voffset */
    WN* deref = WN_CreateIload(OPR_ILOAD,
                               TY_mtype(p_pdiff), TY_mtype(pp_pdiff),
                               0,
                               p_pdiff, pp_pdiff,
                               WN_COPY_Tree(wn));

    wn = WN_Binary (OPR_ADD, arg_mtype,
                    wn,
                    WN_CreateIload(OPR_ILOAD,
                                   TY_mtype(p_pdiff), TY_mtype(p_pdiff),
                                   tree_low_cst (THUNK_VCALL_OFFSET(decl),0),
                                   p_pdiff, p_pdiff,
                                   deref));
#else
    WN* deref = WN_CreateIload(OPR_ILOAD,
                               TY_mtype(p_pdiff), TY_mtype(pp_pdiff),
                               0,
                               p_pdiff, pp_pdiff,
                               WN_Tas(pp_pdiff, TY_mtype(pp_pdiff),
                                      WN_COPY_Tree(wn)));

    wn = WN_Binary (OPR_ADD, arg_mtype,
                    wn,
                    WN_CreateIload(OPR_ILOAD,
                                   TY_mtype(pdiff), TY_mtype(p_pdiff),
                                   THUNK_VCALL_OFFSET(decl),
                                   pdiff, p_pdiff,
                                   deref));
#endif
  }
  wn = WN_Stid (arg_mtype, 0, arg_st, arg_ty, wn);
  WFE_Stmt_Append (wn, Get_Srcpos());

  // generate call to base function
  call_wn = WN_Create (OPR_CALL, ret_mtype, MTYPE_V, nargs);
  WN_st_idx (call_wn) = ST_st_idx (func_st);
  WN_Set_Call_Default_Flags (call_wn);
  WN_Set_Call_Replace_By_Jump (call_wn);
  for (i = 0; i < nargs; i++) {
    arg_st = WN_st (WN_kid (entry_wn, i));
    arg_ty = ST_type (arg_st);
    arg_mtype = TY_mtype (arg_ty);
    arg_wn = WN_Ldid (arg_mtype, 0, arg_st, arg_ty);
    arg_wn = WN_CreateParm (Mtype_comparison (arg_mtype), arg_wn,
                            arg_ty, WN_PARM_BY_VALUE);
    WN_kid (call_wn, i) = arg_wn;
  }

  if (ret_mtype == MTYPE_V) {

    WFE_Stmt_Append (call_wn, Get_Srcpos());
    wn = WN_CreateReturn ();
#ifdef TARG_ST
    //TB: bug #31540 For thunk set return as a lowered return: we
    //don't want to emit: warning that control reaches end of non-void
    //function
    WN_is_return_val_lowered(wn) = TRUE;
#endif
    WFE_Stmt_Append (wn, Get_Srcpos());
  }

  else {

    WN *block_wn = WN_CreateBlock ();
    WN_INSERT_BlockLast (block_wn, call_wn);
#ifdef TARG_ST
    /* (cbr) set correct return type */
    wn = WN_Ldid (ret_mtype, -1, Return_Val_Preg, ret_idx);
#else
    wn = WN_Ldid (ret_mtype, -1, Return_Val_Preg, Be_Type_Tbl (ret_mtype));
#endif
    wn = WN_CreateComma (OPR_COMMA, Mtype_comparison (ret_mtype), MTYPE_V,
			 block_wn, wn);
    wn = WN_CreateReturn_Val (OPR_RETURN_VAL, ret_mtype, MTYPE_V, wn);
    WFE_Stmt_Append (wn, Get_Srcpos());
  }

  Set_PU_is_thunk (Get_Current_PU ());
  WFE_Finish_Function ();
}

static void process_local_classes()
{
  for (int i = 0; i < local_classes->elements_used; ++i) {
    tree t = VARRAY_TREE(local_classes, i);
      if (t->common.code == RECORD_TYPE ||
	  t->common.code == UNION_TYPE)
	Get_TY(t);
  }
}

void WFE_Expand_Decl(tree decl)
{
  Is_True(decl != NULL && TREE_CODE_CLASS(TREE_CODE(decl)) == 'd',
          ("Argument to WFE_Expand_Decl isn't a decl node"));

/*
  int error_count, sorry_count;
  if (decl == global_namespace) {
   check_gnu_errors (&error_count, &sorry_count);
   if (error_count || sorry_count)
     return;
    Init_Deferred_Function_Stack();
  }
*/
  switch (TREE_CODE(decl)) { 

    case CONST_DECL:
#ifndef TARG_ST
      /* (cbr) nothing to do */
      DevWarn("WFE_Expand_Decl:  don't know what to do with CONST_DECL");
#endif
      break;

    case FUNCTION_DECL:
      if (DECL_THUNK_P(decl) &&
          TREE_CODE(CP_DECL_CONTEXT(decl)) != NAMESPACE_DECL) {
        WFE_Generate_Thunk(decl);
      }
      else {
        tree body = DECL_SAVED_TREE(decl);
        
        //                char * name = (char *) IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl));
                //                printf ("EXPAND_DECL %s 0x%x\n", name, decl);

        if (body != NULL_TREE && !DECL_EXTERNAL(decl) &&
#ifndef KEY
	    // For now, emit all template-related funcs from GCC 3.2.  Fix
	    // the code when we have more time, 
            (DECL_TEMPLATE_INFO(decl) == NULL 		   ||
             DECL_FRIEND_PSEUDO_TEMPLATE_INSTANTIATION(decl) ||
             DECL_TEMPLATE_INSTANTIATED(decl) 		   ||
             DECL_TEMPLATE_SPECIALIZATION(decl)) &&
#endif
	     !DECL_MAYBE_IN_CHARGE_CONSTRUCTOR_P(decl) &&
	     !DECL_MAYBE_IN_CHARGE_DESTRUCTOR_P(decl)) {

#ifndef KEY	// Don't call Get_ST we don't have a PU to attach the ST to
		// yet.  It seems this Get_ST is unnecessary anyway because
		// Get_ST is called again in WFE_Start_Function, which is
		// called by WFE_Expand_Function_Body below.
         (void) Get_ST(decl);
#endif
          if (!Enable_WFE_DFE) {
            if (CURRENT_SYMTAB != GLOBAL_SYMTAB ||
                DECL_FUNCTION_MEMBER_P(decl)    ||
		strncmp (IDENTIFIER_POINTER(DECL_NAME(decl)), "__tcf", 5) == 0)
              Push_Deferred_Function (decl);
          else {
            WFE_Expand_Function_Body (decl);
#ifndef KEY
            while (deferred_function_i != -1)
              WFE_Expand_Function_Body (Pop_Deferred_Function ());
#endif
          }
         }
        }
      }

      break;

    case NAMESPACE_DECL: {
      /* We assume for now that there are no TREE_LIST nodes among
       * the namespace declarations.  
       */

      /*      if (decl == std_node)
	      break; // ignore namespace std */
      if (DECL_NAMESPACE_ALIAS(decl))
	break;
      tree subdecl;
      for (subdecl = cp_namespace_decls(decl);
	   subdecl != NULL_TREE;
	   subdecl = TREE_CHAIN(subdecl))
	WFE_Expand_Decl(subdecl);
      if (decl == global_namespace)
	process_local_classes(); 
#ifndef KEY
      while (deferred_function_i != -1) {
//      fprintf(stderr, "NAMESPACE_DECL: Pop_Deferred_Function\n");
	WFE_Expand_Function_Body (Pop_Deferred_Function ());
      }
#endif
      break;
    }

    case TEMPLATE_DECL: {
      if (DECL_CONTEXT(decl) && TREE_CODE(DECL_CONTEXT(decl)) == RECORD_TYPE)
	Get_TY(DECL_CONTEXT(decl));
      tree gentemp = most_general_template(decl);

      for (tree t = DECL_TEMPLATE_SPECIALIZATIONS(gentemp);
           t; t = TREE_CHAIN(t))
#ifdef TARG_ST
        /* (cbr) emit cloned cdtors */
	if (TREE_CODE(TREE_VALUE(t)) == FUNCTION_DECL &&
	    !DECL_EXTERNAL(TREE_VALUE(t))	      &&
	    !uses_template_parms (TREE_VALUE(t))) {
          WFE_Expand_Decl (TREE_VALUE(t));

          for (tree clone = TREE_CHAIN (TREE_VALUE(t));
               clone && DECL_CLONED_FUNCTION_P (clone);
               clone = TREE_CHAIN (clone)) {
            WFE_Expand_Decl (clone);
          }
        }
#else
	if (TREE_CODE(TREE_VALUE(t)) == FUNCTION_DECL &&
	    !DECL_EXTERNAL(TREE_VALUE(t))	      &&
	    !uses_template_parms (TREE_VALUE(t)))
          WFE_Expand_Decl (TREE_VALUE(t));
#endif

	DECL_TEMPLATE_SPECIALIZATIONS(gentemp) = 0; // don't do these twice

      for (tree t = DECL_TEMPLATE_INSTANTIATIONS(gentemp);
	   t; t = TREE_CHAIN(t)) {
	  tree val = TREE_VALUE(t);
	  if (TREE_CODE(val) == RECORD_TYPE &&
	      !uses_template_parms(val))
	    Get_TY(val);
      }

      break;
    }

    case TREE_VEC:
      break;
   
    case TYPE_DECL: {
      tree type = TREE_TYPE(decl);
      (void) Get_TY(type);
      break;
    }

  case VAR_DECL: {
#ifdef KEY
      expanded_decl(decl) = TRUE;
#endif
#ifdef TARG_ST
      /* (cbr) anon unions support */
      ST *st =  Get_ST(decl);

      if (ANON_AGGR_TYPE_P (TREE_TYPE(decl))) {
        for (tree t = DECL_ANON_UNION_ELEMS (decl); t; t = TREE_CHAIN (t)) {
          tree decl_elt = TREE_VALUE (t);
          tree cleanup_elt = TREE_PURPOSE (t);
          if (TREE_USED (decl_elt))
            TREE_USED (decl) = 1;

          DECL_ALIGN (decl_elt) = DECL_ALIGN (decl);
          DECL_USER_ALIGN (decl_elt) = DECL_USER_ALIGN (decl);

          set_DECL_ST(decl_elt, st);
        }
      }
#else
      (void) Get_ST(decl);
#endif

      if (DECL_INITIAL(decl) && !DECL_EXTERNAL(decl)) {
	tree init = DECL_INITIAL(decl);
      	if (TREE_CODE(init) == ERROR_MARK)
	  return;

	if (TREE_CODE(init) == (enum tree_code)PTRMEM_CST)  {
	  init = cplus_expand_constant(init);
	  DECL_INITIAL(decl) = init;
  	}

	if (TREE_CODE(init) == CONSTRUCTOR) {
  	  tree init_type = TREE_TYPE(init);
	  if (TREE_CODE(init_type) != RECORD_TYPE &&
	      TREE_CODE(init_type) != ARRAY_TYPE  &&
	      TREE_CODE(init_type) != UNION_TYPE)
	    return;
	  }

	WFE_Initialize_Decl(decl);
      }
      break;
  }

#ifdef TARG_ST
      /* (cbr) handle using_decl */
  case USING_DECL: {
    tree init = DECL_INITIAL(decl);
    break;
  }
#endif

    default:
      Is_True(FALSE, ("Unexpected tree code"));
      break;

  } /* switch */
} /* WFE_Expand_Decl */

static BOOL
function_has_varargs(tree fndecl)
{
  tree fntype  = TREE_TYPE(fndecl);
  tree arglist = TYPE_ARG_TYPES(fntype);

  while (arglist != NULL_TREE) {
    if (TREE_VALUE(arglist) == void_type_node)
      return FALSE;
    arglist = TREE_CHAIN(arglist);
  }

  return TRUE;
}

#ifdef TARG_ST
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

    INITV_IDX exc_ptr_iv = New_INITV();
    INITV_Set_VAL (Initv_Table[exc_ptr_iv], Enter_tcon (Host_To_Targ (MTYPE_U4,
                                ST_st_idx (exc_ptr_st))), 1);

    ST  * filter_st = New_ST (CURRENT_SYMTAB);
    ST_Init (filter_st, Save_Str ("__Exc_Filter__"), CLASS_VAR, SCLASS_AUTO,
	                EXPORT_LOCAL, MTYPE_To_TY(TARGET_64BIT ? MTYPE_U8 : MTYPE_U4));
    Set_ST_one_per_pu (filter_st);

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

#ifdef KEY
// Generate WHIRL representing an asm at file scope (between functions).
// Taken from kgccfe/wfe_decl.cxx
void
WFE_Assemble_Asm(char *asm_string)
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
#endif

#ifdef TARG_ST
/* (cbr) C++ specific formal parameter */
ST *first_formal;

tree
make_indirect_tree(tree nrv) {
  tree pnrv = copy_node (nrv);
  TREE_TYPE (pnrv) = build_pointer_type(TREE_TYPE(nrv));
  TREE_SET_CODE(nrv, INDIRECT_REF);
  TREE_OPERAND(nrv, 0) = pnrv;
  return pnrv;
}
#endif

extern WN *
WFE_Start_Function (tree fndecl)
{
    Is_True(fndecl != NULL && TREE_CODE(fndecl) == FUNCTION_DECL,
            ("Bad argument to WFE_Start_Function"));

    WN   *entry_wn;
    BOOL  thunk = DECL_THUNK_P(fndecl) &&
                  TREE_CODE(CP_DECL_CONTEXT(fndecl)) != NAMESPACE_DECL;

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

#ifdef KEY
    // Add DSTs for all types seen so far.  Do this now because the expansion
    // of formal parameters needs those DSTs.
    add_deferred_DST_types();
#endif

    if (CURRENT_SYMTAB != GLOBAL_SYMTAB)
      Set_PU_uplevel (Get_Current_PU ());

#ifdef KEY
    try_block_seen = false;
#endif

#ifdef TARG_ST
    // (cbr) 
    need_manual_unwinding = false;
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

    if (DECL_SOURCE_FILE (fndecl)) {
      lineno = DECL_SOURCE_LINE (fndecl);
      WFE_Set_Line_And_File (lineno, DECL_SOURCE_FILE (fndecl));
    }

    // handle VLAs in the declaration
    WN *vla_block = WN_CreateBlock ();
    WFE_Stmt_Push (vla_block, wfe_stmk_func_body, Get_Srcpos());

    ST        *func_st;
    ST_EXPORT eclass;
#ifdef TARG_ST
    eclass = Get_Export_Class_For_Tree (fndecl, CLASS_FUNC, SCLASS_TEXT);
#else
    eclass = TREE_PUBLIC(fndecl) || DECL_WEAK(fndecl) ?
      EXPORT_PREEMPTIBLE : EXPORT_LOCAL;
#endif

#ifndef GPLUSPLUS_FE
    if (DECL_INLINE (fndecl) && TREE_PUBLIC (fndecl)) {
      if (DECL_EXTERNAL (fndecl)) {
        // encountered first extern inline definition
        ST *oldst = DECL_ST (fndecl);
        DECL_ST (fndecl) = 0;
        func_st =  Get_ST (fndecl);
        DECL_ST (fndecl) = oldst;
      }
      else {
        // encountered second definition, the earlier one was extern inline
        func_st = Get_ST (fndecl);
      }
    }
    else
#else

#ifdef TARG_ST
      /* (cbr) redefinition of extern inline function */
      if (DECL_INLINE(fndecl)) {
        if (!DECL_EXTERNAL (fndecl) && TREE_PUBLIC (fndecl)) {
          if ((func_st = DECL_ST (fndecl)) &&
              PU_must_delete (Pu_Table [ST_pu (func_st)])) {
            set_DECL_ST (fndecl, 0);
          }
        }
      }
#endif

      func_st = Get_ST (fndecl);
#endif /* GPLUSPLUS_FE */


    Set_ST_sclass (func_st, SCLASS_TEXT);
    Set_PU_lexical_level (Pu_Table [ST_pu (func_st)], CURRENT_SYMTAB);
    Set_PU_cxx_lang (Pu_Table [ST_pu (func_st)]);

    if (DECL_INLINE(fndecl)) {
      Set_PU_is_inline_function (Pu_Table [ST_pu (func_st)]);
#ifdef TARG_ST
      /* (cbr) handle extern inline extension */
      if (DECL_EXTERNAL (fndecl) && !DECL_NOT_REALLY_EXTERN (fndecl) && TREE_PUBLIC (fndecl)) {
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
    /* (cbr) const attribute */
    /* Warning: 
       gcc pure maps to open64 no_side_effect
       gcc const maps to open64 pure
    */
    if (TREE_READONLY (fndecl))
      Set_PU_is_pure (Pu_Table [ST_pu (func_st)]);
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
// Insert special variables into the local symtab, store their id's
// in the PU_TAB, to be accessed later in the front-end, WN Lowerer,
// inliner/ipa, and back-end.
//    if (key_exceptions)
    if (flag_exceptions)
      Setup_Entry_For_EH ();
#endif

    INT num_args = 0;
    tree pdecl;
    for (pdecl = thunk ? DECL_ARGUMENTS (TREE_OPERAND (DECL_INITIAL (fndecl), 0))
                       : DECL_ARGUMENTS (fndecl);
         pdecl;
         pdecl = TREE_CHAIN (pdecl)) {
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
    bool xtra_parm=false;

    /* (cbr) create a new parameter for nrv */
    tree type_tree = TREE_TYPE(TREE_TYPE (fndecl));
    if (type_tree && RETURN_IN_MEMORY (type_tree) ||
        (TYPE_LANG_SPECIFIC (type_tree) &&
         TREE_ADDRESSABLE (type_tree) && 
         CLASSTYPE_NON_POD_P (type_tree))) {

        num_args++;
        xtra_parm=true;
    }

#endif

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
    entry_wn = WN_CreateEntry ( num_args, func_st, body, NULL, NULL );

    /* from 1..nkids=num_args, create IDNAME args for OPR_FUNC_ENTRY */
    INT i = 0;

#ifdef TARG_ST
    // (cbr) create first formal parameter for nrv or return optimisations
    first_formal = NULL;
    if (xtra_parm) {
      ST *formal;
      tree nrv = DECL_NRV (fndecl);

      if (nrv) {
        tree pnrv = make_indirect_tree(nrv);

        // temporary for -g
        DECL_IGNORED_P (pnrv) = 1;

        /* create formal and map it now to the var's tree */
        formal = Get_ST (pnrv);
        Set_ST_sclass(formal, SCLASS_FORMAL);
      }
      else {
        formal = New_ST (CURRENT_SYMTAB);
        first_formal = formal;
        ST_Init (formal, Save_Str(".return_arg"), CLASS_VAR,
                 SCLASS_FORMAL, EXPORT_LOCAL,
                 Make_Pointer_Type(Get_TY(type_tree)));
      }

      Set_ST_is_value_parm(formal);
      WN_kid(entry_wn,0) = WN_CreateIdname (0, ST_st_idx(formal));
      if (DECL_RESULT(fndecl)) {
	set_DECL_ST(DECL_RESULT(fndecl), formal);
      } else {
	FmtAssert (thunk, ("missing DECL_RESULT in non-thunk function"));
      }
      i++;
    }

    /* and now, the real parameters : */
#endif

    for (pdecl = thunk ? DECL_ARGUMENTS (TREE_OPERAND (DECL_INITIAL (fndecl), 0))
                       : DECL_ARGUMENTS (fndecl);
         pdecl;
         pdecl = TREE_CHAIN (pdecl) )
    {
      TY_IDX arg_ty_idx = Get_TY(TREE_TYPE(pdecl));

      ST *st;
      if (thunk) {
        st = New_ST ();

        ST_Init (st, Save_Str2i(".arg", "", i), CLASS_VAR,
		 SCLASS_FORMAL, EXPORT_LOCAL, arg_ty_idx);
      }
      else
	st = Get_ST(pdecl);

      if (!WFE_Keep_Zero_Length_Structs   &&
          TY_mtype (arg_ty_idx) == MTYPE_M &&
          TY_size (arg_ty_idx) == 0) {
        // zero length struct parameter
      }
      else {
        if (TY_mtype (arg_ty_idx) == MTYPE_F4 &&
            !TY_has_prototype (ST_pu_type (func_st)))
          Set_ST_promote_parm (st);
          WN_kid(entry_wn,i) = WN_CreateIdname ( 0, ST_st_idx(st) );
          ++i;
      }

#ifdef TARG_ST
      /* (cbr) parameters are always passed by values */
      /* passing by refence is passing the pointer by value */
      /* mark formal */
      Set_ST_is_value_parm(st);
#endif
    }

    PU_Info *pu_info;
    /* allocate a new PU_Info and add it to the list */
    pu_info = TYPE_MEM_POOL_ALLOC(PU_Info, Malloc_Mem_Pool);
    PU_Info_init(pu_info);

    Set_PU_Info_tree_ptr (pu_info, entry_wn);
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

    WFE_Stmt_Pop (wfe_stmk_func_body);

    WFE_Stmt_Push (entry_wn, wfe_stmk_func_entry, Get_Srcpos());
    WFE_Stmt_Push (body, wfe_stmk_func_body, Get_Srcpos());

    wn = WN_CreatePragma (WN_PRAGMA_PREAMBLE_END, (ST_IDX) NULL, 0, 0);
    WFE_Stmt_Append (wn, Get_Srcpos());
    WFE_Stmt_Append (vla_block, Get_Srcpos());

    if (function_has_varargs(fndecl) && !thunk) {
      // the function uses varargs.h
      // throw off the old type declaration as it did not 
      // take into account any arguments
      PU& pu = Pu_Table[ST_pu (func_st)];
      TY_IDX ty_idx;
      TY &ty = New_TY (ty_idx);
      TY_Init (ty, 0, KIND_FUNCTION, MTYPE_UNKNOWN, STR_IDX_ZERO);
      Set_TY_align (ty_idx, 1);
      TYLIST tylist_idx;
#ifdef TARG_ST
      if (xtra_parm)
	Set_TY_return_to_param(ty_idx);
#endif
      Set_TYLIST_type (New_TYLIST (tylist_idx),
                       Get_TY(TREE_TYPE(TREE_TYPE(fndecl))));
      Set_TY_tylist (ty, tylist_idx);
      for (pdecl = DECL_ARGUMENTS (fndecl); pdecl; pdecl = TREE_CHAIN (pdecl) ) {
	ST *arg_st = Get_ST(pdecl);
        Set_TYLIST_type (New_TYLIST (tylist_idx), ST_type(arg_st));
      }
      Set_TYLIST_type (New_TYLIST (tylist_idx), 0);
      Set_TY_is_varargs (ty_idx);
      Set_TY_has_prototype (ty_idx);
      Set_PU_prototype (pu, ty_idx);
    }

    if (!thunk && DECL_GLOBAL_CTOR_P(fndecl)) {
#ifndef KEY
      // GLOBAL_INIT_PRIORITY does not exist any more
      if (GLOBAL_INIT_PRIORITY(fndecl) != DEFAULT_INIT_PRIORITY) {
        DevWarn("Discarding ctor priority %d (default %d) at line %d",
                GLOBAL_INIT_PRIORITY(fndecl),
                DEFAULT_INIT_PRIORITY,
                lineno);
      }
#endif // !KEY

      INITV_IDX initv = New_INITV ();
      INITV_Init_Symoff (initv, func_st, 0, 1);
      Set_ST_addr_saved (func_st);
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
      Set_PU_no_inline (Pu_Table [ST_pu (func_st)]);
      Set_PU_no_delete (Pu_Table [ST_pu (func_st)]);
    }

    if (!thunk && DECL_GLOBAL_DTOR_P(fndecl)) {
#ifndef KEY
      // GLOBAL_INIT_PRIORITY does not exist any more
      if (GLOBAL_INIT_PRIORITY(fndecl) != DEFAULT_INIT_PRIORITY) {
        DevWarn("Discarding dtor priority %d (default %d) at line %d",
                GLOBAL_INIT_PRIORITY(fndecl),
                DEFAULT_INIT_PRIORITY,
                lineno);
      }
#endif // !KEY

      INITV_IDX initv = New_INITV ();
      INITV_Init_Symoff (initv, func_st, 0, 1);
      Set_ST_addr_saved (func_st);
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
    }
#ifdef KEY
    // Tell the rest of the front-end this is the current function's entry wn.
    Push_Current_Entry_WN(entry_wn);
#endif

    return entry_wn;
}

extern void
WFE_Finish_Function (void)
{
    WFE_Check_Undefined_Labels ();
    PU_Info *pu_info = PU_Info_Table [CURRENT_SYMTAB];

    if (CURRENT_SYMTAB > GLOBAL_SYMTAB + 1) {

      DevWarn ("Encountered nested function");
      Set_PU_is_nested_func (Get_Current_PU ());
    }

    // Insert a RETURN if it does not exist
    WN * wn = WN_last (WFE_Stmt_Top ());
    if (wn == NULL || WN_operator (wn) != OPR_RETURN &&
        WN_operator (wn) != OPR_RETURN_VAL) {
        WFE_Stmt_Append (WN_CreateReturn (), Get_Srcpos ());
    }

    // Add any handler code
    Do_Handlers ();
    Do_EH_Cleanups ();

#ifdef KEY
    //    if (key_exceptions)
    if (flag_exceptions)
      Do_EH_Tables ();
#endif // KEY

    // write out all the PU information
    WFE_Stmt_Pop (wfe_stmk_func_body);

    WN *func_wn = WFE_Stmt_Pop (wfe_stmk_func_entry);

#ifdef TARG_ST
    // [CG] Handle volatile field accesses.
    function_update_volatile_accesses(func_wn);
#endif

#ifdef WFE_DEBUG
    fprintf(stdout, "================= Dump function ================\n");
    fdump_tree(stdout, func_wn);
#endif


    // deallocate the old map table
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

#ifdef KEY
    try_block_seen = false;

    // Restore the previous entry wn, if any.
    Pop_Current_Entry_WN();
#endif

#ifdef TARG_ST
    // (cbr) 
    need_manual_unwinding = false;
#endif

#ifdef TARG_ST
    // We cannot have both no_inline and must_inline set, but this can happen 
    // Since for instance functions taking addresses of local labels cannot be inlined
    // But may be given the attribute always_inline
    // We fix these contradictions here (time for a user message if -Winline is on)
    if (PU_no_inline (Get_Current_PU ()) && PU_must_inline (Get_Current_PU ()))
      Clear_PU_must_inline(Get_Current_PU ());
#endif

    Delete_Scope (CURRENT_SYMTAB);
    --CURRENT_SYMTAB;
//  if (CURRENT_SYMTAB > GLOBAL_SYMTAB)
//    Current_pu = &Pu_Table[ST_pu (Scope_tab[CURRENT_SYMTAB].st)];

#ifdef TARG_ST
    // (cbr) emit ident directives if any
    if (idents_strs) {
      WFE_Idents();
    }
#endif
}

// Because we build inito's piecemeal via calls into wfe for each piece,
// need to keep track of current inito and last initv that we append to.
static INITO_IDX aggregate_inito = 0;
static INITV_IDX last_aggregate_initv = 0;	
static BOOL not_at_root = FALSE;

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
      // [HK]
#if __GNUC__ >= 3
      std::swap (d_.d[0], d_.d[1]);
#else
    swap (d_.d[0], d_.d[1]);
#endif // __GNUC__ >= 3
  }
  return d_.df ;
}
#endif

void 
#if defined (TARG_ST) && (GNU_FRONT_END==33)
/* (cbr) gcc 3.3 upgrade */
WFE_Add_Aggregate_Init_Double (REAL_VALUE_TYPE rval, INT size)
#else
WFE_Add_Aggregate_Init_Double (double val, INT size)
#endif
{
#if defined (TARG_ST) && (GNU_FRONT_END==33)
/* (cbr) gcc 3.3 upgrade */
  double val = real_value_to_double (rval);
#endif

  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();
  TYPE_ID mtype;
  if (size == 4) mtype = MTYPE_F4;
  else if (size == 8) mtype = MTYPE_F8;
  else FmtAssert(FALSE, ("WFE_Add_Aggregate_Init_Double unexpected size"));
  INITV_Init_Float (inv, mtype, val);
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
}

void 
#if defined (TARG_ST) && (GNU_FRONT_END==33)
/* (cbr) gcc 3.3 upgrade */
WFE_Add_Aggregate_Init_Complex (REAL_VALUE_TYPE rrval, REAL_VALUE_TYPE rival,
                                INT size)
#else
WFE_Add_Aggregate_Init_Complex (double rval, double ival, INT size)
#endif
{
#if defined (TARG_ST) && (GNU_FRONT_END==33)
/* (cbr) gcc 3.3 upgrade */
  double rval = real_value_to_double (rrval);
  double ival = real_value_to_double (rival);
#endif

  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();
  TYPE_ID mtype;
  if (size == 8) mtype = MTYPE_F4;
  else if (size == 16) mtype = MTYPE_F8;
  else FmtAssert(FALSE, ("WFE_Add_Aggregate_Init_Double unexpected size"));
  INITV_Init_Float (inv, mtype, rval);
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
  inv = New_INITV();
  INITV_Init_Float (inv, mtype, ival);
  Set_INITV_next(last_aggregate_initv, inv);
  last_aggregate_initv = inv;
}

void 
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
WFE_Add_Aggregate_Init_String (const char *s, INT size)
#else
WFE_Add_Aggregate_Init_String (char *s, INT size)
#endif
{
  if (aggregate_inito == 0) return;
  INITV_IDX inv = New_INITV();
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 ugrade */
  INITV_Init_String (inv, (char*)s, size);
#else
  INITV_Init_String (inv, s, size);
#endif
  if (last_aggregate_initv != 0)
    Set_INITV_next(last_aggregate_initv, inv);
  else if (! not_at_root)
    Set_INITO_val(aggregate_inito, inv);
  last_aggregate_initv = inv;
}

void
#ifdef TARG_ST
/* (cbr) support for half address relocation */
WFE_Add_Aggregate_Init_Symoff (ST *st, WN_OFFSET offset = 0, BOOL halfword = 0)
#else
WFE_Add_Aggregate_Init_Symoff (ST *st, WN_OFFSET offset = 0)
#endif
{
#ifdef WFE_DEBUG
    fprintf(stdout,"====================================================\n");
    fprintf(stdout,"      WFE_Add_Aggregate_Init_Symoff %s \n\n", ST_name(st));
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

  Set_ST_addr_saved (st);
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
//     fprintf(stdout,"      WFE_Add_Aggregate_Init_Labeldiff %s \n\n", ST_name(st));
    fprintf(stdout,"  last_aggregate_initv = %d\n", last_aggregate_initv);
    fprintf(stdout,"====================================================\n");
#endif

  Set_PU_no_inline (Get_Current_PU ());
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
#ifndef TARG_ST
  /* (cbr) it's ok */
  DevWarn ("taking address of a label at line %d", lineno);
#endif
#ifdef TARG_ST
  /* (cbr) don't inline if we have a label */
  Set_PU_no_inline (Get_Current_PU ());
#endif
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
	{
	ST *st = Get_ST (init);
	WFE_Add_Aggregate_Init_Symoff (st);
	}
	break;

  case STRING_CST:
	{
#ifdef KEY
	TCON tcon = Host_To_Targ_String (MTYPE_STRING,
			const_cast<char*>TREE_STRING_POINTER(init),
			TREE_STRING_LENGTH(init));
#else
	TCON tcon = Host_To_Targ_String (MTYPE_STRING,
				       TREE_STRING_POINTER(init),
				       TREE_STRING_LENGTH(init));
#endif // KEY
	ST *const_st = New_Const_Sym (Enter_tcon (tcon), 
		Get_TY(TREE_TYPE(init)));
      	WFE_Add_Aggregate_Init_Symoff (const_st);
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
		WFE_Add_Aggregate_Init_Symoff ( Get_ST (addr_kid),
			Get_Integer_Value(TREE_OPERAND(init,1)) );
	}
	else
	{
		WN *init_wn = WFE_Expand_Expr (init);
		FmtAssert (WN_operator (init_wn) == OPR_LDA,
				("expected decl under plus_expr"));
		WFE_Add_Aggregate_Init_Symoff (WN_st (init_wn),
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
		FmtAssert (DECL_SYMTAB_IDX(init) == CURRENT_SYMTAB,
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

      WFE_Add_Aggregate_Init_Symoff (temp);
    }
    break;
#endif
  default:
	{
		WN *init_wn = WFE_Expand_Expr (init);
		FmtAssert (WN_operator (init_wn) == OPR_LDA,
				("expected operator encountered"));
		WFE_Add_Aggregate_Init_Symoff (WN_st (init_wn),
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
	if (!CONSTRUCTOR_ELTS(init))
	    return TRUE;
	return Has_Non_Constant_Init_Value (CONSTRUCTOR_ELTS(init));
  case TREE_LIST:
	{
	tree p;
	for (p = init; p != NULL; p = TREE_CHAIN(p)) {
		if (Has_Non_Constant_Init_Value (TREE_VALUE(p))) {
			return TRUE;
		}
/*
		if (TREE_PURPOSE(p) != NULL_TREE 	     &&
		    TREE_CODE(TREE_PURPOSE(p)) == FIELD_DECL &&
		    DECL_BIT_FIELD(TREE_PURPOSE(p)))
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
  case ADDR_EXPR: {
	tree t = TREE_OPERAND(init, 0);
	if (DECL_CONTEXT(t) == 0 ||
	    TREE_CODE(DECL_CONTEXT(t)) == NAMESPACE_DECL)
		return FALSE;
	return TRUE;
  }
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
  /* (cbr) align on C compiler for the size of initvalues to set inline */
	if (TY_size(ST_type(st)) <= (2*MTYPE_byte_size(Spill_Int_Mtype))) {
		return FALSE;
	}
	else
#endif
	return !Has_Non_Constant_Init_Value(init);
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
		WFE_Add_Aggregate_Init_Double (
			TREE_REAL_CST(val), size);
		break;
	case STRING_CST:
		WFE_Add_Aggregate_Init_String (
			TREE_STRING_POINTER(val), size);
		break;
#ifdef TARG_ST
                /* (cbr) pointer to member initialized */
	case PTRMEM_CST:
          {
            INT64 ptrmem = Get_Integer_Value(cplus_expand_constant(val));
            WFE_Add_Aggregate_Init_Integer (ptrmem, size);
            break;
          }
#endif
#if 0
	case PLUS_EXPR:
		if ( TREE_CODE(TREE_OPERAND(val,0)) == ADDR_EXPR
		     && TREE_CODE(TREE_OPERAND(val,1)) == INTEGER_CST)
		{
			tree addr_kid = TREE_OPERAND(TREE_OPERAND(val,0),0);
			FmtAssert(TREE_CODE(addr_kid) == VAR_DECL
				  || TREE_CODE(addr_kid) == FUNCTION_DECL,
				("expected decl under plus_expr"));
			WFE_Add_Aggregate_Init_Symoff ( Get_ST (addr_kid),
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
              else
              if (TREE_CODE(kid) == INTEGER_CST) {
                      WFE_Add_Aggregate_Init_Integer (
                              Get_Integer_Value(kid), size);
		      break;
		}
		// fallthru
	default:
		{
		WN *init_wn;
		init_wn = WFE_Expand_Expr (val);
#ifdef TARG_ST
                // Skip OPR_TAS since it does not change the representation.
                init_wn = skip_tas (init_wn);
#endif

		// handle converts over LDA
		if ((WN_opcode (init_wn) == OPC_I4U4CVT &&
		     WN_opcode (WN_kid0 (init_wn)) == OPC_U4LDA) ||
		    (WN_opcode (init_wn) == OPC_I8U8CVT &&
		     WN_opcode (WN_kid0 (init_wn)) == OPC_U8LDA)) {
			WN *kid0 = WN_kid0(init_wn);
			WFE_Add_Aggregate_Init_Symoff (WN_st (kid0), WN_offset(kid0));
			WN_DELETE_Tree (init_wn);
			break;
		}

		if (WN_operator (init_wn) == OPR_LDA) {
			WFE_Add_Aggregate_Init_Symoff (WN_st (init_wn),
						       WN_offset (init_wn));
			WN_DELETE_Tree (init_wn);
			break;
		}
#ifdef TARG_ST
                /* (cbr) handle label as value gnu ext */
		else if (WN_operator(init_wn) == OPR_LDA_LABEL) {
                  LABEL_IDX label = WN_label_number(init_wn);
                  WFE_Add_Aggregate_Init_Label (label);
                  WN_DELETE_Tree (init_wn);
                  break;
		}
#endif
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
			  WFE_Add_Aggregate_Init_Symoff (WN_st (kid0),
				     WN_offset(kid0) + WN_const_val(kid1));
			  WN_DELETE_Tree (init_wn);
			  break;
			}
		 	else if (WN_operator(kid1) == OPR_LDA &&
			    WN_operator(kid0) == OPR_INTCONST) {
			  WFE_Add_Aggregate_Init_Symoff (WN_st (kid1),
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
			  WFE_Add_Aggregate_Init_Symoff (WN_st (kid0),
				     WN_offset(kid0) - WN_const_val(kid1));
			  WN_DELETE_Tree (init_wn);
			  break;
			}
		}
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
#ifdef TARG_ST
  /* (cbr) pro-release-1-9-0-B/6 need if throw_expr part of the conditiol assignment */
  WN *init_wn;

  if (TREE_CODE (init) == COND_EXPR) {
    init_wn = WFE_Expand_Expr (init, TRUE);

    if (!init_wn) {
      WN * lwn = WN_last (WFE_Stmt_Top ());
      if (WN_operator (lwn) == OPR_IF) {
        WN *awn = WN_kid1(lwn);

        if (TREE_CODE (TREE_OPERAND (init, 1)) == (enum tree_code)THROW_EXPR) {
          awn = WN_kid2(lwn);
          if (WN_operator (awn) == OPR_BLOCK) {
            WN *awn1 = WN_kid0(awn);
            if (WN_operator (awn1) == OPR_EVAL) {
              TYPE_ID mtype = is_bit_field ? MTYPE_BS : TY_mtype(ty);
              WN *awn2 = WN_kid0(awn1);
              awn2 = WN_Stid (mtype, ST_ofst(st) + offset, st,
                              ty, awn2, field_id);
              WN_kid0(awn) = awn2;
              bytes += TY_size(ty);
              return;
            }
          }

        }
        else if (TREE_CODE (TREE_OPERAND (init, 2)) == (enum tree_code)THROW_EXPR) {
          awn = WN_kid1(lwn);
          if (WN_operator (awn) == OPR_BLOCK) {
            WN *awn1 = WN_kid0(awn);
            if (WN_operator (awn1) == OPR_EVAL) {
              TYPE_ID mtype = is_bit_field ? MTYPE_BS : TY_mtype(ty);
              WN *awn2 = WN_kid0(awn1);
              awn2 = WN_Stid (mtype, ST_ofst(st) + offset, st,
                              ty, awn2, field_id);
              WN_kid0(awn) = awn2;
              bytes += TY_size(ty);
              return;
            }
          }
        }
      }
    }
  }
  else
    init_wn = WFE_Expand_Expr (init);
#else
  WN *init_wn = WFE_Expand_Expr (init);
#endif
    
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
      // [CG] Handle overflow and padding with string initialization.
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

#ifdef TARG_ST
  /* (cbr) fix for multiple dim arrays */
  for (init = CONSTRUCTOR_ELTS(init_list);
       init;
       init = TREE_CHAIN(init)) {
    // loop through each array element
    if (TREE_CODE(TREE_VALUE(init)) == (enum tree_code)PTRMEM_CST)  {
      TREE_VALUE(init) = cplus_expand_constant(TREE_VALUE(init));
    }

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

#else
  tree   next;

  for (init = CONSTRUCTOR_ELTS(init_list);
       init;
       init = next) {
    // loop through each array element

    next = TREE_CHAIN(init);

    if (TREE_CODE(TREE_VALUE(init)) == PTRMEM_CST)  {
      TREE_VALUE(init) = cplus_expand_constant(TREE_VALUE(init));
    }

    if (TREE_CODE(TREE_VALUE (init)) == CONSTRUCTOR) {
      // recursively process nested ARRAYs and STRUCTs
      // update array_elem_offset to current_offset to
      // keep track of where each array element starts
      Traverse_Aggregate_Constructor (st, TREE_VALUE(init), TREE_TYPE(type),
                                      gen_initv, current_offset, current_offset,
                                      0);
      emitted_bytes += esize;
    }

    else {
      // initialize SCALARs and POINTERs
      // note that we should not be encountering bit fields
      if (gen_initv) {
        Add_Initv_For_Tree (TREE_VALUE(init), esize);
        emitted_bytes += esize;
      }
      else
        Gen_Assign_Of_Init_Val (st, TREE_VALUE(init), current_offset, 0,
                                ety, FALSE, 0, FLD_HANDLE (), emitted_bytes);
    }

    current_offset += esize;
  }
#endif

  // If the entire array has not been initialized, pad till the end
  pad = TY_size (ty) - emitted_bytes;

  if (pad > 0)
    Traverse_Aggregate_Pad (st, gen_initv, pad, current_offset);

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

  // account for anonymous WHIRL fields being generated for every direct,
  // nonempty nonvirtual base class.
  // these are generated first in Create_TY_For_Tree (tree_symtab.cxx)

#ifndef KEY     // g++'s class.c already laid out the base types.  Bug 11622.
  if (TYPE_BINFO(type) &&
      BINFO_BASETYPES(TYPE_BINFO(type))) {
    tree basetypes = BINFO_BASETYPES(TYPE_BINFO(type));
    INT32 i;
    for (i = 0; i < TREE_VEC_LENGTH(basetypes); ++i) {
      tree binfo = TREE_VEC_ELT(basetypes, i);
      tree basetype = BINFO_TYPE(binfo);
      if (!is_empty_base_class(basetype) ||
          !TREE_VIA_VIRTUAL(binfo)) {
        ++field_id;
        fld = FLD_next (fld);
        field_id += TYPE_FIELD_IDS_USED(basetype);
      }
    }
  }
#endif
 
#ifdef TARG_ST
/* (cbr) real_field can't be static */
  bool real_field = true;
  while (field && TREE_CODE(field) != FIELD_DECL)
    field = next_real_or_virtual_field(type, field, real_field);
#else
  while (field && TREE_CODE(field) != FIELD_DECL)
    field = next_real_or_virtual_field(type, field);
#endif

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

    // if the initialization is not for the current field,
    // advance the fields till we find it
    if (field && TREE_PURPOSE(init) && TREE_CODE(TREE_PURPOSE(init)) == FIELD_DECL) {
#ifndef TARG_ST
      DevWarn ("Encountered FIELD_DECL during initialization");
#endif
      for (;;) {
        if (field == TREE_PURPOSE(init)) {
          break;
        }
        ++field_id;
        fld = FLD_next (fld);
#ifdef TARG_ST
/* (cbr) real_field can't be static */
        bool real_field = true;
        field = next_real_or_virtual_field(type, field, real_field);
        while (field && TREE_CODE(field) != FIELD_DECL)
          field = next_real_or_virtual_field(type, field, real_field);
#else
        field = next_real_or_virtual_field(type, field);
        while (field && TREE_CODE(field) != FIELD_DECL)
          field = next_real_or_virtual_field(type, field);
#endif
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
#ifdef TARG_ST
          /* (cbr) function member pointers are described in a struct {__delta, __index, __pfn}
             that needs to be initialized */
          if (TREE_CODE (TREE_VALUE (init)) == (enum tree_code)PTRMEM_CST) {
            tree cst = cplus_expand_constant(TREE_VALUE(init));
            if (TREE_CODE (cst) == CONSTRUCTOR) {
              // recursively process nested ARRAYs and STRUCTs
              // update array_elem_offset to current_offset to
              // keep track of where each array element starts
              Traverse_Aggregate_Constructor (st, cst, TREE_TYPE(cst),
                                              TRUE, 0, 0, 0);
            }
            else {
              Add_Initv_For_Tree (cst, TY_size(fld_ty));
            }
          }
          else {
            Add_Initv_For_Tree (TREE_VALUE(init), TY_size(fld_ty));
          }
#else
          Add_Initv_For_Tree (TREE_VALUE(init), TY_size(fld_ty));
#endif
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
    fld = FLD_next(fld);
#ifdef TARG_ST
/* (cbr) real_field can't be static */
    bool real_field = true;
    field = next_real_or_virtual_field(type, field, real_field);
    while (field && TREE_CODE(field) != FIELD_DECL)
      field = next_real_or_virtual_field(type, field, real_field);
#else
    field = next_real_or_virtual_field(type, field);
    while (field && TREE_CODE(field) != FIELD_DECL)
      field = next_real_or_virtual_field(type, field);
#endif
  }

  // if not all fields have been initialized, then loop through
  // the remaining fields to update field_id
  while ( ! fld.Is_Null()) {
    ++field_id;
#ifdef TARG_ST
    // [CG 2004/11/22] Merge code from gccfe.
    // Also check to see if any bit fields need to be initialized 
    // to zero to handle the case where the bit field shares the
    // same byte as last bit field which was initialized.
    if (!gen_initv && FLD_is_bit_field(fld)) {
      // [CG 2004/11/22] The original code is wrong, we must initialize
      // to zero bitfields that start before the current offset.
      // The current bitfiled may also span multiple bytes.
      // In this case it is initialized fully even if the trailling
      // bytes will also be initialized by the byte padding below.

      // find number of bytes that have been output with previous bitfields
      INT bytes_out = current_offset - FLD_ofst(fld);
      if (bytes_out > 0) {
	TY_IDX fld_ty = FLD_type(fld);
	WN *init_wn = WN_Intconst (WFE_Promoted_Type(TY_mtype (fld_ty)), 0);
	WN *wn = WN_Stid (MTYPE_BS, ST_ofst(st) + array_elem_offset, st,
			  ty, init_wn, field_id);
	WFE_Stmt_Append(wn, Get_Srcpos());
      }
    }
#endif
    fld = FLD_next(fld);
#ifdef TARG_ST
/* (cbr) real_field can't be static */
    bool real_field = true;
    field = next_real_or_virtual_field(type, field, real_field);
    while (field && TREE_CODE(field) != FIELD_DECL)
      field = next_real_or_virtual_field(type, field, real_field);
#else
    field = next_real_or_virtual_field(type, field);
    while (field && TREE_CODE(field) != FIELD_DECL)
      field = next_real_or_virtual_field(type, field);
#endif
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
  INITV_IDX block_body_initv = 0; // VL: Initialization fixes art #44102
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
    Traverse_Aggregate_Array (st, init_list, type, gen_initv, current_offset);
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
#ifdef TARG_ST
/* (cbr) need decl for DECL_SECTION_NAME */
Add_Inito_For_Tree (tree init, tree decl, ST *st)
#else
Add_Inito_For_Tree (tree init, ST *st)
#endif
{
  tree kid;
#ifdef WFE_DEBUG
  fprintf(stdout, "=============== Add_Inito_For_Tree ==============\n");
#endif
  last_aggregate_initv = 0;
  switch (TREE_CODE(init)) {
  case INTEGER_CST:
	UINT64 val;
	val = Get_Integer_Value (init);
#ifdef TARG_ST
        /* (cbr) section-attribute, keep as dglobal inito */
        if (Zeroinit_in_bss && val == 0
          && ! DECL_SECTION_NAME (decl)) {
#else
	if (val == 0) {
#endif
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
	WFE_Add_Aggregate_Init_Double (TREE_REAL_CST(init), 
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
	aggregate_inito = New_INITO (st);
	not_at_root = FALSE;
	WFE_Add_Aggregate_Init_String (TREE_STRING_POINTER(init), 
                                       TY_size(ST_type(st)) != 0 ?
                                       TY_size(ST_type(st)) :
                                       TREE_STRING_LENGTH(init));
	return;
  case NOP_EXPR:
#ifdef TARG_ST
    /* (cbr) need decl */
    Add_Inito_For_Tree (TREE_OPERAND(init,0), decl, st);
#else
    Add_Inito_For_Tree (TREE_OPERAND(init,0), st);
#endif
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
#ifdef TARG_ST
          /* (cbr) need decl */
		Add_Inito_For_Tree (kid, decl, st);
#else
		Add_Inito_For_Tree (kid, st);
#endif
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
#ifdef TARG_ST
          /* (cbr) need decl */
		Add_Inito_For_Tree (kid, decl, st);
#else
		Add_Inito_For_Tree (kid, st);
#endif
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
	WFE_Add_Aggregate_Init_Symoff (WN_st (init_wn), WN_offset (init_wn));
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
      WFE_Add_Aggregate_Init_Symoff (WN_st(kid0),
		WN_offset(kid0) + WN_const_val(kid1));
#else
    if (WN_operator(WN_kid0(init_wn)) == OPR_LDA &&
        WN_operator(WN_kid1(init_wn)) == OPR_INTCONST) {
      aggregate_inito = New_INITO (st);
      not_at_root = FALSE;
      WFE_Add_Aggregate_Init_Symoff (WN_st(WN_kid0(init_wn)),
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
      WFE_Add_Aggregate_Init_Symoff (WN_st(kid0),
		WN_offset(kid0) - WN_const_val(kid1));
#else
    if (WN_operator(WN_kid0(init_wn)) == OPR_LDA &&
        WN_operator(WN_kid1(init_wn)) == OPR_INTCONST) {
      aggregate_inito = New_INITO (st);
      not_at_root = FALSE;
      WFE_Add_Aggregate_Init_Symoff (WN_st(WN_kid0(init_wn)),
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

#ifdef TARG_ST
  /* (cbr) pro-release-1-9-0-B/30/68. TYPE_SIZE is not set for constructor initializers. 
     need to allocate the object */
  if (TREE_CODE(init) == CONSTRUCTOR && 
      TY_kind (ty_idx) == KIND_ARRAY && TY_size (ty_idx) == 0 && !TYPE_SIZE(TREE_TYPE(init))) {
      tree init_elt;
      UINT tsize = 0;
      UINT align = 1;

      for (init_elt = CONSTRUCTOR_ELTS(init);
           init_elt;
           init_elt = TREE_CHAIN(init_elt)) {
        if (TREE_PURPOSE(init_elt)) {
          tree ppose = TREE_PURPOSE(init_elt);
          tree value = TREE_VALUE (init_elt);
          tree ssize = TYPE_SIZE(TREE_TYPE (value));

          align = MAX((TYPE_ALIGN(TREE_TYPE(value)) / BITSPERBYTE), align);
          tsize += (Get_Integer_Value(ssize) / BITSPERBYTE);
        }
      }
      if (tsize) {
        Set_TY_align (ty_idx, align);
        Set_TY_size (ty_idx, tsize);
      }
    }
#endif

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
	aggregate_inito = New_INITO (temp);
	not_at_root = FALSE;
	last_aggregate_initv = 0;
	Traverse_Aggregate_Constructor (temp, init, TREE_TYPE(init),
                                        TRUE /*gen_initv*/, 0, 0, 0);
	WFE_Finish_Aggregate_Init ();
  }
  return temp;
}

static tree init_decl = NULL;

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
  if (init->common.code == VAR_DECL &&
      DECL_CONTEXT(init)	    &&
      TREE_CODE(DECL_CONTEXT(init)) == RECORD_TYPE)
    Get_TY(DECL_CONTEXT(init));

  if (TREE_STATIC(decl) || DECL_CONTEXT(decl) == NULL) 
  {
	// static or global context, so needs INITO
	if (ST_sclass(st) == SCLASS_UGLOBAL && !ST_init_value_zero(st)  ||
	    ST_sclass(st) == SCLASS_EXTERN  			        ||
	    ST_sclass(st) == SCLASS_COMMON)
		Set_ST_sclass(st, SCLASS_DGLOBAL);
	if (!ST_is_initialized(st)) {
		Set_ST_is_initialized(st);
		if (init_decl) {
			Push_Deferred_Decl_Init (decl);
			return;
		}
		init_decl = decl;
#ifdef TARG_ST
          /* (cbr) need decl */
		Add_Inito_For_Tree (init, decl, st);
#else
		Add_Inito_For_Tree (init, st);
#endif
		while (deferred_decl_init_i >= 0) {
			init_decl = Pop_Deferred_Decl_Init ();
#ifdef TARG_ST
          /* (cbr) need decl */
			Add_Inito_For_Tree (DECL_INITIAL(init_decl), decl,
					    Get_ST(init_decl));
#else
			Add_Inito_For_Tree (DECL_INITIAL(init_decl), 
					    Get_ST(init_decl));
#endif
		}
		init_decl = NULL;
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

void
WFE_Decl (tree decl)
{
  if (DECL_INITIAL (decl) != 0) return;	// already processed
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
  if (DECL_CONTEXT(decl) != 0) return;	// local
  if ( ! TREE_PUBLIC(decl)) return;	// local
  if ( ! TREE_STATIC(decl)) return;	// extern
  // is something that we want to put in symtab
  // (a global var defined in this file).
  (void) Get_ST(decl);
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
  // [CG]: Merge from gccfe
  // [CG]: Aliases on common data do not work in ELF.
  // gcc silently fails. We generate a warning.
  if (ST_class(base_st) == CLASS_VAR && ST_sclass(base_st) == SCLASS_COMMON) {
    warning_with_decl (decl,
		     "alias attribute cannot be specified on common data");
    return;
  }
#endif
  ST *st = Get_ST (decl);

  // [CL] remember symbol if base of alias
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
    // [CG] Merge code from gccfe
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
  tree base_decl = lookup_name (target, 1);

#ifdef TARG_ST
  /* (cbr) don't create symbol now */
  if (!base_decl || !DECL_ST(base_decl) || !DECL_ST(decl)) {
#else
  if (!base_decl || !DECL_ST(base_decl)) {
#endif
    add_alias (IDENTIFIER_POINTER(DECL_NAME(decl)), IDENTIFIER_POINTER (target));
    return;
  }

#ifdef KEY
  expanded_decl(decl) = TRUE;
#endif // KEY 
  WFE_Set_Alias (decl, base_decl);

} /* WFE_Assemble_Alias */

void
WFE_Alias_Finish ()
{
  while (alias_decls) {
    if (alias_decls->bname) {
      tree base_decl = lookup_name (get_identifier (alias_decls->bname), 1);
      if (!base_decl) {
        /* (cbr) Synthetize a function decl */
        tree fnname = get_identifier (alias_decls->bname);
        base_decl = build_decl (FUNCTION_DECL, fnname,
                                build_function_type (void_type_node, NULL_TREE));
      }
      WFE_Set_Alias (lookup_name(get_identifier(alias_decls->name), 1), base_decl);
    }
    alias_decls = alias_decls->next;
  }
}
#else /* !TARG_ST */

void
WFE_Assemble_Alias (tree decl, tree target)
{
  DevWarn ("__attribute alias encountered at line %d", lineno);
  tree base_decl = lookup_name (target, 1);
  FmtAssert (base_decl != NULL,
             ("undeclared base symbol %s not yet declared in __attribute__ alias is not currently implemented",
              IDENTIFIER_POINTER (target)));
  ST *base_st = Get_ST (base_decl);
  ST *st = Get_ST (decl);
  Set_ST_base_idx (st, ST_st_idx (base_st));
  if (ST_is_weak_symbol(st))
    Set_ST_sclass (st, SCLASS_EXTERN);
  else {
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

#ifdef TARG_ST
  /* (cbr) change prototype */
void
WFE_Assemble_Constructor (tree func_decl)
#else
WFE_Assemble_Constructor (char *name)
#endif
{
#ifndef TARG_ST
  DevWarn ("__attribute__ ((constructor)) encountered at line %d", lineno);
  tree func_decl = lookup_name (get_identifier (name), 1);
#endif
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
  Set_PU_no_inline (Pu_Table [ST_pu (func_st)]);
  Set_PU_no_delete (Pu_Table [ST_pu (func_st)]);
  Set_ST_addr_saved (func_st);
}

void
#ifdef TARG_ST
  /* (cbr) change prototype */
WFE_Assemble_Destructor (tree func_decl)
#else
WFE_Assemble_Destructor (char *name)
#endif
{
#ifndef TARG_ST
  DevWarn ("__attribute__ ((destructor)) encountered at line %d", lineno);
  tree func_decl = lookup_name (get_identifier (name), 1);
#endif
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
#ifdef TARG_ST
  /* (cbr) called from Create_ST_For_Tree */
  ST *st = DECL_ST (decl);
#else
  ST *st = Create_ST_For_Tree (decl);
#endif
  ST *alloca_st = New_ST (CURRENT_SYMTAB);
  ST_Init (alloca_st, Save_Str (ST_name (st)),
           CLASS_VAR, SCLASS_AUTO, EXPORT_LOCAL,
           Make_Pointer_Type (ST_type (st), FALSE));
  Set_ST_is_temp_var (alloca_st);
  Set_ST_pt_to_unique_mem (alloca_st);
  Set_ST_base_idx (st, ST_st_idx (alloca_st));
  WN *swn = WFE_Expand_Expr (TYPE_SIZE(TREE_TYPE(decl)));
#ifndef TARG_ST
  // [SC] swn contains the size in bits.  Convert this to bytes.
  TYPE_ID mtype = TY_mtype (WN_object_ty (swn));
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
WFE_Record_Asmspec_For_ST (tree decl, char *asmspec, int reg)
{
#ifndef TARG_ST
  extern PREG_NUM Map_Reg_To_Preg []; // defined in common/com/arch/config_targ.cxx
#else
  //TB now Map_Reg_To_Preg is define
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

#if defined (TARG_ST) && (GNU_FRONT_END==33)
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
  struct weak_syms *t;
  for (t = weak_decls; t; t = t->next) {
    if (t->name) {
      tree decl = lookup_name (get_identifier (t->name), 1);
      if (!decl) {
        INT i;
        BOOL found = FALSE;
        ST *st;
        FOREACH_SYMBOL (GLOBAL_SYMTAB, st, i) {
          if (strcmp (ST_name(st), t->name) == 0) {
            found = TRUE;
            Set_ST_is_weak_symbol (st);
            break;
          }
        }
        if (!found)
          warning ("did not find declaration `%s' for used in #pragma weak", t->name);
      }
      else {
        ST *st = Get_ST (decl);
        if (st) {
          Set_ST_is_weak_symbol (st);
          if (t->value) {
            tree base_decl = lookup_name (get_identifier (t->value), 1);
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

void WFE_Process_Type_Decl (tree);
void WFE_Process_Template_Decl (tree);
void WFE_Process_Var_Decl (tree);
void WFE_Process_Function_Decl (tree);
void WFE_Process_Namespace_Decl (tree);
void WFE_Process_Decl (tree);

void
WFE_Process_Class_Decl (tree decl)
{
//fprintf(stderr, "CLASS_DECL: %s\n", IDENTIFIER_POINTER(DECL_NAME(decl)));
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  if (CP_TYPE_CONST_P(decl) || CP_TYPE_VOLATILE_P(decl) || CP_TYPE_RESTRICT_P (decl))
#else
  if (CP_TYPE_QUALS(decl) != TYPE_UNQUALIFIED)
#endif
    return;

  if (TYPE_TY_IDX(decl))
    return;

  TYPE_TY_IDX(decl) = MTYPE_B;

  tree  binfo     = TYPE_BINFO(decl);
  tree  basetypes = binfo ? BINFO_BASETYPES(binfo) : 0;
  INT32 i;
  if (basetypes)
    for (i = 0; i < TREE_VEC_LENGTH(basetypes); ++i)
      (void) WFE_Process_Class_Decl (BINFO_TYPE(TREE_VEC_ELT(basetypes, i)));

  tree  field;
#ifdef TARG_ST
/* (cbr) real_field can't be static */
  bool real_field = true;
  for (field = TYPE_FIELDS (decl);
    field != NULL_TREE;
    field = next_real_or_virtual_field (decl, field, real_field)) {
#else
  for (field = TYPE_FIELDS (decl);
    field != NULL_TREE;
    field = next_real_or_virtual_field (decl, field)) {
#endif
    if (TREE_CODE(field) == TYPE_DECL) {
      tree field_type = TREE_TYPE(field);
      if (field_type &&
          TREE_CODE(field_type) == RECORD_TYPE &&
          field_type != decl) {
        WFE_Process_Class_Decl (field_type);
      }
    } 
  }

  tree method = TYPE_METHODS(decl);
  while (method != NULL_TREE) {
    if (TREE_CODE(method) == FUNCTION_DECL) {
      tree body = DECL_SAVED_TREE(method);
      if (body != NULL_TREE && !DECL_EXTERNAL(method) &&
          !DECL_WEAK(method) &&
#ifndef TARG_ST
          /* (cbr) */
          !DECL_INLINE(method) && 
#endif
          DECL_ST(method) == NULL &&
          (DECL_TEMPLATE_INFO(method) == NULL              ||
          DECL_FRIEND_PSEUDO_TEMPLATE_INSTANTIATION(method) ||
          DECL_TEMPLATE_INSTANTIATED(method)              ||
#if defined (TARG_ST) && (GNU_FRONT_END==33)
             /* (cbr) defer instantiated template */
             DECL_TEMPLATE_INSTANTIATED(method) ||
#endif  
          DECL_TEMPLATE_SPECIALIZATION(method))) {
          WFE_Process_Function_Decl (method);
      }
    }
    method = TREE_CHAIN(method);
  }
} /* WFE_Process_Class_Decl */

void
WFE_Process_Type_Decl (tree decl)
{
//fprintf(stderr, "TYPE_DECL: %s\n", IDENTIFIER_POINTER(DECL_NAME(decl)));
  if (TREE_CODE(TREE_TYPE(decl)) == RECORD_TYPE &&
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
      !(CP_TYPE_CONST_P(decl) || CP_TYPE_VOLATILE_P(decl) || CP_TYPE_RESTRICT_P (decl))) {
#else
    CP_TYPE_QUALS(decl) == TYPE_UNQUALIFIED) {
#endif
    WFE_Process_Class_Decl (TREE_TYPE(decl));
  }
} /* WFE_Process_Type_Decl */

void
WFE_Process_Template_Decl (tree decl)
{
  //fprintf(stderr, "TEMPLATE_DECL: %s\n", IDENTIFIER_POINTER(DECL_NAME(decl)));
  tree gentemp = most_general_template(decl);
#if 0
  for (tree t = DECL_TEMPLATE_SPECIALIZATIONS(gentemp);
       t; t = TREE_CHAIN(t))
    if (TREE_CODE(TREE_VALUE(t)) == FUNCTION_DECL &&
        DECL_SAVED_TREE(TREE_VALUE(t))            &&
        !DECL_EXTERNAL(TREE_VALUE(t))             &&
        !uses_template_parms (TREE_VALUE(t)))
      st = Get_ST (TREE_VALUE(t));
    DECL_TEMPLATE_SPECIALIZATIONS(gentemp) = 0; // don't do these twice
#endif
  for (tree t = DECL_TEMPLATE_INSTANTIATIONS(gentemp);
       t; t = TREE_CHAIN(t)) {
    tree val = TREE_VALUE(t);
    if (TREE_CODE(val) == RECORD_TYPE &&
        !uses_template_parms(val))
      WFE_Process_Class_Decl (val);
  }
} /* WFE_Process_Template_Decl */

bool
decl_is_needed_vtable (tree decl)
{
  bool needed = false;
  if (DECL_NAME(decl) &&
      IDENTIFIER_POINTER(DECL_NAME(decl)) &&
#ifdef KEY
      !strncmp("_ZTV", IDENTIFIER_POINTER(DECL_NAME(decl)), 4)
#else
      !strncmp("__vt_", IDENTIFIER_POINTER(DECL_NAME(decl)), 5)
#endif
      ) {
            
    tree entries = CONSTRUCTOR_ELTS (DECL_INITIAL (decl));

    for (; entries; entries = TREE_CHAIN (entries)) {

      tree fnaddr;
      tree fn;

      fnaddr = TREE_VALUE (entries);

#ifdef KEY
      if (TREE_CODE (fnaddr) == NOP_EXPR &&
	  TREE_CODE (TREE_OPERAND (fnaddr, 0)) == ADDR_EXPR) {
	fn = TREE_OPERAND (TREE_OPERAND (fnaddr, 0), 0);  // fn can be VAR_DECL
	
      } else if (TREE_CODE (fnaddr) != ADDR_EXPR) {
        /* This entry is an offset: a virtual base class offset, a
           virtual call offset, and RTTI offset, etc.  */
        continue;
      } else
        fn = TREE_OPERAND (fnaddr, 0);
#else
      if (TREE_CODE (fnaddr) != ADDR_EXPR)
        /* This entry is an offset: a virtual base class offset, a
           virtual call offset, and RTTI offset, etc.  */
        continue;

      fn = TREE_OPERAND (fnaddr, 0);
#endif

#ifdef KEY
      // As shown by bug 3133, some objects are emitted by g++ even though they
      // are weak and external.
      if (DECL_EMITTED_BY_GXX(fn)) {
	needed = TRUE;
	break;
      }
#endif

      if (!DECL_EXTERNAL(fn) &&
          !DECL_WEAK(fn)
#ifndef KEY	// Under g++ 3.2 -O3, all functions are marked DECL_INLINE.
          && !DECL_INLINE(fn)
#endif
	  ) {
        needed = TRUE;
        break;
      }
    }
  }

  return needed;
}

void
WFE_Process_Var_Decl (tree decl)
{
//fprintf(stderr, "VAR_DECL: %s\n", IDENTIFIER_POINTER(DECL_NAME(decl)));
  ST *st;
  if (TREE_PUBLIC(decl)    &&
//    !DECL_WEAK(decl)     &&
      !DECL_EXTERNAL(decl) &&
#ifdef TARG_ST
      !expanded_decl(decl)) {
#else
      !DECL_ST(decl)) {
#endif
    if (!DECL_WEAK(decl) || decl_is_needed_vtable (decl)) {
#ifdef KEY
      WFE_Expand_Decl(decl);
#else
      DECL_ST(decl) = (ST *) 1;
      Push_Deferred_Function (decl);
#endif
    }
  }
} /* WFE_Process_Var_Decl */

void
WFE_Process_Function_Decl (tree decl)
{
  //fprintf(stderr, "FUNCTION_DECL: %s\n", IDENTIFIER_POINTER(DECL_NAME(decl)));
  tree body;
  ST *st;
  body = DECL_SAVED_TREE(decl);

  if (body != NULL_TREE && !DECL_EXTERNAL(decl) &&
      !DECL_ARTIFICIAL(decl) &&
#ifdef TARG_ST
      /* (cbr) */
      (TREE_PUBLIC(decl) || TREE_USED (decl) || TREE_ADDRESSABLE (decl)) &&
#else
      !DECL_INLINE(decl) &&
#endif
      DECL_ST(decl) == NULL &&
      (DECL_TEMPLATE_INFO(decl) == NULL              ||
      DECL_FRIEND_PSEUDO_TEMPLATE_INSTANTIATION(decl) ||
      DECL_TEMPLATE_INSTANTIATED(decl)              ||
      DECL_TEMPLATE_SPECIALIZATION(decl))) {
#ifdef KEY
    set_DECL_ST(decl, (ST *) 1);
#else
    DECL_ST(decl) = (ST *) 1;
#endif
    Push_Deferred_Function (decl);
  }
} /* WFE_Process_Function_Decl */

void
WFE_Process_Decl (tree decl)
{
  switch (TREE_CODE (decl)) {

    case NAMESPACE_DECL:
      WFE_Process_Namespace_Decl (decl);
      break;

    case CONST_DECL:
      break;

    case TYPE_DECL:
      WFE_Process_Type_Decl (decl);
      break;

    case TEMPLATE_DECL:
      WFE_Process_Template_Decl (decl);
      break;

    case VAR_DECL:
      WFE_Process_Var_Decl (decl);
      break;

    case FUNCTION_DECL:
      WFE_Process_Function_Decl (decl);
      break;

    default:
      break;
  }
} /* WFE_Process_Decl */

void
WFE_Process_Namespace_Decl (tree namespace_decl)
{
//fprintf(stderr, "NAMESPACE_DECL: %s\n", IDENTIFIER_POINTER(DECL_NAME(namespace_decl)));
  tree decl;

  if (!DECL_NAMESPACE_ALIAS(namespace_decl) && (namespace_decl != std_node)) {
    for (decl = cp_namespace_decls(namespace_decl);
         decl != NULL_TREE;
         decl = TREE_CHAIN(decl)) {
      WFE_Process_Decl(decl);
    }
  }
} /* WFE_Process_Namespace_Decl */

static int WFE_Expand_Decl_stub (tree decl, void * a)
{
  WFE_Expand_Decl (decl);
  return 0;
}

extern "C"
void
WFE_Expand_Top_Level_Decl (tree top_level_decl)
{
  int error_count, sorry_count;

  if (top_level_decl == global_namespace) {
   check_gnu_errors (&error_count, &sorry_count);
   if (error_count || sorry_count)
     return;
    Init_Deferred_Function_Stack();
    Init_Deferred_Decl_Init_Stack();
  }

  if (!Enable_WFE_DFE) {
#ifdef KEY
    // Emit asm statements at global scope, before expanding the functions,
    // to prevent them from getting into wrong sections (e.g. .except_table)
    std::vector<char *>::iterator asm_iter;
    for (asm_iter = gxx_emitted_asm.begin();
         asm_iter != gxx_emitted_asm.end();
	 asm_iter++)
      WFE_Assemble_Asm (*asm_iter);
#endif
    WFE_Expand_Decl (top_level_decl);

#ifdef KEY
    // Catch all the functions that are emitted by g++ that we haven't
    // translated into WHIRL.
    std::vector<tree>::iterator it;
    int changed;
    do {
      changed = 0;
      for (it = gxx_emitted_decls.begin(); 
           it != gxx_emitted_decls.end();
           it++) {
        tree decl = *it;
        if (expanded_decl(decl) == TRUE)
          continue;
        if (TREE_CODE(decl) == FUNCTION_DECL) {
	  if (DECL_THUNK_P(decl))
	    WFE_Generate_Thunk(decl);
	  else        
	    changed |= WFE_Expand_Function_Body(decl);
#ifndef TARG_ST
	  // [SC] Do not need this since we already defer the generation
	  // of debug information for classes.

	  // Bugs 4471, 3041, 3531, 3572, 4563.
	  // In line with the non-debug compilation, delay the DST entry 
	  // creation for member functions for debug compilation.
	  // This avoids the following problem 
	  // (as noted by Tim in tree_symtab.cxx)
	  // Expanding the methods earlier "will cause error when the
	  // methods are for a class B that appears as a field in an
	  // enclosing class A.  When Get_TY is run for A, it will
	  // call Get_TY for B in order to calculate A's field ID's.
	  // (Need Get_TY to find B's TYPE_FIELD_IDS_USED.)  If
	  // Get_TY uses the code below to expand B's methods, it
	  // will lead to error because the expansion requires the
	  // field ID's of the enclosing record (A), and these field
	  // ID's are not yet defined".
	  if (Debug_Level > 0) {
	    tree context = DECL_CONTEXT(decl);
	    if (context && TREE_CODE(context) == RECORD_TYPE) {
	      // member function
	      // g++ seems to put the artificial ones in the output too 
	      // for some reason. In particular, operator= is there.  We do 
	      // want to omit the __base_ctor stuff though
	      BOOL skip = FALSE;
	      if (IDENTIFIER_CTOR_OR_DTOR_P(DECL_NAME(decl)))
	        skip = TRUE;
	      else if (DECL_THUNK_P(decl)) {
		// Skip thunk to constructors and destructors.  Bug 6427.
	        tree addr = DECL_INITIAL_2(decl);
		Is_True(TREE_CODE(addr) == ADDR_EXPR &&
			TREE_CODE(TREE_OPERAND(addr, 0)) == FUNCTION_DECL,
			("WFE_Expand_Top_Level_Decl: invalid thunk decl"));
		skip =
		  IDENTIFIER_CTOR_OR_DTOR_P(DECL_NAME(TREE_OPERAND(addr, 0)));
	      }
	      if (!skip) {
	      	TY_IDX context_ty_idx = Get_TY(context);
		// The type could have been newly created by the above Get_TY.
		// If so, call add_deferred_DST_types to create the DST for it.
		add_deferred_DST_types();
		DST_INFO_IDX context_dst_idx = TYPE_DST_IDX(context);
		DST_enter_member_function(context, context_dst_idx,
					  context_ty_idx, decl);
	      }
	    }
	  }
#endif
        } else if (TREE_CODE(decl) == VAR_DECL) {
	  WFE_Process_Var_Decl (decl);
        } else if (TREE_CODE(decl) == NAMESPACE_DECL) {
	  WFE_Expand_Decl (decl);
        } else {
	  FmtAssert(FALSE, ("WFE_Expand_Top_Level_Decl: invalid node"));
        }
      }
    } while (changed);	// Repeat until emitted all needed copy constructors.
    // Emit any typeinfos that we have referenced
    for (it = emit_typeinfos.begin(); it != emit_typeinfos.end(); ++it) {
    	tree decl = *it;
	if (expanded_decl (decl))
	    continue;
	expanded_decl (decl) = TRUE;
	FmtAssert (TREE_CODE (decl) == VAR_DECL, ("Unexpected node in typeinfo"));
	WFE_Expand_Decl (decl);
    }
#endif
  }
  else {

    WFE_Process_Namespace_Decl (top_level_decl);

    tree  decl;
    INT32 i;
    for (i = deferred_function_i;  i >= 0; --i) {
       decl = deferred_function_stack [i];
#ifdef KEY
       set_DECL_ST(decl, NULL);
#else
       DECL_ST(decl) = NULL;
#endif
    }

    ST *st;
    for (i = deferred_function_i;  i >= 0; --i) {
       decl = deferred_function_stack [i];
       st = Get_ST (decl);
//   st->Print (stderr, TRUE);
    }

    while (deferred_function_i >= 0) {
      decl = Pop_Deferred_Function ();
      if (TREE_CODE(decl) == FUNCTION_DECL)
        if (DECL_THUNK_P(decl))
          WFE_Generate_Thunk(decl);
        else        
          WFE_Expand_Function_Body (decl);
      else {
        st = DECL_ST(decl);
        if (ST_sclass(st) == SCLASS_EXTERN)
          Set_ST_sclass (st, SCLASS_UGLOBAL);
        WFE_Expand_Decl (decl);
      }
    }
  }
#ifdef KEY
  {
    int i;
    // Can't use iterator to access emit_decls because Get_TY may grow
    // emit_decls, which invalids all iterators.  Use operator[] instead.
    for (i = 0; i < emit_decls.size(); i++) {
      tree decl = emit_decls[i];
      if (TREE_CODE(decl) == VAR_DECL)
	WFE_Expand_Decl(decl);
      else if (TREE_CODE(decl) == RECORD_TYPE ||
	       TREE_CODE(decl) == UNION_TYPE)
	Get_TY(decl);
      else
	Is_True(FALSE, ("WFE_Expand_Top_Level_Decl: unexpected tree type"));

#ifdef TARG_ST
      // [SC] Sigh.  Get_TY can cause more deferred functions ... 
      while (deferred_function_i >= 0) {
	decl = Pop_Deferred_Function ();
	if (TREE_CODE(decl) == FUNCTION_DECL)
	  if (DECL_THUNK_P(decl))
	    WFE_Generate_Thunk(decl);
	  else        
	    WFE_Expand_Function_Body (decl);
	else {
	  ST *st = DECL_ST(decl);
	  if (ST_sclass(st) == SCLASS_EXTERN)
	    Set_ST_sclass (st, SCLASS_UGLOBAL);
	  WFE_Expand_Decl (decl);
	}
      }
#endif
    }
  }
  {
    // Set the type for fields whose type we want to set last.
    std::vector<std::pair<tree, FLD_HANDLE> >::iterator it;
    for (it = defer_fields.begin(); it != defer_fields.end(); ++it) {
      tree field = (*it).first;
      FLD_HANDLE fld = (*it).second;
      Is_True(TREE_CODE(field) == FIELD_DECL,
	      ("WFE_Expand_Top_Level_Decl: FIELD_DECL not found"));
      // Currently we defer only pointer types.
      Is_True(TREE_CODE(TREE_TYPE(field)) == POINTER_TYPE,
	      ("WFE_Expand_Top_Level_Decl: POINTER_TYPE not found"));
      TY_IDX fty_idx = Get_TY(TREE_TYPE(field));
      Set_FLD_type(fld, fty_idx);
    }
  }

  {
    // Create DST info.
    int i;

    // Add DSTs for types.
    add_deferred_DST_types();

    // Add DSTs for member functions.  Do this after all the types are handled.
    for (i = 0; i < defer_DST_misc.size(); i++) {
      DST_defer_misc_info *p = defer_DST_misc[i];
      if (p->is_member_function) {
	tree context = p->u1.member_func.context;
	DST_INFO_IDX context_dst_idx = TYPE_DST_IDX(context);
	TY_IDX context_ty_idx = Get_TY(context);
	DST_enter_member_function(context, context_dst_idx, context_ty_idx,
				  p->u1.member_func.fndecl);
      }
    }

    // Add DSTs for new types created.  (Don't know if any new type is ever
    // created.)
    add_deferred_DST_types();
  }
#endif
} /* WFE_Expand_Top_Level_Decl */

#ifdef KEY
// Add DSTs for the defered types.  We defer adding DSTs for types in order to
// avoid calling Get_TY on partially constructed structs, since their field IDs
// are not yet valid.  It is safe to call add_deferred_DST_types to add DSTs
// whenever we are sure there are no partially constructed types.
void
add_deferred_DST_types()
{
  static int last_type_index = -1;
  int i;

  for (i = last_type_index + 1; i < defer_DST_types.size(); i++) {
    DST_defer_type_info *p = defer_DST_types[i];
    DST_INFO_IDX dst = Create_DST_type_For_Tree(p->t, p->ttidx, p->idx);
    TYPE_DST_IDX(p->t) = dst;
  }
  last_type_index = defer_DST_types.size() - 1;
}
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
