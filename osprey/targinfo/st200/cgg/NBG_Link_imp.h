
#ifndef __NBG_LINK_IMP_H__
#define __NBG_LINK_IMP_H__

#include "NBG_State.h"

typedef struct NBG_Link_
{
  int rule;				/* rule num. */
  int nkids;				/* arity of the link. */
  struct NBG_Link_ **kids;		/* kids of the link. */
  struct NBG_Link_ *parent;		/* parent of the link. */
  NBG_State state;			/* link to the corrsponding NBG_State node. */
  void *xptr;				/* pointer to user tree. */
  void *yptr;				/* pointer to user output tree. */
} NBG_Link_;

#endif
