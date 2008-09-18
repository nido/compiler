

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
typedef uint16_t short_OptimizeActivation;
#define OptimizeActivation_default 0


/*
 * OptimizeRegionType --	enumerates the OptimizeRegionType types.
 */
typedef enum {
  OptimizeRegionType_SuperBlock = 0x1, // Superblock region.
  OptimizeRegionType_TraceBlock = 0x2, // TraceBlock region.
  OptimizeRegionType_HyperBlock = 0x4, // Hyperblock region.
  OptimizeRegionType_InnerLoop = 0x8, // Inner loop region.
} OptimizeRegionType;
typedef uint8_t short_OptimizeRegionType;
#define OptimizeRegionType_default OptimizeRegionType_SuperBlock


/*
 * OptimizeProfiling --	Enumeration of the code profiling levels.
 */
typedef enum {
  OptimizeProfiling_Static = 0x1, // Static profiling
  OptimizeProfiling_Edge = 0x2, // Dynamic edge profiling.
  OptimizeProfiling_Path = 0x4, // Dynamic path profiling.
} OptimizeProfiling;
typedef uint8_t short_OptimizeProfiling;
#define OptimizeProfiling_default 0


/*
 * OptimizeScheduling --	Enumeration of the pre- and post-scheduler levels.
 */
typedef enum {
  OptimizeScheduling_None, // No scheduling.
  OptimizeScheduling_Unwinding, // Unwinding modulo scheduling.
  OptimizeScheduling_Insertion, // Insertion modulo scheduling.
  OptimizeScheduling_Iterative, // Iterative modulo scheduling.
  OptimizeScheduling_Formulation, // RCMSP Formulation scheduling.
  OptimizeScheduling__
} OptimizeScheduling;
typedef uint8_t short_OptimizeScheduling;
#define OptimizeScheduling_default OptimizeScheduling_Insertion

extern const char *
OptimizeScheduling_(OptimizeScheduling THIS);

/*
 * OptimizeAllocation --	Enumeration of the register allocation types.
 */
typedef enum {
  OptimizeAllocation_LinearScanning = 0x1, // Linear Scan of Sarkar & Barik.
  OptimizeAllocation_TreeScanning = 0x2, // Tree Scan of Rastello et al.
  OptimizeAllocation_GraphColoring = 0x4, // Some Graph Coloring allocator.
} OptimizeAllocation;
#define OptimizeAllocation_default 0


/*
 * OptimizeConversion --	SSA form conversion flags.
 */
typedef enum {
  OptimizeConversion_Folding = 0x1, // Copy Folding during SSA construction.
  OptimizeConversion_Cleaning = 0x2, // Code Cleaning during SSA construction.
  OptimizeConversion_SemiPruned = 0x4, // Semi-pruned improved SSA construction.
  OptimizeConversion_SigmaGoTo = 0x8, // Sigma operations for the GoTo conditions.
} OptimizeConversion;
#define OptimizeConversion_default OptimizeConversion_Folding | OptimizeConversion_Cleaning



/*
 * OptimizeCoalescing --	SSA form coalescing flags.
 */
typedef enum {
  OptimizeCoalescing_Sreedhar = 0x1, // Sreedhar CSSA conversion.
  OptimizeCoalescing_Boissinot = 0x2, // Boissinot CSSA conversion.
  OptimizeCoalescing_Decoalesce = 0x4, // Decoalesce the SSA-webs (Budimlic).
  OptimizeCoalescing_Virtualize = 0x8, // Virtualize COPY insertion (Method III).
  OptimizeCoalescing_Congruence = 0x10, // Congruence-based coalescing.
  OptimizeCoalescing_SeqCopies = 0x20, // Use sequential copies (parallel by default).
} OptimizeCoalescing;
#define OptimizeCoalescing_default OptimizeCoalescing_Sreedhar | OptimizeCoalescing_Virtualize | OptimizeCoalescing_SeqCopies




/*
 * OptimizeNumbering --	Value Numbering flags.
 */
typedef enum {
  OptimizeNumbering_Basic = 0x1, // Basic value numbering.
  OptimizeNumbering_SCCVN = 0x2, // Simpson's SCCVN improved by Cooper.
  OptimizeNumbering_Avail = 0x4, // AVAIL-based removal (else use Dominance).
} OptimizeNumbering;

/*
 * OptimizePropagation --	Data-flow facts Propagation flags.
 */
typedef enum {
  OptimizePropagation_Constant = 0x1, // Sparse conditional constant propagation.
} OptimizePropagation;

/*
 * OptimizePredication --	Code Predication level.
 */
typedef enum {
  OptimizePredication_None, // Disable code predication.
  OptimizePredication_Select, // Select-only code predication.
  OptimizePredication__
} OptimizePredication;
typedef uint8_t short_OptimizePredication;
extern const char *
OptimizePredication_(OptimizePredication THIS);

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
typedef uint16_t short_OptimizeFormulation;
#define OptimizeFormulation_default OptimizeFormulation_Integral | OptimizeFormulation_Makespan | OptimizeFormulation_Scanning



#define OptimizeFormulations_isOptimize(flags) ( flags & ( OptimizeFormulation_Makespan | OptimizeFormulation_Lifetime ) )






/*
 * OptimizeScoreboarding --	Scoreboarder scheduling levels.
 */
typedef enum {
  OptimizeScoreboarding_Fixup = 0x1, // Fixup pass over the BasicBlock(s).
  OptimizeScoreboarding_Iterate = 0x2, // Iterate forward data-flow problem.
  OptimizeScoreboarding_Priority = 0x4, // Pre-order Operation(s) by priority.
} OptimizeScoreboarding;
#define OptimizeScoreboarding_default OptimizeScoreboarding_Iterate


/*
 * OptimizeRCMSProblem --	RCMS problem description.
 */
typedef enum {
  OptimizeRCMSProblem_Regular = 0x1, // Pass regular nodes and arcs.
  OptimizeRCMSProblem_Overlap = 0x2, // Pass pipeline overlap-limiting arc.
  OptimizeRCMSProblem_Lifetime = 0x4, // Pass lifetine nodes and arcs.
  OptimizeRCMSProblem_Renaming = 0x8, // Pass modulo renaming-limiting arcs.
  OptimizeRCMSProblem_Margins = 0x10, // Pass margin-enforcing arcs.
} OptimizeRCMSProblem;

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
  OptimizeItem_Conversion, // See OptimizeConversion.
  OptimizeItem_Coalescing, // See OptimizeCoalescing.
  OptimizeItem_Numbering, // See OptimizeNumbering.
  OptimizeItem_Propagation, // See OptimizePropagation.
  OptimizeItem_Predication, // See OptimizePredication.
  OptimizeItem_Scheduling, // See OptimizeScheduling.
  OptimizeItem_Allocation, // See OptimizeAllocation.
  OptimizeItem_Formulation, // See OptimizeFormulation.
  OptimizeItem_Scoreboarding, // See OptimizeScoreboarding.
  OptimizeItem_WindowSize, // Scoreboard window size
  OptimizeItem_PrePadding, // See OptimizePrePadding.
  OptimizeItem_PostPadding, // See OptimizePostPadding.
  OptimizeItem_RCMSProblem, // See OptimizeRCMSProblem.
  OptimizeItem__
} OptimizeItem;
typedef uint8_t short_OptimizeItem;
extern const char *
OptimizeItem_(OptimizeItem THIS);

/*
 * ConfigureCompensation --	Enumeration of the code compensation levels.
 *
 * Code compensation results from code motion down a branch.
 */
typedef enum {
  ConfigureCompensation_None, // No code compensation.
  ConfigureCompensation_Local, // Local code compensation.
  ConfigureCompensation_Global, // Global code compensation.
  ConfigureCompensation__
} ConfigureCompensation;
typedef uint8_t short_ConfigureCompensation;
extern const char *
ConfigureCompensation_(ConfigureCompensation THIS);

/*
 * ConfigureSpeculation --	Enumeration of the control speculation levels.
 *
 * Speculation is the execution of code under a more general condition than original.
 */
typedef enum {
  ConfigureSpeculation_None, // No control speculation.
  ConfigureSpeculation_SafeArith, // Speculation of safe arithmetic operations.
  ConfigureSpeculation_SafeAccess, // Speculation of safe memory accesses.
  ConfigureSpeculation_SafeVariant, // Speculation of operations with safe variant.
  ConfigureSpeculation_MayExcept, // Speculation of may be excepting operations.
  ConfigureSpeculation__
} ConfigureSpeculation;
typedef uint8_t short_ConfigureSpeculation;
extern const char *
ConfigureSpeculation_(ConfigureSpeculation THIS);

/*
 * ConfigureRelaxation --	Enumeration of the inductive relaxation levels.
 *
 * Inductive relaxation enables to relax dependences on simple induction variables.
 */
typedef enum {
  ConfigureRelaxation_None, // No inductive relaxation.
  ConfigureRelaxation_Local, // Local inductive relaxation.
  ConfigureRelaxation_Global, // Global inductive relaxation.
  ConfigureRelaxation__
} ConfigureRelaxation;
typedef uint8_t short_ConfigureRelaxation;
extern const char *
ConfigureRelaxation_(ConfigureRelaxation THIS);

/*
 * ConfigureRenaming --	Enumeration of the register renaming levels.
 */
typedef enum {
  ConfigureRenaming_None, // No register renaming.
  ConfigureRenaming_Normal, // Normal register renaming.
  ConfigureRenaming_Local, // Local register modulo renaming.
  ConfigureRenaming_Global, // Global register modulo renaming.
  ConfigureRenaming_Shuffle, // Suffle global registers.
  ConfigureRenaming__
} ConfigureRenaming;
typedef uint8_t short_ConfigureRenaming;
extern const char *
ConfigureRenaming_(ConfigureRenaming THIS);

/*
 * ConfigureBoosting --	Enumeration of the operation boosting levels.
 *
 * Boosting is the enabling of control speculation by code predication.
 */
typedef enum {
  ConfigureBoosting_None, // No operation boosting.
  ConfigureBoosting_Reuse, // Reuse branch predicates.
  ConfigureBoosting_Combine, // Combine branch predicates.
  ConfigureBoosting__
} ConfigureBoosting;
typedef uint8_t short_ConfigureBoosting;
extern const char *
ConfigureBoosting_(ConfigureBoosting THIS);

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
ConfigureAliasing_(ConfigureAliasing THIS);

/*
 * ConfigurePreLoading --	Memory pre-loading level.
 */
typedef enum {
  ConfigurePreLoading_None, // Disable pre-loading.
  ConfigurePreLoading_Simple, // Pre-loading of simple induction LOADs.
  ConfigurePreLoading_Variant, // Pre-loading of variant address LOADs.
  ConfigurePreLoading_Indirect, // Pre-loading of indirect LOADs.
  ConfigurePreLoading_NonSpill, // Pre-loading of non-spill LOADs.
  ConfigurePreLoading__
} ConfigurePreLoading;
typedef uint8_t short_ConfigurePreLoading;
extern const char *
ConfigurePreLoading_(ConfigurePreLoading THIS);

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
  ConfigureItem_Pipelining, // Software pipelining level.
    // 0 => cyclic instruction schedule,
    // 1 => software pipelining with overlap 1,
    // n => software pipelining with overlap (1<<n)-1.
  ConfigureItem_PreLoading, // See ConfigurePreLoading.
  ConfigureItem_L1MissExtra, // Extra latency of L1 miss.
  ConfigureItem__
} ConfigureItem;
typedef uint8_t short_ConfigureItem;
extern const char *
ConfigureItem_(ConfigureItem THIS);

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
  DependenceKind_Relax, // Induction relaxed dependence arc.
  DependenceKind_RAW, // Register Read After Write dependence arc.
  DependenceKind_WAR, // Register Write After Read dependence arc.
  DependenceKind_WAW, // Register Write After Write dependence arc.
  DependenceKind_Life, // Register consumer node to its lifetime node.
  DependenceKind_Link, // Register producer node to its lifetime node.
  DependenceKind_Reuse, // Register reuse arc.
  DependenceKind_Some, // Inserted by DepGraph_complete.
  DependenceKind_Margin, // Inserted by DepGraph_complete.
  DependenceKind__,
  DependenceKind_Definite = 16,
} DependenceKind;
typedef uint8_t short_DependenceKind;
extern const char *
DependenceKind_(DependenceKind THIS);
#define DependenceKind(kind) ((kind) & (DependenceKind_Definite - 1))
#define DependenceKind_mayAdjust(kind) ((kind) <= DependenceKind_WAW)
#define DependenceKind_mayRefine(kind) ((kind) <= DependenceKind_Spill)

extern void lao_init(void);
extern void lao_fini(void);
extern void lao_init_pu(void);
extern void lao_fini_pu(void);
extern bool lao_optimize_pu(unsigned, bool);
