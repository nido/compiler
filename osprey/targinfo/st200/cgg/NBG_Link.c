
#include "NBG_Link.h"

static uint32_t NBG_Link_size(int rule, int nkids, NBG_Link *kids)
{
  return sizeof(NBG_Link_);
}

void NBG_Link_ctor(NBG_Link this, int rule, int nkids, NBG_Link *kids)
{
  this->rule = rule;
  this->nkids = nkids;
  if (nkids > 0) {
    this->kids = (NBG_Link *)NBG_ALLOC(sizeof(NBG_Link)*nkids);
    if (kids == NULL) {
      memset(this->kids, 0, sizeof(NBG_Link)*nkids);
    } else {
      int i;
      for (i = 0; i < nkids; i++) {
	this->kids[i] = kids[i];
      }
    }
  } else {
    this->kids = NULL;
  }
  this->parent = NULL;
  this->xptr = NULL;
  this->yptr = NULL;
  this->state = NULL;
}

void NBG_Link_dtor(NBG_Link this)
{
  if (this->kids != NULL) {
    NBG_FREE(this->kids);
    this->kids = NULL;
  }
}

NBG_Link
NBG_Link_new(int rule, int nkids, NBG_Link *kids)
{
  NBG_Link this = (NBG_Link)NBG_ALLOC(NBG_Link_size(rule, nkids, kids));
  NBG_Link_ctor(this, rule, nkids, kids);
  return this;
}

NBG_Link
NBG_Link_del(NBG_Link this)
{
  NBG_Link_dtor(this);
  NBG_FREE(this);
  return NULL;
}

NBG_Link
NBG_Link_delrec(NBG_Link this)
{
  int i;
  for (i = 0; i < this->nkids; i++) {
    NBG_Link_delrec(this->kids[i]);
  }
  NBG_Link_dtor(this);
  NBG_FREE(this);
  return NULL;
}
