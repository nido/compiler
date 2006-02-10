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
void Push_Cleanup_Deferral (tree);
ST* Get_Deferred_Cleanup ();
void Pop_Cleanup_Deferral (); 
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
"C"
#endif
void WFE_Expand_Stmt (tree stmt);

#ifdef KEY
extern LABEL_IDX WFE_unusable_label_idx;
extern LABEL_IDX WFE_last_label_idx;
#endif

#ifdef TARG_ST
extern bool Current_Function_Has_EH_Spec();
#endif

#endif

