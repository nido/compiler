
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize/Finalize.
 * To be Called once before all operations.
 */
extern void CGG_Initialize(void);
extern void CGG_Finalize(void);

/*
 * This function is the entry point for the code generator.
 * The implementation invoques the generated code generator
 * from the targ_wn_to_cgir.md model.
 * It is a direct replacement for Expand_Expr in whirl2ops.cxx
 */
extern TN * CGG_Expand_Expr(WN *expr, WN *parent, TN *result, OPS *ops);

/*
 * Set tracing mode.
 * 0: no trace
 * >0: trace on
 */
extern void CGG_Set_Trace(int i);

/*
 * Set optimization level
 * 0: default
 * 1: use immediate operands
 * 2: full pattern matcher
 */
extern void CGG_Set_Level(int i);

/*
 * Starts/Ends a new function block.
 * This function informs the code generator that
 * a new function was started/ended.
 */
extern void CGG_Start_function(WN *stmt);
extern void CGG_End_function(WN *stmt);

/*
 * Starts/Ends a new region.
 * This function informs the code generator that
 * a new region was started/ended.
 */
extern void CGG_Start_region(WN *stmt);
extern void CGG_End_region(WN *stmt);

/*
 * Starts a new basic block.
 * This function informs the code generator that
 * a new basic block was started
 */
extern void CGG_Start_bb(WN *stmt);

/*
 * Starts a new stmt.
 * This function informs the code generator that
 * a new stmt was started
 */
extern void CGG_Start_stmt(WN *stmt);

#ifdef __cplusplus
}
#endif
