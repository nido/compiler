/*

Copyright (C) 2008 ST Microelectronics, Inc.  All Rights Reserved.

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

Contact information:  

*/

#include <map>

#include "defs.h"
#include "errors.h"
#include "tracing.h"
#include "timing.h"
#include "stab.h"
#include "wn.h"
#include "wn_simp.h"
#include "wn_util.h"
#include "wn_lower.h"
#include "targ_sim.h"

#include "lai_loader_api.h"
#include "loader.h"
#include "ext_info.h"

#include "intrn_info.h"
#include "insn-config.h" /* for MAX_RECOG_OPERANDS */

#include "symtab_access.h"

static bool verbose_reg_placement = false;
#define VERBOSE_REG_PLACEMENT(...) if (verbose_reg_placement) { fprintf(TFile, __VA_ARGS__); }


/** 
 * Auxiliary function used to replace a stmt by another
 * 
 * @param BB 
 * @param previous_stmt 
 * @param new_stmt 
 * 
 * @return 
 */
static bool
replace_stmt(WN* BB, WN* previous_stmt, WN* new_stmt)
{

  if (new_stmt != previous_stmt)
    {
      WN_next(new_stmt) =  WN_next(previous_stmt);
      WN_prev(new_stmt) =  WN_prev(previous_stmt);
      
      if (WN_first(BB) == previous_stmt)
        WN_first(BB) = new_stmt;

      if (WN_last(BB) == previous_stmt)
        WN_last(BB) = new_stmt;

      if (WN_prev(previous_stmt)!=NULL)
        WN_next(WN_prev(previous_stmt)) = new_stmt;

      if (WN_next(previous_stmt)!=NULL)
        WN_prev(WN_next(previous_stmt)) = new_stmt;
    }
  return true;
}


/** 
 * Create new preg variable with type <ext_type> and
 * whose name is derived from var one
 * 
 * @param var 
 * @param var_type 
 * 
 * @return 
 *
 * @bug maybe we should check that new_name is really new ??
 */
static PREG_NUM
create_cvt_var(PREG_NUM var, TYPE_ID ext_type)
{
  // TODO: check that new name is really new
  const char* oldname = Preg_Name(var);
  STR_IDX idx = Save_Str2 (oldname, "_1");
  char* new_name = Index_To_Str(idx);

  return Create_Preg(MTYPE_To_TY (ext_type), new_name);
}


/**
 * Map that associates a PREG candidate for the conversion from a C native type
 * to an extension type.
 */
typedef struct {
  bool     valid;            // wether or not the association is valid
  TYPE_ID  c_type;           // original type
  TYPE_ID  equiv_ext_type;   // candidate extension type
  PREG_NUM replacement_preg; // replacement PREG, with extension type
} CandidatePreg;
typedef std::map <PREG_NUM, CandidatePreg*> CandidateMap;

// static bool
// CandidateMap_Is_Defined_Preg (CandidateMap *cmap, PREG_NUM var) {
//   return ((*cmap)[var] != NULL);
// }

static bool
CandidateMap_Is_Valid_Preg (CandidateMap *cmap, PREG_NUM var) {
  CandidatePreg* corresp = (*cmap)[var];
  return (corresp!=NULL && corresp->valid);
}

// static TYPE_ID
// CandidateMap_Get_C_Type (CandidateMap *cmap, PREG_NUM var) {
//   CandidatePreg* corresp = (*cmap)[var];
//   FmtAssert((corresp!=NULL), ("PReg not yet defined in CandidateMap"));
//   return (corresp->c_type);
// }

static TYPE_ID
CandidateMap_Get_Ext_Type (CandidateMap *cmap, PREG_NUM var) {
  CandidatePreg* corresp = (*cmap)[var];
  FmtAssert((corresp!=NULL), ("PReg not yet defined in CandidateMap"));
  return (corresp->equiv_ext_type);
}

// static void
// CandidateMap_Set_Valid_Preg (CandidateMap *cmap, PREG_NUM var) {
//   CandidatePreg* corresp = (*cmap)[var];
//   FmtAssert((corresp!=NULL), ("PReg not yet defined in CandidateMap"));
//   corresp->valid = true;
// }

// static void
// CandidateMap_Set_Invalid_Preg (CandidateMap *cmap, PREG_NUM var) {
//   CandidatePreg* corresp = (*cmap)[var];
//   FmtAssert((corresp!=NULL), ("PReg not yet defined in CandidateMap"));
//   corresp->valid = false;
// }

/**
 * If parameter preg <var> has not yet been inserted in the <cmap>, do it.
 *
 * @param cmap 
 * @param var
 * @param equiv_type
 */
static void
CandidateMap_Add_Valid_Preg(CandidateMap *cmap, PREG_NUM var, TYPE_ID src_type, TYPE_ID equiv_type) {
  CandidatePreg* corresp = (*cmap)[var];
  if (corresp == NULL) {
    corresp = TYPE_MEM_POOL_ALLOC(CandidatePreg, Malloc_Mem_Pool);
    corresp->valid            = true;
    corresp->c_type           = src_type;
    corresp->equiv_ext_type   = equiv_type;
    corresp->replacement_preg = 0;
    (*cmap)[var] = corresp;
    
    VERBOSE_REG_PLACEMENT("CandidateMap: Adding preg %d c_type = %s, equiv_type = %s\n",
			   var, MTYPE_name(src_type), MTYPE_name(equiv_type));
  }
  else {
    if (corresp->equiv_ext_type != equiv_type) {
      VERBOSE_REG_PLACEMENT("CandidateMap: Preg %d was already added with another equiv type!! Mark it as invalid\n", var);
      corresp->valid = false;
    }
  }
}

/**
 * Return the PREG that will replace existing one, after generating it if not yet done
 */
static PREG_NUM
CandidateMap_Get_Associated_Extension_Preg(CandidateMap *cmap, PREG_NUM var) {
  CandidatePreg* corresp = (*cmap)[var];
  FmtAssert((corresp!=NULL), ("PReg not yet defined in CandidateMap"));
  if (corresp->replacement_preg == 0) {
    // Create new ST configured with equivalent type
    PREG_NUM cvtvar = create_cvt_var(var, corresp->equiv_ext_type);
    corresp->replacement_preg = cvtvar;
    VERBOSE_REG_PLACEMENT("CandidateMap: Preg %d to be replaced by Preg %d\n", var, cvtvar);
  }
  return corresp->replacement_preg;
}


/** 
 * Check wether preg is a dedicated register.
 * 
 * @param num 
 * 
 * @return 
 */
static bool
Is_Dedicated(PREG_NUM num) {
  return ((int)num<= Last_Dedicated_Preg_Offset);
}


/**
 * Try to remove the conversion of the parameter tree which looks like ISTORE(CVT(...))
 *
 * @param tree ISTORE node whose kid0 is a CVT
 *
 * @return potentially updated tree
 */
static WN *
Simplify_IStore_Cvt(WN *tree) {
  WN *cvt_node = WN_kid0(tree);
  if (MTYPE_is_dynamic(WN_desc(cvt_node)) &&
      EXTENSION_Are_Equivalent_Mtype(WN_desc(cvt_node), WN_rtype(cvt_node))) {
    // Local placement optimization
    VERBOSE_REG_PLACEMENT("Simplify ctyp_ISTORE(ctyp_CVT(exttyp_...)) by exttyp_ISTORE\n");
    TYPE_ID ext_type = WN_desc(cvt_node);
    TY_IDX  new_type = Make_Pointer_Type (MTYPE_To_TY(ext_type)); // TODO: need to propagate initial type alignment?
    WN *new_tree = WN_Istore(ext_type, WN_offset(tree), new_type,
			     /*addr*/WN_kid1(tree), /*val*/WN_kid0(cvt_node),
			     WN_field_id(tree));
    WN_copy_linenum(tree, new_tree);
    WN_Delete(tree);
    WN_Delete(cvt_node);
    tree = new_tree;
  }
  return tree;
}


/**
 * =============================================================================
 * Next come a couple of functions used to detect PREG variable candidate
 * for automatic conversion from Native C type to extension type
 * =============================================================================
 */
/** 
 * Walk through an expression tree and perform the following actions:
 * - Collect PREG candidate for automatic placement to extension type,
 * - Optimize ext_type_CVT(ILOAD(native_type)) to ILOAD(ext_type)
 * - Optimize ext_type_CVT(LDID(native_type)) to LDID(ext_type) for non-PREG variable
 * 
 * @param expr          current expression
 * @param candidate_map map containing already detected candidates
 * 
 * @return processed expression
 */
static WN *
EXT_LOWER_find_register_candidate_expr(WN* expr, CandidateMap* candidate_map)
{
  OPERATOR opr = WN_operator(expr);
  TYPE_ID desc_type = WN_desc(expr);
  TYPE_ID res_type  = WN_rtype(expr);
    
  if (opr == OPR_CVT && MTYPE_is_dynamic(res_type) &&
      EXTENSION_Are_Equivalent_Mtype(desc_type, res_type)) {
    // Found conversion to extension type
    WN*      kid     = WN_kid0(expr);
    OPERATOR kid_opr = WN_operator(kid);
    
    if (kid_opr == OPR_LDID) {
      if (ST_class(WN_st(kid)) == CLASS_PREG && !Is_Dedicated(WN_offset(kid))) {
	// Register corresponding PREG as candidate
	PREG_NUM preg_var = WN_offset(kid);
	CandidateMap_Add_Valid_Preg(candidate_map, preg_var, desc_type, res_type);
      }
      else if (ST_class(WN_st(kid)) != CLASS_PREG) {
	// Local placement optimization
	OPCODE newopc;
	VERBOSE_REG_PLACEMENT("Simplify exttyp_CVT(ctyp_LDID) by exttyp_LDID\n");
	newopc = OPCODE_make_op(OPR_LDID, res_type, res_type);
	WN_set_opcode(kid, newopc);
	WN_Delete(expr); // Delete CVT node
	expr = kid;
      }
    }
    else if (kid_opr == OPR_ILOAD) {
      // Local placement optimization
      OPCODE newopc;
      VERBOSE_REG_PLACEMENT("Simplify exttyp_CVT(ctyp_ILOAD) by exttyp_ILOAD\n");
      newopc = OPCODE_make_op(OPR_ILOAD, res_type, res_type);
      WN_set_opcode(kid, newopc);
      WN_Delete(expr); // Delete CVT node
      expr = kid;
    }
  }

  return expr;
}


/** 
 * Find variables with a native C type that could benefit to be placed
 * on an extension registers rather than on core registers. For each 
 * candidate, a new entry is created in <candidate_map>, indexed by its
 * preg id.
 * Also optimize c_type_ISTORE(CVT(ext_type)) to ext_type_ISTORE
 * 
 * @param tree          expression to process
 * @param candidate_map map containing already detected candidates
 * 
 * @return 
 */
static WN *
EXT_LOWER_find_register_candidate(WN* tree, CandidateMap *candidate_map)
{
  OPERATOR opr = WN_operator(tree);
  INT i;

  // lower kids first
  INT nb_operands = WN_kid_count(tree);
  for (i=0; i<nb_operands; i++) {
    WN_kid(tree, i) = EXT_LOWER_find_register_candidate(WN_kid(tree, i), candidate_map);
  }

  if (OPCODE_is_stmt(WN_opcode(tree))) {
    
    if (opr == OPR_ISTORE) {
      if (WN_operator(WN_kid0(tree)) == OPR_CVT) {
	tree = Simplify_IStore_Cvt(tree);
      }
    }
    else if (opr == OPR_STID) {
      WN *kid0 = WN_kid0(tree);
      if (WN_operator(kid0) == OPR_CVT && MTYPE_is_dynamic(WN_desc(kid0)) &&
	  EXTENSION_Are_Equivalent_Mtype(WN_desc(kid0), WN_rtype(kid0))) {
	// Found conversion from extension type to an equivalent type
	if (ST_class(WN_st(tree)) == CLASS_PREG && !Is_Dedicated(WN_offset(tree))) {
	  // Register corresponding PREG as candidate
	  PREG_NUM preg_var = WN_offset(tree);
	  TYPE_ID c_type    = WN_rtype(kid0);
	  TYPE_ID ext_type  = WN_desc(kid0);
	  CandidateMap_Add_Valid_Preg(candidate_map, preg_var, c_type, ext_type);
	}
	else if (ST_class(WN_st(tree)) != CLASS_PREG) {
	  // Local placement optimization
	  TYPE_ID c_type   = WN_rtype(kid0);
	  TYPE_ID ext_type = WN_desc(kid0);
	  VERBOSE_REG_PLACEMENT("Simplify %sSTID(%s%sCVT) by %sSTID\n",
				MTYPE_name(c_type), MTYPE_name(c_type),
				MTYPE_name(ext_type), MTYPE_name(ext_type));
	  OPCODE newopc = OPCODE_make_op(OPR_STID, MTYPE_V, ext_type);
	  WN_set_opcode(tree, newopc);
	  WN_kid0(tree) = WN_kid0(kid0);
	  WN_Delete(kid0);
	}
      }
    }
  }
  else if (OPCODE_is_scf(WN_opcode(tree))) {
    // OPR_BLOCK type is the only one that required a special handling
    // for a complete tree traversal (because stmt are not in kids)
    if (opr == OPR_BLOCK) {
      WN *stmt, *stmt2;
      for (stmt = WN_first(tree); stmt; stmt = WN_next(stmt)) {
	stmt2 = EXT_LOWER_find_register_candidate(stmt, candidate_map);
	if (stmt2 != stmt) {
	  replace_stmt(tree, stmt, stmt2);
	}
      }
    }
  }
  else if (OPCODE_is_expression(WN_opcode(tree))) {
    tree = EXT_LOWER_find_register_candidate_expr(tree, candidate_map);
  }

  return  tree;
}


/**
 * =============================================================================
 * Next come a couple of functions used to effectively perform the type
 * conversion of candidate PRegs.
 * =============================================================================
 */
/** 
 * Effective conversion of candidate PRegs.
 * 
 * @param expr          current expression
 * @param candidate_map map containing already detected candidates
 * 
 * @return processed expression
 */
static WN *
EXT_LOWER_convert_valid_candidate_expr(WN* expr, CandidateMap *candidate_map)
{
  OPERATOR opr = WN_operator(expr);
  TYPE_ID desc = WN_desc(expr);
  TYPE_ID res = WN_rtype(expr);

  // Match CVT(CVT) and CVT(LDID)
  if (opr == OPR_CVT && EXTENSION_Are_Equivalent_Mtype(desc, res)) {
    WN *kid = WN_kid0(expr);
    OPERATOR kid_opr = WN_operator(kid);
    
    if (kid_opr == OPR_CVT) {
      // Useless dual conversion
      if (res == WN_desc(kid)) {
	VERBOSE_REG_PLACEMENT("Remove useless dual conversion\n");
	WN *new_expr = WN_kid0(kid);
	WN_Delete(kid);
	WN_Delete(expr);
	return new_expr;
      }
    }
    else if (kid_opr == OPR_LDID) {
      if (!(ST_class(WN_st(kid)) == CLASS_PREG && !Is_Dedicated(WN_offset(kid)))) {
	// LDID from memory
	VERBOSE_REG_PLACEMENT("Simplify exttyp_CVT(ctyp_LDID(<mem>)) by exttyp_LDID(<mem>)\n");
	
	OPCODE newopc;
	newopc = OPCODE_make_op(OPR_LDID, res, res);
	WN_set_opcode(kid, newopc);
	WN_Delete(expr); // Delete CVT node
	return kid;
      }
    }
  }

  // Match LDID
  if (opr == OPR_LDID &&
      (ST_class(WN_st(expr)) == CLASS_PREG && !Is_Dedicated(WN_offset(expr)))) {
    PREG_NUM preg_var = WN_offset(expr);
    if (CandidateMap_Is_Valid_Preg(candidate_map, preg_var)) {
      
      PREG_NUM new_ext_var = CandidateMap_Get_Associated_Extension_Preg(candidate_map, preg_var);
      
      VERBOSE_REG_PLACEMENT("Replace LDID(PREG:%d) by CVT(LDID(PREG:%d))\n", preg_var, new_ext_var);
      
      // Add conversion
      TYPE_ID ext_type = CandidateMap_Get_Ext_Type(candidate_map, preg_var);
      TYPE_ID c_type   = res;
      WN *new_expr = WN_Cvt(ext_type, c_type,
			    WN_LdidPreg(ext_type, new_ext_var));
      return new_expr;
    }
  }
  return expr;
}


/** 
 * Effective placement of candidate PRegs.
 * 
 * @param tree          expression to process
 * @param candidate_map map containing already detected candidates
 * 
 * @return 
 */
static WN *
EXT_LOWER_convert_valid_candidate(WN* tree, CandidateMap* candidate_map)
{
  OPERATOR opr = WN_operator(tree);
  INT i;

  // lower kids first
  INT nb_operands = WN_kid_count(tree);
  for (i=0; i<nb_operands; i++) {
    WN_kid(tree, i) = EXT_LOWER_convert_valid_candidate(WN_kid(tree, i), candidate_map);
  }
  
  if (OPCODE_is_stmt(WN_opcode(tree))) {

    if (opr == OPR_ISTORE) {
      if (WN_operator(WN_kid0(tree)) == OPR_CVT) {
	tree = Simplify_IStore_Cvt(tree);
      }
    }
    else if (opr == OPR_STID) {
      if (ST_class(WN_st(tree)) == CLASS_PREG && !Is_Dedicated(WN_offset(tree))) {
	PREG_NUM cur_preg = WN_offset(tree);
	if (CandidateMap_Is_Valid_Preg(candidate_map, cur_preg)) {
	  PREG_NUM new_preg = CandidateMap_Get_Associated_Extension_Preg(candidate_map, cur_preg);
	  TYPE_ID  new_preg_type = CandidateMap_Get_Ext_Type(candidate_map, cur_preg);
	  
	  // STID type will change, so insure that kid0 has the correct type
	  // by either adding, removing or updating conversion node
	  WN *kid0 = WN_kid0(tree);
	  if (WN_operator(kid0) == OPR_CVT) {
	    
	    if (WN_desc(kid0) == new_preg_type) {
	      WN_kid0(tree) = WN_kid0(kid0);
	      WN_Delete(kid0); // Delete CVT node
	    }
	    else {
	      WN_kid0(tree) = WN_Cvt(WN_rtype(kid0), new_preg_type, kid0);
	    }
	  }
	  else {
	    if (WN_operator(kid0) == OPR_LDID && ST_class(WN_st(kid0)) != CLASS_PREG) {
	      // LDID from memory, conversion can be avoided
	      OPCODE newopc;
	      newopc = OPCODE_make_op(OPR_LDID, new_preg_type, new_preg_type);
	      WN_set_opcode(kid0, newopc);
	    }
	    else {
	      WN_kid0(tree) = WN_Cvt(WN_rtype(kid0), new_preg_type, kid0);
	    }
	  }
	  
	  VERBOSE_REG_PLACEMENT("Replace STID(%d, CVT()) by STID(%d)\n", cur_preg, new_preg);
	  
	  // Effective replacement of preg
	  WN *new_tree = WN_StidIntoPreg(new_preg_type, new_preg,
					 MTYPE_To_PREG(new_preg_type/*OK?*/),
					 WN_kid0(tree));
	  WN_copy_linenum(tree, new_tree);
	  WN_Delete(tree);
	  tree = new_tree;
	}
      }
    }
  }
  else if (OPCODE_is_scf(WN_opcode(tree))) {
    // OPR_BLOCK type is the only one that required a special handling
    // for a complete tree traversal (because stmt are not in kids)
    if (opr == OPR_BLOCK) {
      WN *stmt, *stmt2;
      for (stmt = WN_first(tree); stmt; stmt = WN_next(stmt)) {
	stmt2 = EXT_LOWER_convert_valid_candidate(stmt, candidate_map);
	if (stmt2 != stmt) {
	  replace_stmt(tree, stmt, stmt2);
	}
      }
    }
  }
  else if (OPCODE_is_expression(WN_opcode(tree))) {
    tree = EXT_LOWER_convert_valid_candidate_expr(tree, candidate_map);
  }

  return  tree;
}


/** 
 * Create a CVT node to convert tree result into dst TYPE if necessary
 * 
 * @param dst destination type
 * @param tree 
 * 
 * @return new tree with conversion node.
 */
static WN*
Create_Convert_Node(TYPE_ID dst, WN* tree)
{
  /* simplify CVT(CVT(..)) pattern if CVT nodes are useless dst==src. */
  if (WN_operator(tree)==OPR_CVT &&
      WN_desc(tree)==dst)
    {
      return WN_kid(tree, 0);
    }
  
  /* conversion node is necessary and thus generated */
  if (WN_rtype(tree) != dst) {
    return WN_Cvt(WN_rtype(tree), dst, tree);
  }

  /* no conversion necessary */
  return tree;
}


/** 
 * auxiliary function that builds an intrinsic WN.
 * 
 * @param res 
 * @param intrinsic 
 * @param nbkids 
 * @param kids 
 * 
 * @return 
 */
static WN*
Create_Intrinsic_from_OP(TYPE_ID res,
                                     INTRINSIC intrinsic, int nbkids,
                                     WN *kids[])
{
  int i;
  
  for (i=0; i<nbkids; i++)
    {
      kids[i] = WN_CreateParm (WN_rtype(kids[i]),
                               kids[i],
                               Be_Type_Tbl(WN_rtype(kids[i])),
                               WN_PARM_BY_VALUE | WN_PARM_READ_ONLY);
    }
  return WN_Create_Intrinsic(OPCODE_make_op(OPR_INTRINSIC_OP,
                                            res, 
                                            MTYPE_V),
                             intrinsic, nbkids, kids);
}


/** 
 * Lower reccursively tree to find potential instrinsic mappings.
 * 
 * @param tree 
 * 
 * @return lowered tree
 */
static WN *
EXT_LOWER_expr(WN *tree, BOOL* modified)
{
  WN *kids[MAX_RECOG_OPERANDS];
  int i, nb_operands = WN_kid_count(tree);

  // lower kids first
  for (i=0; i<nb_operands; i++){
    WN_kid(tree, i) = EXT_LOWER_expr(WN_kid(tree, i), modified);
  }
  
  // get intrinsic idx if any.
  OPCODE opc = WN_opcode(tree);

  INT itrnidx;
  INT cost = INT_MAX;
  INTRINSIC_Vector_t* itrn_indexes =  Get_Intrinsic_from_OPCODE(opc);

  /* No intrinsic correspond to opcode */
  if (itrn_indexes == NULL) {
    return tree;
  }
    
  /* select the best mapping for opcode */
  for (i=0; i<itrn_indexes->size(); i++) {
    INT new_cost;
    INT new_itrnidx = (*itrn_indexes)[i];
    /* operator mapped on core */
    if ( new_itrnidx == OPCODE_MAPPED_ON_CORE)
      return tree;
    if (! EXTENSION_Is_Meta_INTRINSIC(new_itrnidx)) {
      /* for non meta intrinsic, cost is set to 0 */
      itrnidx = new_itrnidx;
      cost = 0;
    } else {
      /* select better meta instruction according to size or cycles
         criterium */
      if (Meta_Instruction_By_Size) {
        new_cost = EXTENSION_Get_Meta_INTRINSIC_Asm_Size(new_itrnidx);
      } else {
        new_cost = EXTENSION_Get_Meta_INTRINSIC_Asm_Cycles(new_itrnidx);
      }
      /* select meta intrinsic if cost is lower */
      if ( new_cost < cost ) {
        itrnidx = new_itrnidx;
        cost = new_cost;
      }
    }
  }

  if (cost > Meta_Instruction_Threshold)
    return tree;

  FmtAssert((nb_operands<=MAX_RECOG_OPERANDS),
            ("intrinsic nboperand %d cannot exceed %d", nb_operands,
             MAX_RECOG_OPERANDS));
  
  TYPE_ID rettype = INTRN_return_type(INTRN_proto_info(itrnidx));

  FmtAssert((rettype != MTYPE_UNKNOWN), ("no rettype for intrinsic !"));

  /* convert all kids to the proper intrinsic type */
  for (i=0; i<nb_operands; i++) {
    kids[i] = Create_Convert_Node(INTRN_proto_info(itrnidx)->arg_type[i],
                                  WN_kid(tree, i));
  }
  
  *modified = TRUE;

  /* create intrinsic op */
  return Create_Convert_Node
    (WN_rtype(tree),
     Create_Intrinsic_from_OP(rettype, itrnidx, nb_operands, kids));
}

/** 
 * This function detects CVT(INTCONST(0)) pattern and replace it, when
 * possible, with INTRINSIC_OP(CLR).
 * 
 * @param tree 
 * 
 * @return 
 */
static WN *
EXT_LOWER_CLR_detect_expr(WN *tree, BOOL *modified)
{
  int i, nb_operands = WN_kid_count(tree);

  // lower kids first
  for (i=0; i<nb_operands; i++){
    WN_kid(tree, i) = EXT_LOWER_CLR_detect_expr(WN_kid(tree, i), modified);
  }
  
  OPERATOR opr = WN_operator(tree);

  if (opr == OPR_CVT) {
    WN* kid0 = WN_kid(tree, 0);
    if (WN_operator(kid0) == OPR_INTCONST) {
      INT64 value = WN_const_val(kid0);
      INT clr_intrn = EXTENSION_Get_CLR_Intrinsic(WN_rtype(tree));
      if (value==0 && clr_intrn!=-1) {
        *modified = TRUE;
        return Create_Intrinsic_from_OP(WN_rtype(tree), clr_intrn, 0, NULL);
      }
    }
  }

  return tree;
}

/** 
 * generic WN tree parsing code. It parses all expr in the parameter
 * tree and execute the parameter function (*expr_fct) on them. The
 * result of the function is stored in place of the original expr.
 * 
 * This function is used for general lowering of whirl node into
 * extension code (EXT_LOWER_expr) and for CLR detection/selection
 * after reg placement.
 *
 *
 * @param tree 
 * @param expr_fct 
 * 
 * @return 
 */
static WN *
EXT_LOWER_stmt_wn_gen(WN *tree, WN* (*expr_fct)(WN *tree, BOOL* modified))
{
  INT i;
  BOOL modified = FALSE;

  WN_ITER* itr = WN_WALK_StmtIter(tree);
  for ( ; itr; itr = WN_WALK_StmtNext(itr)) {
    WN* wn = itr->wn;
    
    DevAssert((wn!=NULL), ("Unexpected NULL stmt"));
    if (wn!=NULL) {
      for (i = 0; i < WN_kid_count(wn); i++) {
        if (WN_kid(wn,i)!=NULL &&
            OPCODE_is_expression(WN_opcode(WN_kid(wn,i)))) {
          WN_kid(wn,i) = (*expr_fct)(WN_kid(wn,i), &modified);
        }
      }
    }
  }
  
  if (modified)
    return  WN_Simplify_Tree(tree);
  return tree;
}

/* ====================================================================
 * Top level routine for lowering OPERATORs not supported in the target
 * ISA to INTRINSIC_OPs corresponding to target's runtime support.
 * L WHIRL assumed.
 * register placement between extension and core registers is also
 * done here.
 * ==================================================================== */
WN *
EXT_lower_wn(WN *tree)
{

  if (! Enable_Extension_Native_Support) {
    // native support deactivated.
    return tree;
  }

  bool simpfold = WN_Simp_Fold_ILOAD;
  WN_Simp_Fold_ILOAD = TRUE;

  //  Start_Timer(T_Lower_CU);
  Set_Error_Phase("EXT Lowering");

  WN_Lower_Checkdump("EXT Lowering", tree, 0);


  if ( Enable_Extension_Native_Support & EXTENSION_NATIVE_CODEGEN) {
    tree = EXT_LOWER_stmt_wn_gen(tree, EXT_LOWER_expr);
  }
  //  Stop_Timer(T_Lower_CU);

  WN_Lower_Checkdump("After EXT lowering", tree, 0);

  WN_verifier(tree);

  Set_Error_Phase("EXT reg placement Lowering");

  WN_Lower_Checkdump("EXT reg placement Lowering", tree, 0);

  if ( Enable_Extension_Native_Support & EXTENSION_NATIVE_REG_PLACEMENT) {
    CandidateMap candidate_map;
    FmtAssert((WN_operator(tree) == OPR_FUNC_ENTRY), ("Unexpected node type in EXT_lower_wn"));
    verbose_reg_placement = Get_Trace(TP_EXTENSION, TRACE_EXTENSION_REG_PLACEMENT_MASK);
    VERBOSE_REG_PLACEMENT("REG_PLACEMENT: Processing function ## '%s' ##\n", ST_name(WN_st_idx(tree)));
    WN_func_body(tree) = EXT_LOWER_find_register_candidate(WN_func_body(tree), &candidate_map);
    if (!candidate_map.empty()) {
      VERBOSE_REG_PLACEMENT("REG_PLACEMENT: Replace candidate PREG\n");
      WN_func_body(tree) = EXT_LOWER_convert_valid_candidate (WN_func_body(tree), &candidate_map);
    }
  }
  //  Stop_Timer(T_Lower_CU);

  WN_Lower_Checkdump("After EXT reg placement lowering", tree, 0);

  Set_Error_Phase("EXT CLR detection Lowering");


  if ( Enable_Extension_Native_Support & EXTENSION_NATIVE_CLRGEN) {
    tree = EXT_LOWER_stmt_wn_gen(tree, EXT_LOWER_CLR_detect_expr);
  }
  
  WN_Lower_Checkdump("After CLR detection Lowering", tree, 0);
  
  WN_verifier(tree);
  
  WN_Simp_Fold_ILOAD = simpfold;
  
  return tree;
}

