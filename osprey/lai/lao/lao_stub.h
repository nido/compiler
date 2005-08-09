#define __GXX_ABI_VERSION 102
#define __SIZE_TYPE__ unsigned int
#define __PTRDIFF_TYPE__ int
#define __WCHAR_TYPE__ long int
#define __WINT_TYPE__ long int
#define __SCHAR_MAX__ 127
#define __SHRT_MAX__ 32767
#define __INT_MAX__ 2147483647
#define __LONG_MAX__ 2147483647L
#define __LONG_LONG_MAX__ 9223372036854775807LL
#define __WCHAR_MAX__ 2147483647
#define __CHAR_BIT__ 8
#define __FLT_EVAL_METHOD__ 0
#define __FLT_RADIX__ 2
#define __FLT_MANT_DIG__ 24
#define __FLT_DIG__ 6
#define __FLT_MIN_EXP__ (-125)
#define __FLT_MIN_10_EXP__ (-37)
#define __FLT_MAX_EXP__ 128
#define __FLT_MAX_10_EXP__ 38
#define __FLT_MAX__ 3.40282347e+38F
#define __FLT_MIN__ 1.17549435e-38F
#define __FLT_EPSILON__ 1.19209290e-7F
#define __FLT_DENORM_MIN__ 1.40129846e-45F
#define __DBL_MANT_DIG__ 53
#define __DBL_DIG__ 15
#define __DBL_MIN_EXP__ (-1021)
#define __DBL_MIN_10_EXP__ (-307)
#define __DBL_MAX_EXP__ 1024
#define __DBL_MAX_10_EXP__ 308
#define __DBL_MAX__ 1.7976931348623157e+308
#define __DBL_MIN__ 2.2250738585072014e-308
#define __DBL_EPSILON__ 2.2204460492503131e-16
#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
#define __LDBL_MANT_DIG__ 113
#define __LDBL_DIG__ 33
#define __LDBL_MIN_EXP__ (-16381)
#define __LDBL_MIN_10_EXP__ (-4931)
#define __LDBL_MAX_EXP__ 16384
#define __LDBL_MAX_10_EXP__ 4932
#define __DECIMAL_DIG__ 36
#define __LDBL_MAX__ 1.18973149535723176508575932662800702e+4932L
#define __LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L
#define __LDBL_EPSILON__ 1.92592994438723585305597794258492732e-34L
#define __LDBL_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966L
#define __REGISTER_PREFIX__ 
#define __USER_LABEL_PREFIX__ 
#define __VERSION__ "3.3.3"
#define __STDC_HOSTED__ 1
#define __NO_INLINE__ 1
#define __FINITE_MATH_ONLY__ 0
#define __unix 1
#define __unix__ 1
#define unix 1
#define __sun 1
#define __sun__ 1
#define sun 1
#define __svr4__ 1
#define __SVR4 1
#define __PRAGMA_REDEFINE_EXTNAME 1


#define __GNUC__ 3
#define __GNUC_MINOR__ 3
#define __GNUC_PATCHLEVEL__ 3
#define sparc 1
#define __sparc__ 1
#define __sparc 1
#define __GCC_NEW_VARARGS__ 1


/*
  ConfigureRegionType -- enumerates the ConfigureRegionType types.
*/
typedef enum {
  ConfigureRegionType_BasicBlock,
  ConfigureRegionType_SuperBlock,
  ConfigureRegionType_TraceBlock,
  ConfigureRegionType_HyperBlock,
  ConfigureRegionType_BasicInner,
  ConfigureRegionType_SuperInner,
  ConfigureRegionType_TraceInner,
  ConfigureRegionType_HyperInner,
  ConfigureRegionType__
} ConfigureRegionType;
extern const char *
ConfigureRegionType_NAME_[];
#define ConfigureRegionType_NAME_(type) ConfigureRegionType_NAME_[type]

/*
  ConfigureSchedKind -- Enumeration of the scheduling kinds.
*/
typedef enum {
  ConfigureSchedKind_Scoreboard,
  ConfigureSchedKind_GrahamList,
  ConfigureSchedKind_Insertion,
  ConfigureSchedKind_Formulation,
  ConfigureSchedKind__
} ConfigureSchedKind;
extern const char *
ConfigureSchedKind_NAME_[];
#define ConfigureSchedKind_NAME_(type) ConfigureSchedKind_NAME_[type]

/*
  ConfigureAllocKind -- Enumeration of the allocation kinds.
*/
typedef enum {
  ConfigureAllocKind_Localize,
  ConfigureAllocKind_LinearScan,
  ConfigureAllocKind_GraphColor,
  ConfigureAllocKind_Integrated,
  ConfigureAllocKind__
} ConfigureAllocKind;
extern const char *
ConfigureAllocKind_NAME_[];
#define ConfigureAllocKind_NAME_(type) ConfigureAllocKind_NAME_[type]

/*
  ConfigureRenaming -- Enumeration of the renaming levels.
*/
typedef enum {
  ConfigureRenaming_None,
  ConfigureRenaming_Normal,
  ConfigureRenaming_ModuloLocal,
  ConfigureRenaming_ModuloGeneral,
  ConfigureRenaming__
} ConfigureRenaming;
extern const char *
ConfigureRenaming_NAME_[];
#define ConfigureRenaming_NAME_(type) ConfigureRenaming_NAME_[type]

/*
  ConfigureStackModel -- Stack frame model.
*/
typedef enum {
  ConfigureStackModel_Small,
  ConfigureStackModel_Large,
  ConfigureStackModel_Dynamic,
  ConfigureStackModel__
} ConfigureStackModel;
extern const char *
ConfigureStackModel_NAME_[];
#define ConfigureStackModel_NAME_(type) ConfigureStackModel_NAME_[type]

/*
  ConfigureLoopDep -- Loop dependence model.
*/
typedef enum {
  ConfigureLoopDep_Serial,
  ConfigureLoopDep_Scalar,
  ConfigureLoopDep_Vector,
  ConfigureLoopDep_Parallel,
  ConfigureLoopDep_Liberal,
  ConfigureLoopDep__
} ConfigureLoopDep;
extern const char *
ConfigureLoopDep_NAME_[];
#define ConfigureLoopDep_NAME_(type) ConfigureLoopDep_NAME_[type]

/*
  ConfigureItem -- Enumerate the Configure items.
//
  Each ConfigureItem can have a negative value, meaning that the value should be
  ignored.
*/
typedef enum {
  ConfigureItem_LogMaxBBOC, // Log2 of maximum operation count in BasicBlock.
  ConfigureItem_RegionType, // See ConfigureRegionType.
  ConfigureItem_SchedKind, // See ConfigureSchedKind.
  ConfigureItem_AllocKind, // See ConfigureAllocKind.
  ConfigureItem_LogUnwind, // Log2 of default unwind factor.
    // 0 => no unwind,
    // 1 => unwind 2,
    // n => unwind 1<<n.
  ConfigureItem_LogUnroll, // Log2 of default unroll factor.
    // 0 => no unroll,
    // 1 => unroll 2,
    // n => unroll 1<<n.
  ConfigureItem_Compensation, // Code compensation level.
  ConfigureItem_Speculation, // Control speculation level.
    // 0 => no software speculation,
    // 1 => software speculation of non-excepting instructions.
    // 2 => software speculation of dismissable instructions (advanced LOADs).
    // 3 => software speculation of non-dismissable instructions (regular LOADs).
  ConfigureItem_Relaxation, // Dependence relaxation level.
    // 0 => no inductive relaxation,
    // 1 => inductive relaxation without compensation,
    // 2 => inductive relaxation with and without compensation.
  ConfigureItem_Pipelining, // Software pipelining level.
    // 0 => cyclic instruction schedule,
    // 1 => software pipelining with overlap 1,
    // n => software pipelining with overlap (1<<n)-1.
  ConfigureItem_Renaming, // See ConfigureRenaming.
  ConfigureItem_LoopOpt, // Targets of loop optimizations.
    // 0 => no loop optimizations,
    // 1 => innermost loops only,
    // n => n innermost loops.
  ConfigureItem_LoopDep, // See ConfigureLoopDep.
  ConfigureItem_MinTrip, // Minimum value of loop trip count.
  ConfigureItem_Modulus, // Modulus of loop trip count.
  ConfigureItem_Residue, // Residue of loop trip count.
  ConfigureItem_StackModel, // See ConfigureStackModel.
  ConfigureItem_MaxIssue, // Override the maximum issue width.
  ConfigureItem_MinTaken, // Minimum branch taken penalty.
  ConfigureItem__
} ConfigureItem;
extern const char *
ConfigureItem_NAME_[];
#define ConfigureItem_NAME_(type) ConfigureItem_NAME_[type]

/*
  DependenceKind -- Enumerates the Dependence kinds.
*/
typedef enum {
  DependenceKind_Flow, // Flow memory dependence arc.
  DependenceKind_Anti, // Anti memory dependence arc.
  DependenceKind_Input, // Input memory dependence arc.
  DependenceKind_Output, // Output memory dependence arc.
  DependenceKind_Spill, // Spill memory dependence arc.
  DependenceKind_Other, // Other memory dependence arc.
  DependenceKind_Seq, // Sequentialize the volatile operations.
  DependenceKind_RAW, // Register Read After Write dependence arc.
  DependenceKind_WAR, // Register Write After Read dependence arc.
  DependenceKind_WAW, // Register Write After Write dependence arc.
  DependenceKind_Life, // Register consumer node to its lifetime node.
  DependenceKind_Link, // Register producer node to its lifetime node.
  DependenceKind_None, // Inserted by DepGraph_complete.
  DependenceKind_Root, // Inserted by DepGraph_complete.
  DependenceKind__,
  DependenceKind_Definite = 16,
} DependenceKind;
extern const char *
DependenceKind_NAME_[];
#define DependenceKind(type) ((type) & (DependenceKind_Definite - 1))
#define DependenceKind_NAME_(type) DependenceKind_NAME_[(type)]
#define DependenceKind_mayAdjust(type) ((type) <= DependenceKind_WAW)
#define DependenceKind_mayRefine(type) ((type) <= DependenceKind_Spill)
#define DependenceKind_isActive(type) ( (type) >= DependenceKind_RAW && (type) <= DependenceKind_Link )




/*
  OptimizerPhase -- Enumerate the LAO optimizer phases.
*/
typedef enum {
  OptimizerPhase_NativeCode = 0x1,
  OptimizerPhase_PostSched = 0x2,
  OptimizerPhase_Allocate = 0x4,
  OptimizerPhase_PreSched = 0x8,
  OptimizerPhase_Transform = 0x20,
  OptimizerPhase_EnableSSA = 0x40,
  OptimizerPhase_Predicate = 0x80,
  OptimizerPhase_EnableSSI = 0x100,
  // Used to force activation of the LAO interface.
  OptimizerPhase_ForcePostPass = 0x1000,
  OptimizerPhase_ForceRegAlloc = 0x2000,
  OptimizerPhase_ForcePrePass = 0x4000
} OptimizerPhase;
extern struct ETable_ *OptimizerPhaseTable;
#define OptimizerPhase_MustPostPass ( OptimizerPhase_NativeCode | OptimizerPhase_PostSched | OptimizerPhase_ForcePostPass | 0)




#define OptimizerPhase_MustRegAlloc ( OptimizerPhase_Allocate | OptimizerPhase_ForceRegAlloc | 0)



#define OptimizerPhase_MustPrePass ( OptimizerPhase_PreSched | OptimizerPhase_Transform | OptimizerPhase_EnableSSA | OptimizerPhase_Predicate | OptimizerPhase_EnableSSI | OptimizerPhase_ForcePrePass | 0)
extern void lao_init(void);
extern void lao_fini(void);
extern void lao_init_pu(void);
extern void lao_fini_pu(void);
extern bool lao_optimize_pu(unsigned);
