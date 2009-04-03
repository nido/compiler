/* WFE == WHIRL Front End */
/* translate gnu expr trees to whirl */

#ifndef wn_expr_INCLUDED
#define wn_expr_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* One time initialization */
extern void WFE_Expr_Init (void);

/* expand one gnu stmt tree into symtab & whirl */
extern void WFE_One_Stmt (tree exp);

#ifdef KEY
/* same as WFE_One_Stmt but use new label indexes */
extern void WFE_One_Stmt_Cleanup (tree exp);
#endif

/* generate a RET statement */
extern void WFE_Null_Return (void);

#if defined TARG_ST && defined (_NO_WEAK_SUPPORT_)
extern void WFE_Expand_Start_Stmt_Expr (tree t);
extern void WFE_Expand_End_Stmt_Expr (tree t);
extern void WFE_Expand_Start_Compound_Literal_Expr (tree t);
extern void WFE_Expand_End_Compound_Literal_Expr (tree t);
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// expand one gnu expr tree into whirl; called only from C++ files
extern WN * WFE_Expand_Expr (tree exp, 
			     bool need_result = TRUE,
			     TY_IDX nop_ty_idx = 0,
			     TY_IDX component_ty_idx = 0,
			     INT64 component_offset = 0,
			     UINT32 field_id = 0,
			     bool is_bit_field = FALSE,
			     bool is_aggr_init_via_ctor = FALSE);

extern WN  *WFE_Rcomma_Block;
extern int  WFE_Disable_Rcomma;
extern WN* WFE_Expand_Expr_With_Sequence_Point (tree exp, TYPE_ID mtype);

/* rhs is the WN representing the rhs of a MODIFY_EXPR node; this routine
 * processes the lhs of the node and generate the appropriate form of store
 */
extern WN * WFE_Lhs_Of_Modify_Expr (tree_code assign_code,
				    tree lhs,
				    bool need_result,
				    TY_IDX component_ty_idx,
				    INT64 component_offset,
				    UINT32 field_id,
				    bool is_bit_field,
				    WN *rhs_wn,
				    PREG_NUM rhs_preg_num,
				    bool is_realpart,
				    bool is_imagpart);

/* get integer value from INTEGER_CST node */
extern UINT64 Get_Integer_Value (tree exp);

/* traverse the tree and addr_saved if address of a variable is taken */
extern void WFE_Set_ST_Addr_Saved (WN *);

#ifdef KEY
// Convert inplace target order words in 'buf' to host order. 'buf' is a two
// word array. 
extern void Setup_EH_Region (bool =false);
#endif

#endif /* __cplusplus */

#ifdef TARG_ST
// Make standard integral ctype promotions
extern TYPE_ID WFE_Promoted_Type(TYPE_ID mtype);
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void WFE_Expand_Start_Stmt_Expr (tree);
extern void WFE_Expand_End_Stmt_Expr (tree);
#ifdef TARG_ST
extern void WFE_Expand_Start_Compound_Literal_Expr (tree);
extern void WFE_Expand_End_Compound_Literal_Expr (tree);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
