

#ifndef __NBG_LINK_H__
#define __NBG_LINK_H__

#include "NBG_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NBG_Link_ *NBG_Link;

/*
 * Allocates and construct a new link node.
 * @param rule matching rule for the link node (ref NBG_Rule)
 * @param nkids number of kids for the link node
 * @param kids list of kids for the link or NULL
 *		if NULL and nkids > 0, the kids are allocated but initialized
 *		to 0.
 * @return the allocated link
 */
extern NBG_Link NBG_Link_new(int rule, int nkids, NBG_Link *kids);

/*
 * Destructs and deallocates a link node.
 * @param link the link node
 * @return always NULL
 */
extern NBG_Link NBG_Link_del(NBG_Link link);

/*
 * Destructs and deallocates a link node and all its kids.
 * @param link the link node
 * @return always NULL
 */
extern NBG_Link NBG_Link_delrec(NBG_Link t);

/*
 * Constructs an allocated link node (ref to NBG_Link_new).
 */
extern void NBG_Link_ctor(NBG_Link t, int rule, int nkids, NBG_Link *kids);

/*
 * Destructs a link node (ref to NBG_Link_del).
 */
extern void NBG_Link_dtor(NBG_Link t);

#define NBG_Link_rule(t) ((t)->rule)
#define NBG_Link_utree(t) ((t)->xptr)
#define NBG_Link_otree(t) ((t)->yptr)
#define NBG_Link_nkids(t) ((t)->nkids)
#define NBG_Link_kids(t) ((t)->kids)
#define NBG_Link_kid(t, i) ((t)->kids[i])
#define NBG_Link_parent(t) ((t)->parent)
#define NBG_Link_state(t) ((t)->state)

#include "NBG_Link_imp.h"

#ifdef __cplusplus
}
#endif

#endif
