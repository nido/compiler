/* Generated automatically by the program `genrecog' from the target
   machine description file.  */

#include "config.h"
#include "system.h"
#include "rtl.h"
#include "tm_p.h"
#include "function.h"
#include "insn-config.h"
#include "recog.h"
#include "real.h"
#include "output.h"
#include "flags.h"
#include "hard-reg-set.h"
#include "resource.h"
#include "toplev.h"
#include "reload.h"



/* `recog' contains a decision tree that recognizes whether the rtx
   X0 is a valid instruction.

   recog returns -1 if the rtx is not valid.  If the rtx is valid, recog
   returns a nonnegative number which is the insn code number for the
   pattern that matched.  This is the same as the order in the machine
   description of the entry that matched.  This number can be used as an
   index into `insn_data' and other tables.

   The third argument to recog is an optional pointer to an int.  If
   present, recog will accept a pattern if it matches except for missing
   CLOBBER expressions at the end.  In that case, the value pointed to by
   the optional pointer will be set to the number of CLOBBERs that need
   to be added (it should be initialized to zero by the caller).  If it
   is set nonzero, the caller should allocate a PARALLEL of the
   appropriate size, copy the initial entries, and call add_clobbers
   (found in insn-emit.c) to fill in the CLOBBERs.


   The function split_insns returns 0 if the rtl could not
   be split or the split rtl as an INSN list if it can be.

   The function peephole2_insns returns 0 if the rtl could not
   be matched. If there was a match, the new rtl is returned in an INSN list,
   and LAST_INSN will point to the last recognized insn in the old sequence.
*/


extern rtx gen_split_356 PARAMS ((rtx *));
extern rtx gen_split_357 PARAMS ((rtx *));
extern rtx gen_split_358 PARAMS ((rtx *));
extern rtx gen_peephole2_360 PARAMS ((rtx, rtx *));
extern rtx gen_split_361 PARAMS ((rtx *));
extern rtx gen_peephole2_362 PARAMS ((rtx, rtx *));
extern rtx gen_split_365 PARAMS ((rtx *));
extern rtx gen_peephole2_366 PARAMS ((rtx, rtx *));
extern rtx gen_split_368 PARAMS ((rtx *));
extern rtx gen_split_369 PARAMS ((rtx *));
extern rtx gen_split_370 PARAMS ((rtx *));
extern rtx gen_split_371 PARAMS ((rtx *));
extern rtx gen_split_372 PARAMS ((rtx *));
extern rtx gen_split_374 PARAMS ((rtx *));
extern rtx gen_split_376 PARAMS ((rtx *));
extern rtx gen_split_377 PARAMS ((rtx *));
extern rtx gen_split_378 PARAMS ((rtx *));
extern rtx gen_split_380 PARAMS ((rtx *));
extern rtx gen_peephole2_381 PARAMS ((rtx, rtx *));
extern rtx gen_split_383 PARAMS ((rtx *));
extern rtx gen_split_392 PARAMS ((rtx *));
extern rtx gen_split_395 PARAMS ((rtx *));
extern rtx gen_split_396 PARAMS ((rtx *));
extern rtx gen_split_398 PARAMS ((rtx *));
extern rtx gen_split_401 PARAMS ((rtx *));
extern rtx gen_split_402 PARAMS ((rtx *));
extern rtx gen_split_404 PARAMS ((rtx *));
extern rtx gen_split_406 PARAMS ((rtx *));
extern rtx gen_split_409 PARAMS ((rtx *));
extern rtx gen_split_410 PARAMS ((rtx *));
extern rtx gen_split_411 PARAMS ((rtx *));
extern rtx gen_split_425 PARAMS ((rtx *));
extern rtx gen_peephole2_489 PARAMS ((rtx, rtx *));
extern rtx gen_split_496 PARAMS ((rtx *));
extern rtx gen_split_497 PARAMS ((rtx *));
extern rtx gen_split_501 PARAMS ((rtx *));
extern rtx gen_split_502 PARAMS ((rtx *));
extern rtx gen_split_503 PARAMS ((rtx *));
extern rtx gen_split_504 PARAMS ((rtx *));



static int recog_1 PARAMS ((rtx, rtx, int *));
static int
recog_1 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L77;
    case MINUS:
      goto L377;
    case MULT:
      goto L449;
    case AND:
      goto L791;
    case IOR:
      goto L962;
    case XOR:
      goto L1025;
    case NOT:
      goto L1210;
    default:
     break;
   }
 L5908: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x3, SImode))
    {
      operands[3] = x3;
      goto L1172;
    }
  goto ret0;

 L77: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L5911;
  goto ret0;

 L5911: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x4) == MULT)
    goto L489;
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L78;
    }
  goto ret0;

 L489: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L490;
    }
  goto ret0;

 L490: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L491;
    }
  goto ret0;

 L491: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L492;
    }
  goto ret0;

 L492: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L493;
  goto ret0;

 L493: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L494;
    case CLOBBER:
      goto L511;
    default:
     break;
   }
  goto ret0;

 L494: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L495;
    }
  goto ret0;

 L495: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L496;
  goto ret0;

 L496: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MULT)
    goto L497;
  goto ret0;

 L497: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L498;
  goto ret0;

 L498: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[1]))
    goto L499;
  goto ret0;

 L499: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[3])
      && (TARGET_ARM && !arm_is_xscale))
    {
      return 48;
    }
  goto ret0;

 L511: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L512;
    }
  goto ret0;

 L512: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && !arm_is_xscale))
    {
      return 49;
    }
  goto ret0;

 L78: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L79;
    }
  goto ret0;

 L79: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L80;
  goto ret0;

 L80: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L81;
  goto ret0;

 L81: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L82;
    }
  goto ret0;

 L82: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L83;
  goto ret0;

 L83: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L84;
  goto ret0;

 L84: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 6;
    }
  goto ret0;

 L377: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L378;
    }
  goto ret0;

 L378: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L379;
    }
  goto ret0;

 L379: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L380;
  goto ret0;

 L380: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L381;
  goto ret0;

 L381: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L382;
    }
  goto ret0;

 L382: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L383;
  goto ret0;

 L383: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L384;
  goto ret0;

 L384: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 35;
    }
  goto ret0;

 L449: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L450;
    }
  goto L5908;

 L450: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L451;
    }
  goto L5908;

 L451: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L452;
  x3 = XEXP (x2, 0);
  goto L5908;

 L452: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L453;
    case CLOBBER:
      goto L466;
    default:
     break;
   }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5908;

 L453: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L454;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5908;

 L454: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MULT)
    goto L455;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5908;

 L455: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L456;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5908;

 L456: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[1])
      && (TARGET_ARM && !arm_is_xscale))
    {
      return 45;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5908;

 L466: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L467;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5908;

 L467: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && !arm_is_xscale))
    {
      return 46;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5908;

 L791: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L5914;
  goto ret0;

 L5914: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x4) == NOT)
    goto L896;
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L792;
    }
 L5913: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[0] = x4;
      goto L805;
    }
  goto ret0;

 L896: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L897;
    }
  goto ret0;

 L897: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L898;
    }
  goto ret0;

 L898: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L899;
  goto ret0;

 L899: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L900;
    case CLOBBER:
      goto L915;
    default:
     break;
   }
  goto ret0;

 L900: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L901;
    }
  goto ret0;

 L901: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == AND)
    goto L902;
  goto ret0;

 L902: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == NOT)
    goto L903;
  goto ret0;

 L903: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L904;
  goto ret0;

 L904: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[1])
      && (TARGET_ARM))
    {
      return 92;
    }
  goto ret0;

 L915: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L916;
    }
  goto ret0;

 L916: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 93;
    }
  goto ret0;

 L792: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_not_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L793;
    }
  x4 = XEXP (x3, 0);
  goto L5913;

 L793: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L794;
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5913;

 L794: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L795;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5913;

 L795: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L796;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5913;

 L796: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == AND)
    goto L797;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5913;

 L797: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L798;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5913;

 L798: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 82;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5913;

 L805: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_not_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L806;
    }
  goto ret0;

 L806: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L807;
  goto ret0;

 L807: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L808;
  goto ret0;

 L808: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L809;
    }
  goto ret0;

 L809: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 83;
    }
  goto ret0;

 L962: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L963;
    }
  goto ret0;

 L963: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L964;
    }
  goto ret0;

 L964: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L965;
  goto ret0;

 L965: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L966;
    case CLOBBER:
      goto L979;
    default:
     break;
   }
  goto ret0;

 L966: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L967;
    }
  goto ret0;

 L967: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == IOR)
    goto L968;
  goto ret0;

 L968: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L969;
  goto ret0;

 L969: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 99;
    }
  goto ret0;

 L979: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L980;
    }
  goto ret0;

 L980: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 100;
    }
  goto ret0;

 L1025: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L1026;
    }
  goto ret0;

 L1026: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L1027;
    }
  goto ret0;

 L1027: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L1028;
  goto ret0;

 L1028: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1029;
  goto ret0;

 L1029: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1030;
    }
  goto ret0;

 L1030: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == XOR)
    goto L1031;
  goto ret0;

 L1031: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L1032;
  goto ret0;

 L1032: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 106;
    }
  goto ret0;

 L1210: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L5915;
  goto ret0;

 L5915: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x4, SImode))
    {
      operands[3] = x4;
      goto L1211;
    }
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L1384;
    }
  goto ret0;

 L1211: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L1212;
    }
  goto ret0;

 L1212: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (arm_rhs_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L1213;
    }
  goto ret0;

 L1213: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L1214;
  goto ret0;

 L1214: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L1215;
    case CLOBBER:
      goto L1230;
    default:
     break;
   }
  goto ret0;

 L1215: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1216;
    }
  goto ret0;

 L1216: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L1217;
  goto ret0;

 L1217: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[3]))
    {
      operands[3] = x3;
      goto L1218;
    }
  goto ret0;

 L1218: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1219;
  goto ret0;

 L1219: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && (TARGET_ARM))
    {
      return 123;
    }
  goto ret0;

 L1230: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1231;
    }
  goto ret0;

 L1231: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 124;
    }
  goto ret0;

 L1384: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L1385;
  goto ret0;

 L1385: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L1386;
    case CLOBBER:
      goto L1397;
    default:
     break;
   }
  goto ret0;

 L1386: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1387;
    }
  goto ret0;

 L1387: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L1388;
  goto ret0;

 L1388: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1])
      && (TARGET_ARM))
    {
      return 146;
    }
  goto ret0;

 L1397: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1398;
    }
  goto ret0;

 L1398: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 147;
    }
  goto ret0;

 L1172: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L1173;
    }
  goto ret0;

 L1173: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L1174;
    }
  goto ret0;

 L1174: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L1175;
  goto ret0;

 L1175: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L1176;
    case CLOBBER:
      goto L1189;
    default:
     break;
   }
  goto ret0;

 L1176: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1177;
    }
  goto ret0;

 L1177: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (rtx_equal_p (x2, operands[3]))
    {
      operands[3] = x2;
      goto L1178;
    }
  goto ret0;

 L1178: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L1179;
  goto ret0;

 L1179: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 120;
    }
  goto ret0;

 L1189: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1190;
    }
  goto ret0;

 L1190: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 121;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_2 PARAMS ((rtx, rtx, int *));
static int
recog_2 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case DImode:
      goto L5876;
    case CC_NOOVmode:
      goto L5878;
    case CCmode:
      goto L5879;
    case CC_Cmode:
      goto L5880;
    case SImode:
      goto L5882;
    case HImode:
      goto L5885;
    default:
      break;
    }
  goto ret0;

 L5876: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x2, DImode))
    {
      operands[0] = x2;
      goto L3;
    }
 L5877: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[0] = x2;
      goto L17;
    }
 L5881: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x2, DImode))
    {
      operands[0] = x2;
      goto L267;
    }
  goto ret0;

 L3: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode
      && GET_CODE (x2) == PLUS)
    goto L4;
  x2 = XEXP (x1, 0);
  goto L5877;

 L4: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L5;
    }
  x2 = XEXP (x1, 0);
  goto L5877;

 L5: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (register_operand (x3, DImode))
    {
      operands[2] = x3;
      goto L6;
    }
  x2 = XEXP (x1, 0);
  goto L5877;

 L6: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L7;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5877;

 L7: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_THUMB))
    {
      return 0;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5877;

 L17: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode)
    goto L5886;
  x2 = XEXP (x1, 0);
  goto L5881;

 L5886: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case PLUS:
      goto L18;
    case MINUS:
      goto L254;
    case NEG:
      goto L1244;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L5881;

 L18: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode)
    goto L5890;
  x2 = XEXP (x1, 0);
  goto L5881;

 L5890: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case SIGN_EXTEND:
      goto L33;
    case ZERO_EXTEND:
      goto L49;
    case SUBREG:
    case REG:
      goto L5889;
    default:
      x2 = XEXP (x1, 0);
      goto L5881;
   }
 L5889: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L19;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L33: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L34;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L34: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L35;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L35: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L36;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L36: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 2;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L49: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L50;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L50: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L51;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L51: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L52;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L52: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 3;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L19: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[2] = x3;
      goto L20;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L20: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L21;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L21: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 1;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L254: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode)
    goto L5893;
  x2 = XEXP (x1, 0);
  goto L5881;

 L5893: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case ZERO_EXTEND:
      goto L315;
    case SIGN_EXTEND:
      goto L331;
    case SUBREG:
    case REG:
      goto L5892;
    default:
      x2 = XEXP (x1, 0);
      goto L5881;
   }
 L5892: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L255;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L315: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L5895;
  x2 = XEXP (x1, 0);
  goto L5881;

 L5895: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L316;
    }
 L5896: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L348;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L316: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L317;
    }
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5896;

 L317: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L318;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5896;

 L318: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 30;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5896;

 L348: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == ZERO_EXTEND)
    goto L349;
  x2 = XEXP (x1, 0);
  goto L5881;

 L349: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L350;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L350: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L351;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L351: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 32;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L331: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L332;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L332: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L333;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L333: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L334;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L334: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 31;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L255: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DImode)
    goto L5898;
  x2 = XEXP (x1, 0);
  goto L5881;

 L5898: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case ZERO_EXTEND:
      goto L284;
    case SIGN_EXTEND:
      goto L300;
    case SUBREG:
    case REG:
      goto L5897;
    default:
      x2 = XEXP (x1, 0);
      goto L5881;
   }
 L5897: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DImode))
    {
      operands[2] = x3;
      goto L256;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L284: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L285;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L285: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L286;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L286: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 28;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L300: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L301;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L301: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L302;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L302: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 29;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L256: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L257;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L257: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 26;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L1244: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L1245;
    }
  x2 = XEXP (x1, 0);
  goto L5881;

 L1245: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1246;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L1246: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 125;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5881;

 L267: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode)
    goto L5900;
  goto ret0;

 L5900: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case MINUS:
      goto L268;
    case NEG:
      goto L1256;
    default:
     break;
   }
  goto ret0;

 L268: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L269;
    }
  goto ret0;

 L269: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (register_operand (x3, DImode))
    {
      operands[2] = x3;
      goto L270;
    }
  goto ret0;

 L270: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L271;
  goto ret0;

 L271: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_THUMB))
    {
      return 27;
    }
  goto ret0;

 L1256: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L1257;
    }
  goto ret0;

 L1257: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1258;
  goto ret0;

 L1258: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_THUMB))
    {
      return 126;
    }
  goto ret0;

 L5878: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == REG
      && XINT (x2, 0) == 24)
    goto L75;
  goto ret0;

 L75: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == CC_NOOVmode
      && GET_CODE (x2) == COMPARE)
    goto L76;
  goto ret0;

 L76: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L5902;
  goto ret0;

 L5902: ATTRIBUTE_UNUSED_LABEL
  return recog_1 (x0, insn, pnum_clobbers);

 L5879: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == REG
      && XINT (x2, 0) == 24)
    goto L95;
  goto ret0;

 L95: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == COMPARE)
    goto L96;
  goto ret0;

 L96: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L97;
    }
  goto ret0;

 L97: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == NEG)
    goto L98;
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L1604;
  goto ret0;

 L98: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (arm_add_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L99;
    }
  goto ret0;

 L99: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L100;
  goto ret0;

 L100: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L101;
    }
  goto ret0;

 L101: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L102;
  goto ret0;

 L102: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L103;
  goto ret0;

 L103: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 8;
    }
  goto ret0;

 L1604: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1605;
  goto ret0;

 L1605: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1606;
    }
  goto ret0;

 L1606: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (rtx_equal_p (x2, operands[1])
      && (TARGET_ARM))
    {
      return 182;
    }
  goto ret0;

 L5880: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == REG
      && XINT (x2, 0) == 24)
    goto L114;
  goto ret0;

 L114: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == CC_Cmode
      && GET_CODE (x2) == COMPARE)
    goto L115;
  goto ret0;

 L115: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L116;
  goto ret0;

 L116: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L117;
    }
  goto ret0;

 L117: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L118;
    }
  goto ret0;

 L118: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[1]))
    goto L119;
 L131: ATTRIBUTE_UNUSED_LABEL
  if (rtx_equal_p (x3, operands[2]))
    goto L132;
  goto ret0;

 L119: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L120;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L131;

 L120: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L121;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L131;

 L121: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L122;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L131;

 L122: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L123;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L131;

 L123: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 10;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L131;

 L132: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L133;
  goto ret0;

 L133: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L134;
    }
  goto ret0;

 L134: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L135;
  goto ret0;

 L135: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L136;
  goto ret0;

 L136: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 11;
    }
  goto ret0;

 L5882: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L561;
    }
 L5883: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1108;
    }
 L5884: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1493;
    }
  goto ret0;

 L561: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L5917;
  x2 = XEXP (x1, 0);
  goto L5883;

 L5917: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case TRUNCATE:
      goto L562;
    case NE:
      goto L829;
    case SMAX:
      goto L1053;
    case SMIN:
      goto L1067;
    case UMAX:
      goto L1081;
    case UMIN:
      goto L1095;
    case ABS:
      goto L1299;
    case NEG:
      goto L1311;
    case PLUS:
    case MINUS:
    case AND:
    case IOR:
    case XOR:
      goto L5923;
    default:
      x2 = XEXP (x1, 0);
      goto L5883;
   }
 L5923: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x2, SImode))
    {
      operands[4] = x2;
      goto L1123;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L562: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == LSHIFTRT)
    goto L563;
  x2 = XEXP (x1, 0);
  goto L5883;

 L563: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == DImode
      && GET_CODE (x4) == MULT)
    goto L564;
  x2 = XEXP (x1, 0);
  goto L5883;

 L564: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (GET_MODE (x5) == DImode)
    goto L5926;
  x2 = XEXP (x1, 0);
  goto L5883;

 L5926: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x5))
    {
    case SIGN_EXTEND:
      goto L565;
    case ZERO_EXTEND:
      goto L589;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L5883;

 L565: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[1] = x6;
      goto L566;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L566: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_MODE (x5) == DImode
      && GET_CODE (x5) == SIGN_EXTEND)
    goto L567;
  x2 = XEXP (x1, 0);
  goto L5883;

 L567: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[2] = x6;
      goto L568;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L568: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 32)
    goto L569;
  x2 = XEXP (x1, 0);
  goto L5883;

 L569: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L570;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L570: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L571;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L571: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_fast_multiply))
    {
      return 54;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L589: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[1] = x6;
      goto L590;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L590: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_MODE (x5) == DImode
      && GET_CODE (x5) == ZERO_EXTEND)
    goto L591;
  x2 = XEXP (x1, 0);
  goto L5883;

 L591: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[2] = x6;
      goto L592;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L592: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 32)
    goto L593;
  x2 = XEXP (x1, 0);
  goto L5883;

 L593: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L594;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L594: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L595;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L595: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_fast_multiply))
    {
      return 55;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L829: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == ZERO_EXTRACT)
    goto L830;
  x2 = XEXP (x1, 0);
  goto L5883;

 L830: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L831;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L831: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (const_int_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L832;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L832: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 2);
  if (const_int_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L833;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L833: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L834;
  x2 = XEXP (x1, 0);
  goto L5883;

 L834: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L835;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L835: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM
   && (INTVAL (operands[3]) >= 0 && INTVAL (operands[3]) < 32
       && INTVAL (operands[2]) > 0 
       && INTVAL (operands[2]) + (INTVAL (operands[3]) & 1) <= 8
       && INTVAL (operands[2]) + INTVAL (operands[3]) <= 32)))
    {
      return 85;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1053: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1054;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1054: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1055;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1055: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1056;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1056: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 109;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1067: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1068;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1068: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1069;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1069: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1070;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1070: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 110;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1081: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1082;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1082: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1083;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1083: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1084;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1084: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 111;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1095: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1096;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1096: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1097;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1097: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1098;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1098: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 112;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1299: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1300;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1300: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1301;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1301: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 133;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1311: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == ABS)
    goto L1312;
  x2 = XEXP (x1, 0);
  goto L5883;

 L1312: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L1313;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1313: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1314;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1314: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 134;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1123: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (minmax_operator (x3, SImode))
    {
      operands[5] = x3;
      goto L1124;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1124: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L1125;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1125: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L1126;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1126: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1127;
    }
  x2 = XEXP (x1, 0);
  goto L5883;

 L1127: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1128;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1128: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM
   && (GET_CODE (operands[1]) != REG
       || (REGNO(operands[1]) != FRAME_POINTER_REGNUM
           && REGNO(operands[1]) != ARG_POINTER_REGNUM))))
    {
      return 114;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5883;

 L1108: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (minmax_operator (x2, SImode))
    {
      operands[3] = x2;
      goto L1109;
    }
  x2 = XEXP (x1, 0);
  goto L5884;

 L1109: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1110;
    }
  x2 = XEXP (x1, 0);
  goto L5884;

 L1110: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1111;
    }
  x2 = XEXP (x1, 0);
  goto L5884;

 L1111: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1112;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5884;

 L1112: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 113;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5884;

 L1493: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L5928;
  goto ret0;

 L5928: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case SIGN_EXTEND:
      goto L1494;
    case UNSPEC:
      goto L5930;
    default:
     break;
   }
  goto ret0;

 L1494: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (memory_operand (x3, HImode))
    {
      operands[1] = x3;
      goto L1495;
    }
  goto ret0;

 L1495: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1496;
  goto ret0;

 L1496: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1497;
    }
  goto ret0;

 L1497: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 165;
    }
  goto ret0;

 L5930: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x2, 0) == 1
      && XINT (x2, 1) == 4)
    goto L1574;
  goto ret0;

 L1574: ATTRIBUTE_UNUSED_LABEL
  x3 = XVECEXP (x2, 0, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1575;
  goto ret0;

 L1575: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[0]))
    goto L1576;
  goto ret0;

 L1576: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST)
    goto L1577;
  goto ret0;

 L1577: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (GET_MODE (x5) == SImode
      && GET_CODE (x5) == PLUS)
    goto L1578;
  goto ret0;

 L1578: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (GET_CODE (x6) == PC)
    goto L1579;
  goto ret0;

 L1579: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (GET_CODE (x6) == CONST_INT)
    goto L5931;
  goto ret0;

 L5931: ATTRIBUTE_UNUSED_LABEL
  if ((int) XWINT (x6, 0) == XWINT (x6, 0))
    switch ((int) XWINT (x6, 0))
      {
      case 4:
        goto L1580;
      case 8:
        goto L1594;
      default:
        break;
      }
  goto ret0;

 L1580: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L1581;
  goto ret0;

 L1581: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_CODE (x2) == LABEL_REF)
    goto L1582;
  goto ret0;

 L1582: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[1] = x3;
  goto L1583;

 L1583: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB && flag_pic))
    {
      return 180;
    }
  goto ret0;

 L1594: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L1595;
  goto ret0;

 L1595: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_CODE (x2) == LABEL_REF)
    goto L1596;
  goto ret0;

 L1596: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[1] = x3;
  goto L1597;

 L1597: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && flag_pic))
    {
      return 181;
    }
  goto ret0;

 L5885: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x2, HImode))
    {
      operands[0] = x2;
      goto L1641;
    }
  goto ret0;

 L1641: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, HImode))
    {
      operands[1] = x2;
      goto L1642;
    }
  goto ret0;

 L1642: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L1643;
  goto ret0;

 L1643: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1644;
    }
  goto ret0;

 L1644: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 190;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_3 PARAMS ((rtx, rtx, int *));
static int
recog_3 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XVECEXP (x0, 0, 0);
  switch (GET_CODE (x1))
    {
    case CALL:
      goto L2290;
    case SET:
      goto L2341;
    default:
     break;
   }
  goto ret0;

 L2290: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2291;
  goto ret0;

 L2291: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L5944;
 L2405: ATTRIBUTE_UNUSED_LABEL
  operands[0] = x3;
  goto L2406;

 L5944: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L2292;
    }
 L5945: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L2309;
    }
 L5946: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L2326;
    }
  goto L2405;

 L2292: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  operands[1] = x2;
  goto L2293;

 L2293: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2294;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5945;

 L2294: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[2] = x2;
  goto L2295;

 L2295: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == CLOBBER)
    goto L2296;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5945;

 L2296: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 14
      && (TARGET_ARM))
    {
      return 246;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5945;

 L2309: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  operands[1] = x2;
  goto L2310;

 L2310: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2311;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5946;

 L2311: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[2] = x2;
  goto L2312;

 L2312: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == CLOBBER)
    goto L2313;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5946;

 L2313: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 14
      && (TARGET_ARM))
    {
      return 247;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5946;

 L2326: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  operands[1] = x2;
  goto L2327;

 L2327: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2328;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2405;

 L2328: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[2] = x2;
  goto L2329;

 L2329: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == CLOBBER)
    goto L2330;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2405;

 L2330: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 14
      && (TARGET_THUMB))
    {
      return 248;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2405;

 L2406: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  operands[1] = x2;
  goto L2407;

 L2407: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case USE:
      goto L2408;
    case RETURN:
      goto L2484;
    default:
     break;
   }
  goto ret0;

 L2408: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[2] = x2;
  goto L2409;

 L2409: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == CLOBBER)
    goto L2410;
  goto ret0;

 L2410: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L5947;
  goto ret0;

 L5947: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == REG)
    goto L5949;
  goto ret0;

 L5949: ATTRIBUTE_UNUSED_LABEL
  if (XINT (x2, 0) == 14)
    goto L5951;
  goto ret0;

 L5951: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && (GET_CODE (operands[0]) == SYMBOL_REF)
   && !arm_is_longcall_p (operands[0], INTVAL (operands[2]), 1)))
    {
      return 252;
    }
 L5952: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && GET_CODE (operands[0]) == SYMBOL_REF
   && !arm_is_longcall_p (operands[0], INTVAL (operands[2]), 1)))
    {
      return 254;
    }
  goto ret0;

 L2484: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == USE)
    goto L2485;
  goto ret0;

 L2485: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[2] = x2;
  goto L2486;

 L2486: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && GET_CODE (operands[0]) == SYMBOL_REF))
    {
      return 256;
    }
  goto ret0;

 L2341: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[0] = x2;
  goto L2342;
 L2525: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == PC)
    goto L2526;
  if (s_register_operand (x2, VOIDmode))
    {
      operands[0] = x2;
      goto L2422;
    }
 L2459: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x2, VOIDmode))
    {
      operands[0] = x2;
      goto L2460;
    }
 L2489: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, VOIDmode))
    {
      operands[0] = x2;
      goto L2490;
    }
  goto ret0;

 L2342: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CALL)
    goto L2343;
  x2 = XEXP (x1, 0);
  goto L2525;

 L2343: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L2344;
  x2 = XEXP (x1, 0);
  goto L2525;

 L2344: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L5953;
  x2 = XEXP (x1, 0);
  goto L2525;

 L5953: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2345;
    }
 L5954: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2366;
    }
 L5955: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2387;
    }
  x2 = XEXP (x1, 0);
  goto L2525;

 L2345: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2346;

 L2346: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2347;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5954;

 L2347: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2348;

 L2348: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == CLOBBER)
    goto L2349;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5954;

 L2349: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 14
      && (TARGET_THUMB))
    {
      return 249;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5954;

 L2366: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2367;

 L2367: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2368;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5955;

 L2368: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2369;

 L2369: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == CLOBBER)
    goto L2370;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5955;

 L2370: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 14
      && (TARGET_ARM))
    {
      return 250;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5955;

 L2387: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2388;

 L2388: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2389;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2525;

 L2389: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2390;

 L2390: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == CLOBBER)
    goto L2391;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2525;

 L2391: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 14
      && (TARGET_ARM && (!CONSTANT_ADDRESS_P (XEXP (operands[1], 0)))))
    {
      return 251;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2525;

 L2526: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == IF_THEN_ELSE)
    goto L2527;
  goto ret0;

 L2527: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_CODE (x3) == LEU)
    goto L2528;
  goto ret0;

 L2528: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[0] = x4;
      goto L2529;
    }
  goto ret0;

 L2529: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2530;
    }
  goto ret0;

 L2530: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L2531;
  goto ret0;

 L2531: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L2532;
  goto ret0;

 L2532: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (GET_MODE (x5) == SImode
      && GET_CODE (x5) == MULT)
    goto L2533;
  goto ret0;

 L2533: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (rtx_equal_p (x6, operands[0]))
    goto L2534;
  goto ret0;

 L2534: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (GET_CODE (x6) == CONST_INT
      && XWINT (x6, 0) == 4)
    goto L2535;
  goto ret0;

 L2535: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == LABEL_REF)
    goto L2536;
  goto ret0;

 L2536: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  operands[2] = x6;
  goto L2537;

 L2537: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (GET_CODE (x3) == LABEL_REF)
    goto L2538;
  goto ret0;

 L2538: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  operands[3] = x4;
  goto L2539;

 L2539: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2540;
  goto ret0;

 L2540: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24)
    goto L2541;
  goto ret0;

 L2541: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == USE)
    goto L2542;
  goto ret0;

 L2542: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_CODE (x2) == LABEL_REF)
    goto L2543;
  goto ret0;

 L2543: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 263;
    }
  goto ret0;

 L2422: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CALL)
    goto L2423;
  x2 = XEXP (x1, 0);
  goto L2459;

 L2423: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L2424;
  x2 = XEXP (x1, 0);
  goto L2459;

 L2424: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  operands[1] = x4;
  goto L2425;

 L2425: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2426;

 L2426: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2427;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2459;

 L2427: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2428;

 L2428: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == CLOBBER)
    goto L2429;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2459;

 L2429: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 14
      && (TARGET_ARM
   && (GET_CODE (operands[1]) == SYMBOL_REF)
   && !arm_is_longcall_p (operands[1], INTVAL (operands[3]), 1)))
    {
      return 253;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2459;

 L2460: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CALL)
    goto L2461;
  x2 = XEXP (x1, 0);
  goto L2489;

 L2461: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L2462;
  x2 = XEXP (x1, 0);
  goto L2489;

 L2462: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  operands[1] = x4;
  goto L2463;

 L2463: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2464;

 L2464: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2465;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2489;

 L2465: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2466;

 L2466: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == CLOBBER)
    goto L2467;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2489;

 L2467: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 14
      && (TARGET_THUMB
   && GET_CODE (operands[1]) == SYMBOL_REF
   && !arm_is_longcall_p (operands[1], INTVAL (operands[3]), 1)))
    {
      return 255;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2489;

 L2490: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CALL)
    goto L2491;
  goto ret0;

 L2491: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L2492;
  goto ret0;

 L2492: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  operands[1] = x4;
  goto L2493;

 L2493: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2494;

 L2494: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == RETURN)
    goto L2495;
  goto ret0;

 L2495: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == USE)
    goto L2496;
  goto ret0;

 L2496: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2497;

 L2497: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && GET_CODE (operands[1]) == SYMBOL_REF))
    {
      return 257;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_4 PARAMS ((rtx, rtx, int *));
static int
recog_4 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[5] = x3;
      goto L2795;
    }
 L2814: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[6] = x3;
      goto L2815;
    }
 L2884: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[9] = x3;
      goto L2885;
    }
 L3129: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[7] = x3;
      goto L3130;
    }
  goto ret0;

 L2795: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2796;
    }
  goto L2814;

 L2796: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L3030;
    }
  goto L2814;

 L3030: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode)
    goto L5985;
 L2797: ATTRIBUTE_UNUSED_LABEL
  if (arm_rhs_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2798;
    }
 L2998: ATTRIBUTE_UNUSED_LABEL
  if (arm_not_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2999;
    }
  x3 = XEXP (x2, 0);
  goto L2814;

 L5985: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case NOT:
      goto L3031;
    case NEG:
      goto L3250;
    default:
     break;
   }
  goto L2797;

 L3031: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3032;
    }
  goto L2797;

 L3032: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (arm_not_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3033;
    }
  x3 = XEXP (x2, 1);
  goto L2797;

 L3033: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3034;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2797;

 L3034: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 298;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2797;

 L3250: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3251;
    }
  goto L2797;

 L3251: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (arm_not_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3252;
    }
  x3 = XEXP (x2, 1);
  goto L2797;

 L3252: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3253;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2797;

 L3253: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 310;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2797;

 L2798: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2799;
    }
  x3 = XEXP (x2, 1);
  goto L2998;

 L2799: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2800;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2998;

 L2800: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 285;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2998;

 L2999: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (GET_MODE (x3) == SImode)
    goto L5987;
  x3 = XEXP (x2, 0);
  goto L2814;

 L5987: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case NOT:
      goto L3000;
    case NEG:
      goto L3283;
    default:
     break;
   }
  x3 = XEXP (x2, 0);
  goto L2814;

 L3000: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3001;
    }
  x3 = XEXP (x2, 0);
  goto L2814;

 L3001: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3002;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L2814;

 L3002: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 296;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L2814;

 L3283: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3284;
    }
  x3 = XEXP (x2, 0);
  goto L2814;

 L3284: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3285;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L2814;

 L3285: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 312;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L2814;

 L2815: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L5989;
  goto L2884;

 L5989: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L2816;
    }
 L5990: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2927;
    }
  goto L2884;

 L2816: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[5] = x4;
      goto L2817;
    }
  x4 = XEXP (x3, 0);
  goto L5990;

 L2817: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode)
    goto L5991;
 L2852: ATTRIBUTE_UNUSED_LABEL
  if (arm_rhs_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2853;
    }
 L3097: ATTRIBUTE_UNUSED_LABEL
  if (arm_not_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3098;
    }
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5990;

 L5991: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L2818;
    case NOT:
      goto L3174;
    case ASHIFT:
    case ASHIFTRT:
    case LSHIFTRT:
    case ROTATERT:
    case MULT:
      goto L5992;
    default:
      goto L5994;
   }
 L5992: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x3, SImode))
    {
      operands[7] = x3;
      goto L3063;
    }
 L5994: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x3, SImode))
    {
      operands[7] = x3;
      goto L3212;
    }
  goto L2852;

 L2818: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2819;
    }
  goto L5994;

 L2819: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2820;
    }
  goto L5994;

 L2820: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2821;
    }
  x3 = XEXP (x2, 1);
  goto L5994;

 L2821: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2822;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L5994;

 L2822: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 286;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L5994;

 L3174: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3175;
    }
  goto L2852;

 L3175: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (shiftable_operator (x3, SImode))
    {
      operands[7] = x3;
      goto L3176;
    }
  x3 = XEXP (x2, 1);
  goto L2852;

 L3176: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3177;
    }
  x3 = XEXP (x2, 1);
  goto L2852;

 L3177: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L3178;
    }
  x3 = XEXP (x2, 1);
  goto L2852;

 L3178: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3179;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2852;

 L3179: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 306;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2852;

 L3063: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3064;
    }
  goto L2852;

 L3064: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L3065;
    }
  goto L2852;

 L3065: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (arm_not_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3066;
    }
  x3 = XEXP (x2, 1);
  goto L2852;

 L3066: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3067;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2852;

 L3067: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 300;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2852;

 L3212: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3213;
    }
  goto L2852;

 L3213: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L3214;
    }
  goto L2852;

 L3214: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == NOT)
    goto L3215;
  x3 = XEXP (x2, 1);
  goto L2852;

 L3215: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3216;
    }
  x3 = XEXP (x2, 1);
  goto L2852;

 L3216: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3217;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2852;

 L3217: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 308;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L2852;

 L2853: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (GET_MODE (x3) == SImode)
    goto L5995;
  x3 = XEXP (x2, 1);
  goto L3097;

 L5995: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == PLUS)
    goto L2854;
 L5996: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x3, SImode))
    {
      operands[7] = x3;
      goto L2965;
    }
  x3 = XEXP (x2, 1);
  goto L3097;

 L2854: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2855;
    }
  goto L5996;

 L2855: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2856;
    }
  goto L5996;

 L2856: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2857;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 2);
  goto L5996;

 L2857: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 288;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 2);
  goto L5996;

 L2965: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2966;
    }
  x3 = XEXP (x2, 1);
  goto L3097;

 L2966: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2967;
    }
  x3 = XEXP (x2, 1);
  goto L3097;

 L2967: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2968;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L3097;

 L2968: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 294;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L3097;

 L3098: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (shift_operator (x3, SImode))
    {
      operands[7] = x3;
      goto L3099;
    }
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5990;

 L3099: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3100;
    }
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5990;

 L3100: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L3101;
    }
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5990;

 L3101: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3102;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5990;

 L3102: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 302;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5990;

 L2927: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2928;
    }
  goto L2884;

 L2928: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (shiftable_operator (x3, SImode))
    {
      operands[7] = x3;
      goto L2929;
    }
  x3 = XEXP (x2, 0);
  goto L2884;

 L2929: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L2930;
    }
  x3 = XEXP (x2, 0);
  goto L2884;

 L2930: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[5] = x4;
      goto L2931;
    }
  x3 = XEXP (x2, 0);
  goto L2884;

 L2931: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2932;
    }
  x3 = XEXP (x2, 0);
  goto L2884;

 L2932: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2933;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L2884;

 L2933: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 292;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L2884;

 L2885: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[5] = x4;
      goto L2886;
    }
  goto L3129;

 L2886: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[6] = x4;
      goto L2887;
    }
  goto L3129;

 L2887: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (shiftable_operator (x3, SImode))
    {
      operands[8] = x3;
      goto L2888;
    }
  x3 = XEXP (x2, 0);
  goto L3129;

 L2888: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2889;
    }
  x3 = XEXP (x2, 0);
  goto L3129;

 L2889: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2890;
    }
  x3 = XEXP (x2, 0);
  goto L3129;

 L2890: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (shiftable_operator (x3, SImode))
    {
      operands[7] = x3;
      goto L2891;
    }
  x3 = XEXP (x2, 0);
  goto L3129;

 L2891: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2892;
    }
  x3 = XEXP (x2, 0);
  goto L3129;

 L2892: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L2893;
    }
  x3 = XEXP (x2, 0);
  goto L3129;

 L2893: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2894;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L3129;

 L2894: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 290;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L3129;

 L3130: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[5] = x4;
      goto L3131;
    }
  goto ret0;

 L3131: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[6] = x4;
      goto L3132;
    }
  goto ret0;

 L3132: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (shift_operator (x3, SImode))
    {
      operands[8] = x3;
      goto L3133;
    }
  goto ret0;

 L3133: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3134;
    }
  goto ret0;

 L3134: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3135;
    }
  goto ret0;

 L3135: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (shift_operator (x3, SImode))
    {
      operands[9] = x3;
      goto L3136;
    }
  goto ret0;

 L3136: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L3137;
    }
  goto ret0;

 L3137: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L3138;
    }
  goto ret0;

 L3138: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3139;
  goto ret0;

 L3139: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 304;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_5 PARAMS ((rtx, rtx, int *));
static int
recog_5 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  if (GET_CODE (x2) == MEM)
    goto L3398;
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L2670;
    }
 L5960: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L3372;
    }
 L5964: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L3566;
    }
  goto ret0;

 L3398: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L5971;
  goto ret0;

 L5971: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L3399;
    case MINUS:
      goto L3411;
    default:
     break;
   }
  goto ret0;

 L3399: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L5973;
  goto ret0;

 L5973: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3400;
    }
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3536;
    }
  goto ret0;

 L3400: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (index_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3401;
    }
  goto ret0;

 L3401: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L3402;
    }
  goto ret0;

 L3402: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3403;
  goto ret0;

 L3403: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3404;
    }
  goto ret0;

 L3404: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3405;
  goto ret0;

 L3405: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3406;
  goto ret0;

 L3406: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 321;
    }
  goto ret0;

 L3536: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3537;
    }
  goto ret0;

 L3537: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3538;
    }
  goto ret0;

 L3538: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3539;
    }
  goto ret0;

 L3539: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L3540;
    }
  goto ret0;

 L3540: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3541;
  goto ret0;

 L3541: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3542;
    }
  goto ret0;

 L3542: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3543;
  goto ret0;

 L3543: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3544;
    }
  goto ret0;

 L3544: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3545;
  goto ret0;

 L3545: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4]))
    goto L3546;
  goto ret0;

 L3546: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[1])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 331;
    }
  goto ret0;

 L3411: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3412;
    }
  goto ret0;

 L3412: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_MODE (x4) == SImode)
    goto L5975;
  goto ret0;

 L5975: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3413;
    }
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3553;
    }
  goto ret0;

 L3413: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L3414;
    }
  goto ret0;

 L3414: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3415;
  goto ret0;

 L3415: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3416;
    }
  goto ret0;

 L3416: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3417;
  goto ret0;

 L3417: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3418;
  goto ret0;

 L3418: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 322;
    }
  goto ret0;

 L3553: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3554;
    }
  goto ret0;

 L3554: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3555;
    }
  goto ret0;

 L3555: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L3556;
    }
  goto ret0;

 L3556: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3557;
  goto ret0;

 L3557: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3558;
    }
  goto ret0;

 L3558: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3559;
  goto ret0;

 L3559: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3560;
  goto ret0;

 L3560: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3561;
    }
  goto ret0;

 L3561: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3562;
  goto ret0;

 L3562: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 332;
    }
  goto ret0;

 L2670: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L5979;
  x2 = XEXP (x1, 0);
  goto L5960;

 L5979: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case MINUS:
      goto L2714;
    case NEG:
      goto L2778;
    case IF_THEN_ELSE:
      goto L2794;
    case SIGN_EXTRACT:
      goto L3648;
    case NOT:
      goto L3664;
    case EQ:
    case NE:
    case LE:
    case LT:
    case GE:
    case GT:
    case GEU:
    case GTU:
    case LEU:
    case LTU:
    case UNORDERED:
    case ORDERED:
    case UNLT:
    case UNLE:
    case UNGE:
    case UNGT:
      goto L5977;
    default:
      goto L5978;
   }
 L5977: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, SImode))
    {
      operands[1] = x2;
      goto L2671;
    }
 L5978: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x2, SImode))
    {
      operands[5] = x2;
      goto L2696;
    }
 L5982: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x2, SImode))
    {
      operands[1] = x2;
      goto L3310;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L2714: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2715;
    }
  goto L5978;

 L2715: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_comparison_operator (x3, SImode))
    {
      operands[4] = x3;
      goto L2716;
    }
  goto L5978;

 L2716: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2717;
    }
  goto L5978;

 L2717: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2718;
    }
  goto L5978;

 L2718: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2719;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L5978;

 L2719: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 279;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L5978;

 L2778: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[3] = x3;
      goto L2779;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L2779: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2780;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L2780: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2781;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L2781: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2782;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L2782: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 284;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L2794: ATTRIBUTE_UNUSED_LABEL
  tem = recog_4 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  x2 = XEXP (x1, 0);
  goto L5960;

 L3648: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3649;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L3649: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 1)
    goto L3650;
  x2 = XEXP (x1, 0);
  goto L5960;

 L3650: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (const_int_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3651;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L3651: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3652;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L3652: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 340;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L3664: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == SIGN_EXTRACT)
    goto L3665;
  x2 = XEXP (x1, 0);
  goto L5960;

 L3665: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3666;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L3666: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 1)
    goto L3667;
  x2 = XEXP (x1, 0);
  goto L5960;

 L3667: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 2);
  if (const_int_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3668;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L3668: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3669;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L3669: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 341;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L2671: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2672;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L2672: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2673;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L2673: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2674;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L2674: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 276;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L2696: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_comparison_operator (x3, SImode))
    {
      operands[4] = x3;
      goto L2697;
    }
  goto L5982;

 L2697: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2698;
    }
  goto L5982;

 L2698: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2699;
    }
  goto L5982;

 L2699: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2700;
    }
  goto L5982;

 L2700: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L2701;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L5982;

 L2701: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM))
    {
      return 278;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L5982;

 L3310: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (memory_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3311;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L3311: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (memory_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3312;
    }
  x2 = XEXP (x1, 0);
  goto L5960;

 L3312: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3313;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L3313: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L3314;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L3314: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && adjacent_mem_locations (operands[2], operands[3])))
    {
      return 314;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5960;

 L3372: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L5997;
  x2 = XEXP (x1, 0);
  goto L5964;

 L5997: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case ZERO_EXTEND:
      goto L3373;
    case MEM:
      goto L3423;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3373: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == QImode
      && GET_CODE (x3) == MEM)
    goto L3374;
  x2 = XEXP (x1, 0);
  goto L5964;

 L3374: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L5999;
  x2 = XEXP (x1, 0);
  goto L5964;

 L5999: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L3375;
    case MINUS:
      goto L3388;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3375: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L3376;
    }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3376: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (index_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L3377;
    }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3377: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3378;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3378: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3379;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3379: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3380;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3380: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3381;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3381: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 319;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3388: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L3389;
    }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3389: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L3390;
    }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3390: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3391;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3391: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3392;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3392: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3393;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3393: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3394;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3394: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 320;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3423: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6001;
  x2 = XEXP (x1, 0);
  goto L5964;

 L6001: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L3424;
    case MINUS:
      goto L3436;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3424: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3425;
    }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3425: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (index_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3426;
    }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3426: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3427;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3427: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3428;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3428: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3429;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3429: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3430;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3430: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 323;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3436: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3437;
    }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3437: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3438;
    }
  x2 = XEXP (x1, 0);
  goto L5964;

 L3438: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3439;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3439: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3440;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3440: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3441;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3441: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3442;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3442: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 324;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5964;

 L3566: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L3567;
  goto ret0;

 L3567: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6003;
  goto ret0;

 L6003: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L3568;
    case MINUS:
      goto L3584;
    default:
     break;
   }
  goto ret0;

 L3568: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3569;
    }
  goto ret0;

 L3569: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3570;
    }
  goto ret0;

 L3570: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3571;
    }
  goto ret0;

 L3571: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3572;
    }
  goto ret0;

 L3572: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3573;
  goto ret0;

 L3573: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3574;
    }
  goto ret0;

 L3574: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3575;
  goto ret0;

 L3575: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3576;
    }
  goto ret0;

 L3576: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3577;
  goto ret0;

 L3577: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4]))
    goto L3578;
  goto ret0;

 L3578: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[1])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 333;
    }
  goto ret0;

 L3584: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3585;
    }
  goto ret0;

 L3585: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3586;
    }
  goto ret0;

 L3586: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3587;
    }
  goto ret0;

 L3587: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3588;
    }
  goto ret0;

 L3588: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3589;
  goto ret0;

 L3589: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3590;
    }
  goto ret0;

 L3590: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3591;
  goto ret0;

 L3591: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3592;
  goto ret0;

 L3592: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3593;
    }
  goto ret0;

 L3593: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3594;
  goto ret0;

 L3594: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 334;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_6 PARAMS ((rtx, rtx, int *));
static int
recog_6 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  operands[0] = x2;
  goto L2353;
 L2567: ATTRIBUTE_UNUSED_LABEL
  switch (GET_MODE (x2))
    {
    case CC_NOOVmode:
      goto L5956;
    case SImode:
      goto L5961;
    case QImode:
      goto L5958;
    case HImode:
      goto L5962;
    default:
      break;
    }
 L3714: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == PC)
    goto L3715;
  if (s_register_operand (x2, VOIDmode))
    {
      operands[0] = x2;
      goto L2433;
    }
 L2470: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x2, VOIDmode))
    {
      operands[0] = x2;
      goto L2471;
    }
  goto ret0;

 L2353: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CALL)
    goto L2354;
  x2 = XEXP (x1, 0);
  goto L2567;

 L2354: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L2355;
  x2 = XEXP (x1, 0);
  goto L2567;

 L2355: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L5966;
  x2 = XEXP (x1, 0);
  goto L2567;

 L5966: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2356;
    }
 L5967: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2377;
    }
 L5968: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2398;
    }
  x2 = XEXP (x1, 0);
  goto L2567;

 L2356: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2357;

 L2357: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2358;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5967;

 L2358: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2359;

 L2359: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 249;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5967;

 L2377: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2378;

 L2378: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2379;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5968;

 L2379: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2380;

 L2380: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 250;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L5968;

 L2398: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2399;

 L2399: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2400;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2567;

 L2400: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2401;

 L2401: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && (!CONSTANT_ADDRESS_P (XEXP (operands[1], 0))))
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 251;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2567;

 L5956: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == REG
      && XINT (x2, 0) == 24)
    goto L2568;
  goto L3714;

 L2568: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == CC_NOOVmode
      && GET_CODE (x2) == COMPARE)
    goto L2569;
  x2 = XEXP (x1, 0);
  goto L3714;

 L2569: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L5970;
  x2 = XEXP (x1, 0);
  goto L3714;

 L5970: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == MINUS)
    goto L2617;
 L5969: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x3, SImode))
    {
      operands[1] = x3;
      goto L2570;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L2617: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2618;
    }
  goto L5969;

 L2618: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L2619;
    }
  goto L5969;

 L2619: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L2620;
    }
  goto L5969;

 L2620: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (reg_or_int_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L2621;
    }
  goto L5969;

 L2621: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2622;
  x3 = XEXP (x2, 0);
  goto L5969;

 L2622: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L2623;
    case CLOBBER:
      goto L2640;
    default:
     break;
   }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5969;

 L2623: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L2624;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5969;

 L2624: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L2625;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5969;

 L2625: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L2626;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5969;

 L2626: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L2627;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5969;

 L2627: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L2628;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5969;

 L2628: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4])
      && (TARGET_ARM))
    {
      return 272;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5969;

 L2640: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L2641;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5969;

 L2641: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 273;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  goto L5969;

 L2570: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (shift_operator (x4, SImode))
    {
      operands[3] = x4;
      goto L2571;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L2571: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L2572;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L2572: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (reg_or_int_operand (x5, SImode))
    {
      operands[5] = x5;
      goto L2573;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L2573: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2574;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L2574: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2575;
  x2 = XEXP (x1, 0);
  goto L3714;

 L2575: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L2576;
    case CLOBBER:
      goto L2593;
    default:
     break;
   }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L2576: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L2577;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L2577: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (rtx_equal_p (x2, operands[1]))
    {
      operands[1] = x2;
      goto L2578;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L2578: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[3]))
    {
      operands[3] = x3;
      goto L2579;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L2579: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[4]))
    goto L2580;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L2580: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[5]))
    goto L2581;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L2581: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM))
    {
      return 269;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L2593: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L2594;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L2594: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 270;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L5961: ATTRIBUTE_UNUSED_LABEL
  tem = recog_5 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  goto L3714;

 L5958: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == MEM)
    goto L3324;
  if (s_register_operand (x2, QImode))
    {
      operands[3] = x2;
      goto L3348;
    }
 L5963: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, QImode))
    {
      operands[5] = x2;
      goto L3502;
    }
  goto L3714;

 L3324: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6005;
  goto L3714;

 L6005: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L3325;
    case MINUS:
      goto L3337;
    default:
     break;
   }
  goto L3714;

 L3325: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6007;
  goto L3714;

 L6007: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3326;
    }
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3472;
    }
  goto L3714;

 L3326: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (index_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3327;
    }
  goto L3714;

 L3327: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, QImode))
    {
      operands[3] = x2;
      goto L3328;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3328: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3329;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3329: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3330;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3330: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3331;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3331: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3332;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3332: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 315;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3472: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3473;
    }
  goto L3714;

 L3473: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3474;
    }
  goto L3714;

 L3474: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3475;
    }
  goto L3714;

 L3475: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, QImode))
    {
      operands[5] = x2;
      goto L3476;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3476: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3477;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3477: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3478;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3478: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3479;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3479: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3480;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3480: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3481;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3481: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4]))
    goto L3482;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3482: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[1])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 327;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3337: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3338;
    }
  goto L3714;

 L3338: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_MODE (x4) == SImode)
    goto L6009;
  goto L3714;

 L6009: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3339;
    }
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3489;
    }
  goto L3714;

 L3339: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, QImode))
    {
      operands[3] = x2;
      goto L3340;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3340: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3341;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3341: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3342;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3342: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3343;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3343: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3344;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3344: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 316;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3489: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3490;
    }
  goto L3714;

 L3490: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3491;
    }
  goto L3714;

 L3491: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, QImode))
    {
      operands[5] = x2;
      goto L3492;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3492: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3493;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3493: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3494;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3494: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3495;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3495: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3496;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3496: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3497;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3497: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3498;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3498: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 328;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3348: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == QImode
      && GET_CODE (x2) == MEM)
    goto L3349;
  x2 = XEXP (x1, 0);
  goto L5963;

 L3349: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6011;
  x2 = XEXP (x1, 0);
  goto L5963;

 L6011: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L3350;
    case MINUS:
      goto L3362;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L5963;

 L3350: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3351;
    }
  x2 = XEXP (x1, 0);
  goto L5963;

 L3351: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (index_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3352;
    }
  x2 = XEXP (x1, 0);
  goto L5963;

 L3352: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3353;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3353: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3354;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3354: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3355;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3355: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3356;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3356: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 317;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3362: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3363;
    }
  x2 = XEXP (x1, 0);
  goto L5963;

 L3363: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3364;
    }
  x2 = XEXP (x1, 0);
  goto L5963;

 L3364: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3365;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3365: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3366;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3366: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3367;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3367: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3368;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3368: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 318;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5963;

 L3502: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == QImode
      && GET_CODE (x2) == MEM)
    goto L3503;
  x2 = XEXP (x1, 0);
  goto L3714;

 L3503: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6013;
  x2 = XEXP (x1, 0);
  goto L3714;

 L6013: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L3504;
    case MINUS:
      goto L3520;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3504: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3505;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3505: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3506;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3506: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3507;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3507: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3508;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3508: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3509;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3509: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3510;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3510: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3511;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3511: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3512;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3512: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3513;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3513: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4]))
    goto L3514;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3514: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[1])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 329;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3520: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3521;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3521: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3522;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3522: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3523;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3523: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3524;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3524: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3525;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3525: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3526;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3526: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3527;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3527: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3528;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3528: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3529;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3529: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3530;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3530: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4])
      && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 330;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L5962: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, HImode))
    {
      operands[3] = x2;
      goto L3446;
    }
 L5965: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, HImode))
    {
      operands[5] = x2;
      goto L3598;
    }
  goto L3714;

 L3446: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == HImode
      && GET_CODE (x2) == MEM)
    goto L3447;
  x2 = XEXP (x1, 0);
  goto L5965;

 L3447: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6015;
  x2 = XEXP (x1, 0);
  goto L5965;

 L6015: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L3448;
    case MINUS:
      goto L3460;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L5965;

 L3448: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3449;
    }
  x2 = XEXP (x1, 0);
  goto L5965;

 L3449: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (index_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3450;
    }
  x2 = XEXP (x1, 0);
  goto L5965;

 L3450: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3451;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3451: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3452;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3452: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3453;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3453: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3454;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3454: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 325;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3460: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3461;
    }
  x2 = XEXP (x1, 0);
  goto L5965;

 L3461: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3462;
    }
  x2 = XEXP (x1, 0);
  goto L5965;

 L3462: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3463;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3463: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3464;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3464: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3465;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3465: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3466;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3466: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2])
      && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM)))
    {
      return 326;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5965;

 L3598: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == HImode
      && GET_CODE (x2) == MEM)
    goto L3599;
  x2 = XEXP (x1, 0);
  goto L3714;

 L3599: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6017;
  x2 = XEXP (x1, 0);
  goto L3714;

 L6017: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case PLUS:
      goto L3600;
    case MINUS:
      goto L3616;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3600: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3601;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3601: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3602;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3602: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3603;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3603: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3604;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3604: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3605;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3605: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3606;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3606: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3607;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3607: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3608;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3608: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3609;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3609: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4]))
    goto L3610;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3610: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[1])
      && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 335;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3616: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3617;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3617: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (shift_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3618;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3618: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3619;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3619: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (const_shift_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3620;
    }
  x2 = XEXP (x1, 0);
  goto L3714;

 L3620: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L3621;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3621: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3622;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3622: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L3623;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3623: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3624;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3624: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[2]))
    {
      operands[2] = x3;
      goto L3625;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3625: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L3626;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3626: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[4])
      && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM))
    {
      return 336;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L3714;

 L3715: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3716;
    }
  goto ret0;

 L3716: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L3717;
  goto ret0;

 L3717: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_CODE (x2) == LABEL_REF)
    goto L3718;
  goto ret0;

 L3718: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[1] = x3;
  goto L3719;

 L3719: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 351;
    }
  goto ret0;

 L2433: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CALL)
    goto L2434;
  x2 = XEXP (x1, 0);
  goto L2470;

 L2434: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L2435;
  x2 = XEXP (x1, 0);
  goto L2470;

 L2435: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  operands[1] = x4;
  goto L2436;

 L2436: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2437;

 L2437: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2438;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2470;

 L2438: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2439;

 L2439: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && (GET_CODE (operands[1]) == SYMBOL_REF)
   && !arm_is_longcall_p (operands[1], INTVAL (operands[3]), 1))
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 253;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L2470;

 L2471: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CALL)
    goto L2472;
  goto ret0;

 L2472: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L2473;
  goto ret0;

 L2473: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  operands[1] = x4;
  goto L2474;

 L2474: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  operands[2] = x3;
  goto L2475;

 L2475: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2476;
  goto ret0;

 L2476: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[3] = x2;
  goto L2477;

 L2477: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && GET_CODE (operands[1]) == SYMBOL_REF
   && !arm_is_longcall_p (operands[1], INTVAL (operands[3]), 1))
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 255;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_7 PARAMS ((rtx, rtx, int *));
static int
recog_7 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  switch (XVECLEN (x0, 0))
    {
    case 2:
      goto L1;
    case 8:
      goto L1922;
    case 5:
      goto L1963;
    case 6:
      goto L1997;
    case 4:
      goto L2027;
    default:
      break;
    }
 L5851: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) >= 5
      && load_multiple_operation (x0, VOIDmode))
    {
      operands[0] = x0;
      goto L1682;
    }
 L5852: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) >= 4
      && load_multiple_operation (x0, VOIDmode))
    {
      operands[0] = x0;
      goto L1710;
    }
 L5853: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) >= 3
      && load_multiple_operation (x0, VOIDmode))
    {
      operands[0] = x0;
      goto L1732;
    }
 L5854: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) >= 2
      && load_multiple_operation (x0, VOIDmode))
    {
      operands[0] = x0;
      goto L1788;
    }
 L5855: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) >= 5
      && store_multiple_operation (x0, VOIDmode))
    {
      operands[0] = x0;
      goto L1799;
    }
 L5856: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) >= 4
      && store_multiple_operation (x0, VOIDmode))
    {
      operands[0] = x0;
      goto L1828;
    }
 L5857: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) >= 3
      && store_multiple_operation (x0, VOIDmode))
    {
      operands[0] = x0;
      goto L1851;
    }
 L5858: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) >= 2
      && store_multiple_operation (x0, VOIDmode))
    {
      operands[0] = x0;
      goto L1910;
    }
 L5863: ATTRIBUTE_UNUSED_LABEL
  switch (XVECLEN (x0, 0))
    {
    case 3:
      goto L2289;
    case 2:
      goto L3628;
    default:
      break;
    }
 L5869: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) >= 1
      && multi_register_push (x0, VOIDmode))
    {
      operands[2] = x0;
      goto L3679;
    }
  goto ret0;

 L1: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L2;
    case CALL:
      goto L2299;
    default:
     break;
   }
  goto L5854;

 L2: ATTRIBUTE_UNUSED_LABEL
  tem = recog_2 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  goto L5854;

 L2299: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2300;
  goto L5854;

 L2300: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L5933;
  goto L5854;

 L5933: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L2301;
    }
 L5934: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L2318;
    }
 L5935: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L2335;
    }
  goto L5854;

 L2301: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  operands[1] = x2;
  goto L2302;

 L2302: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2303;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5934;

 L2303: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[2] = x2;
  goto L2304;

 L2304: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 246;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5934;

 L2318: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  operands[1] = x2;
  goto L2319;

 L2319: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2320;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5935;

 L2320: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[2] = x2;
  goto L2321;

 L2321: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 247;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L5935;

 L2335: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  operands[1] = x2;
  goto L2336;

 L2336: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2337;
  goto L5854;

 L2337: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[2] = x2;
  goto L2338;

 L2338: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 248;
    }
  goto L5854;

 L1922: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1923;
  goto L5851;

 L1923: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1924;
  goto L5851;

 L1924: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1925;
    }
  goto L5851;

 L1925: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1926;
  goto L5851;

 L1926: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L1927;
    }
  goto L5851;

 L1927: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1928;
  goto L5851;

 L1928: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1929;
  goto L5851;

 L1929: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1930;
  goto L5851;

 L1930: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1931;
  goto L5851;

 L1931: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1932;
  goto L5851;

 L1932: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1933;
  goto L5851;

 L1933: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1934;
  goto L5851;

 L1934: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L1935;
  goto L5851;

 L1935: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1936;
  goto L5851;

 L1936: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1937;
  goto L5851;

 L1937: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1938;
  goto L5851;

 L1938: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1939;
  goto L5851;

 L1939: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1940;
  goto L5851;

 L1940: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1941;
  goto L5851;

 L1941: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1942;
  goto L5851;

 L1942: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1943;
  goto L5851;

 L1943: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L1944;
  goto L5851;

 L1944: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1945;
  goto L5851;

 L1945: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L1946;
  goto L5851;

 L1946: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1947;
    }
  goto L5851;

 L1947: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1948;
  goto L5851;

 L1948: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L1949;
  goto L5851;

 L1949: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 12)
    goto L1950;
  goto L5851;

 L1950: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 4);
  if (GET_CODE (x1) == SET)
    goto L1951;
  goto L5851;

 L1951: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1952;
    }
  goto L5851;

 L1952: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1953;
  goto L5851;

 L1953: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[3]))
    goto L1954;
  goto L5851;

 L1954: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 12)
    goto L1955;
  goto L5851;

 L1955: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 5);
  if (GET_CODE (x1) == CLOBBER)
    goto L1956;
  goto L5851;

 L1956: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1957;
    }
  goto L5851;

 L1957: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 6);
  if (GET_CODE (x1) == CLOBBER)
    goto L1958;
  goto L5851;

 L1958: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L1959;
    }
  goto L5851;

 L1959: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 7);
  if (GET_CODE (x1) == CLOBBER)
    goto L1960;
  goto L5851;

 L1960: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[6] = x2;
      goto L1961;
    }
  goto L5851;

 L1961: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 212;
    }
  goto L5851;

 L1963: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1964;
  goto L5851;

 L1964: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1965;
  goto L5851;

 L1965: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1966;
    }
  goto L5851;

 L1966: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1967;
  goto L5851;

 L1967: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L1968;
    }
  goto L5851;

 L1968: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1969;
  goto L5851;

 L1969: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1970;
  goto L5851;

 L1970: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1971;
  goto L5851;

 L1971: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1972;
  goto L5851;

 L1972: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1973;
  goto L5851;

 L1973: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1974;
  goto L5851;

 L1974: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1975;
  goto L5851;

 L1975: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L1976;
  goto L5851;

 L1976: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1977;
  goto L5851;

 L1977: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1978;
  goto L5851;

 L1978: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1979;
  goto L5851;

 L1979: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1980;
  goto L5851;

 L1980: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1981;
  goto L5851;

 L1981: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1982;
  goto L5851;

 L1982: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1983;
  goto L5851;

 L1983: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1984;
  goto L5851;

 L1984: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L1985;
  goto L5851;

 L1985: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1986;
  goto L5851;

 L1986: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L1987;
  goto L5851;

 L1987: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L1988;
    }
  goto L5851;

 L1988: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1989;
  goto L5851;

 L1989: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L1990;
  goto L5851;

 L1990: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 12)
    goto L1991;
  goto L5851;

 L1991: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 4);
  if (GET_CODE (x1) == SET)
    goto L1992;
  goto L5851;

 L1992: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1993;
    }
  goto L5851;

 L1993: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1994;
  goto L5851;

 L1994: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[3]))
    goto L1995;
  goto L5851;

 L1995: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 12
      && (TARGET_THUMB)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 3;
      return 212;
    }
  goto L5851;

 L1997: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1998;
  goto L5851;

 L1998: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1999;
  goto L5851;

 L1999: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2000;
    }
  goto L5851;

 L2000: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2001;
  goto L5851;

 L2001: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2002;
    }
  goto L5851;

 L2002: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L2003;
  goto L5851;

 L2003: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2004;
  goto L5851;

 L2004: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L2005;
  goto L5851;

 L2005: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L2006;
  goto L5851;

 L2006: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L2007;
  goto L5851;

 L2007: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2008;
  goto L5851;

 L2008: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L2009;
  goto L5851;

 L2009: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L2010;
  goto L5851;

 L2010: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L2011;
  goto L5851;

 L2011: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L2012;
  goto L5851;

 L2012: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L2013;
    }
  goto L5851;

 L2013: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L2014;
  goto L5851;

 L2014: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L2015;
  goto L5851;

 L2015: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 8)
    goto L2016;
  goto L5851;

 L2016: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L2017;
  goto L5851;

 L2017: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2018;
    }
  goto L5851;

 L2018: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L2019;
  goto L5851;

 L2019: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[3]))
    goto L2020;
  goto L5851;

 L2020: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 8)
    goto L2021;
  goto L5851;

 L2021: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 4);
  if (GET_CODE (x1) == CLOBBER)
    goto L2022;
  goto L5851;

 L2022: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L2023;
    }
  goto L5851;

 L2023: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 5);
  if (GET_CODE (x1) == CLOBBER)
    goto L2024;
  goto L5851;

 L2024: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L2025;
    }
  goto L5851;

 L2025: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 213;
    }
  goto L5851;

 L2027: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L2028;
  goto L5852;

 L2028: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2029;
  goto L5852;

 L2029: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2030;
    }
  goto L5852;

 L2030: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2031;
  goto L5852;

 L2031: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2032;
    }
  goto L5852;

 L2032: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L2033;
  goto L5852;

 L2033: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2034;
  goto L5852;

 L2034: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L2035;
  goto L5852;

 L2035: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L2036;
  goto L5852;

 L2036: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L2037;
  goto L5852;

 L2037: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2038;
  goto L5852;

 L2038: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L2039;
  goto L5852;

 L2039: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[3]))
    goto L2040;
  goto L5852;

 L2040: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L2041;
  goto L5852;

 L2041: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L2042;
  goto L5852;

 L2042: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L2043;
    }
  goto L5852;

 L2043: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L2044;
  goto L5852;

 L2044: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L2045;
  goto L5852;

 L2045: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 8)
    goto L2046;
  goto L5852;

 L2046: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L2047;
  goto L5852;

 L2047: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2048;
    }
  goto L5852;

 L2048: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L2049;
  goto L5852;

 L2049: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[3]))
    goto L2050;
  goto L5852;

 L2050: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 8
      && (TARGET_THUMB)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 2;
      return 213;
    }
  goto L5852;

 L1682: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1683;
  goto L5852;

 L1683: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1684;
    }
  goto L5852;

 L1684: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1685;
  goto L5852;

 L1685: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1686;
    }
  goto L5852;

 L1686: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 16)
    goto L1687;
  goto L5852;

 L1687: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1688;
  goto L5852;

 L1688: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1689;
    }
  goto L5852;

 L1689: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1690;
  goto L5852;

 L1690: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L1691;
  goto L5852;

 L1691: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1692;
  goto L5852;

 L1692: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1693;
    }
  goto L5852;

 L1693: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1694;
  goto L5852;

 L1694: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1695;
  goto L5852;

 L1695: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1696;
  goto L5852;

 L1696: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1697;
  goto L5852;

 L1697: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L1698;
  goto L5852;

 L1698: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L1699;
    }
  goto L5852;

 L1699: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1700;
  goto L5852;

 L1700: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1701;
  goto L5852;

 L1701: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1702;
  goto L5852;

 L1702: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1703;
  goto L5852;

 L1703: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 4);
  if (GET_CODE (x1) == SET)
    goto L1704;
  goto L5852;

 L1704: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[6] = x2;
      goto L1705;
    }
  goto L5852;

 L1705: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1706;
  goto L5852;

 L1706: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1707;
  goto L5852;

 L1707: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1708;
  goto L5852;

 L1708: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 12
      && (TARGET_ARM && XVECLEN (operands[0], 0) == 5))
    {
      return 200;
    }
  goto L5852;

 L1710: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1711;
  goto L5853;

 L1711: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L5937;
  goto L5853;

 L5937: ATTRIBUTE_UNUSED_LABEL
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1750;
    }
 L5936: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1712;
    }
  goto L5853;

 L1750: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1751;
  x2 = XEXP (x1, 0);
  goto L5936;

 L1751: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1752;
    }
  x2 = XEXP (x1, 0);
  goto L5936;

 L1752: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1753;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1753: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1754;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1754: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1755;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1755: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1756;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1756: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1757;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1757: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1758;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1758: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1759;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1759: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1760;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1760: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1761;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1761: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1762;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1762: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1763;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1763: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1764;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1764: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L1765;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1765: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L1766;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1766: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1767;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1767: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1768;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1768: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1769;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1769: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 12
      && (TARGET_ARM && XVECLEN (operands[0], 0) == 4))
    {
      return 203;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5936;

 L1712: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1713;
  goto L5853;

 L1713: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1714;
    }
  goto L5853;

 L1714: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 12)
    goto L1715;
  goto L5853;

 L1715: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1716;
  goto L5853;

 L1716: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1717;
    }
  goto L5853;

 L1717: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1718;
  goto L5853;

 L1718: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L1719;
  goto L5853;

 L1719: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1720;
  goto L5853;

 L1720: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1721;
    }
  goto L5853;

 L1721: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1722;
  goto L5853;

 L1722: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1723;
  goto L5853;

 L1723: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1724;
  goto L5853;

 L1724: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1725;
  goto L5853;

 L1725: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L1726;
  goto L5853;

 L1726: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L1727;
    }
  goto L5853;

 L1727: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1728;
  goto L5853;

 L1728: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1729;
  goto L5853;

 L1729: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1730;
  goto L5853;

 L1730: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8
      && (TARGET_ARM && XVECLEN (operands[0], 0) == 4))
    {
      return 201;
    }
  goto L5853;

 L1732: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1733;
  goto L5854;

 L1733: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L5939;
  goto L5854;

 L5939: ATTRIBUTE_UNUSED_LABEL
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1773;
    }
 L5938: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1734;
    }
  goto L5854;

 L1773: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1774;
  x2 = XEXP (x1, 0);
  goto L5938;

 L1774: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1775;
    }
  x2 = XEXP (x1, 0);
  goto L5938;

 L1775: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1776;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1776: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1777;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1777: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1778;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1778: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1779;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1779: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1780;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1780: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1781;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1781: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1782;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1782: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1783;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1783: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1784;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1784: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1785;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1785: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1786;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1786: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8
      && (TARGET_ARM && XVECLEN (operands[0], 0) == 3))
    {
      return 204;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  goto L5938;

 L1734: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1735;
  goto L5854;

 L1735: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1736;
    }
  goto L5854;

 L1736: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 8)
    goto L1737;
  goto L5854;

 L1737: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1738;
  goto L5854;

 L1738: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1739;
    }
  goto L5854;

 L1739: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1740;
  goto L5854;

 L1740: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L1741;
  goto L5854;

 L1741: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1742;
  goto L5854;

 L1742: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1743;
    }
  goto L5854;

 L1743: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1744;
  goto L5854;

 L1744: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1745;
  goto L5854;

 L1745: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1746;
  goto L5854;

 L1746: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4
      && (TARGET_ARM && XVECLEN (operands[0], 0) == 3))
    {
      return 202;
    }
  goto L5854;

 L1788: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1789;
  goto L5855;

 L1789: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1790;
    }
  goto L5855;

 L1790: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1791;
  goto L5855;

 L1791: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1792;
    }
  goto L5855;

 L1792: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1793;
  goto L5855;

 L1793: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1794;
    }
  goto L5855;

 L1794: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1795;
  goto L5855;

 L1795: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1796;
  goto L5855;

 L1796: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1797;
  goto L5855;

 L1797: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4
      && (TARGET_ARM && XVECLEN (operands[0], 0) == 2))
    {
      return 205;
    }
  goto L5855;

 L1799: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1800;
  goto L5856;

 L1800: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1801;
    }
  goto L5856;

 L1801: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1802;
  goto L5856;

 L1802: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1803;
    }
  goto L5856;

 L1803: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 16)
    goto L1804;
  goto L5856;

 L1804: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1805;
  goto L5856;

 L1805: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1806;
  goto L5856;

 L1806: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L1807;
  goto L5856;

 L1807: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1808;
    }
  goto L5856;

 L1808: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1809;
  goto L5856;

 L1809: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1810;
  goto L5856;

 L1810: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1811;
  goto L5856;

 L1811: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1812;
  goto L5856;

 L1812: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1813;
  goto L5856;

 L1813: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1814;
    }
  goto L5856;

 L1814: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L1815;
  goto L5856;

 L1815: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1816;
  goto L5856;

 L1816: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1817;
  goto L5856;

 L1817: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1818;
  goto L5856;

 L1818: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1819;
  goto L5856;

 L1819: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L1820;
    }
  goto L5856;

 L1820: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 4);
  if (GET_CODE (x1) == SET)
    goto L1821;
  goto L5856;

 L1821: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1822;
  goto L5856;

 L1822: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1823;
  goto L5856;

 L1823: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1824;
  goto L5856;

 L1824: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 12)
    goto L1825;
  goto L5856;

 L1825: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[6] = x2;
      goto L1826;
    }
  goto L5856;

 L1826: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && XVECLEN (operands[0], 0) == 5))
    {
      return 206;
    }
  goto L5856;

 L1828: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1829;
  goto L5857;

 L1829: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L5941;
  goto L5857;

 L5941: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == MEM)
    goto L1870;
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1830;
    }
  goto L5857;

 L1870: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1871;
    }
  goto L5857;

 L1871: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1872;
    }
  goto L5857;

 L1872: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1873;
  goto L5857;

 L1873: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1874;
  goto L5857;

 L1874: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1875;
  goto L5857;

 L1875: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1876;
  goto L5857;

 L1876: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1877;
  goto L5857;

 L1877: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1878;
    }
  goto L5857;

 L1878: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1879;
  goto L5857;

 L1879: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1880;
  goto L5857;

 L1880: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1881;
  goto L5857;

 L1881: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1882;
  goto L5857;

 L1882: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1883;
  goto L5857;

 L1883: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1884;
    }
  goto L5857;

 L1884: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L1885;
  goto L5857;

 L1885: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1886;
  goto L5857;

 L1886: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1887;
  goto L5857;

 L1887: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1888;
  goto L5857;

 L1888: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 12)
    goto L1889;
  goto L5857;

 L1889: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L1890;
    }
  goto L5857;

 L1890: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && XVECLEN (operands[0], 0) == 4))
    {
      return 209;
    }
  goto L5857;

 L1830: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1831;
  goto L5857;

 L1831: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1832;
    }
  goto L5857;

 L1832: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 12)
    goto L1833;
  goto L5857;

 L1833: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1834;
  goto L5857;

 L1834: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1835;
  goto L5857;

 L1835: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L1836;
  goto L5857;

 L1836: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1837;
    }
  goto L5857;

 L1837: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1838;
  goto L5857;

 L1838: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1839;
  goto L5857;

 L1839: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1840;
  goto L5857;

 L1840: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1841;
  goto L5857;

 L1841: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1842;
  goto L5857;

 L1842: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1843;
    }
  goto L5857;

 L1843: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 3);
  if (GET_CODE (x1) == SET)
    goto L1844;
  goto L5857;

 L1844: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1845;
  goto L5857;

 L1845: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1846;
  goto L5857;

 L1846: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1847;
  goto L5857;

 L1847: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1848;
  goto L5857;

 L1848: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L1849;
    }
  goto L5857;

 L1849: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && XVECLEN (operands[0], 0) == 4))
    {
      return 207;
    }
  goto L5857;

 L1851: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1852;
  goto L5858;

 L1852: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L5943;
  goto L5858;

 L5943: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == MEM)
    goto L1894;
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1853;
    }
  goto L5858;

 L1894: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1895;
    }
  goto L5858;

 L1895: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1896;
    }
  goto L5858;

 L1896: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1897;
  goto L5858;

 L1897: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1898;
  goto L5858;

 L1898: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1899;
  goto L5858;

 L1899: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1900;
  goto L5858;

 L1900: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1901;
  goto L5858;

 L1901: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1902;
    }
  goto L5858;

 L1902: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1903;
  goto L5858;

 L1903: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1904;
  goto L5858;

 L1904: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1905;
  goto L5858;

 L1905: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1906;
  goto L5858;

 L1906: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L1907;
  goto L5858;

 L1907: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1908;
    }
  goto L5858;

 L1908: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && XVECLEN (operands[0], 0) == 3))
    {
      return 210;
    }
  goto L5858;

 L1853: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L1854;
  goto L5858;

 L1854: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1855;
    }
  goto L5858;

 L1855: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 8)
    goto L1856;
  goto L5858;

 L1856: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1857;
  goto L5858;

 L1857: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1858;
  goto L5858;

 L1858: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[2]))
    goto L1859;
  goto L5858;

 L1859: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1860;
    }
  goto L5858;

 L1860: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 2);
  if (GET_CODE (x1) == SET)
    goto L1861;
  goto L5858;

 L1861: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1862;
  goto L5858;

 L1862: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1863;
  goto L5858;

 L1863: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L1864;
  goto L5858;

 L1864: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1865;
  goto L5858;

 L1865: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L1866;
    }
  goto L5858;

 L1866: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && XVECLEN (operands[0], 0) == 3))
    {
      return 208;
    }
  goto L5858;

 L1910: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L1911;
  goto L5863;

 L1911: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1912;
  goto L5863;

 L1912: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1913;
    }
  goto L5863;

 L1913: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1914;
    }
  goto L5863;

 L1914: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == SET)
    goto L1915;
  goto L5863;

 L1915: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L1916;
  goto L5863;

 L1916: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L1917;
  goto L5863;

 L1917: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L1918;
  goto L5863;

 L1918: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 4)
    goto L1919;
  goto L5863;

 L1919: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_hard_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L1920;
    }
  goto L5863;

 L1920: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && XVECLEN (operands[0], 0) == 2))
    {
      return 211;
    }
  goto L5863;

 L2289: ATTRIBUTE_UNUSED_LABEL
  tem = recog_3 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  goto L5869;

 L3628: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_MODE (x1) == SImode
      && GET_CODE (x1) == UNSPEC
      && XVECLEN (x1, 0) == 1
      && XINT (x1, 1) == 6)
    goto L3629;
  switch (GET_CODE (x1))
    {
    case SET:
      goto L2352;
    case CALL:
      goto L2413;
    default:
     break;
   }
  goto L5869;

 L3629: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 14)
    goto L3630;
  goto L5869;

 L3630: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == UNSPEC_VOLATILE
      && XVECLEN (x1, 0) == 1
      && XINT (x1, 1) == 1)
    goto L3631;
  goto L5869;

 L3631: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == RETURN
      && (TARGET_ARM))
    {
      return 337;
    }
  goto L5869;

 L2352: ATTRIBUTE_UNUSED_LABEL
  tem = recog_6 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  goto L5869;

 L2413: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MEM)
    goto L2414;
  goto L5869;

 L2414: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[0] = x3;
  goto L2415;

 L2415: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  operands[1] = x2;
  goto L2416;

 L2416: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == USE)
    goto L2417;
  goto L5869;

 L2417: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[2] = x2;
  goto L2418;

 L2418: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && (GET_CODE (operands[0]) == SYMBOL_REF)
   && !arm_is_longcall_p (operands[0], INTVAL (operands[2]), 1))
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 252;
    }
 L2456: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && GET_CODE (operands[0]) == SYMBOL_REF
   && !arm_is_longcall_p (operands[0], INTVAL (operands[2]), 1))
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 254;
    }
  goto L5869;

 L3679: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L3680;
  goto ret0;

 L3680: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (memory_operand (x2, BLKmode))
    {
      operands[0] = x2;
      goto L3681;
    }
  goto ret0;

 L3681: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == BLKmode
      && GET_CODE (x2) == UNSPEC
      && XVECLEN (x2, 0) == 1
      && XINT (x2, 1) == 2)
    goto L3682;
  goto ret0;

 L3682: ATTRIBUTE_UNUSED_LABEL
  x3 = XVECEXP (x2, 0, 0);
  switch (GET_MODE (x3))
    {
    case SImode:
      goto L6019;
    case XFmode:
      goto L6020;
    default:
      break;
    }
  goto ret0;

 L6019: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3683;
    }
  goto ret0;

 L3683: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 342;
    }
  goto ret0;

 L6020: ATTRIBUTE_UNUSED_LABEL
  if (f_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L3695;
    }
  goto ret0;

 L3695: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 344;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_8 PARAMS ((rtx, rtx, int *));
static int
recog_8 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 1);
  switch (GET_CODE (x1))
    {
    case PLUS:
      goto L25;
    case MINUS:
      goto L261;
    case MULT:
      goto L535;
    case AND:
      goto L757;
    case IOR:
      goto L928;
    case XOR:
      goto L991;
    case NEG:
      goto L1250;
    case NOT:
      goto L1366;
    case ZERO_EXTEND:
      goto L1453;
    case SIGN_EXTEND:
      goto L1463;
    default:
     break;
   }
  goto ret0;

 L25: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode)
    goto L6073;
  goto ret0;

 L6073: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case SIGN_EXTEND:
      goto L41;
    case ZERO_EXTEND:
      goto L57;
    case MULT:
      goto L526;
    case SUBREG:
    case REG:
      goto L6072;
    default:
      goto ret0;
   }
 L6072: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L26;
    }
  goto ret0;

 L41: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L42;
    }
  goto ret0;

 L42: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L43;
    }
  goto ret0;

 L43: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 2;
    }
  goto ret0;

 L57: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L58;
    }
  goto ret0;

 L58: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L59;
    }
  goto ret0;

 L59: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 3;
    }
  goto ret0;

 L526: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode)
    goto L6076;
  goto ret0;

 L6076: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case SIGN_EXTEND:
      goto L527;
    case ZERO_EXTEND:
      goto L553;
    default:
     break;
   }
  goto ret0;

 L527: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L528;
    }
  goto ret0;

 L528: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == SIGN_EXTEND)
    goto L529;
  goto ret0;

 L529: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L530;
    }
  goto ret0;

 L530: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L531;
    }
  goto ret0;

 L531: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_fast_multiply))
    {
      return 50;
    }
  goto ret0;

 L553: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L554;
    }
  goto ret0;

 L554: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == ZERO_EXTEND)
    goto L555;
  goto ret0;

 L555: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L556;
    }
  goto ret0;

 L556: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L557;
    }
  goto ret0;

 L557: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_fast_multiply))
    {
      return 53;
    }
  goto ret0;

 L26: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode)
    goto L6079;
  goto ret0;

 L6079: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == MULT)
    goto L629;
  if (s_register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L27;
    }
  goto ret0;

 L629: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == SIGN_EXTEND)
    goto L630;
  goto ret0;

 L630: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, HImode))
    {
      operands[2] = x4;
      goto L631;
    }
  goto ret0;

 L631: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == SIGN_EXTEND)
    goto L632;
  goto ret0;

 L632: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, HImode))
    {
      operands[3] = x4;
      goto L633;
    }
  goto ret0;

 L633: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_is_xscale))
    {
      return 58;
    }
  goto ret0;

 L27: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 1;
    }
  goto ret0;

 L261: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode)
    goto L6081;
  goto ret0;

 L6081: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case ZERO_EXTEND:
      goto L323;
    case SIGN_EXTEND:
      goto L339;
    case SUBREG:
    case REG:
      goto L6080;
    default:
      goto ret0;
   }
 L6080: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L262;
    }
  goto ret0;

 L323: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6083;
  goto ret0;

 L6083: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L324;
    }
 L6084: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L357;
    }
  goto ret0;

 L324: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L325;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6084;

 L325: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 30;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6084;

 L357: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode
      && GET_CODE (x2) == ZERO_EXTEND)
    goto L358;
  goto ret0;

 L358: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L359;
    }
  goto ret0;

 L359: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 32;
    }
  goto ret0;

 L339: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L340;
    }
  goto ret0;

 L340: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L341;
    }
  goto ret0;

 L341: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 31;
    }
  goto ret0;

 L262: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode)
    goto L6086;
  goto ret0;

 L6086: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case ZERO_EXTEND:
      goto L292;
    case SIGN_EXTEND:
      goto L308;
    case SUBREG:
    case REG:
      goto L6085;
    default:
      goto ret0;
   }
 L6085: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L263;
    }
  goto ret0;

 L292: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L293;
    }
  goto ret0;

 L293: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 28;
    }
  goto ret0;

 L308: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L309;
    }
  goto ret0;

 L309: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 29;
    }
  goto ret0;

 L263: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 26;
    }
  goto ret0;

 L535: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode)
    goto L6088;
  goto ret0;

 L6088: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case SIGN_EXTEND:
      goto L536;
    case ZERO_EXTEND:
      goto L544;
    default:
     break;
   }
  goto ret0;

 L536: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L537;
    }
  goto ret0;

 L537: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode
      && GET_CODE (x2) == SIGN_EXTEND)
    goto L538;
  goto ret0;

 L538: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L539;
    }
  goto ret0;

 L539: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_fast_multiply))
    {
      return 51;
    }
  goto ret0;

 L544: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L545;
    }
  goto ret0;

 L545: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode
      && GET_CODE (x2) == ZERO_EXTEND)
    goto L546;
  goto ret0;

 L546: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L547;
    }
  goto ret0;

 L547: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_fast_multiply))
    {
      return 52;
    }
  goto ret0;

 L757: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode)
    goto L6091;
  goto ret0;

 L6091: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case ZERO_EXTEND:
      goto L764;
    case SIGN_EXTEND:
      goto L771;
    case NOT:
      goto L848;
    case SUBREG:
    case REG:
      goto L6090;
    default:
      goto ret0;
   }
 L6090: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L758;
    }
  goto ret0;

 L764: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L765;
    }
  goto ret0;

 L765: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L766;
    }
  goto ret0;

 L766: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 78;
    }
  goto ret0;

 L771: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L772;
    }
  goto ret0;

 L772: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L773;
    }
  goto ret0;

 L773: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 79;
    }
  goto ret0;

 L848: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode)
    goto L6095;
  goto ret0;

 L6095: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case ZERO_EXTEND:
      goto L856;
    case SIGN_EXTEND:
      goto L864;
    case SUBREG:
    case REG:
      goto L6094;
    default:
      goto ret0;
   }
 L6094: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L849;
    }
  goto ret0;

 L856: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L857;
    }
  goto ret0;

 L857: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L858;
    }
  goto ret0;

 L858: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 87;
    }
  goto ret0;

 L864: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L865;
    }
  goto ret0;

 L865: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L866;
    }
  goto ret0;

 L866: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 88;
    }
  goto ret0;

 L849: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L850;
    }
  goto ret0;

 L850: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 86;
    }
  goto ret0;

 L758: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L759;
    }
  goto ret0;

 L759: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 77;
    }
  goto ret0;

 L928: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode)
    goto L6098;
  goto ret0;

 L6098: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case ZERO_EXTEND:
      goto L935;
    case SIGN_EXTEND:
      goto L942;
    case SUBREG:
    case REG:
      goto L6097;
    default:
      goto ret0;
   }
 L6097: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L929;
    }
  goto ret0;

 L935: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L936;
    }
  goto ret0;

 L936: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L937;
    }
  goto ret0;

 L937: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 95;
    }
  goto ret0;

 L942: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L943;
    }
  goto ret0;

 L943: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L944;
    }
  goto ret0;

 L944: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 96;
    }
  goto ret0;

 L929: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L930;
    }
  goto ret0;

 L930: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 94;
    }
  goto ret0;

 L991: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode)
    goto L6101;
  goto ret0;

 L6101: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case ZERO_EXTEND:
      goto L998;
    case SIGN_EXTEND:
      goto L1005;
    case SUBREG:
    case REG:
      goto L6100;
    default:
      goto ret0;
   }
 L6100: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L992;
    }
  goto ret0;

 L998: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L999;
    }
  goto ret0;

 L999: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L1000;
    }
  goto ret0;

 L1000: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 102;
    }
  goto ret0;

 L1005: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1006;
    }
  goto ret0;

 L1006: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L1007;
    }
  goto ret0;

 L1007: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 103;
    }
  goto ret0;

 L992: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L993;
    }
  goto ret0;

 L993: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 101;
    }
  goto ret0;

 L1250: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L1251;
    }
  goto ret0;

 L1251: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 125;
    }
  goto ret0;

 L1366: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L1367;
    }
  goto ret0;

 L1367: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 143;
    }
  goto ret0;

 L1453: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case SImode:
      goto L6103;
    case QImode:
      goto L6104;
    default:
      break;
    }
  goto ret0;

 L6103: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1454;
    }
  goto ret0;

 L1454: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 157;
    }
  goto ret0;

 L6104: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x2, QImode))
    {
      operands[1] = x2;
      goto L1459;
    }
  goto ret0;

 L1459: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 158;
    }
  goto ret0;

 L1463: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1464;
    }
  goto ret0;

 L1464: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 159;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_9 PARAMS ((rtx, rtx, int *));
static int
recog_9 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 1);
  switch (GET_CODE (x1))
    {
    case MINUS:
      goto L388;
    case MULT:
      goto L435;
    case PLUS:
      goto L478;
    case TRUNCATE:
      goto L575;
    case AND:
      goto L777;
    case NE:
      goto L839;
    case IOR:
      goto L948;
    case XOR:
      goto L1011;
    case SMAX:
      goto L1060;
    case SMIN:
      goto L1074;
    case UMAX:
      goto L1088;
    case UMIN:
      goto L1102;
    default:
     break;
   }
 L6129: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x1, SImode))
    {
      operands[4] = x1;
      goto L1132;
    }
  goto ret0;

 L388: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L389;
    }
 L369: ATTRIBUTE_UNUSED_LABEL
  if (reg_or_int_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L370;
    }
  goto L6129;

 L389: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_comparison_operator (x2, SImode))
    {
      operands[2] = x2;
      goto L390;
    }
  x2 = XEXP (x1, 0);
  goto L369;

 L390: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[3] = x3;
      goto L391;
    }
  x2 = XEXP (x1, 0);
  goto L369;

 L391: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0
      && (TARGET_ARM))
    {
      return 36;
    }
  x2 = XEXP (x1, 0);
  goto L369;

 L370: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L371;
    }
  goto L6129;

 L371: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 34;
    }
  x1 = XEXP (x0, 1);
  goto L6129;

 L435: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6131;
  goto ret0;

 L6131: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == SIGN_EXTEND)
    goto L610;
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L436;
    }
  goto ret0;

 L610: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, HImode))
    {
      operands[1] = x3;
      goto L611;
    }
  goto ret0;

 L611: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == SIGN_EXTEND)
    goto L612;
  goto ret0;

 L612: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, HImode))
    {
      operands[2] = x3;
      goto L613;
    }
  goto ret0;

 L613: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_is_xscale))
    {
      return 56;
    }
  goto ret0;

 L436: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L437;
    }
  goto ret0;

 L437: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 43;
    }
  goto ret0;

 L478: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6132;
  goto L6129;

 L6132: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == MULT)
    goto L479;
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L618;
    }
  goto L6129;

 L479: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L480;
    }
  goto L6129;

 L480: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L481;
    }
  goto L6129;

 L481: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L482;
    }
  goto L6129;

 L482: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 47;
    }
  x1 = XEXP (x0, 1);
  goto L6129;

 L618: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MULT)
    goto L619;
  goto L6129;

 L619: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == SIGN_EXTEND)
    goto L620;
  goto L6129;

 L620: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, HImode))
    {
      operands[2] = x4;
      goto L621;
    }
  goto L6129;

 L621: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == SIGN_EXTEND)
    goto L622;
  goto L6129;

 L622: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, HImode))
    {
      operands[3] = x4;
      goto L623;
    }
  goto L6129;

 L623: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_is_xscale))
    {
      return 57;
    }
  x1 = XEXP (x0, 1);
  goto L6129;

 L575: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode
      && GET_CODE (x2) == LSHIFTRT)
    goto L576;
  goto ret0;

 L576: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == MULT)
    goto L577;
  goto ret0;

 L577: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == DImode)
    goto L6134;
  goto ret0;

 L6134: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case SIGN_EXTEND:
      goto L578;
    case ZERO_EXTEND:
      goto L602;
    default:
     break;
   }
  goto ret0;

 L578: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L579;
    }
  goto ret0;

 L579: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_MODE (x4) == DImode
      && GET_CODE (x4) == SIGN_EXTEND)
    goto L580;
  goto ret0;

 L580: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L581;
    }
  goto ret0;

 L581: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 32
      && (TARGET_ARM && arm_fast_multiply)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 54;
    }
  goto ret0;

 L602: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L603;
    }
  goto ret0;

 L603: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_MODE (x4) == DImode
      && GET_CODE (x4) == ZERO_EXTEND)
    goto L604;
  goto ret0;

 L604: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L605;
    }
  goto ret0;

 L605: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 32
      && (TARGET_ARM && arm_fast_multiply)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 55;
    }
  goto ret0;

 L777: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6137;
  goto L6129;

 L6137: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case NOT:
      goto L871;
    case IOR:
      goto L1044;
    case SUBREG:
    case REG:
      goto L6136;
    default:
      goto L6129;
   }
 L6136: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L778;
    }
  goto L6129;

 L871: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6139;
  goto L6129;

 L6139: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L872;
    }
  if (shift_operator (x3, SImode))
    {
      operands[4] = x3;
      goto L886;
    }
  goto L6129;

 L872: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L873;
    }
  goto L6129;

 L873: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 89;
    }
  x1 = XEXP (x0, 1);
  goto L6129;

 L886: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L887;
    }
  goto L6129;

 L887: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L888;
    }
  goto L6129;

 L888: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L889;
    }
  goto L6129;

 L889: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 91;
    }
  x1 = XEXP (x0, 1);
  goto L6129;

 L1044: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1045;
    }
  goto L6129;

 L1045: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1046;
    }
  goto L6129;

 L1046: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L1047;
  goto L6129;

 L1047: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L1048;
    }
  goto L6129;

 L1048: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 108;
    }
  x1 = XEXP (x0, 1);
  goto L6129;

 L778: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (reg_or_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L779;
    }
  goto L6129;

 L779: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 80;
    }
  x1 = XEXP (x0, 1);
  goto L6129;

 L839: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == ZERO_EXTRACT)
    goto L840;
  goto ret0;

 L840: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L841;
    }
  goto ret0;

 L841: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (const_int_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L842;
    }
  goto ret0;

 L842: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (const_int_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L843;
    }
  goto ret0;

 L843: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM
   && (INTVAL (operands[3]) >= 0 && INTVAL (operands[3]) < 32
       && INTVAL (operands[2]) > 0 
       && INTVAL (operands[2]) + (INTVAL (operands[3]) & 1) <= 8
       && INTVAL (operands[2]) + INTVAL (operands[3]) <= 32))
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 85;
    }
  goto ret0;

 L948: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L949;
    }
  goto L6129;

 L949: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (reg_or_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L950;
    }
  goto L6129;

 L950: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 97;
    }
  x1 = XEXP (x0, 1);
  goto L6129;

 L1011: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1012;
    }
  goto L6129;

 L1012: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1013;
    }
  goto L6129;

 L1013: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 104;
    }
  x1 = XEXP (x0, 1);
  goto L6129;

 L1060: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1061;
    }
  goto ret0;

 L1061: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1062;
    }
  goto ret0;

 L1062: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 109;
    }
  goto ret0;

 L1074: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1075;
    }
  goto ret0;

 L1075: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1076;
    }
  goto ret0;

 L1076: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 110;
    }
  goto ret0;

 L1088: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1089;
    }
  goto ret0;

 L1089: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1090;
    }
  goto ret0;

 L1090: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 111;
    }
  goto ret0;

 L1102: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1103;
    }
  goto ret0;

 L1103: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1104;
    }
  goto ret0;

 L1104: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 112;
    }
  goto ret0;

 L1132: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (minmax_operator (x2, SImode))
    {
      operands[5] = x2;
      goto L1133;
    }
  goto ret0;

 L1133: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1134;
    }
  goto ret0;

 L1134: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L1135;
    }
  goto ret0;

 L1135: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1136;
    }
  goto ret0;

 L1136: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && (GET_CODE (operands[1]) != REG
       || (REGNO(operands[1]) != FRAME_POINTER_REGNUM
           && REGNO(operands[1]) != ARG_POINTER_REGNUM)))
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 114;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_10 PARAMS ((rtx, rtx, int *));
static int
recog_10 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 0);
  if (s_register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L62;
    }
 L6024: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L68;
    }
 L6032: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L368;
    }
 L6033: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L440;
    }
 L6034: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L1115;
    }
 L6035: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L1163;
    }
 L6036: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L1271;
    }
 L6037: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L1472;
    }
 L6040: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L1520;
    }
 L6043: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L1548;
    }
  goto ret0;

 L62: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode
      && GET_CODE (x1) == PLUS)
    goto L63;
  x1 = XEXP (x0, 0);
  goto L6024;

 L63: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6108;
  x1 = XEXP (x0, 0);
  goto L6024;

 L6108: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case LTU:
      goto L155;
    case PLUS:
      goto L177;
    case SUBREG:
    case REG:
      goto L6107;
    default:
      goto L6110;
   }
 L6107: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L64;
    }
 L6110: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, SImode))
    {
      operands[2] = x2;
      goto L206;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L155: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == CC_Cmode
      && GET_CODE (x3) == REG
      && XINT (x3, 0) == 24)
    goto L156;
  goto L6110;

 L156: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L157;
  goto L6110;

 L157: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L158;
  x2 = XEXP (x1, 0);
  goto L6110;

 L158: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6111;
  x2 = XEXP (x1, 0);
  goto L6110;

 L6111: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L159;
    }
  if (shift_operator (x3, SImode))
    {
      operands[2] = x3;
      goto L169;
    }
  x2 = XEXP (x1, 0);
  goto L6110;

 L159: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L160;
    }
  x2 = XEXP (x1, 0);
  goto L6110;

 L160: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 14;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6110;

 L169: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L170;
    }
  x2 = XEXP (x1, 0);
  goto L6110;

 L170: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (reg_or_int_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L171;
    }
  x2 = XEXP (x1, 0);
  goto L6110;

 L171: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L172;
    }
  x2 = XEXP (x1, 0);
  goto L6110;

 L172: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 15;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6110;

 L177: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6114;
  x1 = XEXP (x0, 0);
  goto L6024;

 L6114: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == LTU)
    goto L187;
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L178;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L187: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == CC_Cmode
      && GET_CODE (x4) == REG
      && XINT (x4, 0) == 24)
    goto L188;
  x1 = XEXP (x0, 0);
  goto L6024;

 L188: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 0)
    goto L189;
  x1 = XEXP (x0, 0);
  goto L6024;

 L189: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L190;
    }
 L199: ATTRIBUTE_UNUSED_LABEL
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L200;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L190: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L191;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 1);
  goto L199;

 L191: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 17;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 1);
  goto L199;

 L200: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L201;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L201: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 18;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L178: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L179;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L179: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == LTU)
    goto L180;
  x1 = XEXP (x0, 0);
  goto L6024;

 L180: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == CC_Cmode
      && GET_CODE (x3) == REG
      && XINT (x3, 0) == 24)
    goto L181;
  x1 = XEXP (x0, 0);
  goto L6024;

 L181: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0
      && (TARGET_ARM))
    {
      return 16;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L64: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (reg_or_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L65;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L65: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 4;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L206: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, CCmode))
    {
      operands[3] = x3;
      goto L207;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L207: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L208;
  x1 = XEXP (x0, 0);
  goto L6024;

 L208: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L209;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L209: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 19;
    }
  x1 = XEXP (x0, 0);
  goto L6024;

 L68: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6115;
  x1 = XEXP (x0, 0);
  goto L6032;

 L6115: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case PLUS:
      goto L69;
    case MINUS:
      goto L363;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L6032;

 L69: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L70;
    }
  x1 = XEXP (x0, 0);
  goto L6032;

 L70: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (nonmemory_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L71;
    }
  x1 = XEXP (x0, 0);
  goto L6032;

 L71: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 5;
    }
  x1 = XEXP (x0, 0);
  goto L6032;

 L363: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L364;
    }
  x1 = XEXP (x0, 0);
  goto L6032;

 L364: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L365;
    }
  x1 = XEXP (x0, 0);
  goto L6032;

 L365: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 33;
    }
  x1 = XEXP (x0, 0);
  goto L6032;

 L368: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6117;
  x1 = XEXP (x0, 0);
  goto L6033;

 L6117: ATTRIBUTE_UNUSED_LABEL
  tem = recog_9 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  x1 = XEXP (x0, 0);
  goto L6033;

 L440: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6141;
  x1 = XEXP (x0, 0);
  goto L6034;

 L6141: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case MULT:
      goto L441;
    case AND:
      goto L783;
    case IOR:
      goto L954;
    case XOR:
      goto L1017;
    case ASHIFT:
      goto L1140;
    case ASHIFTRT:
      goto L1146;
    case LSHIFTRT:
      goto L1152;
    case ROTATERT:
      goto L1158;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L6034;

 L441: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L442;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L442: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L443;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L443: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 44;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L783: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6150;
  x1 = XEXP (x0, 0);
  goto L6034;

 L6150: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == NOT)
    goto L878;
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L784;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L878: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L879;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L879: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L880;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L880: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 90;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L784: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L785;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L785: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 81;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L954: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L955;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L955: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L956;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L956: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 98;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1017: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1018;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1018: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1019;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1019: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 105;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1140: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1141;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1141: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (nonmemory_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1142;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1142: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 115;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1146: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1147;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1147: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (nonmemory_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1148;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1148: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 116;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1152: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1153;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1153: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (nonmemory_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1154;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1154: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 117;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1158: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1159;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1159: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1160;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1160: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 118;
    }
  x1 = XEXP (x0, 0);
  goto L6034;

 L1115: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (minmax_operator (x1, SImode))
    {
      operands[3] = x1;
      goto L1116;
    }
  x1 = XEXP (x0, 0);
  goto L6035;

 L1116: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1117;
    }
  x1 = XEXP (x0, 0);
  goto L6035;

 L1117: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1118;
    }
  x1 = XEXP (x0, 0);
  goto L6035;

 L1118: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 113;
    }
  x1 = XEXP (x0, 0);
  goto L6035;

 L1163: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6152;
  x1 = XEXP (x0, 0);
  goto L6036;

 L6152: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case NOT:
      goto L1201;
    case NEG:
      goto L1267;
    case ABS:
      goto L1305;
    case FIX:
      goto L1423;
    case ASHIFT:
    case ASHIFTRT:
    case LSHIFTRT:
    case ROTATERT:
    case MULT:
      goto L6151;
    default:
      x1 = XEXP (x0, 0);
      goto L6036;
   }
 L6151: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x1, SImode))
    {
      operands[3] = x1;
      goto L1164;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1201: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6156;
  x1 = XEXP (x0, 0);
  goto L6036;

 L6156: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x2, SImode))
    {
      operands[3] = x2;
      goto L1202;
    }
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1372;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1202: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1203;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1203: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1204;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1204: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 122;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1372: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 144;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1267: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6159;
  x1 = XEXP (x0, 0);
  goto L6036;

 L6159: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == ABS)
    goto L1319;
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1268;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1319: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1320;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1320: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 134;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1268: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 127;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1305: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1306;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1306: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 133;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1423: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case SFmode:
      goto L6160;
    case DFmode:
      goto L6161;
    case XFmode:
      goto L6162;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L6160: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L1424;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1424: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 151;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L6161: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L1429;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1429: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 152;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L6162: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L1434;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1434: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 153;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1164: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1165;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1165: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (reg_or_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L1166;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1166: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 119;
    }
  x1 = XEXP (x0, 0);
  goto L6036;

 L1271: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6163;
  x1 = XEXP (x0, 0);
  goto L6037;

 L6163: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case NEG:
      goto L1272;
    case NOT:
      goto L1376;
    case ZERO_EXTEND:
      goto L1468;
    case SIGN_EXTEND:
      goto L1501;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1272: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1273;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1273: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 128;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1376: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1377;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1377: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 145;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1468: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case HImode:
      goto L6167;
    case QImode:
      goto L6168;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L6167: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x2, HImode))
    {
      operands[1] = x2;
      goto L1469;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1469: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 160;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L6168: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x2, QImode))
    {
      operands[1] = x2;
      goto L1479;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1479: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 162;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1501: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (memory_operand (x2, HImode))
    {
      operands[1] = x2;
      goto L1502;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1502: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 165;
    }
  x1 = XEXP (x0, 0);
  goto L6037;

 L1472: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6169;
  x1 = XEXP (x0, 0);
  goto L6040;

 L6169: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case ZERO_EXTEND:
      goto L1473;
    case SIGN_EXTEND:
      goto L1506;
    case UNSPEC:
      goto L6174;
    case ROTATE:
      goto L1614;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1473: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case HImode:
      goto L6176;
    case QImode:
      goto L6177;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L6176: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x2, HImode))
    {
      operands[1] = x2;
      goto L1474;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1474: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_arch4))
    {
      return 161;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L6177: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x2, QImode))
    {
      operands[1] = x2;
      goto L1484;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1484: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 163;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1506: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case HImode:
      goto L6178;
    case QImode:
      goto L6179;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L6178: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x2, HImode))
    {
      operands[1] = x2;
      goto L1507;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1507: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_arch4))
    {
      return 166;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L6179: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x2, QImode))
    {
      operands[1] = x2;
      goto L1517;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1517: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_arch4))
    {
      return 168;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L6174: ATTRIBUTE_UNUSED_LABEL
  switch (XVECLEN (x1, 0))
    {
    case 1:
      goto L6180;
    case 2:
      goto L6181;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L6180: ATTRIBUTE_UNUSED_LABEL
  if (XINT (x1, 1) == 3)
    goto L1557;
  x1 = XEXP (x0, 0);
  goto L6040;

 L1557: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  operands[1] = x2;
  goto L1558;

 L1558: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && flag_pic))
    {
      return 177;
    }
 L1563: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB && flag_pic))
    {
      return 178;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L6181: ATTRIBUTE_UNUSED_LABEL
  if (XINT (x1, 1) == 3)
    goto L1567;
  x1 = XEXP (x0, 0);
  goto L6040;

 L1567: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  operands[1] = x2;
  goto L1568;

 L1568: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (s_register_operand (x2, VOIDmode))
    {
      operands[2] = x2;
      goto L1569;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1569: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_EITHER && flag_pic && operands[2] == pic_offset_table_rtx))
    {
      return 179;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1614: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6182;
  x1 = XEXP (x0, 0);
  goto L6040;

 L6182: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case MEM:
      goto L6184;
    case SUBREG:
      goto L6185;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L6040;

 L6184: ATTRIBUTE_UNUSED_LABEL
  if (offsettable_memory_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1615;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1615: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 16
      && (TARGET_ARM && (!TARGET_MMU_TRAPS)))
    {
      return 184;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L6185: ATTRIBUTE_UNUSED_LABEL
  if (XINT (x2, 1) == 0)
    goto L1632;
  x1 = XEXP (x0, 0);
  goto L6040;

 L1632: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (memory_operand (x3, HImode))
    {
      operands[1] = x3;
      goto L1633;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1633: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 16
      && (TARGET_ARM
   && BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS))
    {
      return 188;
    }
  x1 = XEXP (x0, 0);
  goto L6040;

 L1520: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode
      && GET_CODE (x1) == SIGN_EXTEND)
    goto L1521;
  x1 = XEXP (x0, 0);
  goto L6043;

 L1521: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (memory_operand (x2, QImode))
    {
      operands[1] = x2;
      goto L1522;
    }
  x1 = XEXP (x0, 0);
  goto L6043;

 L1522: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 169;
    }
  x1 = XEXP (x0, 0);
  goto L6043;

 L1548: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (general_operand (x1, SImode))
    {
      operands[1] = x1;
      goto L1549;
    }
  goto ret0;

 L1549: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && (   register_operand (operands[0], SImode)
       || register_operand (operands[1], SImode))))
    {
      return 175;
    }
 L1553: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && (   register_operand (operands[0], SImode) 
       || register_operand (operands[1], SImode))))
    {
      return 176;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_11 PARAMS ((rtx, rtx, int *));
static int
recog_11 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 1);
  switch (GET_CODE (x1))
    {
    case PLUS:
      goto L219;
    case MINUS:
      goto L401;
    case MULT:
      goto L643;
    case DIV:
      goto L683;
    case MOD:
      goto L723;
    case NEG:
      goto L1282;
    case ABS:
      goto L1329;
    case SQRT:
      goto L1350;
    case FLOAT:
      goto L1413;
    case FLOAT_TRUNCATE:
      goto L1448;
    case FLOAT_EXTEND:
      goto L1526;
    default:
     break;
   }
  goto ret0;

 L219: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DFmode)
    goto L6226;
  goto ret0;

 L6226: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L226;
  if (s_register_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L220;
    }
  goto ret0;

 L226: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L227;
    }
  goto ret0;

 L227: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6228;
  goto ret0;

 L6228: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L242;
  if (fpu_add_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L228;
    }
  goto ret0;

 L242: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L243;
    }
  goto ret0;

 L243: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 24;
    }
  goto ret0;

 L228: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 22;
    }
  goto ret0;

 L220: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6230;
  goto ret0;

 L6230: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L234;
  if (fpu_add_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L221;
    }
  goto ret0;

 L234: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L235;
    }
  goto ret0;

 L235: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 23;
    }
  goto ret0;

 L221: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 21;
    }
  goto ret0;

 L401: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DFmode)
    goto L6232;
  goto ret0;

 L6232: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L408;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L402;
    }
  goto ret0;

 L408: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L409;
    }
  goto ret0;

 L409: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6234;
  goto ret0;

 L6234: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L424;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L410;
    }
  goto ret0;

 L424: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L425;
    }
  goto ret0;

 L425: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 41;
    }
  goto ret0;

 L410: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 39;
    }
  goto ret0;

 L402: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6236;
  goto ret0;

 L6236: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L416;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L403;
    }
  goto ret0;

 L416: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L417;
    }
  goto ret0;

 L417: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 40;
    }
  goto ret0;

 L403: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 38;
    }
  goto ret0;

 L643: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DFmode)
    goto L6238;
  goto ret0;

 L6238: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L650;
  if (s_register_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L644;
    }
  goto ret0;

 L650: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L651;
    }
  goto ret0;

 L651: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6240;
  goto ret0;

 L6240: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L666;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L652;
    }
  goto ret0;

 L666: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L667;
    }
  goto ret0;

 L667: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 63;
    }
  goto ret0;

 L652: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 61;
    }
  goto ret0;

 L644: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6242;
  goto ret0;

 L6242: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L658;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L645;
    }
  goto ret0;

 L658: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L659;
    }
  goto ret0;

 L659: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 62;
    }
  goto ret0;

 L645: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 60;
    }
  goto ret0;

 L683: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DFmode)
    goto L6244;
  goto ret0;

 L6244: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L690;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L684;
    }
  goto ret0;

 L690: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L691;
    }
  goto ret0;

 L691: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6246;
  goto ret0;

 L6246: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L706;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L692;
    }
  goto ret0;

 L706: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L707;
    }
  goto ret0;

 L707: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 69;
    }
  goto ret0;

 L692: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 67;
    }
  goto ret0;

 L684: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6248;
  goto ret0;

 L6248: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L698;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L685;
    }
  goto ret0;

 L698: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L699;
    }
  goto ret0;

 L699: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 68;
    }
  goto ret0;

 L685: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 66;
    }
  goto ret0;

 L723: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DFmode)
    goto L6250;
  goto ret0;

 L6250: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L730;
  if (s_register_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L724;
    }
  goto ret0;

 L730: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L731;
    }
  goto ret0;

 L731: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6252;
  goto ret0;

 L6252: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L746;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L732;
    }
  goto ret0;

 L746: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L747;
    }
  goto ret0;

 L747: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 75;
    }
  goto ret0;

 L732: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 73;
    }
  goto ret0;

 L724: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6254;
  goto ret0;

 L6254: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L738;
  if (fpu_rhs_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L725;
    }
  goto ret0;

 L738: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L739;
    }
  goto ret0;

 L739: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 74;
    }
  goto ret0;

 L725: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 72;
    }
  goto ret0;

 L1282: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DFmode)
    goto L6256;
  goto ret0;

 L6256: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L1288;
  if (s_register_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L1283;
    }
  goto ret0;

 L1288: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L1289;
    }
  goto ret0;

 L1289: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 131;
    }
  goto ret0;

 L1283: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 130;
    }
  goto ret0;

 L1329: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DFmode)
    goto L6258;
  goto ret0;

 L6258: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L1335;
  if (s_register_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L1330;
    }
  goto ret0;

 L1335: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L1336;
    }
  goto ret0;

 L1336: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 137;
    }
  goto ret0;

 L1330: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 136;
    }
  goto ret0;

 L1350: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DFmode)
    goto L6260;
  goto ret0;

 L6260: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L1356;
  if (s_register_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L1351;
    }
  goto ret0;

 L1356: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L1357;
    }
  goto ret0;

 L1357: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 141;
    }
  goto ret0;

 L1351: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 140;
    }
  goto ret0;

 L1413: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1414;
    }
  goto ret0;

 L1414: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 149;
    }
  goto ret0;

 L1448: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L1449;
    }
  goto ret0;

 L1449: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 156;
    }
  goto ret0;

 L1526: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L1527;
    }
  goto ret0;

 L1527: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 170;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_12 PARAMS ((rtx, rtx, int *));
static int
recog_12 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[3] = x2;
      goto L2246;
    }
 L2684: ATTRIBUTE_UNUSED_LABEL
  if (equality_operator (x2, VOIDmode))
    {
      operands[3] = x2;
      goto L2685;
    }
 L2804: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[5] = x2;
      goto L2805;
    }
 L2826: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[6] = x2;
      goto L2827;
    }
 L2837: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[4] = x2;
      goto L2838;
    }
 L2898: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[9] = x2;
      goto L2899;
    }
 L2911: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[5] = x2;
      goto L3007;
    }
 L2937: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[6] = x2;
      goto L2938;
    }
 L2948: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[4] = x2;
      goto L2949;
    }
 L3082: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[5] = x2;
      goto L3083;
    }
 L3143: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[7] = x2;
      goto L3144;
    }
 L3156: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[5] = x2;
      goto L3258;
    }
 L3267: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[4] = x2;
      goto L3268;
    }
  goto ret0;

 L2246: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[4] = x3;
      goto L2247;
    }
  goto L2684;

 L2247: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2248;
  goto L2684;

 L2248: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2249;
    }
  x2 = XEXP (x1, 0);
  goto L2684;

 L2249: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_not_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L2250;
    }
  x2 = XEXP (x1, 0);
  goto L2684;

 L2250: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 240;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2684;

 L2685: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[4] = x3;
      goto L2686;
    }
  goto L2804;

 L2686: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (cc_register (x4, VOIDmode))
    {
      operands[5] = x4;
      goto L2687;
    }
  goto L2804;

 L2687: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 0)
    goto L2688;
  goto L2804;

 L2688: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2689;
  goto L2804;

 L2689: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2690;
    }
  x2 = XEXP (x1, 0);
  goto L2804;

 L2690: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L2691;
    }
  x2 = XEXP (x1, 0);
  goto L2804;

 L2691: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 277;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2804;

 L2805: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2806;
    }
  goto L2826;

 L2806: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L2807;
    }
  goto L2826;

 L2807: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2808;
    }
  x2 = XEXP (x1, 0);
  goto L2826;

 L2808: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L2809;
    }
  x2 = XEXP (x1, 0);
  goto L2826;

 L2809: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 285;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2826;

 L2827: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L2828;
    }
  goto L2837;

 L2828: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L2829;
    }
  goto L2837;

 L2829: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L2830;
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2865;
    }
  x2 = XEXP (x1, 0);
  goto L2837;

 L2830: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2831;
    }
  x2 = XEXP (x1, 0);
  goto L2837;

 L2831: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2832;
    }
  x2 = XEXP (x1, 0);
  goto L2837;

 L2832: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2833;
    }
  x2 = XEXP (x1, 0);
  goto L2837;

 L2833: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 286;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2837;

 L2865: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L2866;
  x2 = XEXP (x1, 0);
  goto L2837;

 L2866: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2867;
    }
  x2 = XEXP (x1, 0);
  goto L2837;

 L2867: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2868;
    }
  x2 = XEXP (x1, 0);
  goto L2837;

 L2868: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 288;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2837;

 L2838: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[5] = x3;
      goto L2839;
    }
  goto L2898;

 L2839: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2840;
  goto L2898;

 L2840: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L2841;
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2876;
    }
  x2 = XEXP (x1, 0);
  goto L2898;

 L2841: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2842;
    }
  x2 = XEXP (x1, 0);
  goto L2898;

 L2842: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2843;
    }
  x2 = XEXP (x1, 0);
  goto L2898;

 L2843: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2844;
    }
  x2 = XEXP (x1, 0);
  goto L2898;

 L2844: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 287;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2898;

 L2876: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L2877;
  x2 = XEXP (x1, 0);
  goto L2898;

 L2877: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2878;
    }
  x2 = XEXP (x1, 0);
  goto L2898;

 L2878: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2879;
    }
  x2 = XEXP (x1, 0);
  goto L2898;

 L2879: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 289;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2898;

 L2899: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L2900;
    }
  goto L2911;

 L2900: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[6] = x3;
      goto L2901;
    }
  goto L2911;

 L2901: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (shiftable_operator (x2, SImode))
    {
      operands[8] = x2;
      goto L2902;
    }
  x2 = XEXP (x1, 0);
  goto L2911;

 L2902: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2903;
    }
  x2 = XEXP (x1, 0);
  goto L2911;

 L2903: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2904;
    }
  x2 = XEXP (x1, 0);
  goto L2911;

 L2904: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shiftable_operator (x2, SImode))
    {
      operands[7] = x2;
      goto L2905;
    }
  x2 = XEXP (x1, 0);
  goto L2911;

 L2905: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2906;
    }
  x2 = XEXP (x1, 0);
  goto L2911;

 L2906: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L2907;
    }
  x2 = XEXP (x1, 0);
  goto L2911;

 L2907: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 290;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2911;

 L3007: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3008;
    }
 L2912: ATTRIBUTE_UNUSED_LABEL
  if (cc_register (x3, VOIDmode))
    {
      operands[8] = x3;
      goto L2913;
    }
  goto L2937;

 L3008: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L3041;
    }
  x3 = XEXP (x2, 0);
  goto L2912;

 L3041: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L3042;
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3010;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2912;

 L3042: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3043;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2912;

 L3043: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3044;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2912;

 L3044: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 298;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2912;

 L3010: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L3011;
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2912;

 L3011: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3012;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2912;

 L3012: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 296;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2912;

 L2913: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2914;
  goto L2937;

 L2914: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (shiftable_operator (x2, SImode))
    {
      operands[6] = x2;
      goto L2915;
    }
  x2 = XEXP (x1, 0);
  goto L2937;

 L2915: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2916;
    }
  x2 = XEXP (x1, 0);
  goto L2937;

 L2916: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2917;
    }
  x2 = XEXP (x1, 0);
  goto L2937;

 L2917: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shiftable_operator (x2, SImode))
    {
      operands[7] = x2;
      goto L2918;
    }
  x2 = XEXP (x1, 0);
  goto L2937;

 L2918: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2919;
    }
  x2 = XEXP (x1, 0);
  goto L2937;

 L2919: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L2920;
    }
  x2 = XEXP (x1, 0);
  goto L2937;

 L2920: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 291;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2937;

 L2938: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6302;
  goto L2948;

 L6302: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2939;
    }
 L6303: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L2974;
    }
  goto L2948;

 L2939: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2940;
    }
  x3 = XEXP (x2, 0);
  goto L6303;

 L2940: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (shiftable_operator (x2, SImode))
    {
      operands[7] = x2;
      goto L2941;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6303;

 L2941: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L2942;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6303;

 L2942: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L2943;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6303;

 L2943: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2944;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6303;

 L2944: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 292;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6303;

 L2974: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L3074;
    }
  goto L2948;

 L3074: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L6305;
 L2975: ATTRIBUTE_UNUSED_LABEL
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2976;
    }
 L3109: ATTRIBUTE_UNUSED_LABEL
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3110;
    }
  x2 = XEXP (x1, 0);
  goto L2948;

 L6305: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == NOT)
    goto L3187;
  if (shift_operator (x2, SImode))
    {
      operands[7] = x2;
      goto L3075;
    }
  if (shiftable_operator (x2, SImode))
    {
      operands[7] = x2;
      goto L3225;
    }
  goto L2975;

 L3187: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3188;
    }
  goto L2975;

 L3188: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shiftable_operator (x2, SImode))
    {
      operands[7] = x2;
      goto L3189;
    }
  x2 = XEXP (x1, 1);
  goto L2975;

 L3189: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3190;
    }
  x2 = XEXP (x1, 1);
  goto L2975;

 L3190: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3191;
    }
  x2 = XEXP (x1, 1);
  goto L2975;

 L3191: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 306;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L2975;

 L3075: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3076;
    }
  goto L2975;

 L3076: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3077;
    }
  goto L2975;

 L3077: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3078;
    }
  x2 = XEXP (x1, 1);
  goto L2975;

 L3078: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 300;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L2975;

 L3225: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3226;
    }
  goto L2975;

 L3226: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3227;
    }
  goto L2975;

 L3227: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L3228;
  x2 = XEXP (x1, 1);
  goto L2975;

 L3228: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3229;
    }
  x2 = XEXP (x1, 1);
  goto L2975;

 L3229: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 308;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L2975;

 L2976: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shiftable_operator (x2, SImode))
    {
      operands[7] = x2;
      goto L2977;
    }
  x2 = XEXP (x1, 1);
  goto L3109;

 L2977: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2978;
    }
  x2 = XEXP (x1, 1);
  goto L3109;

 L2978: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2979;
    }
  x2 = XEXP (x1, 1);
  goto L3109;

 L2979: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 294;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L3109;

 L3110: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shift_operator (x2, SImode))
    {
      operands[7] = x2;
      goto L3111;
    }
  x2 = XEXP (x1, 0);
  goto L2948;

 L3111: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3112;
    }
  x2 = XEXP (x1, 0);
  goto L2948;

 L3112: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3113;
    }
  x2 = XEXP (x1, 0);
  goto L2948;

 L3113: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 302;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L2948;

 L2949: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_CODE (x3) == REG)
    goto L6307;
  goto L3082;

 L6307: ATTRIBUTE_UNUSED_LABEL
  if (cc_register (x3, VOIDmode))
    {
      operands[6] = x3;
      goto L2950;
    }
 L6308: ATTRIBUTE_UNUSED_LABEL
  if (cc_register (x3, VOIDmode))
    {
      operands[3] = x3;
      goto L3018;
    }
  goto L3082;

 L2950: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2951;
  x3 = XEXP (x2, 0);
  goto L6308;

 L2951: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (shiftable_operator (x2, SImode))
    {
      operands[5] = x2;
      goto L2952;
    }
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2987;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6308;

 L2952: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2953;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6308;

 L2953: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2954;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6308;

 L2954: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2955;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6308;

 L2955: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 293;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6308;

 L2987: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shiftable_operator (x2, SImode))
    {
      operands[5] = x2;
      goto L2988;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6308;

 L2988: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2989;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6308;

 L2989: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2990;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6308;

 L2990: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 295;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6308;

 L3018: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L3051;
  goto L3082;

 L3051: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L3052;
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3020;
    }
  x2 = XEXP (x1, 0);
  goto L3082;

 L3052: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3053;
    }
  x2 = XEXP (x1, 0);
  goto L3082;

 L3053: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3054;
    }
  x2 = XEXP (x1, 0);
  goto L3082;

 L3054: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 299;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L3082;

 L3020: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L3021;
  x2 = XEXP (x1, 0);
  goto L3082;

 L3021: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3022;
    }
  x2 = XEXP (x1, 0);
  goto L3082;

 L3022: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 297;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L3082;

 L3083: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[6] = x3;
      goto L3084;
    }
  goto L3143;

 L3084: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L3085;
  goto L3143;

 L3085: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (shift_operator (x2, SImode))
    {
      operands[4] = x2;
      goto L3086;
    }
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3121;
    }
  x2 = XEXP (x1, 0);
  goto L3143;

 L3086: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3087;
    }
  x2 = XEXP (x1, 0);
  goto L3143;

 L3087: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3088;
    }
  x2 = XEXP (x1, 0);
  goto L3143;

 L3088: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3089;
    }
  x2 = XEXP (x1, 0);
  goto L3143;

 L3089: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 301;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L3143;

 L3121: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shift_operator (x2, SImode))
    {
      operands[4] = x2;
      goto L3122;
    }
  x2 = XEXP (x1, 0);
  goto L3143;

 L3122: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3123;
    }
  x2 = XEXP (x1, 0);
  goto L3143;

 L3123: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3124;
    }
  x2 = XEXP (x1, 0);
  goto L3143;

 L3124: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 303;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L3143;

 L3144: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L3145;
    }
  goto L3156;

 L3145: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[6] = x3;
      goto L3146;
    }
  goto L3156;

 L3146: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (shift_operator (x2, SImode))
    {
      operands[8] = x2;
      goto L3147;
    }
  x2 = XEXP (x1, 0);
  goto L3156;

 L3147: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3148;
    }
  x2 = XEXP (x1, 0);
  goto L3156;

 L3148: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3149;
    }
  x2 = XEXP (x1, 0);
  goto L3156;

 L3149: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shift_operator (x2, SImode))
    {
      operands[9] = x2;
      goto L3150;
    }
  x2 = XEXP (x1, 0);
  goto L3156;

 L3150: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3151;
    }
  x2 = XEXP (x1, 0);
  goto L3156;

 L3151: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L3152;
    }
  x2 = XEXP (x1, 0);
  goto L3156;

 L3152: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 304;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L3156;

 L3258: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3259;
    }
 L3157: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == REG)
    goto L6309;
  goto L3267;

 L3259: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L3260;
    }
  x3 = XEXP (x2, 0);
  goto L3157;

 L3260: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NEG)
    goto L3261;
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3293;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L3157;

 L3261: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3262;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L3157;

 L3262: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3263;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L3157;

 L3263: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 310;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L3157;

 L3293: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NEG)
    goto L3294;
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L3157;

 L3294: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3295;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L3157;

 L3295: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 312;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L3157;

 L6309: ATTRIBUTE_UNUSED_LABEL
  if (cc_register (x3, VOIDmode))
    {
      operands[8] = x3;
      goto L3158;
    }
 L6310: ATTRIBUTE_UNUSED_LABEL
  if (cc_register (x3, VOIDmode))
    {
      operands[4] = x3;
      goto L3197;
    }
  goto L3267;

 L3158: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L3159;
  x3 = XEXP (x2, 0);
  goto L6310;

 L3159: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (shift_operator (x2, SImode))
    {
      operands[6] = x2;
      goto L3160;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6310;

 L3160: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3161;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6310;

 L3161: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3162;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6310;

 L3162: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shift_operator (x2, SImode))
    {
      operands[7] = x2;
      goto L3163;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6310;

 L3163: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3164;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6310;

 L3164: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L3165;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6310;

 L3165: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 305;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L6310;

 L3197: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L3198;
  goto L3267;

 L3198: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L6311;
  x2 = XEXP (x1, 0);
  goto L3267;

 L6311: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == NOT)
    goto L3199;
  if (shiftable_operator (x2, SImode))
    {
      operands[6] = x2;
      goto L3237;
    }
  x2 = XEXP (x1, 0);
  goto L3267;

 L3199: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3200;
    }
  x2 = XEXP (x1, 0);
  goto L3267;

 L3200: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (shiftable_operator (x2, SImode))
    {
      operands[6] = x2;
      goto L3201;
    }
  x2 = XEXP (x1, 0);
  goto L3267;

 L3201: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3202;
    }
  x2 = XEXP (x1, 0);
  goto L3267;

 L3202: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3203;
    }
  x2 = XEXP (x1, 0);
  goto L3267;

 L3203: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 307;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L3267;

 L3237: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3238;
    }
  x2 = XEXP (x1, 0);
  goto L3267;

 L3238: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3239;
    }
  x2 = XEXP (x1, 0);
  goto L3267;

 L3239: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L3240;
  x2 = XEXP (x1, 0);
  goto L3267;

 L3240: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3241;
    }
  x2 = XEXP (x1, 0);
  goto L3267;

 L3241: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 309;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L3267;

 L3268: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[3] = x3;
      goto L3269;
    }
  goto ret0;

 L3269: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L3270;
  goto ret0;

 L3270: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NEG)
    goto L3271;
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3303;
    }
 L3640: ATTRIBUTE_UNUSED_LABEL
  if (arm_rhs_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3641;
    }
  goto ret0;

 L3271: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3272;
    }
  goto ret0;

 L3272: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (arm_not_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3273;
    }
  goto ret0;

 L3273: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 311;
    }
  goto ret0;

 L3303: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NEG)
    goto L3304;
  x2 = XEXP (x1, 1);
  goto L3640;

 L3304: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3305;
    }
  x2 = XEXP (x1, 1);
  goto L3640;

 L3305: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 313;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L3640;

 L3641: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == NOT)
    goto L3642;
  goto ret0;

 L3642: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3643;
    }
  goto ret0;

 L3643: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 339;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_13 PARAMS ((rtx, rtx, int *));
static int
recog_13 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 0);
  switch (GET_MODE (x1))
    {
    case DImode:
      goto L6021;
    case SImode:
      goto L6023;
    case CC_NOOVmode:
      goto L6025;
    case CCmode:
      goto L6026;
    case CC_Cmode:
      goto L6027;
    case SFmode:
      goto L6028;
    case DFmode:
      goto L6029;
    case XFmode:
      goto L6030;
    case CC_Zmode:
      goto L6038;
    case HImode:
      goto L6039;
    case QImode:
      goto L6046;
    case CC_SWPmode:
      goto L6052;
    case CCFPmode:
      goto L6053;
    case CCFPEmode:
      goto L6054;
    case BLKmode:
      goto L6055;
    default:
      break;
    }
 L2052: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case PC:
      goto L2546;
    case REG:
      goto L6056;
    default:
     break;
   }
 L2226: ATTRIBUTE_UNUSED_LABEL
  switch (GET_MODE (x1))
    {
    case SImode:
      goto L6057;
    case SFmode:
      goto L6058;
    case DFmode:
      goto L6059;
    case CC_NOOVmode:
      goto L6060;
    default:
      break;
    }
 L2729: ATTRIBUTE_UNUSED_LABEL
  if (dominant_cc_register (x1, VOIDmode))
    {
      operands[6] = x1;
      goto L2730;
    }
  goto ret0;

 L6021: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x1, DImode))
    {
      operands[0] = x1;
      goto L10;
    }
 L6022: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, DImode))
    {
      operands[0] = x1;
      goto L24;
    }
 L6031: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x1, DImode))
    {
      operands[0] = x1;
      goto L274;
    }
 L6041: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_di_operand (x1, DImode))
    {
      operands[0] = x1;
      goto L1540;
    }
 L6042: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x1, DImode))
    {
      operands[0] = x1;
      goto L1544;
    }
  goto L2052;

 L10: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == DImode
      && GET_CODE (x1) == PLUS)
    goto L11;
  x1 = XEXP (x0, 0);
  goto L6022;

 L11: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L12;
    }
  x1 = XEXP (x0, 0);
  goto L6022;

 L12: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L13;
    }
  x1 = XEXP (x0, 0);
  goto L6022;

 L13: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 0;
    }
  x1 = XEXP (x0, 0);
  goto L6022;

 L24: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == DImode)
    goto L6062;
  x1 = XEXP (x0, 0);
  goto L6031;

 L6062: ATTRIBUTE_UNUSED_LABEL
  tem = recog_8 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  x1 = XEXP (x0, 0);
  goto L6031;

 L274: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == DImode)
    goto L6105;
  x1 = XEXP (x0, 0);
  goto L6041;

 L6105: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case MINUS:
      goto L275;
    case NEG:
      goto L1262;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L6041;

 L275: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L276;
    }
  x1 = XEXP (x0, 0);
  goto L6041;

 L276: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L277;
    }
  x1 = XEXP (x0, 0);
  goto L6041;

 L277: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 27;
    }
  x1 = XEXP (x0, 0);
  goto L6041;

 L1262: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L1263;
    }
  x1 = XEXP (x0, 0);
  goto L6041;

 L1263: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 126;
    }
  x1 = XEXP (x0, 0);
  goto L6041;

 L1540: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (di_operand (x1, DImode))
    {
      operands[1] = x1;
      goto L1541;
    }
  x1 = XEXP (x0, 0);
  goto L6042;

 L1541: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 173;
    }
  x1 = XEXP (x0, 0);
  goto L6042;

 L1544: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (general_operand (x1, DImode))
    {
      operands[1] = x1;
      goto L1545;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1545: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && (   register_operand (operands[0], DImode)
       || register_operand (operands[1], DImode))))
    {
      return 174;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6023: ATTRIBUTE_UNUSED_LABEL
  tem = recog_10 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  goto L2052;

 L6025: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x1) == REG
      && XINT (x1, 0) == 24)
    goto L87;
  goto L2052;

 L87: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == CC_NOOVmode
      && GET_CODE (x1) == COMPARE)
    goto L88;
  x1 = XEXP (x0, 0);
  goto L2052;

 L88: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6186;
  x1 = XEXP (x0, 0);
  goto L2052;

 L6186: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case PLUS:
      goto L89;
    case MULT:
      goto L472;
    case AND:
      goto L814;
    case ZERO_EXTRACT:
      goto L821;
    case IOR:
      goto L985;
    case XOR:
      goto L1037;
    case NOT:
      goto L1236;
    default:
     break;
   }
 L6192: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x2, SImode))
    {
      operands[3] = x2;
      goto L1195;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L89: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6195;
  x1 = XEXP (x0, 0);
  goto L2052;

 L6195: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == MULT)
    goto L518;
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L90;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L518: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L519;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L519: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L520;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L520: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L521;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L521: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM && !arm_is_xscale)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 49;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L90: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L91;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L91: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM))
    {
      return 7;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L472: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L473;
    }
  goto L6192;

 L473: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L474;
    }
  goto L6192;

 L474: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM && !arm_is_xscale)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 46;
    }
  x2 = XEXP (x1, 0);
  goto L6192;

 L814: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6197;
  x1 = XEXP (x0, 0);
  goto L2052;

 L6197: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == NOT)
    goto L922;
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L815;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L922: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L923;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L923: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L924;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L924: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 93;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L815: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_not_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L816;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L816: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 83;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L821: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L822;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L822: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (const_int_operand (x3, VOIDmode))
    {
      operands[1] = x3;
      goto L823;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L823: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (const_int_operand (x3, VOIDmode))
    {
      operands[2] = x3;
      goto L824;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L824: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM
  && (INTVAL (operands[2]) >= 0 && INTVAL (operands[2]) < 32
      && INTVAL (operands[1]) > 0 
      && INTVAL (operands[1]) + (INTVAL (operands[2]) & 1) <= 8
      && INTVAL (operands[1]) + INTVAL (operands[2]) <= 32)))
    {
      return 84;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L985: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L986;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L986: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L987;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L987: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 100;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1037: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L1038;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1038: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1039;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1039: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM))
    {
      return 107;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1236: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6198;
  x1 = XEXP (x0, 0);
  goto L2052;

 L6198: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x3, SImode))
    {
      operands[3] = x3;
      goto L1237;
    }
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1404;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1237: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L1238;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1238: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L1239;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1239: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 124;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1404: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 147;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1195: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L1196;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1196: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L1197;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1197: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 121;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6026: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x1) == REG
      && XINT (x1, 0) == 24)
    goto L106;
  goto L2052;

 L106: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == CCmode
      && GET_CODE (x1) == COMPARE)
    goto L107;
  x1 = XEXP (x0, 0);
  goto L2052;

 L107: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L108;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L108: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L6200;
 L2074: ATTRIBUTE_UNUSED_LABEL
  if (arm_add_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2075;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6200: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == NEG)
    goto L2097;
  if (shift_operator (x2, SImode))
    {
      operands[3] = x2;
      goto L2081;
    }
  goto L2074;

 L2097: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (shift_operator (x3, SImode))
    {
      operands[3] = x3;
      goto L2098;
    }
  if (arm_add_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L110;
    }
  goto L2074;

 L2098: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2099;
    }
  goto L2074;

 L2099: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2100;
    }
  goto L2074;

 L2100: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 219;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L2074;

 L110: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 9;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L2074;

 L2081: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2082;
    }
  goto L2074;

 L2082: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2083;
    }
  goto L2074;

 L2083: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 217;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L2074;

 L2075: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 216;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6027: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x1) == REG
      && XINT (x1, 0) == 24)
    goto L139;
  goto L2052;

 L139: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == CC_Cmode
      && GET_CODE (x1) == COMPARE)
    goto L140;
  x1 = XEXP (x0, 0);
  goto L2052;

 L140: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L141;
  x1 = XEXP (x0, 0);
  goto L2052;

 L141: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L142;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L142: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_add_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L143;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L143: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (rtx_equal_p (x2, operands[0])
      && (TARGET_ARM))
    {
      return 12;
    }
 L150: ATTRIBUTE_UNUSED_LABEL
  if (rtx_equal_p (x2, operands[1])
      && (TARGET_ARM))
    {
      return 13;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6028: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SFmode))
    {
      operands[0] = x1;
      goto L212;
    }
 L6047: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x1, SFmode))
    {
      operands[0] = x1;
      goto L1655;
    }
  goto L2052;

 L212: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SFmode)
    goto L6202;
  x1 = XEXP (x0, 0);
  goto L6047;

 L6202: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case PLUS:
      goto L213;
    case MINUS:
      goto L395;
    case MULT:
      goto L637;
    case DIV:
      goto L677;
    case MOD:
      goto L717;
    case NEG:
      goto L1277;
    case ABS:
      goto L1324;
    case SQRT:
      goto L1345;
    case FLOAT:
      goto L1408;
    case FLOAT_TRUNCATE:
      goto L1438;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L6047;

 L213: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L214;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L214: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_add_operand (x2, SFmode))
    {
      operands[2] = x2;
      goto L215;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L215: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 20;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L395: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (fpu_rhs_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L396;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L396: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_rhs_operand (x2, SFmode))
    {
      operands[2] = x2;
      goto L397;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L397: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 37;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L637: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L638;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L638: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_rhs_operand (x2, SFmode))
    {
      operands[2] = x2;
      goto L639;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L639: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 59;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L677: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (fpu_rhs_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L678;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L678: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_rhs_operand (x2, SFmode))
    {
      operands[2] = x2;
      goto L679;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L679: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 65;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L717: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L718;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L718: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_rhs_operand (x2, SFmode))
    {
      operands[2] = x2;
      goto L719;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L719: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 71;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1277: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L1278;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1278: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 129;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1324: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L1325;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1325: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 135;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1345: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L1346;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1346: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 139;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1408: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1409;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1409: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 148;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1438: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case DFmode:
      goto L6212;
    case XFmode:
      goto L6213;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L6212: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L1439;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1439: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 154;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L6213: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L1444;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1444: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 155;
    }
  x1 = XEXP (x0, 0);
  goto L6047;

 L1655: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (general_operand (x1, SFmode))
    {
      operands[1] = x1;
      goto L1656;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1656: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && TARGET_HARD_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], SFmode))))
    {
      return 193;
    }
 L1660: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && TARGET_SOFT_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], SFmode))))
    {
      return 194;
    }
 L1664: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && (   register_operand (operands[0], SFmode) 
       || register_operand (operands[1], SFmode))))
    {
      return 195;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6029: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, DFmode))
    {
      operands[0] = x1;
      goto L218;
    }
 L6048: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x1, DFmode))
    {
      operands[0] = x1;
      goto L1667;
    }
 L6049: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_soft_df_operand (x1, DFmode))
    {
      operands[0] = x1;
      goto L1671;
    }
 L6050: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x1, DFmode))
    {
      operands[0] = x1;
      goto L1675;
    }
  goto L2052;

 L218: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == DFmode)
    goto L6214;
  x1 = XEXP (x0, 0);
  goto L6048;

 L6214: ATTRIBUTE_UNUSED_LABEL
  tem = recog_11 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  x1 = XEXP (x0, 0);
  goto L6048;

 L1667: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (general_operand (x1, DFmode))
    {
      operands[1] = x1;
      goto L1668;
    }
  x1 = XEXP (x0, 0);
  goto L6049;

 L1668: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && TARGET_HARD_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], DFmode))))
    {
      return 196;
    }
  x1 = XEXP (x0, 0);
  goto L6049;

 L1671: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (soft_df_operand (x1, DFmode))
    {
      operands[1] = x1;
      goto L1672;
    }
  x1 = XEXP (x0, 0);
  goto L6050;

 L1672: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_SOFT_FLOAT
  ))
    {
      return 197;
    }
  x1 = XEXP (x0, 0);
  goto L6050;

 L1675: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (general_operand (x1, DFmode))
    {
      operands[1] = x1;
      goto L1676;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1676: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && (   register_operand (operands[0], DFmode)
       || register_operand (operands[1], DFmode))))
    {
      return 198;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6030: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, XFmode))
    {
      operands[0] = x1;
      goto L246;
    }
 L6051: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x1, XFmode))
    {
      operands[0] = x1;
      goto L1679;
    }
  goto L2052;

 L246: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == XFmode)
    goto L6261;
  x1 = XEXP (x0, 0);
  goto L6051;

 L6261: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case PLUS:
      goto L247;
    case MINUS:
      goto L429;
    case MULT:
      goto L671;
    case DIV:
      goto L711;
    case MOD:
      goto L751;
    case NEG:
      goto L1293;
    case ABS:
      goto L1340;
    case SQRT:
      goto L1361;
    case FLOAT:
      goto L1418;
    case FLOAT_EXTEND:
      goto L1531;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L6051;

 L247: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L248;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L248: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_add_operand (x2, XFmode))
    {
      operands[2] = x2;
      goto L249;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L249: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 25;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L429: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (fpu_rhs_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L430;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L430: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_rhs_operand (x2, XFmode))
    {
      operands[2] = x2;
      goto L431;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L431: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 42;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L671: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L672;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L672: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_rhs_operand (x2, XFmode))
    {
      operands[2] = x2;
      goto L673;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L673: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 64;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L711: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (fpu_rhs_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L712;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L712: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_rhs_operand (x2, XFmode))
    {
      operands[2] = x2;
      goto L713;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L713: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 70;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L751: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L752;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L752: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_rhs_operand (x2, XFmode))
    {
      operands[2] = x2;
      goto L753;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L753: ATTRIBUTE_UNUSED_LABEL
  if ((ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 76;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1293: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L1294;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1294: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 132;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1340: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L1341;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1341: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 138;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1361: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L1362;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1362: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 142;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1418: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L1419;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1419: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 150;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1531: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case SFmode:
      goto L6271;
    case DFmode:
      goto L6272;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L6271: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L1532;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1532: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 171;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L6272: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L1537;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1537: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 172;
    }
  x1 = XEXP (x0, 0);
  goto L6051;

 L1679: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (general_operand (x1, XFmode))
    {
      operands[1] = x1;
      goto L1680;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1680: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT && (ENABLE_XF_PATTERNS || reload_completed)))
    {
      return 199;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6038: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x1) == REG
      && XINT (x1, 0) == 24)
    goto L1487;
  goto L2052;

 L1487: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == CC_Zmode
      && GET_CODE (x1) == COMPARE)
    goto L1488;
  x1 = XEXP (x0, 0);
  goto L2052;

 L1488: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, QImode))
    {
      operands[0] = x2;
      goto L1489;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1489: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM))
    {
      return 164;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6039: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, HImode))
    {
      operands[0] = x1;
      goto L1510;
    }
 L6044: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x1, HImode))
    {
      operands[0] = x1;
      goto L1609;
    }
 L6045: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, HImode))
    {
      operands[0] = x1;
      goto L1622;
    }
  goto L2052;

 L1510: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == HImode
      && GET_CODE (x1) == SIGN_EXTEND)
    goto L1511;
  x1 = XEXP (x0, 0);
  goto L6044;

 L1511: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (memory_operand (x2, QImode))
    {
      operands[1] = x2;
      goto L1512;
    }
  x1 = XEXP (x0, 0);
  goto L6044;

 L1512: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_arch4))
    {
      return 167;
    }
  x1 = XEXP (x0, 0);
  goto L6044;

 L1609: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (general_operand (x1, HImode))
    {
      operands[1] = x1;
      goto L1610;
    }
  x1 = XEXP (x0, 0);
  goto L6045;

 L1610: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && (   register_operand (operands[0], HImode)
       || register_operand (operands[1], HImode))))
    {
      return 183;
    }
 L1619: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && arm_arch4
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1])))))
    {
      return 185;
    }
  x1 = XEXP (x0, 0);
  goto L6045;

 L1622: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (general_operand (x1, HImode))
    {
      operands[1] = x1;
      goto L1623;
    }
 L1636: ATTRIBUTE_UNUSED_LABEL
  if (arm_rhs_operand (x1, HImode))
    {
      operands[1] = x1;
      goto L1637;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1623: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && !arm_arch4
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1])))))
    {
      return 186;
    }
 L1627: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && !arm_arch4
   && BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1])))))
    {
      return 187;
    }
  x1 = XEXP (x0, 1);
  goto L1636;

 L1637: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_MMU_TRAPS))
    {
      return 189;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6046: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x1, QImode))
    {
      operands[0] = x1;
      goto L1647;
    }
  goto L2052;

 L1647: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (general_operand (x1, QImode))
    {
      operands[1] = x1;
      goto L1648;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L1648: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && (   register_operand (operands[0], QImode)
       || register_operand (operands[1], QImode))))
    {
      return 191;
    }
 L1652: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && (   register_operand (operands[0], QImode)
       || register_operand (operands[1], QImode))))
    {
      return 192;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6052: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x1) == REG
      && XINT (x1, 0) == 24)
    goto L2086;
  goto L2052;

 L2086: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == CC_SWPmode
      && GET_CODE (x1) == COMPARE)
    goto L2087;
  x1 = XEXP (x0, 0);
  goto L2052;

 L2087: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (shift_operator (x2, SImode))
    {
      operands[3] = x2;
      goto L2088;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2088: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2089;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2089: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (reg_or_int_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2090;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2090: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L2091;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2091: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 218;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6053: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x1) == REG
      && XINT (x1, 0) == 24)
    goto L2103;
  goto L2052;

 L2103: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == CCFPmode
      && GET_CODE (x1) == COMPARE)
    goto L2104;
  x1 = XEXP (x0, 0);
  goto L2052;

 L2104: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case SFmode:
      goto L6273;
    case DFmode:
      goto L6275;
    case XFmode:
      goto L6276;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6273: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SFmode))
    {
      operands[0] = x2;
      goto L2105;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2105: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_add_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L2106;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2106: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 220;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6275: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L2117;
  if (s_register_operand (x2, DFmode))
    {
      operands[0] = x2;
      goto L2111;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2117: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[0] = x3;
      goto L2118;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2118: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_add_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L2119;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2119: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 222;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2111: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6278;
  x1 = XEXP (x0, 0);
  goto L2052;

 L6278: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L2125;
  if (fpu_add_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L2112;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2125: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L2126;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2126: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 223;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2112: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 221;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6276: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, XFmode))
    {
      operands[0] = x2;
      goto L2131;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2131: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_add_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L2132;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2132: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 224;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6054: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x1) == REG
      && XINT (x1, 0) == 24)
    goto L2135;
  goto L2052;

 L2135: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == CCFPEmode
      && GET_CODE (x1) == COMPARE)
    goto L2136;
  x1 = XEXP (x0, 0);
  goto L2052;

 L2136: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case SFmode:
      goto L6279;
    case DFmode:
      goto L6281;
    case XFmode:
      goto L6282;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6279: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SFmode))
    {
      operands[0] = x2;
      goto L2137;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2137: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_add_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L2138;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2138: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 225;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6281: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L2149;
  if (s_register_operand (x2, DFmode))
    {
      operands[0] = x2;
      goto L2143;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2149: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[0] = x3;
      goto L2150;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2150: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_add_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L2151;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2151: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 227;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2143: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6284;
  x1 = XEXP (x0, 0);
  goto L2052;

 L6284: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == FLOAT_EXTEND)
    goto L2157;
  if (fpu_add_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L2144;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2157: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L2158;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2158: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 228;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2144: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 226;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6282: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, XFmode))
    {
      operands[0] = x2;
      goto L2163;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2163: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_add_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L2164;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2164: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT))
    {
      return 229;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L6055: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x1) == MEM)
    goto L3686;
  goto L2052;

 L3686: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_CODE (x2) == SCRATCH)
    goto L3687;
  goto L2052;

 L3687: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == BLKmode
      && GET_CODE (x1) == UNSPEC
      && XVECLEN (x1, 0) == 2
      && XINT (x1, 1) == 5)
    goto L3688;
  x1 = XEXP (x0, 0);
  goto L2052;

 L3688: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3689;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L3689: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      return 343;
    }
  x1 = XEXP (x0, 0);
  goto L2052;

 L2546: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6285;
 L2053: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case IF_THEN_ELSE:
      goto L2171;
    case LABEL_REF:
      goto L2281;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L2226;

 L6285: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L2547;
    }
 L6286: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L2551;
    }
 L6287: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L2555;
    }
  goto L2053;

 L2547: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 264;
    }
  x1 = XEXP (x0, 1);
  goto L6286;

 L2551: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 265;
    }
  x1 = XEXP (x0, 1);
  goto L6287;

 L2555: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 266;
    }
  x1 = XEXP (x0, 1);
  goto L2053;

 L2171: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_CODE (x2))
    {
    case UNEQ:
      goto L2172;
    case LTGT:
      goto L2181;
    case EQ:
    case NE:
    case LE:
    case LT:
    case GE:
    case GT:
    case GEU:
    case GTU:
    case LEU:
    case LTU:
    case UNORDERED:
    case ORDERED:
    case UNLT:
    case UNLE:
    case UNGE:
    case UNGT:
      goto L2054;
    default:
      goto L2189;
   }
 L2054: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[0] = x2;
      goto L2055;
    }
 L2189: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[1] = x2;
      goto L2190;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2172: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[1] = x3;
      goto L2173;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2173: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2174;
  x1 = XEXP (x0, 0);
  goto L2226;

 L2174: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  switch (GET_CODE (x2))
    {
    case LABEL_REF:
      goto L2175;
    case PC:
      goto L2202;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2175: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[0] = x3;
  goto L2176;

 L2176: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == PC
      && (TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 231;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2202: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == LABEL_REF)
    goto L2203;
  x1 = XEXP (x0, 0);
  goto L2226;

 L2203: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[0] = x3;
  goto L2204;

 L2204: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 234;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2181: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[1] = x3;
      goto L2182;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2182: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2183;
  x1 = XEXP (x0, 0);
  goto L2226;

 L2183: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  switch (GET_CODE (x2))
    {
    case LABEL_REF:
      goto L2184;
    case PC:
      goto L2212;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2184: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[0] = x3;
  goto L2185;

 L2185: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == PC
      && (TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 232;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2212: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == LABEL_REF)
    goto L2213;
  x1 = XEXP (x0, 0);
  goto L2226;

 L2213: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[0] = x3;
  goto L2214;

 L2214: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 235;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2055: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2065;
    }
 L2503: ATTRIBUTE_UNUSED_LABEL
  if (cc_register (x3, VOIDmode))
    {
      operands[1] = x3;
      goto L2504;
    }
  goto L2189;

 L2065: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == NEG)
    goto L2066;
  if (nonmemory_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2057;
    }
  x3 = XEXP (x2, 0);
  goto L2503;

 L2066: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (nonmemory_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2067;
    }
  x3 = XEXP (x2, 0);
  goto L2503;

 L2067: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == LABEL_REF)
    goto L2068;
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2503;

 L2068: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[3] = x3;
  goto L2069;

 L2069: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == PC
      && (TARGET_THUMB))
    {
      return 215;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2503;

 L2057: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == LABEL_REF)
    goto L2058;
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2503;

 L2058: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[3] = x3;
  goto L2059;

 L2059: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == PC
      && (TARGET_THUMB))
    {
      return 214;
    }
  x2 = XEXP (x1, 0);
  x3 = XEXP (x2, 0);
  goto L2503;

 L2504: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2505;
  goto L2189;

 L2505: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  switch (GET_CODE (x2))
    {
    case RETURN:
      goto L2506;
    case PC:
      goto L2514;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L2189;

 L2506: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == PC
      && (TARGET_ARM && USE_RETURN_INSN (TRUE)))
    {
      return 259;
    }
  x2 = XEXP (x1, 0);
  goto L2189;

 L2514: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == RETURN
      && (TARGET_ARM && USE_RETURN_INSN (TRUE)))
    {
      return 260;
    }
  x2 = XEXP (x1, 0);
  goto L2189;

 L2190: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[2] = x3;
      goto L2191;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2191: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2192;
  x1 = XEXP (x0, 0);
  goto L2226;

 L2192: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  switch (GET_CODE (x2))
    {
    case LABEL_REF:
      goto L2193;
    case PC:
      goto L2222;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2193: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[0] = x3;
  goto L2194;

 L2194: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == PC
      && (TARGET_ARM))
    {
      return 233;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2222: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (GET_CODE (x2) == LABEL_REF)
    goto L2223;
  x1 = XEXP (x0, 0);
  goto L2226;

 L2223: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[0] = x3;
  goto L2224;

 L2224: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 236;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L2281: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  operands[0] = x2;
  goto L2282;

 L2282: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 244;
    }
 L2287: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 245;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L6056: ATTRIBUTE_UNUSED_LABEL
  if (cc_register (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L2167;
    }
  goto L2226;

 L2167: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (rtx_equal_p (x1, operands[0])
      && (TARGET_ARM))
    {
      return 230;
    }
  x1 = XEXP (x0, 0);
  goto L2226;

 L6057: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L2227;
    }
  goto L2729;

 L2227: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6289;
  x1 = XEXP (x0, 0);
  goto L2729;

 L6289: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case NEG:
      goto L2233;
    case NOT:
      goto L2239;
    case IF_THEN_ELSE:
      goto L2245;
    case MINUS:
      goto L2607;
    case AND:
      goto L2654;
    case IOR:
      goto L2662;
    case SIGN_EXTRACT:
      goto L3656;
    case UNSPEC:
      goto L6299;
    case EQ:
    case NE:
    case LE:
    case LT:
    case GE:
    case GT:
    case GEU:
    case GTU:
    case LEU:
    case LTU:
    case UNORDERED:
    case ORDERED:
    case UNLT:
    case UNLE:
    case UNGE:
    case UNGT:
      goto L6288;
    default:
      goto L6292;
   }
 L6288: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, SImode))
    {
      operands[1] = x1;
      goto L2228;
    }
 L6292: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x1, SImode))
    {
      operands[1] = x1;
      goto L2560;
    }
 L6296: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x1, SImode))
    {
      operands[5] = x1;
      goto L2705;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2233: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_comparison_operator (x2, SImode))
    {
      operands[1] = x2;
      goto L2234;
    }
 L2786: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[3] = x2;
      goto L2787;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2234: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[2] = x3;
      goto L2235;
    }
  goto L2786;

 L2235: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0
      && (TARGET_ARM))
    {
      return 238;
    }
  goto L2786;

 L2787: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2788;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2788: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2789;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2789: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 284;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2239: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6301;
  x1 = XEXP (x0, 0);
  goto L2729;

 L6301: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == SIGN_EXTRACT)
    goto L3674;
  if (arm_comparison_operator (x2, SImode))
    {
      operands[1] = x2;
      goto L2240;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L3674: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L3675;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L3675: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 1)
    goto L3676;
  x1 = XEXP (x0, 0);
  goto L2729;

 L3676: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (const_int_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3677;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L3677: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 341;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2240: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[2] = x3;
      goto L2241;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2241: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0
      && (TARGET_ARM))
    {
      return 239;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2245: ATTRIBUTE_UNUSED_LABEL
  tem = recog_12 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  x1 = XEXP (x0, 0);
  goto L2729;

 L2607: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2608;
    }
  goto L6292;

 L2608: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L6313;
  goto L6292;

 L6313: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x2, SImode))
    {
      operands[2] = x2;
      goto L2609;
    }
  if (arm_comparison_operator (x2, SImode))
    {
      operands[4] = x2;
      goto L2725;
    }
  goto L6292;

 L2609: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2610;
    }
  goto L6292;

 L2610: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (reg_or_int_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L2611;
    }
  goto L6292;

 L2611: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 271;
    }
  x1 = XEXP (x0, 1);
  goto L6292;

 L2725: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2726;
    }
  goto L6292;

 L2726: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2727;
    }
  goto L6292;

 L2727: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 279;
    }
  x1 = XEXP (x0, 1);
  goto L6292;

 L2654: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_comparison_operator (x2, SImode))
    {
      operands[1] = x2;
      goto L2655;
    }
  goto L6292;

 L2655: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[3] = x3;
      goto L2656;
    }
  goto L6292;

 L2656: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2657;
  goto L6292;

 L2657: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L2658;
    }
  goto L6292;

 L2658: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 274;
    }
  x1 = XEXP (x0, 1);
  goto L6292;

 L2662: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_comparison_operator (x2, SImode))
    {
      operands[2] = x2;
      goto L2663;
    }
  goto L6292;

 L2663: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[3] = x3;
      goto L2664;
    }
  goto L6292;

 L2664: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2665;
  goto L6292;

 L2665: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2666;
    }
  goto L6292;

 L2666: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 275;
    }
  x1 = XEXP (x0, 1);
  goto L6292;

 L3656: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3657;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L3657: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 1)
    goto L3658;
  x1 = XEXP (x0, 0);
  goto L2729;

 L3658: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (const_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3659;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L3659: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 340;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L6299: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) == 1
      && XINT (x1, 1) == 5)
    goto L3723;
  x1 = XEXP (x0, 0);
  goto L2729;

 L3723: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3724;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L3724: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_arch5))
    {
      return 352;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2228: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[2] = x2;
      goto L2229;
    }
 L2678: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L2679;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2229: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM))
    {
      return 237;
    }
  x2 = XEXP (x1, 0);
  goto L2678;

 L2679: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (arm_add_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L2680;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2680: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 276;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2560: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6315;
  goto L6296;

 L6315: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x2, SImode))
    {
      operands[3] = x2;
      goto L2561;
    }
  if (memory_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3319;
    }
  goto L6296;

 L2561: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L2562;
    }
  goto L6296;

 L2562: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (reg_or_int_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L2563;
    }
  goto L6296;

 L2563: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L2564;
    }
  goto L6296;

 L2564: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return 268;
    }
  x1 = XEXP (x0, 1);
  goto L6296;

 L3319: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (memory_operand (x2, SImode))
    {
      operands[3] = x2;
      goto L3320;
    }
  goto L6296;

 L3320: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && adjacent_mem_locations (operands[2], operands[3]))
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 314;
    }
  x1 = XEXP (x0, 1);
  goto L6296;

 L2705: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_comparison_operator (x2, SImode))
    {
      operands[4] = x2;
      goto L2706;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2706: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2707;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2707: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L2708;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2708: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L2709;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2709: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 278;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L6058: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SFmode))
    {
      operands[0] = x1;
      goto L2253;
    }
  goto L2729;

 L2253: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SFmode
      && GET_CODE (x1) == IF_THEN_ELSE)
    goto L2254;
  x1 = XEXP (x0, 0);
  goto L2729;

 L2254: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[3] = x2;
      goto L2255;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2255: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[4] = x3;
      goto L2256;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2256: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2257;
  x1 = XEXP (x0, 0);
  goto L2729;

 L2257: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SFmode)
    goto L6317;
  x1 = XEXP (x0, 0);
  goto L2729;

 L6317: ATTRIBUTE_UNUSED_LABEL
  if (fpu_add_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L2258;
    }
 L6318: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L2267;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2258: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (fpu_add_operand (x2, SFmode))
    {
      operands[2] = x2;
      goto L2259;
    }
  x2 = XEXP (x1, 1);
  goto L6318;

 L2259: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 241;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L6318;

 L2267: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (s_register_operand (x2, SFmode))
    {
      operands[2] = x2;
      goto L2268;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2268: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_SOFT_FLOAT))
    {
      return 242;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L6059: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, DFmode))
    {
      operands[0] = x1;
      goto L2271;
    }
  goto L2729;

 L2271: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == DFmode
      && GET_CODE (x1) == IF_THEN_ELSE)
    goto L2272;
  x1 = XEXP (x0, 0);
  goto L2729;

 L2272: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (arm_comparison_operator (x2, VOIDmode))
    {
      operands[3] = x2;
      goto L2273;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2273: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (cc_register (x3, VOIDmode))
    {
      operands[4] = x3;
      goto L2274;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2274: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2275;
  x1 = XEXP (x0, 0);
  goto L2729;

 L2275: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (fpu_add_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L2276;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2276: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 2);
  if (fpu_add_operand (x2, DFmode))
    {
      operands[2] = x2;
      goto L2277;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2277: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && TARGET_HARD_FLOAT))
    {
      return 243;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L6060: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x1) == REG)
    goto L6320;
  goto L2729;

 L6320: ATTRIBUTE_UNUSED_LABEL
  if (XINT (x1, 0) == 24)
    goto L2597;
 L6319: ATTRIBUTE_UNUSED_LABEL
  if (cc_register (x1, CC_NOOVmode))
    {
      operands[0] = x1;
      goto L2517;
    }
  goto L2729;

 L2597: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == CC_NOOVmode
      && GET_CODE (x1) == COMPARE)
    goto L2598;
  x1 = XEXP (x0, 0);
  goto L6319;

 L2598: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6322;
  x1 = XEXP (x0, 0);
  goto L6319;

 L6322: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == MINUS)
    goto L2646;
 L6321: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x2, SImode))
    {
      operands[1] = x2;
      goto L2599;
    }
  x1 = XEXP (x0, 0);
  goto L6319;

 L2646: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L2647;
    }
  goto L6321;

 L2647: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (shift_operator (x3, SImode))
    {
      operands[2] = x3;
      goto L2648;
    }
  goto L6321;

 L2648: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2649;
    }
  goto L6321;

 L2649: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (reg_or_int_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L2650;
    }
  goto L6321;

 L2650: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 273;
    }
  x2 = XEXP (x1, 0);
  goto L6321;

 L2599: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (shift_operator (x3, SImode))
    {
      operands[3] = x3;
      goto L2600;
    }
  x1 = XEXP (x0, 0);
  goto L6319;

 L2600: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L2601;
    }
  x1 = XEXP (x0, 0);
  goto L6319;

 L2601: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (reg_or_int_operand (x4, SImode))
    {
      operands[5] = x4;
      goto L2602;
    }
  x1 = XEXP (x0, 0);
  goto L6319;

 L2602: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L2603;
    }
  x1 = XEXP (x0, 0);
  goto L6319;

 L2603: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM)
      && pnum_clobbers != NULL)
    {
      *pnum_clobbers = 1;
      return 270;
    }
  x1 = XEXP (x0, 0);
  goto L6319;

 L2517: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == CC_NOOVmode
      && GET_CODE (x1) == COMPARE)
    goto L2518;
  x1 = XEXP (x0, 0);
  goto L2729;

 L2518: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_CODE (x2) == UNSPEC
      && XVECLEN (x2, 0) == 1
      && XINT (x2, 1) == 7)
    goto L2519;
  x1 = XEXP (x0, 0);
  goto L2729;

 L2519: ATTRIBUTE_UNUSED_LABEL
  x3 = XVECEXP (x2, 0, 0);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0)
    goto L2520;
  x1 = XEXP (x0, 0);
  goto L2729;

 L2520: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM))
    {
      return 261;
    }
  x1 = XEXP (x0, 0);
  goto L2729;

 L2730: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_CODE (x1) == COMPARE)
    goto L2731;
  goto ret0;

 L2731: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == SImode)
    goto L6323;
  goto ret0;

 L6323: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case IF_THEN_ELSE:
      goto L2732;
    case AND:
      goto L2756;
    case IOR:
      goto L2767;
    default:
     break;
   }
  goto ret0;

 L2732: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[4] = x3;
      goto L2733;
    }
  goto ret0;

 L2733: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[0] = x4;
      goto L2734;
    }
  goto ret0;

 L2734: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2735;
    }
  goto ret0;

 L2735: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_comparison_operator (x3, SImode))
    {
      operands[5] = x3;
      goto L2736;
    }
  goto ret0;

 L2736: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2737;
    }
  goto ret0;

 L2737: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2738;
    }
  goto ret0;

 L2738: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (GET_CODE (x3) == CONST_INT)
    goto L6326;
  goto ret0;

 L6326: ATTRIBUTE_UNUSED_LABEL
  if ((int) XWINT (x3, 0) == XWINT (x3, 0))
    switch ((int) XWINT (x3, 0))
      {
      case 0:
        goto L2739;
      case 1:
        goto L2751;
      default:
        break;
      }
  goto ret0;

 L2739: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM))
    {
      return 280;
    }
  goto ret0;

 L2751: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM))
    {
      return 281;
    }
  goto ret0;

 L2756: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[4] = x3;
      goto L2757;
    }
  goto ret0;

 L2757: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[0] = x4;
      goto L2758;
    }
  goto ret0;

 L2758: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2759;
    }
  goto ret0;

 L2759: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_comparison_operator (x3, SImode))
    {
      operands[5] = x3;
      goto L2760;
    }
  goto ret0;

 L2760: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2761;
    }
  goto ret0;

 L2761: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2762;
    }
  goto ret0;

 L2762: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM))
    {
      return 282;
    }
  goto ret0;

 L2767: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[4] = x3;
      goto L2768;
    }
  goto ret0;

 L2768: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[0] = x4;
      goto L2769;
    }
  goto ret0;

 L2769: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L2770;
    }
  goto ret0;

 L2770: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_comparison_operator (x3, SImode))
    {
      operands[5] = x3;
      goto L2771;
    }
  goto ret0;

 L2771: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L2772;
    }
  goto ret0;

 L2772: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L2773;
    }
  goto ret0;

 L2773: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0
      && (TARGET_ARM))
    {
      return 283;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_14 PARAMS ((rtx, rtx, int *));
static int
recog_14 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case SImode:
      goto L6336;
    case SFmode:
      goto L6337;
    case DFmode:
      goto L6338;
    case XFmode:
      goto L6339;
    case DImode:
      goto L6340;
    default:
      break;
    }
  goto ret0;

 L6336: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L4045;
    }
  goto ret0;

 L4045: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L6341;
  goto ret0;

 L6341: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case PLUS:
      goto L4046;
    case MINUS:
      goto L4120;
    case MULT:
      goto L4184;
    case AND:
      goto L4486;
    case IOR:
      goto L4587;
    case XOR:
      goto L4629;
    default:
     break;
   }
  goto ret0;

 L4046: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4047;
    }
  goto ret0;

 L4047: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (reg_or_int_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L4048;
    }
  goto ret0;

 L4048: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 507;
    }
  goto ret0;

 L4120: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (reg_or_int_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4121;
    }
  goto ret0;

 L4121: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L4122;
    }
  goto ret0;

 L4122: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 514;
    }
  goto ret0;

 L4184: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L4185;
    }
  goto ret0;

 L4185: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4186;
    }
  goto ret0;

 L4186: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 520;
    }
  goto ret0;

 L4486: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6348;
  goto ret0;

 L6348: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == NOT)
    goto L4532;
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4487;
    }
  goto ret0;

 L4532: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4533;
    }
  goto ret0;

 L4533: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4534;
    }
  goto ret0;

 L4534: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 550;
    }
  goto ret0;

 L4487: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (reg_or_int_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L4488;
    }
  goto ret0;

 L4488: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 546;
    }
  goto ret0;

 L4587: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4588;
    }
  goto ret0;

 L4588: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (reg_or_int_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L4589;
    }
  goto ret0;

 L4589: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 555;
    }
  goto ret0;

 L4629: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4630;
    }
  goto ret0;

 L4630: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L4631;
    }
  goto ret0;

 L4631: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 559;
    }
  goto ret0;

 L6337: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SFmode))
    {
      operands[0] = x2;
      goto L4055;
    }
  goto ret0;

 L4055: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SFmode)
    goto L6349;
  goto ret0;

 L6349: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case PLUS:
      goto L4056;
    case MULT:
      goto L4294;
    case DIV:
      goto L4358;
    case MOD:
      goto L4422;
    default:
     break;
   }
  goto ret0;

 L4056: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4057;
    }
  goto ret0;

 L4057: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (fpu_add_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L4058;
    }
  goto ret0;

 L4058: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 508;
    }
  goto ret0;

 L4294: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4295;
    }
  goto ret0;

 L4295: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (fpu_rhs_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L4296;
    }
  goto ret0;

 L4296: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 528;
    }
  goto ret0;

 L4358: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (fpu_rhs_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4359;
    }
  goto ret0;

 L4359: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (fpu_rhs_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L4360;
    }
  goto ret0;

 L4360: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 534;
    }
  goto ret0;

 L4422: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4423;
    }
  goto ret0;

 L4423: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (fpu_rhs_operand (x3, SFmode))
    {
      operands[2] = x3;
      goto L4424;
    }
  goto ret0;

 L4424: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 540;
    }
  goto ret0;

 L6338: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DFmode))
    {
      operands[0] = x2;
      goto L4065;
    }
  goto ret0;

 L4065: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6353;
  goto ret0;

 L6353: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case PLUS:
      goto L4066;
    case MINUS:
      goto L4130;
    case MULT:
      goto L4304;
    case DIV:
      goto L4368;
    case MOD:
      goto L4432;
    default:
     break;
   }
  goto ret0;

 L4066: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DFmode)
    goto L6359;
  goto ret0;

 L6359: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4077;
  if (s_register_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4067;
    }
  goto ret0;

 L4077: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[1] = x4;
      goto L4078;
    }
  goto ret0;

 L4078: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6361;
  goto ret0;

 L6361: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4101;
  if (fpu_add_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4079;
    }
  goto ret0;

 L4101: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4102;
    }
  goto ret0;

 L4102: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 512;
    }
  goto ret0;

 L4079: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 510;
    }
  goto ret0;

 L4067: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6363;
  goto ret0;

 L6363: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4089;
  if (fpu_add_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4068;
    }
  goto ret0;

 L4089: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4090;
    }
  goto ret0;

 L4090: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 511;
    }
  goto ret0;

 L4068: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 509;
    }
  goto ret0;

 L4130: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DFmode)
    goto L6365;
  goto ret0;

 L6365: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4141;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4131;
    }
  goto ret0;

 L4141: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[1] = x4;
      goto L4142;
    }
  goto ret0;

 L4142: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6367;
  goto ret0;

 L6367: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4165;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4143;
    }
  goto ret0;

 L4165: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4166;
    }
  goto ret0;

 L4166: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 518;
    }
  goto ret0;

 L4143: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 516;
    }
  goto ret0;

 L4131: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6369;
  goto ret0;

 L6369: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4153;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4132;
    }
  goto ret0;

 L4153: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4154;
    }
  goto ret0;

 L4154: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 517;
    }
  goto ret0;

 L4132: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 515;
    }
  goto ret0;

 L4304: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DFmode)
    goto L6371;
  goto ret0;

 L6371: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4315;
  if (s_register_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4305;
    }
  goto ret0;

 L4315: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[1] = x4;
      goto L4316;
    }
  goto ret0;

 L4316: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6373;
  goto ret0;

 L6373: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4339;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4317;
    }
  goto ret0;

 L4339: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4340;
    }
  goto ret0;

 L4340: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 532;
    }
  goto ret0;

 L4317: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 530;
    }
  goto ret0;

 L4305: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6375;
  goto ret0;

 L6375: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4327;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4306;
    }
  goto ret0;

 L4327: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4328;
    }
  goto ret0;

 L4328: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 531;
    }
  goto ret0;

 L4306: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 529;
    }
  goto ret0;

 L4368: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DFmode)
    goto L6377;
  goto ret0;

 L6377: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4379;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4369;
    }
  goto ret0;

 L4379: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[1] = x4;
      goto L4380;
    }
  goto ret0;

 L4380: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6379;
  goto ret0;

 L6379: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4403;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4381;
    }
  goto ret0;

 L4403: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4404;
    }
  goto ret0;

 L4404: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 538;
    }
  goto ret0;

 L4381: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 536;
    }
  goto ret0;

 L4369: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6381;
  goto ret0;

 L6381: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4391;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4370;
    }
  goto ret0;

 L4391: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4392;
    }
  goto ret0;

 L4392: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 537;
    }
  goto ret0;

 L4370: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 535;
    }
  goto ret0;

 L4432: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DFmode)
    goto L6383;
  goto ret0;

 L6383: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4443;
  if (s_register_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4433;
    }
  goto ret0;

 L4443: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[1] = x4;
      goto L4444;
    }
  goto ret0;

 L4444: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6385;
  goto ret0;

 L6385: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4467;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4445;
    }
  goto ret0;

 L4467: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4468;
    }
  goto ret0;

 L4468: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 544;
    }
  goto ret0;

 L4445: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 542;
    }
  goto ret0;

 L4433: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DFmode)
    goto L6387;
  goto ret0;

 L6387: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4455;
  if (fpu_rhs_operand (x3, DFmode))
    {
      operands[2] = x3;
      goto L4434;
    }
  goto ret0;

 L4455: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[2] = x4;
      goto L4456;
    }
  goto ret0;

 L4456: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 543;
    }
  goto ret0;

 L4434: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 541;
    }
  goto ret0;

 L6339: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, XFmode))
    {
      operands[0] = x2;
      goto L4109;
    }
  goto ret0;

 L4109: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == XFmode)
    goto L6388;
  goto ret0;

 L6388: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case PLUS:
      goto L4110;
    case MINUS:
      goto L4174;
    case MULT:
      goto L4348;
    case DIV:
      goto L4412;
    case MOD:
      goto L4476;
    default:
     break;
   }
  goto ret0;

 L4110: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4111;
    }
  goto ret0;

 L4111: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (fpu_add_operand (x3, XFmode))
    {
      operands[2] = x3;
      goto L4112;
    }
  goto ret0;

 L4112: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 513;
    }
  goto ret0;

 L4174: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (fpu_rhs_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4175;
    }
  goto ret0;

 L4175: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (fpu_rhs_operand (x3, XFmode))
    {
      operands[2] = x3;
      goto L4176;
    }
  goto ret0;

 L4176: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 519;
    }
  goto ret0;

 L4348: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4349;
    }
  goto ret0;

 L4349: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (fpu_rhs_operand (x3, XFmode))
    {
      operands[2] = x3;
      goto L4350;
    }
  goto ret0;

 L4350: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 533;
    }
  goto ret0;

 L4412: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (fpu_rhs_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4413;
    }
  goto ret0;

 L4413: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (fpu_rhs_operand (x3, XFmode))
    {
      operands[2] = x3;
      goto L4414;
    }
  goto ret0;

 L4414: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 539;
    }
  goto ret0;

 L4476: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4477;
    }
  goto ret0;

 L4477: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (fpu_rhs_operand (x3, XFmode))
    {
      operands[2] = x3;
      goto L4478;
    }
  goto ret0;

 L4478: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 545;
    }
  goto ret0;

 L6340: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[0] = x2;
      goto L4219;
    }
  goto ret0;

 L4219: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode)
    goto L6393;
  goto ret0;

 L6393: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case MULT:
      goto L4220;
    case AND:
      goto L4496;
    case IOR:
      goto L4555;
    case XOR:
      goto L4597;
    default:
     break;
   }
  goto ret0;

 L4220: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode)
    goto L6397;
  goto ret0;

 L6397: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case SIGN_EXTEND:
      goto L4221;
    case ZERO_EXTEND:
      goto L4233;
    default:
     break;
   }
  goto ret0;

 L4221: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L4222;
    }
  goto ret0;

 L4222: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == SIGN_EXTEND)
    goto L4223;
  goto ret0;

 L4223: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4224;
    }
  goto ret0;

 L4224: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_fast_multiply)))
    {
      return 523;
    }
  goto ret0;

 L4233: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L4234;
    }
  goto ret0;

 L4234: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == ZERO_EXTEND)
    goto L4235;
  goto ret0;

 L4235: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4236;
    }
  goto ret0;

 L4236: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_fast_multiply)))
    {
      return 524;
    }
  goto ret0;

 L4496: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == NOT)
    goto L4497;
  goto ret0;

 L4497: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == DImode)
    goto L6400;
  goto ret0;

 L6400: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case ZERO_EXTEND:
      goto L4509;
    case SIGN_EXTEND:
      goto L4521;
    case SUBREG:
    case REG:
      goto L6399;
    default:
      goto ret0;
   }
 L6399: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, DImode))
    {
      operands[1] = x4;
      goto L4498;
    }
  goto ret0;

 L4509: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L4510;
    }
  goto ret0;

 L4510: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4511;
    }
  goto ret0;

 L4511: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 548;
    }
  goto ret0;

 L4521: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L4522;
    }
  goto ret0;

 L4522: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4523;
    }
  goto ret0;

 L4523: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 549;
    }
  goto ret0;

 L4498: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[2] = x3;
      goto L4499;
    }
  goto ret0;

 L4499: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 547;
    }
  goto ret0;

 L4555: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode)
    goto L6403;
  goto ret0;

 L6403: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case ZERO_EXTEND:
      goto L4566;
    case SIGN_EXTEND:
      goto L4577;
    case SUBREG:
    case REG:
      goto L6402;
    default:
      goto ret0;
   }
 L6402: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4556;
    }
  goto ret0;

 L4566: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4567;
    }
  goto ret0;

 L4567: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4568;
    }
  goto ret0;

 L4568: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 553;
    }
  goto ret0;

 L4577: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4578;
    }
  goto ret0;

 L4578: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4579;
    }
  goto ret0;

 L4579: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 554;
    }
  goto ret0;

 L4556: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[2] = x3;
      goto L4557;
    }
  goto ret0;

 L4557: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 552;
    }
  goto ret0;

 L4597: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode)
    goto L6406;
  goto ret0;

 L6406: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case ZERO_EXTEND:
      goto L4608;
    case SIGN_EXTEND:
      goto L4619;
    case SUBREG:
    case REG:
      goto L6405;
    default:
      goto ret0;
   }
 L6405: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4598;
    }
  goto ret0;

 L4608: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4609;
    }
  goto ret0;

 L4609: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4610;
    }
  goto ret0;

 L4610: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 557;
    }
  goto ret0;

 L4619: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4620;
    }
  goto ret0;

 L4620: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4621;
    }
  goto ret0;

 L4621: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 558;
    }
  goto ret0;

 L4598: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[2] = x3;
      goto L4599;
    }
  goto ret0;

 L4599: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 556;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_15 PARAMS ((rtx, rtx, int *));
static int
recog_15 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case SImode:
      goto L6421;
    case SFmode:
      goto L6422;
    case DFmode:
      goto L6423;
    case XFmode:
      goto L6424;
    case DImode:
      goto L6425;
    case HImode:
      goto L6426;
    case QImode:
      goto L6430;
    default:
      break;
    }
  goto ret0;

 L6421: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L4672;
    }
 L6427: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L4990;
    }
  goto ret0;

 L4672: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L6434;
  x2 = XEXP (x1, 0);
  goto L6427;

 L6434: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case NEG:
      goto L4673;
    case NOT:
      goto L4802;
    case FIX:
      goto L4838;
    case ZERO_EXTEND:
      goto L4919;
    case SIGN_EXTEND:
      goto L4937;
    case ROTATE:
      goto L5017;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4673: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4674;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4674: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 563;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6427;

 L4802: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4803;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4803: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 577;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6427;

 L4838: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  switch (GET_MODE (x3))
    {
    case SFmode:
      goto L6440;
    case DFmode:
      goto L6441;
    case XFmode:
      goto L6442;
    default:
      break;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L6440: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4839;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4839: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 581;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6427;

 L6441: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4848;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4848: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 582;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6427;

 L6442: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4857;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4857: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 583;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6427;

 L4919: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  switch (GET_MODE (x3))
    {
    case HImode:
      goto L6443;
    case QImode:
      goto L6444;
    default:
      break;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L6443: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x3, HImode))
    {
      operands[1] = x3;
      goto L4920;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4920: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_arch4)))
    {
      return 590;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6427;

 L6444: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x3, QImode))
    {
      operands[1] = x3;
      goto L4929;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4929: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 591;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6427;

 L4937: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  switch (GET_MODE (x3))
    {
    case HImode:
      goto L6445;
    case QImode:
      goto L6446;
    default:
      break;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L6445: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x3, HImode))
    {
      operands[1] = x3;
      goto L4938;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4938: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_arch4)))
    {
      return 592;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6427;

 L6446: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x3, QImode))
    {
      operands[1] = x3;
      goto L4956;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4956: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_arch4)))
    {
      return 594;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6427;

 L5017: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6447;
  x2 = XEXP (x1, 0);
  goto L6427;

 L6447: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case MEM:
      goto L6449;
    case SUBREG:
      goto L6450;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L6427;

 L6449: ATTRIBUTE_UNUSED_LABEL
  if (offsettable_memory_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L5018;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L5018: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 16
      && ((TARGET_ARM) && (TARGET_ARM && (!TARGET_MMU_TRAPS))))
    {
      return 600;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L6450: ATTRIBUTE_UNUSED_LABEL
  if (XINT (x3, 1) == 0)
    goto L5051;
  x2 = XEXP (x1, 0);
  goto L6427;

 L5051: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (memory_operand (x4, HImode))
    {
      operands[1] = x4;
      goto L5052;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L5052: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 16
      && ((TARGET_ARM) && (TARGET_ARM
   && BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS)))
    {
      return 604;
    }
  x2 = XEXP (x1, 0);
  goto L6427;

 L4990: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (general_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L4991;
    }
  goto ret0;

 L4991: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM
   && (   register_operand (operands[0], SImode)
       || register_operand (operands[1], SImode)))))
    {
      return 598;
    }
  goto ret0;

 L6422: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SFmode))
    {
      operands[0] = x2;
      goto L4681;
    }
 L6431: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x2, SFmode))
    {
      operands[0] = x2;
      goto L5075;
    }
  goto ret0;

 L4681: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SFmode)
    goto L6451;
  x2 = XEXP (x1, 0);
  goto L6431;

 L6451: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case NEG:
      goto L4682;
    case ABS:
      goto L4719;
    case SQRT:
      goto L4756;
    case FLOAT:
      goto L4811;
    case FLOAT_TRUNCATE:
      goto L4865;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L6431;

 L4682: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4683;
    }
  x2 = XEXP (x1, 0);
  goto L6431;

 L4683: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 564;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6431;

 L4719: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4720;
    }
  x2 = XEXP (x1, 0);
  goto L6431;

 L4720: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 568;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6431;

 L4756: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4757;
    }
  x2 = XEXP (x1, 0);
  goto L6431;

 L4757: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 572;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6431;

 L4811: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4812;
    }
  x2 = XEXP (x1, 0);
  goto L6431;

 L4812: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 578;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6431;

 L4865: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  switch (GET_MODE (x3))
    {
    case DFmode:
      goto L6456;
    case XFmode:
      goto L6457;
    default:
      break;
    }
  x2 = XEXP (x1, 0);
  goto L6431;

 L6456: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4866;
    }
  x2 = XEXP (x1, 0);
  goto L6431;

 L4866: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 584;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6431;

 L6457: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4875;
    }
  x2 = XEXP (x1, 0);
  goto L6431;

 L4875: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 585;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6431;

 L5075: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (general_operand (x2, SFmode))
    {
      operands[1] = x2;
      goto L5076;
    }
  goto ret0;

 L5076: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM
   && TARGET_HARD_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], SFmode)))))
    {
      return 607;
    }
 L5084: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM
   && TARGET_SOFT_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], SFmode)))))
    {
      return 608;
    }
  goto ret0;

 L6423: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DFmode))
    {
      operands[0] = x2;
      goto L4690;
    }
 L6432: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x2, DFmode))
    {
      operands[0] = x2;
      goto L5091;
    }
  goto ret0;

 L4690: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DFmode)
    goto L6458;
  x2 = XEXP (x1, 0);
  goto L6432;

 L6458: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case NEG:
      goto L4691;
    case ABS:
      goto L4728;
    case SQRT:
      goto L4765;
    case FLOAT:
      goto L4820;
    case FLOAT_TRUNCATE:
      goto L4883;
    case FLOAT_EXTEND:
      goto L4964;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4691: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DFmode)
    goto L6465;
  x2 = XEXP (x1, 0);
  goto L6432;

 L6465: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4701;
  if (s_register_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4692;
    }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4701: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[1] = x4;
      goto L4702;
    }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4702: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 566;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6432;

 L4692: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 565;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6432;

 L4728: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DFmode)
    goto L6467;
  x2 = XEXP (x1, 0);
  goto L6432;

 L6467: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4738;
  if (s_register_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4729;
    }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4738: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[1] = x4;
      goto L4739;
    }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4739: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 570;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6432;

 L4729: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 569;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6432;

 L4765: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DFmode)
    goto L6469;
  x2 = XEXP (x1, 0);
  goto L6432;

 L6469: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == FLOAT_EXTEND)
    goto L4775;
  if (s_register_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4766;
    }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4775: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SFmode))
    {
      operands[1] = x4;
      goto L4776;
    }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4776: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 574;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6432;

 L4766: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 573;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6432;

 L4820: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4821;
    }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4821: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 579;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6432;

 L4883: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4884;
    }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4884: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 586;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6432;

 L4964: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4965;
    }
  x2 = XEXP (x1, 0);
  goto L6432;

 L4965: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT)))
    {
      return 595;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6432;

 L5091: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (general_operand (x2, DFmode))
    {
      operands[1] = x2;
      goto L5092;
    }
  goto ret0;

 L5092: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM
   && TARGET_HARD_FLOAT
   && (GET_CODE (operands[0]) != MEM
       || register_operand (operands[1], DFmode)))))
    {
      return 609;
    }
  goto ret0;

 L6424: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, XFmode))
    {
      operands[0] = x2;
      goto L4709;
    }
 L6433: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x2, XFmode))
    {
      operands[0] = x2;
      goto L5099;
    }
  goto ret0;

 L4709: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == XFmode)
    goto L6470;
  x2 = XEXP (x1, 0);
  goto L6433;

 L6470: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case NEG:
      goto L4710;
    case ABS:
      goto L4747;
    case SQRT:
      goto L4784;
    case FLOAT:
      goto L4829;
    case FLOAT_EXTEND:
      goto L4973;
    default:
     break;
   }
  x2 = XEXP (x1, 0);
  goto L6433;

 L4710: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4711;
    }
  x2 = XEXP (x1, 0);
  goto L6433;

 L4711: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 567;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6433;

 L4747: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4748;
    }
  x2 = XEXP (x1, 0);
  goto L6433;

 L4748: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 571;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6433;

 L4784: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, XFmode))
    {
      operands[1] = x3;
      goto L4785;
    }
  x2 = XEXP (x1, 0);
  goto L6433;

 L4785: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 575;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6433;

 L4829: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4830;
    }
  x2 = XEXP (x1, 0);
  goto L6433;

 L4830: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 580;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6433;

 L4973: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  switch (GET_MODE (x3))
    {
    case SFmode:
      goto L6475;
    case DFmode:
      goto L6476;
    default:
      break;
    }
  x2 = XEXP (x1, 0);
  goto L6433;

 L6475: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SFmode))
    {
      operands[1] = x3;
      goto L4974;
    }
  x2 = XEXP (x1, 0);
  goto L6433;

 L4974: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 596;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6433;

 L6476: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DFmode))
    {
      operands[1] = x3;
      goto L4983;
    }
  x2 = XEXP (x1, 0);
  goto L6433;

 L4983: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)))
    {
      return 597;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6433;

 L5099: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (general_operand (x2, XFmode))
    {
      operands[1] = x2;
      goto L5100;
    }
  goto ret0;

 L5100: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_HARD_FLOAT && (ENABLE_XF_PATTERNS || reload_completed))))
    {
      return 610;
    }
  goto ret0;

 L6425: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[0] = x2;
      goto L4792;
    }
  goto ret0;

 L4792: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode)
    goto L6477;
  goto ret0;

 L6477: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case NOT:
      goto L4793;
    case ZERO_EXTEND:
      goto L4892;
    case SIGN_EXTEND:
      goto L4910;
    default:
     break;
   }
  goto ret0;

 L4793: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4794;
    }
  goto ret0;

 L4794: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 576;
    }
  goto ret0;

 L4892: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  switch (GET_MODE (x3))
    {
    case SImode:
      goto L6480;
    case QImode:
      goto L6481;
    default:
      break;
    }
  goto ret0;

 L6480: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4893;
    }
  goto ret0;

 L4893: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 587;
    }
  goto ret0;

 L6481: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x3, QImode))
    {
      operands[1] = x3;
      goto L4902;
    }
  goto ret0;

 L4902: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 588;
    }
  goto ret0;

 L4910: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4911;
    }
  goto ret0;

 L4911: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 589;
    }
  goto ret0;

 L6426: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, HImode))
    {
      operands[0] = x2;
      goto L4945;
    }
 L6428: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x2, HImode))
    {
      operands[0] = x2;
      goto L5025;
    }
 L6429: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, HImode))
    {
      operands[0] = x2;
      goto L5033;
    }
  goto ret0;

 L4945: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == HImode
      && GET_CODE (x2) == SIGN_EXTEND)
    goto L4946;
  x2 = XEXP (x1, 0);
  goto L6428;

 L4946: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (memory_operand (x3, QImode))
    {
      operands[1] = x3;
      goto L4947;
    }
  x2 = XEXP (x1, 0);
  goto L6428;

 L4947: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_arch4)))
    {
      return 593;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6428;

 L5025: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (general_operand (x2, HImode))
    {
      operands[1] = x2;
      goto L5026;
    }
  x2 = XEXP (x1, 0);
  goto L6429;

 L5026: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM
   && arm_arch4
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1]))))))
    {
      return 601;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 0);
  goto L6429;

 L5033: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (general_operand (x2, HImode))
    {
      operands[1] = x2;
      goto L5034;
    }
 L5059: ATTRIBUTE_UNUSED_LABEL
  if (arm_rhs_operand (x2, HImode))
    {
      operands[1] = x2;
      goto L5060;
    }
  goto ret0;

 L5034: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM
   && !arm_arch4
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1]))))))
    {
      return 602;
    }
 L5042: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM
   && !arm_arch4
   && BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && (GET_CODE (operands[1]) != CONST_INT
       || const_ok_for_arm (INTVAL (operands[1]))
       || const_ok_for_arm (~INTVAL (operands[1]))))))
    {
      return 603;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L5059;

 L5060: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && TARGET_MMU_TRAPS)))
    {
      return 605;
    }
  goto ret0;

 L6430: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x2, QImode))
    {
      operands[0] = x2;
      goto L5067;
    }
  goto ret0;

 L5067: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (general_operand (x2, QImode))
    {
      operands[1] = x2;
      goto L5068;
    }
  goto ret0;

 L5068: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM
   && (   register_operand (operands[0], QImode)
       || register_operand (operands[1], QImode)))))
    {
      return 606;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_16 PARAMS ((rtx, rtx, int *));
static int
recog_16 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 1);
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  switch (GET_MODE (x3))
    {
    case QImode:
      goto L6487;
    case SImode:
      goto L6489;
    case HImode:
      goto L6491;
    default:
      break;
    }
  goto ret0;

 L6487: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == MEM)
    goto L5657;
  if (s_register_operand (x3, QImode))
    {
      operands[5] = x3;
      goto L5697;
    }
  goto ret0;

 L5657: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6492;
  goto ret0;

 L6492: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5658;
    case MINUS:
      goto L5678;
    default:
     break;
   }
  goto ret0;

 L5658: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5659;
    }
  goto ret0;

 L5659: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5660;
    }
  goto ret0;

 L5660: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5661;
    }
  goto ret0;

 L5661: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5662;
    }
  goto ret0;

 L5662: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, QImode))
    {
      operands[5] = x3;
      goto L5663;
    }
  goto ret0;

 L5663: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5664;
  goto ret0;

 L5664: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5665;
    }
  goto ret0;

 L5665: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5666;
  goto ret0;

 L5666: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5667;
    }
  goto ret0;

 L5667: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5668;
  goto ret0;

 L5668: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4]))
    goto L5669;
  goto ret0;

 L5669: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[1])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 642;
    }
  goto ret0;

 L5678: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5679;
    }
  goto ret0;

 L5679: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5680;
    }
  goto ret0;

 L5680: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5681;
    }
  goto ret0;

 L5681: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5682;
    }
  goto ret0;

 L5682: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, QImode))
    {
      operands[5] = x3;
      goto L5683;
    }
  goto ret0;

 L5683: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5684;
  goto ret0;

 L5684: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5685;
    }
  goto ret0;

 L5685: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5686;
  goto ret0;

 L5686: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5687;
  goto ret0;

 L5687: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5688;
    }
  goto ret0;

 L5688: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5689;
  goto ret0;

 L5689: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 643;
    }
  goto ret0;

 L5697: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == QImode
      && GET_CODE (x3) == MEM)
    goto L5698;
  goto ret0;

 L5698: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6494;
  goto ret0;

 L6494: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5699;
    case MINUS:
      goto L5719;
    default:
     break;
   }
  goto ret0;

 L5699: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5700;
    }
  goto ret0;

 L5700: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5701;
    }
  goto ret0;

 L5701: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5702;
    }
  goto ret0;

 L5702: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5703;
    }
  goto ret0;

 L5703: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5704;
  goto ret0;

 L5704: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5705;
    }
  goto ret0;

 L5705: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5706;
  goto ret0;

 L5706: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5707;
    }
  goto ret0;

 L5707: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5708;
  goto ret0;

 L5708: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4]))
    goto L5709;
  goto ret0;

 L5709: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[1])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 644;
    }
  goto ret0;

 L5719: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5720;
    }
  goto ret0;

 L5720: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5721;
    }
  goto ret0;

 L5721: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5722;
    }
  goto ret0;

 L5722: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5723;
    }
  goto ret0;

 L5723: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5724;
  goto ret0;

 L5724: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5725;
    }
  goto ret0;

 L5725: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5726;
  goto ret0;

 L5726: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5727;
  goto ret0;

 L5727: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5728;
    }
  goto ret0;

 L5728: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5729;
  goto ret0;

 L5729: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 645;
    }
  goto ret0;

 L6489: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == MEM)
    goto L5737;
  if (s_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L5777;
    }
  goto ret0;

 L5737: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6496;
  goto ret0;

 L6496: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5738;
    case MINUS:
      goto L5758;
    default:
     break;
   }
  goto ret0;

 L5738: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5739;
    }
  goto ret0;

 L5739: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5740;
    }
  goto ret0;

 L5740: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5741;
    }
  goto ret0;

 L5741: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5742;
    }
  goto ret0;

 L5742: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L5743;
    }
  goto ret0;

 L5743: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5744;
  goto ret0;

 L5744: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5745;
    }
  goto ret0;

 L5745: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5746;
  goto ret0;

 L5746: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5747;
    }
  goto ret0;

 L5747: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5748;
  goto ret0;

 L5748: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4]))
    goto L5749;
  goto ret0;

 L5749: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[1])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 646;
    }
  goto ret0;

 L5758: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5759;
    }
  goto ret0;

 L5759: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5760;
    }
  goto ret0;

 L5760: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5761;
    }
  goto ret0;

 L5761: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5762;
    }
  goto ret0;

 L5762: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L5763;
    }
  goto ret0;

 L5763: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5764;
  goto ret0;

 L5764: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5765;
    }
  goto ret0;

 L5765: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5766;
  goto ret0;

 L5766: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5767;
  goto ret0;

 L5767: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5768;
    }
  goto ret0;

 L5768: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5769;
  goto ret0;

 L5769: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 647;
    }
  goto ret0;

 L5777: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5778;
  goto ret0;

 L5778: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6498;
  goto ret0;

 L6498: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5779;
    case MINUS:
      goto L5799;
    default:
     break;
   }
  goto ret0;

 L5779: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5780;
    }
  goto ret0;

 L5780: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5781;
    }
  goto ret0;

 L5781: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5782;
    }
  goto ret0;

 L5782: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5783;
    }
  goto ret0;

 L5783: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5784;
  goto ret0;

 L5784: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5785;
    }
  goto ret0;

 L5785: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5786;
  goto ret0;

 L5786: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5787;
    }
  goto ret0;

 L5787: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5788;
  goto ret0;

 L5788: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4]))
    goto L5789;
  goto ret0;

 L5789: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[1])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 648;
    }
  goto ret0;

 L5799: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5800;
    }
  goto ret0;

 L5800: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5801;
    }
  goto ret0;

 L5801: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5802;
    }
  goto ret0;

 L5802: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5803;
    }
  goto ret0;

 L5803: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5804;
  goto ret0;

 L5804: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5805;
    }
  goto ret0;

 L5805: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5806;
  goto ret0;

 L5806: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5807;
  goto ret0;

 L5807: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5808;
    }
  goto ret0;

 L5808: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5809;
  goto ret0;

 L5809: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 649;
    }
  goto ret0;

 L6491: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, HImode))
    {
      operands[5] = x3;
      goto L5817;
    }
  goto ret0;

 L5817: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == HImode
      && GET_CODE (x3) == MEM)
    goto L5818;
  goto ret0;

 L5818: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6500;
  goto ret0;

 L6500: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5819;
    case MINUS:
      goto L5839;
    default:
     break;
   }
  goto ret0;

 L5819: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5820;
    }
  goto ret0;

 L5820: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5821;
    }
  goto ret0;

 L5821: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5822;
    }
  goto ret0;

 L5822: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5823;
    }
  goto ret0;

 L5823: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5824;
  goto ret0;

 L5824: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5825;
    }
  goto ret0;

 L5825: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5826;
  goto ret0;

 L5826: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5827;
    }
  goto ret0;

 L5827: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5828;
  goto ret0;

 L5828: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4]))
    goto L5829;
  goto ret0;

 L5829: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[1])
      && ((TARGET_ARM) && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 650;
    }
  goto ret0;

 L5839: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5840;
    }
  goto ret0;

 L5840: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (shift_operator (x5, SImode))
    {
      operands[2] = x5;
      goto L5841;
    }
  goto ret0;

 L5841: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[3] = x6;
      goto L5842;
    }
  goto ret0;

 L5842: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (const_shift_operand (x6, SImode))
    {
      operands[4] = x6;
      goto L5843;
    }
  goto ret0;

 L5843: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5844;
  goto ret0;

 L5844: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5845;
    }
  goto ret0;

 L5845: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5846;
  goto ret0;

 L5846: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5847;
  goto ret0;

 L5847: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2]))
    {
      operands[2] = x4;
      goto L5848;
    }
  goto ret0;

 L5848: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[3]))
    goto L5849;
  goto ret0;

 L5849: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (rtx_equal_p (x5, operands[4])
      && ((TARGET_ARM) && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && REGNO (operands[3]) != FRAME_POINTER_REGNUM)))
    {
      return 651;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_17 PARAMS ((rtx, rtx, int *));
static int
recog_17 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 1);
  switch (GET_CODE (x1))
    {
    case PARALLEL:
      goto L6486;
    case SET:
      goto L5424;
    default:
     break;
   }
  goto ret0;

 L6486: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) == 2)
    goto L5655;
  if (XVECLEN (x1, 0) >= 4
      && load_multiple_operation (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5138;
    }
 L6485: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) >= 4
      && store_multiple_operation (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5280;
    }
  goto ret0;

 L5655: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5656;
  goto ret0;

 L5656: ATTRIBUTE_UNUSED_LABEL
  return recog_16 (x0, insn, pnum_clobbers);

 L5138: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5139;
  goto L6485;

 L5139: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6503;
  goto L6485;

 L6503: ATTRIBUTE_UNUSED_LABEL
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L5186;
    }
 L6502: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L5140;
    }
  goto L6485;

 L5186: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5187;
  x3 = XEXP (x2, 0);
  goto L6502;

 L5187: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L5188;
    }
  x3 = XEXP (x2, 0);
  goto L6502;

 L5188: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5189;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5189: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5190;
    }
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5190: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5191;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5191: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5192;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5192: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5193;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5193: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5194;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5194: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5195;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5195: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5196;
    }
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5196: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5197;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5197: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5198;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5198: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5199;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5199: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 8)
    goto L5200;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5200: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 3);
  if (GET_CODE (x2) == SET)
    goto L5201;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5201: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L5202;
    }
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5202: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5203;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5203: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5204;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5204: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5205;
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5205: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 12
      && ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 4)))
    {
      return 614;
    }
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  goto L6502;

 L5140: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5141;
  goto L6485;

 L5141: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L5142;
    }
  goto L6485;

 L5142: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 12)
    goto L5143;
  goto L6485;

 L5143: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5144;
  goto L6485;

 L5144: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5145;
    }
  goto L6485;

 L5145: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5146;
  goto L6485;

 L5146: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L5147;
  goto L6485;

 L5147: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5148;
  goto L6485;

 L5148: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5149;
    }
  goto L6485;

 L5149: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5150;
  goto L6485;

 L5150: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5151;
  goto L6485;

 L5151: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5152;
  goto L6485;

 L5152: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5153;
  goto L6485;

 L5153: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 3);
  if (GET_CODE (x2) == SET)
    goto L5154;
  goto L6485;

 L5154: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L5155;
    }
  goto L6485;

 L5155: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5156;
  goto L6485;

 L5156: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5157;
  goto L6485;

 L5157: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5158;
  goto L6485;

 L5158: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 8
      && ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 4)))
    {
      return 612;
    }
  goto L6485;

 L5280: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5281;
  goto ret0;

 L5281: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6505;
  goto ret0;

 L6505: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == MEM)
    goto L5330;
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L5282;
    }
  goto ret0;

 L5330: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L5331;
    }
  goto ret0;

 L5331: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L5332;
    }
  goto ret0;

 L5332: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5333;
  goto ret0;

 L5333: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5334;
  goto ret0;

 L5334: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5335;
  goto ret0;

 L5335: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5336;
  goto ret0;

 L5336: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5337;
  goto ret0;

 L5337: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5338;
    }
  goto ret0;

 L5338: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5339;
  goto ret0;

 L5339: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5340;
  goto ret0;

 L5340: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5341;
  goto ret0;

 L5341: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5342;
  goto ret0;

 L5342: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 8)
    goto L5343;
  goto ret0;

 L5343: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5344;
    }
  goto ret0;

 L5344: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 3);
  if (GET_CODE (x2) == SET)
    goto L5345;
  goto ret0;

 L5345: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5346;
  goto ret0;

 L5346: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5347;
  goto ret0;

 L5347: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5348;
  goto ret0;

 L5348: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 12)
    goto L5349;
  goto ret0;

 L5349: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L5350;
    }
  goto ret0;

 L5350: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 4)))
    {
      return 620;
    }
  goto ret0;

 L5282: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5283;
  goto ret0;

 L5283: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L5284;
    }
  goto ret0;

 L5284: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 12)
    goto L5285;
  goto ret0;

 L5285: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5286;
  goto ret0;

 L5286: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5287;
  goto ret0;

 L5287: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L5288;
  goto ret0;

 L5288: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5289;
    }
  goto ret0;

 L5289: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5290;
  goto ret0;

 L5290: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5291;
  goto ret0;

 L5291: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5292;
  goto ret0;

 L5292: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5293;
  goto ret0;

 L5293: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5294;
  goto ret0;

 L5294: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5295;
    }
  goto ret0;

 L5295: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 3);
  if (GET_CODE (x2) == SET)
    goto L5296;
  goto ret0;

 L5296: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5297;
  goto ret0;

 L5297: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5298;
  goto ret0;

 L5298: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5299;
  goto ret0;

 L5299: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 8)
    goto L5300;
  goto ret0;

 L5300: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L5301;
    }
  goto ret0;

 L5301: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 4)))
    {
      return 618;
    }
  goto ret0;

 L5424: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L5425;
    }
  goto ret0;

 L5425: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (shiftable_operator (x2, SImode))
    {
      operands[1] = x2;
      goto L5426;
    }
  goto ret0;

 L5426: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (shift_operator (x3, SImode))
    {
      operands[3] = x3;
      goto L5427;
    }
  goto ret0;

 L5427: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L5428;
    }
  goto ret0;

 L5428: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (reg_or_int_operand (x4, SImode))
    {
      operands[5] = x4;
      goto L5429;
    }
  goto ret0;

 L5429: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L5430;
    }
  goto ret0;

 L5430: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 627;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_18 PARAMS ((rtx, rtx, int *));
static int
recog_18 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 1);
  x2 = XVECEXP (x1, 0, 0);
  x3 = XEXP (x2, 0);
  switch (GET_MODE (x3))
    {
    case QImode:
      goto L6514;
    case SImode:
      goto L6517;
    case HImode:
      goto L6518;
    default:
      break;
    }
  goto ret0;

 L6514: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == MEM)
    goto L5463;
  if (s_register_operand (x3, QImode))
    {
      operands[3] = x3;
      goto L5495;
    }
  goto ret0;

 L5463: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6519;
  goto ret0;

 L6519: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5464;
    case MINUS:
      goto L5480;
    default:
     break;
   }
  goto ret0;

 L5464: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5465;
    }
  goto ret0;

 L5465: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (index_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5466;
    }
  goto ret0;

 L5466: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, QImode))
    {
      operands[3] = x3;
      goto L5467;
    }
  goto ret0;

 L5467: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5468;
  goto ret0;

 L5468: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5469;
    }
  goto ret0;

 L5469: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5470;
  goto ret0;

 L5470: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5471;
  goto ret0;

 L5471: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 630;
    }
  goto ret0;

 L5480: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5481;
    }
  goto ret0;

 L5481: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5482;
    }
  goto ret0;

 L5482: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, QImode))
    {
      operands[3] = x3;
      goto L5483;
    }
  goto ret0;

 L5483: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5484;
  goto ret0;

 L5484: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5485;
    }
  goto ret0;

 L5485: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5486;
  goto ret0;

 L5486: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5487;
  goto ret0;

 L5487: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 631;
    }
  goto ret0;

 L5495: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == QImode
      && GET_CODE (x3) == MEM)
    goto L5496;
  goto ret0;

 L5496: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6521;
  goto ret0;

 L6521: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5497;
    case MINUS:
      goto L5513;
    default:
     break;
   }
  goto ret0;

 L5497: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5498;
    }
  goto ret0;

 L5498: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (index_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5499;
    }
  goto ret0;

 L5499: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5500;
  goto ret0;

 L5500: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5501;
    }
  goto ret0;

 L5501: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5502;
  goto ret0;

 L5502: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5503;
  goto ret0;

 L5503: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 632;
    }
  goto ret0;

 L5513: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5514;
    }
  goto ret0;

 L5514: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5515;
    }
  goto ret0;

 L5515: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5516;
  goto ret0;

 L5516: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5517;
    }
  goto ret0;

 L5517: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5518;
  goto ret0;

 L5518: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5519;
  goto ret0;

 L5519: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 633;
    }
  goto ret0;

 L6517: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == MEM)
    goto L5561;
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5527;
    }
  goto ret0;

 L5561: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6523;
  goto ret0;

 L6523: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5562;
    case MINUS:
      goto L5578;
    default:
     break;
   }
  goto ret0;

 L5562: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5563;
    }
  goto ret0;

 L5563: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (index_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5564;
    }
  goto ret0;

 L5564: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5565;
    }
  goto ret0;

 L5565: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5566;
  goto ret0;

 L5566: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5567;
    }
  goto ret0;

 L5567: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5568;
  goto ret0;

 L5568: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5569;
  goto ret0;

 L5569: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 636;
    }
  goto ret0;

 L5578: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5579;
    }
  goto ret0;

 L5579: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5580;
    }
  goto ret0;

 L5580: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5581;
    }
  goto ret0;

 L5581: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5582;
  goto ret0;

 L5582: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5583;
    }
  goto ret0;

 L5583: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5584;
  goto ret0;

 L5584: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5585;
  goto ret0;

 L5585: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 637;
    }
  goto ret0;

 L5527: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode)
    goto L6525;
  goto ret0;

 L6525: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case ZERO_EXTEND:
      goto L5528;
    case MEM:
      goto L5594;
    default:
     break;
   }
  goto ret0;

 L5528: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == QImode
      && GET_CODE (x4) == MEM)
    goto L5529;
  goto ret0;

 L5529: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (GET_MODE (x5) == SImode)
    goto L6527;
  goto ret0;

 L6527: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x5))
    {
    case PLUS:
      goto L5530;
    case MINUS:
      goto L5547;
    default:
     break;
   }
  goto ret0;

 L5530: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[1] = x6;
      goto L5531;
    }
  goto ret0;

 L5531: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (index_operand (x6, SImode))
    {
      operands[2] = x6;
      goto L5532;
    }
  goto ret0;

 L5532: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5533;
  goto ret0;

 L5533: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5534;
    }
  goto ret0;

 L5534: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5535;
  goto ret0;

 L5535: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5536;
  goto ret0;

 L5536: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 634;
    }
  goto ret0;

 L5547: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (s_register_operand (x6, SImode))
    {
      operands[1] = x6;
      goto L5548;
    }
  goto ret0;

 L5548: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (s_register_operand (x6, SImode))
    {
      operands[2] = x6;
      goto L5549;
    }
  goto ret0;

 L5549: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5550;
  goto ret0;

 L5550: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5551;
    }
  goto ret0;

 L5551: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5552;
  goto ret0;

 L5552: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5553;
  goto ret0;

 L5553: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 635;
    }
  goto ret0;

 L5594: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6529;
  goto ret0;

 L6529: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5595;
    case MINUS:
      goto L5611;
    default:
     break;
   }
  goto ret0;

 L5595: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5596;
    }
  goto ret0;

 L5596: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (index_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5597;
    }
  goto ret0;

 L5597: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5598;
  goto ret0;

 L5598: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5599;
    }
  goto ret0;

 L5599: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5600;
  goto ret0;

 L5600: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5601;
  goto ret0;

 L5601: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 638;
    }
  goto ret0;

 L5611: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5612;
    }
  goto ret0;

 L5612: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5613;
    }
  goto ret0;

 L5613: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5614;
  goto ret0;

 L5614: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5615;
    }
  goto ret0;

 L5615: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5616;
  goto ret0;

 L5616: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5617;
  goto ret0;

 L5617: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 639;
    }
  goto ret0;

 L6518: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, HImode))
    {
      operands[3] = x3;
      goto L5625;
    }
  goto ret0;

 L5625: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == HImode
      && GET_CODE (x3) == MEM)
    goto L5626;
  goto ret0;

 L5626: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode)
    goto L6531;
  goto ret0;

 L6531: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case PLUS:
      goto L5627;
    case MINUS:
      goto L5643;
    default:
     break;
   }
  goto ret0;

 L5627: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5628;
    }
  goto ret0;

 L5628: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (index_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5629;
    }
  goto ret0;

 L5629: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5630;
  goto ret0;

 L5630: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5631;
    }
  goto ret0;

 L5631: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5632;
  goto ret0;

 L5632: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5633;
  goto ret0;

 L5633: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 640;
    }
  goto ret0;

 L5643: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L5644;
    }
  goto ret0;

 L5644: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L5645;
    }
  goto ret0;

 L5645: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5646;
  goto ret0;

 L5646: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5647;
    }
  goto ret0;

 L5647: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MINUS)
    goto L5648;
  goto ret0;

 L5648: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[1]))
    goto L5649;
  goto ret0;

 L5649: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (rtx_equal_p (x4, operands[2])
      && ((TARGET_ARM) && (TARGET_ARM
   && !BYTES_BIG_ENDIAN
   && !TARGET_MMU_TRAPS
   && !arm_arch4
   && REGNO (operands[0]) != FRAME_POINTER_REGNUM
   && REGNO (operands[1]) != FRAME_POINTER_REGNUM
   && (GET_CODE (operands[2]) != REG
       || REGNO (operands[2]) != FRAME_POINTER_REGNUM))))
    {
      return 641;
    }
  goto ret0;
 ret0:
  return -1;
}

static int recog_19 PARAMS ((rtx, rtx, int *));
static int
recog_19 (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;

  x1 = XEXP (x0, 0);
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[3] = x1;
      goto L4041;
    }
 L4188: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[4] = x1;
      goto L4189;
    }
 L4536: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[5] = x1;
      goto L4537;
    }
 L4667: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[2] = x1;
      goto L4668;
    }
 L5102: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[7] = x1;
      goto L5103;
    }
 L5134: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[6] = x1;
      goto L5135;
    }
 L5207: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[5] = x1;
      goto L5208;
    }
 L5228: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[4] = x1;
      goto L5229;
    }
 L5390: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[1] = x1;
      goto L5391;
    }
 L5399: ATTRIBUTE_UNUSED_LABEL
  if (arm_comparison_operator (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5400;
    }
  goto ret0;

 L4041: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[4] = x2;
      goto L4042;
    }
  goto L4188;

 L4042: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L4043;
  goto L4188;

 L4043: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_CODE (x1) == SET)
    goto L4044;
  x1 = XEXP (x0, 0);
  goto L4188;

 L4044: ATTRIBUTE_UNUSED_LABEL
  tem = recog_14 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  x1 = XEXP (x0, 0);
  goto L4188;

 L4189: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[5] = x2;
      goto L4190;
    }
  goto L4536;

 L4190: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L4191;
  goto L4536;

 L4191: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L4192;
    case PARALLEL:
      goto L6408;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4192: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case SImode:
      goto L6409;
    case DImode:
      goto L6410;
    default:
      break;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L6409: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L4193;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4193: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L6411;
  x1 = XEXP (x0, 0);
  goto L4536;

 L6411: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case PLUS:
      goto L4194;
    case AND:
      goto L4639;
    case NOT:
      goto L4662;
    case ASHIFT:
    case ASHIFTRT:
    case LSHIFTRT:
    case ROTATERT:
    case MULT:
      goto L6413;
    default:
      x1 = XEXP (x0, 0);
      goto L4536;
   }
 L6413: ATTRIBUTE_UNUSED_LABEL
  if (shift_operator (x2, SImode))
    {
      operands[3] = x2;
      goto L4652;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4194: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MULT)
    goto L4195;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4195: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4196;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4196: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L4197;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4197: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L4198;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4198: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 521;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4639: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == IOR)
    goto L4640;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4640: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L4641;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4641: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4642;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4642: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == NOT)
    goto L4643;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4643: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L4644;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4644: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 560;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4662: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (shift_operator (x3, SImode))
    {
      operands[3] = x3;
      goto L4663;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4663: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L4664;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4664: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_rhs_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4665;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4665: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 562;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4652: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4653;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4653: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (reg_or_int_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L4654;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4654: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 561;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L6410: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[0] = x2;
      goto L4205;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4205: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode
      && GET_CODE (x2) == PLUS)
    goto L4206;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4206: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode
      && GET_CODE (x3) == MULT)
    goto L4207;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4207: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == DImode)
    goto L6415;
  x1 = XEXP (x0, 0);
  goto L4536;

 L6415: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x4))
    {
    case SIGN_EXTEND:
      goto L4208;
    case ZERO_EXTEND:
      goto L4246;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4208: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L4209;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4209: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_MODE (x4) == DImode
      && GET_CODE (x4) == SIGN_EXTEND)
    goto L4210;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4210: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L4211;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4211: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4212;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4212: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_fast_multiply)))
    {
      return 522;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4246: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L4247;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4247: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_MODE (x4) == DImode
      && GET_CODE (x4) == ZERO_EXTEND)
    goto L4248;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4248: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L4249;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4249: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L4250;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4250: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_fast_multiply)))
    {
      return 525;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L6408: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) == 2)
    goto L4256;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4256: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L4257;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4257: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L4258;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4258: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == TRUNCATE)
    goto L4259;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4259: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == DImode
      && GET_CODE (x4) == LSHIFTRT)
    goto L4260;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4260: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (GET_MODE (x5) == DImode
      && GET_CODE (x5) == MULT)
    goto L4261;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4261: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (GET_MODE (x6) == DImode)
    goto L6417;
  x1 = XEXP (x0, 0);
  goto L4536;

 L6417: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x6))
    {
    case SIGN_EXTEND:
      goto L4262;
    case ZERO_EXTEND:
      goto L4280;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4262: ATTRIBUTE_UNUSED_LABEL
  x7 = XEXP (x6, 0);
  if (s_register_operand (x7, SImode))
    {
      operands[1] = x7;
      goto L4263;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4263: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (GET_MODE (x6) == DImode
      && GET_CODE (x6) == SIGN_EXTEND)
    goto L4264;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4264: ATTRIBUTE_UNUSED_LABEL
  x7 = XEXP (x6, 0);
  if (s_register_operand (x7, SImode))
    {
      operands[2] = x7;
      goto L4265;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4265: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 32)
    goto L4266;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4266: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == CLOBBER)
    goto L4267;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4267: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (scratch_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L4268;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4268: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_fast_multiply)))
    {
      return 526;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4280: ATTRIBUTE_UNUSED_LABEL
  x7 = XEXP (x6, 0);
  if (s_register_operand (x7, SImode))
    {
      operands[1] = x7;
      goto L4281;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4281: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 1);
  if (GET_MODE (x6) == DImode
      && GET_CODE (x6) == ZERO_EXTEND)
    goto L4282;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4282: ATTRIBUTE_UNUSED_LABEL
  x7 = XEXP (x6, 0);
  if (s_register_operand (x7, SImode))
    {
      operands[2] = x7;
      goto L4283;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4283: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 32)
    goto L4284;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4284: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == CLOBBER)
    goto L4285;
  x1 = XEXP (x0, 0);
  goto L4536;

 L4285: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (scratch_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L4286;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4286: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && arm_fast_multiply)))
    {
      return 527;
    }
  x1 = XEXP (x0, 0);
  goto L4536;

 L4537: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[6] = x2;
      goto L4538;
    }
  goto L4667;

 L4538: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L4539;
  goto L4667;

 L4539: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L4540;
    case PARALLEL:
      goto L6419;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L4667;

 L4540: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L4541;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L4541: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == AND)
    goto L4542;
  x1 = XEXP (x0, 0);
  goto L4667;

 L4542: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == NOT)
    goto L4543;
  x1 = XEXP (x0, 0);
  goto L4667;

 L4543: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (shift_operator (x4, SImode))
    {
      operands[4] = x4;
      goto L4544;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L4544: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L4545;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L4545: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (arm_rhs_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L4546;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L4546: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L4547;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L4547: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 551;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L6419: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) >= 3
      && load_multiple_operation (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5164;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L5164: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5165;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5165: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L5166;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L5166: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5167;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5167: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L5168;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L5168: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L5169;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5169: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5170;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5170: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5171;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L5171: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5172;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5172: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L5173;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5173: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5174;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5174: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5175;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L5175: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5176;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5176: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5177;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5177: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5178;
  x1 = XEXP (x0, 0);
  goto L4667;

 L5178: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4
      && ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 3)))
    {
      return 613;
    }
  x1 = XEXP (x0, 0);
  goto L4667;

 L4668: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[3] = x2;
      goto L4669;
    }
  goto L5102;

 L4669: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L4670;
  goto L5102;

 L4670: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  switch (GET_CODE (x1))
    {
    case SET:
      goto L4671;
    case PARALLEL:
      goto L6420;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L5102;

 L4671: ATTRIBUTE_UNUSED_LABEL
  tem = recog_15 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  x1 = XEXP (x0, 0);
  goto L5102;

 L6420: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) == 2)
    goto L4997;
  x1 = XEXP (x0, 0);
  goto L5102;

 L4997: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L4998;
  x1 = XEXP (x0, 0);
  goto L5102;

 L4998: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L4999;
    }
  x1 = XEXP (x0, 0);
  goto L5102;

 L4999: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == UNSPEC
      && XVECLEN (x3, 0) == 1
      && XINT (x3, 1) == 4)
    goto L5000;
  x1 = XEXP (x0, 0);
  goto L5102;

 L5000: ATTRIBUTE_UNUSED_LABEL
  x4 = XVECEXP (x3, 0, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5001;
  x1 = XEXP (x0, 0);
  goto L5102;

 L5001: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[0]))
    goto L5002;
  x1 = XEXP (x0, 0);
  goto L5102;

 L5002: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST)
    goto L5003;
  x1 = XEXP (x0, 0);
  goto L5102;

 L5003: ATTRIBUTE_UNUSED_LABEL
  x6 = XEXP (x5, 0);
  if (GET_MODE (x6) == SImode
      && GET_CODE (x6) == PLUS)
    goto L5004;
  x1 = XEXP (x0, 0);
  goto L5102;

 L5004: ATTRIBUTE_UNUSED_LABEL
  x7 = XEXP (x6, 0);
  if (GET_CODE (x7) == PC)
    goto L5005;
  x1 = XEXP (x0, 0);
  goto L5102;

 L5005: ATTRIBUTE_UNUSED_LABEL
  x7 = XEXP (x6, 1);
  if (GET_CODE (x7) == CONST_INT
      && XWINT (x7, 0) == 8)
    goto L5006;
  x1 = XEXP (x0, 0);
  goto L5102;

 L5006: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == USE)
    goto L5007;
  x1 = XEXP (x0, 0);
  goto L5102;

 L5007: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_CODE (x3) == LABEL_REF)
    goto L5008;
  x1 = XEXP (x0, 0);
  goto L5102;

 L5008: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  operands[1] = x4;
  goto L5009;

 L5009: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && flag_pic)))
    {
      return 599;
    }
  x1 = XEXP (x0, 0);
  goto L5102;

 L5103: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[8] = x2;
      goto L5104;
    }
  goto L5134;

 L5104: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L5105;
  goto L5134;

 L5105: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_CODE (x1) == PARALLEL)
    goto L6482;
  x1 = XEXP (x0, 0);
  goto L5134;

 L6482: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) >= 5
      && load_multiple_operation (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5106;
    }
 L6483: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) >= 5
      && store_multiple_operation (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5247;
    }
  x1 = XEXP (x0, 0);
  goto L5134;

 L5106: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5107;
  goto L6483;

 L5107: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L5108;
    }
  goto L6483;

 L5108: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5109;
  goto L6483;

 L5109: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L5110;
    }
  goto L6483;

 L5110: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 16)
    goto L5111;
  goto L6483;

 L5111: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5112;
  goto L6483;

 L5112: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5113;
    }
  goto L6483;

 L5113: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5114;
  goto L6483;

 L5114: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L5115;
  goto L6483;

 L5115: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5116;
  goto L6483;

 L5116: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5117;
    }
  goto L6483;

 L5117: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5118;
  goto L6483;

 L5118: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5119;
  goto L6483;

 L5119: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5120;
  goto L6483;

 L5120: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5121;
  goto L6483;

 L5121: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 3);
  if (GET_CODE (x2) == SET)
    goto L5122;
  goto L6483;

 L5122: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L5123;
    }
  goto L6483;

 L5123: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5124;
  goto L6483;

 L5124: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5125;
  goto L6483;

 L5125: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5126;
  goto L6483;

 L5126: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 8)
    goto L5127;
  goto L6483;

 L5127: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 4);
  if (GET_CODE (x2) == SET)
    goto L5128;
  goto L6483;

 L5128: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[6] = x3;
      goto L5129;
    }
  goto L6483;

 L5129: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5130;
  goto L6483;

 L5130: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5131;
  goto L6483;

 L5131: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5132;
  goto L6483;

 L5132: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 12
      && ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 5)))
    {
      return 611;
    }
  goto L6483;

 L5247: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5248;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5248: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L5249;
    }
  x1 = XEXP (x0, 0);
  goto L5134;

 L5249: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5250;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5250: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L5251;
    }
  x1 = XEXP (x0, 0);
  goto L5134;

 L5251: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 16)
    goto L5252;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5252: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5253;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5253: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5254;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5254: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L5255;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5255: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5256;
    }
  x1 = XEXP (x0, 0);
  goto L5134;

 L5256: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5257;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5257: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5258;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5258: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5259;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5259: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5260;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5260: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5261;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5261: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5262;
    }
  x1 = XEXP (x0, 0);
  goto L5134;

 L5262: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 3);
  if (GET_CODE (x2) == SET)
    goto L5263;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5263: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5264;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5264: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5265;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5265: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5266;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5266: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 8)
    goto L5267;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5267: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[5] = x3;
      goto L5268;
    }
  x1 = XEXP (x0, 0);
  goto L5134;

 L5268: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 4);
  if (GET_CODE (x2) == SET)
    goto L5269;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5269: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5270;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5270: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5271;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5271: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5272;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5272: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 12)
    goto L5273;
  x1 = XEXP (x0, 0);
  goto L5134;

 L5273: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[6] = x3;
      goto L5274;
    }
  x1 = XEXP (x0, 0);
  goto L5134;

 L5274: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 5)))
    {
      return 617;
    }
  x1 = XEXP (x0, 0);
  goto L5134;

 L5135: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[7] = x2;
      goto L5136;
    }
  goto L5207;

 L5136: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L5137;
  goto L5207;

 L5137: ATTRIBUTE_UNUSED_LABEL
  tem = recog_17 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  x1 = XEXP (x0, 0);
  goto L5207;

 L5208: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[6] = x2;
      goto L5209;
    }
  goto L5228;

 L5209: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L5210;
  goto L5228;

 L5210: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  switch (GET_CODE (x1))
    {
    case PARALLEL:
      goto L6508;
    case SET:
      goto L5436;
    default:
     break;
   }
  x1 = XEXP (x0, 0);
  goto L5228;

 L6508: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) == 2)
    goto L5448;
  if (XVECLEN (x1, 0) >= 3
      && load_multiple_operation (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5211;
    }
 L6507: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) >= 3
      && store_multiple_operation (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5307;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5448: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5449;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5449: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[0] = x3;
      goto L5450;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5450: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (shiftable_operator (x3, SImode))
    {
      operands[1] = x3;
      goto L5451;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5451: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (memory_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L5452;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5452: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (memory_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L5453;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5453: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == CLOBBER)
    goto L5454;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5454: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (scratch_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5455;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5455: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && adjacent_mem_locations (operands[2], operands[3]))))
    {
      return 629;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5211: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5212;
  goto L6507;

 L5212: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L5213;
    }
  goto L6507;

 L5213: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5214;
  goto L6507;

 L5214: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L5215;
    }
  goto L6507;

 L5215: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5216;
  goto L6507;

 L5216: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5217;
    }
  goto L6507;

 L5217: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5218;
  goto L6507;

 L5218: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5219;
  goto L6507;

 L5219: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5220;
  goto L6507;

 L5220: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5221;
  goto L6507;

 L5221: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5222;
  goto L6507;

 L5222: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5223;
    }
  goto L6507;

 L5223: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5224;
  goto L6507;

 L5224: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5225;
  goto L6507;

 L5225: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5226;
  goto L6507;

 L5226: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 8
      && ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 3)))
    {
      return 615;
    }
  goto L6507;

 L5307: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5308;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5308: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6510;
  x1 = XEXP (x0, 0);
  goto L5228;

 L6510: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == MEM)
    goto L5358;
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L5309;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5358: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L5359;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5359: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L5360;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5360: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5361;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5361: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5362;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5362: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5363;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5363: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5364;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5364: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5365;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5365: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5366;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5366: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5367;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5367: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5368;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5368: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5369;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5369: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5370;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5370: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 8)
    goto L5371;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5371: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5372;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5372: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 3)))
    {
      return 621;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5309: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == PLUS)
    goto L5310;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5310: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L5311;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5311: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 8)
    goto L5312;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5312: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5313;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5313: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5314;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5314: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (rtx_equal_p (x4, operands[2]))
    goto L5315;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5315: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5316;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5316: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 2);
  if (GET_CODE (x2) == SET)
    goto L5317;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5317: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5318;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5318: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5319;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5319: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[2]))
    goto L5320;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5320: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5321;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5321: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L5322;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5322: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 3)))
    {
      return 619;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5436: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L5437;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5437: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == MINUS)
    goto L5438;
  x1 = XEXP (x0, 0);
  goto L5228;

 L5438: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[1] = x3;
      goto L5439;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5439: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (shift_operator (x3, SImode))
    {
      operands[2] = x3;
      goto L5440;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5440: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L5441;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5441: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (reg_or_int_operand (x4, SImode))
    {
      operands[4] = x4;
      goto L5442;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5442: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 628;
    }
  x1 = XEXP (x0, 0);
  goto L5228;

 L5229: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[5] = x2;
      goto L5230;
    }
  goto L5390;

 L5230: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L5231;
  goto L5390;

 L5231: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_CODE (x1) == PARALLEL)
    goto L6513;
  x1 = XEXP (x0, 0);
  goto L5390;

 L6513: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) == 2)
    goto L5461;
 L6511: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) >= 2
      && load_multiple_operation (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5232;
    }
 L6512: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x1, 0) >= 2
      && store_multiple_operation (x1, VOIDmode))
    {
      operands[0] = x1;
      goto L5378;
    }
  x1 = XEXP (x0, 0);
  goto L5390;

 L5461: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5462;
  goto L6511;

 L5462: ATTRIBUTE_UNUSED_LABEL
  tem = recog_18 (x0, insn, pnum_clobbers);
  if (tem >= 0)
    return tem;
  goto L6511;

 L5232: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5233;
  goto L6512;

 L5233: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L5234;
    }
  goto L6512;

 L5234: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5235;
  goto L6512;

 L5235: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L5236;
    }
  goto L6512;

 L5236: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5237;
  goto L6512;

 L5237: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5238;
    }
  goto L6512;

 L5238: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5239;
  goto L6512;

 L5239: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5240;
  goto L6512;

 L5240: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5241;
  goto L6512;

 L5241: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4
      && ((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 2)))
    {
      return 616;
    }
  goto L6512;

 L5378: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 0);
  if (GET_CODE (x2) == SET)
    goto L5379;
  x1 = XEXP (x0, 0);
  goto L5390;

 L5379: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5380;
  x1 = XEXP (x0, 0);
  goto L5390;

 L5380: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L5381;
    }
  x1 = XEXP (x0, 0);
  goto L5390;

 L5381: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L5382;
    }
  x1 = XEXP (x0, 0);
  goto L5390;

 L5382: ATTRIBUTE_UNUSED_LABEL
  x2 = XVECEXP (x1, 0, 1);
  if (GET_CODE (x2) == SET)
    goto L5383;
  x1 = XEXP (x0, 0);
  goto L5390;

 L5383: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == MEM)
    goto L5384;
  x1 = XEXP (x0, 0);
  goto L5390;

 L5384: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == PLUS)
    goto L5385;
  x1 = XEXP (x0, 0);
  goto L5390;

 L5385: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (rtx_equal_p (x5, operands[1]))
    goto L5386;
  x1 = XEXP (x0, 0);
  goto L5390;

 L5386: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (GET_CODE (x5) == CONST_INT
      && XWINT (x5, 0) == 4)
    goto L5387;
  x1 = XEXP (x0, 0);
  goto L5390;

 L5387: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_hard_register_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L5388;
    }
  x1 = XEXP (x0, 0);
  goto L5390;

 L5388: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM && XVECLEN (operands[0], 0) == 2)))
    {
      return 622;
    }
  x1 = XEXP (x0, 0);
  goto L5390;

 L5391: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[2] = x2;
      goto L5392;
    }
  goto L5399;

 L5392: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L5393;
  goto L5399;

 L5393: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_CODE (x1) == SET)
    goto L5394;
  x1 = XEXP (x0, 0);
  goto L5399;

 L5394: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_CODE (x2) == PC)
    goto L5409;
  x1 = XEXP (x0, 0);
  goto L5399;

 L5409: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L6533;
 L5395: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == LABEL_REF)
    goto L5396;
  x1 = XEXP (x0, 0);
  goto L5399;

 L6533: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L5410;
    }
 L6534: ATTRIBUTE_UNUSED_LABEL
  if (memory_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L5418;
    }
  goto L5395;

 L5410: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 625;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L6534;

 L5418: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 626;
    }
  x1 = XEXP (x0, 1);
  x2 = XEXP (x1, 1);
  goto L5395;

 L5396: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  operands[0] = x3;
  goto L5397;

 L5397: ATTRIBUTE_UNUSED_LABEL
  if (((TARGET_ARM) && (TARGET_ARM)))
    {
      return 623;
    }
  x1 = XEXP (x0, 0);
  goto L5399;

 L5400: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (cc_register (x2, VOIDmode))
    {
      operands[1] = x2;
      goto L5401;
    }
  goto ret0;

 L5401: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_CODE (x2) == CONST_INT
      && XWINT (x2, 0) == 0)
    goto L5402;
  goto ret0;

 L5402: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_CODE (x1) == RETURN
      && ((TARGET_ARM) && (TARGET_ARM && USE_RETURN_INSN (FALSE))))
    {
      return 624;
    }
  goto ret0;
 ret0:
  return -1;
}

int recog PARAMS ((rtx, rtx, int *));
int
recog (x0, insn, pnum_clobbers)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *pnum_clobbers ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  int tem ATTRIBUTE_UNUSED;
  recog_data.insn = NULL_RTX;

  if (GET_MODE (x0) == SImode
      && GET_CODE (x0) == UNSPEC
      && XVECLEN (x0, 0) == 1
      && XINT (x0, 1) == 6)
    goto L3731;
  switch (GET_CODE (x0))
    {
    case PARALLEL:
      goto L5850;
    case SET:
      goto L9;
    case RETURN:
      goto L5865;
    case UNSPEC_VOLATILE:
      goto L5866;
    case CONST_INT:
      goto L5867;
    case PREFETCH:
      goto L3726;
    case COND_EXEC:
      goto L4040;
    default:
     break;
   }
  goto ret0;

 L3731: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (register_operand (x1, SImode))
    {
      operands[0] = x1;
      return 354;
    }
  goto ret0;

 L5850: ATTRIBUTE_UNUSED_LABEL
  return recog_7 (x0, insn, pnum_clobbers);

 L9: ATTRIBUTE_UNUSED_LABEL
  return recog_13 (x0, insn, pnum_clobbers);

 L5865: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && USE_RETURN_INSN (FALSE)))
    {
      return 258;
    }
  goto ret0;

 L5866: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) == 1)
    goto L6328;
  goto ret0;

 L6328: ATTRIBUTE_UNUSED_LABEL
  switch (XINT (x0, 1))
    {
    case 0:
      goto L2522;
    case 1:
      goto L3633;
    case 2:
      goto L3697;
    case 3:
      goto L3699;
    case 4:
      goto L3701;
    case 5:
      goto L3704;
    case 6:
      goto L3707;
    case 7:
      goto L3710;
    default:
      break;
    }
  goto ret0;

 L2522: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == CONST_INT
      && XWINT (x1, 0) == 0
      && (TARGET_EITHER))
    {
      return 262;
    }
  goto ret0;

 L3633: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == RETURN
      && (TARGET_EITHER))
    {
      return 338;
    }
  goto ret0;

 L3697: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == CONST_INT
      && XWINT (x1, 0) == 0
      && (TARGET_EITHER))
    {
      return 345;
    }
  goto ret0;

 L3699: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == CONST_INT
      && XWINT (x1, 0) == 0
      && (TARGET_EITHER))
    {
      return 346;
    }
  goto ret0;

 L3701: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  operands[0] = x1;
  goto L3702;

 L3702: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 347;
    }
  goto ret0;

 L3704: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  operands[0] = x1;
  goto L3705;

 L3705: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB))
    {
      return 348;
    }
  goto ret0;

 L3707: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  operands[0] = x1;
  goto L3708;

 L3708: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_EITHER))
    {
      return 349;
    }
  goto ret0;

 L3710: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  operands[0] = x1;
  goto L3711;

 L3711: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_EITHER))
    {
      return 350;
    }
  goto ret0;

 L5867: ATTRIBUTE_UNUSED_LABEL
  if (XWINT (x0, 0) == 0
      && (TARGET_EITHER))
    {
      return 267;
    }
  goto ret0;

 L3726: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 0);
  if (address_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L3727;
    }
  goto ret0;

 L3727: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  operands[1] = x1;
  goto L3728;

 L3728: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 2);
  operands[2] = x1;
  goto L3729;

 L3729: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_arch5e))
    {
      return 353;
    }
  goto ret0;

 L4040: ATTRIBUTE_UNUSED_LABEL
  return recog_19 (x0, insn, pnum_clobbers);
 ret0:
  return -1;
}

static rtx split_1 PARAMS ((rtx, rtx));
static rtx
split_1 (x0, insn)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  rtx tem ATTRIBUTE_UNUSED;

  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 0);
  switch (GET_MODE (x2))
    {
    case DImode:
      goto L6536;
    case SImode:
      goto L6537;
    case CC_Zmode:
      goto L6538;
    default:
      break;
    }
  goto ret0;

 L6536: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, DImode))
    {
      operands[0] = x2;
      goto L3735;
    }
  goto ret0;

 L3735: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == DImode
      && GET_CODE (x2) == PLUS)
    goto L3736;
  goto ret0;

 L3736: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode)
    goto L6540;
  goto ret0;

 L6540: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case SIGN_EXTEND:
      goto L3745;
    case ZERO_EXTEND:
      goto L3754;
    case SUBREG:
    case REG:
      goto L6539;
    default:
      goto ret0;
   }
 L6539: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L3737;
    }
  goto ret0;

 L3745: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3746;
    }
  goto ret0;

 L3746: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L3747;
    }
  goto ret0;

 L3747: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3748;
  goto ret0;

 L3748: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM && reload_completed))
    {
      return gen_split_357 (operands);
    }
  goto ret0;

 L3754: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3755;
    }
  goto ret0;

 L3755: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L3756;
    }
  goto ret0;

 L3756: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3757;
  goto ret0;

 L3757: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM && reload_completed))
    {
      return gen_split_358 (operands);
    }
  goto ret0;

 L3737: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, DImode))
    {
      operands[2] = x3;
      goto L3738;
    }
  goto ret0;

 L3738: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3739;
  goto ret0;

 L3739: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM && reload_completed))
    {
      return gen_split_356 (operands);
    }
  goto ret0;

 L6537: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x2, SImode))
    {
      operands[0] = x2;
      goto L3873;
    }
  goto ret0;

 L3873: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode)
    goto L6542;
  goto ret0;

 L6542: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case IOR:
      goto L3874;
    case ZERO_EXTEND:
      goto L3892;
    case SIGN_EXTEND:
      goto L3919;
    case AND:
      goto L3972;
    case IF_THEN_ELSE:
      goto L3998;
    default:
     break;
   }
 L6544: ATTRIBUTE_UNUSED_LABEL
  if (shiftable_operator (x2, SImode))
    {
      operands[3] = x2;
      goto L3900;
    }
  goto ret0;

 L3874: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == AND)
    goto L3875;
  goto L6544;

 L3875: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == NOT)
    goto L3876;
  goto L6544;

 L3876: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[1] = x5;
      goto L3877;
    }
  goto L6544;

 L3877: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == NOT)
    goto L3878;
  goto L6544;

 L3878: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (arm_rhs_operand (x5, SImode))
    {
      operands[2] = x5;
      goto L3879;
    }
  goto L6544;

 L3879: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[3] = x3;
      goto L3880;
    }
  goto L6544;

 L3880: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3881;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L6544;

 L3881: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[4] = x2;
      goto L3882;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L6544;

 L3882: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return gen_split_383 (operands);
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L6544;

 L3892: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  switch (GET_MODE (x3))
    {
    case HImode:
      goto L6548;
    case QImode:
      goto L6549;
    default:
      break;
    }
  goto ret0;

 L6548: ATTRIBUTE_UNUSED_LABEL
  if (alignable_memory_operand (x3, HImode))
    {
      operands[1] = x3;
      goto L3893;
    }
  goto ret0;

 L3893: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3894;
  goto ret0;

 L3894: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3895;
    }
  goto ret0;

 L3895: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && (!arm_arch4)))
    {
      return gen_split_395 (operands);
    }
  goto ret0;

 L6549: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x3) == SUBREG
      && XINT (x3, 1) == 0)
    goto L3911;
  goto ret0;

 L3911: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  operands[1] = x4;
  goto L3912;

 L3912: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3913;
  goto ret0;

 L3913: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3914;
    }
  goto ret0;

 L3914: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && (GET_CODE (operands[1]) != MEM) && ! BYTES_BIG_ENDIAN))
    {
      return gen_split_398 (operands);
    }
  goto ret0;

 L3919: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (alignable_memory_operand (x3, HImode))
    {
      operands[1] = x3;
      goto L3920;
    }
  goto ret0;

 L3920: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3921;
  goto ret0;

 L3921: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3922;
    }
  goto ret0;

 L3922: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && (!arm_arch4)))
    {
      return gen_split_401 (operands);
    }
  goto ret0;

 L3972: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == GE)
    goto L3973;
  goto L6544;

 L3973: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[1] = x4;
      goto L3974;
    }
  goto L6544;

 L3974: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 0)
    goto L3975;
  goto L6544;

 L3975: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == NEG)
    goto L3976;
  goto L6544;

 L3976: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (arm_comparison_operator (x4, SImode))
    {
      operands[2] = x4;
      goto L3977;
    }
  goto L6544;

 L3977: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (s_register_operand (x5, SImode))
    {
      operands[3] = x5;
      goto L3978;
    }
  goto L6544;

 L3978: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 1);
  if (arm_rhs_operand (x5, SImode))
    {
      operands[4] = x5;
      goto L3979;
    }
  goto L6544;

 L3979: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3980;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L6544;

 L3980: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[5] = x2;
      goto L3981;
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L6544;

 L3981: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM))
    {
      return gen_split_496 (operands);
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  goto L6544;

 L3998: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (arm_comparison_operator (x3, VOIDmode))
    {
      operands[1] = x3;
      goto L3999;
    }
  goto ret0;

 L3999: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  operands[2] = x4;
  goto L4000;
 L4032: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L4033;
    }
  goto ret0;

 L4000: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  operands[3] = x4;
  goto L4001;

 L4001: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (rtx_equal_p (x3, operands[0]))
    goto L4002;
 L4012: ATTRIBUTE_UNUSED_LABEL
  operands[4] = x3;
  goto L4013;

 L4002: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  operands[4] = x3;
  goto L4003;

 L4003: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L4004;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L4012;

 L4004: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM && reload_completed))
    {
      return gen_split_501 (operands);
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 1);
  goto L4012;

 L4013: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (rtx_equal_p (x3, operands[0]))
    goto L4014;
 L4024: ATTRIBUTE_UNUSED_LABEL
  operands[5] = x3;
  goto L4025;

 L4014: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L4015;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 2);
  goto L4024;

 L4015: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM && reload_completed))
    {
      return gen_split_502 (operands);
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 2);
  goto L4024;

 L4025: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L4026;
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L4032;

 L4026: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM && reload_completed))
    {
      return gen_split_503 (operands);
    }
  x1 = XVECEXP (x0, 0, 0);
  x2 = XEXP (x1, 1);
  x3 = XEXP (x2, 0);
  x4 = XEXP (x3, 0);
  goto L4032;

 L4033: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (arm_add_operand (x4, SImode))
    {
      operands[3] = x4;
      goto L4034;
    }
  goto ret0;

 L4034: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (arm_rhs_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L4035;
    }
  goto ret0;

 L4035: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 2);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == NOT)
    goto L4036;
  goto ret0;

 L4036: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[5] = x4;
      goto L4037;
    }
  goto ret0;

 L4037: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L4038;
  goto ret0;

 L4038: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24
      && (TARGET_ARM && reload_completed))
    {
      return gen_split_504 (operands);
    }
  goto ret0;

 L3900: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode)
    goto L6550;
  goto ret0;

 L6550: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case ZERO_EXTEND:
      goto L3901;
    case SIGN_EXTEND:
      goto L3928;
    default:
     break;
   }
  goto ret0;

 L3901: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (alignable_memory_operand (x4, HImode))
    {
      operands[1] = x4;
      goto L3902;
    }
  goto ret0;

 L3902: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L3903;
    }
  goto ret0;

 L3903: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3904;
  goto ret0;

 L3904: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3905;
    }
  goto ret0;

 L3905: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && (!arm_arch4)))
    {
      return gen_split_396 (operands);
    }
  goto ret0;

 L3928: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (alignable_memory_operand (x4, HImode))
    {
      operands[1] = x4;
      goto L3929;
    }
  goto ret0;

 L3929: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (s_register_operand (x3, SImode))
    {
      operands[4] = x3;
      goto L3930;
    }
  goto ret0;

 L3930: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3931;
  goto ret0;

 L3931: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3932;
    }
  goto ret0;

 L3932: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && (!arm_arch4)))
    {
      return gen_split_402 (operands);
    }
  goto ret0;

 L6538: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == REG
      && XINT (x2, 0) == 24)
    goto L3985;
  goto ret0;

 L3985: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == CC_Zmode
      && GET_CODE (x2) == COMPARE)
    goto L3986;
  goto ret0;

 L3986: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == SImode
      && GET_CODE (x3) == ASHIFT)
    goto L3987;
  goto ret0;

 L3987: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (GET_MODE (x4) == SImode
      && GET_CODE (x4) == SUBREG
      && XINT (x4, 1) == 0)
    goto L3988;
  goto ret0;

 L3988: ATTRIBUTE_UNUSED_LABEL
  x5 = XEXP (x4, 0);
  if (memory_operand (x5, QImode))
    {
      operands[0] = x5;
      goto L3989;
    }
  goto ret0;

 L3989: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 1);
  if (GET_CODE (x4) == CONST_INT
      && XWINT (x4, 0) == 24)
    goto L3990;
  goto ret0;

 L3990: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (const_int_operand (x3, VOIDmode))
    {
      operands[1] = x3;
      goto L3991;
    }
  goto ret0;

 L3991: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 1);
  if (GET_CODE (x1) == CLOBBER)
    goto L3992;
  goto ret0;

 L3992: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (scratch_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3993;
    }
  goto ret0;

 L3993: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && (((unsigned HOST_WIDE_INT) INTVAL (operands[1]))
       == (((unsigned HOST_WIDE_INT) INTVAL (operands[1])) >> 24) << 24)))
    {
      return gen_split_497 (operands);
    }
  goto ret0;
 ret0:
  return 0;
}

rtx split_insns PARAMS ((rtx, rtx));
rtx
split_insns (x0, insn)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  rtx tem ATTRIBUTE_UNUSED;
  recog_data.insn = NULL_RTX;

  switch (GET_CODE (x0))
    {
    case PARALLEL:
      goto L6535;
    case SET:
      goto L3766;
    default:
     break;
   }
  goto ret0;

 L6535: ATTRIBUTE_UNUSED_LABEL
  if (XVECLEN (x0, 0) == 2)
    goto L3733;
  goto ret0;

 L3733: ATTRIBUTE_UNUSED_LABEL
  x1 = XVECEXP (x0, 0, 0);
  if (GET_CODE (x1) == SET)
    goto L3734;
  goto ret0;

 L3734: ATTRIBUTE_UNUSED_LABEL
  return split_1 (x0, insn);

 L3766: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 0);
  switch (GET_MODE (x1))
    {
    case SImode:
      goto L6552;
    case DImode:
      goto L6553;
    case HImode:
      goto L6554;
    case SFmode:
      goto L6556;
    default:
      break;
    }
  goto ret0;

 L6552: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L3767;
    }
 L6555: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L3949;
    }
  goto ret0;

 L3767: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6557;
 L3945: ATTRIBUTE_UNUSED_LABEL
  if (const_int_operand (x1, SImode))
    {
      operands[1] = x1;
      goto L3946;
    }
  x1 = XEXP (x0, 0);
  goto L6555;

 L6557: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case PLUS:
      goto L3768;
    case MINUS:
      goto L3784;
    case AND:
      goto L3831;
    case IOR:
      goto L3860;
    case SIGN_EXTEND:
      goto L3941;
    default:
     break;
   }
  goto L3945;

 L3768: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3769;
    }
  goto L3945;

 L3769: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (reg_or_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3770;
    }
  goto L3945;

 L3770: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM &&
   GET_CODE (operands[2]) == CONST_INT
   && !(const_ok_for_arm (INTVAL (operands[2]))
        || const_ok_for_arm (-INTVAL (operands[2])))))
    {
      return gen_split_361 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L3945;

 L3784: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (reg_or_int_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3785;
    }
  goto L3945;

 L3785: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3786;
    }
  goto L3945;

 L3786: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && GET_CODE (operands[1]) == CONST_INT
   && !const_ok_for_arm (INTVAL (operands[1]))))
    {
      return gen_split_365 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L3945;

 L3831: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3832;
    }
  goto L3945;

 L3832: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (reg_or_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3833;
    }
  goto L3945;

 L3833: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && GET_CODE (operands[2]) == CONST_INT
   && !(const_ok_for_arm (INTVAL (operands[2]))
	|| const_ok_for_arm (~INTVAL (operands[2])))))
    {
      return gen_split_374 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L3945;

 L3860: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3861;
    }
  goto L3945;

 L3861: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (reg_or_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3862;
    }
  goto L3945;

 L3862: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && GET_CODE (operands[2]) == CONST_INT
   && !const_ok_for_arm (INTVAL (operands[2]))))
    {
      return gen_split_380 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L3945;

 L3941: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (bad_signed_byte_operand (x2, QImode))
    {
      operands[1] = x2;
      goto L3942;
    }
  goto L3945;

 L3942: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_arch4 && reload_completed))
    {
      return gen_split_406 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L3945;

 L3946: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
  && (!(const_ok_for_arm (INTVAL (operands[1]))
        || const_ok_for_arm (~INTVAL (operands[1]))))))
    {
      return gen_split_409 (operands);
    }
  x1 = XEXP (x0, 0);
  goto L6555;

 L3949: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (const_int_operand (x1, SImode))
    {
      operands[1] = x1;
      goto L3950;
    }
  goto ret0;

 L3950: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB && CONST_OK_FOR_THUMB_LETTER (INTVAL (operands[1]), 'J')))
    {
      return gen_split_410 (operands);
    }
 L3954: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB && CONST_OK_FOR_THUMB_LETTER (INTVAL (operands[1]), 'K')))
    {
      return gen_split_411 (operands);
    }
  goto ret0;

 L6553: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, DImode))
    {
      operands[0] = x1;
      goto L3796;
    }
  goto ret0;

 L3796: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == DImode)
    goto L6563;
  goto ret0;

 L6563: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case IOR:
      goto L3810;
    case XOR:
      goto L3817;
    case AND:
      goto L3824;
    case NOT:
      goto L3886;
    default:
     break;
   }
 L6562: ATTRIBUTE_UNUSED_LABEL
  if (logical_binary_operator (x1, DImode))
    {
      operands[6] = x1;
      goto L3797;
    }
  goto ret0;

 L3810: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode
      && GET_CODE (x2) == ZERO_EXTEND)
    goto L3811;
  goto L6562;

 L3811: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3812;
    }
  goto L6562;

 L3812: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L3813;
    }
  goto L6562;

 L3813: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && operands[0] != operands[1] && reload_completed))
    {
      return gen_split_370 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L6562;

 L3817: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode
      && GET_CODE (x2) == ZERO_EXTEND)
    goto L3818;
  goto L6562;

 L3818: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3819;
    }
  goto L6562;

 L3819: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L3820;
    }
  goto L6562;

 L3820: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && operands[0] != operands[1] && reload_completed))
    {
      return gen_split_371 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L6562;

 L3824: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode)
    goto L6567;
  goto L6562;

 L6567: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x2))
    {
    case ZERO_EXTEND:
      goto L3825;
    case NOT:
      goto L3838;
    default:
     break;
   }
  goto L6562;

 L3825: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3826;
    }
  goto L6562;

 L3826: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L3827;
    }
  goto L6562;

 L3827: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && reload_completed))
    {
      return gen_split_372 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L6562;

 L3838: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (GET_MODE (x3) == DImode)
    goto L6570;
  goto L6562;

 L6570: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x3))
    {
    case ZERO_EXTEND:
      goto L3846;
    case SIGN_EXTEND:
      goto L3854;
    case SUBREG:
    case REG:
      goto L6569;
    default:
      goto L6562;
   }
 L6569: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x3, DImode))
    {
      operands[1] = x3;
      goto L3839;
    }
  goto L6562;

 L3846: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3847;
    }
  goto L6562;

 L3847: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L3848;
    }
  goto L6562;

 L3848: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && reload_completed
   && operands[0] != operands[1]))
    {
      return gen_split_377 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L6562;

 L3854: ATTRIBUTE_UNUSED_LABEL
  x4 = XEXP (x3, 0);
  if (s_register_operand (x4, SImode))
    {
      operands[2] = x4;
      goto L3855;
    }
  goto L6562;

 L3855: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L3856;
    }
  goto L6562;

 L3856: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && reload_completed))
    {
      return gen_split_378 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L6562;

 L3839: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L3840;
    }
  goto L6562;

 L3840: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && reload_completed))
    {
      return gen_split_376 (operands);
    }
  x1 = XEXP (x0, 1);
  goto L6562;

 L3886: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L3887;
    }
  goto ret0;

 L3887: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && reload_completed))
    {
      return gen_split_392 (operands);
    }
  goto ret0;

 L3797: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == DImode)
    goto L6573;
  goto ret0;

 L6573: ATTRIBUTE_UNUSED_LABEL
  if (GET_CODE (x2) == SIGN_EXTEND)
    goto L3804;
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L3798;
    }
  goto ret0;

 L3804: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (s_register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3805;
    }
  goto ret0;

 L3805: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[1] = x2;
      goto L3806;
    }
  goto ret0;

 L3806: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && reload_completed))
    {
      return gen_split_369 (operands);
    }
  goto ret0;

 L3798: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, DImode))
    {
      operands[2] = x2;
      goto L3799;
    }
  goto ret0;

 L3799: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && reload_completed))
    {
      return gen_split_368 (operands);
    }
  goto ret0;

 L6554: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, HImode))
    {
      operands[0] = x1;
      goto L3935;
    }
  goto ret0;

 L3935: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == HImode
      && GET_CODE (x1) == SIGN_EXTEND)
    goto L3936;
  goto ret0;

 L3936: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (bad_signed_byte_operand (x2, QImode))
    {
      operands[1] = x2;
      goto L3937;
    }
  goto ret0;

 L3937: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM && arm_arch4 && reload_completed))
    {
      return gen_split_404 (operands);
    }
  goto ret0;

 L6556: ATTRIBUTE_UNUSED_LABEL
  if (nonimmediate_operand (x1, SFmode))
    {
      operands[0] = x1;
      goto L3957;
    }
  goto ret0;

 L3957: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (immediate_operand (x1, SFmode))
    {
      operands[1] = x1;
      goto L3958;
    }
  goto ret0;

 L3958: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_ARM
   && !TARGET_HARD_FLOAT
   && reload_completed
   && GET_CODE (operands[1]) == CONST_DOUBLE))
    {
      return gen_split_425 (operands);
    }
  goto ret0;
 ret0:
  return 0;
}

#ifdef HAVE_peephole2
rtx peephole2_insns PARAMS ((rtx, rtx, int *));
rtx
peephole2_insns (x0, insn, _pmatch_len)
     rtx x0 ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
     int *_pmatch_len ATTRIBUTE_UNUSED;
{
  rtx * const operands ATTRIBUTE_UNUSED = &recog_data.operand[0];
  rtx x1 ATTRIBUTE_UNUSED;
  rtx x2 ATTRIBUTE_UNUSED;
  rtx x3 ATTRIBUTE_UNUSED;
  rtx x4 ATTRIBUTE_UNUSED;
  rtx x5 ATTRIBUTE_UNUSED;
  rtx x6 ATTRIBUTE_UNUSED;
  rtx x7 ATTRIBUTE_UNUSED;
  rtx tem ATTRIBUTE_UNUSED;
  recog_data.insn = NULL_RTX;

  if (GET_CODE (x0) == SET)
    goto L3760;
  goto ret0;

 L3760: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 0);
  if (GET_MODE (x1) == SImode)
    goto L6574;
  goto ret0;

 L6574: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L3761;
    }
 L6575: ATTRIBUTE_UNUSED_LABEL
  if (register_operand (x1, SImode))
    {
      operands[0] = x1;
      goto L3774;
    }
  goto ret0;

 L3761: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (GET_MODE (x1) == SImode)
    goto L6576;
  x1 = XEXP (x0, 0);
  goto L6575;

 L6576: ATTRIBUTE_UNUSED_LABEL
  switch (GET_CODE (x1))
    {
    case PLUS:
      goto L3762;
    case MINUS:
      goto L3791;
    case IOR:
      goto L3867;
    case SUBREG:
    case REG:
      goto L6579;
    default:
      x1 = XEXP (x0, 0);
      goto L6575;
   }
 L6579: ATTRIBUTE_UNUSED_LABEL
  if (s_register_operand (x1, SImode))
    {
      operands[1] = x1;
      goto L3963;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3762: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3763;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3763: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (const_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3764;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3764: ATTRIBUTE_UNUSED_LABEL
  tem = peep2_next_insn (0);
  if (tem == NULL_RTX)
    goto L6575;
  x1 = PATTERN (tem);
  if ((TARGET_ARM &&
   !(const_ok_for_arm (INTVAL (operands[2]))
     || const_ok_for_arm (-INTVAL (operands[2])))
    && const_ok_for_arm (~INTVAL (operands[2]))))
    {
      *_pmatch_len = 0;
      tem = gen_peephole2_360 (insn, operands);
      if (tem != 0)
        return tem;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3791: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (const_int_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3792;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3792: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (s_register_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3793;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3793: ATTRIBUTE_UNUSED_LABEL
  tem = peep2_next_insn (0);
  if (tem == NULL_RTX)
    goto L6575;
  x1 = PATTERN (tem);
  if ((TARGET_ARM
   && !const_ok_for_arm (INTVAL (operands[1]))
   && const_ok_for_arm (~INTVAL (operands[1]))))
    {
      *_pmatch_len = 0;
      tem = gen_peephole2_366 (insn, operands);
      if (tem != 0)
        return tem;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3867: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (s_register_operand (x2, SImode))
    {
      operands[1] = x2;
      goto L3868;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3868: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (const_int_operand (x2, SImode))
    {
      operands[2] = x2;
      goto L3869;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3869: ATTRIBUTE_UNUSED_LABEL
  tem = peep2_next_insn (0);
  if (tem == NULL_RTX)
    goto L6575;
  x1 = PATTERN (tem);
  if ((TARGET_ARM
   && !const_ok_for_arm (INTVAL (operands[2]))
   && const_ok_for_arm (~INTVAL (operands[2]))))
    {
      *_pmatch_len = 0;
      tem = gen_peephole2_381 (insn, operands);
      if (tem != 0)
        return tem;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3963: ATTRIBUTE_UNUSED_LABEL
  tem = peep2_next_insn (1);
  if (tem == NULL_RTX)
    goto L6575;
  x1 = PATTERN (tem);
  if (GET_CODE (x1) == SET)
    goto L3964;
  x1 = XEXP (x0, 0);
  goto L6575;

 L3964: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == REG
      && XINT (x2, 0) == 24)
    goto L3965;
  x1 = XEXP (x0, 0);
  goto L6575;

 L3965: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == CCmode
      && GET_CODE (x2) == COMPARE)
    goto L3966;
  x1 = XEXP (x0, 0);
  goto L6575;

 L3966: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[1]))
    goto L3967;
  x1 = XEXP (x0, 0);
  goto L6575;

 L3967: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (GET_CODE (x3) == CONST_INT
      && XWINT (x3, 0) == 0
      && (TARGET_ARM
  ))
    {
      *_pmatch_len = 1;
      tem = gen_peephole2_489 (insn, operands);
      if (tem != 0)
        return tem;
    }
  x1 = XEXP (x0, 0);
  goto L6575;

 L3774: ATTRIBUTE_UNUSED_LABEL
  x1 = XEXP (x0, 1);
  if (const_int_operand (x1, SImode))
    {
      operands[1] = x1;
      goto L3775;
    }
  goto ret0;

 L3775: ATTRIBUTE_UNUSED_LABEL
  tem = peep2_next_insn (1);
  if (tem == NULL_RTX)
    goto ret0;
  x1 = PATTERN (tem);
  if (GET_CODE (x1) == SET)
    goto L3776;
  goto ret0;

 L3776: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 0);
  if (rtx_equal_p (x2, operands[0]))
    goto L3777;
  goto ret0;

 L3777: ATTRIBUTE_UNUSED_LABEL
  x2 = XEXP (x1, 1);
  if (GET_MODE (x2) == SImode
      && GET_CODE (x2) == PLUS)
    goto L3778;
  goto ret0;

 L3778: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 0);
  if (rtx_equal_p (x3, operands[0]))
    goto L3779;
  goto ret0;

 L3779: ATTRIBUTE_UNUSED_LABEL
  x3 = XEXP (x2, 1);
  if (register_operand (x3, SImode))
    {
      operands[2] = x3;
      goto L3780;
    }
  goto ret0;

 L3780: ATTRIBUTE_UNUSED_LABEL
  if ((TARGET_THUMB
   && REGNO (operands[2]) == STACK_POINTER_REGNUM 
   && (unsigned HOST_WIDE_INT) (INTVAL (operands[1])) < 1024
   && (INTVAL (operands[1]) & 3) == 0))
    {
      *_pmatch_len = 1;
      tem = gen_peephole2_362 (insn, operands);
      if (tem != 0)
        return tem;
    }
  goto ret0;
 ret0:
  return 0;
}

#endif
