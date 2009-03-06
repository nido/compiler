/* Generated automatically by the program `genconditions' from the target
   machine description file.  */

#include "hconfig.h"
#include "insn-constants.h"

/* Do not allow checking to confuse the issue.  */
#undef ENABLE_CHECKING
#undef ENABLE_TREE_CHECKING
#undef ENABLE_RTL_CHECKING
#undef ENABLE_RTL_FLAG_CHECKING
#undef ENABLE_GC_CHECKING
#undef ENABLE_GC_ALWAYS_COLLECT

#include "system.h"
#include "rtl.h"
#include "tm_p.h"
#include "function.h"

/* Fake - insn-config.h doesn't exist yet.  */
#define MAX_RECOG_OPERANDS 10
#define MAX_DUP_OPERANDS 10
#define MAX_INSNS_PER_SPLIT 5

#include "regs.h"
#include "recog.h"
#include "real.h"
#include "output.h"
#include "flags.h"
#include "hard-reg-set.h"
#include "resource.h"
#include "toplev.h"
#include "reload.h"
#include "gensupport.h"

#include "except.h"

/* Dummy external declarations.  */
extern rtx insn;
extern rtx ins1;
extern rtx operands[];
extern int next_insn_tests_no_inequality PARAMS ((rtx));

/* If we don't have __builtin_constant_p, or it's not acceptable in
   array initializers, fall back to assuming that all conditions
   potentially vary at run time.  It works in 3.0.1 and later; 3.0
   only when not optimizing.  */
#if (GCC_VERSION >= 3001) || ((GCC_VERSION == 3000) && !__OPTIMIZE__)
# define MAYBE_EVAL(expr) (__builtin_constant_p(expr) ? (int) (expr) : -1)
#else
# define MAYBE_EVAL(expr) -1
#endif

/* This table lists each condition found in the machine description.
   Each condition is mapped to its truth value (0 or 1), or -1 if that
   cannot be calculated at compile time. */

const struct c_test insn_conditions[] = {
  { "flag_pic",
    MAYBE_EVAL (flag_pic) },
  { "TARGET_ARM && arm_arch5e",
    MAYBE_EVAL (TARGET_ARM && arm_arch5e) },
  { "TARGET_ARM\n\
   && !TARGET_HARD_FLOAT\n\
   && reload_completed\n\
   && GET_CODE (operands[1]) == CONST_DOUBLE",
    MAYBE_EVAL (TARGET_ARM
   && !TARGET_HARD_FLOAT
   && reload_completed
   && GET_CODE (operands[1]) == CONST_DOUBLE) },
  { "TARGET_EITHER && flag_pic && operands[2] == pic_offset_table_rtx",
    MAYBE_EVAL (TARGET_EITHER && flag_pic && operands[2] == pic_offset_table_rtx) },
  { "TARGET_ARM && GET_CODE (operands[0]) == SYMBOL_REF",
    MAYBE_EVAL (TARGET_ARM && GET_CODE (operands[0]) == SYMBOL_REF) },
  { "TARGET_ARM && !arm_is_xscale",
    MAYBE_EVAL (TARGET_ARM && !arm_is_xscale) },
  { "TARGET_ARM\n\
   && (((unsigned HOST_WIDE_INT) INTVAL (operands[1]))\n\
       == (((unsigned HOST_WIDE_INT) INTVAL (operands[1])) >> 24) << 24)",
    MAYBE_EVAL (TARGET_ARM
   && (((unsigned HOST_WIDE_INT) INTVAL (operands[1]))
       == (((unsigned HOST_WIDE_INT) INTVAL (operands[1])) >> 24) << 24)) },
  { "TARGET_ARM && arm_fast_multiply",
    MAYBE_EVAL (TARGET_ARM && arm_fast_multiply) },
  { "TARGET_ARM &&\n\
   !(const_ok_for_arm (INTVAL (operands[2]))\n\
     || const_ok_for_arm (-INTVAL (operands[2])))\n\
    && const_ok_for_arm (~INTVAL (operands[2]))",
    MAYBE_EVAL (TARGET_ARM &&
   !(const_ok_for_arm (INTVAL (operands[2]))
     || const_ok_for_arm (-INTVAL (operands[2])))
    && const_ok_for_arm (~INTVAL (operands[2]))) },
  { "TARGET_ARM\n\
   && reload_completed\n\
   && operands[0] != operands[1]",
    MAYBE_EVAL (TARGET_ARM
   && reload_completed
   && operands[0] != operands[1]) },
  { "TARGET_ARM\n\
   && GET_CODE (operands[2]) == CONST_INT\n\
   && !const_ok_for_arm (INTVAL (operands[2]))",
    MAYBE_EVAL (TARGET_ARM
   && GET_CODE (operands[2]) == CONST_INT
   && !const_ok_for_arm (INTVAL (operands[2]))) },
  { "TARGET_ARM && arm_is_xscale",
    MAYBE_EVAL (TARGET_ARM && arm_is_xscale) },
  { "TARGET_THUMB",
    MAYBE_EVAL (TARGET_THUMB) },
  { "TARGET_THUMB && CONST_OK_FOR_THUMB_LETTER (INTVAL (operands[1]), 'J')",
    MAYBE_EVAL (TARGET_THUMB && CONST_OK_FOR_THUMB_LETTER (INTVAL (operands[1]), 'J')) },
  { "(TARGET_ARM) && (TARGET_ARM && arm_fast_multiply)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && arm_fast_multiply)) },
  { "TARGET_THUMB || (TARGET_ARM && TARGET_MMU_TRAPS)",
    MAYBE_EVAL (TARGET_THUMB || (TARGET_ARM && TARGET_MMU_TRAPS)) },
  { "TARGET_ARM && load_multiple_sequence (operands, 2, NULL, NULL, NULL)",
    MAYBE_EVAL (TARGET_ARM && load_multiple_sequence (operands, 2, NULL, NULL, NULL)) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && TARGET_SOFT_FLOAT\n\
   && (GET_CODE (operands[0]) != MEM\n\
       || register_operand (operands[1], SFmode)))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && TARGET_SOFT_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], SFmode)))) },
  { "TARGET_ARM && GET_CODE (operands[1]) == SYMBOL_REF",
    MAYBE_EVAL (TARGET_ARM && GET_CODE (operands[1]) == SYMBOL_REF) },
  { "TARGET_ARM && XVECLEN (operands[0], 0) == 5",
    MAYBE_EVAL (TARGET_ARM && XVECLEN (operands[0], 0) == 5) },
  { "TARGET_ARM\n\
   && (INTVAL (operands[3]) >= 0 && INTVAL (operands[3]) < 32\n\
       && INTVAL (operands[2]) > 0 \n\
       && INTVAL (operands[2]) + (INTVAL (operands[3]) & 1) <= 8\n\
       && INTVAL (operands[2]) + INTVAL (operands[3]) <= 32)",
    MAYBE_EVAL (TARGET_ARM
   && (INTVAL (operands[3]) >= 0 && INTVAL (operands[3]) < 32
       && INTVAL (operands[2]) > 0 
       && INTVAL (operands[2]) + (INTVAL (operands[3]) & 1) <= 8
       && INTVAL (operands[2]) + INTVAL (operands[3]) <= 32)) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && !BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS\n\
   && !arm_arch4\n\
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)) },
  { "TARGET_ARM\n\
   && !arm_arch4\n\
   && !BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS\n\
   && (GET_CODE (operands[1]) != CONST_INT\n\
       || const_ok_for_arm (INTVAL (operands[1]))\n\
       || const_ok_for_arm (~INTVAL (operands[1])))",
    MAYBE_EVAL (TARGET_ARM
   && !arm_arch4
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1])))) },
  { "TARGET_ARM && load_multiple_sequence (operands, 3, NULL, NULL, NULL)",
    MAYBE_EVAL (TARGET_ARM && load_multiple_sequence (operands, 3, NULL, NULL, NULL)) },
  { "TARGET_ARM\n\
  && (INTVAL (operands[2]) >= 0 && INTVAL (operands[2]) < 32\n\
      && INTVAL (operands[1]) > 0 \n\
      && INTVAL (operands[1]) + (INTVAL (operands[2]) & 1) <= 8\n\
      && INTVAL (operands[1]) + INTVAL (operands[2]) <= 32)",
    MAYBE_EVAL (TARGET_ARM
  && (INTVAL (operands[2]) >= 0 && INTVAL (operands[2]) < 32
      && INTVAL (operands[1]) > 0 
      && INTVAL (operands[1]) + (INTVAL (operands[2]) & 1) <= 8
      && INTVAL (operands[1]) + INTVAL (operands[2]) <= 32)) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && !BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS\n\
   && !arm_arch4\n\
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM\n\
   && (GET_CODE (operands[2]) != REG\n\
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))) },
  { "(TARGET_ARM) && (TARGET_ARM && (!TARGET_MMU_TRAPS))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && (!TARGET_MMU_TRAPS))) },
  { "TARGET_THUMB\n\
   && GET_CODE (operands[1]) == SYMBOL_REF\n\
   && !arm_is_longcall_p (operands[1], INTVAL (operands[3]), 1)",
    MAYBE_EVAL (TARGET_THUMB
   && GET_CODE (operands[1]) == SYMBOL_REF
   && !arm_is_longcall_p (operands[1], INTVAL (operands[3]), 1)) },
  { "TARGET_ARM && XVECLEN (operands[0], 0) == 4",
    MAYBE_EVAL (TARGET_ARM && XVECLEN (operands[0], 0) == 4) },
  { "ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT",
    MAYBE_EVAL (ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT) },
  { "TARGET_ARM\n\
   && !BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS\n\
   && !arm_arch4\n\
   && REGNO (operands[0]) != REGNO(operands[1])\n\
   && (GET_CODE (operands[2]) != REG\n\
       || REGNO(operands[0]) != REGNO (operands[2]))",
    MAYBE_EVAL (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != REGNO(operands[1])
   && (GET_CODE (operands[2]) != REG
       || REGNO(operands[0]) != REGNO (operands[2]))) },
  { "TARGET_ARM\n\
   && (REGNO (operands[2]) != REGNO (operands[0]))\n\
   && (GET_CODE (operands[1]) != REG\n\
       || (REGNO (operands[1]) != REGNO (operands[0])))",
    MAYBE_EVAL (TARGET_ARM
   && (REGNO (operands[2]) != REGNO (operands[0]))
   && (GET_CODE (operands[1]) != REG
       || (REGNO (operands[1]) != REGNO (operands[0])))) },
  { "TARGET_ARM && flag_pic",
    MAYBE_EVAL (TARGET_ARM && flag_pic) },
  { "TARGET_THUMB\n\
   && (   register_operand (operands[0], HImode)\n\
       || register_operand (operands[1], HImode))",
    MAYBE_EVAL (TARGET_THUMB
   && (   register_operand (operands[0], HImode)
       || register_operand (operands[1], HImode))) },
  { "TARGET_ARM\n\
   && !BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS\n\
   && !arm_arch4\n\
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM",
    MAYBE_EVAL (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM) },
  { "TARGET_ARM && (!TARGET_MMU_TRAPS)",
    MAYBE_EVAL (TARGET_ARM && (!TARGET_MMU_TRAPS)) },
  { "(TARGET_ARM) && (TARGET_ARM && flag_pic)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && flag_pic)) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)) },
  { "TARGET_ARM\n\
   && (   register_operand (operands[0], QImode)\n\
       || register_operand (operands[1], QImode))",
    MAYBE_EVAL (TARGET_ARM
   && (   register_operand (operands[0], QImode)
       || register_operand (operands[1], QImode))) },
  { "TARGET_ARM\n\
   && TARGET_HARD_FLOAT\n\
   && (GET_CODE (operands[0]) != MEM\n\
       || register_operand (operands[1], DFmode))",
    MAYBE_EVAL (TARGET_ARM
   && TARGET_HARD_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], DFmode))) },
  { "TARGET_THUMB\n\
   && (   register_operand (operands[0], DImode)\n\
       || register_operand (operands[1], DImode))",
    MAYBE_EVAL (TARGET_THUMB
   && (   register_operand (operands[0], DImode)
       || register_operand (operands[1], DImode))) },
  { "(TARGET_ARM) && (TARGET_ARM && TARGET_MMU_TRAPS)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && TARGET_MMU_TRAPS)) },
  { "(TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)) },
  { "TARGET_ARM && TARGET_HARD_FLOAT && (ENABLE_XF_PATTERNS || reload_completed)",
    MAYBE_EVAL (TARGET_ARM && TARGET_HARD_FLOAT && (ENABLE_XF_PATTERNS || reload_completed)) },
  { "TARGET_ARM && store_multiple_sequence (operands, 3, NULL, NULL, NULL)",
    MAYBE_EVAL (TARGET_ARM && store_multiple_sequence (operands, 3, NULL, NULL, NULL)) },
  { "TARGET_ARM && (GET_CODE (operands[1]) != MEM) && ! BYTES_BIG_ENDIAN",
    MAYBE_EVAL (TARGET_ARM && (GET_CODE (operands[1]) != MEM) && ! BYTES_BIG_ENDIAN) },
  { "TARGET_THUMB\n\
   && (   register_operand (operands[0], SFmode) \n\
       || register_operand (operands[1], SFmode))",
    MAYBE_EVAL (TARGET_THUMB
   && (   register_operand (operands[0], SFmode) 
       || register_operand (operands[1], SFmode))) },
  { "TARGET_ARM\n\
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM\n\
   && (GET_CODE (operands[2]) != REG\n\
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)",
    MAYBE_EVAL (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)) },
  { "TARGET_EITHER",
    MAYBE_EVAL (TARGET_EITHER) },
  { "TARGET_ARM\n\
   && (REGNO (operands[3]) != REGNO (operands[2]))\n\
   && (REGNO (operands[0]) != REGNO (operands[2]))",
    MAYBE_EVAL (TARGET_ARM
   && (REGNO (operands[3]) != REGNO (operands[2]))
   && (REGNO (operands[0]) != REGNO (operands[2]))) },
  { "TARGET_ARM\n\
   && (GET_CODE (operands[0]) == SYMBOL_REF)\n\
   && !arm_is_longcall_p (operands[0], INTVAL (operands[2]), 1)",
    MAYBE_EVAL (TARGET_ARM
   && (GET_CODE (operands[0]) == SYMBOL_REF)
   && !arm_is_longcall_p (operands[0], INTVAL (operands[2]), 1)) },
  { "TARGET_ARM\n\
   && !const_ok_for_arm (INTVAL (operands[2]))\n\
   && const_ok_for_arm (~INTVAL (operands[2]))",
    MAYBE_EVAL (TARGET_ARM
   && !const_ok_for_arm (INTVAL (operands[2]))
   && const_ok_for_arm (~INTVAL (operands[2]))) },
  { "(TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 5)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 5)) },
  { "TARGET_THUMB\n\
   && (   register_operand (operands[0], SImode) \n\
       || register_operand (operands[1], SImode))",
    MAYBE_EVAL (TARGET_THUMB
   && (   register_operand (operands[0], SImode) 
       || register_operand (operands[1], SImode))) },
  { "TARGET_ARM\n\
  && (!(const_ok_for_arm (INTVAL (operands[1]))\n\
        || const_ok_for_arm (~INTVAL (operands[1]))))",
    MAYBE_EVAL (TARGET_ARM
  && (!(const_ok_for_arm (INTVAL (operands[1]))
        || const_ok_for_arm (~INTVAL (operands[1]))))) },
  { "TARGET_THUMB\n\
   && (   register_operand (operands[0], QImode)\n\
       || register_operand (operands[1], QImode))",
    MAYBE_EVAL (TARGET_THUMB
   && (   register_operand (operands[0], QImode)
       || register_operand (operands[1], QImode))) },
  { "TARGET_ARM\n\
   && arm_arch4\n\
   && (GET_CODE (operands[1]) != CONST_INT\n\
       || const_ok_for_arm (INTVAL (operands[1]))\n\
       || const_ok_for_arm (~INTVAL (operands[1])))",
    MAYBE_EVAL (TARGET_ARM
   && arm_arch4
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1])))) },
  { "TARGET_ARM && arm_arch5",
    MAYBE_EVAL (TARGET_ARM && arm_arch5) },
  { "(TARGET_ARM) && (ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)",
    MAYBE_EVAL ((TARGET_ARM) && (ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)) },
  { "TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT",
    MAYBE_EVAL (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && arm_arch4\n\
   && (GET_CODE (operands[1]) != CONST_INT\n\
       || const_ok_for_arm (INTVAL (operands[1]))\n\
       || const_ok_for_arm (~INTVAL (operands[1]))))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && arm_arch4
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1]))))) },
  { "TARGET_ARM && arm_arch4",
    MAYBE_EVAL (TARGET_ARM && arm_arch4) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM\n\
   && (GET_CODE (operands[2]) != REG\n\
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))) },
  { "(TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 4)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 4)) },
  { "TARGET_ARM && store_multiple_sequence (operands, 2, NULL, NULL, NULL)",
    MAYBE_EVAL (TARGET_ARM && store_multiple_sequence (operands, 2, NULL, NULL, NULL)) },
  { "TARGET_ARM && TARGET_SOFT_FLOAT",
    MAYBE_EVAL (TARGET_ARM && TARGET_SOFT_FLOAT) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && !arm_arch4\n\
   && BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS\n\
   && (GET_CODE (operands[1]) != CONST_INT\n\
       || const_ok_for_arm (INTVAL (operands[1]))\n\
       || const_ok_for_arm (~INTVAL (operands[1]))))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && !arm_arch4
   && BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1]))))) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && TARGET_HARD_FLOAT\n\
   && (GET_CODE (operands[0]) != MEM\n\
       || register_operand (operands[1], SFmode)))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && TARGET_HARD_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], SFmode)))) },
  { "TARGET_ARM && store_multiple_sequence (operands, 4, NULL, NULL, NULL)",
    MAYBE_EVAL (TARGET_ARM && store_multiple_sequence (operands, 4, NULL, NULL, NULL)) },
  { "TARGET_ARM && TARGET_MMU_TRAPS",
    MAYBE_EVAL (TARGET_ARM && TARGET_MMU_TRAPS) },
  { "(TARGET_ARM) && (TARGET_ARM && adjacent_mem_locations (operands[2], operands[3]))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && adjacent_mem_locations (operands[2], operands[3]))) },
  { "TARGET_ARM\n\
   && TARGET_HARD_FLOAT\n\
   && (GET_CODE (operands[0]) != MEM\n\
       || register_operand (operands[1], SFmode))",
    MAYBE_EVAL (TARGET_ARM
   && TARGET_HARD_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], SFmode))) },
  { "TARGET_ARM && operands[0] != operands[1] && reload_completed",
    MAYBE_EVAL (TARGET_ARM && operands[0] != operands[1] && reload_completed) },
  { "TARGET_ARM\n\
  ",
    MAYBE_EVAL (TARGET_ARM
  ) },
  { "TARGET_ARM\n\
   && BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS",
    MAYBE_EVAL (TARGET_ARM
   && BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS) },
  { "(TARGET_ARM) && (TARGET_ARM && arm_arch4)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && arm_arch4)) },
  { "TARGET_ARM && USE_RETURN_INSN (TRUE)",
    MAYBE_EVAL (TARGET_ARM && USE_RETURN_INSN (TRUE)) },
  { "TARGET_ARM\n\
   && !const_ok_for_arm (INTVAL (operands[1]))\n\
   && const_ok_for_arm (~INTVAL (operands[1]))",
    MAYBE_EVAL (TARGET_ARM
   && !const_ok_for_arm (INTVAL (operands[1]))
   && const_ok_for_arm (~INTVAL (operands[1]))) },
  { "TARGET_ARM\n\
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM",
    MAYBE_EVAL (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM) },
  { "TARGET_ARM",
    MAYBE_EVAL (TARGET_ARM) },
  { "TARGET_THUMB\n\
   && (   register_operand (operands[0], DFmode)\n\
       || register_operand (operands[1], DFmode))",
    MAYBE_EVAL (TARGET_THUMB
   && (   register_operand (operands[0], DFmode)
       || register_operand (operands[1], DFmode))) },
  { "TARGET_ARM && load_multiple_sequence (operands, 4, NULL, NULL, NULL)",
    MAYBE_EVAL (TARGET_ARM && load_multiple_sequence (operands, 4, NULL, NULL, NULL)) },
  { "TARGET_THUMB && flag_pic",
    MAYBE_EVAL (TARGET_THUMB && flag_pic) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && (   register_operand (operands[0], SImode)\n\
       || register_operand (operands[1], SImode)))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && (   register_operand (operands[0], SImode)
       || register_operand (operands[1], SImode)))) },
  { "TARGET_ARM\n\
   && (GET_CODE (operands[1]) != REG\n\
       || (REGNO(operands[1]) != FRAME_POINTER_REGNUM\n\
           && REGNO(operands[1]) != ARG_POINTER_REGNUM))",
    MAYBE_EVAL (TARGET_ARM
   && (GET_CODE (operands[1]) != REG
       || (REGNO(operands[1]) != FRAME_POINTER_REGNUM
           && REGNO(operands[1]) != ARG_POINTER_REGNUM))) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && !arm_arch4\n\
   && !BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS\n\
   && (GET_CODE (operands[1]) != CONST_INT\n\
       || const_ok_for_arm (INTVAL (operands[1]))\n\
       || const_ok_for_arm (~INTVAL (operands[1]))))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && !arm_arch4
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1]))))) },
  { "TARGET_ARM\n\
   && !arm_arch4\n\
   && BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS\n\
   && (GET_CODE (operands[1]) != CONST_INT\n\
       || const_ok_for_arm (INTVAL (operands[1]))\n\
       || const_ok_for_arm (~INTVAL (operands[1])))",
    MAYBE_EVAL (TARGET_ARM
   && !arm_arch4
   && BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1])))) },
  { "TARGET_THUMB\n\
   && REGNO (operands[2]) == STACK_POINTER_REGNUM \n\
   && (unsigned HOST_WIDE_INT) (INTVAL (operands[1])) < 1024\n\
   && (INTVAL (operands[1]) & 3) == 0",
    MAYBE_EVAL (TARGET_THUMB
   && REGNO (operands[2]) == STACK_POINTER_REGNUM 
   && (unsigned HOST_WIDE_INT) (INTVAL (operands[1])) < 1024
   && (INTVAL (operands[1]) & 3) == 0) },
  { "TARGET_ARM && reload_completed",
    MAYBE_EVAL (TARGET_ARM && reload_completed) },
  { "TARGET_ARM\n\
   && !BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS\n\
   && !arm_arch4\n\
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM\n\
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM\n\
   && (GET_CODE (operands[2]) != REG\n\
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)",
    MAYBE_EVAL (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)) },
  { "TARGET_ARM && XVECLEN (operands[0], 0) == 2",
    MAYBE_EVAL (TARGET_ARM && XVECLEN (operands[0], 0) == 2) },
  { "TARGET_ARM\n\
   && GET_CODE (operands[1]) == CONST_INT\n\
   && !const_ok_for_arm (INTVAL (operands[1]))",
    MAYBE_EVAL (TARGET_ARM
   && GET_CODE (operands[1]) == CONST_INT
   && !const_ok_for_arm (INTVAL (operands[1]))) },
  { "TARGET_ARM && (!arm_arch4)",
    MAYBE_EVAL (TARGET_ARM && (!arm_arch4)) },
  { "TARGET_ARM && TARGET_SOFT_FLOAT\n\
  ",
    MAYBE_EVAL (TARGET_ARM && TARGET_SOFT_FLOAT
  ) },
  { "(TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 2)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 2)) },
  { "TARGET_ARM && (!CONSTANT_ADDRESS_P (XEXP (operands[1], 0)))",
    MAYBE_EVAL (TARGET_ARM && (!CONSTANT_ADDRESS_P (XEXP (operands[1], 0)))) },
  { "TARGET_ARM\n\
   && REGNO (operands[0]) != REGNO(operands[1])\n\
   && (GET_CODE (operands[2]) != REG\n\
       || REGNO(operands[0]) != REGNO (operands[2]))",
    MAYBE_EVAL (TARGET_ARM
   && REGNO (operands[0]) != REGNO(operands[1])
   && (GET_CODE (operands[2]) != REG
       || REGNO(operands[0]) != REGNO (operands[2]))) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && BYTES_BIG_ENDIAN\n\
   && !TARGET_MMU_TRAPS)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS)) },
  { "(TARGET_ARM) && (TARGET_ARM && USE_RETURN_INSN (FALSE))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && USE_RETURN_INSN (FALSE))) },
  { "TARGET_ARM\n\
   && (GET_CODE (operands[1]) == SYMBOL_REF)\n\
   && !arm_is_longcall_p (operands[1], INTVAL (operands[3]), 1)",
    MAYBE_EVAL (TARGET_ARM
   && (GET_CODE (operands[1]) == SYMBOL_REF)
   && !arm_is_longcall_p (operands[1], INTVAL (operands[3]), 1)) },
  { "TARGET_ARM && adjacent_mem_locations (operands[2], operands[3])",
    MAYBE_EVAL (TARGET_ARM && adjacent_mem_locations (operands[2], operands[3])) },
  { "TARGET_ARM\n\
   && GET_CODE (operands[2]) == CONST_INT\n\
   && !(const_ok_for_arm (INTVAL (operands[2]))\n\
	|| const_ok_for_arm (~INTVAL (operands[2])))",
    MAYBE_EVAL (TARGET_ARM
   && GET_CODE (operands[2]) == CONST_INT
   && !(const_ok_for_arm (INTVAL (operands[2]))
	|| const_ok_for_arm (~INTVAL (operands[2])))) },
  { "TARGET_ARM &&\n\
   GET_CODE (operands[2]) == CONST_INT\n\
   && !(const_ok_for_arm (INTVAL (operands[2]))\n\
        || const_ok_for_arm (-INTVAL (operands[2])))",
    MAYBE_EVAL (TARGET_ARM &&
   GET_CODE (operands[2]) == CONST_INT
   && !(const_ok_for_arm (INTVAL (operands[2]))
        || const_ok_for_arm (-INTVAL (operands[2])))) },
  { "(TARGET_ARM) && (TARGET_ARM)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM)) },
  { "TARGET_THUMB && CONST_OK_FOR_THUMB_LETTER (INTVAL (operands[1]), 'K')",
    MAYBE_EVAL (TARGET_THUMB && CONST_OK_FOR_THUMB_LETTER (INTVAL (operands[1]), 'K')) },
  { "(TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT && (ENABLE_XF_PATTERNS || reload_completed))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT && (ENABLE_XF_PATTERNS || reload_completed))) },
  { "TARGET_ARM && XVECLEN (operands[0], 0) == 3",
    MAYBE_EVAL (TARGET_ARM && XVECLEN (operands[0], 0) == 3) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && TARGET_HARD_FLOAT\n\
   && (GET_CODE (operands[0]) != MEM\n\
       || register_operand (operands[1], DFmode)))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && TARGET_HARD_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], DFmode)))) },
  { "(TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 3)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 3)) },
  { "TARGET_ARM\n\
   && TARGET_SOFT_FLOAT\n\
   && (GET_CODE (operands[0]) != MEM\n\
       || register_operand (operands[1], SFmode))",
    MAYBE_EVAL (TARGET_ARM
   && TARGET_SOFT_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], SFmode))) },
  { "TARGET_ARM && arm_arch4 && reload_completed",
    MAYBE_EVAL (TARGET_ARM && arm_arch4 && reload_completed) },
  { "(TARGET_ARM) && (TARGET_ARM\n\
   && (   register_operand (operands[0], QImode)\n\
       || register_operand (operands[1], QImode)))",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM
   && (   register_operand (operands[0], QImode)
       || register_operand (operands[1], QImode)))) },
  { "TARGET_ARM\n\
   && (   register_operand (operands[0], SImode)\n\
       || register_operand (operands[1], SImode))",
    MAYBE_EVAL (TARGET_ARM
   && (   register_operand (operands[0], SImode)
       || register_operand (operands[1], SImode))) },
  { "TARGET_ARM && USE_RETURN_INSN (FALSE)",
    MAYBE_EVAL (TARGET_ARM && USE_RETURN_INSN (FALSE)) },
  { "TARGET_THUMB\n\
   && GET_CODE (operands[0]) == SYMBOL_REF\n\
   && !arm_is_longcall_p (operands[0], INTVAL (operands[2]), 1)",
    MAYBE_EVAL (TARGET_THUMB
   && GET_CODE (operands[0]) == SYMBOL_REF
   && !arm_is_longcall_p (operands[0], INTVAL (operands[2]), 1)) },
  { "TARGET_ARM && TARGET_HARD_FLOAT",
    MAYBE_EVAL (TARGET_ARM && TARGET_HARD_FLOAT) },
  { "(TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)",
    MAYBE_EVAL ((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)) },
};

const size_t n_insn_conditions = 117;
const int insn_elision_unavailable = 0;
