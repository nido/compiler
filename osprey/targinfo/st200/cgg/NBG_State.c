#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "NBG_State.h"

#define NBG_State_SIZE(op,arity) sizeof(NBG_State_)


void
NBG_State_ctor(NBG_State state, NBG_Op op, int arity)
{
  int i;
  state->op = op;
  state->arity = arity;
  for (i = 0; i < arity; i++) {
    state->children[i] = NULL;
  }
  for (i = 0; i < NBG_MAX_NTERMS; i++) {
    state->cost[i] = NBG_COST_MAX;
    state->rule[i] = NBG_RULE_NUL;
  }
  state->parent = NULL;
  state->xptr = NULL;
  state->next = NULL;
  state->reuse = NULL;
}
    
void
NBG_State_dtor(NBG_State state)
{
}

NBG_State
NBG_State_new(NBG_Op op, int arity)
{
  NBG_State state = NBG_ALLOC(NBG_State_SIZE(op, arity));
  NBG_State_ctor(state, op, arity);
  return state;
}

NBG_State 
NBG_State_del(NBG_State state)
{
  NBG_State_dtor(state);
  NBG_FREE(state);
  return NULL;
}

NBG_State 
NBG_State_delrec(NBG_State state)
{
  int i;
  for (i = 0; i < state->arity; i++) {
    NBG_State_delrec(state->children[i]);
  }
  NBG_State_dtor(state);
  NBG_FREE(state);
  return NULL;
}

NBG_State 
NBG_State_delnext(NBG_State state)
{
  do {
    NBG_State next = state->next;
    NBG_State_dtor(state);
    NBG_FREE(state);
    state = next;
  } while(state != NULL);
  return NULL;
}

static void
findent(int indent, FILE *file)
{
  int i;
  for (i = 0; i < indent; i++) {
    fprintf(file, " ");
  }
}

static void
NBG_Op_print(NBG_Op op, FILE *file)
{
  fprintf(file, "OP_%d", op);
}

static void
NBG_State_print_indented(NBG_State state, int indent, FILE *file)
{
  int i;

  assert(state != NULL);
  findent(indent, file);
  NBG_Op_print(state->op, file);
  fprintf(file, " [\n");
  for (i = 0; i < NBG_MAX_NTERMS; i++) {
    if (state->rule[i] != NBG_RULE_NUL) {
      findent(indent+1, file);
      fprintf(file, "(rule %d, cost %d)\n", state->rule[i], state->cost[i]);
    }
  }

  for (i = 0; i < state->arity; i++) {
    NBG_State_print_indented(state->children[i], indent+2, file);
  }
  findent(indent, file);
  fprintf(file, "]\n");
  if (state->reuse != NULL) {
    findent(indent+1, file);
    fprintf(file, " REUSE \n");
    NBG_State_print_indented(state->reuse, indent+1, file);
  }
}

void
NBG_State_print(NBG_State state, FILE *file)
{
  NBG_State_print_indented(state, 0, file);
}

