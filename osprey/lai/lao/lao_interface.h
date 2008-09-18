

#define i386 1
#define linux 1
#define unix 1



#define Except_REQUIRE(t) 
#define Except_CHECK(t) 
typedef struct Memory_ *Memory;
typedef struct Label_ *Label;
typedef struct Symbol_ *Symbol;
typedef struct Temporary_ *Temporary;
typedef struct Operation_ *Operation;
typedef struct BasicBlock_ *BasicBlock;
typedef struct LoopScope_ *LoopScope;

#define CGIR_h_INCLUDED 

/*
 * !!!!	CGIR.xcc
 *
 * Benoit Dupont de Dinechin (Benoit.Dupont-de-Dinechin@st.com).
 * Christophe Guillon (Christophe.Guillon@st.com).
 *
 * Copyright 2002 - 2007 STMicroelectronics.
 * Copyright 1995 - 1998 Commissariat a l'Energie Atomique.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of either (at your option): the GNU
 * General Public License (GPL) version 2; the GNU Lesser General
 * Public License (LGPL) version 2.1; any later version of these
 * licences as published by the Free Software Foundation.
 */

/*
 * Host compiler CGIR types.
 */
typedef uint32_t CGIR_LAB;
typedef uint32_t CGIR_SYM;
typedef struct WN *CGIR_WN;
typedef struct tn *CGIR_TN;
typedef struct op *CGIR_OP;
typedef struct bb *CGIR_BB;
typedef struct LOOP_DESCR *CGIR_LD;
typedef struct region_id *CGIR_RID;

/*
 * Interface LAO enumerations.
 */
typedef int O64_OptimizeItem;
typedef int O64_ConfigureItem;
typedef void *O64_DependenceNode;
typedef int O64_DependenceKind;

/*
 * Interface LAO target enumerations.
 */
typedef int O64_Immediate;
typedef int O64_Processor;
typedef int O64_NativeType;
typedef int O64_Convention;
typedef int O64_Operator;
typedef int O64_RegFile;
typedef int O64_Register;

//
typedef enum {
  CGIR_Type_Virtual,
  CGIR_Type_Assigned,
  CGIR_Type_Dedicated,
  CGIR_Type_Absolute,
  CGIR_Type_Symbol,
  CGIR_Type_Label,
  CGIR_Type__,
} CGIR_Type;
const char *
CGIR_Type_(CGIR_Type type);

/*
 * O64_SymbolClass --	Used for symbol class definition.
 * Must match the LAO Sclass definition in LIR/Symbol.h.
 */
typedef enum {
  O64_SymbolClass_UNDEF,
  O64_SymbolClass_VAR,
  O64_SymbolClass_FUNC,
  O64_SymbolClass_CONST,
  O64_SymbolClass_PREG,
  O64_SymbolClass_BLOCK,
  O64_SymbolClass__
} O64_SymbolClass;

/*
 * O64_SymbolStore --	Used for symbol store definition.
 * Must match the LAO SymbolStore definition in LIR/Symbol.h.
 */
typedef enum {
  O64_SymbolStore_UNDEF,
  O64_SymbolStore_AUTO,
  O64_SymbolStore_FORMAL,
  O64_SymbolStore_FORMAL_REF,
  O64_SymbolStore_PSTATIC,
  O64_SymbolStore_FSTATIC,
  O64_SymbolStore_COMMON,
  O64_SymbolStore_EXTERN,
  O64_SymbolStore_UGLOBAL,
  O64_SymbolStore_DGLOBAL,
  O64_SymbolStore_TEXT,
  O64_SymbolStore_REG,
  O64_SymbolStore__
} O64_SymbolStore;

/*
 * O64_SymbolExport --	Used for symbol export definition.
 * Must match the LAO SymbolExport definition in LIR/Symbol.h.
 */
typedef enum {
  O64_SymbolExport_UNDEF,
  O64_SymbolExport_LOCAL,
  O64_SymbolExport_LOCAL_INTERNAL,
  O64_SymbolExport_GLOBAL_INTERNAL,
  O64_SymbolExport_GLOBAL_HIDDEN,
  O64_SymbolExport_GLOBAL_PROTECTED,
  O64_SymbolExport_GLOBAL_PREEMPTIBLE,
  O64_SymbolExport__
} O64_SymbolExport;

/*
 * O64_OperationCGIR --	Used to pass OperationFlag to CGIR_OP.
 */
typedef enum {
  O64_OperationCGIR_SafeAccess = 0x1,
  O64_OperationCGIR_Hoisted = 0x2,
  O64_OperationCGIR_Volatile = 0x4,
  O64_OperationCGIR_Prefetch = 0x8,
  O64_OperationCGIR_Preload = 0x10,
  O64_OperationCGIR_Barrier = 0x20,
  O64_OperationCGIR_SpillCode = 0x40,
  O64_OperationCGIR_SafePerfs = 0x80,
} O64_OperationCGIR;

/*
 * O64_BasicBlockCGIR --	Used to pass BasicBlockFlag to CGIR_OP.
 */
typedef enum {
  O64_BasicBlockCGIR_Allocated = 0x1,
  O64_BasicBlockCGIR_Scheduled = 0x2,
} O64_BasicBlockCGIR;

/*
 * uint32_t CGIR_LAB_identity(CGIR_LAB cgir_lab); --	Identity of a CGIR_LAB.
 */
#define CGIR_LAB_identity(cgir_lab) (uint32_t)(cgir_lab)


/*
 * uint32_t CGIR_SYM_identity(CGIR_SYM cgir_sym); --	Identity of a CGIR_SYM.
 */
#define CGIR_SYM_identity(cgir_sym) (uint32_t)(cgir_sym)


/*
 * uint32_t CGIR_TN_identity(CGIR_TN cgir_tn); --	Identity of a CGIR_TN.
 */
#define CGIR_TN_identity(cgir_tn) (uint32_t)(cgir_tn)


/*
 * uint32_t CGIR_OP_identity(CGIR_OP cgir_op); --	Identity of a CGIR_OP.
 */
#define CGIR_OP_identity(cgir_op) (uint32_t)(cgir_op)


/*
 * uint32_t CGIR_BB_identity(CGIR_BB cgir_bb); --	Identity of a CGIR_BB.
 */
#define CGIR_BB_identity(cgir_bb) (uint32_t)(cgir_bb)


/*
 * uint32_t CGIR_LD_identity(CGIR_LD cgir_ld); --	Identity of a CGIR_LD.
 */
#define CGIR_LD_identity(cgir_ld) (uint32_t)(cgir_ld)


/*
 * CGIR_LAB_make --	Update a CGIR_LAB.
 */
static CGIR_LAB
CGIR_LAB_make(CGIR_LAB cgir_lab, const char *name);

/*
 * CGIR_SYM_make --	Update a CGIR_SYM.
 */
static CGIR_SYM
CGIR_SYM_make(CGIR_SYM cgir_sym, const char *name, bool isSpill, O64_NativeType lai_nativeType);

/*
 * CGIR_TN_make --	Update a CGIR_TN.
 */
static CGIR_TN
CGIR_TN_make(CGIR_TN cgir_tn, CGIR_Type cgir_type, ...);

/*
 * CGIR_OP_make --	Update a CGIR_OP from a LIR Operation.
 */
static CGIR_OP
CGIR_OP_make(CGIR_OP cgir_op, O64_Operator lai_operator, CGIR_TN arguments[], CGIR_TN results[], CGIR_OP orig_op);

//
static void
CGIR_OP_more(CGIR_OP cgir_op, int iteration, int issueDate, unsigned flags);

/*
 * void CGIR_BB_make --	Update a CGIR_BB.
 */
static CGIR_BB
CGIR_BB_make(CGIR_BB cgir_bb, CGIR_LAB labels[], CGIR_OP operations[], CGIR_RID cgir_rid, float frequency);

//
static void
CGIR_BB_more(CGIR_BB cgir_bb, CGIR_BB loop_bb, intptr_t traceId, int unrolled, unsigned flags);

/*
 * CGIR_LD_make --	Update a CGIR_LD.
 */
static CGIR_LD
CGIR_LD_make(CGIR_LD cgir_ld, CGIR_BB head_bb, CGIR_TN trip_count_tn, int unrolled);

/*
 * CGIR_BB_chain --	Chain two CGIR_BBs in the CGIR.
 */
static void
CGIR_BB_chain(CGIR_BB cgir_bb, CGIR_BB succ_cgir_bb);

/*
 * CGIR_BB_unchain --	Unchain a CGIR_BB in the CGIR.
 */
static void
CGIR_BB_unchain(CGIR_BB cgir_bb);

/*
 * CGIR_BB_link --	Link two CGIR_BBs in the CGIR with the given branch probability.
 */
static void
CGIR_BB_link(CGIR_BB tail_cgir_bb, CGIR_BB head_cgir_bb, float probability);

/*
 * void CGIR_BB_unlink(CGIR_BB cgir_bb, bool preds, bool succs);
 * --	Unlink the predecessors and/or the successors of a CGIR_BB in the CGIR.
 */
static void
CGIR_BB_unlink(CGIR_BB cgir_bb, bool preds, bool succs);

/*
 * void CGIR_BB_discard(CGIR_BB cgir_bb);
 * --	Discard a CGIR_BB in the CGIR.
 */
static void
CGIR_BB_discard(CGIR_BB cgir_bb);

/*
 * CGIR_CallBack --	Call back functions.
 */
struct CGIR_CallBack_ {
  CGIR_LAB (*LAB_make)(CGIR_LAB cgir_lab, const char *name);
  CGIR_SYM (*SYM_make)(CGIR_SYM cgir_sym, const char *name, bool isSpill, O64_NativeType lai_nativeType);
  CGIR_TN (*TN_make)(CGIR_TN cgir_tn, CGIR_Type cgir_type, ...);
  CGIR_OP (*OP_make)(CGIR_OP cgir_op, O64_Operator lai_operator, CGIR_TN arguments[], CGIR_TN results[], CGIR_OP orig_op);
  void (*OP_more)(CGIR_OP cgir_op, int iteration, int issueDate, unsigned flags);
  CGIR_BB (*BB_make)(CGIR_BB cgir_bb, CGIR_LAB labels[], CGIR_OP operations[], CGIR_RID cgir_rid, float frequency);
  void (*BB_more)(CGIR_BB cgir_bb, CGIR_BB loop_bb, intptr_t traceId, int unrolled, unsigned flags);
  CGIR_LD (*LD_make)(CGIR_LD cgir_ld, CGIR_BB head_bb, CGIR_TN trip_count_tn, int unrolled);
  void (*BB_chain)(CGIR_BB cgir_bb, CGIR_BB succ_cgir_bb);
  void (*BB_unchain)(CGIR_BB cgir_bb);
  void (*BB_link)(CGIR_BB tail_cgir_bb, CGIR_BB head_cgir_bb, float probability);
  void (*BB_unlink)(CGIR_BB cgir_bb, bool preds, bool succs);
  void (*BB_discard)(CGIR_BB cgir_bb);
};
typedef struct CGIR_CallBack_ CGIR_CallBack_, *CGIR_CallBack;
typedef const struct CGIR_CallBack_ *const_CGIR_CallBack;
typedef struct CGIR_CallBack_ * restrict_CGIR_CallBack;

#define CGIR_CallBack_LAB_make(THIS) ((THIS)->LAB_make)
#define CGIR_CallBack__LAB_make(THIS) (&(THIS)->LAB_make)
#define CGIR_CallBack_SYM_make(THIS) ((THIS)->SYM_make)
#define CGIR_CallBack__SYM_make(THIS) (&(THIS)->SYM_make)
#define CGIR_CallBack_TN_make(THIS) ((THIS)->TN_make)
#define CGIR_CallBack__TN_make(THIS) (&(THIS)->TN_make)
#define CGIR_CallBack_OP_make(THIS) ((THIS)->OP_make)
#define CGIR_CallBack__OP_make(THIS) (&(THIS)->OP_make)
#define CGIR_CallBack_OP_more(THIS) ((THIS)->OP_more)
#define CGIR_CallBack__OP_more(THIS) (&(THIS)->OP_more)
#define CGIR_CallBack_BB_make(THIS) ((THIS)->BB_make)
#define CGIR_CallBack__BB_make(THIS) (&(THIS)->BB_make)
#define CGIR_CallBack_BB_more(THIS) ((THIS)->BB_more)
#define CGIR_CallBack__BB_more(THIS) (&(THIS)->BB_more)
#define CGIR_CallBack_LD_make(THIS) ((THIS)->LD_make)
#define CGIR_CallBack__LD_make(THIS) (&(THIS)->LD_make)
#define CGIR_CallBack_BB_chain(THIS) ((THIS)->BB_chain)
#define CGIR_CallBack__BB_chain(THIS) (&(THIS)->BB_chain)
#define CGIR_CallBack_BB_unchain(THIS) ((THIS)->BB_unchain)
#define CGIR_CallBack__BB_unchain(THIS) (&(THIS)->BB_unchain)
#define CGIR_CallBack_BB_link(THIS) ((THIS)->BB_link)
#define CGIR_CallBack__BB_link(THIS) (&(THIS)->BB_link)
#define CGIR_CallBack_BB_unlink(THIS) ((THIS)->BB_unlink)
#define CGIR_CallBack__BB_unlink(THIS) (&(THIS)->BB_unlink)
#define CGIR_CallBack_BB_discard(THIS) ((THIS)->BB_discard)
#define CGIR_CallBack__BB_discard(THIS) (&(THIS)->BB_discard)


//
int
CGIR_Print_LAB_Vector(CGIR_LAB cgir_labels[], FILE *file);

//
int
CGIR_Print_TN_Vector(CGIR_TN cgir_temporaries[], FILE *file);

//
int
CGIR_Print_OP_Vector(CGIR_OP cgir_operations[], FILE *file);

/*
 * O64_Interface --	O64 Interface object.
 * All functions for the LAO Interface are available
 * from THIS object.
 */
struct O64_Interface_ {
  int size;
  void (*Interface_Initialize)(void);
  void (*Interface_Finalize)(void);
  struct Interface_* (*Interface_getInstance)(void);
  void (*Interface_setMaxIssue)(struct Interface_* THIS, O64_Processor processor,
                                int maxIssue);
  void (*Interface_setMinTaken)(struct Interface_* THIS, O64_Processor processor,
                                int minTaken);
  void (*Interface_setReserved)(struct Interface_* THIS, O64_Convention convention,
                                O64_Register reserved);
  bool (*Interface_setArgStage)(struct Interface_* THIS, O64_Processor processor,
                                O64_Operator operator, int index, int stage);
  bool (*Interface_setResStage)(struct Interface_* THIS, O64_Processor processor,
                                O64_Operator operator, int index, int stage);
  Label (*Interface_makeLabel)(struct Interface_* THIS, CGIR_LAB cgir_lab,
                               const char *name);
  Label (*Interface_findLabel)(struct Interface_* THIS, CGIR_LAB cgir_lab);
  Symbol (*Interface_makeSymbol)(struct Interface_* THIS, CGIR_SYM cgir_sym,
                                 const char *name);
  void (*Interface_Symbol_setClasses)(struct Interface_* THIS, Symbol symbol,
                                      O64_SymbolClass sclass, O64_SymbolStore sstore,
                                      O64_SymbolExport sexport);
  Symbol (*Interface_findSymbol)(struct Interface_* THIS, CGIR_SYM cgir_sym);
  Temporary (*Interface_makeDedicatedTemporary)(struct Interface_* THIS, CGIR_TN cgir_tn,
                                                O64_Register registre);
  Temporary (*Interface_makeAssignedTemporary)(struct Interface_* THIS, CGIR_TN cgir_tn,
                                               O64_Register registre);
  Temporary (*Interface_makeVirtualTemporary)(struct Interface_* THIS, CGIR_TN cgir_tn,
                                              O64_RegFile regFile);
  Temporary (*Interface_makeAbsoluteTemporary)(struct Interface_* THIS, CGIR_TN cgir_tn,
                                               O64_Immediate immediate, int64_t value);
  Temporary (*Interface_makeSymbolTemporary)(struct Interface_* THIS, CGIR_TN cgir_tn,
                                             O64_Immediate immediate, Symbol symbol,
                                             int64_t offset);
  Temporary (*Interface_makeLabelTemporary)(struct Interface_* THIS, CGIR_TN cgir_tn,
                                            O64_Immediate immediate, Label label);
  void (*Interface_Temporary_setWidth)(struct Interface_* THIS, Temporary temporary,
                                       unsigned width);
  void (*Interface_Temporary_setRemater)(struct Interface_* THIS, Temporary temporary,
                                         Temporary value);
  void (*Interface_Temporary_setHomeable)(struct Interface_* THIS, Temporary temporary,
                                          Temporary location);
  void (*Interface_Temporary_setDedicated)(struct Interface_* THIS, Temporary temporary);
  Temporary (*Interface_findTemporary)(struct Interface_* THIS, CGIR_TN cgir_tn);
  uint32_t (*Interface_Temporary_identity)(Temporary temporary);
  int (*Interface_Temporary_isAbsolute)(Temporary temporary);
  int64_t (*Interface_Temporary_value)(Temporary temporary);
  int (*Interface_Temporary_isSymbol)(Temporary temporary);
  Symbol (*Interface_Temporary_symbol)(Temporary temporary);
  int64_t (*Interface_Temporary_offset)(Temporary temporary);
  O64_Immediate (*Interface_Temporary_invariant)(Temporary temporary);
  int (*Interface_Temporary_isVirtual)(Temporary temporary);
  int (*Interface_Temporary_isDedicated)(Temporary temporary);
  int (*Interface_Temporary_isAssigned)(Temporary temporary);
  O64_Register (*Interface_Temporary_register)(Temporary temporary);
  O64_RegFile (*Interface_Temporary_regFile)(Temporary temporary);
  uint32_t (*Interface_Symbol_identity)(Symbol symbol);
  const char * (*Interface_Symbol_name)(Symbol symbol);
  int (*Interface_Symbol_isSpill)(Symbol symbol);
  O64_NativeType (*Interface_Symbol_nativeType)(Symbol symbol);
  uint32_t (*Interface_Operation_identity)(Operation operation);
  O64_Operator (*Interface_Operation_operator)(Operation operation);
  int (*Interface_Operation_iteration)(Operation operation);
  int (*Interface_Operation_issueDate)(Operation operation);
  int (*Interface_Operation_isSpillCode)(Operation operation);
  int (*Interface_Operation_isVolatile)(Operation operation);
  int (*Interface_Operation_isHoisted)(Operation operation);
  uint32_t (*Interface_Label_identity)(Label label);
  const char * (*Interface_Label_name)(Label label);
  uint32_t (*Interface_BasicBlock_identity)(BasicBlock basicBlock);
  int (*Interface_BasicBlock_unrolled)(BasicBlock basicBlock);
  intptr_t (*Interface_BasicBlock_traceId)(BasicBlock basicBlock);
  uint32_t (*Interface_LoopScope_identity)(LoopScope loopScope);
  int (*Interface_LoopScope_unrolled)(LoopScope loopScope);
  Operation (*Interface_makeOperation)(struct Interface_* THIS,
      CGIR_OP cgir_op, O64_Operator operator, int iteration,
      int argCount, Temporary arguments[], int resCount,
      Temporary results[], int regCount, int registers[]);
  Operation (*Interface_findOperation)(struct Interface_* THIS, CGIR_OP cgir_op);
  void (*Interface_Operation_setSafeAccess)(struct Interface_* THIS, Operation operation);
  void (*Interface_Operation_setHoisted)(struct Interface_* THIS, Operation operation);
  void (*Interface_Operation_setVolatile)(struct Interface_* THIS, Operation operation);
  void (*Interface_Operation_setPrefetch)(struct Interface_* THIS, Operation operation);
  void (*Interface_Operation_setPreload)(struct Interface_* THIS, Operation operation);
  void (*Interface_Operation_setBarrier)(struct Interface_* THIS, Operation operation);
  void (*Interface_Operation_setSafePerfs)(struct Interface_* THIS, Operation operation);
  void (*Interface_Operation_setSpillCode)(struct Interface_* THIS, Operation operation,
                                           Symbol symbol);
  BasicBlock (*Interface_makeBasicBlock)(struct Interface_* THIS, CGIR_BB cgir_bb,
                                         O64_Processor processor, int unrolled,
                                         int labelCount, Label labels[],
                                         int operationCount, Operation operations[],
                                         intptr_t regionId, float frequency);
  BasicBlock (*Interface_findBasicBlock)(struct Interface_* THIS, CGIR_BB cgir_bb);
  void (*Interface_linkBasicBlocks)(struct Interface_* THIS,
      BasicBlock tail_block, BasicBlock head_block, float probability);
  LoopScope (*Interface_makeLoopScope)(struct Interface_* THIS, CGIR_LD cgir_ld,
                                     BasicBlock basicBlock, Temporary temporary,
                                     O64_ConfigureItem item, ...);
  LoopScope (*Interface_findLoopScope)(struct Interface_* THIS, CGIR_LD cgir_ld);
  void (*Interface_LoopScope_setDependenceNode)(struct Interface_* THIS,
      LoopScope loopScope, Operation operation, O64_DependenceNode node);
  void (*Interface_LoopScope_setDependenceArc)(struct Interface_* THIS,
      LoopScope loopScope, Operation tail_operation, Operation head_operation,
      int latency, int omega, O64_DependenceKind type);
  void (*Interface_setBody)(struct Interface_* THIS, BasicBlock basicBlock);
  void (*Interface_setEntry)(struct Interface_* THIS, BasicBlock basicBlock);
  void (*Interface_setExit)(struct Interface_* THIS, BasicBlock basicBlock);
  void (*Interface_setStart)(struct Interface_* THIS, BasicBlock basicBlock);
  void (*Interface_updateCGIR)(struct Interface_* THIS, CGIR_CallBack callback);
  void (*Interface_open)(struct Interface_* THIS, const char *name,
                         O64_ConfigureItem item, ...);
  unsigned (*Interface_optimize)(struct Interface_* THIS, O64_OptimizeItem item, ...);
  void (*Interface_close)(struct Interface_* THIS);
};
typedef struct O64_Interface_ O64_Interface_, *O64_Interface;
typedef const struct O64_Interface_ *const_O64_Interface;
typedef struct O64_Interface_ * restrict_O64_Interface;

#define O64_Interface_size(THIS) ((THIS)->size)
#define O64_Interface__size(THIS) (&(THIS)->size)
#define O64_Interface_Interface_Initialize(THIS) ((THIS)->Interface_Initialize)
#define O64_Interface__Interface_Initialize(THIS) (&(THIS)->Interface_Initialize)
#define O64_Interface_Interface_Finalize(THIS) ((THIS)->Interface_Finalize)
#define O64_Interface__Interface_Finalize(THIS) (&(THIS)->Interface_Finalize)
#define O64_Interface_Interface_getInstance(THIS) ((THIS)->Interface_getInstance)
#define O64_Interface__Interface_getInstance(THIS) (&(THIS)->Interface_getInstance)
#define O64_Interface_Interface_setMaxIssue(THIS) ((THIS)->Interface_setMaxIssue)
#define O64_Interface__Interface_setMaxIssue(THIS) (&(THIS)->Interface_setMaxIssue)
#define O64_Interface_Interface_setMinTaken(THIS) ((THIS)->Interface_setMinTaken)
#define O64_Interface__Interface_setMinTaken(THIS) (&(THIS)->Interface_setMinTaken)
#define O64_Interface_Interface_setReserved(THIS) ((THIS)->Interface_setReserved)
#define O64_Interface__Interface_setReserved(THIS) (&(THIS)->Interface_setReserved)
#define O64_Interface_Interface_setArgStage(THIS) ((THIS)->Interface_setArgStage)
#define O64_Interface__Interface_setArgStage(THIS) (&(THIS)->Interface_setArgStage)
#define O64_Interface_Interface_setResStage(THIS) ((THIS)->Interface_setResStage)
#define O64_Interface__Interface_setResStage(THIS) (&(THIS)->Interface_setResStage)
#define O64_Interface_Interface_makeLabel(THIS) ((THIS)->Interface_makeLabel)
#define O64_Interface__Interface_makeLabel(THIS) (&(THIS)->Interface_makeLabel)
#define O64_Interface_Interface_findLabel(THIS) ((THIS)->Interface_findLabel)
#define O64_Interface__Interface_findLabel(THIS) (&(THIS)->Interface_findLabel)
#define O64_Interface_Interface_makeSymbol(THIS) ((THIS)->Interface_makeSymbol)
#define O64_Interface__Interface_makeSymbol(THIS) (&(THIS)->Interface_makeSymbol)
#define O64_Interface_Interface_Symbol_setClasses(THIS) ((THIS)->Interface_Symbol_setClasses)
#define O64_Interface__Interface_Symbol_setClasses(THIS) (&(THIS)->Interface_Symbol_setClasses)
#define O64_Interface_Interface_findSymbol(THIS) ((THIS)->Interface_findSymbol)
#define O64_Interface__Interface_findSymbol(THIS) (&(THIS)->Interface_findSymbol)
#define O64_Interface_Interface_makeDedicatedTemporary(THIS) ((THIS)->Interface_makeDedicatedTemporary)
#define O64_Interface__Interface_makeDedicatedTemporary(THIS) (&(THIS)->Interface_makeDedicatedTemporary)
#define O64_Interface_Interface_makeAssignedTemporary(THIS) ((THIS)->Interface_makeAssignedTemporary)
#define O64_Interface__Interface_makeAssignedTemporary(THIS) (&(THIS)->Interface_makeAssignedTemporary)
#define O64_Interface_Interface_makeVirtualTemporary(THIS) ((THIS)->Interface_makeVirtualTemporary)
#define O64_Interface__Interface_makeVirtualTemporary(THIS) (&(THIS)->Interface_makeVirtualTemporary)
#define O64_Interface_Interface_makeAbsoluteTemporary(THIS) ((THIS)->Interface_makeAbsoluteTemporary)
#define O64_Interface__Interface_makeAbsoluteTemporary(THIS) (&(THIS)->Interface_makeAbsoluteTemporary)
#define O64_Interface_Interface_makeSymbolTemporary(THIS) ((THIS)->Interface_makeSymbolTemporary)
#define O64_Interface__Interface_makeSymbolTemporary(THIS) (&(THIS)->Interface_makeSymbolTemporary)
#define O64_Interface_Interface_makeLabelTemporary(THIS) ((THIS)->Interface_makeLabelTemporary)
#define O64_Interface__Interface_makeLabelTemporary(THIS) (&(THIS)->Interface_makeLabelTemporary)
#define O64_Interface_Interface_Temporary_setWidth(THIS) ((THIS)->Interface_Temporary_setWidth)
#define O64_Interface__Interface_Temporary_setWidth(THIS) (&(THIS)->Interface_Temporary_setWidth)
#define O64_Interface_Interface_Temporary_setRemater(THIS) ((THIS)->Interface_Temporary_setRemater)
#define O64_Interface__Interface_Temporary_setRemater(THIS) (&(THIS)->Interface_Temporary_setRemater)
#define O64_Interface_Interface_Temporary_setHomeable(THIS) ((THIS)->Interface_Temporary_setHomeable)
#define O64_Interface__Interface_Temporary_setHomeable(THIS) (&(THIS)->Interface_Temporary_setHomeable)
#define O64_Interface_Interface_Temporary_setDedicated(THIS) ((THIS)->Interface_Temporary_setDedicated)
#define O64_Interface__Interface_Temporary_setDedicated(THIS) (&(THIS)->Interface_Temporary_setDedicated)
#define O64_Interface_Interface_findTemporary(THIS) ((THIS)->Interface_findTemporary)
#define O64_Interface__Interface_findTemporary(THIS) (&(THIS)->Interface_findTemporary)
#define O64_Interface_Interface_Temporary_identity(THIS) ((THIS)->Interface_Temporary_identity)
#define O64_Interface__Interface_Temporary_identity(THIS) (&(THIS)->Interface_Temporary_identity)
#define O64_Interface_Interface_Temporary_isAbsolute(THIS) ((THIS)->Interface_Temporary_isAbsolute)
#define O64_Interface__Interface_Temporary_isAbsolute(THIS) (&(THIS)->Interface_Temporary_isAbsolute)
#define O64_Interface_Interface_Temporary_value(THIS) ((THIS)->Interface_Temporary_value)
#define O64_Interface__Interface_Temporary_value(THIS) (&(THIS)->Interface_Temporary_value)
#define O64_Interface_Interface_Temporary_isSymbol(THIS) ((THIS)->Interface_Temporary_isSymbol)
#define O64_Interface__Interface_Temporary_isSymbol(THIS) (&(THIS)->Interface_Temporary_isSymbol)
#define O64_Interface_Interface_Temporary_symbol(THIS) ((THIS)->Interface_Temporary_symbol)
#define O64_Interface__Interface_Temporary_symbol(THIS) (&(THIS)->Interface_Temporary_symbol)
#define O64_Interface_Interface_Temporary_offset(THIS) ((THIS)->Interface_Temporary_offset)
#define O64_Interface__Interface_Temporary_offset(THIS) (&(THIS)->Interface_Temporary_offset)
#define O64_Interface_Interface_Temporary_invariant(THIS) ((THIS)->Interface_Temporary_invariant)
#define O64_Interface__Interface_Temporary_invariant(THIS) (&(THIS)->Interface_Temporary_invariant)
#define O64_Interface_Interface_Temporary_isVirtual(THIS) ((THIS)->Interface_Temporary_isVirtual)
#define O64_Interface__Interface_Temporary_isVirtual(THIS) (&(THIS)->Interface_Temporary_isVirtual)
#define O64_Interface_Interface_Temporary_isDedicated(THIS) ((THIS)->Interface_Temporary_isDedicated)
#define O64_Interface__Interface_Temporary_isDedicated(THIS) (&(THIS)->Interface_Temporary_isDedicated)
#define O64_Interface_Interface_Temporary_isAssigned(THIS) ((THIS)->Interface_Temporary_isAssigned)
#define O64_Interface__Interface_Temporary_isAssigned(THIS) (&(THIS)->Interface_Temporary_isAssigned)
#define O64_Interface_Interface_Temporary_register(THIS) ((THIS)->Interface_Temporary_register)
#define O64_Interface__Interface_Temporary_register(THIS) (&(THIS)->Interface_Temporary_register)
#define O64_Interface_Interface_Temporary_regFile(THIS) ((THIS)->Interface_Temporary_regFile)
#define O64_Interface__Interface_Temporary_regFile(THIS) (&(THIS)->Interface_Temporary_regFile)
#define O64_Interface_Interface_Symbol_identity(THIS) ((THIS)->Interface_Symbol_identity)
#define O64_Interface__Interface_Symbol_identity(THIS) (&(THIS)->Interface_Symbol_identity)
#define O64_Interface_Interface_Symbol_name(THIS) ((THIS)->Interface_Symbol_name)
#define O64_Interface__Interface_Symbol_name(THIS) (&(THIS)->Interface_Symbol_name)
#define O64_Interface_Interface_Symbol_isSpill(THIS) ((THIS)->Interface_Symbol_isSpill)
#define O64_Interface__Interface_Symbol_isSpill(THIS) (&(THIS)->Interface_Symbol_isSpill)
#define O64_Interface_Interface_Symbol_nativeType(THIS) ((THIS)->Interface_Symbol_nativeType)
#define O64_Interface__Interface_Symbol_nativeType(THIS) (&(THIS)->Interface_Symbol_nativeType)
#define O64_Interface_Interface_Operation_identity(THIS) ((THIS)->Interface_Operation_identity)
#define O64_Interface__Interface_Operation_identity(THIS) (&(THIS)->Interface_Operation_identity)
#define O64_Interface_Interface_Operation_operator(THIS) ((THIS)->Interface_Operation_operator)
#define O64_Interface__Interface_Operation_operator(THIS) (&(THIS)->Interface_Operation_operator)
#define O64_Interface_Interface_Operation_iteration(THIS) ((THIS)->Interface_Operation_iteration)
#define O64_Interface__Interface_Operation_iteration(THIS) (&(THIS)->Interface_Operation_iteration)
#define O64_Interface_Interface_Operation_issueDate(THIS) ((THIS)->Interface_Operation_issueDate)
#define O64_Interface__Interface_Operation_issueDate(THIS) (&(THIS)->Interface_Operation_issueDate)
#define O64_Interface_Interface_Operation_isSpillCode(THIS) ((THIS)->Interface_Operation_isSpillCode)
#define O64_Interface__Interface_Operation_isSpillCode(THIS) (&(THIS)->Interface_Operation_isSpillCode)
#define O64_Interface_Interface_Operation_isVolatile(THIS) ((THIS)->Interface_Operation_isVolatile)
#define O64_Interface__Interface_Operation_isVolatile(THIS) (&(THIS)->Interface_Operation_isVolatile)
#define O64_Interface_Interface_Operation_isHoisted(THIS) ((THIS)->Interface_Operation_isHoisted)
#define O64_Interface__Interface_Operation_isHoisted(THIS) (&(THIS)->Interface_Operation_isHoisted)
#define O64_Interface_Interface_Label_identity(THIS) ((THIS)->Interface_Label_identity)
#define O64_Interface__Interface_Label_identity(THIS) (&(THIS)->Interface_Label_identity)
#define O64_Interface_Interface_Label_name(THIS) ((THIS)->Interface_Label_name)
#define O64_Interface__Interface_Label_name(THIS) (&(THIS)->Interface_Label_name)
#define O64_Interface_Interface_BasicBlock_identity(THIS) ((THIS)->Interface_BasicBlock_identity)
#define O64_Interface__Interface_BasicBlock_identity(THIS) (&(THIS)->Interface_BasicBlock_identity)
#define O64_Interface_Interface_BasicBlock_unrolled(THIS) ((THIS)->Interface_BasicBlock_unrolled)
#define O64_Interface__Interface_BasicBlock_unrolled(THIS) (&(THIS)->Interface_BasicBlock_unrolled)
#define O64_Interface_Interface_BasicBlock_traceId(THIS) ((THIS)->Interface_BasicBlock_traceId)
#define O64_Interface__Interface_BasicBlock_traceId(THIS) (&(THIS)->Interface_BasicBlock_traceId)
#define O64_Interface_Interface_LoopScope_identity(THIS) ((THIS)->Interface_LoopScope_identity)
#define O64_Interface__Interface_LoopScope_identity(THIS) (&(THIS)->Interface_LoopScope_identity)
#define O64_Interface_Interface_LoopScope_unrolled(THIS) ((THIS)->Interface_LoopScope_unrolled)
#define O64_Interface__Interface_LoopScope_unrolled(THIS) (&(THIS)->Interface_LoopScope_unrolled)
#define O64_Interface_Interface_makeOperation(THIS) ((THIS)->Interface_makeOperation)
#define O64_Interface__Interface_makeOperation(THIS) (&(THIS)->Interface_makeOperation)
#define O64_Interface_Interface_findOperation(THIS) ((THIS)->Interface_findOperation)
#define O64_Interface__Interface_findOperation(THIS) (&(THIS)->Interface_findOperation)
#define O64_Interface_Interface_Operation_setSafeAccess(THIS) ((THIS)->Interface_Operation_setSafeAccess)
#define O64_Interface__Interface_Operation_setSafeAccess(THIS) (&(THIS)->Interface_Operation_setSafeAccess)
#define O64_Interface_Interface_Operation_setHoisted(THIS) ((THIS)->Interface_Operation_setHoisted)
#define O64_Interface__Interface_Operation_setHoisted(THIS) (&(THIS)->Interface_Operation_setHoisted)
#define O64_Interface_Interface_Operation_setVolatile(THIS) ((THIS)->Interface_Operation_setVolatile)
#define O64_Interface__Interface_Operation_setVolatile(THIS) (&(THIS)->Interface_Operation_setVolatile)
#define O64_Interface_Interface_Operation_setPrefetch(THIS) ((THIS)->Interface_Operation_setPrefetch)
#define O64_Interface__Interface_Operation_setPrefetch(THIS) (&(THIS)->Interface_Operation_setPrefetch)
#define O64_Interface_Interface_Operation_setPreload(THIS) ((THIS)->Interface_Operation_setPreload)
#define O64_Interface__Interface_Operation_setPreload(THIS) (&(THIS)->Interface_Operation_setPreload)
#define O64_Interface_Interface_Operation_setBarrier(THIS) ((THIS)->Interface_Operation_setBarrier)
#define O64_Interface__Interface_Operation_setBarrier(THIS) (&(THIS)->Interface_Operation_setBarrier)
#define O64_Interface_Interface_Operation_setSafePerfs(THIS) ((THIS)->Interface_Operation_setSafePerfs)
#define O64_Interface__Interface_Operation_setSafePerfs(THIS) (&(THIS)->Interface_Operation_setSafePerfs)
#define O64_Interface_Interface_Operation_setSpillCode(THIS) ((THIS)->Interface_Operation_setSpillCode)
#define O64_Interface__Interface_Operation_setSpillCode(THIS) (&(THIS)->Interface_Operation_setSpillCode)
#define O64_Interface_Interface_makeBasicBlock(THIS) ((THIS)->Interface_makeBasicBlock)
#define O64_Interface__Interface_makeBasicBlock(THIS) (&(THIS)->Interface_makeBasicBlock)
#define O64_Interface_Interface_findBasicBlock(THIS) ((THIS)->Interface_findBasicBlock)
#define O64_Interface__Interface_findBasicBlock(THIS) (&(THIS)->Interface_findBasicBlock)
#define O64_Interface_Interface_linkBasicBlocks(THIS) ((THIS)->Interface_linkBasicBlocks)
#define O64_Interface__Interface_linkBasicBlocks(THIS) (&(THIS)->Interface_linkBasicBlocks)
#define O64_Interface_Interface_makeLoopScope(THIS) ((THIS)->Interface_makeLoopScope)
#define O64_Interface__Interface_makeLoopScope(THIS) (&(THIS)->Interface_makeLoopScope)
#define O64_Interface_Interface_findLoopScope(THIS) ((THIS)->Interface_findLoopScope)
#define O64_Interface__Interface_findLoopScope(THIS) (&(THIS)->Interface_findLoopScope)
#define O64_Interface_Interface_LoopScope_setDependenceNode(THIS) ((THIS)->Interface_LoopScope_setDependenceNode)
#define O64_Interface__Interface_LoopScope_setDependenceNode(THIS) (&(THIS)->Interface_LoopScope_setDependenceNode)
#define O64_Interface_Interface_LoopScope_setDependenceArc(THIS) ((THIS)->Interface_LoopScope_setDependenceArc)
#define O64_Interface__Interface_LoopScope_setDependenceArc(THIS) (&(THIS)->Interface_LoopScope_setDependenceArc)
#define O64_Interface_Interface_setBody(THIS) ((THIS)->Interface_setBody)
#define O64_Interface__Interface_setBody(THIS) (&(THIS)->Interface_setBody)
#define O64_Interface_Interface_setEntry(THIS) ((THIS)->Interface_setEntry)
#define O64_Interface__Interface_setEntry(THIS) (&(THIS)->Interface_setEntry)
#define O64_Interface_Interface_setExit(THIS) ((THIS)->Interface_setExit)
#define O64_Interface__Interface_setExit(THIS) (&(THIS)->Interface_setExit)
#define O64_Interface_Interface_setStart(THIS) ((THIS)->Interface_setStart)
#define O64_Interface__Interface_setStart(THIS) (&(THIS)->Interface_setStart)
#define O64_Interface_Interface_updateCGIR(THIS) ((THIS)->Interface_updateCGIR)
#define O64_Interface__Interface_updateCGIR(THIS) (&(THIS)->Interface_updateCGIR)
#define O64_Interface_Interface_open(THIS) ((THIS)->Interface_open)
#define O64_Interface__Interface_open(THIS) (&(THIS)->Interface_open)
#define O64_Interface_Interface_optimize(THIS) ((THIS)->Interface_optimize)
#define O64_Interface__Interface_optimize(THIS) (&(THIS)->Interface_optimize)
#define O64_Interface_Interface_close(THIS) ((THIS)->Interface_close)
#define O64_Interface__Interface_close(THIS) (&(THIS)->Interface_close)


/*
 * O64_getInstance --	The singleton O64_Interface instance.
 */
O64_Interface
O64_getInstance(void);

/*
 * Accessors to the O64_Interface functions.
 * O64_instance must be an O64_Interface object.
 */
#define O64_Interface_Initialize (*O64_instance->Interface_Initialize)
#define O64_Interface_Finalize (*O64_instance->Interface_Finalize)
#define O64_Interface_getInstance (*O64_instance->Interface_getInstance)
#define O64_Interface_setMaxIssue (*O64_instance->Interface_setMaxIssue)
#define O64_Interface_setMinTaken (*O64_instance->Interface_setMinTaken)
#define O64_Interface_setReserved (*O64_instance->Interface_setReserved)
#define O64_Interface_setArgStage (*O64_instance->Interface_setArgStage)
#define O64_Interface_setResStage (*O64_instance->Interface_setResStage)
#define O64_Interface_makeLabel (*O64_instance->Interface_makeLabel)
#define O64_Interface_findLabel (*O64_instance->Interface_findLabel)
#define O64_Interface_makeSymbol (*O64_instance->Interface_makeSymbol)
#define O64_Interface_Symbol_setClasses (*O64_instance->Interface_Symbol_setClasses)
#define O64_Interface_findSymbol (*O64_instance->Interface_findSymbol)
#define O64_Interface_makeDedicatedTemporary (*O64_instance->Interface_makeDedicatedTemporary)
#define O64_Interface_makeAssignedTemporary (*O64_instance->Interface_makeAssignedTemporary)
#define O64_Interface_makeVirtualTemporary (*O64_instance->Interface_makeVirtualTemporary)
#define O64_Interface_makeAbsoluteTemporary (*O64_instance->Interface_makeAbsoluteTemporary)
#define O64_Interface_makeSymbolTemporary (*O64_instance->Interface_makeSymbolTemporary)
#define O64_Interface_makeLabelTemporary (*O64_instance->Interface_makeLabelTemporary)
#define O64_Interface_Temporary_setWidth (*O64_instance->Interface_Temporary_setWidth)
#define O64_Interface_Temporary_setRemater (*O64_instance->Interface_Temporary_setRemater)
#define O64_Interface_Temporary_setHomeable (*O64_instance->Interface_Temporary_setHomeable)
#define O64_Interface_Temporary_setDedicated (*O64_instance->Interface_Temporary_setDedicated)
#define O64_Interface_findTemporary (*O64_instance->Interface_findTemporary)
#define O64_Interface_Temporary_identity (*O64_instance->Interface_Temporary_identity)
#define O64_Interface_Temporary_isAbsolute (*O64_instance->Interface_Temporary_isAbsolute)
#define O64_Interface_Temporary_value (*O64_instance->Interface_Temporary_value)
#define O64_Interface_Temporary_isSymbol (*O64_instance->Interface_Temporary_isSymbol)
#define O64_Interface_Temporary_symbol (*O64_instance->Interface_Temporary_symbol)
#define O64_Interface_Temporary_offset (*O64_instance->Interface_Temporary_offset)
#define O64_Interface_Temporary_invariant (*O64_instance->Interface_Temporary_invariant)
#define O64_Interface_Temporary_isVirtual (*O64_instance->Interface_Temporary_isVirtual)
#define O64_Interface_Temporary_isDedicated (*O64_instance->Interface_Temporary_isDedicated)
#define O64_Interface_Temporary_isAssigned (*O64_instance->Interface_Temporary_isAssigned)
#define O64_Interface_Temporary_register (*O64_instance->Interface_Temporary_register)
#define O64_Interface_Temporary_regFile (*O64_instance->Interface_Temporary_regFile)
#define O64_Interface_Symbol_identity (*O64_instance->Interface_Symbol_identity)
#define O64_Interface_Symbol_name (*O64_instance->Interface_Symbol_name)
#define O64_Interface_Symbol_isSpill (*O64_instance->Interface_Symbol_isSpill)
#define O64_Interface_Symbol_nativeType (*O64_instance->Interface_Symbol_nativeType)
#define O64_Interface_Operation_identity (*O64_instance->Interface_Operation_identity)
#define O64_Interface_Operation_operator (*O64_instance->Interface_Operation_operator)
#define O64_Interface_Operation_iteration (*O64_instance->Interface_Operation_iteration)
#define O64_Interface_Operation_issueDate (*O64_instance->Interface_Operation_issueDate)
#define O64_Interface_Operation_isSpillCode (*O64_instance->Interface_Operation_isSpillCode)
#define O64_Interface_Operation_isVolatile (*O64_instance->Interface_Operation_isVolatile)
#define O64_Interface_Operation_isHoisted (*O64_instance->Interface_Operation_isHoisted)
#define O64_Interface_Label_identity (*O64_instance->Interface_Label_identity)
#define O64_Interface_Label_name (*O64_instance->Interface_Label_name)
#define O64_Interface_BasicBlock_identity (*O64_instance->Interface_BasicBlock_identity)
#define O64_Interface_BasicBlock_unrolled (*O64_instance->Interface_BasicBlock_unrolled)
#define O64_Interface_BasicBlock_traceId (*O64_instance->Interface_BasicBlock_traceId)
#define O64_Interface_LoopScope_identity (*O64_instance->Interface_LoopScope_identity)
#define O64_Interface_LoopScope_unrolled (*O64_instance->Interface_LoopScope_unrolled)
#define O64_Interface_makeOperation (*O64_instance->Interface_makeOperation)
#define O64_Interface_findOperation (*O64_instance->Interface_findOperation)
#define O64_Interface_Operation_setSafeAccess (*O64_instance->Interface_Operation_setSafeAccess)
#define O64_Interface_Operation_setHoisted (*O64_instance->Interface_Operation_setHoisted)
#define O64_Interface_Operation_setVolatile (*O64_instance->Interface_Operation_setVolatile)
#define O64_Interface_Operation_setPrefetch (*O64_instance->Interface_Operation_setPrefetch)
#define O64_Interface_Operation_setPreload (*O64_instance->Interface_Operation_setPreload)
#define O64_Interface_Operation_setBarrier (*O64_instance->Interface_Operation_setBarrier)
#define O64_Interface_Operation_setSafePerfs (*O64_instance->Interface_Operation_setSafePerfs)
#define O64_Interface_Operation_setSpillCode (*O64_instance->Interface_Operation_setSpillCode)
#define O64_Interface_makeBasicBlock (*O64_instance->Interface_makeBasicBlock)
#define O64_Interface_findBasicBlock (*O64_instance->Interface_findBasicBlock)
#define O64_Interface_linkBasicBlocks (*O64_instance->Interface_linkBasicBlocks)
#define O64_Interface_makeLoopScope (*O64_instance->Interface_makeLoopScope)
#define O64_Interface_findLoopScope (*O64_instance->Interface_findLoopScope)
#define O64_Interface_LoopScope_setDependenceNode (*O64_instance->Interface_LoopScope_setDependenceNode)
#define O64_Interface_LoopScope_setDependenceArc (*O64_instance->Interface_LoopScope_setDependenceArc)
#define O64_Interface_setBody (*O64_instance->Interface_setBody)
#define O64_Interface_setEntry (*O64_instance->Interface_setEntry)
#define O64_Interface_setExit (*O64_instance->Interface_setExit)
#define O64_Interface_setStart (*O64_instance->Interface_setStart)
#define O64_Interface_updateCGIR (*O64_instance->Interface_updateCGIR)
#define O64_Interface_open (*O64_instance->Interface_open)
#define O64_Interface_optimize (*O64_instance->Interface_optimize)
#define O64_Interface_close (*O64_instance->Interface_close)




#define Interface_h_INCLUDED 

/*
 * !!!!	Interface.xcc
 *
 * Benoit Dupont de Dinechin (Benoit.Dupont-de-Dinechin@st.com).
 * Francois de Ferriere (Francois.de-Ferriere@st.com).
 * Christophe Guillon (Christophe.Guillon@st.com).
 *
 * Copyright 2002 - 2007 STMicroelectronics.
 * Copyright 1995 - 1998 Commissariat a l'Energie Atomique.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of either (at your option): the GNU
 * General Public License (GPL) version 2; the GNU Lesser General
 * Public License (LGPL) version 2.1; any later version of these
 * licences as published by the Free Software Foundation.
 */

//
typedef void *InterfaceCGIR;
typedef void *LIR2CGIR;

/*
  InterfaceType -- Enumerate the Interface types in reverse mapping.
 */
typedef enum {
  InterfaceType_CGIR,
  InterfaceType_Label,
  InterfaceType_Symbol,
  InterfaceType_Temporary,
  InterfaceType_Operation,
  InterfaceType_BasicBlock,
  InterfaceType_LoopScope,
  InterfaceType_CGIRImport,
  InterfaceType_CGIRValid,
  InterfaceType__
} InterfaceType;
typedef uint8_t short_InterfaceType;

/*
 * InterfaceMapped --	Mapped value in the LIR2CGIR HTables.
 */
struct InterfaceMapped_ {
  //@args	void *cgir, InterfaceType type
  InterfaceCGIR CGIR; // The CGIR object mapped to.
  short_InterfaceType TYPE; // The LIR key object type.
};
typedef struct InterfaceMapped_ InterfaceMapped_, *InterfaceMapped;
typedef const struct InterfaceMapped_ *const_InterfaceMapped;
typedef struct InterfaceMapped_ * restrict_InterfaceMapped;

extern InterfaceMapped
InterfaceMapped_Ctor(InterfaceMapped THIS, void *cgir, InterfaceType type);

extern InterfaceMapped
InterfaceMapped_Copy(InterfaceMapped THIS, const_InterfaceMapped that);

extern void
InterfaceMapped_Dtor(InterfaceMapped THIS);

extern size_t
InterfaceMapped_Size(void *cgir, InterfaceType type);

#define InterfaceMapped_CGIR(THIS) ((THIS)->CGIR)
#define InterfaceMapped__CGIR(THIS) (&(THIS)->CGIR)
#define InterfaceMapped_TYPE(THIS) ((THIS)->TYPE)
#define InterfaceMapped__TYPE(THIS) (&(THIS)->TYPE)


/*
 * Interface --	Interface between the LAO LIR and a host compiler CGIR.
 *
 * The purpose of the interface is to maintain mappings between LIR Labels,
 * Symbols, Temporaries, Operations, BasicBlocks, LoopScopes, and the corresponding
 * host compiler CGIR (Code Generator Intermediate Representation) objects.
 *
 * There must not be more than one CGIR object mapped to the same LIR object
 * to ensure that all CGIR objects are correctly updated. This constraint
 * apply to all CGIR->LIR object tables.
 * However there may be more than one LIR object mapped to the same CGIR
 * object. So factorization of the objects in the CGIR side is allowed.
 * These duplicated LIR objects however should not be mutable as in THIS
 * case information from the LIR side may not be passed back to the CGIR
 * side. So the LIR->CGIR reversed tables may have several keys pointing
 * to the same CGIR object. In THIS case only one update is performed on
 * the CGIR object.
 */
struct Interface_;
typedef struct Interface_ Interface_, *Interface;
typedef const struct Interface_ *const_Interface;
typedef struct Interface_ * restrict_Interface;

/*
 * Interface_getInstance --	Returns the singleton Interface, for use by the LAO clients.
 */
Interface
Interface_getInstance(void);

//
int *Interface_Registers(int count, ...);

//
Label *
Interface_Labels(int count, ...);

//
Temporary *
Interface_Temporaries(int count, ...);

//
Operation *
Interface_Operations(int count, ...);

//
/*--------------------- LAO Interface Input Functions ------------------------*/

/*
 * Interface_setMaxIssue --	Set the processor max issue (override MDS information).
 */
void
Interface_setMaxIssue(Interface THIS, O64_Processor processor, int maxIssue);

/*
 * Interface_setMinTaken --	Set the processor min taken (override MDS information).
 */
void
Interface_setMinTaken(Interface THIS, O64_Processor processor, int minTaken);

/*
 * Interface_setReserved --	Set a register as reserved (override MDS information).
 */
void
Interface_setReserved(Interface THIS, O64_Convention convention, O64_Register reserved);

/*
 * Interface_setArgStage --	Set the argument read stage (override MDS information).
 */
bool
Interface_setArgStage(Interface THIS, O64_Processor processor,
                      O64_Operator operator, int index, int stage);

/*
 * Interface_setResStage --	Set the result write stage (override MDS information).
 */
bool
Interface_setResStage(Interface THIS, O64_Processor processor,
                      O64_Operator operator, int index, int stage);

/*
 * Interface_makeLabel --	Make a Label.
 */
Label
Interface_makeLabel(Interface THIS, CGIR_LAB cgir_lab, const char *name);

/*
 * Interface_findLabel --	Find a Label.
 */
Label
Interface_findLabel(Interface THIS, CGIR_LAB cgir_lab);

/*
 * Interface_makeSymbol --	Make a Symbol.
 */
Symbol
Interface_makeSymbol(Interface THIS, CGIR_SYM cgir_sym, const char *name);

/*
 * Interface_Symbol_setClasses --	Set the (class,store,export) for a Symbol.
 */
void
Interface_Symbol_setClasses(Interface THIS, Symbol symbol, O64_SymbolClass sclass, O64_SymbolStore sstore, O64_SymbolExport sexport);

/*
 * Interface_findSymbol --	Find a Symbol.
 */
Symbol
Interface_findSymbol(Interface THIS, CGIR_SYM cgir_sym);

/*
 * Interface_makeDedicatedTemporary --	Make a Dedicated Temporary.
 */
Temporary
Interface_makeDedicatedTemporary(Interface THIS, CGIR_TN cgir_tn, O64_Register cgir_register);

/*
 * Interface_makeAssignedTemporary --	Make a Assigned Temporary.
 */
Temporary
Interface_makeAssignedTemporary(Interface THIS, CGIR_TN cgir_tn, O64_Register cgir_register);

/*
 * Interface_makeVirtualTemporary --	Make a Virtual Temporary.
 */
Temporary
Interface_makeVirtualTemporary(Interface THIS, CGIR_TN cgir_tn, O64_RegFile regFile);

/*
 * Interface_makeAbsoluteTemporary --	Make an Absolute Temporary.
 */
Temporary
Interface_makeAbsoluteTemporary(Interface THIS, CGIR_TN cgir_tn, O64_Immediate immediate, int64_t value);

/*
 * Interface_makeSymbolTemporary --	Make a Symbol Temporary.
 */
Temporary
Interface_makeSymbolTemporary(Interface THIS, CGIR_TN cgir_tn, O64_Immediate immediate, Symbol symbol, int64_t offset);

/*
 * Interface_makeLabelTemporary --	Make a Label Temporary.
 */
Temporary
Interface_makeLabelTemporary(Interface THIS, CGIR_TN cgir_tn, O64_Immediate immediate, Label label);

/*
 * Interface_Temporary_setWidth --	Set a Temporary bit-width.
 */
void
Interface_Temporary_setWidth(Interface THIS, Temporary temporary, unsigned width);

/*
 * Interface_Temporary_setRemater --	Set an Temporary as remat.
 */
void
Interface_Temporary_setRemater(Interface THIS, Temporary temporary, Temporary value);

/*
 * Interface_Temporary_setHomeable --	Set an Temporary as homeable to location.
 */
void
Interface_Temporary_setHomeable(Interface THIS, Temporary temporary, Temporary location);

/*
 * Interface_Temporary_setDedicated --	Set an Temporary as dedicated.
 */
void
Interface_Temporary_setDedicated(Interface THIS, Temporary temporary);

/*
 * Interface_findTemporary --	Find a Temporary.
 */
Temporary
Interface_findTemporary(Interface THIS, CGIR_TN cgir_tn);

//
/*--------------------- LAO Interface Accessors Functions ------------------------*/

/*
 * O64 Interface for queries on LAO Temporaries.
 */
uint32_t
Interface_Temporary_identity(Temporary temporary);
int
Interface_Temporary_isAbsolute(Temporary temporary);
int64_t
Interface_Temporary_value(Temporary temporary);
int
Interface_Temporary_isSymbol(Temporary temporary);
Symbol
Interface_Temporary_symbol(Temporary temporary);
int64_t
Interface_Temporary_offset(Temporary temporary);
O64_Immediate
Interface_Temporary_invariant(Temporary temporary);
int
Interface_Temporary_isVirtual(Temporary temporary);
int
Interface_Temporary_isDedicated(Temporary temporary);
int
Interface_Temporary_isAssigned(Temporary temporary);
O64_Register
Interface_Temporary_register(Temporary temporary);
O64_RegFile
Interface_Temporary_regFile(Temporary temporary);

/*
 * O64Interface for queries on LAO Symbols.
 */
uint32_t
Interface_Symbol_identity(Symbol symbol);
const char *
Interface_Symbol_name(Symbol symbol);
int
Interface_Symbol_isSpill(Symbol symbol);
O64_NativeType
Interface_Symbol_nativeType(Symbol symbol);

/*
 * O64 Interface for queries on LAO Operations.
 */
uint32_t
Interface_Operation_identity(Operation operation);
O64_Operator
Interface_Operation_operator(Operation operation);
int
Interface_Operation_iteration(Operation operation);
int
Interface_Operation_issueDate(Operation operation);
int
Interface_Operation_isSpillCode(Operation operation);
int
Interface_Operation_isVolatile(Operation operation);
int
Interface_Operation_isHoisted(Operation operation);

/*
 * O64 Interface for queries on LAO Labels.
 */
uint32_t
Interface_Label_identity(Label label);
const char *
Interface_Label_name(Label label);

/*
 * O64 Interface for queries on LAO BasicBlocks.
 */
uint32_t
Interface_BasicBlock_identity(BasicBlock basicBlock);
int
Interface_BasicBlock_unrolled(BasicBlock basicBlock);
intptr_t
Interface_BasicBlock_traceId(BasicBlock basicBlock);

/*
 * O64 Interface for queries on LAO LoopScopes.
 */
uint32_t
Interface_LoopScope_identity(LoopScope loopScope);
int
Interface_LoopScope_unrolled(LoopScope loopScope);

/*
 * Interface_makeOperation --	Make an Operation.
 */
Operation
Interface_makeOperation(Interface THIS, CGIR_OP cgir_op,
    O64_Operator operator, int iteration, int argCount, Temporary arguments[],
    int resCount, Temporary results[], int regCount, int registers[]);

/*
 * Interface_findOperation --	Find a Operation.
 */
Operation
Interface_findOperation(Interface THIS, CGIR_OP cgir_op);

/*
 * Interface_Operation_setSafeAccess --	Set an Operation as safe memory access.
 */
void
Interface_Operation_setSafeAccess(Interface THIS, Operation operation);

/*
 * Interface_Operation_setSafePerfs --	Set an Operation as safe performance effects.
 */
void
Interface_Operation_setSafePerfs(Interface THIS, Operation operation);

/*
 * Interface_Operation_setHoisted --	Set an Operation as volatile.
 */
void
Interface_Operation_setHoisted(Interface THIS, Operation operation);

/*
 * Interface_Operation_setVolatile --	Set an Operation as volatile.
 */
void
Interface_Operation_setVolatile(Interface THIS, Operation operation);

/*
 * Interface_Operation_setPrefetch --	Set an Operation as memory prefetch.
 */
void
Interface_Operation_setPrefetch(Interface THIS, Operation operation);

/*
 * Interface_Operation_setPreload --	Set an Operation as memory preload.
 */
void
Interface_Operation_setPreload(Interface THIS, Operation operation);

/*
 * Interface_Operation_setBarrier --	Set an Operation as a memory barrier.
 */
void
Interface_Operation_setBarrier(Interface THIS, Operation operation);

/*
 * Interface_Operation_setSpillCode --	Set an Operation as spill code.
 */
void
Interface_Operation_setSpillCode(Interface THIS, Operation operation, Symbol symbol);

/*
 * Interface_makeBasicBlock --	Make a BasicBlock.
 */
BasicBlock
Interface_makeBasicBlock(Interface THIS, CGIR_BB cgir_bb, O64_Processor processor,
                         int unrolled, int labelCount, Label labels[], int operationCount,
                         Operation operations[], intptr_t regionId, float frequency);

/*
 * Interface_findBasicBlock --	Find a BasicBlock.
 */
BasicBlock
Interface_findBasicBlock(Interface THIS, CGIR_BB cgir_bb);

/*
 * Interface_linkBasicBlocks --	Link two BasicBlocks.
 */
void
Interface_linkBasicBlocks(Interface THIS, BasicBlock tail_block,
                          BasicBlock head_block, float probability);

/*
 * Interface_makeLoopScope --	Make a LoopScope.
 */
LoopScope
Interface_makeLoopScope(Interface THIS, CGIR_LD cgir_ld, BasicBlock basicBlock,
                        Temporary temporary, O64_ConfigureItem item, ...);

/*
 * Interface_findLoopScope --	Find a LoopScope.
 */
LoopScope
Interface_findLoopScope(Interface THIS, CGIR_LD cgir_ld);

/*
 * Interface_LoopScope_setDependenceNode --	Set Operation with a memory dependence node.
 */
void
Interface_LoopScope_setDependenceNode(Interface THIS, LoopScope loopScope, Operation operation, O64_DependenceNode node);

/*
 * Interface_LoopScope_setDependenceArc --	Set a memory dependence arc between two operations.
 */
void
Interface_LoopScope_setDependenceArc(Interface THIS, LoopScope loopScope,
    Operation tail_operation, Operation head_operation, int latency, int omega, O64_DependenceKind kind);

//
/*
 * Interface_setBody --	Set a BasicBlock as a body block.
 */
void
Interface_setBody(Interface THIS, BasicBlock basicBlock);

//
/*
 * Interface_setEntry --	Set a BasicBlock as an entry block.
 */
void
Interface_setEntry(Interface THIS, BasicBlock basicBlock);

//
/*
 * Interface_setExit --	Set a BasicBlock as an exit block.
 */
void
Interface_setExit(Interface THIS, BasicBlock basicBlock);

//
/*
 * Interface_setStart --	Set a BasicBlock as trace start block.
 */
void
Interface_setStart(Interface THIS, BasicBlock basicBlock);

//
/*--------------------- LAO Interface Output Functions -----------------------*/

/*
 * Interface_updateCGIR --	Update the CGIR from the LAO Interface.
 */
void
Interface_updateCGIR(Interface THIS, CGIR_CallBack callback);

//
/*--------------------- LAO Interface Control Functions ----------------------*/

/*
 * Interface_Initialize --	Initialize the interface, must be called once per process.
 */
void
Interface_Initialize(void);

/*
 * Interface_open --	Open THIS Interface.
 */
void
Interface_open(Interface THIS, const char *name, O64_ConfigureItem item, ...);

/*
 * Interface_optimize --	Apply the LAO optimizations through THIS Interface.
 */
unsigned
Interface_optimize(Interface THIS, O64_OptimizeItem item, ...);

/*
 * Interface_close --	Close THIS Interface.
 */
void
Interface_close(Interface THIS);

/*
 * Interface_Finalize --	Finalize the interface, must be called once per process.
 */
void
Interface_Finalize(void);
