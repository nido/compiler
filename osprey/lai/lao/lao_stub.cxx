#include <stdio.h>
#include <vector.h>

#include "bb.h"
#include "wn.h"
#include "wn_core.h"
#include "wn_util.h"
#include "cg_region.h"
#include "freq.h"
#include "label_util.h"
#include "op.h"
#include "tag.h"
#include "tn_map.h"
#include "gtn_universe.h"
#include "gtn_tn_set.h"
#include "gra.h"
#include "gra_live.h"
#include "cg_loop.h"
#include "cgprep.h"
#include "dominate.h"
#include "findloops.h"
#include "hb.h"

#include "cg_dep_graph.h"

#include "erglob.h"
#include "tracing.h"

#include "cg_flags.h"

#include "lao_init.h"
extern "C" {
#define this THIS
#define operator OPERATOR
#define __STDBOOL_H__
#include "CCL.h"
#include "CSD.h"
#include "LAO_Driver.h"
#include "LAO_Interface.h"
#undef operator
#undef this
}

// Map CGIR TOP to LIR Operator.
static Operator TOP__Operator[TOP_UNDEFINED];

// Map LIR Operator to CGIR TOP.
static TOP Operator__TOP[Operator__COUNT];

// Map CGIR ISA_ENUM_CLASS to LIR Modifier.
static Modifier IEC__Modifier[EC_MAX];

// Map CGIR Literal to LIR Immediate.
static Immediate LC__Immediate[LC_MAX];

// Map CGIR ISA_REGISTER_CLASS to LIR RegClass.
// WARNING! ISA_REGISTER_CLASS reaches ISA_REGISTER_CLASS_MAX
static RegClass IRC__RegClass[ISA_REGISTER_CLASS_MAX+1];

// Map LIR RegClass to CGIR ISA_REGISTER_CLASS.
static ISA_REGISTER_CLASS RegClass__IRC[RegClass__COUNT];

// Variable used to skip multiple lao_init / lao_fini calls.
static int lao_initialized = 0;

extern "C" {
#include <unistd.h>
}

typedef vector<BB*> BB_VECTOR;


/*-------------------- CGIR -> LIR Conversion Fonctions ----------------------*/
// These functions are the only ones to call the Interface_make functions.

// Convert CGIR TOP to LIR Operator.
static inline Operator
CGIR_TOP_to_Operator(TOP top) {
  Operator lao_operator = TOP__Operator[top];
  Is_True(top >= 0 && top < TOP_UNDEFINED, ("TOPcode out of range"));
  Is_True(lao_operator != Operator_, ("Cannot map TOPcode to Operator"));
  return lao_operator;
}

// Convert CGIR ISA_ENUM_CLASS to LIR Modifier.
static inline Modifier
CGIR_IEC_to_Modifier(ISA_ENUM_CLASS iec) {
  Modifier lao_modifier = IEC__Modifier[iec];
  Is_True(iec >= 0 && iec < EC_MAX, ("ISA_ENUM_CLASS out of range"));
  Is_True(lao_modifier != Modifier_, ("Cannot map ISA_ENUM_CLASS to Modifier"));
  return lao_modifier;
}

// Convert CGIR ISA_LIT_CLASS to LIR Immediate.
static inline Immediate
CGIR_LC_to_Immediate(ISA_LIT_CLASS ilc) {
  Immediate lao_immediate = LC__Immediate[ilc];
  Is_True(ilc >= 0 && ilc < LC_MAX, ("ISA_LIT_CLASS out of range"));
  Is_True(lao_immediate != Immediate_, ("Cannot map ISA_LIT_CLASS to Immediate"));
  return lao_immediate;
}

// Convert CGIR ISA_REGISTER_CLASS to LIR RegClass.
static inline RegClass
CGIR_IRC_to_RegClass(ISA_REGISTER_CLASS irc) {
  RegClass lao_regclass = IRC__RegClass[irc];
  Is_True(irc >= 0 && irc <= ISA_REGISTER_CLASS_MAX, ("ISA_REGISTER_CLASS out of range"));
  Is_True(lao_regclass != RegClass_, ("Cannot map ISA_REGISTER_CLASS to RegClass"));
  return lao_regclass;
}

// Convert CGIR CLASS_REG_PAIR to LIR Register.
static inline Register
CGIR_CRP_to_Register(CLASS_REG_PAIR crp) {
  mREGISTER reg = CLASS_REG_PAIR_reg(crp);
  ISA_REGISTER_CLASS irc = CLASS_REG_PAIR_rclass(crp);
  RegClass regclass = CGIR_IRC_to_RegClass(irc);
  Register lowreg = RegClass_lowreg(regclass);
  return (Register)(lowreg + (reg - 1));
}

// Convert CGIR_LAB to LIR Label.
static inline Label
CGIR_LAB_to_Label(CGIR_LAB cgir_lab) {
  Label label = NULL;
  label = Interface_makeLabel(interface, cgir_lab, LABEL_name(cgir_lab));
  return label;
}

// Convert CGIR_SYM to LIR Symbol.
static inline Symbol
CGIR_SYM_to_Symbol(CGIR_SYM cgir_sym) {
  Symbol symbol = NULL;
  if (ST_class(cgir_sym) == CLASS_CONST) {
    char buffer[64];
    sprintf(buffer, "CONST#%llu", (uint64_t)cgir_sym);
    symbol = Interface_makeSymbol(interface, cgir_sym, String_S(buffer));
  } else {
    symbol = Interface_makeSymbol(interface, cgir_sym, ST_name(cgir_sym));
  }
  return symbol;
}

// Convert CGIR_TN to LIR TempName.
static inline TempName
CGIR_TN_to_TempName(CGIR_TN cgir_tn) {
  TempName tempname = NULL;
  if (TN_is_register(cgir_tn)) {
    if (TN_is_dedicated(cgir_tn)) {
      CLASS_REG_PAIR tn_crp = TN_class_reg(cgir_tn);
      tempname = Interface_makeDedicatedTempName(interface, cgir_tn, CGIR_CRP_to_Register(tn_crp));
    } else {
      ISA_REGISTER_CLASS tn_irc = TN_register_class(cgir_tn);
      tempname = Interface_makePseudoRegTempName(interface, cgir_tn, CGIR_IRC_to_RegClass(tn_irc));
    }
  } else if (TN_is_constant(cgir_tn)) {
    if (TN_has_value(cgir_tn)) {
      int64_t value = TN_value(cgir_tn);
      Immediate immediate = CGIR_LC_to_Immediate((ISA_LIT_CLASS)0); // HACK ALERT
      tempname = Interface_makeAbsoluteTempName(interface, cgir_tn, immediate, value);
    } else if (TN_is_symbol(cgir_tn)) {
      Symbol symbol = NULL;
      ST *var = TN_var(cgir_tn);
      ST_IDX st_idx = ST_st_idx(*var);
      int64_t offset = TN_offset(cgir_tn);
      Immediate immediate = CGIR_LC_to_Immediate((ISA_LIT_CLASS)0); // HACK ALERT
      symbol = CGIR_SYM_to_Symbol(st_idx);
      tempname = Interface_makeSymbolTempName(interface, cgir_tn, immediate, symbol, offset);
    } else if (TN_is_label(cgir_tn)) {
      CGIR_LAB cgir_lab = TN_label(cgir_tn);
      Immediate immediate = CGIR_LC_to_Immediate((ISA_LIT_CLASS)0); // HACK ALERT
      Label label = CGIR_LAB_to_Label(cgir_lab);
      tempname = Interface_makeLabelTempName(interface, cgir_tn, immediate, label);
      Is_True(TN_offset(cgir_tn) == 0, ("LAO requires zero offset from label."));
    } else if (TN_is_enum(cgir_tn)) {
      ISA_ENUM_CLASS_VALUE value = TN_enum(cgir_tn);
      Modifier modifier = CGIR_IEC_to_Modifier((ISA_ENUM_CLASS)0);	// HACK ALERT
      tempname = Interface_makeModifierTempName(interface, cgir_tn, modifier, value);
    } else {
      Is_True(FALSE, ("Unknown constant TN type."));
    }
  } else {
    Is_True(FALSE, ("Unknown TN type."));
  }
  Is_True(tempname != NULL, ("tempname should not be NULL."));
  return tempname;
}

// Convert CGIR_OP to LIR Operation.
static Operation
CGIR_OP_to_Operation(CGIR_OP cgir_op) {
  // the Operation arguments
  int argCount = OP_opnds(cgir_op);
  TempName *arguments = (TempName *)(argCount ? alloca(argCount*sizeof(TempName)) : NULL);
  for (int i = 0; i < argCount; i++) Is_True(!Is_CG_LOOP_Op(cgir_op) || (OP_omega(cgir_op, i) <= 1), ("LAO called on TN with omega > 1"));
  for (int i = 0; i < argCount; i++) arguments[i] = CGIR_TN_to_TempName(OP_opnd(cgir_op, i));
  // the Operation results
  int resCount = OP_results(cgir_op);
  TempName *results = (TempName *)(resCount ? alloca(resCount*sizeof(TempName)) : NULL);
  for (int i = 0; i < resCount; i++) results[i] = CGIR_TN_to_TempName(OP_result(cgir_op, i));
  // make the Operation
  Operator OPERATOR = CGIR_TOP_to_Operator(OP_code(cgir_op));
  Operation operation = Interface_makeOperation(interface, cgir_op,
      OPERATOR, argCount, arguments, resCount, results);
  if (OP_volatile(cgir_op)) Interface_Operation_setVolatile(interface, operation);
  return operation;
}

// Convert CGIR_BB to LIR BasicBlock.
static BasicBlock
CGIR_BB_to_BasicBlock(CGIR_BB cgir_bb) {
  // the BasicBlock label(s)
  int labelCount = 0, MAX_LABEL_COUNT = 256;
  Label *labels = (Label *)alloca(MAX_LABEL_COUNT*sizeof(Label));
  if (BB_has_label(cgir_bb)) {
    ANNOTATION *annot;
    for (annot = ANNOT_First(BB_annotations(cgir_bb), ANNOT_LABEL);
	 annot != NULL;
	 annot = ANNOT_Next(annot, ANNOT_LABEL)) {
      Is_True(labelCount < MAX_LABEL_COUNT, ("BB has more than MAX_LABEL_COUNT labels"));
      CGIR_LAB cgir_lab = ANNOT_label(annot);
      labels[labelCount++] = CGIR_LAB_to_Label(cgir_lab);
    }
  }
  // the BasicBlock operations
  int operationCount = 0, MAX_OPERATION_COUNT = 16384;
  Operation *operations = (Operation *)alloca(MAX_OPERATION_COUNT*sizeof(Operation));
  CGIR_OP cgir_op = NULL;
  FOR_ALL_BB_OPs(cgir_bb, cgir_op) {
    Is_True(operationCount < MAX_OPERATION_COUNT, ("BB has more than MAX_OPERATION_COUNT operations"));
    operations[operationCount++] = CGIR_OP_to_Operation(cgir_op);
  }
  // make the BasicBlock
  BasicBlock basicblock = Interface_makeBasicBlock(interface, cgir_bb,
      labelCount, labels, operationCount, operations);
  return basicblock;
}

// Convert CGIR_BB to LIR ControlNode.
static ControlNode
CGIR_BB_to_ControlNode(CGIR_BB cgir_bb) {
  float frequency = BB_freq(cgir_bb);
  BasicBlock basicblock = Interface_makeBasicBlock(interface, cgir_bb, 0, NULL, 0, NULL);
  int liveinCount = 0, MAX_LIVEIN_COUNT = 16384;
  TempName *liveins = (TempName *)alloca(MAX_LIVEIN_COUNT*sizeof(TempName));
  for (TN *tn = GTN_SET_Choose(BB_live_in(cgir_bb));
       tn != GTN_SET_CHOOSE_FAILURE;
       tn = GTN_SET_Choose_Next(BB_live_in(cgir_bb), tn)) {
    Is_True(liveinCount < MAX_LIVEIN_COUNT, ("BB has more than MAX_LIVEIN_COUNT liveins"));
    liveins[liveinCount++] = CGIR_TN_to_TempName(tn);
  }
  ControlNode controlnode = Interface_makeControlNode(interface, basicblock, frequency, liveinCount, liveins);
  return controlnode;
}


/*-------------------- LIR -> CGIR Conversion Fonctions ----------------------*/

// Convert LIR RegClass to CGIR ISA_REGISTER_CLASS.
static inline ISA_REGISTER_CLASS
RegClass_to_CGIR_IRC(RegClass regclass) {
  Is_True(regclass > RegClass_ && regclass < RegClass__COUNT, ("RegClass out of range"));
  ISA_REGISTER_CLASS irc = RegClass__IRC[regclass];
  Is_True(IRC__RegClass[irc] == regclass, ("Unexpected error in RegClass_to_CGIR_IRC"));
  Is_True(irc != ISA_REGISTER_CLASS_UNDEFINED, ("Cannot map RegClass to ISA_REGISTER_CLASS"));
  return irc;
}

// Convert LIR Register to CGIR CLASS_REG_PAIR.
static inline CLASS_REG_PAIR
Register_to_CGIR_CRP(Register registre) {
  RegClass regclass = Register_regclass(registre);
  Register lowreg = RegClass_lowreg(regclass);
  ISA_REGISTER_CLASS irc = RegClass_to_CGIR_IRC(regclass);
  REGISTER reg = (registre - lowreg) + 1;
  CLASS_REG_PAIR crp;
  Set_CLASS_REG_PAIR(crp, irc, reg);
  return crp;
}

// Convert LIR Operator to TOP.
static inline TOP
Operator_to_CGIR_TOP(Operator lir_operator) {
  Is_True(lir_operator > Operator_ && lir_operator < Operator__COUNT, ("Operator out of range"));
  TOP top = Operator__TOP[lir_operator];
  Is_True(TOP__Operator[top] == lir_operator, ("Unexpected error in Operator_to_CGIR_TOP"));
  Is_True(top != TOP_UNDEFINED, ("Cannot map Operator to TOP"));
  return top;
}


/*-------------------- LIR Interface Call-Back Functions ---------------------*/

// Identity of a CGIR_LAB.
static uint32_t
CGIR_LAB_identity(CGIR_LAB cgir_lab) {
  return cgir_lab;
}

// Create a CGIR_LAB.
static CGIR_LAB
CGIR_LAB_create(CGIR_LAB cgir_lab, const char *name) {
  CGIR_LAB new_lab = 0;
  // code borrowed from Gen_Label_For_BB
  LABEL *plabel = &New_LABEL(CURRENT_SYMTAB, new_lab);
  LABEL_Init(*plabel, Save_Str(name), LKIND_DEFAULT);
  //
  return new_lab;
}

// Update a CGIR_LAB.
static void
CGIR_LAB_update(CGIR_LAB cgir_lab, const char *name) {
}

// Identity of a CGIR_SYM.
static uint32_t
CGIR_SYM_identity(CGIR_SYM cgir_sym) {
  return cgir_sym;
}

// Create a CGIR_SYM.
static CGIR_SYM
CGIR_SYM_create(CGIR_SYM cgir_sym) {
}

// Update a CGIR_SYM.
static void
CGIR_SYM_update(CGIR_SYM cgir_sym) {
}

// Identity of a CGIR_TN.
static uint32_t
CGIR_TN_identity(CGIR_TN cgir_tn) {
  return (uintptr_t)cgir_tn/sizeof(*cgir_tn);
}

// Create a Dedicated CGIR_TN.
static CGIR_TN
CGIR_Dedicated_TN_create(CGIR_TN cgir_tn, Register registre) {
  int size = 0;		// not used in Build_Dedicated_TN
  CLASS_REG_PAIR crp = Register_to_CGIR_CRP(registre);
  return Build_Dedicated_TN(CLASS_REG_PAIR_rclass(crp), CLASS_REG_PAIR_reg(crp), size);
}

// Create a PseudoReg CGIR_TN.
static CGIR_TN
CGIR_PseudoReg_TN_create(CGIR_TN cgir_tn, RegClass regclass) {
  int size = (RegClass_bitwidth(regclass) + 7)/8;
  return Gen_Register_TN(RegClass_to_CGIR_IRC(regclass), size);
}

// Create a Modifier CGIR_TN.
static CGIR_TN
CGIR_Modifier_TN_create(CGIR_TN cgir_tn, Modifier modifier) {
  Is_True(0, ("CGIR_Modifier_TN_create not implemented"));
  return NULL;
}

// Create an Absolute CGIR_TN.
static CGIR_TN
CGIR_Absolute_TN_create(CGIR_TN cgir_tn, Immediate immediate, int64_t value) {
  int size = (value >= (int64_t)0x80000000 && value <= (int64_t)0x7FFFFFFF) ? 4 : 8;
  return Gen_Literal_TN(value, size);
}

// Create a Symbol CGIR_TN.
static CGIR_TN
CGIR_Symbol_TN_create(CGIR_TN cgir_tn, Immediate immediate, CGIR_SYM cgir_sym, int64_t offset) {
  Is_True(0, ("CGIR_Symbol_TN_create not implemented"));
  return NULL;
}

// Create a Label CGIR_TN.
static CGIR_TN
CGIR_Label_TN_create(CGIR_TN cgir_tn, Immediate immediate, CGIR_LAB cgir_lab) {
  return Gen_Label_TN(cgir_lab, 0);
}

// Update a CGIR_TN.
static void
CGIR_TN_update(CGIR_TN cgir_tn) {
  // TODO: commit register allocation.
}

// Identity of a CGIR_OP.
static uint32_t
CGIR_OP_identity(CGIR_OP cgir_op) {
  return (uintptr_t)cgir_op/OP_sizeof(0, 0);
}

// Create a CGIR_OP.
static CGIR_OP
CGIR_OP_create(CGIR_OP cgir_op, Operator OPERATOR, int argCount, CGIR_TN arguments[], int resCount, CGIR_TN results[], int unrolled, int iteration, int issueDate) {
  TOP top = Operator_to_CGIR_TOP(OPERATOR);
  CGIR_OP new_op = Mk_VarOP(top, resCount, argCount, results, arguments);
  CGPREP_Init_Op(new_op);
  // _CG_LOOP_info_map may not be defined for multi-bb loops.
  if (Is_CG_LOOP_Op(cgir_op)) CG_LOOP_Init_Op(new_op);
  // If a duplicate, set orig_idx and copy WN.
  if (cgir_op != NULL) {
    Set_OP_orig_idx(new_op, OP_map_idx(cgir_op));
    Copy_WN_For_Memory_OP(new_op, cgir_op);
    // Set unrolling.
    Set_OP_unrolling(new_op, OP_unrolling(cgir_op) + unrolled * iteration);
  }
  // Set scycle
  OP_scycle(new_op) = (issueDate == -1) ? 0 : issueDate;
  //
  return new_op;
}

// Update a CGIR_OP.
static void
CGIR_OP_update(CGIR_OP cgir_op, Operator OPERATOR, int argCount, CGIR_TN arguments[], int resCount, CGIR_TN results[], int unrolled, int iteration, int issueDate) {
  TOP top = Operator_to_CGIR_TOP(OPERATOR);
  if (OP_code(cgir_op) != top) {
    OP_Change_Opcode(cgir_op, top);
  }
  Is_True(argCount == OP_opnds(cgir_op), ("OP_opnds mismatch in CGIR_update_OP"));
  for (int i = 0; i < argCount; i++) {
    CGIR_TN cgir_tn = arguments[i];
    if (OP_opnd(cgir_op, i) != cgir_tn) Set_OP_opnd(cgir_op, i, cgir_tn);
  }
  Is_True(resCount == OP_results(cgir_op), ("OP_results mismatch in CGIR_update_OP"));
  for (int i = 0; i < resCount; i++) {
    CGIR_TN cgir_tn = results[i];
    if (OP_result(cgir_op, i) != cgir_tn) Set_OP_result(cgir_op, i, cgir_tn);
  }
  Is_True(iteration == 0, ("CGIR_OP_update called with iteration > 0"));
  // Set scycle.
  OP_scycle(cgir_op) = (issueDate == -1) ? 0 : issueDate;
}

// Identity of a CGIR_BB.
static uint32_t
CGIR_BB_identity(CGIR_BB cgir_bb) {
  return (uintptr_t)cgir_bb/sizeof(*cgir_bb);
}

// Create a CGIR_BB.
static CGIR_BB
CGIR_BB_create(CGIR_BB cgir_bb, int labelCount, CGIR_LAB labels[], int opCount, CGIR_OP operations[], CGIR_LI cgir_li, int unrolled, unsigned optimizations) {
  CGIR_BB new_bb = Gen_BB();
  // Add the labels.
  for (int i = 0; i < labelCount; i++) {
    CGIR_LAB cgir_lab = labels[i];
    // code borrowed from Gen_Label_For_BB
    Set_Label_BB(cgir_lab, new_bb);
    BB_Add_Annotation(new_bb, ANNOT_LABEL, (void *)cgir_lab);
  }
  // Add the operations.
  OPS ops = OPS_EMPTY;
  for (int i = 0; i < opCount; i++) {
    OPS_Append_Op(&ops, operations[i]);
    if (OP_unrolling(operations[i]) != 0)
      Set_OP_unroll_bb(operations[i], new_bb);
  }
  BB_Append_Ops(new_bb, &ops);
  // Add the cgir_li.
  if (cgir_li != NULL) {
    BB_Add_Annotation(new_bb, ANNOT_LOOPINFO, cgir_li);
  }
  // Transfer annotations.
  if (cgir_bb != NULL) {
    if (BB_has_pragma(cgir_bb)) {
      BB_Copy_Annotations(new_bb, cgir_bb, ANNOT_PRAGMA);
    }
    Is_True(!BB_entry(cgir_bb), ("Cannot update a CGIR BB with ENTRY property"));
    Is_True(!BB_exit(cgir_bb), ("Cannot update a CGIR BB with EXIT property"));
    if (BB_call(cgir_bb)) {
      BB_Copy_Annotations(new_bb, cgir_bb, ANNOT_CALLINFO);
    }
    if (BB_has_note(cgir_bb)) {
      BB_Copy_Annotations(new_bb, cgir_bb, ANNOT_NOTE);
    }
  }
  // Set unrollings.
  if (BB_unrollings(new_bb) == 0)
    Set_BB_unrollings(new_bb, unrolled);
  else
    Set_BB_unrollings(new_bb, BB_unrollings(new_bb)*unrolled);
  //
  // Set flags.
  if (optimizations & Optimization_Prepass) Set_BB_scheduled(new_bb);
  if (optimizations & Optimization_RegAlloc) Set_BB_reg_alloc(new_bb);
  if (optimizations & Optimization_Postpass) Set_BB_scheduled(new_bb);
  //
  return new_bb;
}

// Update a CGIR_BB.
static void
CGIR_BB_update(CGIR_BB cgir_bb, int labelCount, CGIR_LAB labels[], int opCount, CGIR_OP operations[], CGIR_LI cgir_li, int unrolled, unsigned optimizations) {
  // Add the labels.
  for (int i = 0; i < labelCount; i++) {
    CGIR_LAB cgir_lab = labels[i];
    if (!Is_Label_For_BB(cgir_lab, cgir_bb)) {
      // code borrowed from Gen_Label_For_BB
      Set_Label_BB(cgir_lab, cgir_bb);
      BB_Add_Annotation(cgir_bb, ANNOT_LABEL, (void *)cgir_lab);
    }
  }
  // Add the operations.
  BB_Remove_All(cgir_bb);
  OPS ops = OPS_EMPTY;
  for (int i = 0; i < opCount; i++) {
    OPS_Append_Op(&ops, operations[i]);
    if (OP_unrolling(operations[i]) != 0)
      Is_True(OP_unroll_bb(operations[i]), ("Unrolled operation with NULL unroll_bb"));
  }
  BB_Append_Ops(cgir_bb, &ops);
  // Remove the cgir_li if any.
  ANNOTATION *annot = ANNOT_Get(BB_annotations(cgir_bb), ANNOT_LOOPINFO);
  if (annot != NULL) {
    BB_annotations(cgir_bb) = ANNOT_Unlink(BB_annotations(cgir_bb), annot);
  }
  // Add the cgir_li, unless it is a dummy created by CGIR_LI_create.
  if (cgir_li != NULL && LOOPINFO_wn(cgir_li) != NULL) {
    BB_Add_Annotation(cgir_bb, ANNOT_LOOPINFO, cgir_li);
  }
  // Set unrollings.
  if (BB_unrollings(cgir_bb) == 0)
    Set_BB_unrollings(cgir_bb, unrolled);
  else
    Set_BB_unrollings(cgir_bb, BB_unrollings(cgir_bb)*unrolled);
  Set_BB_unrollings(cgir_bb, unrolled);
  // Set flags.
  if (optimizations & Optimization_Prepass) Set_BB_scheduled(cgir_bb);
  if (optimizations & Optimization_RegAlloc) Set_BB_reg_alloc(cgir_bb);
  if (optimizations & Optimization_Postpass) Set_BB_scheduled(cgir_bb);
}

// Chain two CGIR_BBs in the CGIR.
static void
CGIR_BB_chain(CGIR_BB cgir_bb, CGIR_BB succ_cgir_bb) {
  Insert_BB(succ_cgir_bb, cgir_bb);
}

// Unchain a CGIR_BB in the CGIR.
static void
CGIR_BB_unchain(CGIR_BB cgir_bb) {
  Remove_BB(cgir_bb);
}

// Link two CGIR_BBs in the CGIR with the given branch probability.
static void
CGIR_BB_link(CGIR_BB orig_cgir_bb, CGIR_BB dest_cgir_bb, float probability) {
  Link_Pred_Succ_with_Prob(orig_cgir_bb, dest_cgir_bb, probability);
}

// Unlink all the predecessors and successors of a CGIR_BB in the CGIR.
static void
CGIR_BB_unlink(CGIR_BB cgir_bb) {
  BBLIST *edge;
  //
  // Remove successor edges.
  FOR_ALL_BB_SUCCS(cgir_bb, edge) {
    BB *succ = BBLIST_item(edge);
    BBlist_Delete_BB(&BB_preds(succ), cgir_bb);
  }
  BBlist_Free(&BB_succs(cgir_bb));
  //
  // Remove predecessor edges.
  FOR_ALL_BB_PREDS(cgir_bb, edge) {
    BB *pred = BBLIST_item(edge);
    BBlist_Delete_BB(&BB_succs(pred), cgir_bb);
  }
  BBlist_Free(&BB_preds(cgir_bb));
}

// Link two nodes that were linked through a fall-thru edge.
static void
CGIR_BB_linkFallThru(CGIR_BB cgir_bb, CGIR_BB succ_cgir_bb, float probability) {
  CGIR_BB bb_next = BB_next(cgir_bb);
  if (bb_next != succ_cgir_bb) {
    CGIR_BB goto_bb = Gen_And_Insert_BB_After(cgir_bb);
    Unlink_Pred_Succ(cgir_bb, succ_cgir_bb);
    Link_Pred_Succ_with_Prob(cgir_bb, goto_bb, probability);
    Add_Goto(goto_bb, succ_cgir_bb);
    Change_Succ_Prob(goto_bb, succ_cgir_bb, probability);
    fprintf(TFile, "Connecting to fall-through node requires a new node %d\n", BB_id(goto_bb));
  }
}

// Identity of a CGIR_LI.
static uint32_t
CGIR_LI_identity(CGIR_LI cgir_li) {
  return (uintptr_t)cgir_li/sizeof(*cgir_li);
}

// Create a CGIR_LI.
static CGIR_LI
CGIR_LI_create(CGIR_LI cgir_li, int unrolled) {
  CGIR_LI new_li = TYPE_P_ALLOC(LOOPINFO);
  if (cgir_li != NULL && LOOPINFO_wn(cgir_li) != NULL) {
    // Code adapted from Unroll_Dowhile_Loop.
    WN *wn = WN_COPY_Tree(LOOPINFO_wn(cgir_li));
    if (unrolled != 0) {
      WN_loop_trip_est(wn) /= unrolled;
      WN_loop_trip_est(wn) += 1;
    }
    LOOPINFO_wn(new_li) = wn;
    LOOPINFO_srcpos(new_li) = LOOPINFO_srcpos(cgir_li);
  } else {
    // Create a dummy CGIR_LI.
    LOOPINFO_wn(new_li) = NULL;
  }
  //
  return new_li;
}

// Update a CGIR_LI.
static void
CGIR_LI_update(CGIR_LI cgir_li, int unrolled) {
}

/*--------------------------- lao_init / lao_fini ----------------------------*/

// Optimize a LOOP_DESCR through the LAO.
static bool lao_optimize_LOOP(LOOP_DESCR *loop, unsigned lao_optimizations);

// Optimize a HB through the LAO.
static bool lao_optimize_HB(HB *hb, unsigned lao_optimizations);

// Optimize a PU through the LAO.
static bool lao_optimize_PU(unsigned lao_optimizations);

static void CGIR_print(void);

CG_EXPORTED extern bool (*lao_optimize_LOOP_p)(LOOP_DESCR *loop, unsigned lao_optimizations);
CG_EXPORTED extern bool (*lao_optimize_HB_p)(HB *hb, unsigned lao_optimizations);
CG_EXPORTED extern bool (*lao_optimize_PU_p)(unsigned lao_optimizations);
CG_EXPORTED extern void (*CGIR_print_p)(void);

// Initialization of the LAO, needs to be called once.
void
lao_init() {
  if (getenv("PID")) {
    int dummy; fprintf(stderr, "PID=%lld\n", (int64_t)getpid()); scanf("%d", &dummy);
  }
  if (lao_initialized++ == 0) {
    // initialize the PRO64/LAO interface pointers
    lao_optimize_LOOP_p = lao_optimize_LOOP;
    lao_optimize_HB_p = lao_optimize_HB;
    lao_optimize_PU_p = lao_optimize_PU;
    CGIR_print_p = CGIR_print;
    // initialize the TOP__Operator array
    for (int i = 0; i < TOP_UNDEFINED; i++) TOP__Operator[i] = Operator_;
    TOP__Operator[TOP_add_i] = Operator_CODE_ADD_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_add_ii] = Operator_CODE_ADD_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_add_r] = Operator_CODE_ADD_DEST_SRC1_SRC2;
    TOP__Operator[TOP_addcg] = Operator_CODE_ADDCG_DEST_BDEST_SRC1_SRC2_SCOND;
    TOP__Operator[TOP_and_i] = Operator_CODE_AND_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_and_ii] = Operator_CODE_AND_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_and_r] = Operator_CODE_AND_DEST_SRC1_SRC2;
    TOP__Operator[TOP_andc_i] = Operator_CODE_ANDC_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_andc_ii] = Operator_CODE_ANDC_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_andc_r] = Operator_CODE_ANDC_DEST_SRC1_SRC2;
    TOP__Operator[TOP_andl_i_b] = Operator_CODE_ANDL_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_andl_ii_b] = Operator_CODE_ANDL_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_andl_i_r] = Operator_CODE_ANDL_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_andl_ii_r] = Operator_CODE_ANDL_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_andl_r_b] = Operator_CODE_ANDL_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_andl_r_r] = Operator_CODE_ANDL_DEST_SRC1_SRC2;
    TOP__Operator[TOP_asm] = Operator_CODE_ASM15_DEST_SRC1_SRC2;
    TOP__Operator[TOP_begin_pregtn] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_br] = Operator_CODE_BR_BCOND_BTARG;
    TOP__Operator[TOP_break] = Operator_CODE_BREAK;
    TOP__Operator[TOP_brf] = Operator_CODE_BRF_BCOND_BTARG;
    TOP__Operator[TOP_bswap_r] = Operator_CODE_BSWAP_IDEST_SRC1;
    TOP__Operator[TOP_bwd_bar] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_call] = Operator_CODE_CALL_BTARG;
    TOP__Operator[TOP_clz] = Operator_CODE_CLZ_IDEST_SRC1;
    TOP__Operator[TOP_cmpeq_i_b] = Operator_CODE_CMPEQ_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpeq_ii_b] = Operator_CODE_CMPEQ_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpeq_i_r] = Operator_CODE_CMPEQ_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpeq_ii_r] = Operator_CODE_CMPEQ_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpeq_r_b] = Operator_CODE_CMPEQ_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpeq_r_r] = Operator_CODE_CMPEQ_DEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpge_i_b] = Operator_CODE_CMPGE_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpge_ii_b] = Operator_CODE_CMPGE_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpge_i_r] = Operator_CODE_CMPGE_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpge_ii_r] = Operator_CODE_CMPGE_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpge_r_b] = Operator_CODE_CMPGE_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpge_r_r] = Operator_CODE_CMPGE_DEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpgeu_i_b] = Operator_CODE_CMPGEU_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpgeu_ii_b] = Operator_CODE_CMPGEU_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpgeu_i_r] = Operator_CODE_CMPGEU_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpgeu_ii_r] = Operator_CODE_CMPGEU_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpgeu_r_b] = Operator_CODE_CMPGEU_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpgeu_r_r] = Operator_CODE_CMPGEU_DEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpgt_i_b] = Operator_CODE_CMPGT_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpgt_ii_b] = Operator_CODE_CMPGT_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpgt_i_r] = Operator_CODE_CMPGT_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpgt_ii_r] = Operator_CODE_CMPGT_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpgt_r_b] = Operator_CODE_CMPGT_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpgt_r_r] = Operator_CODE_CMPGT_DEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpgtu_i_b] = Operator_CODE_CMPGTU_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpgtu_ii_b] = Operator_CODE_CMPGTU_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpgtu_i_r] = Operator_CODE_CMPGTU_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpgtu_ii_r] = Operator_CODE_CMPGTU_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpgtu_r_b] = Operator_CODE_CMPGTU_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpgtu_r_r] = Operator_CODE_CMPGTU_DEST_SRC1_SRC2;
    TOP__Operator[TOP_cmple_i_b] = Operator_CODE_CMPLE_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmple_ii_b] = Operator_CODE_CMPLE_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmple_i_r] = Operator_CODE_CMPLE_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmple_ii_r] = Operator_CODE_CMPLE_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmple_r_b] = Operator_CODE_CMPLE_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmple_r_r] = Operator_CODE_CMPLE_DEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpleu_i_b] = Operator_CODE_CMPLEU_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpleu_ii_b] = Operator_CODE_CMPLEU_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpleu_i_r] = Operator_CODE_CMPLEU_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpleu_ii_r] = Operator_CODE_CMPLEU_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpleu_r_b] = Operator_CODE_CMPLEU_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpleu_r_r] = Operator_CODE_CMPLEU_DEST_SRC1_SRC2;
    TOP__Operator[TOP_cmplt_i_b] = Operator_CODE_CMPLT_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmplt_ii_b] = Operator_CODE_CMPLT_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmplt_i_r] = Operator_CODE_CMPLT_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmplt_ii_r] = Operator_CODE_CMPLT_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmplt_r_b] = Operator_CODE_CMPLT_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmplt_r_r] = Operator_CODE_CMPLT_DEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpltu_i_b] = Operator_CODE_CMPLTU_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpltu_ii_b] = Operator_CODE_CMPLTU_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpltu_i_r] = Operator_CODE_CMPLTU_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpltu_ii_r] = Operator_CODE_CMPLTU_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpltu_r_b] = Operator_CODE_CMPLTU_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpltu_r_r] = Operator_CODE_CMPLTU_DEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpne_i_b] = Operator_CODE_CMPNE_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpne_ii_b] = Operator_CODE_CMPNE_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpne_i_r] = Operator_CODE_CMPNE_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_cmpne_ii_r] = Operator_CODE_CMPNE_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_cmpne_r_b] = Operator_CODE_CMPNE_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_cmpne_r_r] = Operator_CODE_CMPNE_DEST_SRC1_SRC2;
    TOP__Operator[TOP_copy_br] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_dfixup] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_divs] = Operator_CODE_DIVS_DEST_BDEST_SRC1_SRC2_SCOND;
    TOP__Operator[TOP_end_pregtn] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_ffixup] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_fwd_bar] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_goto] = Operator_CODE_GOTO_BTARG;
    TOP__Operator[TOP_icall] = Operator_CODE_ICALL;
    TOP__Operator[TOP_ifixup] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_igoto] = Operator_CODE_IGOTO;
    TOP__Operator[TOP_imml] = Operator_CODE_IMML_IMM;
    TOP__Operator[TOP_immr] = Operator_CODE_IMMR_IMM;
    TOP__Operator[TOP_intrncall] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_label] = Operator_PSEUDO_LABEL;
    TOP__Operator[TOP_ldb_d_i] = Operator_CODE_LDBD_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldb_d_ii] = Operator_CODE_LDBD_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_ldb_i] = Operator_CODE_LDB_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldb_ii] = Operator_CODE_LDB_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_ldbu_d_i] = Operator_CODE_LDBUD_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldbu_d_ii] = Operator_CODE_LDBUD_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_ldbu_i] = Operator_CODE_LDBU_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldbu_ii] = Operator_CODE_LDBU_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_ldh_d_i] = Operator_CODE_LDHD_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldh_d_ii] = Operator_CODE_LDHD_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_ldh_i] = Operator_CODE_LDH_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldh_ii] = Operator_CODE_LDH_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_ldhu_d_i] = Operator_CODE_LDHUD_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldhu_d_ii] = Operator_CODE_LDHUD_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_ldhu_i] = Operator_CODE_LDHU_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldhu_ii] = Operator_CODE_LDHU_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_ldw_d_i] = Operator_CODE_LDWD_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldw_d_ii] = Operator_CODE_LDWD_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_ldw_i] = Operator_CODE_LDW_IDESTL_ISRC2_SRC1;
    TOP__Operator[TOP_ldw_ii] = Operator_CODE_LDW_IDESTL_ISRCX_SRC1;
    TOP__Operator[TOP_max_i] = Operator_CODE_MAX_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_max_ii] = Operator_CODE_MAX_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_max_r] = Operator_CODE_MAX_DEST_SRC1_SRC2;
    TOP__Operator[TOP_maxu_i] = Operator_CODE_MAXU_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_maxu_ii] = Operator_CODE_MAXU_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_maxu_r] = Operator_CODE_MAXU_DEST_SRC1_SRC2;
    TOP__Operator[TOP_mfb] = Operator_CODE_MFB_IDEST_SCOND;
    TOP__Operator[TOP_min_i] = Operator_CODE_MIN_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_min_ii] = Operator_CODE_MIN_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_min_r] = Operator_CODE_MIN_DEST_SRC1_SRC2;
    TOP__Operator[TOP_minu_i] = Operator_CODE_MINU_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_minu_ii] = Operator_CODE_MINU_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_minu_r] = Operator_CODE_MINU_DEST_SRC1_SRC2;
    TOP__Operator[TOP_mov_i] = Operator_CODE_MOV_IDEST_ISRC2;
    TOP__Operator[TOP_mov_ii] = Operator_CODE_MOV_IDEST_ISRCX;
    TOP__Operator[TOP_mov_r] = Operator_CODE_MOV_DEST_SRC2;
    TOP__Operator[TOP_mtb] = Operator_CODE_MTB_BDEST_SRC1;
    TOP__Operator[TOP_mulh_i] = Operator_CODE_MULH_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mulh_ii] = Operator_CODE_MULH_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mulh_r] = Operator_CODE_MULH_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mulhh_i] = Operator_CODE_MULHH_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mulhh_ii] = Operator_CODE_MULHH_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mulhh_r] = Operator_CODE_MULHH_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mulhhs_i] = Operator_CODE_MULHHS_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mulhhs_ii] = Operator_CODE_MULHHS_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mulhhs_r] = Operator_CODE_MULHHS_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mulhhu_i] = Operator_CODE_MULHHU_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mulhhu_ii] = Operator_CODE_MULHHU_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mulhhu_r] = Operator_CODE_MULHHU_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mulhs_i] = Operator_CODE_MULHS_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mulhs_ii] = Operator_CODE_MULHS_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mulhs_r] = Operator_CODE_MULHS_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mulhu_i] = Operator_CODE_MULHU_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mulhu_ii] = Operator_CODE_MULHU_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mulhu_r] = Operator_CODE_MULHU_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mull_i] = Operator_CODE_MULL_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mull_ii] = Operator_CODE_MULL_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mull_r] = Operator_CODE_MULL_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mullh_i] = Operator_CODE_MULLH_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mullh_ii] = Operator_CODE_MULLH_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mullh_r] = Operator_CODE_MULLH_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mullhu_i] = Operator_CODE_MULLHU_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mullhu_ii] = Operator_CODE_MULLHU_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mullhu_r] = Operator_CODE_MULLHU_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mullhus_i] = Operator_CODE_MULLHUS_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mullhus_ii] = Operator_CODE_MULLHUS_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mullhus_r] = Operator_CODE_MULLHUS_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mulll_i] = Operator_CODE_MULLL_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mulll_ii] = Operator_CODE_MULLL_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mulll_r] = Operator_CODE_MULLL_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mulllu_i] = Operator_CODE_MULLLU_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mulllu_ii] = Operator_CODE_MULLLU_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mulllu_r] = Operator_CODE_MULLLU_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_mullu_i] = Operator_CODE_MULLU_IDESTL_SRC1_ISRC2;
    TOP__Operator[TOP_mullu_ii] = Operator_CODE_MULLU_IDESTL_SRC1_ISRCX;
    TOP__Operator[TOP_mullu_r] = Operator_CODE_MULLU_DESTL_SRC1_SRC2;
    TOP__Operator[TOP_nandl_i_b] = Operator_CODE_NANDL_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_nandl_ii_b] = Operator_CODE_NANDL_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_nandl_i_r] = Operator_CODE_NANDL_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_nandl_ii_r] = Operator_CODE_NANDL_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_nandl_r_b] = Operator_CODE_NANDL_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_nandl_r_r] = Operator_CODE_NANDL_DEST_SRC1_SRC2;
    TOP__Operator[TOP_noop] = Operator_PSEUDO_NOP;
    TOP__Operator[TOP_nop] = Operator_CODE_NOP;
    TOP__Operator[TOP_norl_i_b] = Operator_CODE_NORL_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_norl_ii_b] = Operator_CODE_NORL_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_norl_i_r] = Operator_CODE_NORL_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_norl_ii_r] = Operator_CODE_NORL_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_norl_r_b] = Operator_CODE_NORL_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_norl_r_r] = Operator_CODE_NORL_DEST_SRC1_SRC2;
    TOP__Operator[TOP_or_i] = Operator_CODE_OR_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_or_ii] = Operator_CODE_OR_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_or_r] = Operator_CODE_OR_DEST_SRC1_SRC2;
    TOP__Operator[TOP_orc_i] = Operator_CODE_ORC_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_orc_ii] = Operator_CODE_ORC_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_orc_r] = Operator_CODE_ORC_DEST_SRC1_SRC2;
    TOP__Operator[TOP_orl_i_b] = Operator_CODE_ORL_IBDEST_SRC1_ISRC2;
    TOP__Operator[TOP_orl_ii_b] = Operator_CODE_ORL_IBDEST_SRC1_ISRCX;
    TOP__Operator[TOP_orl_i_r] = Operator_CODE_ORL_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_orl_ii_r] = Operator_CODE_ORL_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_orl_r_b] = Operator_CODE_ORL_BDEST_SRC1_SRC2;
    TOP__Operator[TOP_orl_r_r] = Operator_CODE_ORL_DEST_SRC1_SRC2;
    TOP__Operator[TOP_pft] = Operator_CODE_PFT_ISRC2_SRC1;
    TOP__Operator[TOP_phi] = Operator_PSEUDO_PHI;
    TOP__Operator[TOP_prgadd] = Operator_CODE_PRGADD_ISRC2_SRC1;
    TOP__Operator[TOP_prgins] = Operator_CODE_PRGINS;
    TOP__Operator[TOP_prgset] = Operator_CODE_PRGSET_ISRC2_SRC1;
    TOP__Operator[TOP_psi] = Operator_PSEUDO_PSI;
    TOP__Operator[TOP_return] = Operator_MACRO_RETURN;
    TOP__Operator[TOP_rfi] = Operator_CODE_RFI;
    TOP__Operator[TOP_sbrk] = Operator_CODE_SBRK;
    TOP__Operator[TOP_sh1add_i] = Operator_CODE_SH1ADD_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_sh1add_ii] = Operator_CODE_SH1ADD_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_sh1add_r] = Operator_CODE_SH1ADD_DEST_SRC1_SRC2;
    TOP__Operator[TOP_sh2add_i] = Operator_CODE_SH2ADD_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_sh2add_ii] = Operator_CODE_SH2ADD_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_sh2add_r] = Operator_CODE_SH2ADD_DEST_SRC1_SRC2;
    TOP__Operator[TOP_sh3add_i] = Operator_CODE_SH3ADD_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_sh3add_ii] = Operator_CODE_SH3ADD_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_sh3add_r] = Operator_CODE_SH3ADD_DEST_SRC1_SRC2;
    TOP__Operator[TOP_sh4add_i] = Operator_CODE_SH4ADD_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_sh4add_ii] = Operator_CODE_SH4ADD_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_sh4add_r] = Operator_CODE_SH4ADD_DEST_SRC1_SRC2;
    TOP__Operator[TOP_shl_i] = Operator_CODE_SHL_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_shl_ii] = Operator_CODE_SHL_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_shl_r] = Operator_CODE_SHL_DEST_SRC1_SRC2;
    TOP__Operator[TOP_shr_i] = Operator_CODE_SHR_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_shr_ii] = Operator_CODE_SHR_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_shr_r] = Operator_CODE_SHR_DEST_SRC1_SRC2;
    TOP__Operator[TOP_shru_i] = Operator_CODE_SHRU_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_shru_ii] = Operator_CODE_SHRU_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_shru_r] = Operator_CODE_SHRU_DEST_SRC1_SRC2;
    TOP__Operator[TOP_slct_i] = Operator_CODE_SLCT_IDEST_SCOND_SRC1_ISRC2;
    TOP__Operator[TOP_slct_ii] = Operator_CODE_SLCT_IDEST_SCOND_SRC1_ISRCX;
    TOP__Operator[TOP_slct_r] = Operator_CODE_SLCT_DEST_SCOND_SRC1_SRC2;
    TOP__Operator[TOP_slctf_i] = Operator_CODE_SLCTF_IDEST_SCOND_SRC1_ISRC2;
    TOP__Operator[TOP_slctf_ii] = Operator_CODE_SLCTF_IDEST_SCOND_SRC1_ISRCX;
    TOP__Operator[TOP_slctf_r] = Operator_CODE_SLCTF_DEST_SCOND_SRC1_SRC2;
    TOP__Operator[TOP_spadjust] = Operator_PSEUDO_PRO64;
    TOP__Operator[TOP_stb_i] = Operator_CODE_STB_ISRC2_SRC1_SRC2;
    TOP__Operator[TOP_stb_ii] = Operator_CODE_STB_ISRCX_SRC1_SRC2;
    TOP__Operator[TOP_sth_i] = Operator_CODE_STH_ISRC2_SRC1_SRC2;
    TOP__Operator[TOP_sth_ii] = Operator_CODE_STH_ISRCX_SRC1_SRC2;
    TOP__Operator[TOP_stw_i] = Operator_CODE_STW_ISRC2_SRC1_SRC2;
    TOP__Operator[TOP_stw_ii] = Operator_CODE_STW_ISRCX_SRC1_SRC2;
    TOP__Operator[TOP_sub_i] = Operator_CODE_SUB_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_sub_ii] = Operator_CODE_SUB_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_sub_r] = Operator_CODE_SUB_DEST_SRC1_SRC2;
    TOP__Operator[TOP_sxtb_r] = Operator_CODE_SXTB_IDEST_SRC1;
    TOP__Operator[TOP_sxth_r] = Operator_CODE_SXTH_IDEST_SRC1;
    TOP__Operator[TOP_sync] = Operator_CODE_SYNC;
    TOP__Operator[TOP_syscall] = Operator_CODE_SYSCALL;
    TOP__Operator[TOP_xor_i] = Operator_CODE_XOR_IDEST_SRC1_ISRC2;
    TOP__Operator[TOP_xor_ii] = Operator_CODE_XOR_IDEST_SRC1_ISRCX;
    TOP__Operator[TOP_xor_r] = Operator_CODE_XOR_DEST_SRC1_SRC2;
    TOP__Operator[TOP_zxth_r] = Operator_CODE_ZXTH_IDEST_SRC1;
    // initialize Operator__TOP;
    for (int i = 0; i < Operator__COUNT; i++) Operator__TOP[i] = TOP_UNDEFINED;
    for (int i = 0; i < TOP_UNDEFINED; i++) {
      if (TOP__Operator[i] < 0 || TOP__Operator[i] >= Operator__COUNT);
      else Operator__TOP[TOP__Operator[i]] = (TOP)i;
    }
    // initialize IEC__Modifier
    for (int i = 0; i < EC_MAX; i++) IEC__Modifier[i] = Modifier_;
    // initialize LC__Immediate
    for (int i = 0; i < LC_MAX; i++) LC__Immediate[i] = Immediate_;
    LC__Immediate[0] = Immediate_I_signed_32_overflow_dont; // HACK ALERT
    LC__Immediate[LC_s32] = Immediate_I_signed_32_overflow_dont;
    LC__Immediate[LC_u32] = Immediate_I_signed_32_overflow_dont;
    LC__Immediate[LC_s23] = Immediate_I_signed_23_overflow_signed;
    LC__Immediate[LC_u23] = Immediate_I_unsigned_23_overflow_unsigned;
    LC__Immediate[LC_s9] = Immediate_I_signed_9_overflow_signed;
    // initialize IRC__RegClass
    for (int i = 0; i <= ISA_REGISTER_CLASS_MAX; i++) IRC__RegClass[i] = RegClass_;
    IRC__RegClass[ISA_REGISTER_CLASS_integer] = RegClass_GRC;
    IRC__RegClass[ISA_REGISTER_CLASS_branch] = RegClass_BRC;
    // initialize RegClass__IRC
    for (int i = 0; i < RegClass__COUNT; i++) RegClass__IRC[i] = ISA_REGISTER_CLASS_UNDEFINED;
    for (int i = 0; i <= ISA_REGISTER_CLASS_MAX; i++) {
      if (IRC__RegClass[i] < 0 || IRC__RegClass[i] >= RegClass__COUNT);
      else RegClass__IRC[IRC__RegClass[i]] = (ISA_REGISTER_CLASS)i;
    }
  }
}

// Finalization of the LAO, needs to be called once.
void
lao_fini() {
  if (--lao_initialized == 0) {
    // Release the PRO64/LAO interface pointers.
    lao_optimize_LOOP_p = NULL;
    lao_optimize_HB_p = NULL;
    lao_optimize_PU_p = NULL;
    CGIR_print_p = NULL;
  }
}


/*-------------------------- LAO Utility Functions----------------------------*/

// Initialize the interface pointers.
static void
lao_initializeInterface() {
  // Initialize the interface call-back functions.
  *Interface__CGIR_LAB_identity(interface) = CGIR_LAB_identity;
  *Interface__CGIR_LAB_create(interface) = CGIR_LAB_create;
  *Interface__CGIR_LAB_update(interface) = CGIR_LAB_update;
  *Interface__CGIR_SYM_identity(interface) = CGIR_SYM_identity;
  *Interface__CGIR_SYM_create(interface) = CGIR_SYM_create;
  *Interface__CGIR_SYM_update(interface) = CGIR_SYM_update;
  *Interface__CGIR_TN_identity(interface) = CGIR_TN_identity;
  *Interface__CGIR_Dedicated_TN_create(interface) = CGIR_Dedicated_TN_create;
  *Interface__CGIR_PseudoReg_TN_create(interface) = CGIR_PseudoReg_TN_create;
  *Interface__CGIR_Modifier_TN_create(interface) = CGIR_Modifier_TN_create;
  *Interface__CGIR_Absolute_TN_create(interface) = CGIR_Absolute_TN_create;
  *Interface__CGIR_Symbol_TN_create(interface) = CGIR_Symbol_TN_create;
  *Interface__CGIR_Label_TN_create(interface) = CGIR_Label_TN_create;
  *Interface__CGIR_TN_update(interface) = CGIR_TN_update;
  *Interface__CGIR_OP_identity(interface) = CGIR_OP_identity;
  *Interface__CGIR_OP_create(interface) = CGIR_OP_create;
  *Interface__CGIR_OP_update(interface) = CGIR_OP_update;
  *Interface__CGIR_BB_identity(interface) = CGIR_BB_identity;
  *Interface__CGIR_BB_create(interface) = CGIR_BB_create;
  *Interface__CGIR_BB_update(interface) = CGIR_BB_update;
  *Interface__CGIR_BB_chain(interface) = CGIR_BB_chain;
  *Interface__CGIR_BB_unchain(interface) = CGIR_BB_unchain;
  *Interface__CGIR_BB_link(interface) = CGIR_BB_link;
  *Interface__CGIR_BB_unlink(interface) = CGIR_BB_unlink;
  *Interface__CGIR_BB_linkFallThru(interface) = CGIR_BB_linkFallThru;
  *Interface__CGIR_LI_identity(interface) = CGIR_LI_identity;
  *Interface__CGIR_LI_create(interface) = CGIR_LI_create;
  *Interface__CGIR_LI_update(interface) = CGIR_LI_update;
}

typedef list<BB*> BB_List;

// Test if a BB belongs to a BB_List.
static bool
lao_in_bblist(BB_List& bb_list, BB *bb) {
  //
  BB_List::iterator bb_iter;
  for (bb_iter = bb_list.begin(); bb_iter != bb_list.end(); bb_iter++) {
    if (*bb_iter == bb) return true;
  }
  //
  return false;
}

// Find the entry BBs of a BB_SET.
static int lao_fill_entry_bblist(BB_SET *body_set, BB_List &bodyBBs, BB_List &entryBBs) {
  int entry_count = 0;
  BB *head_bb = bodyBBs.front();
  BB *fall_entry_bb = BB_Fall_Thru_Predecessor(head_bb);
  BBLIST* pred_list = NULL;
  FOR_ALL_BB_PREDS(head_bb, pred_list) {
    BB* pred_bb = BBLIST_item(pred_list);
    if (!BB_SET_MemberP(body_set, pred_bb)) {
      if (pred_bb != fall_entry_bb) {
	entryBBs.push_back(pred_bb);
	++entry_count;
      }
    }
  }
  if (fall_entry_bb != NULL) {
    entryBBs.push_back(fall_entry_bb);
    ++entry_count;
  }
  return entry_count;
}

// Find the exit BBs of a BB_SET.
static int lao_fill_exit_bblist(BB_SET *body_set, BB_List &bodyBBs, BB_List &exitBBs) {
  int exit_count = 0;
  BB *tail_bb = bodyBBs.back();
  BB *fall_exit_bb = BB_Fall_Thru_Successor(tail_bb);
  if (fall_exit_bb != NULL) {
    exitBBs.push_back(fall_exit_bb);
    ++exit_count;
  }
  list<BB*>::iterator bb_iter;
  FOR_ALL_BB_STLLIST_ITEMS_FWD(bodyBBs, bb_iter) {
    BB *body_bb = *bb_iter;
    BBLIST* succ_list = NULL;
    FOR_ALL_BB_SUCCS(body_bb, succ_list) {
      BB* succ_bb = BBLIST_item(succ_list);
      if (!BB_SET_MemberP(body_set, succ_bb) &&
	  succ_bb != fall_exit_bb &&
	  !lao_in_bblist(exitBBs, succ_bb)) {
	exitBBs.push_back(succ_bb);
	++exit_count;
      }
    }
  }
  return exit_count;
}

// Declare CG_DEP_Compute_Region_MEM_Arcs().
void 
CG_DEP_Compute_Region_MEM_Arcs(list<BB*>    bb_list, 
			    BOOL         compute_cyclic, 
			    BOOL         memread_arcs);

#define LAO_OPS_LIMIT 128	// Maximum number of OPs to compute memory dependences.

// Make a LAO LoopInfo from the LOOP_DESCR supplied.
static LoopInfo
lao_makeLoopInfo(LOOP_DESCR *loop, int pipeline) {
  LoopInfo loopinfo = NULL;
  BB *head_bb = LOOP_DESCR_loophead(loop);
  BasicBlock head_block = CGIR_BB_to_BasicBlock(head_bb);
  ANNOTATION *annot = ANNOT_Get(BB_annotations(head_bb), ANNOT_LOOPINFO);
  if (annot != NULL) {
    // Make the LoopInfo for this loop.
    CGIR_LI cgir_li = ANNOT_loopinfo(annot);
    TN *trip_count_tn = LOOPINFO_trip_count_tn(cgir_li);
    if (trip_count_tn != NULL && TN_is_constant(trip_count_tn)) {
      uint64_t trip_count = TN_value(trip_count_tn);
      int8_t min_trip_count = trip_count <= 127 ? trip_count : 127;
      uint64_t trip_factor = trip_count & -trip_count;
      int8_t min_trip_factor = trip_factor <= 64 ? trip_factor : 64;
      loopinfo = Interface_makeLoopInfo(interface, cgir_li, head_block, 3,
	  Configuration_MinTrip, min_trip_count,
	  Configuration_Modulus, min_trip_factor,
	  Configuration_Residue, 0);
    } else {
      loopinfo = Interface_makeLoopInfo(interface, cgir_li, head_block, 0);
    }
    // Make a BB_List of the loop body and compute its op_count.
    BB *bb = NULL;
    BB_List bb_list;
    int nest_level = BB_nest_level(head_bb), op_count = 0;
    BB_SET *loop_set = LOOP_DESCR_bbset(loop);
    FOR_ALL_BB_SET_members(loop_set, bb) {
      if (BB_nest_level(bb) == nest_level) {
	op_count += BB_length(bb);
	bb_list.push_back(bb);
      }
      if (op_count >= LAO_OPS_LIMIT) break;
    }
    // Compute the memory dependence graph.
    if (op_count < LAO_OPS_LIMIT) {
      bool cyclic = BB_innermost(head_bb) && pipeline > 0;
      CG_DEP_Compute_Region_MEM_Arcs(bb_list, cyclic, false);
      BB_List::iterator bb_iter;
      for (bb_iter = bb_list.begin(); bb_iter != bb_list.end(); bb_iter++) {
	OP *op = NULL;
	FOR_ALL_BB_OPs(*bb_iter, op) {
	  ARC_LIST *arcs = NULL;
	  if (_CG_DEP_op_info(op)) {
	    Operation orig_operation = CGIR_OP_to_Operation(op);
	    for (arcs = OP_succs(op); arcs; arcs = ARC_LIST_rest(arcs)) {
	      ARC *arc = ARC_LIST_first(arcs);
	      CG_DEP_KIND kind = ARC_kind(arc);
	      if (ARC_is_mem(arc)) {
		bool definite = ARC_is_definite(arc);
		int latency = ARC_latency(arc), omega = ARC_omega(arc);
		OP *pred_op = ARC_pred(arc), *succ_op = ARC_succ(arc);
		Is_True(pred_op == op, ("Error in lao_setMemoryDependences"));
		Operation dest_operation = CGIR_OP_to_Operation(succ_op);
		Interface_LoopInfo_setMemoryDependence(interface, loopinfo,
		    orig_operation, dest_operation, latency, omega, definite);
		//CG_DEP_Trace_Arc(arc, TRUE, FALSE);
	      }
	    }
	  } else fprintf(TFile, "<arc>   CG_DEP INFO is NULL\n");
	}
      }
      CG_DEP_Delete_Graph(&bb_list);
    }
  }
  return loopinfo;
}

/*----------------------- LAO Optimization Functions -------------------------*/

// Low-level LAO_optimize entry point.
static bool
lao_optimize(BB_List &entryBBs, BB_List &bodyBBs, BB_List &exitBBs, int pipeline, unsigned lao_optimizations) {
  bool result = false;
  //
  if (getenv("PRINT")) CGIR_print();
  LAO_INIT();
  Interface_open(interface, 3,
      Configuration_Schedule, CG_LAO_schedule,
      Configuration_Pipeline, CG_LAO_pipeline,
      Configuration_Speculate, CG_LAO_speculate);
  lao_initializeInterface();
  //
  // Create the LAO BasicBlocks.
  BB_List nonexitBBs;
  BB_List::iterator bb_iter;
  for (bb_iter = entryBBs.begin(); bb_iter != entryBBs.end(); bb_iter++) {
    BasicBlock basicblock = CGIR_BB_to_BasicBlock(*bb_iter);
    Interface_setEntry(interface, basicblock);
    nonexitBBs.push_back(*bb_iter);
    //fprintf(TFile, "BB_entry(%d)\n", BB_id(*bb_iter));
  }
  for (bb_iter = bodyBBs.begin(); bb_iter != bodyBBs.end(); bb_iter++) {
    BasicBlock basicblock = CGIR_BB_to_BasicBlock(*bb_iter);
    Interface_setBody(interface, basicblock);
    nonexitBBs.push_back(*bb_iter);
    //fprintf(TFile, "BB_body(%d)\n", BB_id(*bb_iter));
  }
  for (bb_iter = exitBBs.begin(); bb_iter != exitBBs.end(); bb_iter++) {
    BasicBlock basicblock = CGIR_BB_to_BasicBlock(*bb_iter);
    Interface_setExit(interface, basicblock);
    //fprintf(TFile, "BB_exit(%d)\n", BB_id(*bb_iter));
  }
  // Make the control-flow nodes and the control-flow arcs.
  for (bb_iter = nonexitBBs.begin(); bb_iter != nonexitBBs.end(); bb_iter++) {
    BB *orig_bb = *bb_iter;
    ControlNode orig_controlnode = CGIR_BB_to_ControlNode(orig_bb);
    BBLIST *bblist = NULL;
    FOR_ALL_BB_SUCCS(orig_bb, bblist) {
      BB *succ_bb = BBLIST_item(bblist);
      if (lao_in_bblist(exitBBs, succ_bb) || lao_in_bblist(nonexitBBs, succ_bb)) {
	ControlNode succ_controlnode = CGIR_BB_to_ControlNode(succ_bb);
	Interface_linkControlNodes(interface, orig_controlnode, succ_controlnode, BBLIST_prob(bblist));
      }
    }
  }
  // Make the LoopInfos for the bodyBBs.
  for (bb_iter = bodyBBs.begin(); bb_iter != bodyBBs.end(); bb_iter++) {
    BB *bb = *bb_iter;
    if (BB_loophead(bb)) {
      LOOP_DESCR *loop = LOOP_DESCR_Find_Loop(bb);
      lao_makeLoopInfo(loop, pipeline);
    }
  }
  //
  result = LAO_Optimize(lao_optimizations);
  if (0 && result) {
    Interface_updateCGIR(interface);
    if (getenv("PRINT")) CGIR_print();
  }
  //
  Interface_close(interface);
  LAO_FINI();
  //
  return result;
}

// Optimize a LOOP_DESCR inner loop through the LAO.
static bool
lao_optimize_LOOP(LOOP_DESCR *loop, unsigned lao_optimizations) {
  Is_True(BB_innermost(LOOP_DESCR_loophead(loop)),
      ("lao_optimize_LOOP must be called on inner loop"));
  BB *head_bb = LOOP_DESCR_loophead(loop);
  BB_SET *body_set = LOOP_DESCR_bbset(loop);
//cerr << "lao_optimize_LOOP(" << BB_id(head_bb) << ", " << lao_optimizations << ")\n";
  //
  // Compute the pipeline value.
  BB *tail_bb = LOOP_DESCR_Find_Unique_Tail(loop);
  bool prepass = (lao_optimizations & Optimization_Prepass) != 0;
  int pipeline = (tail_bb != NULL)*prepass*CG_LAO_pipeline;
  //
  // Adjust the control-flow if required.
  if (pipeline > 0) {
    // Software pipelining (implies prepass scheduling).
    //BB *prolog_bb = CG_LOOP_Gen_And_Prepend_To_Prolog(head_bb, loop);
    //GRA_LIVE_Compute_Liveness_For_BB(prolog_bb);
  } else {
    // Non software pipelining (prepass or postpass scheduling).
  }
  //
  // List the body BBs.
  BB_List bodyBBs;
  int body_count = 0;
  for (BB *bb = head_bb; bb != BB_next(tail_bb); bb = BB_next(bb)) {
    if (BB_SET_MemberP(body_set, bb)) {
      bodyBBs.push_back(bb);
      body_count++;
    }
  }
  Is_True(body_count == BB_SET_Size(body_set), ("lao_optimize_LOOP computed incorrect body"));
  //
  // List the entry BBs.
  BB_List entryBBs;
  int entry_count = lao_fill_entry_bblist(body_set, bodyBBs, entryBBs);
  //
  // List the exit BBs.
  BB_List exitBBs;
  int exit_count = lao_fill_exit_bblist(body_set, bodyBBs, exitBBs);
  Is_True(exit_count == LOOP_DESCR_num_exits(loop), ("lao_optimize_LOOP computed incorrect exits"));
  //
  // Call the lower level lao_optimize function.
  return lao_optimize(entryBBs, bodyBBs, exitBBs, pipeline, lao_optimizations);
}

// Optimize a HB through the LAO.
static bool
lao_optimize_HB(HB *hb, unsigned lao_optimizations) {
  BB *head_bb = HB_Entry(hb);
  BB_SET *body_set = HB_Blocks(hb);
  //
  // Call lao_optimize_LOOP if HB is an inner loop.
  LOOP_DESCR *loop = LOOP_DESCR_Find_Loop(head_bb);
  if (loop != NULL) {
    if (Is_Inner_Loop(loop) && LOOP_DESCR_loophead(loop) == head_bb &&
	BB_SET_Size(body_set) == BB_SET_Size(LOOP_DESCR_bbset(loop))) {
      return lao_optimize_LOOP(loop, lao_optimizations);
    }
  }
  //
  // List the body BBs.
  BB_List &bodyBBs = *HB_Blocks_List(hb);
  Is_True(bodyBBs.front() == head_bb, ("lao_optimize_HB computed incorrect body"));
  //
  // List the entry BBs.
  BB_List entryBBs;
  int entry_count = lao_fill_entry_bblist(body_set, bodyBBs, entryBBs);
  //
  // List the exit BBs.
  BB_List exitBBs;
  int exit_count = lao_fill_exit_bblist(body_set, bodyBBs, exitBBs);
  //Is_True(exitBBs.front() == HB_Fall_Thru_Exit(hb),
      //("lao_optimize_HB computed incorrect exits: BB(%d) != BB(%d)",
	  //BB_id(exitBBs.front()), BB_id(HB_Fall_Thru_Exit(hb))));
  //
  // Call the lower level lao_optimize function with pipeline=0.
  return lao_optimize(entryBBs, bodyBBs, exitBBs, 0, lao_optimizations);
}

// Optimize the complete PU through the LAO.
static bool
lao_optimize_PU(unsigned lao_optimizations) {
//cerr << "lao_optimize_PU(" << lao_optimizations << ")\n";
  bool result = false;
  MEM_POOL lao_loop_pool;
  MEM_POOL_Initialize(&lao_loop_pool, "LAO loop_descriptors", TRUE);
  //
  MEM_POOL_Push(&lao_loop_pool);
  Calculate_Dominators();
  LOOP_DESCR *loop = LOOP_DESCR_Detect_Loops(&lao_loop_pool);
  //
  // Software pipeline the innermost loops.
  if (lao_optimizations & Optimization_Prepass && CG_LAO_pipeline > 0) {
    while (loop != NULL && BB_innermost(LOOP_DESCR_loophead(loop))) {
      result != lao_optimize_LOOP(loop, lao_optimizations);
      loop = LOOP_DESCR_next(loop);
    }
  }
  //
  // Update the LOOP_DESCRs in order to update the BB fields.
  if (result) {
    Free_Dominators_Memory();
    MEM_POOL_Pop(&lao_loop_pool);
    //
    GRA_LIVE_Recalc_Liveness(NULL);
    GRA_LIVE_Rename_TNs();
    //
    MEM_POOL_Push(&lao_loop_pool);
    Calculate_Dominators();
    LOOP_DESCR_Detect_Loops(&lao_loop_pool);
  }
  //
  // List the BBs, entry BBs, exit BBs, body BBs.
  BB_List entryBBs, exitBBs, bodyBBs;
  for (BB *bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    if (BB_entry(bb)) {
      entryBBs.push_back(bb);
    } else if (BB_exit(bb)) {
      exitBBs.push_back(bb);
    } else {
      bodyBBs.push_back(bb);
    }
  }
  //
  // Call the lower level lao_optimize function with pipeline=0.
  result |= lao_optimize(entryBBs, bodyBBs, exitBBs, 0, lao_optimizations);
  //
  Free_Dominators_Memory();
  MEM_POOL_Pop(&lao_loop_pool);
  //
  MEM_POOL_Delete(&lao_loop_pool);
  return result;
}

/*-------------------------- CGIR Print Functions ----------------------------*/

typedef struct OP_list {
  OP *op;
  struct OP_list *next;
} OP_list;

static OP_list * OP_list_new(OP_list *head)
{
  OP_list * elem;
  //
  elem = (OP_list *)malloc(sizeof(OP_list));
  elem->next = head;
  head = elem;
  return head;
}

static void
CGIR_TN_print ( const TN *tn )
{
  //
  if (TN_is_constant(tn)) {
    if ( TN_has_value(tn)) {
      fprintf ( TFile, "(0x%llx)", TN_value(tn) );
      if (TN_size(tn) == 4 && 
	  TN_value(tn) >> 32 != 0 &&
	  TN_value(tn) >> 31 != -1)
	fprintf ( TFile, "!!! TN_value=0x%llx is too big to fit in a word",
		  TN_value(tn));
    }
    else if (TN_is_enum(tn)) {
      fprintf ( TFile, "(enum:%s)", ISA_ECV_Name(TN_enum(tn)) );
    }
    else if ( TN_is_label(tn) ) {
      CGIR_LAB lab = TN_label(tn);
      const char *name = LABEL_name(lab);
      INT64 offset = TN_offset(tn);
      BB *targetBB;
      if ( offset == 0 ) {
	fprintf ( TFile, "(lab:%s)", name );
      }
      else {
	fprintf ( TFile, "(lab:%s+%lld)", name, offset );
      }
      targetBB = Get_Label_BB(lab); // XXX
      if (targetBB != NULL)
	fprintf(TFile, " --> %d", BB_id(targetBB));
    } 
    else if ( TN_is_tag(tn) ) {
      CGIR_LAB lab = TN_label(tn);
      const char *name = LABEL_name(lab);
      fprintf ( TFile, "(tag:%s)", name );
    }
    else if ( TN_is_symbol(tn) ) {
      ST *var = TN_var(tn);
      //
      fprintf ( TFile, "(sym" );
      fprintf ( TFile, TN_RELOCS_Name(TN_relocs(tn)) );
      //
      if (ST_class(var) == CLASS_CONST)
      	fprintf ( TFile, ":%s)", Targ_Print(NULL, ST_tcon_val(var)));
      else
      	fprintf ( TFile, ":%s%+lld)", ST_name(var), TN_offset(tn) );
    } 
    else {
      ErrMsg (EC_Unimplemented, "CGIR_TN_print: illegal constant TN");
    }
  }
  else {  /* register TN */
    if (TN_register(tn) != REGISTER_UNDEFINED) {
      if (TN_register(tn) <= REGISTER_CLASS_last_register(TN_register_class(tn))) {
	fprintf ( TFile, "%s", 
		  REGISTER_name(TN_register_class(tn), TN_register(tn)));
      } else {
	fprintf ( TFile, "(%d,%d)", TN_register_class(tn), TN_register(tn));
      }
    }
    else if (TN_is_global_reg(tn)) {
      fprintf ( TFile, "G%d", TN_number(tn) );
    }
    else {
      fprintf ( TFile, "T%d", TN_number(tn) );
    }
    if (TN_is_save_reg(tn)) {
      fprintf ( TFile, "(sv:%s)", 
		REGISTER_name(TN_save_rclass(tn), TN_save_reg(tn)));
    }
  }
}

static void
CGIR_OP_print ( const OP *op, bool bb_scheduled)
{
  int i;
  //
  //
  {
    LABEL_IDX tag = Get_OP_Tag(op);
    if(tag) fprintf (TFile, "<tag %s>: ", LABEL_name(tag));
  }
  //
  fprintf(TFile, "%s", TOP_Name(OP_code(op)));
  //
  if ( OP_variant(op) != 0 ) {
    fprintf ( TFile, "(%x)", OP_variant(op));
  }
  //
  if (OP_results(op) == 0)
    fprintf(TFile, " void");
  //
  else for (i = 0; i < OP_results(op); i++) {
    fprintf(TFile, "%s", (i == 0 ? " " : ", "));
    CGIR_TN_print(OP_result(op,i));
  }
  //
  fprintf(TFile, " =");
  //
  for (i=0; i<OP_opnds(op); i++) {
    fprintf(TFile, "%s", (i == 0 ? " " : ", "));
    TN *tn = OP_opnd(op,i);
    CGIR_TN_print(tn);
    if (OP_Defs_TN(op, tn)) fprintf(TFile, "<def>");
  }

  if (bb_scheduled)
    fprintf(TFile, "\tscycle = %d", OP_scycle(op));

  // TBD: Print other attributes on operations.
}

static void
CGIR_OP_prints ( const OP *op , bool bb_scheduled)
{
  for ( ; op; op = OP_next(op)) {
    fprintf(TFile, "\t");
    CGIR_OP_print(op, bb_scheduled);
    fprintf(TFile, "       \t#line[%4d]", Srcpos_To_Line(OP_srcpos(op)));
    fprintf(TFile, "\n");
  }
}

static void
CGIR_BB_print_header (BB *bp)
{
  BBLIST *bl;
  INT16 i;
  ANNOTATION *annot = ANNOT_Get(BB_annotations(bp), ANNOT_LOOPINFO);
  BOOL freqs = FREQ_Frequencies_Computed();
  //
  if ( BB_entry(bp) ) {
    ANNOTATION *ant = ANNOT_Get (BB_annotations(bp), ANNOT_ENTRYINFO);
    ENTRYINFO *ent = ANNOT_entryinfo (ant);
    OP *sp_adj = BB_entry_sp_adj_op(bp);
    Is_True ((sp_adj == ENTRYINFO_sp_adj(ent)),("bad sp_adj"));
    //
    fprintf ( TFile, "Entrypoint: %s\t Starting Line %d\n",
	      ST_name(ENTRYINFO_name(ent)),
	      Srcpos_To_Line(ENTRYINFO_srcpos(ent)));
    //
    if (sp_adj) {
      OP *op;
      BOOL found_sp_adj = FALSE;
      fprintf ( TFile, "SP entry adj: " );
      Print_OP_No_SrcLine (sp_adj);
      FOR_ALL_BB_OPs_FWD(bp,op)
	if (op == sp_adj) {
	  found_sp_adj = TRUE;
	  break;
	}
      if (found_sp_adj == FALSE)
	fprintf ( TFile, "******** ERROR ******** sp adjust not found in entry block\n");
    }
  }
  //
  if ( BB_exit(bp) ) {
    ANNOTATION *ant = ANNOT_Get (BB_annotations(bp), ANNOT_EXITINFO);
    EXITINFO *exit = ANNOT_exitinfo (ant);
    OP *sp_adj = BB_exit_sp_adj_op(bp);
    Is_True ((sp_adj == EXITINFO_sp_adj(exit)),("bad sp_adj"));
    //
    if (sp_adj) {
      OP *op;
      BOOL found_sp_adj = FALSE;
      fprintf ( TFile, "SP exit adj: " );
      Print_OP_No_SrcLine (sp_adj);
      //
      FOR_ALL_BB_OPs_FWD(bp,op)
	if (op == sp_adj) {
	  found_sp_adj = TRUE;
	  break;
	}
      if (found_sp_adj == FALSE)
	fprintf ( TFile, "******** ERROR ******** sp adjust not found in exit block\n");
    }
  }
  //
  fprintf ( TFile, "    BB %d, flags 0x%04x",
	    BB_id(bp), BB_flag(bp) );
  //
  if (freqs || BB_freq_fb_based(bp))
    fprintf(TFile, ", freq %g (%s)", BB_freq(bp),
	    BB_freq_fb_based(bp) ? "feedback" : "heuristic");
  
  if (BB_unreachable(bp)) fprintf ( TFile, ", Unreachable");
  if (BB_entry(bp))	fprintf ( TFile, ", Entry" );
  if (BB_handler(bp))	fprintf ( TFile, ", Handler" );
  if (BB_asm(bp)) 	fprintf ( TFile, ", Asm" );
  //
  if (BB_exit(bp)) {
    if (BB_call(bp))	fprintf ( TFile, ", Tail-call" );
    else		fprintf ( TFile, ", Exit" );
  } else if (BB_call(bp)) fprintf ( TFile, ", Call" );
  //
  if (BB_rid(bp)) {
    INT exits;
    RID *rid = BB_rid(bp);
    CGRIN *cgrin = RID_cginfo(rid);
    if (cgrin) {
      if (bp == CGRIN_entry(cgrin)) {
	fprintf ( TFile, ", Region-entry " );
      }
      exits = RID_num_exits(rid);
      for (i = 0; i < exits; ++i) {
	if (bp == CGRIN_exit_i(cgrin, i)) {
	  fprintf ( TFile, ", Region-exit[%d]", i );
	}
      }
    }
  }
  //
  fprintf ( TFile, "\n");
  //
  if (annot)
    Print_LOOPINFO(ANNOT_loopinfo(annot));
  //
  if (BB_loop_head_bb(bp)) {
    if (BB_loophead(bp)) {
      if (!annot) {
	fprintf(TFile, "\tHead of loop body line %d\n", BB_Loop_Lineno(bp));
      }
    } else {
      BB *head = BB_loop_head_bb(bp);
      fprintf(TFile,
	      "\tPart of loop body starting at line %d with head BB:%d\n",
	      BB_Loop_Lineno(head), BB_id(head));
    }
  }
  //
  if (BB_unrollings(bp) > 1)
    fprintf(TFile, "\tUnrolled %d times%s\n", BB_unrollings(bp),
	    BB_unrolled_fully(bp) ? " (fully)" : "");
  //
  if ( BB_rid(bp) )
    RID_Fprint( TFile, BB_rid(bp) );
  //
  fprintf ( TFile, "\tpred" );
  FOR_ALL_BB_PREDS (bp, bl) {
    fprintf ( TFile, " %d", BB_id(BBLIST_item(bl)));
  }
  //
  fprintf ( TFile, "\n\tsucc%s", freqs ? " (w/probs)" : "" );
  FOR_ALL_BB_SUCCS (bp, bl) {
    fprintf ( TFile, " %d",
	      BB_id(BBLIST_item(bl)));
    if (freqs) fprintf(TFile, "(%g)", BBLIST_prob(bl));
  }
  fprintf ( TFile, "\n" );
  //
  if (BB_has_label(bp)) {
    ANNOTATION *ant;
    fprintf(TFile, "\tLabel");
    for (ant = ANNOT_First(BB_annotations(bp), ANNOT_LABEL);
	 ant != NULL;
	 ant = ANNOT_Next(ant, ANNOT_LABEL))
      {
	INT eh_labs = 0;
	LABEL_IDX lab = ANNOT_label(ant);
	fprintf (TFile," %s", LABEL_name(lab));
	FmtAssert((Get_Label_BB(lab) == bp),
		  (" Inconsistent ST for BB:%2d label", BB_id(bp)));
	switch (LABEL_kind(Label_Table[lab])) {
	case LKIND_BEGIN_EH_RANGE:
	  fprintf (TFile,"%cbegin_eh_range", eh_labs++ ? ' ' : '(');
	  break;
	case LKIND_END_EH_RANGE:
	  fprintf (TFile,"%cend_eh_range", eh_labs++ ? ' ' : '(');
	  break;
	}
	if (eh_labs)
	  fprintf (TFile,")");
      }
    fprintf(TFile, "\n");
  }
  //
  return;
}

static void
CGIR_BB_print (BB *bp)
{
  CGIR_BB_print_header (bp );
  if (BB_first_op(bp))	CGIR_OP_prints (BB_first_op(bp), BB_scheduled(bp));
}

static void
CGIR_Alias_print()
{
  OP_list *memops = NULL, *elt1, *elt2;
  BB *bp;
  OP *op;
  BOOL alias, identical;
  //
  for (bp = REGION_First_BB; bp; bp = BB_next(bp)) {
    for (op = BB_first_op(bp); op; op = OP_next(op)) {
      if (OP_memory(op)) {
	memops = OP_list_new(memops);
	memops->op = op;
      }
    }
  }
  //
  fprintf(TFile, "--------------- Begin Print Alias ---------------\n");
  //
  for (elt1 = memops; elt1; elt1 = elt1->next) {
    fprintf(TFile, "<Alias>"); CGIR_OP_print(elt1->op, FALSE); fprintf(TFile, "\n");
    for (elt2 = memops; elt2 != elt1; elt2 = elt2->next) {
      fprintf(TFile, "\t<with>"); CGIR_OP_print(elt2->op, FALSE); fprintf(TFile, "\t");
      alias = CG_DEP_Mem_Ops_Alias(elt1->op, elt2->op, &identical);
      if (!alias)          fprintf(TFile, "NO-ALIAS");
      else if (!identical) fprintf(TFile, "   ALIAS");
      else                 fprintf(TFile, "IDENTICAL");
      fprintf(TFile, "</with>\n");
    }
    fprintf(TFile, "</Alias>\n");
  }
  //
  fprintf(TFile, "---------------- End Print Alias ----------------\n");
}

static void
CGIR_print()
{
  BB *bp;
  //
  fprintf(TFile, "--------CFG Begin--------\n");
  for (bp = REGION_First_BB; bp; bp = BB_next(bp)) {
    CGIR_BB_print ( bp );
    fprintf ( TFile,"\n" );
  }
  //
  //CGIR_Alias_print();
  fprintf(TFile, "-------- CFG End --------\n");
}

