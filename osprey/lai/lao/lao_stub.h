

#define i386 1
#define linux 1
#define unix 1



/*
 * OptimizeActivation --	Activation of LAO phases.
 */
typedef enum {
  OptimizeActivation_Encode = 0x1, // Encoding of instructions.
  OptimizeActivation_PostPass = 0x2, // Post-pass optimizations.
  OptimizeActivation_RegAlloc = 0x4, // Register allocation
  OptimizeActivation_PrePass = 0x8, // Pre-pass optimizations.
} OptimizeActivation;

/*
 * OptimizeRegionType --	enumerates the OptimizeRegionType types.
 */
typedef enum {
  OptimizeRegionType_BasicBlock,
  OptimizeRegionType_SuperBlock,
  OptimizeRegionType_TraceBlock,
  OptimizeRegionType_HyperBlock,
  OptimizeRegionType_BasicInner,
  OptimizeRegionType_SuperInner,
  OptimizeRegionType_TraceInner,
  OptimizeRegionType_HyperInner,
  OptimizeRegionType__
} OptimizeRegionType;
typedef uint8_t short_OptimizeRegionType;
extern const char *
OptimizeRegionType_NAME_[];
#define OptimizeRegionType_NAME_(type) OptimizeRegionType_NAME_[type]

/*
 * OptimizeProfiling --	Enumeration of the code profiling levels.
 */
typedef enum {
  OptimizeProfiling_, // No code profiling.
  OptimizeProfiling_Static, // Static profiling
  OptimizeProfiling_Edge, // Dynamic edge profiling.
  OptimizeProfiling_Path, // Dynamic path profiling.
  OptimizeProfiling__
} OptimizeProfiling;
typedef uint8_t short_OptimizeProfiling;
extern const char *
OptimizeProfiling_NAME_[];
#define OptimizeProfiling_NAME_(type) OptimizeProfiling_NAME_[type]

/*
 * OptimizeScheduling --	Enumeration of the pre- and post-scheduler levels.
 */
typedef enum {
  OptimizeScheduling_, // No scheduling.
  OptimizeScheduling_GrahamList, // Graham list scheduling.
  OptimizeScheduling_Insertion, // Insertion scheduling.
  OptimizeScheduling_Formulation, // RCMSP Formulation scheduling.
  OptimizeScheduling__
} OptimizeScheduling;
typedef uint8_t short_OptimizeScheduling;
extern const char *
OptimizeScheduling_NAME_[];
#define OptimizeScheduling_NAME_(type) OptimizeScheduling_NAME_[type]

/*
 * OptimizeScoreboarding --	Scoreboarder levels.
 */
typedef enum {
  OptimizeScoreboarding_, // No scoreboard schedule.
  OptimizeScoreboarding_Fixup, // Fixup pass over the BasicBlock(s).
  OptimizeScoreboarding_Iterate, // Iterate forward data-flow problem.
  OptimizeScoreboarding_Priority, // Pre-order Operation(s) by priority.
  OptimizeScoreboarding__
} OptimizeScoreboarding;
typedef uint8_t short_OptimizeScoreboarding;
extern const char *
OptimizeScoreboarding_NAME_[];
#define OptimizeScoreboarding_NAME_(type) OptimizeScoreboarding_NAME_[type]

/*
 * OptimizeLiveness --	Liveness analysis precision level.
 */
typedef enum {
  OptimizeLiveness_Upward, // Upward exposed uses approximation.
  OptimizeLiveness_Linear, // Linear pass to improve upon Upward.
  OptimizeLiveness_Iterate, // Iterate backward data-flow problem.
  OptimizeLiveness__
} OptimizeLiveness;
typedef uint8_t short_OptimizeLiveness;
extern const char *
OptimizeLiveness_NAME_[];
#define OptimizeLiveness_NAME_(type) OptimizeLiveness_NAME_[type]

/*
 * OptimizeConversion --	SSA form conversion flags.
 */
typedef enum {
  OptimizeConversion_Enable = 0x1, // Enable SSA conversion.
  OptimizeConversion_Folding = 0x2, // Copy Folding during SSA construction.
  OptimizeConversion_Cleaning = 0x4, // Code Cleaning during SSA construction.
  OptimizeConversion_Numbering = 0x8, // Value Numbering during SSA construction.
  OptimizeConversion_MethodIII = 0x10, // Sreedhar method III for SSA destruction.
  OptimizeConversion_Coalescing = 0x20, // Copy coalescing before SSA destruction.
} OptimizeConversion;

/*
 * OptimizePredication --	Predication level.
 */
typedef enum {
  OptimizePredication_, // Disable code predication.
  OptimizePredication_Select, // Select-only code predication.
  OptimizePredication__
} OptimizePredication;
typedef uint8_t short_OptimizePredication;
extern const char *
OptimizePredication_NAME_[];
#define OptimizePredication_NAME_(type) OptimizePredication_NAME_[type]

/*
 * OptimizeFormulation --	Enumerate the Formulation flags.
 */
typedef enum {
  OptimizeFormulation_Integral = 0x1, // Require integral solutions.
  OptimizeFormulation_EicDavAb = 0x2, // Formulation of Eichenberger et al.
  OptimizeFormulation_AggrDeps = 0x4, // Use aggregated dependences.
  OptimizeFormulation_Makespan = 0x10, // Minimize the makespan.
  OptimizeFormulation_Lifetime = 0x20, // Minimize the register lifetimes.
  OptimizeFormulation_Scanning = 0x40, // Use the scanning heuristic.
  OptimizeFormulation_PostPass = 0x80, // Apply to postpass problems.
  OptimizeFormulation_Acyclic = 0x100, // Apply to acyclic problems.
} OptimizeFormulation;
typedef unsigned OptimizeFormulations;
typedef uint16_t short_OptimizeFormulations;
#define OptimizeFormulations_isOptimize(flags) ( flags & ( OptimizeFormulation_Makespan | OptimizeFormulation_Lifetime ) )






/*
 * OptimizeItem --	Enumerate the Optimize items.
 *
 * Each OptimizeItem maps to a set of flags.
 */
typedef enum {
  OptimizeItem_Activation, // See OptimizeActivation.
  OptimizeItem_LogMaxBBOC, // Log2 of max BasicBlock Operation count.
  OptimizeItem_Convention, // Calling Convention in use.
  OptimizeItem_Profiling, // See OptimizeProfiling.
  OptimizeItem_RegionType, // See OptimizeRegionType.
  OptimizeItem_Liveness, // See OptimizeLiveness.
  OptimizeItem_Conversion, // See OptimizeConversion.
  OptimizeItem_Predication, // See OptimizePredication.
  OptimizeItem_Scheduling, // See OptimizeScheduling.
  OptimizeItem_Allocation, // See OptimizeAllocation.
  OptimizeItem_Formulation, // See OptimizeFormulation.
  OptimizeItem_Scoreboarding, // See OptimizeScoreboarding.
  OptimizeItem_WindowSize, // Scoreboarder window size
  OptimizeItem_PrePadding, // See OptimizePrePadding.
  OptimizeItem_PostPadding, // See OptimizePostPadding.
  OptimizeItem_L1MissExtra, // Extra latency of L1 miss.
  OptimizeItem__
} OptimizeItem;
typedef uint8_t short_OptimizeItem;
extern const char *
OptimizeItem_NAME_[];
#define OptimizeItem_NAME_(type) OptimizeItem_NAME_[type]

/*
 * ConfigureCompensation --	Enumeration of the code compensation levels.
 *
 * Code compensation results from code motion down a branch.
 */
typedef enum {
  ConfigureCompensation_, // No code compensation.
  ConfigureCompensation_Local, // Local code compensation.
  ConfigureCompensation_Global, // Global code compensation.
  ConfigureCompensation__
} ConfigureCompensation;
typedef uint8_t short_ConfigureCompensation;
extern const char *
ConfigureCompensation_NAME_[];
#define ConfigureCompensation_NAME_(type) ConfigureCompensation_NAME_[type]

/*
 * ConfigureSpeculation --	Enumeration of the control speculation levels.
 * 
 * Speculation is the execution of code under a more general condition than original.
 */
typedef enum {
  ConfigureSpeculation_, // No control speculation.
  ConfigureSpeculation_SafeArith, // Speculation of safe arithmetic operations.
  ConfigureSpeculation_SafeAccess, // Speculation of safe memory accesses.
  ConfigureSpeculation_SafeVariant, // Speculation of operations with safe variant.
  ConfigureSpeculation_MayExcept, // Speculation of may be excepting operations.
  ConfigureSpeculation__
} ConfigureSpeculation;
typedef uint8_t short_ConfigureSpeculation;
extern const char *
ConfigureSpeculation_NAME_[];
#define ConfigureSpeculation_NAME_(type) ConfigureSpeculation_NAME_[type]

/*
 * ConfigureRelaxation --	Enumeration of the inductive relaxation levels.
 *
 * Inductive relaxation enables to relax dependences on simple induction variables.
 */
typedef enum {
  ConfigureRelaxation_, // No inductive relaxation.
  ConfigureRelaxation_Local, // Local inductive relaxation.
  ConfigureRelaxation_Global, // Global inductive relaxation.
  ConfigureRelaxation__
} ConfigureRelaxation;
typedef uint8_t short_ConfigureRelaxation;
extern const char *
ConfigureRelaxation_NAME_[];
#define ConfigureRelaxation_NAME_(type) ConfigureRelaxation_NAME_[type]

/*
 * ConfigureRenaming --	Enumeration of the register renaming levels.
 */
typedef enum {
  ConfigureRenaming_, // No register renaming.
  ConfigureRenaming_Normal, // Normal register renaming.
  ConfigureRenaming_Local, // Local register modulo renaming.
  ConfigureRenaming_Global, // Global register modulo renaming.
  ConfigureRenaming__
} ConfigureRenaming;
typedef uint8_t short_ConfigureRenaming;
extern const char *
ConfigureRenaming_NAME_[];
#define ConfigureRenaming_NAME_(type) ConfigureRenaming_NAME_[type]

/*
 * ConfigureBoosting --	Enumeration of the operation boosting levels.
 *
 * Boosting is the enabling of control speculation by code transformations.
 */
typedef enum {
  ConfigureBoosting_, // No operation boosting.
  ConfigureBoosting_Reuse, // Reuse branch predicates.
  ConfigureBoosting_Combine, // Combine branch predicates.
  ConfigureBoosting__
} ConfigureBoosting;
typedef uint8_t short_ConfigureBoosting;
extern const char *
ConfigureBoosting_NAME_[];
#define ConfigureBoosting_NAME_(type) ConfigureBoosting_NAME_[type]

/*
 * ConfigureAliasing --	Memory aliasing level.
 *
 * Enumerate how memory aliasing inside a loop can be overridden.
 */
typedef enum {
  ConfigureAliasing_Serial, // Serialize all memory accesses.
  ConfigureAliasing_Normal, // Normal aliasing of memory accesses.
  ConfigureAliasing_Vector, // Assume vector loop memory aliasing.
  ConfigureAliasing_Parallel, // Assume parallel loop memory aliasing.
  ConfigureAliasing_Liberal, // Assume no alias of variant memory accesses.
  ConfigureAliasing__
} ConfigureAliasing;
typedef uint8_t short_ConfigureAliasing;
extern const char *
ConfigureAliasing_NAME_[];
#define ConfigureAliasing_NAME_(type) ConfigureAliasing_NAME_[type]

/*
 * ConfigurePreLoading --	Memory pre-loading level.
 */
typedef enum {
  ConfigurePreLoading_, // Disable pre-loading.
  ConfigurePreLoading_Simple, // Pre-loading of simple induction LOADs.
  ConfigurePreLoading_Strided, // Pre-loading of strided induction LOADs.
  ConfigurePreLoading__
} ConfigurePreLoading;
typedef uint8_t short_ConfigurePreLoading;
extern const char *
ConfigurePreLoading_NAME_[];
#define ConfigurePreLoading_NAME_(type) ConfigurePreLoading_NAME_[type]

/*
 * ConfigureItem --	Enumerate the Configure items.
 *
 * Each ConfigureItem can have a negative value, meaning that the value is undefined.
 */
typedef enum {
  ConfigureItem_Processor, // Processor in use.
  ConfigureItem_Compensation, // See ConfigureCompensation.
  ConfigureItem_Speculation, // See ConfigureSpeculation.
  ConfigureItem_Relaxation, // See ConfigureRelaxation.
  ConfigureItem_Renaming, // See ConfigureRenaming.
  ConfigureItem_Boosting, // See ConfigureBoosting.
  ConfigureItem_Aliasing, // See ConfigureAliasing.
  ConfigureItem_TripModulus, // Modulus of loop trip count.
  ConfigureItem_TripResidue, // Residue of loop trip count.
  ConfigureItem_TripOverRun, // OverRun of loop trip count.
  ConfigureItem_Pipelining, // Software pipelining level.
    // 0 => cyclic instruction schedule,
    // 1 => software pipelining with overlap 1,
    // n => software pipelining with overlap (1<<n)-1.
  ConfigureItem_PreLoading, // See ConfigurePreLoading.
  ConfigureItem__
} ConfigureItem;
typedef uint8_t short_ConfigureItem;
extern const char *
ConfigureItem_NAME_[];
#define ConfigureItem_NAME_(type) ConfigureItem_NAME_[type]

/*
 * DependenceKind --	Enumerates the Dependence kinds.
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
typedef uint8_t short_DependenceKind;
extern const char *
DependenceKind_NAME_[];
#define DependenceKind(kind) ((kind) & (DependenceKind_Definite - 1))
#define DependenceKind_NAME_(kind) DependenceKind_NAME_[(kind)]
#define DependenceKind_mayAdjust(kind) ((kind) <= DependenceKind_WAW)
#define DependenceKind_mayRefine(kind) ((kind) <= DependenceKind_Spill)
#define DependenceKind_isActive(kind) ( (kind) >= DependenceKind_RAW && (kind) <= DependenceKind_Link )




extern void lao_init(void);
extern void lao_fini(void);
extern void lao_init_pu(void);
extern void lao_fini_pu(void);
extern bool lao_optimize_pu(unsigned);
