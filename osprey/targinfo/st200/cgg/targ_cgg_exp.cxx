#include <stdlib.h>
#include "defs.h"
#include "config.h"
#include "glob.h"
#include "tracing.h"

#include "opcode.h"
#include "intrn_info.h"
#include "cgir.h"
#include "wn.h"
#include "wn_util.h"
#include "ir_reader.h"

#include "NBG_burs.h"	/* Included from NBG_PATH. */

#include "targ_cgg_exp.h"

static int trace_cgg;

#ifdef __cplusplus
extern "C" {
#endif

extern int CGG_trace_level;
extern int CGG_debug_level;
extern int CGG_opt_level;
extern void CGG_set_reuse_state(WN *tree, NBG_State state);

void
CGG_Set_Trace(int i)
{
  CGG_trace_level = i;
  trace_cgg = i;
}

void
CGG_Set_Level(int i)
{
  CGG_opt_level = i;
}


/* defined in <target>.gen.c. */
extern void CGG_visit_td_prealloc(NBG_Link LINK, void *STATE);

TN * 
CGG_Expand_Expr(WN *expr, WN *parent, TN *result, OPS *ops)
{
  NBG_State state;
  NBG_Link link;
  const char *str;

  if ((str = getenv("CGG_DEBUG")) != NULL) CGG_debug_level = atoi(str);

  FmtAssert(ops != NULL, ("unexpected NULL ops passed to CGG"));
  FmtAssert(expr != NULL, ("unexpected NULL expr passed to CGG"));
  FmtAssert(result == NULL, ("unexpected non NULL result passed to CGG, operator: %s", OPCODE_name(WN_opcode(expr))));

  if (trace_cgg) {
    fprintf (TFile,
	     "----- CGG_Expand_Expr at %s...-------------------\n",  OPCODE_name(WN_opcode(expr)));
  }

  if (CGG_opt_level < 2) {
    state = NBG_label(expr, NULL);
  } else {
    state = NBG_label_uncse(expr, NULL);
  }

  if (trace_cgg) {
    fprintf (TFile, "  NBG_State after label:\n");
    NBG_labelled_print(state, TFile);
  }

  if (!NBG_matched(state)) {
    fprintf (stderr, "!!! ERROR in CGG_Expand_Expr\n");
    fprintf (stderr, "    no match for tree:\n");
    fdump_tree_with_freq (stderr, expr, WN_MAP_UNDEFINED);
    fprintf (stderr, "  NBG_State after label:\n");
    NBG_State_print(state, stderr);
    FmtAssert(0, ("CGG_Expand_Expr: No match found for tree\n"));
  }

  link = NBG_link(state);

  if (link == NULL) {
    fprintf (stderr, "!!! ERROR in CGG_Expand_Expr\n");
    fprintf (stderr, "    no link for state:\n");
    NBG_labelled_print(state, stderr);
    FmtAssert(0, ("CGG_Expand_Expr: link phase failed\n"));
  }

  if (trace_cgg) {
    fprintf (TFile, "  NBG_Link after link:\n");
    NBG_linked_print(link, TFile);
  }


  /* Preallocation top down pass. */
  NBG_visit_td(link, CGG_visit_td_prealloc, NULL);

  NBG_emit(link, ops);

  /* If a value was generated, we get it at root. */
  result = (TN *)NBG_Link_otree(link);
  
  NBG_Link_delrec(link);
  if (CGG_opt_level < 2) {
    NBG_del_state_list();
  }
    

  return result;
}

#ifdef __cplusplus
}
#endif
