/* Include builtin types and operators */
#include <whirl2c.h>

/* Types */
/* File-level symbolic constants */
static _STRING anon = "short_circuit.instr0";

static _STRING anon0 = "short_circuit.c";

static _STRING anon1 = "main";

/* File-level vars and routines */
static _INT32 glob;

extern _INT32 main();

extern void __profile_invoke();

extern void __profile_short_circuit();

extern void __profile_branch();

extern void __profile_loop();

extern void __profile_loop_iter();

extern void __profile_init();

extern _UINT32 __profile_pu_init();

extern void __profile_invoke_init();

extern void __profile_branch_init();

extern void __profile_loop_init();

extern void __profile_short_circuit_init();

