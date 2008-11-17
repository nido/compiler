/* WFE == WHIRL Front End */
/* translate gnu stmt trees to whirl */

#ifndef wn_stmt_INCLUDED
#define wn_stmt_INCLUDED

extern void WFE_Stmt_Init (void);
extern LABEL_IDX WFE_Get_LABEL (tree label, int def);
extern void WFE_Check_Undefined_Labels(void);
extern void Mark_Scopes_And_Labels (tree);
#ifndef KEY
extern void Push_Temp_Cleanup (tree, bool);
#endif
extern void Do_Temp_Cleanups (tree);
extern void Do_Handlers (void);
extern void Do_EH_Cleanups (void);
#ifdef TARG_ST
extern void Do_Cleanups_For_EH (void);
#endif
extern void Call_Throw();

#ifdef TARG_ST
/* (cbr) support for deferred cleanups */
void Push_Cleanup_Deferral ();
ST* Get_Deferred_Cleanup ();
#endif

#ifdef KEY
extern void Push_Temp_Cleanup (tree, bool, bool=0);
extern void Do_EH_Tables (void);
extern void Call_Terminate();
extern LABEL_IDX lookup_cleanups(INITV_IDX&);
extern bool try_block_seen;
extern bool in_cleanup;
#ifdef TARG_ST
// (cbr) 
extern bool need_manual_unwinding;
#endif
#endif
extern INT Current_Handler_Count();
extern void Add_Handler_Info (WN * call_wn, INT i, INT num_handlers);

extern
#ifdef __cplusplus
"C" {
#endif
void WFE_Expand_Stmt (tree stmt);

#ifdef _NO_WEAK_SUPPORT_
extern void WFE_Expand_Start_Case (tree selector, tree sel_type);
extern void WFE_Expand_End_Case (void);
extern void WFE_Add_Case_Node (tree low, tree high, tree label);
extern void WFE_Record_Switch_Default_Label (tree label);
extern void WFE_Expand_Start_Case_Dummy (void);
extern void WFE_Expand_End_Case_Dummy (void);
extern void WFE_Expand_Return (tree stmt, tree retval);
extern void WFE_Expand_Exit_Something (struct nesting *n,
                           struct nesting *cond_stack,
                           struct nesting *loop_stack,
                           struct nesting *case_stack,
				       LABEL_IDX      *label_idx);
extern void WFE_Expand_Exit_Loop_If_False (struct nesting *whichloop, tree cond);
extern void WFE_Expand_Exit_Loop (struct nesting *whichloop);
extern void WFE_Expand_Continue (void);
extern void WFE_Expand_Continue_Loop (struct nesting *whichloop);
extern void WFE_Expand_End_Loop (void);
extern void WFE_Expand_Loop_Continue_Here (void);
extern void WFE_Expand_Start_Loop_Continue_Elsewhere (int exitflag, struct nesting *whichloop);
extern void WFE_Expand_Start_Loop (int exitflag, struct nesting *whichloop);
extern void WFE_Expand_End_Cond (void);
extern void WFE_Expand_Start_Else (void);
extern void WFE_Expand_Start_Cond (tree cond, int exitflag);
extern void Wfe_Expand_Asm_Operands (tree  string,
			 tree  outputs,
			 tree  inputs,
			 tree  clobbers,
			 int   vol,
			 char *filename,
  		     int   line);
extern void WFE_Expand_Goto (tree label);
extern void WFE_Declare_Nonlocal_Label (tree label);
extern void WFE_Expand_Label (tree label);
extern void WFE_Expand_Computed_Goto (tree exp);
extern void Push_Scope_Cleanup (tree t, bool eh_only);
extern void Start_Lexical_Block(LEXICAL_BLOCK_INFO* lexical_block);
extern void End_Lexical_Block(LEXICAL_BLOCK_INFO* lexical_block);
extern void Pop_Scope_And_Do_Cleanups (void);
extern LEXICAL_BLOCK_INFO* Pop_Lexical_Block ();
extern LEXICAL_BLOCK_INFO* Push_Lexical_Block ();
extern void WFE_Add_Aggregate_Init_Real (REAL_VALUE_TYPE real, INT size);
#endif
#ifdef __cplusplus
}
#endif

#ifdef KEY
extern LABEL_IDX WFE_unusable_label_idx;
extern LABEL_IDX WFE_last_label_idx;
#endif

#ifdef TARG_ST
extern bool Current_Function_Has_EH_Spec();

#endif

#endif

