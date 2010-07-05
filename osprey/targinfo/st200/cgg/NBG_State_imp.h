
#ifndef __NBG_STATE_IMP_H__
#define __NBG_STATE_IMP_H__

#define NBG_MAX_ARITY	16
#define NBG_MAX_NTERMS	128

typedef struct NBG_State_
{
  NBG_Op op;				/* operator id. */
  int arity;				/* arity of the operator (fixed). */
  NBG_State children[NBG_MAX_ARITY];	/* children list. */
  NBG_Rule rule[NBG_MAX_NTERMS];	/* list of matched rules by nterm. */
  NBG_Cost cost[NBG_MAX_NTERMS];	/* costs for matched rules by nterm. */
  NBG_State parent;			/* parent state (except for root or reused states). */
  void *xptr;				/* pointer to user tree. */
  NBG_State reuse;			/* pointer to reuse tree. */
  NBG_State next;			/* pointer to next for linked states. */
} NBG_State_;


/* Returns a child or reuse child for a state. */
static inline NBG_State
NBG_State_get_child(NBG_State s, int i)
{
  NBG_State child = NBG_State_child(s, i);
  return child;
}


#endif
