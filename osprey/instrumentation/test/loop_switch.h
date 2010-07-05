/* Include builtin types and operators */
#include <whirl2c.h>

/* Types */
/* File-level symbolic constants */
static _STRING anon = "loop_switch.instr0";

static _STRING anon0 = "loop_switch.c";

static _STRING anon1 = "branch_exple";

static _STRING anon2 = "switch_exple";

static _STRING anon3 = "main";

/* File-level vars and routines */
extern _INT32 branch_exple(_INT32);

extern _INT32 switch_exple(_INT32);

extern _INT32 main();

extern void __profile_invoke();

extern void __profile_branch();

extern void __profile_init();

extern _UINT32 __profile_pu_init();

extern void __profile_invoke_init();

extern void __profile_branch_init();

extern void __profile_switch();

extern void __profile_switch_init();

extern void __profile_call_entry();

extern void __profile_call_exit();

extern void __profile_loop();

extern void __profile_loop_iter();

extern void __profile_loop_init();

extern void __profile_call_init();

