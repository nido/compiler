
#ifndef __NBG_STATE_H__
#define __NBG_STATE_H__

#include <stdio.h>

#include "NBG_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t NBG_Op;
typedef uint16_t NBG_Cost;
typedef uint16_t NBG_Rule;
typedef uint16_t NBG_NTerm;

#define NBG_COST_MAX ((NBG_Cost)-1)
#define NBG_COST_UNDEF ((NBG_Cost)-1)
#define NBG_RULE_NUL ((NBG_Rule)0)
#define NBG_NTERM_NUL ((NBG_NTerm)0)

/*
 * NBG_State type
 * A NBG_State is a node into the selection tree.
 */
typedef struct NBG_State_ *NBG_State;

/*
 * NBG_State constructors.
 */
extern void NBG_State_ctor(NBG_State state, NBG_Op op, int arity);
extern void NBG_State_dtor(NBG_State state);
extern NBG_State NBG_State_new(NBG_Op op, int arity);
extern NBG_State NBG_State_del(NBG_State state);
extern NBG_State NBG_State_delrec(NBG_State state);
extern NBG_State NBG_State_delnext(NBG_State state);

/* Returns the operator for the state. */
#define NBG_State_op(s) ((s)->op)

/* Returns the arity for the state. */
#define NBG_State_arity(s)	((s)->arity)

/* Returns a child of the state. */
#define NBG_State_children(s)	((s)->children)
#define NBG_State_child(s,i)	((s)->children[i])
#define NBG_State__child(s,i,v)	(((s)->children[i]) = (v))

/* Returns/Defines the parent of a state node. */
#define NBG_State_parent(s)	((s)->parent)
#define NBG_State__parent(s,p)	((s)->parent = (p))

/* Returns/Defines the next of a state node. */
#define NBG_State_next(s)	((s)->parent)
#define NBG_State__next(s,p)	((s)->parent = (p))

/* Returns/Sets the cost for the nterm idx. */
#define NBG_State_cost(s,i)	((s)->cost[i])
#define NBG_State__cost(s,i,c)	((s)->cost[i] = (c))

/* Returns/Sets the rule for the nterm idx. */
#define NBG_State_rule(s,i)	((s)->rule[i])
#define NBG_State__rule(s,i,r)	((s)->rule[i] = (r))

/* Returns/Sets the user tree for the state. */
#define NBG_State_utree(s)	((s)->xptr)
#define NBG_State__utree(s,v)	((s)->xptr = (void *)(v))

/* Returns/Sets the reuse tree for the state. */
#define NBG_State_reuse(s)	((s)->reuse)
#define NBG_State__reuse(s,v)	((s)->reuse = (v))

/* Returns the user tree for the state. */
extern void NBG_State_print(NBG_State state, FILE *file);

#include "NBG_State_imp.h"

#ifdef __cplusplus
}
#endif


#endif
