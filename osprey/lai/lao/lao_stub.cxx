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
#include "bb_map.h"
#include "gtn_universe.h"
#include "gtn_tn_set.h"
#include "gra.h"
#include "gra_live.h"
#include "cg_loop.h"
#include "cgprep.h"
#include "dominate.h"
#include "findloops.h"
#include "hb.h"
#include "cgtarget.h"
#include "cgexp.h"
#include "data_layout.h"

#include "annotations.h"
#include "cg_dep_graph.h"
#include "cg_spill.h"

#include "erglob.h"
#include "tracing.h"

#include "cg_flags.h"

#include <dlfcn.h>		    /* for dlsym() */
#include "dso.h"		    /* for load_so() */

extern "C" {
#define this THIS
#define operator OPERATOR
#include "LAO_Interface.h"
#undef operator
#undef this
}
/*-------------------------- LAI Interface instance -------*/
static LAI_Interface LAI_instance;
static Interface interface;

/*-------------------------- This stub interface -------*/
extern void lao_init(void);
extern void lao_fini(void);
extern void lao_init_pu(void);
extern void lao_fini_pu(void);
extern void lao_init_region(void);
extern void lao_fini_region(void);
extern bool lao_optimize_pu(unsigned lao_optimizations);

/*------------------------- LAO Region Map -------------*/
BB_MAP CG_LAO_Region_Map;


/*-------------------------- Interface for target dependencies -------*/
#include "targ_cgir_lao.h"

/*--------------------------- lao_init / lao_fini ----------------------------*/
extern "C" {
#include <unistd.h>
}

#ifdef Is_True_On
static void CGIR_print(FILE *file);
// For debug only
extern "C" char *getenv(const char *);
#define GETENV(x) getenv(x)
#else
#define GETENV(x) ((char *)0)
#endif

// The CGIR_CallBack structure and its pointer.
static CGIR_CallBack_ callback_, *callback = &callback_;
// Forward reference to the callback initialization function.
static void LIR_CGIR_callback_init(CGIR_CallBack callback);

// Variable used to skip multiple lao_init / lao_fini calls.
static int lao_initialized = 0;

// LAO so handler
#if defined(__MINGW32__) || defined(__CYGWIN__)
#define SO_EXT ".dll"
#else
#define SO_EXT ".so"
#endif
static void* lao_handler = NULL;

// Initialization of the LAO, needs to be called once per running process.
void
lao_init(void) 
{
  if (lao_initialized++ == 0) {
    LAI_Interface (*LAI_getInstance_p)(void);
    lao_handler = load_so("lao"SO_EXT, CG_Path, 0);
    LAI_getInstance_p = (LAI_Interface (*)(void))dlsym(lao_handler, "LAI_getInstance");
    LAI_instance = (*LAI_getInstance_p)();
    FmtAssert(LAI_Interface_size(LAI_instance) == sizeof(LAI_Interface_), ("LAI_Instance mistmatch"));
    // Initialize LAI Interface
    LAI_Interface_Initialize();
    interface = LAI_Interface_getInstance();
    // Initialize the LIR->CGIR callback object
    LIR_CGIR_callback_init(callback);
    // Initialize the target dependent LIR<->CGIR interface
    TARG_CGIR_LAI_Init();
  }
#ifdef Is_True_On
  if (GETENV("LAO_PID")) {
    int dummy; fprintf(stderr, "PID=%lld\n", (int64_t)getpid()); scanf("%d", &dummy);
  }
#endif
}

// Finalization of the LAO, needs to be called once.
void
lao_fini(void) {
  if (--lao_initialized == 0) {
    // Finalize target dependent interface
    TARG_CGIR_LAI_Fini();
    // Finalize LAI Interface
    LAI_Interface_Finalize();
    interface = NULL;
    LAI_instance = NULL;
    close_so(lao_handler);
    lao_handler = NULL;
  }
}

// Per PU initialization.
void
lao_init_pu(void) {
  CG_LAO_Region_Map = NULL;
}

// Per PU finalization.
void
lao_fini_pu(void) {
  if (CG_LAO_Region_Map) {
    BB_MAP_Delete(CG_LAO_Region_Map);
    CG_LAO_Region_Map = NULL;
  }
}

// Per Region initialization.
void
lao_init_region(void) {
}

// Per Region finalization.
void
lao_fini_region(void) {
}

/*-------------------------- CGIR Utility Functions ------------------*/

#define OP_gnu_asm(op)	(OP_code(op) == TOP_asm)

extern OP_MAP OP_Asm_Map;

static BOOL
OP_clobber_reg(OP *op) {
  if (OP_call(op)) return TRUE;
  if (OP_gnu_asm(op)) {
    ASM_OP_ANNOT *asm_info = (ASM_OP_ANNOT*)OP_MAP_Get(OP_Asm_Map, op);
    ISA_REGISTER_CLASS irc;
    FOR_ALL_ISA_REGISTER_CLASS(irc) {
      REGISTER_SET regset = ASM_OP_clobber_set(asm_info)[irc];
      if (!REGISTER_SET_EmptyP(regset)) return TRUE;
    }
  }
  return FALSE;
}

static BOOL
OP_barrier(OP *op) {
  if (OP_Is_Barrier(op)) return TRUE;
  return FALSE;
}


/*-------------------------- LAO Utility Functions-------------------------*/

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

static void
lao_topsort_DFS(BB *bb, BB_SET *region_set, BB_MAP visited_map, BB_List &bblist) {
  BBLIST *succs;
  //
  BB_MAP32_Set(visited_map, bb, 1);
  //
  FOR_ALL_BB_SUCCS(bb, succs) {
    BB *succ_bb = BBLIST_item(succs);
    if (!BB_MAP32_Get(visited_map, succ_bb) &&
	BB_SET_MemberP(region_set, succ_bb)) {
      lao_topsort_DFS(succ_bb, region_set, visited_map, bblist);
    }
  }
  //
  bblist.push_front(bb);
}

static void
lao_topsort(BB *entry, BB_SET *region_set, BB_List &bblist) {
  BB_MAP visited_map = BB_MAP32_Create();
  lao_topsort_DFS(entry, region_set, visited_map, bblist);
  BB_MAP_Delete(visited_map);
}

/*-------------------- CGIR -> LIR Conversion Fonctions ----------------------*/
// These functions are the only ones to call and must only call the 
// Interface_make functions.

// Convert CGIR_LAB to LIR Label.
static inline Label
CGIR_LAB_to_Label(CGIR_LAB cgir_lab) {
  Label label = LAI_Interface_findLabel(interface, cgir_lab);
  if (label == NULL) {
    label = LAI_Interface_makeLabel(interface, cgir_lab, LABEL_name(cgir_lab));
  }
  return label;
}

// Convert CGIR_ST_CLASS to LIR SClass
static inline LAI_SClass
CGIR_ST_CLASS_to_SClass(ST_CLASS sclass)
{
  static LAI_SClass sclasses[CLASS_COUNT] = {
    LAI_SClass_UNDEF,	// CLASS_UNK
    LAI_SClass_VAR,	// CLASS_VAR
    LAI_SClass_FUNC,	// CLASS_FUNC
    LAI_SClass_CONST,	// CLASS_CONST
    LAI_SClass_PREG,	// CLASS_PREG
    LAI_SClass_BLOCK,	// CLASS_BLOCK
    LAI_SClass_UNDEF	// CLASS_NAME has no LAI mapping
  };
  return sclasses[sclass];
}

// Convert CGIR_ST_SCLASS to LIR SStorage
static inline LAI_SStorage
CGIR_ST_SCLASS_to_SStorage(ST_SCLASS sstorage)
{
  static LAI_SStorage sstorages[SCLASS_COUNT] = {
    LAI_SStorage_UNDEF,		// SCLASS_UNKNOWN
    LAI_SStorage_AUTO,		// SCLASS_AUTO
    LAI_SStorage_FORMAL,	// SCLASS_FORMAL
    LAI_SStorage_FORMAL_REF,	// SCLASS_FORMAL_REF
    LAI_SStorage_PSTATIC,	// SCLASS_PSTATIC
    LAI_SStorage_FSTATIC,	// SCLASS_FSTATIC
    LAI_SStorage_COMMON,	// SCLASS_COMMON
    LAI_SStorage_EXTERN,	// SCLASS_EXTERN
    LAI_SStorage_UGLOBAL,	// SCLASS_UGLOBAL
    LAI_SStorage_DGLOBAL,	// SCLASS_DGLOBAL
    LAI_SStorage_TEXT,		// SCLASS_TEXT
    LAI_SStorage_REG,		// SCLASS_REG
    LAI_SStorage_UNDEF,		// SCLASS_CPLINIT not mapped
    LAI_SStorage_UNDEF,		// SCLASS_EH_REGION not mapped
    LAI_SStorage_UNDEF,		// SCLASS_EH_REGION_SUPP not mapped
    LAI_SStorage_UNDEF,		// SCLASS_DISTR_ARRAY not mapped
    LAI_SStorage_UNDEF,		// SCLASS_COMMENT not mapped 
    LAI_SStorage_UNDEF		// SCLASS_THREAD_PRIVATE_FUNCS not mapped
  };
  return sstorages[sstorage];
}

// Convert CGIR_ST_EXPORT to LIR SExport
static inline LAI_SExport
CGIR_ST_EXPORT_to_SExport(ST_EXPORT sexport)
{
  static LAI_SExport sexports[EXPORT_COUNT] = {
    LAI_SExport_LOCAL,			// EXPORT_LOCAL
    LAI_SExport_LOCAL_INTERNAL,		// EXPORT_LOCAL_INTERNAL
    LAI_SExport_GLOBAL_INTERNAL,	// EXPORT_INTERNAL
    LAI_SExport_GLOBAL_HIDDEN,		// EXPORT_HIDDEN
    LAI_SExport_GLOBAL_PROTECTED,	// EXPORT_PROTECTED
    LAI_SExport_GLOBAL_PREEMPTIBLE,	// EXPORT_PREEMPTIBLE
    LAI_SExport_UNDEF			// EXPORT_OPTIONAL not mapped
  };
  return sexports[sexport];
}

// Convert CGIR_SYM to LIR Symbol.
static inline Symbol
CGIR_SYM_to_Symbol(CGIR_SYM cgir_sym) {
  Symbol symbol = LAI_Interface_findSymbol(interface, cgir_sym);
  if (symbol == NULL) {
    if (ST_class(cgir_sym) == CLASS_CONST) {
      char buffer[64];
      sprintf(buffer, "CONST#%llu", (uint64_t)cgir_sym);
      symbol = LAI_Interface_makeSymbol(interface, cgir_sym, buffer);
    } else {
      symbol = LAI_Interface_makeSymbol(interface, cgir_sym, ST_name(cgir_sym));
    }
    LAI_Interface_Symbol_setClasses(interface, symbol, 
				CGIR_ST_CLASS_to_SClass(ST_sym_class(St_Table[cgir_sym])),
				CGIR_ST_SCLASS_to_SStorage(ST_storage_class(St_Table[cgir_sym])),
				CGIR_ST_EXPORT_to_SExport(ST_export(St_Table[cgir_sym])));
  }
  return symbol;
}

static inline Temporary CGIR_TN_to_Temporary(CGIR_TN cgir_tn);

// Returns a TN for the rematerializable value
static Temporary
CGIR_TN_REMAT_to_Temporary(CGIR_TN cgir_tn)
{
  WN *home = TN_home(cgir_tn);
  Temporary temporary = NULL;
  switch (WN_operator(home)) {
  case OPR_LDA: {
    Immediate immediate = TARG_CGIR_LC_to_Immediate((ISA_LIT_CLASS)0);	// HACK ALERT
    ST *var_st = WN_st(home);
    ST_IDX st_idx = ST_st_idx(*var_st);
    int64_t offset = WN_lda_offset(home);
    TN * tn = Gen_Symbol_TN (var_st, offset, 0);
    temporary = CGIR_TN_to_Temporary(tn);
  } break;
  case OPR_INTCONST: {
    if (WN_rtype(home) == MTYPE_I4 ||
	WN_rtype(home) == MTYPE_U4) {
      TN *tn = Gen_Literal_TN ((INT32) WN_const_val(home), 4);
      temporary = CGIR_TN_to_Temporary(tn);
    } else {
      // Currently not handled.
    }
  } break;
  case OPR_CONST: {
    // Currently not handled.
  } break;
  }
  return temporary;
}

// Returns a TN for the homeable value
static Temporary
CGIR_TN_HOME_to_Temporary(CGIR_TN cgir_tn)
{
  DevWarn("Should pass TN_is_gra_homeable to LAO for TN%d\n", TN_number(cgir_tn));
  return NULL;
}

// Convert CGIR_TN to LIR Temporary.
static inline Temporary
CGIR_TN_to_Temporary(CGIR_TN cgir_tn) {
  Temporary temporary = LAI_Interface_findTemporary(interface, cgir_tn);
  if (temporary == NULL) {
    if (TN_is_register(cgir_tn)) {
      if (TN_is_dedicated(cgir_tn)) {
	CLASS_REG_PAIR tn_crp = TN_class_reg(cgir_tn);
	// On the open64 side, the dedicated property can be set to a temporary register not
	// in the initial dedicated set. And additional information such as home location
	// may be set on such dedicated registers.
	// On the LAO side, dedicated temporaries are shared, so we create dedicated temporaries
	// only if the cgir_tn is in the initial dedicated set. Otherwise we create an
	// assigned temporary and set the dedicated flag.
	if (Build_Dedicated_TN(CLASS_REG_PAIR_rclass(tn_crp), CLASS_REG_PAIR_reg(tn_crp), 0) == cgir_tn)
	  temporary = LAI_Interface_makeDedicatedTemporary(interface, cgir_tn, TARG_CGIR_CRP_to_Register(tn_crp));
	else {
	  temporary = LAI_Interface_makeAssignRegTemporary(interface, cgir_tn, TARG_CGIR_CRP_to_Register(tn_crp));
	  LAI_Interface_Temporary_setDedicated(interface, temporary);
	}
      } else if (TN_register(cgir_tn) != REGISTER_UNDEFINED) {
	CLASS_REG_PAIR tn_crp = TN_class_reg(cgir_tn);
	temporary = LAI_Interface_makeAssignRegTemporary(interface, cgir_tn, TARG_CGIR_CRP_to_Register(tn_crp));
      } else {
	ISA_REGISTER_CLASS tn_irc = TN_register_class(cgir_tn);
	temporary = LAI_Interface_makePseudoRegTemporary(interface, cgir_tn, TARG_CGIR_IRC_to_RegClass(tn_irc));
      }
      // Pass special tn flags
      if (TN_is_rematerializable(cgir_tn)) {
	Temporary remat = CGIR_TN_REMAT_to_Temporary(cgir_tn);
	if (remat != NULL) {
	  LAI_Interface_Temporary_setRematerializable(interface, temporary, remat);
	}
      } else if (TN_is_gra_homeable(cgir_tn)) {
	Temporary home = CGIR_TN_HOME_to_Temporary(cgir_tn);
	if (home != NULL) {
	  LAI_Interface_Temporary_setHomeable(interface, temporary, home);
	}
      }
    } else if (TN_is_constant(cgir_tn)) {
      if (TN_has_value(cgir_tn)) {
	int64_t value = TN_value(cgir_tn);
	Immediate immediate = TARG_CGIR_LC_to_Immediate((ISA_LIT_CLASS)0);	// HACK ALERT
	temporary = LAI_Interface_makeAbsoluteTemporary(interface, cgir_tn, immediate, value);
      } else if (TN_is_symbol(cgir_tn)) {
	Symbol symbol = NULL;
	ST *var_st = TN_var(cgir_tn);
	ST_IDX st_idx = ST_st_idx(*var_st);
	int64_t offset = TN_offset(cgir_tn);
	Immediate immediate = TARG_CGIR_LC_to_Immediate((ISA_LIT_CLASS)0);	// HACK ALERT
	symbol = CGIR_SYM_to_Symbol(st_idx);
	temporary = LAI_Interface_makeSymbolTemporary(interface, cgir_tn, immediate, symbol, offset);
      } else if (TN_is_label(cgir_tn)) {
	CGIR_LAB cgir_lab = TN_label(cgir_tn);
	Immediate immediate = TARG_CGIR_LC_to_Immediate((ISA_LIT_CLASS)0);	// HACK ALERT
	Label label = CGIR_LAB_to_Label(cgir_lab);
	temporary = LAI_Interface_makeLabelTemporary(interface, cgir_tn, immediate, label);
	Is_True(TN_offset(cgir_tn) == 0, ("LAO requires zero offset from label."));
      } else if (TN_is_enum(cgir_tn)) {
	ISA_ENUM_CLASS_VALUE value = TN_enum(cgir_tn);
	Modifier modifier = TARG_CGIR_IEC_to_Modifier((ISA_ENUM_CLASS)0);	// HACK ALERT
	temporary = LAI_Interface_makeModifierTemporary(interface, cgir_tn, modifier, value);
      } else {
	Is_True(FALSE, ("Unknown constant TN type."));
      }
    } else {
      Is_True(FALSE, ("Unknown TN type."));
    }
  }
  return temporary;
}

// Convert CGIR_OP to LIR Operation.
static Operation
CGIR_OP_to_Operation(CGIR_OP cgir_op) {
  Operation operation = LAI_Interface_findOperation(interface, cgir_op);
  if (operation == NULL) {
    // the Operation arguments
    int argCount = OP_opnds(cgir_op);
    Temporary *arguments = (Temporary *)(argCount ? alloca(argCount*sizeof(Temporary)) : NULL);
    for (int i = 0; i < argCount; i++) arguments[i] = CGIR_TN_to_Temporary(OP_opnd(cgir_op, i));
    // the Operation results
    int resCount = OP_results(cgir_op);
    Temporary *results = (Temporary *)(resCount ? alloca(resCount*sizeof(Temporary)) : NULL);
    for (int i = 0; i < resCount; i++) results[i] = CGIR_TN_to_Temporary(OP_result(cgir_op, i));
    // the Operation clobber
    int clobberCount = 0;
    int clobbers[ISA_REGISTER_CLASS_COUNT*ISA_REGISTER_MAX];
    if (OP_clobber_reg(cgir_op)) {
      ISA_REGISTER_CLASS irc;
      FOR_ALL_ISA_REGISTER_CLASS(irc) {
	REGISTER_SET regset;
	if (OP_gnu_asm(cgir_op)) {
	  ASM_OP_ANNOT* asm_info = (ASM_OP_ANNOT*) OP_MAP_Get(OP_Asm_Map, cgir_op);
	  regset = ASM_OP_clobber_set(asm_info)[irc];
	} else if (OP_call(cgir_op)) {
	  regset = REGISTER_CLASS_caller_saves(irc);
	}
	for (REGISTER reg = REGISTER_SET_Choose(regset);
	     reg != REGISTER_UNDEFINED;
	     reg = REGISTER_SET_Choose_Next(regset, reg)) {
	  TN* cgir_tn = Build_Dedicated_TN(irc, reg, 0);
	  CLASS_REG_PAIR tn_crp = TN_class_reg(cgir_tn);
	  clobbers[clobberCount++] = TARG_CGIR_CRP_to_Register(tn_crp);
	}
      }
      Is_True(clobberCount > 0, ("Empty register clobber list"));
    }
    // make the Operation
    Operator OPERATOR = TARG_CGIR_TOP_to_Operator(OP_code(cgir_op));
    operation = LAI_Interface_makeOperation(interface, cgir_op,
	OPERATOR, argCount, arguments, resCount, results, clobberCount, clobbers);
    if (OP_volatile(cgir_op)) LAI_Interface_Operation_setVolatile(interface, operation);
    if (OP_prefetch(cgir_op)) LAI_Interface_Operation_setPrefetch(interface, operation);
    if (OP_barrier(cgir_op)) LAI_Interface_Operation_setBarrier(interface, operation);
    ST *spill_st = CGSPILL_OP_Spill_Location(cgir_op);
    if (spill_st != NULL && OP_spill(cgir_op)) {
      Symbol symbol = CGIR_SYM_to_Symbol(ST_st_idx(*spill_st));
      LAI_Interface_Operation_setSpillCode(interface, operation, symbol);
    }
  }
  return operation;
}

// Convert CGIR_BB to LIR BasicBlock.
static BasicBlock
CGIR_BB_to_BasicBlock(CGIR_BB cgir_bb) {
  BasicBlock basicblock = LAI_Interface_findBasicBlock(interface, cgir_bb);
  if (basicblock == NULL) {
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
    // For instruction mode currently the targ interface does not
    // account for isa subset. HACK.
    InstrMode instrmode = TARG_CGIR_IS_to_InstrMode((ISA_SUBSET)0);
    // make the BasicBlock
    basicblock = LAI_Interface_makeBasicBlock(interface, cgir_bb, instrmode,
	labelCount, labels, operationCount, operations);
    // more the BasicBlock
    int liveinCount = 0, MAX_LIVEIN_COUNT = 16384;
    Temporary *liveins = (Temporary *)alloca(MAX_LIVEIN_COUNT*sizeof(Temporary));
    for (TN *tn = GTN_SET_Choose(BB_live_in(cgir_bb));
	 tn != GTN_SET_CHOOSE_FAILURE;
	 tn = GTN_SET_Choose_Next(BB_live_in(cgir_bb), tn)) {
      Temporary temp = CGIR_TN_to_Temporary(tn);
      // All live in are global
      LAI_Interface_Temporary_setGlobal(interface, temp);
      // We only take live-in that are defreach_in
      if (GRA_LIVE_TN_Live_Into_BB(tn, cgir_bb)) {
	Is_True(liveinCount < MAX_LIVEIN_COUNT, ("BB has more than MAX_LIVEIN_COUNT liveins"));
	liveins[liveinCount++] = temp;
      }
    }
    int liveoutCount = 0, MAX_LIVEOUT_COUNT = 16384;
    Temporary *liveouts = (Temporary *)alloca(MAX_LIVEOUT_COUNT*sizeof(Temporary));
    for (TN *tn = GTN_SET_Choose(BB_live_out(cgir_bb));
	 tn != GTN_SET_CHOOSE_FAILURE;
	 tn = GTN_SET_Choose_Next(BB_live_out(cgir_bb), tn)) {
      Temporary temp = CGIR_TN_to_Temporary(tn);
      // All live out are global
      LAI_Interface_Temporary_setGlobal(interface, temp);
      // We only take live-out that are defreach_out
      if (GRA_LIVE_TN_Live_Outof_BB(tn, cgir_bb)) {
	Is_True(liveoutCount < MAX_LIVEOUT_COUNT, ("BB has more than MAX_LIVEOUT_COUNT liveouts"));
	liveouts[liveoutCount++] = temp;
      }
    }
    intptr_t regionId = (intptr_t)BB_rid(cgir_bb);
    float frequency = BB_freq(cgir_bb);
    LAI_Interface_moreBasicBlock(interface, basicblock, regionId, frequency, liveinCount, liveins, liveoutCount, liveouts);
  }
  return basicblock;
}

#define LAO_OPS_LIMIT 512	// Maximum number of OPs to compute memory dependences.

// Convert CGIR_LD to LIR LoopInfo.
static LoopInfo
CGIR_LD_to_LoopInfo(CGIR_LD cgir_ld) {
  LoopInfo loopinfo = LAI_Interface_findLoopInfo(interface, cgir_ld);
  if (loopinfo == NULL) {
    BB *head_bb = LOOP_DESCR_loophead(cgir_ld);
    BasicBlock head_block = CGIR_BB_to_BasicBlock(head_bb);
    LOOPINFO *cgir_li = LOOP_DESCR_loopinfo(cgir_ld);
    if (cgir_li != NULL) {
      TN *trip_count_tn = LOOPINFO_trip_count_tn(cgir_li);
      if (trip_count_tn != NULL && TN_is_constant(trip_count_tn)) {
	uint64_t trip_count = TN_value(trip_count_tn);
	int8_t min_trip_count = trip_count <= 127 ? trip_count : 127;
	uint64_t trip_factor = trip_count & -trip_count;
	int8_t min_trip_factor = trip_factor <= 64 ? trip_factor : 64;
	loopinfo = LAI_Interface_makeLoopInfo(interface, cgir_ld, head_block,
	    Configuration_Pipelining, CG_LAO_pipelining,
	    Configuration_MinTrip, min_trip_count,
	    Configuration_Modulus, min_trip_factor,
	    Configuration_Residue, 0,
	    ConfigurationItem__);
      } else {
	loopinfo = LAI_Interface_makeLoopInfo(interface, cgir_ld, head_block,
	    Configuration_Pipelining, CG_LAO_pipelining,
	    ConfigurationItem__);
      }
    } else {
      loopinfo = LAI_Interface_makeLoopInfo(interface, cgir_ld, head_block,
	  Configuration_Pipelining, CG_LAO_pipelining,
	  ConfigurationItem__);
    }
    // Fill the LoopInfo dependence table.
    //
    // Make a BB_List of the loop body and compute its op_count.
    // This BB_List is reordered in topological order.
    int nest_level = BB_nest_level(head_bb), op_count = 0;
    BB_List bb_topo_list, bb_list;
    BB_SET *loop_set = LOOP_DESCR_bbset(cgir_ld);
    lao_topsort(head_bb, loop_set, bb_topo_list);
    BB_List::iterator bb_iter;
    for (bb_iter = bb_topo_list.begin(); bb_iter != bb_topo_list.end(); bb_iter++) {
      if (BB_nest_level(*bb_iter) == nest_level) {
	OP *op = NULL;
	FOR_ALL_BB_OPs(*bb_iter, op) {
	  if (OP_memory(op) || OP_barrier(op)) ++op_count;
	}
	bb_list.push_back(*bb_iter);
	if (op_count >= LAO_OPS_LIMIT) {
	  DevWarn("LAO_OPS_LIMIT exceeded (%d memory operations)", op_count);
	  break;
	}
      }
    }
    //
    // Compute the memory dependence graph.
    if (op_count < LAO_OPS_LIMIT && CG_LAO_loopdep > 0) {
      bool cyclic = BB_innermost(head_bb) != 0;
      CG_DEP_Compute_Region_MEM_Arcs(bb_list, cyclic, false);
      BB_List::iterator bb_iter;
      for (bb_iter = bb_list.begin(); bb_iter != bb_list.end(); bb_iter++) {
	OP *op = NULL;
	FOR_ALL_BB_OPs(*bb_iter, op) {
	  if (_CG_DEP_op_info(op)) {
	    Operation orig_operation = CGIR_OP_to_Operation(op);
	    if (OP_memory(op) || OP_barrier(op)) {
	      LAI_Interface_LoopInfo_setDependenceNode(interface, loopinfo, orig_operation);
	    }
	    for (ARC_LIST *arcs = OP_succs(op); arcs; arcs = ARC_LIST_rest(arcs)) {
	      ARC *arc = ARC_LIST_first(arcs);
	      CG_DEP_KIND kind = ARC_kind(arc);
	      if (ARC_is_mem(arc) && kind != CG_DEP_MEMVOL) {
		unsigned type = Dependence_Other;
		if (kind == CG_DEP_MEMIN) type = Dependence_Flow;
		if (kind == CG_DEP_MEMOUT) type = Dependence_Output;
		if (kind == CG_DEP_MEMANTI) type = Dependence_Anti;
		if (kind == CG_DEP_MEMREAD) type = Dependence_Input;
		if (kind == CG_DEP_SPILLIN) type = Dependence_Spill;
		if (ARC_is_definite(arc)) type += Dependence_Definite;
		int latency = ARC_latency(arc), omega = ARC_omega(arc);
		OP *pred_op = ARC_pred(arc), *succ_op = ARC_succ(arc);
		Is_True(pred_op == op, ("Error in lao_setDependences"));
		Operation dest_operation = CGIR_OP_to_Operation(succ_op);		
		LAI_Interface_LoopInfo_setDependenceArc(interface, loopinfo,
		    orig_operation, dest_operation, latency, omega, (DependenceType)type);
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

/*-------------------- LIR -> CGIR Interface Call-Backs -------------------*/

// Create a CGIR_LAB.
static CGIR_LAB
CGIR_LAB_create(Label label, CGIR_LAB cgir_lab) {
  const char *name = LAI_Interface_Label_name(label);
  CGIR_LAB new_lab = 0;
  // code borrowed from Gen_Label_For_BB
  LABEL *plabel = &New_LABEL(CURRENT_SYMTAB, new_lab);
  LABEL_Init(*plabel, Save_Str(name), LKIND_DEFAULT);
  //
  return new_lab;
}

// Update a CGIR_LAB.
static void
CGIR_LAB_update(Label label, CGIR_LAB cgir_lab) {
  // should not be modified
}

// Create a CGIR_SYM.
static CGIR_SYM
CGIR_SYM_create(Symbol symbol, CGIR_SYM cgir_sym) {
  // Currently LAO is allowed to generate:
  // - spill symbols
  // - that's all
  //
  // Spill symbol.
  if (LAI_Interface_Symbol_isSpill(symbol)) {
    // We use the CGSPILL interface to generate a CGIR spill symbol
    TY_IDX ty = 
      MTYPE_To_TY(TARG_MType_to_CGIR_TYPE_ID(LAI_Interface_Symbol_mtype(symbol)));
    ST *st = CGSPILL_Gen_Spill_Symbol(ty, (const char *)LAI_Interface_Symbol_name(symbol));
    return ST_st_idx(*st);
  }
  return (CGIR_SYM)0;
}

// Update a CGIR_SYM.
static void
CGIR_SYM_update(Symbol symbol, CGIR_SYM cgir_sym) {
  // Currently LAO is allowed to update:
  // - symbol referenced by a symbol Temporary
  //   This case occurs for generation of homed/rematerialized spill by LAO
  //   where the remat symbol was passed without being allocated yet.
  //   The symbol is not modified, but should be allocated is not.
  if (!Is_Allocated(&St_Table[cgir_sym])) {
    Allocate_Object(&St_Table[cgir_sym]);
  }
}

// Create a Dedicated CGIR_TN.
static CGIR_TN
CGIR_Dedicated_TN_create(Temporary temporary, CGIR_TN cgir_tn) {
  Register registre = LAI_Interface_Temporary_assigned(temporary);
  INT size = 0;		// not used in Build_Dedicated_TN
  CLASS_REG_PAIR crp = TARG_Register_to_CGIR_CRP(registre);
  return Build_Dedicated_TN(CLASS_REG_PAIR_rclass(crp), CLASS_REG_PAIR_reg(crp), size);
}

// Create a PseudoReg CGIR_TN.
static CGIR_TN
CGIR_PseudoReg_TN_create(Temporary temporary, CGIR_TN cgir_tn) {
  RegClass regClass = LAI_Interface_Temporary_regClass(temporary);
  ISA_REGISTER_CLASS irc = TARG_RegClass_to_CGIR_IRC(regClass);
  INT bsize = ISA_REGISTER_CLASS_INFO_Bit_Size(ISA_REGISTER_CLASS_Info(irc));
  INT size = (bsize + 7)/8;
  return Gen_Register_TN(irc, size);
}

// Create an AssignReg CGIR_TN.
static CGIR_TN
CGIR_AssignReg_TN_create(Temporary temporary, CGIR_TN cgir_tn) {
  RegClass regClass = LAI_Interface_Temporary_regClass(temporary);
  Register assigned = LAI_Interface_Temporary_assigned(temporary);
  ISA_REGISTER_CLASS irc = TARG_RegClass_to_CGIR_IRC(regClass);
  INT bsize = ISA_REGISTER_CLASS_INFO_Bit_Size(ISA_REGISTER_CLASS_Info(irc));
  INT size = (bsize + 7)/8;
  TN *tn = Gen_Register_TN(irc, size);
  CLASS_REG_PAIR crp = TARG_Register_to_CGIR_CRP(assigned);
  Set_TN_register(tn, CLASS_REG_PAIR_reg(crp));
  return tn;
}

// Create a Modifier CGIR_TN.
static CGIR_TN
CGIR_Modifier_TN_create(Temporary temporary, CGIR_TN cgir_tn) {
  Modifier modifier = LAI_Interface_Temporary_modifier(temporary);
  FmtAssert(0, ("CGIR_Modifier_TN_create not implemented"));
  return NULL;
}

// Create an Absolute CGIR_TN.
static CGIR_TN
CGIR_Absolute_TN_create(Temporary temporary, CGIR_TN cgir_tn) {
  Immediate immediate = LAI_Interface_Temporary_immediate(temporary);
  int64_t value = LAI_Interface_Temporary_value(temporary);
  INT size = (value >= (int64_t)0x80000000 && 
	      value <= (int64_t)0x7FFFFFFF) ? 4 : 8;
  return Gen_Literal_TN(value, size);
}

// Create a Symbol CGIR_TN.
static CGIR_TN
CGIR_Symbol_TN_create(Temporary temporary, CGIR_TN cgir_tn, CGIR_SYM cgir_sym) {
  int64_t offset = LAI_Interface_Temporary_offset(temporary);
  return Gen_Symbol_TN (&St_Table[cgir_sym], offset, 0);
}

// Create a Label CGIR_TN.
static CGIR_TN
CGIR_Label_TN_create(Temporary temporary, CGIR_TN cgir_tn, CGIR_LAB cgir_lab) {
  return Gen_Label_TN(cgir_lab, 0);
}

// Update a CGIR_TN.
static void
CGIR_TN_update(Temporary temporary, CGIR_TN cgir_tn) {
  // Currently LAO is allowed to update:
  // - pseudo temporaries into assigned temporary
  // - that's all
  //
  // Temporary that were assigned
  if (!LAI_Interface_Temporary_isDedicated(temporary) &&
      LAI_Interface_Temporary_isAssignReg(temporary)) {
    CLASS_REG_PAIR cgir_crp = 
      TARG_Register_to_CGIR_CRP(LAI_Interface_Temporary_assigned(temporary));
    Set_TN_register(cgir_tn, CLASS_REG_PAIR_reg(cgir_crp));
  }
}

// Create a CGIR_OP.
static CGIR_OP
CGIR_OP_create(Operation operation, CGIR_OP cgir_op, CGIR_TN arguments[], CGIR_TN results[], int unrolled) {
  int iteration = LAI_Interface_Operation_iteration(operation);
  int issueDate = LAI_Interface_Operation_issueDate(operation);
  Operator opr = LAI_Interface_Operation_operator(operation);
  int argCount = 0, resCount = 0;
  TOP top = TARG_Operator_to_CGIR_TOP(opr);
  for (argCount = 0; arguments[argCount] != NULL; argCount++);
  for (resCount = 0; results[resCount] != NULL; resCount++);
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
  // Add spill information
  if (LAI_Interface_Operation_isSpillCode(operation)) {
    TN *spilled_tn;
    TN *offset_tn;
    TN *base_tn;
    if (OP_store(new_op)) {
      int val_idx = TOP_Find_Operand_Use(OP_code(new_op),OU_storeval);
      int offset_idx = TOP_Find_Operand_Use(OP_code(new_op),OU_offset);
      int base_idx = TOP_Find_Operand_Use(OP_code(new_op),OU_base);
      spilled_tn = OP_opnd(new_op, val_idx);
      offset_tn = OP_opnd(new_op, offset_idx);
      base_tn = OP_opnd(new_op, base_idx);
    } else if (OP_load(new_op) && OP_results(new_op) == 1) {
      int offset_idx = TOP_Find_Operand_Use(OP_code(new_op),OU_offset);
      int base_idx = TOP_Find_Operand_Use(OP_code(new_op),OU_base);
      spilled_tn = OP_result(new_op, 0);
      offset_tn = OP_opnd(new_op, offset_idx);
      base_tn = OP_opnd(new_op, base_idx);
    } else {
      Is_True(0, ("Invalid LAO isSpilledOP operation"));
    }
    Is_True(base_tn == SP_TN || base_tn == FP_TN, ("Invalid base TN for LAO spill op"));
    Set_TN_spill(spilled_tn, TN_var(offset_tn));
    Set_OP_spill(new_op);
  }
  // TODO: shouldn't we add volatile  information 
  if (LAI_Interface_Operation_isVolatile(operation)) {
    DevWarn("Operation volatile on LAO side");
  }
  // Set scycle
  OP_scycle(new_op) = issueDate;
  //
  return new_op;
}

// Update a CGIR_OP.
static void
CGIR_OP_update(Operation operation, CGIR_OP cgir_op, CGIR_TN arguments[], CGIR_TN results[], int unrolled) {
  int iteration = LAI_Interface_Operation_iteration(operation);
  int issueDate = LAI_Interface_Operation_issueDate(operation);
  Operator opr = LAI_Interface_Operation_operator(operation);
  BB *bb = OP_bb(cgir_op);
  if (bb != NULL) BB_Remove_Op(bb, cgir_op);
  int argCount = 0, resCount = 0;
  TOP top = TARG_Operator_to_CGIR_TOP(opr);
  if (OP_code(cgir_op) != top) {
    OP_Change_Opcode(cgir_op, top);
  }
  for (argCount = 0; arguments[argCount] != NULL; argCount++) {
    CGIR_TN cgir_tn = arguments[argCount];
    if (OP_opnd(cgir_op, argCount) != cgir_tn) Set_OP_opnd(cgir_op, argCount, cgir_tn);
  }
  Is_True(argCount == OP_opnds(cgir_op), ("OP_opnds mismatch in CGIR_update_OP"));
  for (resCount = 0; results[resCount] != NULL; resCount++) {
    CGIR_TN cgir_tn = results[resCount];
    if (OP_result(cgir_op, resCount) != cgir_tn) Set_OP_result(cgir_op, resCount, cgir_tn);
  }
  Is_True(iteration == 0, ("OP_update must not change OP_unrolling"));
  Is_True(resCount == OP_results(cgir_op), ("OP_results mismatch in CGIR_update_OP"));
  Is_True(iteration == 0, ("CGIR_OP_update called with iteration > 0"));
  // Set scycle.
  OP_scycle(cgir_op) = issueDate;
}

// Create a CGIR_BB.
static CGIR_BB
CGIR_BB_create(BasicBlock basicBlock, CGIR_BB cgir_bb, CGIR_LAB labels[], CGIR_OP operations[], CGIR_RID cgir_rid, unsigned optimizations) {
  int unrolled = LAI_Interface_BasicBlock_unrolled(basicBlock);
  int ordering = LAI_Interface_BasicBlock_ordering(basicBlock);
  CGIR_BB new_bb = Gen_BB();
  // Add the labels.
  for (int labelCount = 0; labels[labelCount] != 0; labelCount++) {
    CGIR_LAB cgir_lab = labels[labelCount];
    // code borrowed from Gen_Label_For_BB
    Set_Label_BB(cgir_lab, new_bb);
    BB_Add_Annotation(new_bb, ANNOT_LABEL, (void *)cgir_lab);
  }
  // Add the operations.
  OPS ops = OPS_EMPTY;
  for (int opCount = 0; operations[opCount] != NULL; opCount++) {
    OPS_Append_Op(&ops, operations[opCount]);
    if (OP_unrolling(operations[opCount]) != 0)
      Set_OP_unroll_bb(operations[opCount], new_bb);
  }
  BB_Append_Ops(new_bb, &ops);
  // Transfer annotations and attributes.
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
    // Set unrollings.
    if (BB_unrollings(cgir_bb) > 0 && unrolled > 0)
      Set_BB_unrollings(new_bb, BB_unrollings(cgir_bb)*unrolled);
    else if (BB_unrollings(cgir_bb) == 0)
      Set_BB_unrollings(new_bb, unrolled);
  } else if (unrolled > 0) {
    Set_BB_unrollings(new_bb, unrolled);
  }
  //
  // Set flags.
  if (optimizations & Optimization_RegAlloc) Set_BB_reg_alloc(new_bb);
  if (optimizations & Optimization_PostSched) Set_BB_scheduled(new_bb);
  // Set the rid.
  BB_rid(new_bb) = cgir_rid;
  //
  if (CG_LAO_Region_Map != NULL)
    BB_MAP32_Set(CG_LAO_Region_Map, new_bb, ordering);
  //
  return new_bb;
}

// Update a CGIR_BB.
static void
CGIR_BB_update(BasicBlock basicBlock, CGIR_BB cgir_bb, CGIR_LAB labels[], CGIR_OP operations[], CGIR_RID cgir_rid, unsigned optimizations) {
  int unrolled = LAI_Interface_BasicBlock_unrolled(basicBlock);
  int ordering = LAI_Interface_BasicBlock_ordering(basicBlock);
  // Add the labels.
  for (int labelCount = 0; labels[labelCount] != 0; labelCount++) {
    CGIR_LAB cgir_lab = labels[labelCount];
    if (!Is_Label_For_BB(cgir_lab, cgir_bb)) {
      // code borrowed from Gen_Label_For_BB
      Set_Label_BB(cgir_lab, cgir_bb);
      BB_Add_Annotation(cgir_bb, ANNOT_LABEL, (void *)cgir_lab);
    }
  }
  // Add the operations.
  BB_Remove_All(cgir_bb);
  OPS ops = OPS_EMPTY;
  for (int opCount = 0; operations[opCount] != NULL; opCount++) {
    OPS_Append_Op(&ops, operations[opCount]);
    if (OP_unrolling(operations[opCount]) != 0)
      Is_True(OP_unroll_bb(operations[opCount]), ("Unrolled operation with NULL unroll_bb"));
  }
  BB_Append_Ops(cgir_bb, &ops);
  // Remove the LOOPINFO if any.
  ANNOTATION *annot = ANNOT_Get(BB_annotations(cgir_bb), ANNOT_LOOPINFO);
  if (annot != NULL) {
    BB_annotations(cgir_bb) = ANNOT_Unlink(BB_annotations(cgir_bb), annot);
  }
  // Set unrollings.
  if (BB_unrollings(cgir_bb) > 0 && unrolled > 0)
    Set_BB_unrollings(cgir_bb, BB_unrollings(cgir_bb)*unrolled);
  else if (BB_unrollings(cgir_bb) == 0)
    Set_BB_unrollings(cgir_bb, unrolled);
  // Set flags.
  if (optimizations & Optimization_RegAlloc) Set_BB_reg_alloc(cgir_bb);
  if (optimizations & Optimization_PostSched) Set_BB_scheduled(cgir_bb);
  // Set the rid.
  BB_rid(cgir_bb) = cgir_rid;
  //
  if (CG_LAO_Region_Map != NULL)
    BB_MAP32_Set(CG_LAO_Region_Map, cgir_bb, ordering);
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
CGIR_BB_unlink(CGIR_BB cgir_bb, int preds, int succs) {
  BBLIST *edge;
  //
  // Remove successor edges.
  if (succs) {
    FOR_ALL_BB_SUCCS(cgir_bb, edge) {
      BB *succ = BBLIST_item(edge);
      BBlist_Delete_BB(&BB_preds(succ), cgir_bb);
    }
    BBlist_Free(&BB_succs(cgir_bb));
  }
  //
  // Remove predecessor edges.
  if (preds) {
    FOR_ALL_BB_PREDS(cgir_bb, edge) {
      BB *pred = BBLIST_item(edge);
      BBlist_Delete_BB(&BB_succs(pred), cgir_bb);
    }
    BBlist_Free(&BB_preds(cgir_bb));
  }
}

// Create a CGIR_LD.
static CGIR_LD
CGIR_LD_create(LoopInfo loopInfo, CGIR_LD cgir_ld, CGIR_BB head_bb) {
  // LOOP_DESCR are re-created by LOOP_DESCR_Detect_Loops.
  Is_True(0, ("CGIR_LD_create should not be called"));
  return cgir_ld;
}

// Update a CGIR_LD.
static void
CGIR_LD_update(LoopInfo loopInfo, CGIR_LD cgir_ld, CGIR_BB head_bb) {
  int unrolled = LAI_Interface_LoopInfo_unrolled(loopInfo);
  Is_True(head_bb == LOOP_DESCR_loophead(cgir_ld), ("Broken CGIR_LD in CGIR_LD_update"));
  // We only update the LOOPINFOs for use by LOOP_DESCR_Detect_Loops.
  ANNOTATION *annot = ANNOT_Get(BB_annotations(head_bb), ANNOT_LOOPINFO);
  if (annot != NULL) ANNOT_Unlink(BB_annotations(head_bb), annot);
  LOOPINFO *cgir_li = LOOP_DESCR_loopinfo(cgir_ld);
  if (cgir_li != NULL) {
    LOOPINFO *new_li = TYPE_P_ALLOC(LOOPINFO);
    LOOPINFO_wn(new_li) = LOOPINFO_wn(cgir_li);
    if (LOOPINFO_wn(cgir_li) != NULL && unrolled > 1) {
      // Code adapted from Unroll_Dowhile_Loop.
      WN *wn = WN_COPY_Tree(LOOPINFO_wn(cgir_li));
      WN_loop_trip_est(wn) /= unrolled;
      WN_loop_trip_est(wn) += 1;
      LOOPINFO_wn(new_li) = wn;
    }
    LOOPINFO_srcpos(new_li) = LOOPINFO_srcpos(cgir_li);
    BB_Add_Annotation(head_bb, ANNOT_LOOPINFO, new_li);
  }
}

// Initialization of the LIR->CGIR callbacks object.
static void
LIR_CGIR_callback_init(CGIR_CallBack callback) {
    // Initialize the callback pointers.
    *CGIR_CallBack__LAB_create(callback) = CGIR_LAB_create;
    *CGIR_CallBack__LAB_update(callback) = CGIR_LAB_update;
    *CGIR_CallBack__SYM_create(callback) = CGIR_SYM_create;
    *CGIR_CallBack__SYM_update(callback) = CGIR_SYM_update;
    *CGIR_CallBack__Dedicated_TN_create(callback) = CGIR_Dedicated_TN_create;
    *CGIR_CallBack__PseudoReg_TN_create(callback) = CGIR_PseudoReg_TN_create;
    *CGIR_CallBack__AssignReg_TN_create(callback) = CGIR_AssignReg_TN_create;
    *CGIR_CallBack__Modifier_TN_create(callback) = CGIR_Modifier_TN_create;
    *CGIR_CallBack__Absolute_TN_create(callback) = CGIR_Absolute_TN_create;
    *CGIR_CallBack__Symbol_TN_create(callback) = CGIR_Symbol_TN_create;
    *CGIR_CallBack__Label_TN_create(callback) = CGIR_Label_TN_create;
    *CGIR_CallBack__TN_update(callback) = CGIR_TN_update;
    *CGIR_CallBack__OP_create(callback) = CGIR_OP_create;
    *CGIR_CallBack__OP_update(callback) = CGIR_OP_update;
    *CGIR_CallBack__BB_create(callback) = CGIR_BB_create;
    *CGIR_CallBack__BB_update(callback) = CGIR_BB_update;
    *CGIR_CallBack__BB_chain(callback) = CGIR_BB_chain;
    *CGIR_CallBack__BB_unchain(callback) = CGIR_BB_unchain;
    *CGIR_CallBack__BB_link(callback) = CGIR_BB_link;
    *CGIR_CallBack__BB_unlink(callback) = CGIR_BB_unlink;
    *CGIR_CallBack__LD_create(callback) = CGIR_LD_create;
    *CGIR_CallBack__LD_update(callback) = CGIR_LD_update;
}


/*----------------------- LAO Optimization Functions -----------------------*/

// Low-level LAO_optimize entry point.
static bool
lao_optimize(BB_List &bodyBBs, BB_List &entryBBs, BB_List &exitBBs, int pipelining, unsigned lao_optimizations) {
  //
#ifdef Is_True_On
  if (GETENV("CGIR_PRINT")) CGIR_print(TFile);
#endif
  //
  // Get stack model
  int stackmodel = 
    Current_PU_Stack_Model == SMODEL_SMALL   ? 0 : 
    Current_PU_Stack_Model == SMODEL_LARGE   ? 1 :
    Current_PU_Stack_Model == SMODEL_DYNAMIC ? 2 : -1;
  //
  // Open Interface
  LAI_Interface_open(interface, ST_name(Get_Current_PU_ST()),
      Configuration_RegionType, CG_LAO_regiontype,
      Configuration_SchedKind, CG_LAO_schedkind,
      Configuration_Pipelining, CG_LAO_pipelining,
      Configuration_Speculation, CG_LAO_speculation,
      Configuration_LoopDep, CG_LAO_loopdep,
      Configuration_StackModel, stackmodel,
      ConfigurationItem__);
  //
  // Create the LAO BasicBlocks.
  BB_List::iterator bb_iter;
  // First enter the bodyBBs.
  for (bb_iter = bodyBBs.begin(); bb_iter != bodyBBs.end(); bb_iter++) {
    BasicBlock basicblock = CGIR_BB_to_BasicBlock(*bb_iter);
    LAI_Interface_setBody(interface, basicblock);
    //fprintf(TFile, "BB_body(%d)\n", BB_id(*bb_iter));
  }
  // Then enter the entryBBs.
  for (bb_iter = entryBBs.begin(); bb_iter != entryBBs.end(); bb_iter++) {
    BasicBlock basicblock = CGIR_BB_to_BasicBlock(*bb_iter);
    LAI_Interface_setEntry(interface, basicblock);
    //fprintf(TFile, "BB_entry(%d)\n", BB_id(*bb_iter));
  }
  // Last enter the exitBBs.
  for (bb_iter = exitBBs.begin(); bb_iter != exitBBs.end(); bb_iter++) {
    BasicBlock basicblock = CGIR_BB_to_BasicBlock(*bb_iter);
    LAI_Interface_setExit(interface, basicblock);
    //fprintf(TFile, "BB_exit(%d)\n", BB_id(*bb_iter));
  }
  // Make the control-flow nodes and the control-flow arcs.
  for (bb_iter = bodyBBs.begin(); bb_iter != bodyBBs.end(); bb_iter++) {
    BB *orig_bb = *bb_iter;
    BasicBlock tail_block = CGIR_BB_to_BasicBlock(orig_bb);
    if (lao_in_bblist(exitBBs, orig_bb)) continue;
    BBLIST *bblist = NULL;
    FOR_ALL_BB_SUCCS(orig_bb, bblist) {
      BB *succ_bb = BBLIST_item(bblist);
      BasicBlock head_block = CGIR_BB_to_BasicBlock(succ_bb);
      LAI_Interface_linkBasicBlocks(interface, tail_block, head_block, BBLIST_prob(bblist));
    }
  }
  //
  // Make the LoopInfos for the bodyBBs.
  // Get loop headers and function entry that may have a pseudo loop descr
  for (bb_iter = bodyBBs.begin(); bb_iter != bodyBBs.end(); bb_iter++) {
    BB *bb = *bb_iter;
    if (BB_loophead(bb) || BB_entry(bb)) {
      LOOP_DESCR *loop = LOOP_DESCR_Find_Loop(bb);
      if (loop != NULL && LOOP_DESCR_loophead(loop) == bb) {
	LoopInfo loopinfo = CGIR_LD_to_LoopInfo(loop);
      } else {
	// Note, the loophead is not reset by find loops, thus
	// we may have a block having it set while it's no more a loophead.
	if (BB_loophead(bb) && !BB_unrolled_fully(bb)) {
	  DevWarn("Inconsistent loop information for BB %d", BB_id(bb));
	}
      }
    }
  }
  //
  unsigned optimizations = LAI_Interface_optimize(interface, lao_optimizations);
  if (optimizations != 0) {
    LAI_Interface_updateCGIR(interface, callback);
#ifdef Is_True_On
    if (GETENV("CGIR_PRINT")) CGIR_print(TFile);
#endif
  }
  //
  LAI_Interface_close(interface);
  //
  return optimizations != 0;
}

//
// Makes a pesudo loop descr for a single entry region at nesting level 0.
//
static LOOP_DESCR *
make_pseudo_loopdescr(BB *entry, BB_List &bodyBBs, BB_List &exitBBs, MEM_POOL *pool) {
  LOOP_DESCR *newloop = TYPE_L_ALLOC (LOOP_DESCR);
  BB_SET *loop_set = BB_SET_Create_Empty (PU_BB_Count + 2, pool);
  newloop->mem_pool = pool;
  LOOP_DESCR_bbset(newloop) = loop_set;
  LOOP_DESCR_loophead(newloop) = entry;
  LOOP_DESCR_nestlevel(newloop) = 0;
  LOOP_DESCR_num_exits(newloop) = 0;
  LOOP_DESCR_next(newloop) = NULL;
  //
  // Fill the loop bb set
  BB_List::iterator bb_iter;
  for (bb_iter = bodyBBs.begin(); bb_iter != bodyBBs.end(); bb_iter++) {
    BB_SET_Union1D(loop_set, *bb_iter, NULL);
    if (!BB_MAP_Get(LOOP_DESCR_map, *bb_iter)) {
      // Set the LOOP_DESCR_map to that LOOP_DESCR_Find_Loop will return newloop.
      BB_MAP_Set(LOOP_DESCR_map, *bb_iter, newloop);
    }
  }
  //
  // Count the exitBBs.
  int n_exit = 0;
  for (bb_iter = exitBBs.begin(); bb_iter != exitBBs.end(); bb_iter++) {
    n_exit++;
  }
  LOOP_DESCR_num_exits(newloop) = n_exit;
  return newloop;
}

// Optimize the complete PU through the LAO.
bool
lao_optimize_pu(unsigned lao_optimizations) {
//cerr << "lao_optimize_PU(" << lao_optimizations << ")\n";
  bool result = false;
  MEM_POOL lao_loop_pool;
  MEM_POOL_Initialize(&lao_loop_pool, "LAO loop_descriptors", TRUE);
  //
  MEM_POOL_Push(&lao_loop_pool);
  Calculate_Dominators();
  LOOP_DESCR_Detect_Loops(&lao_loop_pool);
  //
  // List the BBs, entry BBs, exit BBs, body BBs.
  int n_entry = 0, n_exit = 0;
  BB_List entryBBs, exitBBs, bodyBBs;
  for (BB *bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    if (BB_entry(bb)) { entryBBs.push_back(bb); n_entry++; }
    if (BB_exit(bb)) { exitBBs.push_back(bb); n_exit++; }
    bodyBBs.push_back(bb);
  }
  // Create a pseudo loop descr for the function entry if not done
  if (n_entry == 1) {
    BB *entry = *entryBBs.begin();
    LOOP_DESCR *loop = LOOP_DESCR_Find_Loop(entry);
    if (loop == NULL) make_pseudo_loopdescr(entry, bodyBBs, exitBBs, &lao_loop_pool);
  }
  //
  // Create region map for postpass optimizations
  if (lao_optimizations & Optimization_PostSched) {
    CG_LAO_Region_Map = BB_MAP32_Create();
  }
  //
  // Call the lower level lao_optimize function with pipelining=0.
  result |= lao_optimize(bodyBBs, entryBBs, exitBBs, 0, lao_optimizations);
  //
  if (result) {
    GRA_LIVE_Recalc_Liveness(NULL);
    GRA_LIVE_Rename_TNs();
  }
  //
  Free_Dominators_Memory();
  MEM_POOL_Pop(&lao_loop_pool);
  //
  MEM_POOL_Delete(&lao_loop_pool);
  return result;
}

#ifdef Is_True_On
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
CGIR_TN_print ( const TN *tn, FILE *file )
{
  //
  if (TN_is_constant(tn)) {
    if ( TN_has_value(tn)) {
      fprintf ( file, "(0x%llx)", TN_value(tn) );
      if (TN_size(tn) == 4 && 
	  TN_value(tn) >> 32 != 0 &&
	  TN_value(tn) >> 31 != -1)
	fprintf ( file, "!!! TN_value=0x%llx is too big to fit in a word",
		  TN_value(tn));
    }
    else if (TN_is_enum(tn)) {
      fprintf ( file, "(enum:%s)", ISA_ECV_Name(TN_enum(tn)) );
    }
    else if ( TN_is_label(tn) ) {
      CGIR_LAB lab = TN_label(tn);
      const char *name = LABEL_name(lab);
      INT64 offset = TN_offset(tn);
      BB *targetBB;
      if ( offset == 0 ) {
	fprintf ( file, "(lab:%s)", name );
      }
      else {
	fprintf ( file, "(lab:%s+%lld)", name, offset );
      }
      targetBB = Get_Label_BB(lab); // XXX
      if (targetBB != NULL)
	fprintf(file, " --> %d", BB_id(targetBB));
    } 
    else if ( TN_is_tag(tn) ) {
      CGIR_LAB lab = TN_label(tn);
      const char *name = LABEL_name(lab);
      fprintf ( file, "(tag:%s)", name );
    }
    else if ( TN_is_symbol(tn) ) {
      ST *var = TN_var(tn);
      //
      fprintf ( file, "(sym" );
      fprintf ( file, TN_RELOCS_Name(TN_relocs(tn)) );
      //
      if (ST_class(var) == CLASS_CONST)
      	fprintf ( file, ":%s)", Targ_Print(NULL, ST_tcon_val(var)));
      else
      	fprintf ( file, ":%s%+lld)", ST_name(var), TN_offset(tn) );
    } 
    else {
      ErrMsg (EC_Unimplemented, "CGIR_TN_print: illegal constant TN");
    }
  }
  else {  /* register TN */
    if (TN_register(tn) != REGISTER_UNDEFINED) {
      if (TN_register(tn) <= REGISTER_CLASS_last_register(TN_register_class(tn))) {
	fprintf ( file, "%s", 
		  REGISTER_name(TN_register_class(tn), TN_register(tn)));
      } else {
	fprintf ( file, "(%d,%d)", TN_register_class(tn), TN_register(tn));
      }
    }
    else if (TN_is_global_reg(tn)) {
      fprintf ( file, "G%d", TN_number(tn) );
    }
    else {
      fprintf ( file, "T%d", TN_number(tn) );
    }
    if (TN_is_save_reg(tn)) {
      fprintf ( file, "(sv:%s)", 
		REGISTER_name(TN_save_rclass(tn), TN_save_reg(tn)));
    }
  }
}

static void
CGIR_OP_print ( const OP *op, bool bb_scheduled, FILE *file)
{
  int i;
  //
  //
  {
    LABEL_IDX tag = Get_OP_Tag(op);
    if(tag) fprintf (file, "<tag %s>: ", LABEL_name(tag));
  }
  //
  fprintf(file, "%s", TOP_Name(OP_code(op)));
  //
  if ( OP_variant(op) != 0 ) {
    fprintf ( file, "(%x)", OP_variant(op));
  }
  //
  if (OP_results(op) == 0)
    fprintf(file, " void");
  //
  else for (i = 0; i < OP_results(op); i++) {
    fprintf(file, "%s", (i == 0 ? " " : ", "));
    CGIR_TN_print(OP_result(op,i), file);
  }
  //
  fprintf(file, " =");
  //
  for (i=0; i<OP_opnds(op); i++) {
    fprintf(file, "%s", (i == 0 ? " " : ", "));
    TN *tn = OP_opnd(op,i);
    CGIR_TN_print(tn, file);
    if (OP_Defs_TN(op, tn)) fprintf(file, "<def>");
  }
  //if (bb_scheduled)
    fprintf(file, "\tscycle = %d", OP_scycle(op));
  // TBD: Print other attributes on operations.
}

static void
CGIR_OPS_print ( const OPS *ops , bool bb_scheduled, FILE *file)
{
  for (OP *op = OPS_first(ops) ; op; op = OP_next(op)) {
    fprintf(file, "\t");
    CGIR_OP_print(op, bb_scheduled, file);
    fprintf(file, "       \t#line[%4d]", Srcpos_To_Line(OP_srcpos(op)));
    fprintf(file, "\n");
  }
}

static void
CGIR_BB_print_header (BB *bp, FILE *file)
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
    fprintf ( file, "Entrypoint: %s\t Starting Line %d\n",
	      ST_name(ENTRYINFO_name(ent)),
	      Srcpos_To_Line(ENTRYINFO_srcpos(ent)));
    //
    if (sp_adj) {
      OP *op;
      BOOL found_sp_adj = FALSE;
      fprintf ( file, "SP entry adj: " );
      Print_OP_No_SrcLine (sp_adj);
      FOR_ALL_BB_OPs_FWD(bp,op)
	if (op == sp_adj) {
	  found_sp_adj = TRUE;
	  break;
	}
      if (found_sp_adj == FALSE)
	fprintf ( file, "******** ERROR ******** sp adjust not found in entry block\n");
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
      fprintf ( file, "SP exit adj: " );
      Print_OP_No_SrcLine (sp_adj);
      //
      FOR_ALL_BB_OPs_FWD(bp,op)
	if (op == sp_adj) {
	  found_sp_adj = TRUE;
	  break;
	}
      if (found_sp_adj == FALSE)
	fprintf ( file, "******** ERROR ******** sp adjust not found in exit block\n");
    }
  }
  //
  fprintf ( file, "    BB %d, flags 0x%04x",
	    BB_id(bp), BB_flag(bp) );
  //
  if (freqs || BB_freq_fb_based(bp))
    fprintf(file, ", freq %g (%s)", BB_freq(bp),
	    BB_freq_fb_based(bp) ? "feedback" : "heuristic");
  
  if (BB_scheduled(bp)) fprintf ( file, ", Scheduled");
  if (BB_unreachable(bp)) fprintf ( file, ", Unreachable");
  if (BB_entry(bp))	fprintf ( file, ", Entry" );
  if (BB_handler(bp))	fprintf ( file, ", Handler" );
  if (BB_asm(bp)) 	fprintf ( file, ", Asm" );
  //
  if (BB_exit(bp)) {
    if (BB_call(bp))	fprintf ( file, ", Tail-call" );
    else		fprintf ( file, ", Exit" );
  } else if (BB_call(bp)) fprintf ( file, ", Call" );
  //
  if (BB_rid(bp)) {
    INT exits;
    RID *rid = BB_rid(bp);
    CGRIN *cgrin = RID_cginfo(rid);
    if (cgrin) {
      if (bp == CGRIN_entry(cgrin)) {
	fprintf ( file, ", Region-entry " );
      }
      exits = RID_num_exits(rid);
      for (i = 0; i < exits; ++i) {
	if (bp == CGRIN_exit_i(cgrin, i)) {
	  fprintf ( file, ", Region-exit[%d]", i );
	}
      }
    }
  }
  //
  fprintf ( file, "\n");
  //
  if (annot)
    Print_LOOPINFO(ANNOT_loopinfo(annot));
  //
  if (BB_loop_head_bb(bp)) {
    if (BB_loophead(bp)) {
      if (!annot) {
	fprintf(file, "\tHead of loop body line %d\n", BB_Loop_Lineno(bp));
      }
    } else {
      BB *head = BB_loop_head_bb(bp);
      fprintf(file,
	      "\tPart of loop body starting at line %d with head BB:%d\n",
	      BB_Loop_Lineno(head), BB_id(head));
    }
  }
  //
  if (BB_unrollings(bp) > 1)
    fprintf(file, "\tUnrolled %d times%s\n", BB_unrollings(bp),
	    BB_unrolled_fully(bp) ? " (fully)" : "");
  //
  if ( BB_rid(bp) )
    RID_Fprint( file, BB_rid(bp) );
  //
  fprintf ( file, "\tpred" );
  FOR_ALL_BB_PREDS (bp, bl) {
    fprintf ( file, " %d", BB_id(BBLIST_item(bl)));
  }
  //
  fprintf ( file, "\n\tsucc%s", freqs ? " (w/probs)" : "" );
  FOR_ALL_BB_SUCCS (bp, bl) {
    fprintf ( file, " %d",
	      BB_id(BBLIST_item(bl)));
    if (freqs) fprintf(file, "(%g)", BBLIST_prob(bl));
  }
  fprintf ( file, "\n" );
  //
  if (BB_has_label(bp)) {
    ANNOTATION *ant;
    fprintf(file, "\tLabel");
    for (ant = ANNOT_First(BB_annotations(bp), ANNOT_LABEL);
	 ant != NULL;
	 ant = ANNOT_Next(ant, ANNOT_LABEL))
      {
	INT eh_labs = 0;
	LABEL_IDX lab = ANNOT_label(ant);
	fprintf (file," %s", LABEL_name(lab));
	FmtAssert((Get_Label_BB(lab) == bp),
		  (" Inconsistent ST for BB:%2d label", BB_id(bp)));
	switch (LABEL_kind(Label_Table[lab])) {
	case LKIND_BEGIN_EH_RANGE:
	  fprintf (file,"%cbegin_eh_range", eh_labs++ ? ' ' : '(');
	  break;
	case LKIND_END_EH_RANGE:
	  fprintf (file,"%cend_eh_range", eh_labs++ ? ' ' : '(');
	  break;
	}
	if (eh_labs)
	  fprintf (file,")");
      }
    fprintf(file, "\n");
  }
  //
  return;
}

static void
CGIR_BB_print (BB *bp, FILE *file)
{
  CGIR_BB_print_header (bp, file);
  if (BB_first_op(bp))	CGIR_OPS_print (&bp->ops, BB_scheduled(bp), file);
}

static void
CGIR_Alias_print(FILE *file)
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
  fprintf(file, "--------------- Begin Print Alias ---------------\n");
  //
  for (elt1 = memops; elt1; elt1 = elt1->next) {
    fprintf(file, "<Alias>"); CGIR_OP_print(elt1->op, FALSE, file); fprintf(file, "\n");
    for (elt2 = memops; elt2 != elt1; elt2 = elt2->next) {
      fprintf(file, "\t<with>"); CGIR_OP_print(elt2->op, FALSE, file); fprintf(file, "\t");
      alias = CG_DEP_Mem_Ops_Alias(elt1->op, elt2->op, &identical);
      if (!alias)          fprintf(file, "NO-ALIAS");
      else if (!identical) fprintf(file, "   ALIAS");
      else                 fprintf(file, "IDENTICAL");
      fprintf(file, "</with>\n");
    }
    fprintf(file, "</Alias>\n");
  }
  //
  fprintf(file, "---------------- End Print Alias ----------------\n");
}

static void
CGIR_print( FILE *file)
{
  BB *bp;
  //
  fprintf(file, "--------CFG Begin--------\n");
  for (bp = REGION_First_BB; bp; bp = BB_next(bp)) {
    CGIR_BB_print ( bp, file );
    fprintf ( file,"\n" );
  }
  //
  //CGIR_Alias_print(file);
  fprintf(file, "-------- CFG End --------\n");
}

#endif // Is_True_On
