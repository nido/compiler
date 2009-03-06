/* Generated automatically by the program `genoutput'
   from the machine description file `md'.  */

#include "config.h"
#include "system.h"
#include "flags.h"
#include "ggc.h"
#include "rtl.h"
#include "expr.h"
#include "insn-codes.h"
#include "tm_p.h"
#include "function.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "real.h"
#include "insn-config.h"

#include "conditions.h"
#include "insn-attr.h"

#include "recog.h"

#include "toplev.h"
#include "output.h"

static const char * const output_4[] = {
  "add%?\t%0, %1, %2",
  "sub%?\t%0, %1, #%n2",
  "#",
};

static const char *output_5 PARAMS ((rtx *, rtx));

static const char *
output_5 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

   static const char * const asms[] = 
   {
     "add\t%0, %0, %2",
     "sub\t%0, %0, #%n2",
     "add\t%0, %1, %2",
     "add\t%0, %0, %2",
     "add\t%0, %0, %2",
     "add\t%0, %1, %2",
     "add\t%0, %1, %2"
   };
   if ((which_alternative == 2 || which_alternative == 6)
       && GET_CODE (operands[2]) == CONST_INT
       && INTVAL (operands[2]) < 0)
     return "sub\t%0, %1, #%n2";
   return asms[which_alternative];
  
}

static const char * const output_6[] = {
  "add%?s\t%0, %1, %2",
  "sub%?s\t%0, %1, #%n2",
};

static const char * const output_7[] = {
  "cmn%?\t%0, %1",
  "cmp%?\t%0, #%n1",
};

static const char * const output_8[] = {
  "add%?s\t%0, %1, %2",
  "sub%?s\t%0, %1, #%n2",
};

static const char * const output_9[] = {
  "cmn%?\t%0, %1",
  "cmp%?\t%0, #%n1",
};

static const char * const output_10[] = {
  "add%?s\t%0, %1, %2",
  "sub%?s\t%0, %1, #%n2",
};

static const char * const output_11[] = {
  "add%?s\t%0, %1, %2",
  "sub%?s\t%0, %1, #%n2",
};

static const char * const output_12[] = {
  "cmn%?\t%0, %1",
  "cmp%?\t%0, #%n1",
};

static const char * const output_13[] = {
  "cmn%?\t%0, %1",
  "cmp%?\t%0, #%n1",
};

static const char * const output_19[] = {
  "add%d2\t%0, %1, #1",
  "mov%D2\t%0, %1\n\tadd%d2\t%0, %1, #1",
};

static const char * const output_20[] = {
  "adf%?s\t%0, %1, %2",
  "suf%?s\t%0, %1, #%N2",
};

static const char * const output_21[] = {
  "adf%?d\t%0, %1, %2",
  "suf%?d\t%0, %1, #%N2",
};

static const char * const output_22[] = {
  "adf%?d\t%0, %1, %2",
  "suf%?d\t%0, %1, #%N2",
};

static const char * const output_25[] = {
  "adf%?e\t%0, %1, %2",
  "suf%?e\t%0, %1, #%N2",
};

static const char * const output_34[] = {
  "rsb%?\t%0, %2, %1",
  "#",
};

static const char * const output_35[] = {
  "sub%?s\t%0, %1, %2",
  "rsb%?s\t%0, %2, %1",
};

static const char * const output_36[] = {
  "sub%d2\t%0, %1, #1",
  "mov%D2\t%0, %1\n\tsub%d2\t%0, %1, #1",
};

static const char * const output_37[] = {
  "suf%?s\t%0, %1, %2",
  "rsf%?s\t%0, %2, %1",
};

static const char * const output_38[] = {
  "suf%?d\t%0, %1, %2",
  "rsf%?d\t%0, %2, %1",
};

static const char * const output_40[] = {
  "suf%?d\t%0, %1, %2",
  "rsf%?d\t%0, %2, %1",
};

static const char * const output_42[] = {
  "suf%?e\t%0, %1, %2",
  "rsf%?e\t%0, %2, %1",
};

static const char *output_44 PARAMS ((rtx *, rtx));

static const char *
output_44 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (which_alternative < 2)
    return "mov\t%0, %1\n\tmul\t%0, %0, %2";
  else
    return "mul\t%0, %0, %2";
  
}

static const char * const output_65[] = {
  "fdv%?s\t%0, %1, %2",
  "frd%?s\t%0, %2, %1",
};

static const char * const output_66[] = {
  "dvf%?d\t%0, %1, %2",
  "rdf%?d\t%0, %2, %1",
};

static const char * const output_70[] = {
  "dvf%?e\t%0, %1, %2",
  "rdf%?e\t%0, %2, %1",
};

static const char * const output_80[] = {
  "and%?\t%0, %1, %2",
  "bic%?\t%0, %1, #%B2",
  "#",
};

static const char * const output_82[] = {
  "and%?s\t%0, %1, %2",
  "bic%?s\t%0, %1, #%B2",
};

static const char * const output_83[] = {
  "tst%?\t%0, %1",
  "bic%?s\t%2, %0, #%B1",
};

static const char *output_84 PARAMS ((rtx *, rtx));

static const char *
output_84 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  operands[1] = GEN_INT (((1 << INTVAL (operands[1])) - 1)
			 << INTVAL (operands[2]));
  output_asm_insn ("tst%?\t%0, %1", operands);
  return "";
  
}

static const char *output_85 PARAMS ((rtx *, rtx));

static const char *
output_85 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  operands[2] = GEN_INT (((1 << INTVAL (operands[2])) - 1)
			 << INTVAL (operands[3]));
  output_asm_insn ("ands\t%0, %1, %2", operands);
  return "movne\t%0, #1";
  
}

static const char * const output_87[] = {
  "bic%?\t%Q0, %Q1, %2",
  "#",
};

static const char * const output_95[] = {
  "orr%?\t%Q0, %Q1, %2",
  "#",
};

static const char * const output_97[] = {
  "orr%?\t%0, %1, %2",
  "#",
};

static const char * const output_102[] = {
  "eor%?\t%Q0, %Q1, %2",
  "#",
};

static const char * const output_109[] = {
  "cmp\t%1, %2\n\tmovlt\t%0, %2",
  "cmp\t%1, %2\n\tmovge\t%0, %1",
  "cmp\t%1, %2\n\tmovge\t%0, %1\n\tmovlt\t%0, %2",
};

static const char * const output_110[] = {
  "cmp\t%1, %2\n\tmovge\t%0, %2",
  "cmp\t%1, %2\n\tmovlt\t%0, %1",
  "cmp\t%1, %2\n\tmovlt\t%0, %1\n\tmovge\t%0, %2",
};

static const char * const output_111[] = {
  "cmp\t%1, %2\n\tmovcc\t%0, %2",
  "cmp\t%1, %2\n\tmovcs\t%0, %1",
  "cmp\t%1, %2\n\tmovcs\t%0, %1\n\tmovcc\t%0, %2",
};

static const char * const output_112[] = {
  "cmp\t%1, %2\n\tmovcs\t%0, %2",
  "cmp\t%1, %2\n\tmovcc\t%0, %1",
  "cmp\t%1, %2\n\tmovcc\t%0, %1\n\tmovcs\t%0, %2",
};

static const char *output_113 PARAMS ((rtx *, rtx));

static const char *
output_113 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  operands[3] = gen_rtx (minmax_code (operands[3]), SImode, operands[1],
			 operands[2]);
  output_asm_insn ("cmp\t%1, %2", operands);
  output_asm_insn ("str%d3\t%1, %0", operands);
  output_asm_insn ("str%D3\t%2, %0", operands);
  return "";
  
}

static const char *output_114 PARAMS ((rtx *, rtx));

static const char *
output_114 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    enum rtx_code code = GET_CODE (operands[4]);

    operands[5] = gen_rtx (minmax_code (operands[5]), SImode, operands[2],
			   operands[3]);
    output_asm_insn ("cmp\t%2, %3", operands);
    output_asm_insn ("%i4%d5\t%0, %1, %2", operands);
    if (which_alternative != 0 || operands[3] != const0_rtx
        || (code != PLUS && code != MINUS && code != IOR && code != XOR))
      output_asm_insn ("%i4%D5\t%0, %1, %3", operands);
    return "";
  }
}

static const char * const output_133[] = {
  "cmp\t%0, #0\n\trsblt\t%0, %0, #0",
  "eor%?\t%0, %1, %1, asr #31\n\tsub%?\t%0, %0, %1, asr #31",
};

static const char * const output_134[] = {
  "cmp\t%0, #0\n\trsbgt\t%0, %0, #0",
  "eor%?\t%0, %1, %1, asr #31\n\trsb%?\t%0, %0, %1, asr #31",
};

static const char *output_157 PARAMS ((rtx *, rtx));

static const char *
output_157 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    if (REGNO (operands[1])
        != REGNO (operands[0]) + (WORDS_BIG_ENDIAN ? 1 : 0))
      output_asm_insn ("mov%?\t%Q0, %1", operands);
    return "mov%?\t%R0, #0";
  
}

static const char * const output_158[] = {
  "and%?\t%Q0, %1, #255\n\tmov%?\t%R0, #0",
  "ldr%?b\t%Q0, %1\n\tmov%?\t%R0, #0",
};

static const char *output_159 PARAMS ((rtx *, rtx));

static const char *
output_159 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    if (REGNO (operands[1])
        != REGNO (operands[0]) + (WORDS_BIG_ENDIAN ? 1 : 0))
      output_asm_insn ("mov%?\t%Q0, %1", operands);
    return "mov%?\t%R0, %Q0, asr #31";
  
}

static const char *output_160 PARAMS ((rtx *, rtx));

static const char *
output_160 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  rtx mem = XEXP (operands[1], 0);

  if (GET_CODE (mem) == CONST)
    mem = XEXP (mem, 0);
    
  if (GET_CODE (mem) == LABEL_REF)
    return "ldr\t%0, %1";
    
  if (GET_CODE (mem) == PLUS)
    {
      rtx a = XEXP (mem, 0);
      rtx b = XEXP (mem, 1);

      /* This can happen due to bugs in reload.  */
      if (GET_CODE (a) == REG && REGNO (a) == SP_REGNUM)
        {
          rtx ops[2];
          ops[0] = operands[0];
          ops[1] = a;
      
          output_asm_insn ("mov	%0, %1", ops);

          XEXP (mem, 0) = operands[0];
       }

      else if (   GET_CODE (a) == LABEL_REF
	       && GET_CODE (b) == CONST_INT)
        return "ldr\t%0, %1";
    }
    
  return "ldrh\t%0, %1";
  
}

static const char *output_165 PARAMS ((rtx *, rtx));

static const char *
output_165 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    rtx ops[4];
    rtx mem = XEXP (operands[1], 0);

    /* This code used to try to use 'V', and fix the address only if it was
       offsettable, but this fails for e.g. REG+48 because 48 is outside the
       range of QImode offsets, and offsettable_address_p does a QImode
       address check.  */
       
    if (GET_CODE (mem) == CONST)
      mem = XEXP (mem, 0);
    
    if (GET_CODE (mem) == LABEL_REF)
      return "ldr\t%0, %1";
    
    if (GET_CODE (mem) == PLUS)
      {
        rtx a = XEXP (mem, 0);
        rtx b = XEXP (mem, 1);

        if (GET_CODE (a) == LABEL_REF
	    && GET_CODE (b) == CONST_INT)
          return "ldr\t%0, %1";

        if (GET_CODE (b) == REG)
          return "ldrsh\t%0, %1";
	  
        ops[1] = a;
        ops[2] = b;
      }
    else
      {
        ops[1] = mem;
        ops[2] = const0_rtx;
      }
      
    if (GET_CODE (ops[1]) != REG)
      {
        debug_rtx (ops[1]);
        abort ();
      }

    ops[0] = operands[0];
    ops[3] = operands[2];
    output_asm_insn ("mov\t%3, %2\n\tldrsh\t%0, [%1, %3]", ops);
    return "";
  }
}

static const char *output_167 PARAMS ((rtx *, rtx));

static const char *
output_167 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  /* If the address is invalid, this will split the instruction into two. */
  if (bad_signed_byte_operand (operands[1], VOIDmode))
    return "#";
  return "ldr%?sb\t%0, %1";
  
}

static const char *output_168 PARAMS ((rtx *, rtx));

static const char *
output_168 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  /* If the address is invalid, this will split the instruction into two. */
  if (bad_signed_byte_operand (operands[1], VOIDmode))
    return "#";
  return "ldr%?sb\t%0, %1";
  
}

static const char *output_169 PARAMS ((rtx *, rtx));

static const char *
output_169 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    rtx ops[3];
    rtx mem = XEXP (operands[1], 0);
    
    if (GET_CODE (mem) == CONST)
      mem = XEXP (mem, 0);
    
    if (GET_CODE (mem) == LABEL_REF)
      return "ldr\t%0, %1";

    if (GET_CODE (mem) == PLUS
        && GET_CODE (XEXP (mem, 0)) == LABEL_REF)
      return "ldr\t%0, %1";
      
    if (which_alternative == 0)
      return "ldrsb\t%0, %1";
      
    ops[0] = operands[0];
    
    if (GET_CODE (mem) == PLUS)
      {
        rtx a = XEXP (mem, 0);
	rtx b = XEXP (mem, 1);
	
        ops[1] = a;
        ops[2] = b;

        if (GET_CODE (a) == REG)
	  {
	    if (GET_CODE (b) == REG)
              output_asm_insn ("ldrsb\t%0, [%1, %2]", ops);
            else if (REGNO (a) == REGNO (ops[0]))
	      {
                output_asm_insn ("ldrb\t%0, [%1, %2]", ops);
		output_asm_insn ("lsl\t%0, %0, #24", ops);
		output_asm_insn ("asr\t%0, %0, #24", ops);
	      }
	    else
              output_asm_insn ("mov\t%0, %2\n\tldrsb\t%0, [%1, %0]", ops);
	  }
        else if (GET_CODE (b) != REG)
	  abort ();
	else
          {
            if (REGNO (b) == REGNO (ops[0]))
	      {
                output_asm_insn ("ldrb\t%0, [%2, %1]", ops);
		output_asm_insn ("lsl\t%0, %0, #24", ops);
		output_asm_insn ("asr\t%0, %0, #24", ops);
	      }
	    else
              output_asm_insn ("mov\t%0, %2\n\tldrsb\t%0, [%1, %0]", ops);
          }
      }
    else if (GET_CODE (mem) == REG && REGNO (ops[0]) == REGNO (mem))
      {
        output_asm_insn ("ldrb\t%0, [%0, #0]", ops);
	output_asm_insn ("lsl\t%0, %0, #24", ops);
	output_asm_insn ("asr\t%0, %0, #24", ops);
      }
    else
      {
        ops[1] = mem;
        ops[2] = const0_rtx;
	
        output_asm_insn ("mov\t%0, %2\n\tldrsb\t%0, [%1, %0]", ops);
      }
    return "";
  }
}

static const char *output_173 PARAMS ((rtx *, rtx));

static const char *
output_173 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return (output_move_double (operands));
  
}

static const char *output_174 PARAMS ((rtx *, rtx));

static const char *
output_174 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
  switch (which_alternative)
    {
    default:
    case 0:
      if (REGNO (operands[1]) == REGNO (operands[0]) + 1)
	return "add\t%0,  %1,  #0\n\tadd\t%H0, %H1, #0";
      return   "add\t%H0, %H1, #0\n\tadd\t%0,  %1,  #0";
    case 1:
      return "mov\t%Q0, %1\n\tmov\t%R0, #0";
    case 2:
      operands[1] = GEN_INT (- INTVAL (operands[1]));
      return "mov\t%Q0, %1\n\tneg\t%Q0, %Q0\n\tasr\t%R0, %Q0, #31";
    case 3:
      return "ldmia\t%1, {%0, %H0}";
    case 4:
      return "stmia\t%0, {%1, %H1}";
    case 5:
      return thumb_load_double_from_address (operands);
    case 6:
      operands[2] = gen_rtx (MEM, SImode,
			     plus_constant (XEXP (operands[0], 0), 4));
      output_asm_insn ("str\t%1, %0\n\tstr\t%H1, %2", operands);
      return "";
    case 7:
      if (REGNO (operands[1]) == REGNO (operands[0]) + 1)
	return "mov\t%0, %1\n\tmov\t%H0, %H1";
      return "mov\t%H0, %H1\n\tmov\t%0, %1";
    }
  }
}

static const char * const output_175[] = {
  "mov%?\t%0, %1",
  "mvn%?\t%0, #%B1",
  "ldr%?\t%0, %1",
  "str%?\t%1, %0",
};

static const char * const output_176[] = {
  "mov	%0, %1",
  "mov	%0, %1",
  "#",
  "#",
  "ldmia\t%1, {%0}",
  "stmia\t%0, {%1}",
  "ldr\t%0, %1",
  "str\t%1, %0",
  "mov\t%0, %1",
};

static const char *output_179 PARAMS ((rtx *, rtx));

static const char *
output_179 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

#ifdef AOF_ASSEMBLER
  operands[1] = aof_pic_entry (operands[1]);
#endif
  output_asm_insn ("ldr%?\t%0, %a1", operands);
  return "";
  
}

static const char *output_180 PARAMS ((rtx *, rtx));

static const char *
output_180 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  ASM_OUTPUT_INTERNAL_LABEL (asm_out_file, "L",
			     CODE_LABEL_NUMBER (operands[1]));
  return "add\t%0, %|pc";
  
}

static const char *output_181 PARAMS ((rtx *, rtx));

static const char *
output_181 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    ASM_OUTPUT_INTERNAL_LABEL (asm_out_file, "L",
			       CODE_LABEL_NUMBER (operands[1]));
    return "add%?\t%0, %|pc, %0";
  
}

static const char * const output_182[] = {
  "cmp%?\t%0, #0",
  "sub%?s\t%0, %1, #0",
};

static const char *output_183 PARAMS ((rtx *, rtx));

static const char *
output_183 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  switch (which_alternative)
    {
    case 0: return "add	%0, %1, #0";
    case 2: return "strh	%1, %0";
    case 3: return "mov	%0, %1";
    case 4: return "mov	%0, %1";
    case 5: return "mov	%0, %1";
    default: abort ();
    case 1:
      /* The stack pointer can end up being taken as an index register.
          Catch this case here and deal with it.  */
      if (GET_CODE (XEXP (operands[1], 0)) == PLUS
	  && GET_CODE (XEXP (XEXP (operands[1], 0), 0)) == REG
	  && REGNO    (XEXP (XEXP (operands[1], 0), 0)) == SP_REGNUM)
        {
	  rtx ops[2];
          ops[0] = operands[0];
          ops[1] = XEXP (XEXP (operands[1], 0), 0);
      
          output_asm_insn ("mov	%0, %1", ops);

          XEXP (XEXP (operands[1], 0), 0) = operands[0];
    
	}
      return "ldrh	%0, %1";
    }
}

static const char *output_184 PARAMS ((rtx *, rtx));

static const char *
output_184 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    rtx ops[2];

    ops[0] = operands[0];
    ops[1] = gen_rtx_MEM (SImode, plus_constant (XEXP (operands[1], 0), 2));
    output_asm_insn ("ldr%?\t%0, %1\t%@ load-rotate", ops);
    return "";
  }
}

static const char * const output_185[] = {
  "mov%?\t%0, %1\t%@ movhi",
  "mvn%?\t%0, #%B1\t%@ movhi",
  "str%?h\t%1, %0\t%@ movhi ",
  "ldr%?h\t%0, %1\t%@ movhi",
};

static const char * const output_186[] = {
  "mov%?\t%0, %1\t%@ movhi",
  "mvn%?\t%0, #%B1\t%@ movhi",
  "ldr%?\t%0, %1\t%@ movhi",
};

static const char * const output_187[] = {
  "mov%?\t%0, %1\t%@ movhi",
  "mvn%?\t%0, #%B1\t%@ movhi",
  "ldr%?\t%0, %1\t%@ movhi_bigend\n\tmov%?\t%0, %0, asr #16",
};

static const char * const output_189[] = {
  "mov%?\t%0, %1\t%@ movhi",
  "mvn%?\t%0, #%B1\t%@ movhi",
};

static const char *output_190 PARAMS ((rtx *, rtx));

static const char *
output_190 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  abort ();
}

static const char * const output_191[] = {
  "mov%?\t%0, %1",
  "mvn%?\t%0, #%B1",
  "ldr%?b\t%0, %1",
  "str%?b\t%1, %0",
};

static const char * const output_192[] = {
  "add\t%0, %1, #0",
  "ldrb\t%0, %1",
  "strb\t%1, %0",
  "mov\t%0, %1",
  "mov\t%0, %1",
  "mov\t%0, %1",
};

static const char * const output_193[] = {
  "mvf%?s\t%0, %1",
  "mnf%?s\t%0, #%N1",
  "ldf%?s\t%0, %1",
  "stf%?s\t%1, %0",
  "str%?\t%1, [%|sp, #-4]!\n\tldf%?s\t%0, [%|sp], #4",
  "stf%?s\t%1, [%|sp, #-4]!\n\tldr%?\t%0, [%|sp], #4",
  "mov%?\t%0, %1",
  "ldr%?\t%0, %1\t%@ float",
  "str%?\t%1, %0\t%@ float",
};

static const char * const output_194[] = {
  "mov%?\t%0, %1",
  "ldr%?\t%0, %1\t%@ float",
  "str%?\t%1, %0\t%@ float",
};

static const char * const output_195[] = {
  "add\t%0, %1, #0",
  "ldmia\t%1, {%0}",
  "stmia\t%0, {%1}",
  "ldr\t%0, %1",
  "str\t%1, %0",
  "mov\t%0, %1",
  "mov\t%0, %1",
};

static const char *output_196 PARAMS ((rtx *, rtx));

static const char *
output_196 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
  switch (which_alternative)
    {
    default:
    case 0: return "ldm%?ia\t%m1, %M0\t%@ double";
    case 1: return "stm%?ia\t%m0, %M1\t%@ double";
    case 2: case 3: case 4: return output_move_double (operands);
    case 5: return "mvf%?d\t%0, %1";
    case 6: return "mnf%?d\t%0, #%N1";
    case 7: return "ldf%?d\t%0, %1";
    case 8: return "stf%?d\t%1, %0";
    case 9: return output_mov_double_fpu_from_arm (operands);
    case 10: return output_mov_double_arm_from_fpu (operands);
    }
  }
  
}

static const char *output_197 PARAMS ((rtx *, rtx));

static const char *
output_197 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{
 return output_move_double (operands);
}

static const char *output_198 PARAMS ((rtx *, rtx));

static const char *
output_198 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  switch (which_alternative)
    {
    default:
    case 0:
      if (REGNO (operands[1]) == REGNO (operands[0]) + 1)
	return "add\t%0, %1, #0\n\tadd\t%H0, %H1, #0";
      return "add\t%H0, %H1, #0\n\tadd\t%0, %1, #0";
    case 1:
      return "ldmia\t%1, {%0, %H0}";
    case 2:
      return "stmia\t%0, {%1, %H1}";
    case 3:
      return thumb_load_double_from_address (operands);
    case 4:
      operands[2] = gen_rtx (MEM, SImode,
			     plus_constant (XEXP (operands[0], 0), 4));
      output_asm_insn ("str\t%1, %0\n\tstr\t%H1, %2", operands);
      return "";
    case 5:
      if (REGNO (operands[1]) == REGNO (operands[0]) + 1)
	return "mov\t%0, %1\n\tmov\t%H0, %H1";
      return "mov\t%H0, %H1\n\tmov\t%0, %1";
    }
  
}

static const char *output_199 PARAMS ((rtx *, rtx));

static const char *
output_199 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  switch (which_alternative)
    {
    default:
    case 0: return "mvf%?e\t%0, %1";
    case 1: return "mnf%?e\t%0, #%N1";
    case 2: return "ldf%?e\t%0, %1";
    case 3: return "stf%?e\t%1, %0";
    case 4: return output_mov_long_double_fpu_from_arm (operands);
    case 5: return output_mov_long_double_arm_from_fpu (operands);
    case 6: return output_mov_long_double_arm_from_arm (operands);
    }
  
}

static const char *output_212 PARAMS ((rtx *, rtx));

static const char *
output_212 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{
 return thumb_output_move_mem_multiple (3, operands);
}

static const char *output_213 PARAMS ((rtx *, rtx));

static const char *
output_213 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{
 return thumb_output_move_mem_multiple (2, operands);
}

static const char *output_214 PARAMS ((rtx *, rtx));

static const char *
output_214 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  output_asm_insn ("cmp\t%1, %2", operands);
  switch (get_attr_length (insn))
    {
    case 4:  return "b%d0\t%l3";
    case 6:  return "b%D0\t.LCB%=\n\tb\t%l3\t%@long jump\n.LCB%=:";
    default: return "b%D0\t.LCB%=\n\tbl\t%l3\t%@far jump\n.LCB%=:";
    }
  
}

static const char *output_215 PARAMS ((rtx *, rtx));

static const char *
output_215 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  output_asm_insn ("cmn\t%1, %2", operands);
  switch (get_attr_length (insn))
    {
    case 4:  return "b%d0\t%l3";
    case 6:  return "b%D0\t.LCB%=\n\tb\t%l3\t%@long jump\n.LCB%=:";
    default: return "b%D0\t.LCB%=\n\tbl\t%l3\t%@far jump\n.LCB%=:";
    }
  
}

static const char * const output_216[] = {
  "cmp%?\t%0, %1",
  "cmn%?\t%0, #%n1",
};

static const char * const output_220[] = {
  "cmf%?\t%0, %1",
  "cnf%?\t%0, #%N1",
};

static const char * const output_221[] = {
  "cmf%?\t%0, %1",
  "cnf%?\t%0, #%N1",
};

static const char * const output_222[] = {
  "cmf%?\t%0, %1",
  "cnf%?\t%0, #%N1",
};

static const char * const output_224[] = {
  "cmf%?\t%0, %1",
  "cnf%?\t%0, #%N1",
};

static const char * const output_225[] = {
  "cmf%?e\t%0, %1",
  "cnf%?e\t%0, #%N1",
};

static const char * const output_226[] = {
  "cmf%?e\t%0, %1",
  "cnf%?e\t%0, #%N1",
};

static const char * const output_227[] = {
  "cmf%?e\t%0, %1",
  "cnf%?e\t%0, #%N1",
};

static const char * const output_229[] = {
  "cmf%?e\t%0, %1",
  "cnf%?e\t%0, #%N1",
};

static const char *output_231 PARAMS ((rtx *, rtx));

static const char *
output_231 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (arm_ccfsm_state != 0)
    abort ();

  return "bvs\t%l0\n\tbeq\t%l0";
  
}

static const char *output_232 PARAMS ((rtx *, rtx));

static const char *
output_232 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (arm_ccfsm_state != 0)
    abort ();

  return "bmi\t%l0\n\tbgt\t%l0";
  
}

static const char *output_233 PARAMS ((rtx *, rtx));

static const char *
output_233 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (arm_ccfsm_state == 1 || arm_ccfsm_state == 2)
    {
      arm_ccfsm_state += 2;
      return "";
    }
  return "b%d1\t%l0";
  
}

static const char *output_234 PARAMS ((rtx *, rtx));

static const char *
output_234 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (arm_ccfsm_state != 0)
    abort ();

  return "bmi\t%l0\n\tbgt\t%l0";
  
}

static const char *output_235 PARAMS ((rtx *, rtx));

static const char *
output_235 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (arm_ccfsm_state != 0)
    abort ();

  return "bvs\t%l0\n\tbeq\t%l0";
  
}

static const char *output_236 PARAMS ((rtx *, rtx));

static const char *
output_236 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (arm_ccfsm_state == 1 || arm_ccfsm_state == 2)
    {
      arm_ccfsm_state += 2;
      return "";
    }
  return "b%D1\t%l0";
  
}

static const char * const output_240[] = {
  "mov%D3\t%0, %2",
  "mvn%D3\t%0, #%B2",
  "mov%d3\t%0, %1",
  "mvn%d3\t%0, #%B1",
  "mov%d3\t%0, %1\n\tmov%D3\t%0, %2",
  "mov%d3\t%0, %1\n\tmvn%D3\t%0, #%B2",
  "mvn%d3\t%0, #%B1\n\tmov%D3\t%0, %2",
  "mvn%d3\t%0, #%B1\n\tmvn%D3\t%0, #%B2",
};

static const char * const output_241[] = {
  "mvf%D3s\t%0, %2",
  "mnf%D3s\t%0, #%N2",
  "mvf%d3s\t%0, %1",
  "mnf%d3s\t%0, #%N1",
  "mvf%d3s\t%0, %1\n\tmvf%D3s\t%0, %2",
  "mvf%d3s\t%0, %1\n\tmnf%D3s\t%0, #%N2",
  "mnf%d3s\t%0, #%N1\n\tmvf%D3s\t%0, %2",
  "mnf%d3s\t%0, #%N1\n\tmnf%D3s\t%0, #%N2",
};

static const char * const output_242[] = {
  "mov%D3\t%0, %2",
  "mov%d3\t%0, %1",
};

static const char * const output_243[] = {
  "mvf%D3d\t%0, %2",
  "mnf%D3d\t%0, #%N2",
  "mvf%d3d\t%0, %1",
  "mnf%d3d\t%0, #%N1",
  "mvf%d3d\t%0, %1\n\tmvf%D3d\t%0, %2",
  "mvf%d3d\t%0, %1\n\tmnf%D3d\t%0, #%N2",
  "mnf%d3d\t%0, #%N1\n\tmvf%D3d\t%0, %2",
  "mnf%d3d\t%0, #%N1\n\tmnf%D3d\t%0, #%N2",
};

static const char *output_244 PARAMS ((rtx *, rtx));

static const char *
output_244 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    if (arm_ccfsm_state == 1 || arm_ccfsm_state == 2)
      {
        arm_ccfsm_state += 2;
        return "";
      }
    return "b%?\t%l0";
  }
  
}

static const char *output_245 PARAMS ((rtx *, rtx));

static const char *
output_245 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (get_attr_length (insn) == 2)
    return "b\t%l0";
  return "bl\t%l0\t%@ far jump";
  
}

static const char *output_246 PARAMS ((rtx *, rtx));

static const char *
output_246 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return output_call (operands);
  
}

static const char *output_247 PARAMS ((rtx *, rtx));

static const char *
output_247 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return output_call_mem (operands);
  
}

static const char *output_248 PARAMS ((rtx *, rtx));

static const char *
output_248 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    if (TARGET_CALLER_INTERWORKING)
      return "bl\t%__interwork_call_via_%0";
    else
      return "bl\t%__call_via_%0";
  }
}

static const char *output_249 PARAMS ((rtx *, rtx));

static const char *
output_249 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    if (TARGET_CALLER_INTERWORKING)
      return "bl\t%__interwork_call_via_%1";
    else
      return "bl\t%__call_via_%1";
  }
}

static const char *output_250 PARAMS ((rtx *, rtx));

static const char *
output_250 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return output_call (&operands[1]);
  
}

static const char *output_251 PARAMS ((rtx *, rtx));

static const char *
output_251 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return output_call_mem (&operands[1]);
  
}

static const char *output_252 PARAMS ((rtx *, rtx));

static const char *
output_252 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    return NEED_PLT_RELOC ? "bl%?\t%a0(PLT)" : "bl%?\t%a0";
  }
}

static const char *output_253 PARAMS ((rtx *, rtx));

static const char *
output_253 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    return NEED_PLT_RELOC ? "bl%?\t%a1(PLT)" : "bl%?\t%a1";
  }
}

static const char *output_256 PARAMS ((rtx *, rtx));

static const char *
output_256 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return NEED_PLT_RELOC ? "b%?\t%a0(PLT)" : "b%?\t%a0";
  
}

static const char *output_257 PARAMS ((rtx *, rtx));

static const char *
output_257 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return NEED_PLT_RELOC ? "b%?\t%a1(PLT)" : "b%?\t%a1";
  
}

static const char *output_258 PARAMS ((rtx *, rtx));

static const char *
output_258 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    if (arm_ccfsm_state == 2)
      {
        arm_ccfsm_state += 2;
        return "";
      }
    return output_return_instruction (const_true_rtx, TRUE, FALSE);
  }
}

static const char *output_259 PARAMS ((rtx *, rtx));

static const char *
output_259 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    if (arm_ccfsm_state == 2)
      {
        arm_ccfsm_state += 2;
        return "";
      }
    return output_return_instruction (operands[0], TRUE, FALSE);
  }
}

static const char *output_260 PARAMS ((rtx *, rtx));

static const char *
output_260 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    if (arm_ccfsm_state == 2)
      {
        arm_ccfsm_state += 2;
        return "";
      }
    return output_return_instruction (operands[0], TRUE, TRUE);
  }
}

static const char *output_263 PARAMS ((rtx *, rtx));

static const char *
output_263 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    if (flag_pic)
      return "cmp\t%0, %1\n\taddls\t%|pc, %|pc, %0, asl #2\n\tb\t%l3";
    return   "cmp\t%0, %1\n\tldrls\t%|pc, [%|pc, %0, asl #2]\n\tb\t%l3";
  
}

static const char *output_267 PARAMS ((rtx *, rtx));

static const char *
output_267 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (TARGET_ARM)
    return "mov%?\t%|r0, %|r0\t%@ nop";
  return  "mov\tr8, r8";
  
}

static const char * const output_275[] = {
  "orr%d2\t%0, %1, #1",
  "mov%D2\t%0, %1\n\torr%d2\t%0, %1, #1",
};

static const char *output_276 PARAMS ((rtx *, rtx));

static const char *
output_276 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    if (GET_CODE (operands[1]) == LT && operands[3] == const0_rtx)
      return "mov\t%0, %2, lsr #31";

    if (GET_CODE (operands[1]) == GE && operands[3] == const0_rtx)
      return "mvn\t%0, %2\n\tmov\t%0, %0, lsr #31";

    if (GET_CODE (operands[1]) == NE)
      {
        if (which_alternative == 1)
	  return "adds\t%0, %2, #%n3\n\tmovne\t%0, #1";
        return "subs\t%0, %2, %3\n\tmovne\t%0, #1";
      }
    if (which_alternative == 1)
      output_asm_insn ("cmn\t%2, #%n3", operands);
    else
      output_asm_insn ("cmp\t%2, %3", operands);
    return "mov%D1\t%0, #0\n\tmov%d1\t%0, #1";
  
}

static const char *output_277 PARAMS ((rtx *, rtx));

static const char *
output_277 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    if (GET_CODE (operands[3]) == NE)
      {
        if (which_alternative != 1)
	  output_asm_insn ("mov%D4\t%0, %2", operands);
        if (which_alternative != 0)
	  output_asm_insn ("mov%d4\t%0, %1", operands);
        return "";
      }
    if (which_alternative != 0)
      output_asm_insn ("mov%D4\t%0, %1", operands);
    if (which_alternative != 1)
      output_asm_insn ("mov%d4\t%0, %2", operands);
    return "";
  
}

static const char *output_278 PARAMS ((rtx *, rtx));

static const char *
output_278 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    if (GET_CODE (operands[4]) == LT && operands[3] == const0_rtx)
      return "%i5\t%0, %1, %2, lsr #31";

    output_asm_insn ("cmp\t%2, %3", operands);
    if (GET_CODE (operands[5]) == AND)
      output_asm_insn ("mov%D4\t%0, #0", operands);
    else if (GET_CODE (operands[5]) == MINUS)
      output_asm_insn ("rsb%D4\t%0, %1, #0", operands);
    else if (which_alternative != 0)
      output_asm_insn ("mov%D4\t%0, %1", operands);
    return "%i5%d4\t%0, %1, #1";
  
}

static const char *output_279 PARAMS ((rtx *, rtx));

static const char *
output_279 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    output_asm_insn ("cmp\t%2, %3", operands);
    if (which_alternative != 0)
      output_asm_insn ("mov%D4\t%0, %1", operands);
    return "sub%d4\t%0, %1, #1";
  
}

static const char *output_280 PARAMS ((rtx *, rtx));

static const char *
output_280 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    static const char * const opcodes[4][2] =
    {
      {"cmp\t%2, %3\n\tcmp%d5\t%0, %1",
       "cmp\t%0, %1\n\tcmp%d4\t%2, %3"},
      {"cmp\t%2, %3\n\tcmn%d5\t%0, #%n1",
       "cmn\t%0, #%n1\n\tcmp%d4\t%2, %3"},
      {"cmn\t%2, #%n3\n\tcmp%d5\t%0, %1",
       "cmp\t%0, %1\n\tcmn%d4\t%2, #%n3"},
      {"cmn\t%2, #%n3\n\tcmn%d5\t%0, #%n1",
       "cmn\t%0, #%n1\n\tcmn%d4\t%2, #%n3"}
    };
    int swap =
      comparison_dominates_p (GET_CODE (operands[5]), GET_CODE (operands[4]));

    return opcodes[which_alternative][swap];
  }
}

static const char *output_281 PARAMS ((rtx *, rtx));

static const char *
output_281 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    static const char * const opcodes[4][2] =
    {
      {"cmp\t%0, %1\n\tcmp%d4\t%2, %3",
       "cmp\t%2, %3\n\tcmp%D5\t%0, %1"},
      {"cmn\t%0, #%n1\n\tcmp%d4\t%2, %3",
       "cmp\t%2, %3\n\tcmn%D5\t%0, #%n1"},
      {"cmp\t%0, %1\n\tcmn%d4\t%2, #%n3",
       "cmn\t%2, #%n3\n\tcmp%D5\t%0, %1"},
      {"cmn\t%0, #%n1\n\tcmn%d4\t%2, #%n3",
       "cmn\t%2, #%n3\n\tcmn%D5\t%0, #%n1"}
    };
    int swap =
      comparison_dominates_p (GET_CODE (operands[5]),
			      reverse_condition (GET_CODE (operands[4])));

    return opcodes[which_alternative][swap];
  }
}

static const char *output_282 PARAMS ((rtx *, rtx));

static const char *
output_282 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    static const char *const opcodes[4][2] =
    {
      {"cmp\t%2, %3\n\tcmp%d5\t%0, %1",
       "cmp\t%0, %1\n\tcmp%d4\t%2, %3"},
      {"cmp\t%2, %3\n\tcmn%d5\t%0, #%n1",
       "cmn\t%0, #%n1\n\tcmp%d4\t%2, %3"},
      {"cmn\t%2, #%n3\n\tcmp%d5\t%0, %1",
       "cmp\t%0, %1\n\tcmn%d4\t%2, #%n3"},
      {"cmn\t%2, #%n3\n\tcmn%d5\t%0, #%n1",
       "cmn\t%0, #%n1\n\tcmn%d4\t%2, #%n3"}
    };
    int swap =
      comparison_dominates_p (GET_CODE (operands[5]), GET_CODE (operands[4]));

    return opcodes[which_alternative][swap];
  }
}

static const char *output_283 PARAMS ((rtx *, rtx));

static const char *
output_283 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

{
  static const char *const opcodes[4][2] =
  {
    {"cmp\t%0, %1\n\tcmp%D4\t%2, %3",
     "cmp\t%2, %3\n\tcmp%D5\t%0, %1"},
    {"cmn\t%0, #%n1\n\tcmp%D4\t%2, %3",
     "cmp\t%2, %3\n\tcmn%D5\t%0, #%n1"},
    {"cmp\t%0, %1\n\tcmn%D4\t%2, #%n3",
     "cmn\t%2, #%n3\n\tcmp%D5\t%0, %1"},
    {"cmn\t%0, #%n1\n\tcmn%D4\t%2, #%n3",
     "cmn\t%2, #%n3\n\tcmn%D5\t%0, #%n1"}
  };
  int swap =
    comparison_dominates_p (GET_CODE (operands[5]), GET_CODE (operands[4]));

  return opcodes[which_alternative][swap];
}

}

static const char *output_284 PARAMS ((rtx *, rtx));

static const char *
output_284 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (GET_CODE (operands[3]) == LT && operands[3] == const0_rtx)
    return "mov\t%0, %1, asr #31";

  if (GET_CODE (operands[3]) == NE)
    return "subs\t%0, %1, %2\n\tmvnne\t%0, #0";

  if (GET_CODE (operands[3]) == GT)
    return "subs\t%0, %1, %2\n\tmvnne\t%0, %0, asr #31";

  output_asm_insn ("cmp\t%1, %2", operands);
  output_asm_insn ("mov%D3\t%0, #0", operands);
  return "mvn%d3\t%0, #0";
  
}

static const char *output_285 PARAMS ((rtx *, rtx));

static const char *
output_285 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (GET_CODE (operands[5]) == LT
      && (operands[4] == const0_rtx))
    {
      if (which_alternative != 1 && GET_CODE (operands[1]) == REG)
	{
	  if (operands[2] == const0_rtx)
	    return "and\t%0, %1, %3, asr #31";
	  return "ands\t%0, %1, %3, asr #32\n\tmovcc\t%0, %2";
	}
      else if (which_alternative != 0 && GET_CODE (operands[2]) == REG)
	{
	  if (operands[1] == const0_rtx)
	    return "bic\t%0, %2, %3, asr #31";
	  return "bics\t%0, %2, %3, asr #32\n\tmovcs\t%0, %1";
	}
      /* The only case that falls through to here is when both ops 1 & 2
	 are constants */
    }

  if (GET_CODE (operands[5]) == GE
      && (operands[4] == const0_rtx))
    {
      if (which_alternative != 1 && GET_CODE (operands[1]) == REG)
	{
	  if (operands[2] == const0_rtx)
	    return "bic\t%0, %1, %3, asr #31";
	  return "bics\t%0, %1, %3, asr #32\n\tmovcs\t%0, %2";
	}
      else if (which_alternative != 0 && GET_CODE (operands[2]) == REG)
	{
	  if (operands[1] == const0_rtx)
	    return "and\t%0, %2, %3, asr #31";
	  return "ands\t%0, %2, %3, asr #32\n\tmovcc\t%0, %1";
	}
      /* The only case that falls through to here is when both ops 1 & 2
	 are constants */
    }
  if (GET_CODE (operands[4]) == CONST_INT
      && !const_ok_for_arm (INTVAL (operands[4])))
    output_asm_insn ("cmn\t%3, #%n4", operands);
  else
    output_asm_insn ("cmp\t%3, %4", operands);
  if (which_alternative != 0)
    output_asm_insn ("mov%d5\t%0, %1", operands);
  if (which_alternative != 1)
    output_asm_insn ("mov%D5\t%0, %2", operands);
  return "";
  
}

static const char * const output_287[] = {
  "add%d4\t%0, %2, %3",
  "sub%d4\t%0, %2, #%n3",
  "add%d4\t%0, %2, %3\n\tmov%D4\t%0, %1",
  "sub%d4\t%0, %2, #%n3\n\tmov%D4\t%0, %1",
};

static const char * const output_289[] = {
  "add%D4\t%0, %2, %3",
  "sub%D4\t%0, %2, #%n3",
  "add%D4\t%0, %2, %3\n\tmov%d4\t%0, %1",
  "sub%D4\t%0, %2, #%n3\n\tmov%d4\t%0, %1",
};

static const char *output_292 PARAMS ((rtx *, rtx));

static const char *
output_292 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  /* If we have an operation where (op x 0) is the identity operation and
     the conditional operator is LT or GE and we are comparing against zero and
     everything is in registers then we can do this in two instructions */
  if (operands[3] == const0_rtx
      && GET_CODE (operands[7]) != AND
      && GET_CODE (operands[5]) == REG
      && GET_CODE (operands[1]) == REG 
      && REGNO (operands[1]) == REGNO (operands[4])
      && REGNO (operands[4]) != REGNO (operands[0]))
    {
      if (GET_CODE (operands[6]) == LT)
	return "and\t%0, %5, %2, asr #31\n\t%I7\t%0, %4, %0";
      else if (GET_CODE (operands[6]) == GE)
	return "bic\t%0, %5, %2, asr #31\n\t%I7\t%0, %4, %0";
    }
  if (GET_CODE (operands[3]) == CONST_INT
      && !const_ok_for_arm (INTVAL (operands[3])))
    output_asm_insn ("cmn\t%2, #%n3", operands);
  else
    output_asm_insn ("cmp\t%2, %3", operands);
  output_asm_insn ("%I7%d6\t%0, %4, %5", operands);
  if (which_alternative != 0)
    return "mov%D6\t%0, %1";
  return "";
  
}

static const char * const output_293[] = {
  "%I5%d4\t%0, %2, %3",
  "%I5%d4\t%0, %2, %3\n\tmov%D4\t%0, %1",
};

static const char *output_294 PARAMS ((rtx *, rtx));

static const char *
output_294 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  /* If we have an operation where (op x 0) is the identity operation and
     the conditional operator is LT or GE and we are comparing against zero and
     everything is in registers then we can do this in two instructions */
  if (operands[5] == const0_rtx
      && GET_CODE (operands[7]) != AND
      && GET_CODE (operands[3]) == REG
      && GET_CODE (operands[1]) == REG 
      && REGNO (operands[1]) == REGNO (operands[2])
      && REGNO (operands[2]) != REGNO (operands[0]))
    {
      if (GET_CODE (operands[6]) == GE)
	return "and\t%0, %3, %4, asr #31\n\t%I7\t%0, %2, %0";
      else if (GET_CODE (operands[6]) == LT)
	return "bic\t%0, %3, %4, asr #31\n\t%I7\t%0, %2, %0";
    }

  if (GET_CODE (operands[5]) == CONST_INT
      && !const_ok_for_arm (INTVAL (operands[5])))
    output_asm_insn ("cmn\t%4, #%n5", operands);
  else
    output_asm_insn ("cmp\t%4, %5", operands);

  if (which_alternative != 0)
    output_asm_insn ("mov%d6\t%0, %1", operands);
  return "%I7%D6\t%0, %2, %3";
  
}

static const char * const output_295[] = {
  "%I5%D4\t%0, %2, %3",
  "%I5%D4\t%0, %2, %3\n\tmov%d4\t%0, %1",
};

static const char * const output_297[] = {
  "mvn%D4\t%0, %2",
  "mov%d4\t%0, %1\n\tmvn%D4\t%0, %2",
  "mvn%d4\t%0, #%B1\n\tmvn%D4\t%0, %2",
};

static const char * const output_299[] = {
  "mvn%d4\t%0, %2",
  "mov%D4\t%0, %1\n\tmvn%d4\t%0, %2",
  "mvn%D4\t%0, #%B1\n\tmvn%d4\t%0, %2",
};

static const char * const output_301[] = {
  "mov%d5\t%0, %2%S4",
  "mov%D5\t%0, %1\n\tmov%d5\t%0, %2%S4",
  "mvn%D5\t%0, #%B1\n\tmov%d5\t%0, %2%S4",
};

static const char * const output_303[] = {
  "mov%D5\t%0, %2%S4",
  "mov%d5\t%0, %1\n\tmov%D5\t%0, %2%S4",
  "mvn%d5\t%0, #%B1\n\tmov%D5\t%0, %2%S4",
};

static const char * const output_311[] = {
  "rsb%d4\t%0, %2, #0",
  "mov%D4\t%0, %1\n\trsb%d4\t%0, %2, #0",
  "mvn%D4\t%0, #%B1\n\trsb%d4\t%0, %2, #0",
};

static const char * const output_313[] = {
  "rsb%D4\t%0, %2, #0",
  "mov%d4\t%0, %1\n\trsb%D4\t%0, %2, #0",
  "mvn%d4\t%0, #%B1\n\trsb%D4\t%0, %2, #0",
};

static const char *output_314 PARAMS ((rtx *, rtx));

static const char *
output_314 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    rtx ldm[3];
    rtx arith[4];
    int val1 = 0, val2 = 0;

    if (REGNO (operands[0]) > REGNO (operands[4]))
      {
	ldm[1] = operands[4];
	ldm[2] = operands[0];
      }
    else
      {
	ldm[1] = operands[0];
	ldm[2] = operands[4];
      }
    if (GET_CODE (XEXP (operands[2], 0)) != REG)
      val1 = INTVAL (XEXP (XEXP (operands[2], 0), 1));
    if (GET_CODE (XEXP (operands[3], 0)) != REG)
      val2 = INTVAL (XEXP (XEXP (operands[3], 0), 1));
    arith[0] = operands[0];
    arith[3] = operands[1];
    if (val1 < val2)
      {
	arith[1] = ldm[1];
	arith[2] = ldm[2];
      }
    else
      {
	arith[1] = ldm[2];
	arith[2] = ldm[1];
      }
   if (val1 && val2)
      {
	rtx ops[3];
	ldm[0] = ops[0] = operands[4];
	ops[1] = XEXP (XEXP (operands[2], 0), 0);
	ops[2] = XEXP (XEXP (operands[2], 0), 1);
	output_add_immediate (ops);
	if (val1 < val2)
	  output_asm_insn ("ldm%?ia\t%0, {%1, %2}", ldm);
	else
	  output_asm_insn ("ldm%?da\t%0, {%1, %2}", ldm);
      }
    else if (val1)
      {
	ldm[0] = XEXP (operands[3], 0);
	if (val1 < val2)
	  output_asm_insn ("ldm%?da\t%0, {%1, %2}", ldm);
	else
	  output_asm_insn ("ldm%?ia\t%0, {%1, %2}", ldm);
      }
    else
      {
	ldm[0] = XEXP (operands[2], 0);
	if (val1 < val2)
	  output_asm_insn ("ldm%?ia\t%0, {%1, %2}", ldm);
	else
	  output_asm_insn ("ldm%?da\t%0, {%1, %2}", ldm);
      }
    output_asm_insn ("%I3%?\t%0, %1, %2", arith);
    return "";
  }
}

static const char *output_337 PARAMS ((rtx *, rtx));

static const char *
output_337 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (USE_RETURN_INSN (FALSE))
    return output_return_instruction (const_true_rtx, FALSE, FALSE);
  return arm_output_epilogue (FALSE);
  
}

static const char *output_338 PARAMS ((rtx *, rtx));

static const char *
output_338 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  if (TARGET_ARM)
    return arm_output_epilogue (TRUE);
  else /* TARGET_THUMB */
    return thumb_unexpanded_epilogue ();
  
}

static const char * const output_339[] = {
  "mvn%D4\t%0, %2",
  "mov%d4\t%0, %1\n\tmvn%D4\t%0, %2",
};

static const char *output_340 PARAMS ((rtx *, rtx));

static const char *
output_340 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    operands[2] = GEN_INT (1 << INTVAL (operands[2]));
    output_asm_insn ("ands\t%0, %1, %2", operands);
    return "mvnne\t%0, #0";
  
}

static const char *output_341 PARAMS ((rtx *, rtx));

static const char *
output_341 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    operands[2] = GEN_INT (1 << INTVAL (operands[2]));
    output_asm_insn ("tst\t%1, %2", operands);
    output_asm_insn ("mvneq\t%0, #0", operands);
    return "movne\t%0, #0";
  
}

static const char *output_342 PARAMS ((rtx *, rtx));

static const char *
output_342 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    int num_saves = XVECLEN (operands[2], 0);
     
    /* For the StrongARM at least it is faster to
       use STR to store only a single register.  */
    if (num_saves == 1)
      output_asm_insn ("str\t%1, [%m0, #-4]!", operands);
    else
      {
	int i;
	char pattern[100];

	strcpy (pattern, "stmfd\t%m0!, {%1");

	for (i = 1; i < num_saves; i++)
	  {
	    strcat (pattern, ", %|");
	    strcat (pattern,
		    reg_names[REGNO (XEXP (XVECEXP (operands[2], 0, i), 0))]);
	  }

	strcat (pattern, "}");
	output_asm_insn (pattern, operands);
      }

    return "";
  }
}

static const char *output_344 PARAMS ((rtx *, rtx));

static const char *
output_344 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    char pattern[100];

    sprintf (pattern, "sfmfd\t%%1, %d, [%%m0]!", XVECLEN (operands[2], 0));
    output_asm_insn (pattern, operands);
    return "";
  }
}

static const char *output_345 PARAMS ((rtx *, rtx));

static const char *
output_345 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  assemble_align (32);
  return "";
  
}

static const char *output_346 PARAMS ((rtx *, rtx));

static const char *
output_346 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  making_const_table = FALSE;
  return "";
  
}

static const char *output_347 PARAMS ((rtx *, rtx));

static const char *
output_347 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  making_const_table = TRUE;
  assemble_integer (operands[0], 1, BITS_PER_WORD, 1);
  assemble_zeros (3);
  return "";
  
}

static const char *output_348 PARAMS ((rtx *, rtx));

static const char *
output_348 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  making_const_table = TRUE;
  assemble_integer (operands[0], 2, BITS_PER_WORD, 1);
  assemble_zeros (2);
  return "";
  
}

static const char *output_349 PARAMS ((rtx *, rtx));

static const char *
output_349 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    making_const_table = TRUE;
    switch (GET_MODE_CLASS (GET_MODE (operands[0])))
      {
      case MODE_FLOAT:
      {
        REAL_VALUE_TYPE r;
        REAL_VALUE_FROM_CONST_DOUBLE (r, operands[0]);
        assemble_real (r, GET_MODE (operands[0]), BITS_PER_WORD);
        break;
      }
      default:
        assemble_integer (operands[0], 4, BITS_PER_WORD, 1);
        break;
      }
    return "";
  }
}

static const char *output_350 PARAMS ((rtx *, rtx));

static const char *
output_350 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    making_const_table = TRUE;
    switch (GET_MODE_CLASS (GET_MODE (operands[0])))
      {
       case MODE_FLOAT:
        {
          REAL_VALUE_TYPE r;
          REAL_VALUE_FROM_CONST_DOUBLE (r, operands[0]);
          assemble_real (r, GET_MODE (operands[0]), BITS_PER_WORD);
          break;
        }
      default:
        assemble_integer (operands[0], 8, BITS_PER_WORD, 1);
        break;
      }
    return "";
  }
}

static const char *output_490 PARAMS ((rtx *, rtx));

static const char *
output_490 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return emit_ldm_seq (operands, 4);
  
}

static const char *output_491 PARAMS ((rtx *, rtx));

static const char *
output_491 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return emit_ldm_seq (operands, 3);
  
}

static const char *output_492 PARAMS ((rtx *, rtx));

static const char *
output_492 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return emit_ldm_seq (operands, 2);
  
}

static const char *output_493 PARAMS ((rtx *, rtx));

static const char *
output_493 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return emit_stm_seq (operands, 4);
  
}

static const char *output_494 PARAMS ((rtx *, rtx));

static const char *
output_494 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return emit_stm_seq (operands, 3);
  
}

static const char *output_495 PARAMS ((rtx *, rtx));

static const char *
output_495 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  return emit_stm_seq (operands, 2);
  
}

static const char * const output_507[] = {
  "add%?\t%0, %1, %2",
  "sub%?\t%0, %1, #%n2",
  "#",
};

static const char * const output_508[] = {
  "adf%?s\t%0, %1, %2",
  "suf%?s\t%0, %1, #%N2",
};

static const char * const output_509[] = {
  "adf%?d\t%0, %1, %2",
  "suf%?d\t%0, %1, #%N2",
};

static const char * const output_510[] = {
  "adf%?d\t%0, %1, %2",
  "suf%?d\t%0, %1, #%N2",
};

static const char * const output_513[] = {
  "adf%?e\t%0, %1, %2",
  "suf%?e\t%0, %1, #%N2",
};

static const char * const output_514[] = {
  "rsb%?\t%0, %2, %1",
  "#",
};

static const char * const output_515[] = {
  "suf%?d\t%0, %1, %2",
  "rsf%?d\t%0, %2, %1",
};

static const char * const output_517[] = {
  "suf%?d\t%0, %1, %2",
  "rsf%?d\t%0, %2, %1",
};

static const char * const output_519[] = {
  "suf%?e\t%0, %1, %2",
  "rsf%?e\t%0, %2, %1",
};

static const char * const output_534[] = {
  "fdv%?s\t%0, %1, %2",
  "frd%?s\t%0, %2, %1",
};

static const char * const output_535[] = {
  "dvf%?d\t%0, %1, %2",
  "rdf%?d\t%0, %2, %1",
};

static const char * const output_539[] = {
  "dvf%?e\t%0, %1, %2",
  "rdf%?e\t%0, %2, %1",
};

static const char * const output_546[] = {
  "and%?\t%0, %1, %2",
  "bic%?\t%0, %1, #%B2",
  "#",
};

static const char * const output_548[] = {
  "bic%?\t%Q0, %Q1, %2",
  "#",
};

static const char * const output_553[] = {
  "orr%?\t%Q0, %Q1, %2",
  "#",
};

static const char * const output_555[] = {
  "orr%?\t%0, %1, %2",
  "#",
};

static const char * const output_557[] = {
  "eor%?\t%Q0, %Q1, %2",
  "#",
};

static const char *output_587 PARAMS ((rtx *, rtx));

static const char *
output_587 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    if (REGNO (operands[1])
        != REGNO (operands[0]) + (WORDS_BIG_ENDIAN ? 1 : 0))
      output_asm_insn ("mov%?\t%Q0, %1", operands);
    return "mov%?\t%R0, #0";
  
}

static const char * const output_588[] = {
  "and%?\t%Q0, %1, #255\n\tmov%?\t%R0, #0",
  "ldr%?b\t%Q0, %1\n\tmov%?\t%R0, #0",
};

static const char *output_589 PARAMS ((rtx *, rtx));

static const char *
output_589 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    if (REGNO (operands[1])
        != REGNO (operands[0]) + (WORDS_BIG_ENDIAN ? 1 : 0))
      output_asm_insn ("mov%?\t%Q0, %1", operands);
    return "mov%?\t%R0, %Q0, asr #31";
  
}

static const char *output_593 PARAMS ((rtx *, rtx));

static const char *
output_593 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  /* If the address is invalid, this will split the instruction into two. */
  if (bad_signed_byte_operand (operands[1], VOIDmode))
    return "#";
  return "ldr%?sb\t%0, %1";
  
}

static const char *output_594 PARAMS ((rtx *, rtx));

static const char *
output_594 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  /* If the address is invalid, this will split the instruction into two. */
  if (bad_signed_byte_operand (operands[1], VOIDmode))
    return "#";
  return "ldr%?sb\t%0, %1";
  
}

static const char * const output_598[] = {
  "mov%?\t%0, %1",
  "mvn%?\t%0, #%B1",
  "ldr%?\t%0, %1",
  "str%?\t%1, %0",
};

static const char *output_599 PARAMS ((rtx *, rtx));

static const char *
output_599 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

    ASM_OUTPUT_INTERNAL_LABEL (asm_out_file, "L",
			       CODE_LABEL_NUMBER (operands[1]));
    return "add%?\t%0, %|pc, %0";
  
}

static const char *output_600 PARAMS ((rtx *, rtx));

static const char *
output_600 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    rtx ops[2];

    ops[0] = operands[0];
    ops[1] = gen_rtx_MEM (SImode, plus_constant (XEXP (operands[1], 0), 2));
    output_asm_insn ("ldr%?\t%0, %1\t%@ load-rotate", ops);
    return "";
  }
}

static const char * const output_601[] = {
  "mov%?\t%0, %1\t%@ movhi",
  "mvn%?\t%0, #%B1\t%@ movhi",
  "str%?h\t%1, %0\t%@ movhi ",
  "ldr%?h\t%0, %1\t%@ movhi",
};

static const char * const output_602[] = {
  "mov%?\t%0, %1\t%@ movhi",
  "mvn%?\t%0, #%B1\t%@ movhi",
  "ldr%?\t%0, %1\t%@ movhi",
};

static const char * const output_603[] = {
  "mov%?\t%0, %1\t%@ movhi",
  "mvn%?\t%0, #%B1\t%@ movhi",
  "ldr%?\t%0, %1\t%@ movhi_bigend\n\tmov%?\t%0, %0, asr #16",
};

static const char * const output_605[] = {
  "mov%?\t%0, %1\t%@ movhi",
  "mvn%?\t%0, #%B1\t%@ movhi",
};

static const char * const output_606[] = {
  "mov%?\t%0, %1",
  "mvn%?\t%0, #%B1",
  "ldr%?b\t%0, %1",
  "str%?b\t%1, %0",
};

static const char * const output_607[] = {
  "mvf%?s\t%0, %1",
  "mnf%?s\t%0, #%N1",
  "ldf%?s\t%0, %1",
  "stf%?s\t%1, %0",
  "str%?\t%1, [%|sp, #-4]!\n\tldf%?s\t%0, [%|sp], #4",
  "stf%?s\t%1, [%|sp, #-4]!\n\tldr%?\t%0, [%|sp], #4",
  "mov%?\t%0, %1",
  "ldr%?\t%0, %1\t%@ float",
  "str%?\t%1, %0\t%@ float",
};

static const char * const output_608[] = {
  "mov%?\t%0, %1",
  "ldr%?\t%0, %1\t%@ float",
  "str%?\t%1, %0\t%@ float",
};

static const char *output_609 PARAMS ((rtx *, rtx));

static const char *
output_609 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
  switch (which_alternative)
    {
    default:
    case 0: return "ldm%?ia\t%m1, %M0\t%@ double";
    case 1: return "stm%?ia\t%m0, %M1\t%@ double";
    case 2: case 3: case 4: return output_move_double (operands);
    case 5: return "mvf%?d\t%0, %1";
    case 6: return "mnf%?d\t%0, #%N1";
    case 7: return "ldf%?d\t%0, %1";
    case 8: return "stf%?d\t%1, %0";
    case 9: return output_mov_double_fpu_from_arm (operands);
    case 10: return output_mov_double_arm_from_fpu (operands);
    }
  }
  
}

static const char *output_610 PARAMS ((rtx *, rtx));

static const char *
output_610 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  switch (which_alternative)
    {
    default:
    case 0: return "mvf%?e\t%0, %1";
    case 1: return "mnf%?e\t%0, #%N1";
    case 2: return "ldf%?e\t%0, %1";
    case 3: return "stf%?e\t%1, %0";
    case 4: return output_mov_long_double_fpu_from_arm (operands);
    case 5: return output_mov_long_double_arm_from_fpu (operands);
    case 6: return output_mov_long_double_arm_from_arm (operands);
    }
  
}

static const char *output_623 PARAMS ((rtx *, rtx));

static const char *
output_623 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    if (arm_ccfsm_state == 1 || arm_ccfsm_state == 2)
      {
        arm_ccfsm_state += 2;
        return "";
      }
    return "b%?\t%l0";
  }
  
}

static const char *output_624 PARAMS ((rtx *, rtx));

static const char *
output_624 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    if (arm_ccfsm_state == 2)
      {
        arm_ccfsm_state += 2;
        return "";
      }
    return output_return_instruction (const_true_rtx, TRUE, FALSE);
  }
}

static const char *output_629 PARAMS ((rtx *, rtx));

static const char *
output_629 (operands, insn)
     rtx *operands ATTRIBUTE_UNUSED;
     rtx insn ATTRIBUTE_UNUSED;
{

  {
    rtx ldm[3];
    rtx arith[4];
    int val1 = 0, val2 = 0;

    if (REGNO (operands[0]) > REGNO (operands[4]))
      {
	ldm[1] = operands[4];
	ldm[2] = operands[0];
      }
    else
      {
	ldm[1] = operands[0];
	ldm[2] = operands[4];
      }
    if (GET_CODE (XEXP (operands[2], 0)) != REG)
      val1 = INTVAL (XEXP (XEXP (operands[2], 0), 1));
    if (GET_CODE (XEXP (operands[3], 0)) != REG)
      val2 = INTVAL (XEXP (XEXP (operands[3], 0), 1));
    arith[0] = operands[0];
    arith[3] = operands[1];
    if (val1 < val2)
      {
	arith[1] = ldm[1];
	arith[2] = ldm[2];
      }
    else
      {
	arith[1] = ldm[2];
	arith[2] = ldm[1];
      }
   if (val1 && val2)
      {
	rtx ops[3];
	ldm[0] = ops[0] = operands[4];
	ops[1] = XEXP (XEXP (operands[2], 0), 0);
	ops[2] = XEXP (XEXP (operands[2], 0), 1);
	output_add_immediate (ops);
	if (val1 < val2)
	  output_asm_insn ("ldm%?ia\t%0, {%1, %2}", ldm);
	else
	  output_asm_insn ("ldm%?da\t%0, {%1, %2}", ldm);
      }
    else if (val1)
      {
	ldm[0] = XEXP (operands[3], 0);
	if (val1 < val2)
	  output_asm_insn ("ldm%?da\t%0, {%1, %2}", ldm);
	else
	  output_asm_insn ("ldm%?ia\t%0, {%1, %2}", ldm);
      }
    else
      {
	ldm[0] = XEXP (operands[2], 0);
	if (val1 < val2)
	  output_asm_insn ("ldm%?ia\t%0, {%1, %2}", ldm);
	else
	  output_asm_insn ("ldm%?da\t%0, {%1, %2}", ldm);
      }
    output_asm_insn ("%I3%?\t%0, %1, %2", arith);
    return "";
  }
}


extern int register_operand PARAMS ((rtx, machine_mode_t));
extern int s_register_operand PARAMS ((rtx, machine_mode_t));
extern int reg_or_int_operand PARAMS ((rtx, machine_mode_t));
extern int nonmemory_operand PARAMS ((rtx, machine_mode_t));
extern int arm_add_operand PARAMS ((rtx, machine_mode_t));
extern int arm_rhs_operand PARAMS ((rtx, machine_mode_t));
extern int shift_operator PARAMS ((rtx, machine_mode_t));
extern int arm_comparison_operator PARAMS ((rtx, machine_mode_t));
extern int cc_register PARAMS ((rtx, machine_mode_t));
extern int fpu_add_operand PARAMS ((rtx, machine_mode_t));
extern int fpu_rhs_operand PARAMS ((rtx, machine_mode_t));
extern int scratch_operand PARAMS ((rtx, machine_mode_t));
extern int arm_not_operand PARAMS ((rtx, machine_mode_t));
extern int const_int_operand PARAMS ((rtx, machine_mode_t));
extern int memory_operand PARAMS ((rtx, machine_mode_t));
extern int minmax_operator PARAMS ((rtx, machine_mode_t));
extern int shiftable_operator PARAMS ((rtx, machine_mode_t));
extern int nonimmediate_operand PARAMS ((rtx, machine_mode_t));
extern int nonimmediate_di_operand PARAMS ((rtx, machine_mode_t));
extern int di_operand PARAMS ((rtx, machine_mode_t));
extern int general_operand PARAMS ((rtx, machine_mode_t));
extern int offsettable_memory_operand PARAMS ((rtx, machine_mode_t));
extern int nonimmediate_soft_df_operand PARAMS ((rtx, machine_mode_t));
extern int soft_df_operand PARAMS ((rtx, machine_mode_t));
extern int load_multiple_operation PARAMS ((rtx, machine_mode_t));
extern int arm_hard_register_operand PARAMS ((rtx, machine_mode_t));
extern int store_multiple_operation PARAMS ((rtx, machine_mode_t));
extern int equality_operator PARAMS ((rtx, machine_mode_t));
extern int dominant_cc_register PARAMS ((rtx, machine_mode_t));
extern int index_operand PARAMS ((rtx, machine_mode_t));
extern int const_shift_operand PARAMS ((rtx, machine_mode_t));
extern int multi_register_push PARAMS ((rtx, machine_mode_t));
extern int f_register_operand PARAMS ((rtx, machine_mode_t));
extern int address_operand PARAMS ((rtx, machine_mode_t));
extern int logical_binary_operator PARAMS ((rtx, machine_mode_t));
extern int alignable_memory_operand PARAMS ((rtx, machine_mode_t));
extern int bad_signed_byte_operand PARAMS ((rtx, machine_mode_t));
extern int arm_reload_memory_operand PARAMS ((rtx, machine_mode_t));
extern int immediate_operand PARAMS ((rtx, machine_mode_t));



static const struct insn_operand_data operand_data[] = 
{
  {
    0,
    "",
    VOIDmode,
    0,
    0
  },
  {
    register_operand,
    "=l",
    DImode,
    0,
    1
  },
  {
    register_operand,
    "%0",
    DImode,
    0,
    1
  },
  {
    register_operand,
    "l",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0,0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r,r,r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI,L,?n",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "=l,l,l,*r,*h,l,!k",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "%0,0,l,*0,*0,!k,!k",
    SImode,
    0,
    1
  },
  {
    nonmemory_operand,
    "I,J,lL,*h,*r,!M,!O",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rI,L",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,?r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    SImode,
    0,
    0
  },
  {
    cc_register,
    "",
    CCmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "%f,f",
    SFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "%f,f",
    DFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f,f",
    SFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f,f",
    XFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,r,0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,0,0",
    DImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    DImode,
    0,
    1
  },
  {
    register_operand,
    "0",
    DImode,
    0,
    1
  },
  {
    register_operand,
    "l",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "?r,0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "l",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI,?n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "r,I",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,?r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    SImode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    SFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "f,G",
    SFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG,f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "f,G",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG,f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "f,G",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f,f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    XFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "f,G",
    XFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG,f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%?r,0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "=&l,&l,&l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "%l,*h,0",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "l,l,l",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=&r,&r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "%?r,0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r,&r,&r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r,0,r,0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "?r,r,0,0",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=&r,&r,&r,&r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "%r,0,r,0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "?r,r,0,0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r,0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=&r,&r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0,r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI,K,?n",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "l",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_not_operand,
    "rI,K",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=X,r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "n",
    VOIDmode,
    0,
    1
  },
  {
    const_int_operand,
    "n",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,?r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    scratch_operand,
    "=r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI,?n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "%r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r,&r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,0",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,0,rI",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,rI,rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,r,?r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,0,rI",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "=m",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    minmax_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,?r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,rI",
    SImode,
    0,
    1
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    minmax_operator,
    "",
    SImode,
    0,
    0
  },
  {
    register_operand,
    "=l,l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "l,0",
    SImode,
    0,
    1
  },
  {
    nonmemory_operand,
    "N,l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "l",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    scratch_operand,
    "=r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=&r,r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "?r,0",
    DImode,
    0,
    1
  },
  {
    register_operand,
    "=&l",
    DImode,
    0,
    1
  },
  {
    register_operand,
    "l",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,&r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    DImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "r,m",
    QImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    HImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    QImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    HImode,
    0,
    1
  },
  {
    scratch_operand,
    "=&l",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    QImode,
    0,
    1
  },
  {
    register_operand,
    "=l,l",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "V,m",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    nonimmediate_di_operand,
    "=r,r,o<>",
    DImode,
    0,
    1
  },
  {
    di_operand,
    "rIK,mi,r",
    DImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=l,l,l,l,>,l,m,*r",
    DImode,
    0,
    1
  },
  {
    general_operand,
    "l,I,J,>,l,mi,l,*r",
    DImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,r,r,m",
    SImode,
    0,
    1
  },
  {
    general_operand,
    "rI,K,mi,r",
    SImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=l,l,l,l,l,>,l,m,*lh",
    SImode,
    0,
    1
  },
  {
    general_operand,
    "l,I,J,K,>,l,mi,l,*lh",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    0,
    "mX",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    0,
    "mX",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    VOIDmode,
    0,
    1
  },
  {
    register_operand,
    "+r",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,r",
    SImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=l,l,m,*r,*h,l",
    HImode,
    0,
    1
  },
  {
    general_operand,
    "l,mn,l,*h,*r,I",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    offsettable_memory_operand,
    "o",
    SImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,r,m,r",
    HImode,
    0,
    1
  },
  {
    general_operand,
    "rI,K,r,m",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r,r",
    HImode,
    0,
    1
  },
  {
    general_operand,
    "rI,K,m",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    HImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,K",
    HImode,
    0,
    1
  },
  {
    memory_operand,
    "=m",
    HImode,
    0,
    1
  },
  {
    register_operand,
    "l",
    HImode,
    0,
    1
  },
  {
    register_operand,
    "=&l",
    SImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,r,r,m",
    QImode,
    0,
    1
  },
  {
    general_operand,
    "rI,K,m,r",
    QImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=l,l,m,*r,*h,l",
    QImode,
    0,
    1
  },
  {
    general_operand,
    "l,m,l,*h,*r,I",
    QImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=f,f,f,m,f,r,r,r,m",
    SFmode,
    0,
    1
  },
  {
    general_operand,
    "fG,H,mE,f,r,f,r,mE,r",
    SFmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,r,m",
    SFmode,
    0,
    1
  },
  {
    general_operand,
    "r,mE,r",
    SFmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=l,l,>,l,m,*r,*h",
    SFmode,
    0,
    1
  },
  {
    general_operand,
    "l,>,l,mF,l,*h,*r",
    SFmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,Q,r,m,r,f,f,f,m,!f,!r",
    DFmode,
    0,
    1
  },
  {
    general_operand,
    "Q,r,r,r,mF,fG,H,mF,f,r,f",
    DFmode,
    0,
    1
  },
  {
    nonimmediate_soft_df_operand,
    "=r,r,m",
    DFmode,
    0,
    1
  },
  {
    soft_df_operand,
    "r,mF,r",
    DFmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=l,l,>,l,m,*r",
    DFmode,
    0,
    1
  },
  {
    general_operand,
    "l,>,l,mF,l,*r",
    DFmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=f,f,f,m,f,r,r",
    XFmode,
    0,
    1
  },
  {
    general_operand,
    "fG,H,m,f,r,f,r",
    XFmode,
    0,
    1
  },
  {
    load_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "1",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    load_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    store_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "1",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    store_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "1",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=&l",
    SImode,
    0,
    0
  },
  {
    scratch_operand,
    "=&l",
    SImode,
    0,
    0
  },
  {
    scratch_operand,
    "=&l",
    SImode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    register_operand,
    "l,r",
    SImode,
    0,
    1
  },
  {
    nonmemory_operand,
    "rI,r",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    register_operand,
    "l",
    SImode,
    0,
    1
  },
  {
    nonmemory_operand,
    "l",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "f,f",
    SFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f,f",
    DFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    DFmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    SImode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r,r,r,r,r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_not_operand,
    "0,0,rI,K,rI,rI,K,K",
    SImode,
    0,
    1
  },
  {
    arm_not_operand,
    "rI,K,0,0,rI,K,rI,K",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f,f,f,f,f,f,f",
    SFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "0,0,fG,H,fG,fG,H,H",
    SFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H,0,0,fG,H,fG,H",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "0,r",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "r,0",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f,f,f,f,f,f,f",
    DFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "0,0,fG,H,fG,fG,H,H",
    DFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H,0,0,fG,H,fG,H",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    register_operand,
    "l*r",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "=l",
    VOIDmode,
    0,
    1
  },
  {
    register_operand,
    "l*r",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "=r,f",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "=r,f",
    VOIDmode,
    0,
    1
  },
  {
    memory_operand,
    "m,m",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "X",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,f",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "X,X",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "X",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    register_operand,
    "=l",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "X",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,f",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "X,X",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    cc_register,
    "",
    CC_NOOVmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=r",
    SImode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rI,L",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "0,rI,?rI",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,0,rI",
    SImode,
    0,
    1
  },
  {
    equality_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,?r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,rI",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    SImode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r,r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rI,L,rI,L",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rI,rI,L,L",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    SImode,
    0,
    0
  },
  {
    dominant_cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "0,rI,?rI",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,0,rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL,rIL,rIL",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "0,?rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL,rIL",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL,rIL",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r,r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "0,0,?rI,?rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rI,L,rI,L",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL",
    SImode,
    0,
    1
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "0,?rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL,rIL",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,rI",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "0,?rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,rI",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "0,?rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL,rIL",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    arm_not_operand,
    "0,?rIK",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL,rIL",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_not_operand,
    "0,?rI,K",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r",
    SImode,
    0,
    1
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    arm_not_operand,
    "0,?rIK",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM,rM",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL,rIL",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_not_operand,
    "0,?rI,K",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM,rM,rM",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "rIL",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "0,?rI",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    memory_operand,
    "=m",
    BLKmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    multi_register_push,
    "",
    VOIDmode,
    0,
    0
  },
  {
    memory_operand,
    "=m",
    BLKmode,
    0,
    1
  },
  {
    f_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    multi_register_push,
    "",
    VOIDmode,
    0,
    0
  },
  {
    address_operand,
    "p",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "r",
    SImode,
    0,
    0
  },
  {
    register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "r",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    0
  },
  {
    0,
    "",
    VOIDmode,
    0,
    0
  },
  {
    0,
    "",
    VOIDmode,
    0,
    0
  },
  {
    logical_binary_operator,
    "",
    DImode,
    0,
    0
  },
  {
    s_register_operand,
    "",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    0
  },
  {
    0,
    "",
    VOIDmode,
    0,
    0
  },
  {
    0,
    "",
    VOIDmode,
    0,
    0
  },
  {
    logical_binary_operator,
    "",
    DImode,
    0,
    0
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    general_operand,
    "",
    SImode,
    0,
    1
  },
  {
    general_operand,
    "",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    alignable_memory_operand,
    "",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    HImode,
    0,
    1
  },
  {
    general_operand,
    "",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    HImode,
    0,
    1
  },
  {
    bad_signed_byte_operand,
    "",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    general_operand,
    "",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    bad_signed_byte_operand,
    "",
    QImode,
    0,
    1
  },
  {
    general_operand,
    "",
    DImode,
    0,
    1
  },
  {
    general_operand,
    "",
    DImode,
    0,
    1
  },
  {
    memory_operand,
    "",
    HImode,
    0,
    1
  },
  {
    general_operand,
    "",
    HImode,
    0,
    1
  },
  {
    general_operand,
    "",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    HImode,
    0,
    1
  },
  {
    memory_operand,
    "",
    HImode,
    0,
    1
  },
  {
    arm_reload_memory_operand,
    "=o",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&l",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    arm_reload_memory_operand,
    "o",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r",
    DImode,
    0,
    1
  },
  {
    general_operand,
    "",
    QImode,
    0,
    1
  },
  {
    general_operand,
    "",
    QImode,
    0,
    1
  },
  {
    general_operand,
    "",
    SFmode,
    0,
    1
  },
  {
    general_operand,
    "",
    SFmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "",
    SFmode,
    0,
    1
  },
  {
    immediate_operand,
    "",
    SFmode,
    0,
    1
  },
  {
    general_operand,
    "",
    DFmode,
    0,
    1
  },
  {
    general_operand,
    "",
    DFmode,
    0,
    1
  },
  {
    arm_reload_memory_operand,
    "=o",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r",
    SImode,
    0,
    1
  },
  {
    general_operand,
    "",
    XFmode,
    0,
    1
  },
  {
    general_operand,
    "",
    XFmode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    general_operand,
    "",
    BLKmode,
    0,
    1
  },
  {
    general_operand,
    "",
    BLKmode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    XFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    1
  },
  {
    arm_not_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_not_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SFmode,
    0,
    1
  },
  {
    nonmemory_operand,
    "",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    DFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "",
    DFmode,
    0,
    1
  },
  {
    memory_operand,
    "",
    VOIDmode,
    0,
    1
  },
  {
    general_operand,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    memory_operand,
    "",
    VOIDmode,
    0,
    1
  },
  {
    general_operand,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    register_operand,
    "",
    VOIDmode,
    0,
    1
  },
  {
    memory_operand,
    "",
    VOIDmode,
    0,
    1
  },
  {
    general_operand,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "+r",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    QImode,
    0,
    1
  },
  {
    s_register_operand,
    "+r",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    s_register_operand,
    "+r",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "+r",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_int_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "+r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    QImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "=m",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "=m",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "=m",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "=m",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "",
    QImode,
    0,
    1
  },
  {
    const_int_operand,
    "",
    VOIDmode,
    0,
    1
  },
  {
    scratch_operand,
    "",
    SImode,
    0,
    0
  },
  {
    register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_add_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r,r,r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI,L,?n",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "%f,f",
    SFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "%f,f",
    DFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f,f",
    SFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f,f",
    XFmode,
    0,
    1
  },
  {
    fpu_add_operand,
    "fG,H",
    XFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI,?n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "f,G",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG,f",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "f,G",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f,f",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    XFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "f,G",
    XFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG,f",
    XFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%?r,0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r,&r,&r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r,0,r,0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "?r,r,0,0",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%r,0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=&r,&r",
    SImode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    XFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f,f",
    SFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "f,G",
    SFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG,f",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    fpu_rhs_operand,
    "fG",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI,K,?n",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,r",
    DImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,?r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "0,r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0,r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    DImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "?r,0",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI,?n",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r,&r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r,r,0",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,0,rI",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,rI,rI",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=&r,&r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "?r,0",
    DImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    SFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    DFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    XFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    DImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    DImode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "r,m",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    HImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=f",
    XFmode,
    0,
    1
  },
  {
    s_register_operand,
    "f",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,r,r,m",
    SImode,
    0,
    1
  },
  {
    general_operand,
    "rI,K,mi,r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    register_operand,
    "+r",
    SImode,
    0,
    1
  },
  {
    0,
    "",
    VOIDmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    offsettable_memory_operand,
    "o",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,r,m,r",
    HImode,
    0,
    1
  },
  {
    general_operand,
    "rI,K,r,m",
    HImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r,r",
    HImode,
    0,
    1
  },
  {
    general_operand,
    "rI,K,m",
    HImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r,r",
    HImode,
    0,
    1
  },
  {
    arm_rhs_operand,
    "rI,K",
    HImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,r,r,m",
    QImode,
    0,
    1
  },
  {
    general_operand,
    "rI,K,m,r",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=f,f,f,m,f,r,r,r,m",
    SFmode,
    0,
    1
  },
  {
    general_operand,
    "fG,H,mE,f,r,f,r,mE,r",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,r,m",
    SFmode,
    0,
    1
  },
  {
    general_operand,
    "r,mE,r",
    SFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=r,Q,r,m,r,f,f,f,m,!f,!r",
    DFmode,
    0,
    1
  },
  {
    general_operand,
    "Q,r,r,r,mF,fG,H,mF,f,r,f",
    DFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    nonimmediate_operand,
    "=f,f,f,m,f,r,r",
    XFmode,
    0,
    1
  },
  {
    general_operand,
    "fG,H,m,f,r,f,r",
    XFmode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    load_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "1",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    load_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "1",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    load_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "1",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    load_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    load_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    load_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    store_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "1",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    store_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "1",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    store_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "1",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    store_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    store_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    store_multiple_operation,
    "",
    VOIDmode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_hard_register_operand,
    "",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rI",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    reg_or_int_operand,
    "rM",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    shiftable_operator,
    "",
    SImode,
    0,
    0
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    memory_operand,
    "m",
    SImode,
    0,
    1
  },
  {
    scratch_operand,
    "=r",
    SImode,
    0,
    0
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "%0",
    SImode,
    0,
    1
  },
  {
    index_operand,
    "rJ",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    QImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "0",
    SImode,
    0,
    1
  },
  {
    shift_operator,
    "",
    SImode,
    0,
    0
  },
  {
    s_register_operand,
    "r",
    SImode,
    0,
    1
  },
  {
    const_shift_operand,
    "n",
    SImode,
    0,
    1
  },
  {
    s_register_operand,
    "=r",
    HImode,
    0,
    1
  },
  {
    arm_comparison_operator,
    "",
    VOIDmode,
    0,
    0
  },
  {
    cc_register,
    "",
    VOIDmode,
    0,
    1
  },
};



const struct insn_data insn_data[] = 
{
  {
    "*thumb_adddi3",
    "add\t%Q0, %Q0, %Q2\n\tadc\t%R0, %R0, %R2",
    0,
    &operand_data[1],
    3,
    0,
    1,
    1
  },
  {
    "*arm_adddi3",
    "#",
    0,
    &operand_data[4],
    3,
    0,
    2,
    1
  },
  {
    "*adddi_sesidi_di",
    "#",
    0,
    &operand_data[7],
    3,
    0,
    2,
    1
  },
  {
    "*adddi_zesidi_di",
    "#",
    0,
    &operand_data[7],
    3,
    0,
    2,
    1
  },
  {
    "*arm_addsi3",
    (const PTR) output_4,
    0,
    &operand_data[10],
    3,
    0,
    3,
    2
  },
  {
    "*thumb_addsi3",
    (const PTR) output_5,
    0,
    &operand_data[13],
    3,
    0,
    7,
    3
  },
  {
    "*addsi3_compare0",
    (const PTR) output_6,
    0,
    &operand_data[16],
    3,
    2,
    2,
    2
  },
  {
    "*addsi3_compare0_scratch",
    (const PTR) output_7,
    0,
    &operand_data[17],
    2,
    0,
    2,
    2
  },
  {
    "*addsi3_compare0_for_combiner",
    (const PTR) output_8,
    0,
    &operand_data[16],
    3,
    2,
    2,
    2
  },
  {
    "*addsi3_compare0_scratch_for_combiner",
    (const PTR) output_9,
    0,
    &operand_data[17],
    2,
    0,
    2,
    2
  },
  {
    "*addsi3_compare_op1",
    (const PTR) output_10,
    0,
    &operand_data[16],
    3,
    3,
    2,
    2
  },
  {
    "*addsi3_compare_op2",
    (const PTR) output_11,
    0,
    &operand_data[16],
    3,
    3,
    2,
    2
  },
  {
    "*compare_addsi2_op0",
    (const PTR) output_12,
    0,
    &operand_data[17],
    2,
    1,
    2,
    2
  },
  {
    "*compare_addsi2_op1",
    (const PTR) output_13,
    0,
    &operand_data[17],
    2,
    1,
    2,
    2
  },
  {
    "*addsi3_carryin",
    "adc%?\t%0, %1, %2",
    0,
    &operand_data[19],
    3,
    0,
    1,
    1
  },
  {
    "*addsi3_carryin_shift",
    "adc%?\t%0, %1, %3%S2",
    0,
    &operand_data[22],
    5,
    0,
    0,
    1
  },
  {
    "*addsi3_carryin_alt1",
    "adc%?\t%0, %1, %2",
    0,
    &operand_data[19],
    3,
    0,
    1,
    1
  },
  {
    "*addsi3_carryin_alt2",
    "adc%?\t%0, %1, %2",
    0,
    &operand_data[19],
    3,
    0,
    1,
    1
  },
  {
    "*addsi3_carryin_alt3",
    "adc%?\t%0, %1, %2",
    0,
    &operand_data[19],
    3,
    0,
    1,
    1
  },
  {
    "incscc",
    (const PTR) output_19,
    (insn_gen_fn) gen_incscc,
    &operand_data[27],
    4,
    0,
    2,
    2
  },
  {
    "addsf3",
    (const PTR) output_20,
    (insn_gen_fn) gen_addsf3,
    &operand_data[31],
    3,
    0,
    2,
    2
  },
  {
    "adddf3",
    (const PTR) output_21,
    (insn_gen_fn) gen_adddf3,
    &operand_data[34],
    3,
    0,
    2,
    2
  },
  {
    "*adddf_esfdf_df",
    (const PTR) output_22,
    0,
    &operand_data[37],
    3,
    0,
    2,
    2
  },
  {
    "*adddf_df_esfdf",
    "adf%?d\t%0, %1, %2",
    0,
    &operand_data[40],
    3,
    0,
    1,
    1
  },
  {
    "*adddf_esfdf_esfdf",
    "adf%?d\t%0, %1, %2",
    0,
    &operand_data[43],
    3,
    0,
    1,
    1
  },
  {
    "addxf3",
    (const PTR) output_25,
    (insn_gen_fn) gen_addxf3,
    &operand_data[46],
    3,
    0,
    2,
    2
  },
  {
    "*arm_subdi3",
    "subs\t%Q0, %Q1, %Q2\n\tsbc\t%R0, %R1, %R2",
    0,
    &operand_data[49],
    3,
    0,
    3,
    1
  },
  {
    "*thumb_subdi3",
    "sub\t%Q0, %Q0, %Q2\n\tsbc\t%R0, %R0, %R2",
    0,
    &operand_data[52],
    3,
    0,
    1,
    1
  },
  {
    "*subdi_di_zesidi",
    "subs\t%Q0, %Q1, %2\n\tsbc\t%R0, %R1, #0",
    0,
    &operand_data[55],
    3,
    0,
    2,
    1
  },
  {
    "*subdi_di_sesidi",
    "subs\t%Q0, %Q1, %2\n\tsbc\t%R0, %R1, %2, asr #31",
    0,
    &operand_data[7],
    3,
    0,
    2,
    1
  },
  {
    "*subdi_zesidi_di",
    "rsbs\t%Q0, %Q1, %2\n\trsc\t%R0, %R1, #0",
    0,
    &operand_data[55],
    3,
    0,
    2,
    1
  },
  {
    "*subdi_sesidi_di",
    "rsbs\t%Q0, %Q1, %2\n\trsc\t%R0, %R1, %2, asr #31",
    0,
    &operand_data[55],
    3,
    0,
    2,
    1
  },
  {
    "*subdi_zesidi_zesidi",
    "subs\t%Q0, %1, %2\n\trsc\t%R0, %1, %1",
    0,
    &operand_data[58],
    3,
    0,
    1,
    1
  },
  {
    "*thumb_subsi3_insn",
    "sub\t%0, %1, %2",
    0,
    &operand_data[61],
    3,
    0,
    1,
    1
  },
  {
    "*arm_subsi3_insn",
    (const PTR) output_34,
    0,
    &operand_data[64],
    3,
    0,
    2,
    2
  },
  {
    "*subsi3_compare0",
    (const PTR) output_35,
    0,
    &operand_data[67],
    3,
    2,
    2,
    2
  },
  {
    "decscc",
    (const PTR) output_36,
    (insn_gen_fn) gen_decscc,
    &operand_data[70],
    4,
    0,
    2,
    2
  },
  {
    "subsf3",
    (const PTR) output_37,
    (insn_gen_fn) gen_subsf3,
    &operand_data[74],
    3,
    0,
    2,
    2
  },
  {
    "subdf3",
    (const PTR) output_38,
    (insn_gen_fn) gen_subdf3,
    &operand_data[77],
    3,
    0,
    2,
    2
  },
  {
    "*subdf_esfdf_df",
    "suf%?d\t%0, %1, %2",
    0,
    &operand_data[80],
    3,
    0,
    1,
    1
  },
  {
    "*subdf_df_esfdf",
    (const PTR) output_40,
    0,
    &operand_data[83],
    3,
    0,
    2,
    2
  },
  {
    "*subdf_esfdf_esfdf",
    "suf%?d\t%0, %1, %2",
    0,
    &operand_data[43],
    3,
    0,
    1,
    1
  },
  {
    "subxf3",
    (const PTR) output_42,
    (insn_gen_fn) gen_subxf3,
    &operand_data[86],
    3,
    0,
    2,
    2
  },
  {
    "*arm_mulsi3",
    "mul%?\t%0, %2, %1",
    0,
    &operand_data[89],
    3,
    0,
    2,
    1
  },
  {
    "*thumb_mulsi3",
    (const PTR) output_44,
    0,
    &operand_data[92],
    3,
    0,
    3,
    3
  },
  {
    "*mulsi3_compare0",
    "mul%?s\t%0, %2, %1",
    0,
    &operand_data[89],
    3,
    2,
    2,
    1
  },
  {
    "*mulsi_compare0_scratch",
    "mul%?s\t%0, %2, %1",
    0,
    &operand_data[95],
    3,
    0,
    2,
    1
  },
  {
    "*mulsi3addsi",
    "mla%?\t%0, %2, %1, %3",
    0,
    &operand_data[98],
    4,
    0,
    4,
    1
  },
  {
    "*mulsi3addsi_compare0",
    "mla%?s\t%0, %2, %1, %3",
    0,
    &operand_data[98],
    4,
    3,
    4,
    1
  },
  {
    "*mulsi3addsi_compare0_scratch",
    "mla%?s\t%0, %2, %1, %3",
    0,
    &operand_data[102],
    4,
    0,
    4,
    1
  },
  {
    "*mulsidi3adddi",
    "smlal%?\t%Q0, %R0, %3, %2",
    0,
    &operand_data[106],
    4,
    0,
    1,
    1
  },
  {
    "mulsidi3",
    "smull%?\t%Q0, %R0, %1, %2",
    (insn_gen_fn) gen_mulsidi3,
    &operand_data[110],
    3,
    0,
    1,
    1
  },
  {
    "umulsidi3",
    "umull%?\t%Q0, %R0, %1, %2",
    (insn_gen_fn) gen_umulsidi3,
    &operand_data[110],
    3,
    0,
    1,
    1
  },
  {
    "*umulsidi3adddi",
    "umlal%?\t%Q0, %R0, %3, %2",
    0,
    &operand_data[106],
    4,
    0,
    1,
    1
  },
  {
    "smulsi3_highpart",
    "smull%?\t%3, %0, %2, %1",
    (insn_gen_fn) gen_smulsi3_highpart,
    &operand_data[113],
    4,
    0,
    2,
    1
  },
  {
    "umulsi3_highpart",
    "umull%?\t%3, %0, %2, %1",
    (insn_gen_fn) gen_umulsi3_highpart,
    &operand_data[113],
    4,
    0,
    2,
    1
  },
  {
    "mulhisi3",
    "smulbb%?\t%0, %1, %2",
    (insn_gen_fn) gen_mulhisi3,
    &operand_data[117],
    3,
    0,
    1,
    1
  },
  {
    "*mulhisi3addsi",
    "smlabb%?\t%0, %2, %3, %1",
    0,
    &operand_data[120],
    4,
    0,
    1,
    1
  },
  {
    "*mulhidi3adddi",
    "smlalbb%?\t%Q0, %R0, %2, %3",
    0,
    &operand_data[124],
    4,
    0,
    1,
    1
  },
  {
    "mulsf3",
    "fml%?s\t%0, %1, %2",
    (insn_gen_fn) gen_mulsf3,
    &operand_data[128],
    3,
    0,
    1,
    1
  },
  {
    "muldf3",
    "muf%?d\t%0, %1, %2",
    (insn_gen_fn) gen_muldf3,
    &operand_data[131],
    3,
    0,
    1,
    1
  },
  {
    "*muldf_esfdf_df",
    "muf%?d\t%0, %1, %2",
    0,
    &operand_data[80],
    3,
    0,
    1,
    1
  },
  {
    "*muldf_df_esfdf",
    "muf%?d\t%0, %1, %2",
    0,
    &operand_data[40],
    3,
    0,
    1,
    1
  },
  {
    "*muldf_esfdf_esfdf",
    "muf%?d\t%0, %1, %2",
    0,
    &operand_data[43],
    3,
    0,
    1,
    1
  },
  {
    "mulxf3",
    "muf%?e\t%0, %1, %2",
    (insn_gen_fn) gen_mulxf3,
    &operand_data[134],
    3,
    0,
    1,
    1
  },
  {
    "divsf3",
    (const PTR) output_65,
    (insn_gen_fn) gen_divsf3,
    &operand_data[74],
    3,
    0,
    2,
    2
  },
  {
    "divdf3",
    (const PTR) output_66,
    (insn_gen_fn) gen_divdf3,
    &operand_data[77],
    3,
    0,
    2,
    2
  },
  {
    "*divdf_esfdf_df",
    "dvf%?d\t%0, %1, %2",
    0,
    &operand_data[80],
    3,
    0,
    1,
    1
  },
  {
    "*divdf_df_esfdf",
    "rdf%?d\t%0, %2, %1",
    0,
    &operand_data[137],
    3,
    0,
    1,
    1
  },
  {
    "*divdf_esfdf_esfdf",
    "dvf%?d\t%0, %1, %2",
    0,
    &operand_data[43],
    3,
    0,
    1,
    1
  },
  {
    "divxf3",
    (const PTR) output_70,
    (insn_gen_fn) gen_divxf3,
    &operand_data[86],
    3,
    0,
    2,
    2
  },
  {
    "modsf3",
    "rmf%?s\t%0, %1, %2",
    (insn_gen_fn) gen_modsf3,
    &operand_data[128],
    3,
    0,
    1,
    1
  },
  {
    "moddf3",
    "rmf%?d\t%0, %1, %2",
    (insn_gen_fn) gen_moddf3,
    &operand_data[131],
    3,
    0,
    1,
    1
  },
  {
    "*moddf_esfdf_df",
    "rmf%?d\t%0, %1, %2",
    0,
    &operand_data[80],
    3,
    0,
    1,
    1
  },
  {
    "*moddf_df_esfdf",
    "rmf%?d\t%0, %1, %2",
    0,
    &operand_data[40],
    3,
    0,
    1,
    1
  },
  {
    "*moddf_esfdf_esfdf",
    "rmf%?d\t%0, %1, %2",
    0,
    &operand_data[43],
    3,
    0,
    1,
    1
  },
  {
    "modxf3",
    "rmf%?e\t%0, %1, %2",
    (insn_gen_fn) gen_modxf3,
    &operand_data[134],
    3,
    0,
    1,
    1
  },
  {
    "anddi3",
    "#",
    (insn_gen_fn) gen_anddi3,
    &operand_data[140],
    3,
    0,
    2,
    1
  },
  {
    "*anddi_zesidi_di",
    "#",
    0,
    &operand_data[55],
    3,
    0,
    2,
    1
  },
  {
    "*anddi_sesdi_di",
    "#",
    0,
    &operand_data[55],
    3,
    0,
    2,
    1
  },
  {
    "*arm_andsi3_insn",
    (const PTR) output_80,
    0,
    &operand_data[143],
    3,
    0,
    3,
    2
  },
  {
    "*thumb_andsi3_insn",
    "and\t%0, %0, %2",
    0,
    &operand_data[146],
    3,
    0,
    1,
    1
  },
  {
    "*andsi3_compare0",
    (const PTR) output_82,
    0,
    &operand_data[149],
    3,
    2,
    2,
    2
  },
  {
    "*andsi3_compare0_scratch",
    (const PTR) output_83,
    0,
    &operand_data[150],
    3,
    0,
    2,
    2
  },
  {
    "*zeroextractsi_compare0_scratch",
    (const PTR) output_84,
    0,
    &operand_data[153],
    3,
    0,
    1,
    3
  },
  {
    "*ne_zeroextractsi",
    (const PTR) output_85,
    0,
    &operand_data[156],
    4,
    0,
    1,
    3
  },
  {
    "*anddi_notdi_di",
    "#",
    0,
    &operand_data[160],
    3,
    0,
    2,
    1
  },
  {
    "*anddi_notzesidi_di",
    (const PTR) output_87,
    0,
    &operand_data[163],
    3,
    0,
    2,
    2
  },
  {
    "*anddi_notsesidi_di",
    "#",
    0,
    &operand_data[166],
    3,
    0,
    2,
    1
  },
  {
    "andsi_notsi_si",
    "bic%?\t%0, %1, %2",
    (insn_gen_fn) gen_andsi_notsi_si,
    &operand_data[169],
    3,
    0,
    1,
    1
  },
  {
    "bicsi3",
    "bic\t%0, %0, %1",
    (insn_gen_fn) gen_bicsi3,
    &operand_data[172],
    3,
    0,
    1,
    1
  },
  {
    "andsi_not_shiftsi_si",
    "bic%?\t%0, %1, %2%S4",
    (insn_gen_fn) gen_andsi_not_shiftsi_si,
    &operand_data[175],
    5,
    0,
    1,
    1
  },
  {
    "*andsi_notsi_si_compare0",
    "bic%?s\t%0, %1, %2",
    0,
    &operand_data[169],
    3,
    2,
    1,
    1
  },
  {
    "*andsi_notsi_si_compare0_scratch",
    "bic%?s\t%0, %1, %2",
    0,
    &operand_data[180],
    3,
    0,
    1,
    1
  },
  {
    "iordi3",
    "#",
    (insn_gen_fn) gen_iordi3,
    &operand_data[140],
    3,
    0,
    2,
    1
  },
  {
    "*iordi_zesidi_di",
    (const PTR) output_95,
    0,
    &operand_data[163],
    3,
    0,
    2,
    2
  },
  {
    "*iordi_sesidi_di",
    "#",
    0,
    &operand_data[55],
    3,
    0,
    2,
    1
  },
  {
    "*arm_iorsi3",
    (const PTR) output_97,
    0,
    &operand_data[183],
    3,
    0,
    2,
    2
  },
  {
    "*thumb_iorsi3",
    "orr\t%0, %0, %2",
    0,
    &operand_data[146],
    3,
    0,
    1,
    1
  },
  {
    "*iorsi3_compare0",
    "orr%?s\t%0, %1, %2",
    0,
    &operand_data[186],
    3,
    2,
    1,
    1
  },
  {
    "*iorsi3_compare0_scratch",
    "orr%?s\t%0, %1, %2",
    0,
    &operand_data[189],
    3,
    0,
    1,
    1
  },
  {
    "xordi3",
    "#",
    (insn_gen_fn) gen_xordi3,
    &operand_data[140],
    3,
    0,
    2,
    1
  },
  {
    "*xordi_zesidi_di",
    (const PTR) output_102,
    0,
    &operand_data[163],
    3,
    0,
    2,
    2
  },
  {
    "*xordi_sesidi_di",
    "#",
    0,
    &operand_data[55],
    3,
    0,
    2,
    1
  },
  {
    "*arm_xorsi3",
    "eor%?\t%0, %1, %2",
    0,
    &operand_data[19],
    3,
    0,
    1,
    1
  },
  {
    "*thumb_xorsi3",
    "eor\t%0, %0, %2",
    0,
    &operand_data[146],
    3,
    0,
    1,
    1
  },
  {
    "*xorsi3_compare0",
    "eor%?s\t%0, %1, %2",
    0,
    &operand_data[19],
    3,
    2,
    1,
    1
  },
  {
    "*xorsi3_compare0_scratch",
    "teq%?\t%0, %1",
    0,
    &operand_data[20],
    2,
    0,
    1,
    1
  },
  {
    "*andsi_iorsi3_notsi",
    "orr%?\t%0, %1, %2\n\tbic%?\t%0, %0, %3",
    0,
    &operand_data[192],
    4,
    0,
    3,
    1
  },
  {
    "smaxsi3",
    (const PTR) output_109,
    (insn_gen_fn) gen_smaxsi3,
    &operand_data[196],
    3,
    0,
    3,
    2
  },
  {
    "sminsi3",
    (const PTR) output_110,
    (insn_gen_fn) gen_sminsi3,
    &operand_data[196],
    3,
    0,
    3,
    2
  },
  {
    "umaxsi3",
    (const PTR) output_111,
    (insn_gen_fn) gen_umaxsi3,
    &operand_data[196],
    3,
    0,
    3,
    2
  },
  {
    "uminsi3",
    (const PTR) output_112,
    (insn_gen_fn) gen_uminsi3,
    &operand_data[196],
    3,
    0,
    3,
    2
  },
  {
    "*store_minmaxsi",
    (const PTR) output_113,
    0,
    &operand_data[199],
    4,
    0,
    1,
    3
  },
  {
    "*minmax_arithsi",
    (const PTR) output_114,
    0,
    &operand_data[203],
    6,
    0,
    2,
    3
  },
  {
    "*thumb_ashlsi3",
    "lsl\t%0, %1, %2",
    0,
    &operand_data[209],
    3,
    0,
    2,
    1
  },
  {
    "*thumb_ashrsi3",
    "asr\t%0, %1, %2",
    0,
    &operand_data[209],
    3,
    0,
    2,
    1
  },
  {
    "*thumb_lshrsi3",
    "lsr\t%0, %1, %2",
    0,
    &operand_data[209],
    3,
    0,
    2,
    1
  },
  {
    "*thumb_rotrsi3",
    "ror\t%0, %0, %2",
    0,
    &operand_data[212],
    3,
    0,
    1,
    1
  },
  {
    "*arm_shiftsi3",
    "mov%?\t%0, %1%S3",
    0,
    &operand_data[215],
    4,
    0,
    1,
    1
  },
  {
    "*shiftsi3_compare0",
    "mov%?s\t%0, %1%S3",
    0,
    &operand_data[219],
    4,
    3,
    1,
    1
  },
  {
    "*shiftsi3_compare0_scratch",
    "mov%?s\t%0, %1%S3",
    0,
    &operand_data[223],
    4,
    0,
    1,
    1
  },
  {
    "*notsi_shiftsi",
    "mvn%?\t%0, %1%S3",
    0,
    &operand_data[219],
    4,
    0,
    1,
    1
  },
  {
    "*notsi_shiftsi_compare0",
    "mvn%?s\t%0, %1%S3",
    0,
    &operand_data[219],
    4,
    3,
    1,
    1
  },
  {
    "*not_shiftsi_compare0_scratch",
    "mvn%?s\t%0, %1%S3",
    0,
    &operand_data[223],
    4,
    0,
    1,
    1
  },
  {
    "*arm_negdi2",
    "rsbs\t%Q0, %Q1, #0\n\trsc\t%R0, %R1, #0",
    0,
    &operand_data[227],
    2,
    0,
    2,
    1
  },
  {
    "*thumb_negdi2",
    "mov\t%R0, #0\n\tneg\t%Q0, %Q1\n\tsbc\t%R0, %R1",
    0,
    &operand_data[229],
    2,
    0,
    1,
    1
  },
  {
    "*arm_negsi2",
    "rsb%?\t%0, %1, #0",
    0,
    &operand_data[19],
    2,
    0,
    1,
    1
  },
  {
    "*thumb_negsi2",
    "neg\t%0, %1",
    0,
    &operand_data[61],
    2,
    0,
    1,
    1
  },
  {
    "negsf2",
    "mnf%?s\t%0, %1",
    (insn_gen_fn) gen_negsf2,
    &operand_data[128],
    2,
    0,
    1,
    1
  },
  {
    "negdf2",
    "mnf%?d\t%0, %1",
    (insn_gen_fn) gen_negdf2,
    &operand_data[40],
    2,
    0,
    1,
    1
  },
  {
    "*negdf_esfdf",
    "mnf%?d\t%0, %1",
    0,
    &operand_data[43],
    2,
    0,
    1,
    1
  },
  {
    "negxf2",
    "mnf%?e\t%0, %1",
    (insn_gen_fn) gen_negxf2,
    &operand_data[134],
    2,
    0,
    1,
    1
  },
  {
    "abssi2",
    (const PTR) output_133,
    (insn_gen_fn) gen_abssi2,
    &operand_data[231],
    2,
    0,
    2,
    2
  },
  {
    "*neg_abssi2",
    (const PTR) output_134,
    0,
    &operand_data[231],
    2,
    0,
    2,
    2
  },
  {
    "abssf2",
    "abs%?s\t%0, %1",
    (insn_gen_fn) gen_abssf2,
    &operand_data[128],
    2,
    0,
    1,
    1
  },
  {
    "absdf2",
    "abs%?d\t%0, %1",
    (insn_gen_fn) gen_absdf2,
    &operand_data[40],
    2,
    0,
    1,
    1
  },
  {
    "*absdf_esfdf",
    "abs%?d\t%0, %1",
    0,
    &operand_data[43],
    2,
    0,
    1,
    1
  },
  {
    "absxf2",
    "abs%?e\t%0, %1",
    (insn_gen_fn) gen_absxf2,
    &operand_data[134],
    2,
    0,
    1,
    1
  },
  {
    "sqrtsf2",
    "sqt%?s\t%0, %1",
    (insn_gen_fn) gen_sqrtsf2,
    &operand_data[128],
    2,
    0,
    1,
    1
  },
  {
    "sqrtdf2",
    "sqt%?d\t%0, %1",
    (insn_gen_fn) gen_sqrtdf2,
    &operand_data[40],
    2,
    0,
    1,
    1
  },
  {
    "*sqrtdf_esfdf",
    "sqt%?d\t%0, %1",
    0,
    &operand_data[43],
    2,
    0,
    1,
    1
  },
  {
    "sqrtxf2",
    "sqt%?e\t%0, %1",
    (insn_gen_fn) gen_sqrtxf2,
    &operand_data[134],
    2,
    0,
    1,
    1
  },
  {
    "one_cmpldi2",
    "#",
    (insn_gen_fn) gen_one_cmpldi2,
    &operand_data[55],
    2,
    0,
    2,
    1
  },
  {
    "*arm_one_cmplsi2",
    "mvn%?\t%0, %1",
    0,
    &operand_data[19],
    2,
    0,
    1,
    1
  },
  {
    "*thumb_one_cmplsi2",
    "mvn\t%0, %1",
    0,
    &operand_data[61],
    2,
    0,
    1,
    1
  },
  {
    "*notsi_compare0",
    "mvn%?s\t%0, %1",
    0,
    &operand_data[19],
    2,
    1,
    1,
    1
  },
  {
    "*notsi_compare0_scratch",
    "mvn%?s\t%0, %1",
    0,
    &operand_data[180],
    2,
    0,
    1,
    1
  },
  {
    "floatsisf2",
    "flt%?s\t%0, %1",
    (insn_gen_fn) gen_floatsisf2,
    &operand_data[233],
    2,
    0,
    1,
    1
  },
  {
    "floatsidf2",
    "flt%?d\t%0, %1",
    (insn_gen_fn) gen_floatsidf2,
    &operand_data[235],
    2,
    0,
    1,
    1
  },
  {
    "floatsixf2",
    "flt%?e\t%0, %1",
    (insn_gen_fn) gen_floatsixf2,
    &operand_data[237],
    2,
    0,
    1,
    1
  },
  {
    "fix_truncsfsi2",
    "fix%?z\t%0, %1",
    (insn_gen_fn) gen_fix_truncsfsi2,
    &operand_data[239],
    2,
    0,
    1,
    1
  },
  {
    "fix_truncdfsi2",
    "fix%?z\t%0, %1",
    (insn_gen_fn) gen_fix_truncdfsi2,
    &operand_data[241],
    2,
    0,
    1,
    1
  },
  {
    "fix_truncxfsi2",
    "fix%?z\t%0, %1",
    (insn_gen_fn) gen_fix_truncxfsi2,
    &operand_data[243],
    2,
    0,
    1,
    1
  },
  {
    "truncdfsf2",
    "mvf%?s\t%0, %1",
    (insn_gen_fn) gen_truncdfsf2,
    &operand_data[245],
    2,
    0,
    1,
    1
  },
  {
    "truncxfsf2",
    "mvf%?s\t%0, %1",
    (insn_gen_fn) gen_truncxfsf2,
    &operand_data[247],
    2,
    0,
    1,
    1
  },
  {
    "truncxfdf2",
    "mvf%?d\t%0, %1",
    (insn_gen_fn) gen_truncxfdf2,
    &operand_data[249],
    2,
    0,
    1,
    1
  },
  {
    "zero_extendsidi2",
    (const PTR) output_157,
    (insn_gen_fn) gen_zero_extendsidi2,
    &operand_data[58],
    2,
    0,
    1,
    3
  },
  {
    "zero_extendqidi2",
    (const PTR) output_158,
    (insn_gen_fn) gen_zero_extendqidi2,
    &operand_data[251],
    2,
    0,
    2,
    2
  },
  {
    "extendsidi2",
    (const PTR) output_159,
    (insn_gen_fn) gen_extendsidi2,
    &operand_data[58],
    2,
    0,
    1,
    3
  },
  {
    "*thumb_zero_extendhisi2",
    (const PTR) output_160,
    0,
    &operand_data[253],
    2,
    0,
    1,
    3
  },
  {
    "*arm_zero_extendhisi2",
    "ldr%?h\t%0, %1",
    0,
    &operand_data[255],
    2,
    0,
    1,
    1
  },
  {
    "*thumb_zero_extendqisi2",
    "ldrb\t%0, %1",
    0,
    &operand_data[257],
    2,
    0,
    1,
    1
  },
  {
    "*arm_zero_extendqisi2",
    "ldr%?b\t%0, %1\t%@ zero_extendqisi2",
    0,
    &operand_data[259],
    2,
    0,
    1,
    1
  },
  {
    "*compareqi_eq0",
    "tst\t%0, #255",
    0,
    &operand_data[261],
    1,
    0,
    1,
    1
  },
  {
    "*thumb_extendhisi2_insn",
    (const PTR) output_165,
    0,
    &operand_data[262],
    3,
    0,
    1,
    3
  },
  {
    "*arm_extendhisi_insn",
    "ldr%?sh\t%0, %1",
    0,
    &operand_data[255],
    2,
    0,
    1,
    1
  },
  {
    "*extendqihi_insn",
    (const PTR) output_167,
    0,
    &operand_data[265],
    2,
    0,
    1,
    3
  },
  {
    "*arm_extendqisi_insn",
    (const PTR) output_168,
    0,
    &operand_data[259],
    2,
    0,
    1,
    3
  },
  {
    "*thumb_extendqisi2_insn",
    (const PTR) output_169,
    0,
    &operand_data[267],
    2,
    0,
    2,
    3
  },
  {
    "extendsfdf2",
    "mvf%?d\t%0, %1",
    (insn_gen_fn) gen_extendsfdf2,
    &operand_data[43],
    2,
    0,
    1,
    1
  },
  {
    "extendsfxf2",
    "mvf%?e\t%0, %1",
    (insn_gen_fn) gen_extendsfxf2,
    &operand_data[269],
    2,
    0,
    1,
    1
  },
  {
    "extenddfxf2",
    "mvf%?e\t%0, %1",
    (insn_gen_fn) gen_extenddfxf2,
    &operand_data[271],
    2,
    0,
    1,
    1
  },
  {
    "*arm_movdi",
    (const PTR) output_173,
    0,
    &operand_data[273],
    2,
    0,
    3,
    3
  },
  {
    "*thumb_movdi_insn",
    (const PTR) output_174,
    0,
    &operand_data[275],
    2,
    0,
    8,
    3
  },
  {
    "*arm_movsi_insn",
    (const PTR) output_175,
    0,
    &operand_data[277],
    2,
    0,
    4,
    2
  },
  {
    "*thumb_movsi_insn",
    (const PTR) output_176,
    0,
    &operand_data[279],
    2,
    0,
    9,
    2
  },
  {
    "pic_load_addr_arm",
    "ldr%?\t%0, %1",
    (insn_gen_fn) gen_pic_load_addr_arm,
    &operand_data[281],
    2,
    0,
    1,
    1
  },
  {
    "pic_load_addr_thumb",
    "ldr\t%0, %1",
    (insn_gen_fn) gen_pic_load_addr_thumb,
    &operand_data[283],
    2,
    0,
    1,
    1
  },
  {
    "*pic_load_addr_based_insn",
    (const PTR) output_179,
    0,
    &operand_data[285],
    3,
    0,
    1,
    3
  },
  {
    "pic_add_dot_plus_four",
    (const PTR) output_180,
    (insn_gen_fn) gen_pic_add_dot_plus_four,
    &operand_data[288],
    2,
    1,
    1,
    3
  },
  {
    "pic_add_dot_plus_eight",
    (const PTR) output_181,
    (insn_gen_fn) gen_pic_add_dot_plus_eight,
    &operand_data[288],
    2,
    1,
    1,
    3
  },
  {
    "*movsi_compare0",
    (const PTR) output_182,
    0,
    &operand_data[290],
    2,
    1,
    2,
    2
  },
  {
    "*thumb_movhi_insn",
    (const PTR) output_183,
    0,
    &operand_data[292],
    2,
    0,
    6,
    3
  },
  {
    "rotated_loadsi",
    (const PTR) output_184,
    (insn_gen_fn) gen_rotated_loadsi,
    &operand_data[294],
    2,
    0,
    1,
    3
  },
  {
    "*movhi_insn_arch4",
    (const PTR) output_185,
    0,
    &operand_data[296],
    2,
    0,
    4,
    2
  },
  {
    "*movhi_insn_littleend",
    (const PTR) output_186,
    0,
    &operand_data[298],
    2,
    0,
    3,
    2
  },
  {
    "*movhi_insn_bigend",
    (const PTR) output_187,
    0,
    &operand_data[298],
    2,
    0,
    3,
    2
  },
  {
    "*loadhi_si_bigend",
    "ldr%?\t%0, %1\t%@ movhi_bigend",
    0,
    &operand_data[255],
    2,
    0,
    1,
    1
  },
  {
    "*movhi_bytes",
    (const PTR) output_189,
    0,
    &operand_data[300],
    2,
    0,
    2,
    2
  },
  {
    "thumb_movhi_clobber",
    (const PTR) output_190,
    (insn_gen_fn) gen_thumb_movhi_clobber,
    &operand_data[302],
    3,
    0,
    1,
    3
  },
  {
    "*arm_movqi_insn",
    (const PTR) output_191,
    0,
    &operand_data[305],
    2,
    0,
    4,
    2
  },
  {
    "*thumb_movqi_insn",
    (const PTR) output_192,
    0,
    &operand_data[307],
    2,
    0,
    6,
    2
  },
  {
    "*arm_movsf_hard_insn",
    (const PTR) output_193,
    0,
    &operand_data[309],
    2,
    0,
    9,
    2
  },
  {
    "*arm_movsf_soft_insn",
    (const PTR) output_194,
    0,
    &operand_data[311],
    2,
    0,
    3,
    2
  },
  {
    "*thumb_movsf_insn",
    (const PTR) output_195,
    0,
    &operand_data[313],
    2,
    0,
    7,
    2
  },
  {
    "*movdf_hard_insn",
    (const PTR) output_196,
    0,
    &operand_data[315],
    2,
    0,
    11,
    3
  },
  {
    "*movdf_soft_insn",
    (const PTR) output_197,
    0,
    &operand_data[317],
    2,
    0,
    3,
    3
  },
  {
    "*thumb_movdf_insn",
    (const PTR) output_198,
    0,
    &operand_data[319],
    2,
    0,
    6,
    3
  },
  {
    "*movxf_hard_insn",
    (const PTR) output_199,
    0,
    &operand_data[321],
    2,
    0,
    7,
    3
  },
  {
    "*ldmsi_postinc4",
    "ldm%?ia\t%1!, {%3, %4, %5, %6}",
    0,
    &operand_data[323],
    7,
    4,
    1,
    1
  },
  {
    "*ldmsi_postinc3",
    "ldm%?ia\t%1!, {%3, %4, %5}",
    0,
    &operand_data[323],
    6,
    3,
    1,
    1
  },
  {
    "*ldmsi_postinc2",
    "ldm%?ia\t%1!, {%3, %4}",
    0,
    &operand_data[323],
    5,
    2,
    1,
    1
  },
  {
    "*ldmsi4",
    "ldm%?ia\t%1, {%2, %3, %4, %5}",
    0,
    &operand_data[330],
    6,
    3,
    1,
    1
  },
  {
    "*ldmsi3",
    "ldm%?ia\t%1, {%2, %3, %4}",
    0,
    &operand_data[330],
    5,
    2,
    1,
    1
  },
  {
    "*ldmsi2",
    "ldm%?ia\t%1, {%2, %3}",
    0,
    &operand_data[330],
    4,
    1,
    1,
    1
  },
  {
    "*stmsi_postinc4",
    "stm%?ia\t%1!, {%3, %4, %5, %6}",
    0,
    &operand_data[336],
    7,
    4,
    1,
    1
  },
  {
    "*stmsi_postinc3",
    "stm%?ia\t%1!, {%3, %4, %5}",
    0,
    &operand_data[336],
    6,
    3,
    1,
    1
  },
  {
    "*stmsi_postinc2",
    "stm%?ia\t%1!, {%3, %4}",
    0,
    &operand_data[336],
    5,
    2,
    1,
    1
  },
  {
    "*stmsi4",
    "stm%?ia\t%1, {%2, %3, %4, %5}",
    0,
    &operand_data[343],
    6,
    3,
    1,
    1
  },
  {
    "*stmsi3",
    "stm%?ia\t%1, {%2, %3, %4}",
    0,
    &operand_data[343],
    5,
    2,
    1,
    1
  },
  {
    "*stmsi2",
    "stm%?ia\t%1, {%2, %3}",
    0,
    &operand_data[343],
    4,
    1,
    1,
    1
  },
  {
    "movmem12b",
    (const PTR) output_212,
    (insn_gen_fn) gen_movmem12b,
    &operand_data[349],
    7,
    6,
    1,
    3
  },
  {
    "movmem8b",
    (const PTR) output_213,
    (insn_gen_fn) gen_movmem8b,
    &operand_data[349],
    6,
    4,
    1,
    3
  },
  {
    "cbranchsi4",
    (const PTR) output_214,
    (insn_gen_fn) gen_cbranchsi4,
    &operand_data[356],
    4,
    0,
    2,
    3
  },
  {
    "*negated_cbranchsi4",
    (const PTR) output_215,
    0,
    &operand_data[360],
    4,
    0,
    1,
    3
  },
  {
    "*arm_cmpsi_insn",
    (const PTR) output_216,
    0,
    &operand_data[17],
    2,
    0,
    2,
    2
  },
  {
    "*cmpsi_shiftsi",
    "cmp%?\t%0, %1%S3",
    0,
    &operand_data[176],
    4,
    0,
    1,
    1
  },
  {
    "*cmpsi_shiftsi_swp",
    "cmp%?\t%0, %1%S3",
    0,
    &operand_data[364],
    4,
    0,
    1,
    1
  },
  {
    "*cmpsi_neg_shiftsi",
    "cmn%?\t%0, %1%S3",
    0,
    &operand_data[176],
    4,
    0,
    1,
    1
  },
  {
    "*cmpsf_insn",
    (const PTR) output_220,
    0,
    &operand_data[368],
    2,
    0,
    2,
    2
  },
  {
    "*cmpdf_insn",
    (const PTR) output_221,
    0,
    &operand_data[370],
    2,
    0,
    2,
    2
  },
  {
    "*cmpesfdf_df",
    (const PTR) output_222,
    0,
    &operand_data[38],
    2,
    0,
    2,
    2
  },
  {
    "*cmpdf_esfdf",
    "cmf%?\t%0, %1",
    0,
    &operand_data[41],
    2,
    0,
    1,
    1
  },
  {
    "*cmpxf_insn",
    (const PTR) output_224,
    0,
    &operand_data[47],
    2,
    0,
    2,
    2
  },
  {
    "*cmpsf_trap",
    (const PTR) output_225,
    0,
    &operand_data[368],
    2,
    0,
    2,
    2
  },
  {
    "*cmpdf_trap",
    (const PTR) output_226,
    0,
    &operand_data[370],
    2,
    0,
    2,
    2
  },
  {
    "*cmp_esfdf_df_trap",
    (const PTR) output_227,
    0,
    &operand_data[38],
    2,
    0,
    2,
    2
  },
  {
    "*cmp_df_esfdf_trap",
    "cmf%?e\t%0, %1",
    0,
    &operand_data[41],
    2,
    0,
    1,
    1
  },
  {
    "*cmpxf_trap",
    (const PTR) output_229,
    0,
    &operand_data[47],
    2,
    0,
    2,
    2
  },
  {
    "*deleted_compare",
    "\t%@ deleted compare",
    0,
    &operand_data[73],
    1,
    1,
    0,
    1
  },
  {
    "*arm_buneq",
    (const PTR) output_231,
    0,
    &operand_data[372],
    2,
    0,
    0,
    3
  },
  {
    "*arm_bltgt",
    (const PTR) output_232,
    0,
    &operand_data[372],
    2,
    0,
    0,
    3
  },
  {
    "*arm_cond_branch",
    (const PTR) output_233,
    0,
    &operand_data[374],
    3,
    0,
    0,
    3
  },
  {
    "*arm_buneq_reversed",
    (const PTR) output_234,
    0,
    &operand_data[372],
    2,
    0,
    0,
    3
  },
  {
    "*arm_bltgt_reversed",
    (const PTR) output_235,
    0,
    &operand_data[372],
    2,
    0,
    0,
    3
  },
  {
    "*arm_cond_branch_reversed",
    (const PTR) output_236,
    0,
    &operand_data[374],
    3,
    0,
    0,
    3
  },
  {
    "*mov_scc",
    "mov%D1\t%0, #0\n\tmov%d1\t%0, #1",
    0,
    &operand_data[377],
    3,
    0,
    1,
    1
  },
  {
    "*mov_negscc",
    "mov%D1\t%0, #0\n\tmvn%d1\t%0, #0",
    0,
    &operand_data[377],
    3,
    0,
    1,
    1
  },
  {
    "*mov_notscc",
    "mov%D1\t%0, #0\n\tmvn%d1\t%0, #1",
    0,
    &operand_data[377],
    3,
    0,
    1,
    1
  },
  {
    "*movsicc_insn",
    (const PTR) output_240,
    0,
    &operand_data[380],
    5,
    0,
    8,
    2
  },
  {
    "*movsfcc_hard_insn",
    (const PTR) output_241,
    0,
    &operand_data[385],
    5,
    0,
    8,
    2
  },
  {
    "*movsfcc_soft_insn",
    (const PTR) output_242,
    0,
    &operand_data[390],
    5,
    0,
    2,
    2
  },
  {
    "*movdfcc_insn",
    (const PTR) output_243,
    0,
    &operand_data[395],
    5,
    0,
    8,
    2
  },
  {
    "*arm_jump",
    (const PTR) output_244,
    0,
    &operand_data[286],
    1,
    0,
    0,
    3
  },
  {
    "*thumb_jump",
    (const PTR) output_245,
    0,
    &operand_data[286],
    1,
    0,
    0,
    3
  },
  {
    "*call_reg",
    (const PTR) output_246,
    0,
    &operand_data[400],
    3,
    0,
    1,
    3
  },
  {
    "*call_mem",
    (const PTR) output_247,
    0,
    &operand_data[403],
    3,
    0,
    1,
    3
  },
  {
    "*call_indirect",
    (const PTR) output_248,
    0,
    &operand_data[406],
    3,
    0,
    1,
    3
  },
  {
    "*call_value_indirect",
    (const PTR) output_249,
    0,
    &operand_data[409],
    4,
    0,
    1,
    3
  },
  {
    "*call_value_reg",
    (const PTR) output_250,
    0,
    &operand_data[413],
    4,
    0,
    2,
    3
  },
  {
    "*call_value_mem",
    (const PTR) output_251,
    0,
    &operand_data[417],
    4,
    0,
    2,
    3
  },
  {
    "*call_symbol",
    (const PTR) output_252,
    0,
    &operand_data[421],
    3,
    0,
    1,
    3
  },
  {
    "*call_value_symbol",
    (const PTR) output_253,
    0,
    &operand_data[424],
    4,
    0,
    2,
    3
  },
  {
    "*call_insn",
    "bl\t%a0",
    0,
    &operand_data[428],
    3,
    0,
    1,
    1
  },
  {
    "*call_value_insn",
    "bl\t%a1",
    0,
    &operand_data[431],
    4,
    0,
    1,
    1
  },
  {
    "*sibcall_insn",
    (const PTR) output_256,
    0,
    &operand_data[421],
    3,
    0,
    1,
    3
  },
  {
    "*sibcall_value_insn",
    (const PTR) output_257,
    0,
    &operand_data[435],
    4,
    0,
    2,
    3
  },
  {
    "return",
    (const PTR) output_258,
    (insn_gen_fn) gen_return,
    &operand_data[0],
    0,
    0,
    0,
    3
  },
  {
    "*cond_return",
    (const PTR) output_259,
    0,
    &operand_data[375],
    2,
    0,
    0,
    3
  },
  {
    "*cond_return_inverted",
    (const PTR) output_260,
    0,
    &operand_data[375],
    2,
    0,
    0,
    3
  },
  {
    "*check_arch2",
    "teq\t%|r0, %|r0\n\tteq\t%|pc, %|pc",
    0,
    &operand_data[439],
    1,
    0,
    0,
    1
  },
  {
    "blockage",
    "",
    (insn_gen_fn) gen_blockage,
    &operand_data[0],
    0,
    0,
    0,
    1
  },
  {
    "casesi_internal",
    (const PTR) output_263,
    (insn_gen_fn) gen_casesi_internal,
    &operand_data[440],
    4,
    2,
    1,
    3
  },
  {
    "*arm_indirect_jump",
    "mov%?\t%|pc, %0\t%@ indirect register jump",
    0,
    &operand_data[20],
    1,
    0,
    1,
    1
  },
  {
    "*load_indirect_jump",
    "ldr%?\t%|pc, %0\t%@ indirect memory jump",
    0,
    &operand_data[403],
    1,
    0,
    1,
    1
  },
  {
    "*thumb_indirect_jump",
    "mov\tpc, %0",
    0,
    &operand_data[406],
    1,
    0,
    1,
    1
  },
  {
    "nop",
    (const PTR) output_267,
    (insn_gen_fn) gen_nop,
    &operand_data[0],
    0,
    0,
    0,
    3
  },
  {
    "*arith_shiftsi",
    "%i1%?\t%0, %2, %4%S3",
    0,
    &operand_data[444],
    6,
    0,
    1,
    1
  },
  {
    "*arith_shiftsi_compare0",
    "%i1%?s\t%0, %2, %4%S3",
    0,
    &operand_data[444],
    6,
    5,
    1,
    1
  },
  {
    "*arith_shiftsi_compare0_scratch",
    "%i1%?s\t%0, %2, %4%S3",
    0,
    &operand_data[450],
    6,
    0,
    1,
    1
  },
  {
    "*sub_shiftsi",
    "sub%?\t%0, %1, %3%S2",
    0,
    &operand_data[456],
    5,
    0,
    1,
    1
  },
  {
    "*sub_shiftsi_compare0",
    "sub%?s\t%0, %1, %3%S2",
    0,
    &operand_data[456],
    5,
    4,
    1,
    1
  },
  {
    "*sub_shiftsi_compare0_scratch",
    "sub%?s\t%0, %1, %3%S2",
    0,
    &operand_data[461],
    5,
    0,
    1,
    1
  },
  {
    "*and_scc",
    "mov%D1\t%0, #0\n\tand%d1\t%0, %2, #1",
    0,
    &operand_data[466],
    4,
    0,
    1,
    1
  },
  {
    "*ior_scc",
    (const PTR) output_275,
    0,
    &operand_data[70],
    4,
    0,
    2,
    2
  },
  {
    "*compare_scc",
    (const PTR) output_276,
    0,
    &operand_data[470],
    4,
    0,
    2,
    3
  },
  {
    "*cond_move",
    (const PTR) output_277,
    0,
    &operand_data[474],
    6,
    0,
    3,
    3
  },
  {
    "*cond_arith",
    (const PTR) output_278,
    0,
    &operand_data[480],
    6,
    0,
    2,
    3
  },
  {
    "*cond_sub",
    (const PTR) output_279,
    0,
    &operand_data[480],
    5,
    0,
    2,
    3
  },
  {
    "*cmp_ite0",
    (const PTR) output_280,
    0,
    &operand_data[486],
    7,
    0,
    4,
    3
  },
  {
    "*cmp_ite1",
    (const PTR) output_281,
    0,
    &operand_data[486],
    7,
    0,
    4,
    3
  },
  {
    "*cmp_and",
    (const PTR) output_282,
    0,
    &operand_data[486],
    7,
    0,
    4,
    3
  },
  {
    "*cmp_ior",
    (const PTR) output_283,
    0,
    &operand_data[486],
    7,
    0,
    4,
    3
  },
  {
    "*negscc",
    (const PTR) output_284,
    0,
    &operand_data[493],
    4,
    0,
    1,
    3
  },
  {
    "movcond",
    (const PTR) output_285,
    (insn_gen_fn) gen_movcond,
    &operand_data[497],
    6,
    0,
    3,
    3
  },
  {
    "*ifcompare_plus_move",
    "#",
    0,
    &operand_data[503],
    7,
    0,
    2,
    1
  },
  {
    "*if_plus_move",
    (const PTR) output_287,
    0,
    &operand_data[510],
    6,
    0,
    4,
    2
  },
  {
    "*ifcompare_move_plus",
    "#",
    0,
    &operand_data[503],
    7,
    0,
    2,
    1
  },
  {
    "*if_move_plus",
    (const PTR) output_289,
    0,
    &operand_data[510],
    6,
    0,
    4,
    2
  },
  {
    "*ifcompare_arith_arith",
    "#",
    0,
    &operand_data[516],
    10,
    0,
    1,
    1
  },
  {
    "*if_arith_arith",
    "%I6%d5\t%0, %1, %2\n\t%I7%D5\t%0, %3, %4",
    0,
    &operand_data[526],
    9,
    0,
    1,
    1
  },
  {
    "*ifcompare_arith_move",
    (const PTR) output_292,
    0,
    &operand_data[535],
    8,
    0,
    2,
    3
  },
  {
    "*if_arith_move",
    (const PTR) output_293,
    0,
    &operand_data[543],
    7,
    0,
    2,
    2
  },
  {
    "*ifcompare_move_arith",
    (const PTR) output_294,
    0,
    &operand_data[550],
    8,
    0,
    2,
    3
  },
  {
    "*if_move_arith",
    (const PTR) output_295,
    0,
    &operand_data[543],
    7,
    0,
    2,
    2
  },
  {
    "*ifcompare_move_not",
    "#",
    0,
    &operand_data[558],
    6,
    0,
    2,
    1
  },
  {
    "*if_move_not",
    (const PTR) output_297,
    0,
    &operand_data[564],
    5,
    0,
    3,
    2
  },
  {
    "*ifcompare_not_move",
    "#",
    0,
    &operand_data[558],
    6,
    0,
    2,
    1
  },
  {
    "*if_not_move",
    (const PTR) output_299,
    0,
    &operand_data[564],
    5,
    0,
    3,
    2
  },
  {
    "*ifcompare_shift_move",
    "#",
    0,
    &operand_data[569],
    8,
    0,
    2,
    1
  },
  {
    "*if_shift_move",
    (const PTR) output_301,
    0,
    &operand_data[577],
    7,
    0,
    3,
    2
  },
  {
    "*ifcompare_move_shift",
    "#",
    0,
    &operand_data[569],
    8,
    0,
    2,
    1
  },
  {
    "*if_move_shift",
    (const PTR) output_303,
    0,
    &operand_data[577],
    7,
    0,
    3,
    2
  },
  {
    "*ifcompare_shift_shift",
    "#",
    0,
    &operand_data[584],
    10,
    0,
    1,
    1
  },
  {
    "*if_shift_shift",
    "mov%d5\t%0, %1%S6\n\tmov%D5\t%0, %3%S7",
    0,
    &operand_data[594],
    9,
    0,
    1,
    1
  },
  {
    "*ifcompare_not_arith",
    "#",
    0,
    &operand_data[603],
    8,
    0,
    1,
    1
  },
  {
    "*if_not_arith",
    "mvn%d5\t%0, %1\n\t%I6%D5\t%0, %2, %3",
    0,
    &operand_data[611],
    7,
    0,
    1,
    1
  },
  {
    "*ifcompare_arith_not",
    "#",
    0,
    &operand_data[603],
    8,
    0,
    1,
    1
  },
  {
    "*if_arith_not",
    "mvn%D5\t%0, %1\n\t%I6%d5\t%0, %2, %3",
    0,
    &operand_data[611],
    7,
    0,
    1,
    1
  },
  {
    "*ifcompare_neg_move",
    "#",
    0,
    &operand_data[558],
    6,
    0,
    2,
    1
  },
  {
    "*if_neg_move",
    (const PTR) output_311,
    0,
    &operand_data[564],
    5,
    0,
    3,
    2
  },
  {
    "*ifcompare_move_neg",
    "#",
    0,
    &operand_data[558],
    6,
    0,
    2,
    1
  },
  {
    "*if_move_neg",
    (const PTR) output_313,
    0,
    &operand_data[564],
    5,
    0,
    3,
    2
  },
  {
    "*arith_adjacentmem",
    (const PTR) output_314,
    0,
    &operand_data[618],
    5,
    0,
    1,
    3
  },
  {
    "*strqi_preinc",
    "str%?b\t%3, [%0, %2]!",
    0,
    &operand_data[623],
    4,
    2,
    1,
    1
  },
  {
    "*strqi_predec",
    "str%?b\t%3, [%0, -%2]!",
    0,
    &operand_data[627],
    4,
    2,
    1,
    1
  },
  {
    "*loadqi_preinc",
    "ldr%?b\t%3, [%0, %2]!",
    0,
    &operand_data[631],
    4,
    2,
    1,
    1
  },
  {
    "*loadqi_predec",
    "ldr%?b\t%3, [%0, -%2]!",
    0,
    &operand_data[635],
    4,
    2,
    1,
    1
  },
  {
    "*loadqisi_preinc",
    "ldr%?b\t%3, [%0, %2]!\t%@ z_extendqisi",
    0,
    &operand_data[639],
    4,
    2,
    1,
    1
  },
  {
    "*loadqisi_predec",
    "ldr%?b\t%3, [%0, -%2]!\t%@ z_extendqisi",
    0,
    &operand_data[642],
    4,
    2,
    1,
    1
  },
  {
    "*strsi_preinc",
    "str%?\t%3, [%0, %2]!",
    0,
    &operand_data[645],
    4,
    2,
    1,
    1
  },
  {
    "*strsi_predec",
    "str%?\t%3, [%0, -%2]!",
    0,
    &operand_data[649],
    4,
    2,
    1,
    1
  },
  {
    "*loadsi_preinc",
    "ldr%?\t%3, [%0, %2]!",
    0,
    &operand_data[639],
    4,
    2,
    1,
    1
  },
  {
    "*loadsi_predec",
    "ldr%?\t%3, [%0, -%2]!",
    0,
    &operand_data[642],
    4,
    2,
    1,
    1
  },
  {
    "*loadhi_preinc",
    "ldr%?\t%3, [%0, %2]!\t%@ loadhi",
    0,
    &operand_data[653],
    4,
    2,
    1,
    1
  },
  {
    "*loadhi_predec",
    "ldr%?\t%3, [%0, -%2]!\t%@ loadhi",
    0,
    &operand_data[657],
    4,
    2,
    1,
    1
  },
  {
    "*strqi_shiftpreinc",
    "str%?b\t%5, [%0, %3%S2]!",
    0,
    &operand_data[661],
    6,
    4,
    1,
    1
  },
  {
    "*strqi_shiftpredec",
    "str%?b\t%5, [%0, -%3%S2]!",
    0,
    &operand_data[661],
    6,
    4,
    1,
    1
  },
  {
    "*loadqi_shiftpreinc",
    "ldr%?b\t%5, [%0, %3%S2]!",
    0,
    &operand_data[667],
    6,
    4,
    1,
    1
  },
  {
    "*loadqi_shiftpredec",
    "ldr%?b\t%5, [%0, -%3%S2]!",
    0,
    &operand_data[667],
    6,
    4,
    1,
    1
  },
  {
    "*strsi_shiftpreinc",
    "str%?\t%5, [%0, %3%S2]!",
    0,
    &operand_data[673],
    6,
    4,
    1,
    1
  },
  {
    "*strsi_shiftpredec",
    "str%?\t%5, [%0, -%3%S2]!",
    0,
    &operand_data[673],
    6,
    4,
    1,
    1
  },
  {
    "*loadsi_shiftpreinc",
    "ldr%?\t%5, [%0, %3%S2]!",
    0,
    &operand_data[679],
    6,
    4,
    1,
    1
  },
  {
    "*loadsi_shiftpredec",
    "ldr%?\t%5, [%0, -%3%S2]!",
    0,
    &operand_data[679],
    6,
    4,
    1,
    1
  },
  {
    "*loadhi_shiftpreinc",
    "ldr%?\t%5, [%0, %3%S2]!\t%@ loadhi",
    0,
    &operand_data[684],
    6,
    4,
    1,
    1
  },
  {
    "*loadhi_shiftpredec",
    "ldr%?\t%5, [%0, -%3%S2]!\t%@ loadhi",
    0,
    &operand_data[684],
    6,
    4,
    1,
    1
  },
  {
    "sibcall_epilogue",
    (const PTR) output_337,
    (insn_gen_fn) gen_sibcall_epilogue,
    &operand_data[0],
    0,
    0,
    0,
    3
  },
  {
    "*epilogue_insns",
    (const PTR) output_338,
    0,
    &operand_data[0],
    0,
    0,
    0,
    3
  },
  {
    "*cond_move_not",
    (const PTR) output_339,
    0,
    &operand_data[690],
    5,
    0,
    2,
    2
  },
  {
    "*sign_extract_onebit",
    (const PTR) output_340,
    0,
    &operand_data[156],
    3,
    0,
    1,
    3
  },
  {
    "*not_signextract_onebit",
    (const PTR) output_341,
    0,
    &operand_data[156],
    3,
    0,
    1,
    3
  },
  {
    "*push_multi",
    (const PTR) output_342,
    0,
    &operand_data[695],
    3,
    0,
    1,
    3
  },
  {
    "stack_tie",
    "",
    (insn_gen_fn) gen_stack_tie,
    &operand_data[59],
    2,
    0,
    1,
    1
  },
  {
    "*push_fp_multi",
    (const PTR) output_344,
    0,
    &operand_data[698],
    3,
    0,
    1,
    3
  },
  {
    "align_4",
    (const PTR) output_345,
    (insn_gen_fn) gen_align_4,
    &operand_data[0],
    0,
    0,
    0,
    3
  },
  {
    "consttable_end",
    (const PTR) output_346,
    (insn_gen_fn) gen_consttable_end,
    &operand_data[0],
    0,
    0,
    0,
    3
  },
  {
    "consttable_1",
    (const PTR) output_347,
    (insn_gen_fn) gen_consttable_1,
    &operand_data[286],
    1,
    0,
    0,
    3
  },
  {
    "consttable_2",
    (const PTR) output_348,
    (insn_gen_fn) gen_consttable_2,
    &operand_data[286],
    1,
    0,
    0,
    3
  },
  {
    "consttable_4",
    (const PTR) output_349,
    (insn_gen_fn) gen_consttable_4,
    &operand_data[286],
    1,
    0,
    0,
    3
  },
  {
    "consttable_8",
    (const PTR) output_350,
    (insn_gen_fn) gen_consttable_8,
    &operand_data[286],
    1,
    0,
    0,
    3
  },
  {
    "*thumb_tablejump",
    "mov\t%|pc, %0",
    0,
    &operand_data[406],
    2,
    0,
    1,
    1
  },
  {
    "clz",
    "clz\t%0, %1",
    (insn_gen_fn) gen_clz,
    &operand_data[19],
    2,
    0,
    1,
    1
  },
  {
    "prefetch",
    "pld\t%a0",
    (insn_gen_fn) gen_prefetch,
    &operand_data[701],
    3,
    0,
    1,
    1
  },
  {
    "prologue_use",
    "%@ %0 needed for prologue",
    (insn_gen_fn) gen_prologue_use,
    &operand_data[704],
    1,
    0,
    0,
    1
  },
  {
    "adddi3",
    0,
    (insn_gen_fn) gen_adddi3,
    &operand_data[705],
    3,
    0,
    0,
    0
  },
  {
    "adddi3+1",
    0,
    0,
    &operand_data[705],
    3,
    0,
    0,
    0
  },
  {
    "adddi3+2",
    0,
    0,
    &operand_data[706],
    3,
    0,
    0,
    0
  },
  {
    "addsi3-1",
    0,
    0,
    &operand_data[706],
    3,
    0,
    0,
    0
  },
  {
    "addsi3",
    0,
    (insn_gen_fn) gen_addsi3,
    &operand_data[708],
    3,
    0,
    0,
    0
  },
  {
    "addsi3+1",
    0,
    0,
    &operand_data[711],
    4,
    0,
    0,
    0
  },
  {
    "addsi3+2",
    0,
    0,
    &operand_data[708],
    3,
    0,
    0,
    0
  },
  {
    "subdi3-1",
    0,
    0,
    &operand_data[715],
    3,
    0,
    0,
    0
  },
  {
    "subdi3",
    0,
    (insn_gen_fn) gen_subdi3,
    &operand_data[705],
    3,
    0,
    0,
    0
  },
  {
    "subsi3",
    0,
    (insn_gen_fn) gen_subsi3,
    &operand_data[709],
    3,
    0,
    0,
    0
  },
  {
    "subsi3+1",
    0,
    0,
    &operand_data[709],
    3,
    0,
    0,
    0
  },
  {
    "mulsi3-1",
    0,
    0,
    &operand_data[718],
    4,
    0,
    0,
    0
  },
  {
    "mulsi3",
    0,
    (insn_gen_fn) gen_mulsi3,
    &operand_data[722],
    3,
    0,
    0,
    0
  },
  {
    "mulsi3+1",
    0,
    0,
    &operand_data[725],
    7,
    0,
    0,
    0
  },
  {
    "mulsi3+2",
    0,
    0,
    &operand_data[732],
    7,
    0,
    0,
    0
  },
  {
    "mulsi3+3",
    0,
    0,
    &operand_data[706],
    3,
    0,
    0,
    0
  },
  {
    "andsi3-2",
    0,
    0,
    &operand_data[706],
    3,
    0,
    0,
    0
  },
  {
    "andsi3-1",
    0,
    0,
    &operand_data[706],
    3,
    0,
    0,
    0
  },
  {
    "andsi3",
    0,
    (insn_gen_fn) gen_andsi3,
    &operand_data[708],
    3,
    0,
    0,
    0
  },
  {
    "andsi3+1",
    0,
    0,
    &operand_data[708],
    3,
    0,
    0,
    0
  },
  {
    "insv",
    0,
    (insn_gen_fn) gen_insv,
    &operand_data[739],
    4,
    0,
    0,
    0
  },
  {
    "insv+1",
    0,
    0,
    &operand_data[705],
    3,
    0,
    0,
    0
  },
  {
    "insv+2",
    0,
    0,
    &operand_data[706],
    3,
    0,
    0,
    0
  },
  {
    "iorsi3-1",
    0,
    0,
    &operand_data[706],
    3,
    0,
    0,
    0
  },
  {
    "iorsi3",
    0,
    (insn_gen_fn) gen_iorsi3,
    &operand_data[708],
    3,
    0,
    0,
    0
  },
  {
    "iorsi3+1",
    0,
    0,
    &operand_data[708],
    3,
    0,
    0,
    0
  },
  {
    "xorsi3-1",
    0,
    0,
    &operand_data[711],
    4,
    0,
    0,
    0
  },
  {
    "xorsi3",
    0,
    (insn_gen_fn) gen_xorsi3,
    &operand_data[743],
    3,
    0,
    0,
    0
  },
  {
    "xorsi3+1",
    0,
    0,
    &operand_data[743],
    5,
    0,
    0,
    0
  },
  {
    "ashlsi3",
    0,
    (insn_gen_fn) gen_ashlsi3,
    &operand_data[743],
    3,
    0,
    0,
    0
  },
  {
    "ashrsi3",
    0,
    (insn_gen_fn) gen_ashrsi3,
    &operand_data[743],
    3,
    0,
    0,
    0
  },
  {
    "lshrsi3",
    0,
    (insn_gen_fn) gen_lshrsi3,
    &operand_data[743],
    3,
    0,
    0,
    0
  },
  {
    "rotlsi3",
    0,
    (insn_gen_fn) gen_rotlsi3,
    &operand_data[708],
    3,
    0,
    0,
    0
  },
  {
    "rotrsi3",
    0,
    (insn_gen_fn) gen_rotrsi3,
    &operand_data[743],
    3,
    0,
    0,
    0
  },
  {
    "extzv",
    0,
    (insn_gen_fn) gen_extzv,
    &operand_data[748],
    4,
    2,
    0,
    0
  },
  {
    "negdi2",
    0,
    (insn_gen_fn) gen_negdi2,
    &operand_data[705],
    2,
    0,
    0,
    0
  },
  {
    "negsi2",
    0,
    (insn_gen_fn) gen_negsi2,
    &operand_data[22],
    2,
    0,
    0,
    0
  },
  {
    "negsi2+1",
    0,
    0,
    &operand_data[705],
    2,
    0,
    0,
    0
  },
  {
    "one_cmplsi2",
    0,
    (insn_gen_fn) gen_one_cmplsi2,
    &operand_data[22],
    2,
    0,
    0,
    0
  },
  {
    "zero_extendhisi2",
    0,
    (insn_gen_fn) gen_zero_extendhisi2,
    &operand_data[752],
    2,
    2,
    0,
    0
  },
  {
    "zero_extendhisi2+1",
    0,
    0,
    &operand_data[754],
    3,
    0,
    0,
    0
  },
  {
    "zero_extendqisi2-1",
    0,
    0,
    &operand_data[754],
    5,
    0,
    0,
    0
  },
  {
    "zero_extendqisi2",
    0,
    (insn_gen_fn) gen_zero_extendqisi2,
    &operand_data[758],
    2,
    0,
    0,
    0
  },
  {
    "zero_extendqisi2+1",
    0,
    0,
    &operand_data[760],
    3,
    0,
    0,
    0
  },
  {
    "extendhisi2",
    0,
    (insn_gen_fn) gen_extendhisi2,
    &operand_data[752],
    2,
    2,
    0,
    0
  },
  {
    "extendhisi2_mem",
    0,
    (insn_gen_fn) gen_extendhisi2_mem,
    &operand_data[763],
    2,
    7,
    0,
    0
  },
  {
    "extendhisi2_mem+1",
    0,
    0,
    &operand_data[754],
    3,
    0,
    0,
    0
  },
  {
    "extendqihi2-1",
    0,
    0,
    &operand_data[754],
    5,
    0,
    0,
    0
  },
  {
    "extendqihi2",
    0,
    (insn_gen_fn) gen_extendqihi2,
    &operand_data[765],
    2,
    2,
    0,
    0
  },
  {
    "extendqihi2+1",
    0,
    0,
    &operand_data[767],
    2,
    0,
    0,
    0
  },
  {
    "extendqisi2",
    0,
    (insn_gen_fn) gen_extendqisi2,
    &operand_data[769],
    2,
    2,
    0,
    0
  },
  {
    "extendqisi2+1",
    0,
    0,
    &operand_data[771],
    2,
    0,
    0,
    0
  },
  {
    "movdi",
    0,
    (insn_gen_fn) gen_movdi,
    &operand_data[773],
    2,
    0,
    0,
    0
  },
  {
    "movsi",
    0,
    (insn_gen_fn) gen_movsi,
    &operand_data[740],
    2,
    0,
    0,
    0
  },
  {
    "movsi+1",
    0,
    0,
    &operand_data[712],
    2,
    0,
    0,
    0
  },
  {
    "movsi+2",
    0,
    0,
    &operand_data[715],
    2,
    0,
    0,
    0
  },
  {
    "pic_load_addr_based-1",
    0,
    0,
    &operand_data[715],
    2,
    0,
    0,
    0
  },
  {
    "pic_load_addr_based",
    0,
    (insn_gen_fn) gen_pic_load_addr_based,
    &operand_data[285],
    2,
    1,
    1,
    0
  },
  {
    "builtin_setjmp_receiver",
    0,
    (insn_gen_fn) gen_builtin_setjmp_receiver,
    &operand_data[286],
    1,
    0,
    0,
    0
  },
  {
    "storehi",
    0,
    (insn_gen_fn) gen_storehi,
    &operand_data[401],
    2,
    4,
    0,
    0
  },
  {
    "storehi_bigend",
    0,
    (insn_gen_fn) gen_storehi_bigend,
    &operand_data[401],
    2,
    4,
    0,
    0
  },
  {
    "storeinthi",
    0,
    (insn_gen_fn) gen_storeinthi,
    &operand_data[401],
    2,
    2,
    0,
    0
  },
  {
    "storehi_single_op",
    0,
    (insn_gen_fn) gen_storehi_single_op,
    &operand_data[775],
    2,
    0,
    0,
    0
  },
  {
    "movhi",
    0,
    (insn_gen_fn) gen_movhi,
    &operand_data[776],
    2,
    0,
    0,
    0
  },
  {
    "movhi_bytes",
    0,
    (insn_gen_fn) gen_movhi_bytes,
    &operand_data[763],
    2,
    5,
    0,
    0
  },
  {
    "movhi_bigend",
    0,
    (insn_gen_fn) gen_movhi_bigend,
    &operand_data[778],
    2,
    4,
    0,
    0
  },
  {
    "reload_outhi",
    0,
    (insn_gen_fn) gen_reload_outhi,
    &operand_data[780],
    3,
    0,
    1,
    0
  },
  {
    "reload_inhi",
    0,
    (insn_gen_fn) gen_reload_inhi,
    &operand_data[783],
    3,
    0,
    1,
    0
  },
  {
    "movqi",
    0,
    (insn_gen_fn) gen_movqi,
    &operand_data[786],
    2,
    0,
    0,
    0
  },
  {
    "movsf",
    0,
    (insn_gen_fn) gen_movsf,
    &operand_data[788],
    2,
    0,
    0,
    0
  },
  {
    "movsf+1",
    0,
    0,
    &operand_data[790],
    2,
    0,
    0,
    0
  },
  {
    "movdf",
    0,
    (insn_gen_fn) gen_movdf,
    &operand_data[792],
    2,
    0,
    0,
    0
  },
  {
    "reload_outdf",
    0,
    (insn_gen_fn) gen_reload_outdf,
    &operand_data[794],
    3,
    0,
    1,
    0
  },
  {
    "movxf",
    0,
    (insn_gen_fn) gen_movxf,
    &operand_data[797],
    2,
    0,
    0,
    0
  },
  {
    "load_multiple",
    0,
    (insn_gen_fn) gen_load_multiple,
    &operand_data[799],
    3,
    1,
    0,
    0
  },
  {
    "store_multiple",
    0,
    (insn_gen_fn) gen_store_multiple,
    &operand_data[799],
    3,
    1,
    0,
    0
  },
  {
    "movstrqi",
    0,
    (insn_gen_fn) gen_movstrqi,
    &operand_data[802],
    4,
    0,
    0,
    0
  },
  {
    "cmpsi",
    0,
    (insn_gen_fn) gen_cmpsi,
    &operand_data[806],
    2,
    0,
    0,
    0
  },
  {
    "cmpsf",
    0,
    (insn_gen_fn) gen_cmpsf,
    &operand_data[808],
    2,
    0,
    0,
    0
  },
  {
    "cmpdf",
    0,
    (insn_gen_fn) gen_cmpdf,
    &operand_data[810],
    2,
    0,
    0,
    0
  },
  {
    "cmpxf",
    0,
    (insn_gen_fn) gen_cmpxf,
    &operand_data[812],
    2,
    0,
    0,
    0
  },
  {
    "beq",
    0,
    (insn_gen_fn) gen_beq,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bne",
    0,
    (insn_gen_fn) gen_bne,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bgt",
    0,
    (insn_gen_fn) gen_bgt,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "ble",
    0,
    (insn_gen_fn) gen_ble,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bge",
    0,
    (insn_gen_fn) gen_bge,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "blt",
    0,
    (insn_gen_fn) gen_blt,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bgtu",
    0,
    (insn_gen_fn) gen_bgtu,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bleu",
    0,
    (insn_gen_fn) gen_bleu,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bgeu",
    0,
    (insn_gen_fn) gen_bgeu,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bltu",
    0,
    (insn_gen_fn) gen_bltu,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bunordered",
    0,
    (insn_gen_fn) gen_bunordered,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bordered",
    0,
    (insn_gen_fn) gen_bordered,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bungt",
    0,
    (insn_gen_fn) gen_bungt,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bunlt",
    0,
    (insn_gen_fn) gen_bunlt,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bunge",
    0,
    (insn_gen_fn) gen_bunge,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bunle",
    0,
    (insn_gen_fn) gen_bunle,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "buneq",
    0,
    (insn_gen_fn) gen_buneq,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "bltgt",
    0,
    (insn_gen_fn) gen_bltgt,
    &operand_data[286],
    1,
    1,
    0,
    0
  },
  {
    "seq",
    0,
    (insn_gen_fn) gen_seq,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sne",
    0,
    (insn_gen_fn) gen_sne,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sgt",
    0,
    (insn_gen_fn) gen_sgt,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sle",
    0,
    (insn_gen_fn) gen_sle,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sge",
    0,
    (insn_gen_fn) gen_sge,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "slt",
    0,
    (insn_gen_fn) gen_slt,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sgtu",
    0,
    (insn_gen_fn) gen_sgtu,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sleu",
    0,
    (insn_gen_fn) gen_sleu,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sgeu",
    0,
    (insn_gen_fn) gen_sgeu,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sltu",
    0,
    (insn_gen_fn) gen_sltu,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sunordered",
    0,
    (insn_gen_fn) gen_sunordered,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sordered",
    0,
    (insn_gen_fn) gen_sordered,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sungt",
    0,
    (insn_gen_fn) gen_sungt,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sunge",
    0,
    (insn_gen_fn) gen_sunge,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sunlt",
    0,
    (insn_gen_fn) gen_sunlt,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "sunle",
    0,
    (insn_gen_fn) gen_sunle,
    &operand_data[19],
    1,
    1,
    1,
    0
  },
  {
    "movsicc",
    0,
    (insn_gen_fn) gen_movsicc,
    &operand_data[814],
    4,
    0,
    0,
    0
  },
  {
    "movsfcc",
    0,
    (insn_gen_fn) gen_movsfcc,
    &operand_data[818],
    4,
    0,
    0,
    0
  },
  {
    "movdfcc",
    0,
    (insn_gen_fn) gen_movdfcc,
    &operand_data[822],
    4,
    0,
    0,
    0
  },
  {
    "jump",
    0,
    (insn_gen_fn) gen_jump,
    &operand_data[286],
    1,
    0,
    0,
    0
  },
  {
    "call",
    0,
    (insn_gen_fn) gen_call,
    &operand_data[826],
    3,
    0,
    0,
    0
  },
  {
    "call_value",
    0,
    (insn_gen_fn) gen_call_value,
    &operand_data[828],
    4,
    0,
    0,
    0
  },
  {
    "sibcall",
    0,
    (insn_gen_fn) gen_sibcall,
    &operand_data[826],
    3,
    0,
    0,
    0
  },
  {
    "sibcall_value",
    0,
    (insn_gen_fn) gen_sibcall_value,
    &operand_data[832],
    4,
    0,
    0,
    0
  },
  {
    "return_addr_mask",
    0,
    (insn_gen_fn) gen_return_addr_mask,
    &operand_data[22],
    1,
    2,
    0,
    0
  },
  {
    "untyped_call",
    0,
    (insn_gen_fn) gen_untyped_call,
    &operand_data[835],
    3,
    0,
    0,
    0
  },
  {
    "casesi",
    0,
    (insn_gen_fn) gen_casesi,
    &operand_data[838],
    5,
    0,
    0,
    0
  },
  {
    "indirect_jump",
    0,
    (insn_gen_fn) gen_indirect_jump,
    &operand_data[22],
    1,
    0,
    0,
    0
  },
  {
    "indirect_jump+1",
    "str%?b\t%2, [%0], %1",
    0,
    &operand_data[843],
    3,
    0,
    1,
    1
  },
  {
    "indirect_jump+2",
    "ldr%?b\t%0, [%1], %2",
    0,
    &operand_data[846],
    3,
    0,
    1,
    1
  },
  {
    "indirect_jump+3",
    "str%?\t%2, [%0], %1",
    0,
    &operand_data[847],
    3,
    0,
    1,
    1
  },
  {
    "indirect_jump+4",
    "ldr%?\t%0, [%1], %2\t%@ loadhi",
    0,
    &operand_data[850],
    3,
    0,
    1,
    1
  },
  {
    "indirect_jump+5",
    "ldr%?\t%0, [%1], %2",
    0,
    &operand_data[853],
    3,
    0,
    1,
    1
  },
  {
    "indirect_jump+6",
    "str%?b\t%2, [%0, %1]!",
    0,
    &operand_data[843],
    3,
    0,
    1,
    1
  },
  {
    "indirect_jump+7",
    "str%?b\t%3, [%2, %0%S4]!",
    0,
    &operand_data[856],
    5,
    0,
    1,
    1
  },
  {
    "indirect_jump+8",
    0,
    0,
    &operand_data[22],
    2,
    0,
    0,
    0
  },
  {
    "prologue-8",
    (const PTR) output_490,
    0,
    &operand_data[861],
    8,
    0,
    1,
    3
  },
  {
    "prologue-7",
    (const PTR) output_491,
    0,
    &operand_data[862],
    6,
    0,
    1,
    3
  },
  {
    "prologue-6",
    (const PTR) output_492,
    0,
    &operand_data[863],
    4,
    0,
    1,
    3
  },
  {
    "prologue-5",
    (const PTR) output_493,
    0,
    &operand_data[869],
    8,
    0,
    1,
    3
  },
  {
    "prologue-4",
    (const PTR) output_494,
    0,
    &operand_data[870],
    6,
    0,
    1,
    3
  },
  {
    "prologue-3",
    (const PTR) output_495,
    0,
    &operand_data[871],
    4,
    0,
    1,
    3
  },
  {
    "prologue-2",
    0,
    0,
    &operand_data[877],
    6,
    0,
    0,
    0
  },
  {
    "prologue-1",
    0,
    0,
    &operand_data[883],
    3,
    0,
    0,
    0
  },
  {
    "prologue",
    0,
    (insn_gen_fn) gen_prologue,
    &operand_data[0],
    0,
    0,
    0,
    0
  },
  {
    "epilogue",
    0,
    (insn_gen_fn) gen_epilogue,
    &operand_data[0],
    0,
    0,
    0,
    0
  },
  {
    "eh_epilogue",
    0,
    (insn_gen_fn) gen_eh_epilogue,
    &operand_data[886],
    3,
    0,
    1,
    0
  },
  {
    "eh_epilogue+1",
    0,
    0,
    &operand_data[889],
    5,
    0,
    0,
    0
  },
  {
    "eh_epilogue+2",
    0,
    0,
    &operand_data[889],
    5,
    0,
    0,
    0
  },
  {
    "tablejump-2",
    0,
    0,
    &operand_data[889],
    6,
    0,
    0,
    0
  },
  {
    "tablejump-1",
    0,
    0,
    &operand_data[895],
    6,
    0,
    0,
    0
  },
  {
    "tablejump",
    0,
    (insn_gen_fn) gen_tablejump,
    &operand_data[406],
    2,
    0,
    1,
    0
  },
  {
    "ffssi2",
    0,
    (insn_gen_fn) gen_ffssi2,
    &operand_data[22],
    2,
    0,
    0,
    0
  },
  {
    "ffssi2+1",
    (const PTR) output_507,
    0,
    &operand_data[901],
    5,
    0,
    3,
    2
  },
  {
    "ffssi2+2",
    (const PTR) output_508,
    0,
    &operand_data[906],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+3",
    (const PTR) output_509,
    0,
    &operand_data[911],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+4",
    (const PTR) output_510,
    0,
    &operand_data[916],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+5",
    "adf%?d\t%0, %1, %2",
    0,
    &operand_data[921],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+6",
    "adf%?d\t%0, %1, %2",
    0,
    &operand_data[926],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+7",
    (const PTR) output_513,
    0,
    &operand_data[931],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+8",
    (const PTR) output_514,
    0,
    &operand_data[936],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+9",
    (const PTR) output_515,
    0,
    &operand_data[941],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+10",
    "suf%?d\t%0, %1, %2",
    0,
    &operand_data[946],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+11",
    (const PTR) output_517,
    0,
    &operand_data[951],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+12",
    "suf%?d\t%0, %1, %2",
    0,
    &operand_data[926],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+13",
    (const PTR) output_519,
    0,
    &operand_data[956],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+14",
    "mul%?\t%0, %2, %1",
    0,
    &operand_data[961],
    5,
    0,
    2,
    1
  },
  {
    "ffssi2+15",
    "mla%?\t%0, %2, %1, %3",
    0,
    &operand_data[966],
    6,
    0,
    4,
    1
  },
  {
    "ffssi2+16",
    "smlal%?\t%Q0, %R0, %3, %2",
    0,
    &operand_data[972],
    6,
    0,
    1,
    1
  },
  {
    "ffssi2+17",
    "smull%?\t%Q0, %R0, %1, %2",
    0,
    &operand_data[978],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+18",
    "umull%?\t%Q0, %R0, %1, %2",
    0,
    &operand_data[978],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+19",
    "umlal%?\t%Q0, %R0, %3, %2",
    0,
    &operand_data[972],
    6,
    0,
    1,
    1
  },
  {
    "ffssi2+20",
    "smull%?\t%3, %0, %2, %1",
    0,
    &operand_data[983],
    6,
    0,
    2,
    1
  },
  {
    "ffssi2+21",
    "umull%?\t%3, %0, %2, %1",
    0,
    &operand_data[983],
    6,
    0,
    2,
    1
  },
  {
    "ffssi2+22",
    "fml%?s\t%0, %1, %2",
    0,
    &operand_data[989],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+23",
    "muf%?d\t%0, %1, %2",
    0,
    &operand_data[994],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+24",
    "muf%?d\t%0, %1, %2",
    0,
    &operand_data[946],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+25",
    "muf%?d\t%0, %1, %2",
    0,
    &operand_data[921],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+26",
    "muf%?d\t%0, %1, %2",
    0,
    &operand_data[926],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+27",
    "muf%?e\t%0, %1, %2",
    0,
    &operand_data[999],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+28",
    (const PTR) output_534,
    0,
    &operand_data[1004],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+29",
    (const PTR) output_535,
    0,
    &operand_data[941],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+30",
    "dvf%?d\t%0, %1, %2",
    0,
    &operand_data[946],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+31",
    "rdf%?d\t%0, %2, %1",
    0,
    &operand_data[1009],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+32",
    "dvf%?d\t%0, %1, %2",
    0,
    &operand_data[926],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+33",
    (const PTR) output_539,
    0,
    &operand_data[956],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+34",
    "rmf%?s\t%0, %1, %2",
    0,
    &operand_data[989],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+35",
    "rmf%?d\t%0, %1, %2",
    0,
    &operand_data[994],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+36",
    "rmf%?d\t%0, %1, %2",
    0,
    &operand_data[946],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+37",
    "rmf%?d\t%0, %1, %2",
    0,
    &operand_data[921],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+38",
    "rmf%?d\t%0, %1, %2",
    0,
    &operand_data[926],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+39",
    "rmf%?e\t%0, %1, %2",
    0,
    &operand_data[999],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+40",
    (const PTR) output_546,
    0,
    &operand_data[1014],
    5,
    0,
    3,
    2
  },
  {
    "ffssi2+41",
    "#",
    0,
    &operand_data[1019],
    5,
    0,
    2,
    1
  },
  {
    "ffssi2+42",
    (const PTR) output_548,
    0,
    &operand_data[1024],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+43",
    "#",
    0,
    &operand_data[1029],
    5,
    0,
    2,
    1
  },
  {
    "ffssi2+44",
    "bic%?\t%0, %1, %2",
    0,
    &operand_data[1034],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+45",
    "bic%?\t%0, %1, %2%S4",
    0,
    &operand_data[1039],
    7,
    0,
    1,
    1
  },
  {
    "ffssi2+46",
    "#",
    0,
    &operand_data[1046],
    5,
    0,
    2,
    1
  },
  {
    "ffssi2+47",
    (const PTR) output_553,
    0,
    &operand_data[1024],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+48",
    "#",
    0,
    &operand_data[1051],
    5,
    0,
    2,
    1
  },
  {
    "ffssi2+49",
    (const PTR) output_555,
    0,
    &operand_data[1056],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+50",
    "#",
    0,
    &operand_data[1046],
    5,
    0,
    2,
    1
  },
  {
    "ffssi2+51",
    (const PTR) output_557,
    0,
    &operand_data[1024],
    5,
    0,
    2,
    2
  },
  {
    "ffssi2+52",
    "#",
    0,
    &operand_data[1051],
    5,
    0,
    2,
    1
  },
  {
    "ffssi2+53",
    "eor%?\t%0, %1, %2",
    0,
    &operand_data[1061],
    5,
    0,
    1,
    1
  },
  {
    "ffssi2+54",
    "orr%?\t%0, %1, %2\n\tbic%?\t%0, %0, %3",
    0,
    &operand_data[1066],
    6,
    0,
    3,
    1
  },
  {
    "ffssi2+55",
    "mov%?\t%0, %1%S3",
    0,
    &operand_data[1072],
    6,
    0,
    1,
    1
  },
  {
    "ffssi2+56",
    "mvn%?\t%0, %1%S3",
    0,
    &operand_data[1078],
    6,
    0,
    1,
    1
  },
  {
    "ffssi2+57",
    "rsb%?\t%0, %1, #0",
    0,
    &operand_data[1084],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+58",
    "mnf%?s\t%0, %1",
    0,
    &operand_data[1088],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+59",
    "mnf%?d\t%0, %1",
    0,
    &operand_data[1092],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+60",
    "mnf%?d\t%0, %1",
    0,
    &operand_data[1096],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+61",
    "mnf%?e\t%0, %1",
    0,
    &operand_data[1100],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+62",
    "abs%?s\t%0, %1",
    0,
    &operand_data[1088],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+63",
    "abs%?d\t%0, %1",
    0,
    &operand_data[1092],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+64",
    "abs%?d\t%0, %1",
    0,
    &operand_data[1096],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+65",
    "abs%?e\t%0, %1",
    0,
    &operand_data[1100],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+66",
    "sqt%?s\t%0, %1",
    0,
    &operand_data[1088],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+67",
    "sqt%?d\t%0, %1",
    0,
    &operand_data[1092],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+68",
    "sqt%?d\t%0, %1",
    0,
    &operand_data[1096],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+69",
    "sqt%?e\t%0, %1",
    0,
    &operand_data[1100],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+70",
    "#",
    0,
    &operand_data[1104],
    4,
    0,
    2,
    1
  },
  {
    "ffssi2+71",
    "mvn%?\t%0, %1",
    0,
    &operand_data[1084],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+72",
    "flt%?s\t%0, %1",
    0,
    &operand_data[1108],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+73",
    "flt%?d\t%0, %1",
    0,
    &operand_data[1112],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+74",
    "flt%?e\t%0, %1",
    0,
    &operand_data[1116],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+75",
    "fix%?z\t%0, %1",
    0,
    &operand_data[1120],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+76",
    "fix%?z\t%0, %1",
    0,
    &operand_data[1124],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+77",
    "fix%?z\t%0, %1",
    0,
    &operand_data[1128],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+78",
    "mvf%?s\t%0, %1",
    0,
    &operand_data[1132],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+79",
    "mvf%?s\t%0, %1",
    0,
    &operand_data[1136],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+80",
    "mvf%?d\t%0, %1",
    0,
    &operand_data[1140],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+81",
    (const PTR) output_587,
    0,
    &operand_data[1144],
    4,
    0,
    1,
    3
  },
  {
    "ffssi2+82",
    (const PTR) output_588,
    0,
    &operand_data[1148],
    4,
    0,
    2,
    2
  },
  {
    "ffssi2+83",
    (const PTR) output_589,
    0,
    &operand_data[1144],
    4,
    0,
    1,
    3
  },
  {
    "ffssi2+84",
    "ldr%?h\t%0, %1",
    0,
    &operand_data[1152],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+85",
    "ldr%?b\t%0, %1\t%@ zero_extendqisi2",
    0,
    &operand_data[1156],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+86",
    "ldr%?sh\t%0, %1",
    0,
    &operand_data[1152],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+87",
    (const PTR) output_593,
    0,
    &operand_data[1160],
    4,
    0,
    1,
    3
  },
  {
    "ffssi2+88",
    (const PTR) output_594,
    0,
    &operand_data[1156],
    4,
    0,
    1,
    3
  },
  {
    "ffssi2+89",
    "mvf%?d\t%0, %1",
    0,
    &operand_data[1096],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+90",
    "mvf%?e\t%0, %1",
    0,
    &operand_data[1164],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+91",
    "mvf%?e\t%0, %1",
    0,
    &operand_data[1168],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+92",
    (const PTR) output_598,
    0,
    &operand_data[1172],
    4,
    0,
    4,
    2
  },
  {
    "ffssi2+93",
    (const PTR) output_599,
    0,
    &operand_data[1176],
    4,
    1,
    1,
    3
  },
  {
    "ffssi2+94",
    (const PTR) output_600,
    0,
    &operand_data[1180],
    4,
    0,
    1,
    3
  },
  {
    "ffssi2+95",
    (const PTR) output_601,
    0,
    &operand_data[1184],
    4,
    0,
    4,
    2
  },
  {
    "ffssi2+96",
    (const PTR) output_602,
    0,
    &operand_data[1188],
    4,
    0,
    3,
    2
  },
  {
    "ffssi2+97",
    (const PTR) output_603,
    0,
    &operand_data[1188],
    4,
    0,
    3,
    2
  },
  {
    "ffssi2+98",
    "ldr%?\t%0, %1\t%@ movhi_bigend",
    0,
    &operand_data[1152],
    4,
    0,
    1,
    1
  },
  {
    "ffssi2+99",
    (const PTR) output_605,
    0,
    &operand_data[1192],
    4,
    0,
    2,
    2
  },
  {
    "ffssi2+100",
    (const PTR) output_606,
    0,
    &operand_data[1196],
    4,
    0,
    4,
    2
  },
  {
    "ffssi2+101",
    (const PTR) output_607,
    0,
    &operand_data[1200],
    4,
    0,
    9,
    2
  },
  {
    "ffssi2+102",
    (const PTR) output_608,
    0,
    &operand_data[1204],
    4,
    0,
    3,
    2
  },
  {
    "ffssi2+103",
    (const PTR) output_609,
    0,
    &operand_data[1208],
    4,
    0,
    11,
    3
  },
  {
    "ffssi2+104",
    (const PTR) output_610,
    0,
    &operand_data[1212],
    4,
    0,
    7,
    3
  },
  {
    "ffssi2+105",
    "ldm%?ia\t%1!, {%3, %4, %5, %6}",
    0,
    &operand_data[1216],
    9,
    4,
    1,
    1
  },
  {
    "ffssi2+106",
    "ldm%?ia\t%1!, {%3, %4, %5}",
    0,
    &operand_data[1225],
    8,
    3,
    1,
    1
  },
  {
    "ffssi2+107",
    "ldm%?ia\t%1!, {%3, %4}",
    0,
    &operand_data[1233],
    7,
    2,
    1,
    1
  },
  {
    "ffssi2+108",
    "ldm%?ia\t%1, {%2, %3, %4, %5}",
    0,
    &operand_data[1240],
    8,
    3,
    1,
    1
  },
  {
    "ffssi2+109",
    "ldm%?ia\t%1, {%2, %3, %4}",
    0,
    &operand_data[1248],
    7,
    2,
    1,
    1
  },
  {
    "ffssi2+110",
    "ldm%?ia\t%1, {%2, %3}",
    0,
    &operand_data[1255],
    6,
    1,
    1,
    1
  },
  {
    "ffssi2+111",
    "stm%?ia\t%1!, {%3, %4, %5, %6}",
    0,
    &operand_data[1261],
    9,
    4,
    1,
    1
  },
  {
    "ffssi2+112",
    "stm%?ia\t%1!, {%3, %4, %5}",
    0,
    &operand_data[1270],
    8,
    3,
    1,
    1
  },
  {
    "ffssi2+113",
    "stm%?ia\t%1!, {%3, %4}",
    0,
    &operand_data[1278],
    7,
    2,
    1,
    1
  },
  {
    "ffssi2+114",
    "stm%?ia\t%1, {%2, %3, %4, %5}",
    0,
    &operand_data[1285],
    8,
    3,
    1,
    1
  },
  {
    "ffssi2+115",
    "stm%?ia\t%1, {%2, %3, %4}",
    0,
    &operand_data[1293],
    7,
    2,
    1,
    1
  },
  {
    "ffssi2+116",
    "stm%?ia\t%1, {%2, %3}",
    0,
    &operand_data[1300],
    6,
    1,
    1,
    1
  },
  {
    "ffssi2+117",
    (const PTR) output_623,
    0,
    &operand_data[374],
    3,
    0,
    0,
    3
  },
  {
    "ffssi2+118",
    (const PTR) output_624,
    0,
    &operand_data[375],
    2,
    0,
    0,
    3
  },
  {
    "ffssi2+119",
    "mov%?\t%|pc, %0\t%@ indirect register jump",
    0,
    &operand_data[975],
    3,
    0,
    1,
    1
  },
  {
    "ffssi2+120",
    "ldr%?\t%|pc, %0\t%@ indirect memory jump",
    0,
    &operand_data[1306],
    3,
    0,
    1,
    1
  },
  {
    "ffssi2+121",
    "%i1%?\t%0, %2, %4%S3",
    0,
    &operand_data[1309],
    8,
    0,
    1,
    1
  },
  {
    "ffssi2+122",
    "sub%?\t%0, %1, %3%S2",
    0,
    &operand_data[1317],
    7,
    0,
    1,
    1
  },
  {
    "ffssi2+123",
    (const PTR) output_629,
    0,
    &operand_data[1324],
    7,
    0,
    1,
    3
  },
  {
    "ffssi2+124",
    "str%?b\t%3, [%0, %2]!",
    0,
    &operand_data[1331],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+125",
    "str%?b\t%3, [%0, -%2]!",
    0,
    &operand_data[1337],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+126",
    "ldr%?b\t%3, [%0, %2]!",
    0,
    &operand_data[1343],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+127",
    "ldr%?b\t%3, [%0, -%2]!",
    0,
    &operand_data[1349],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+128",
    "ldr%?b\t%3, [%0, %2]!\t%@ z_extendqisi",
    0,
    &operand_data[1355],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+129",
    "ldr%?b\t%3, [%0, -%2]!\t%@ z_extendqisi",
    0,
    &operand_data[1361],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+130",
    "str%?\t%3, [%0, %2]!",
    0,
    &operand_data[1367],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+131",
    "str%?\t%3, [%0, -%2]!",
    0,
    &operand_data[1373],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+132",
    "ldr%?\t%3, [%0, %2]!",
    0,
    &operand_data[1355],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+133",
    "ldr%?\t%3, [%0, -%2]!",
    0,
    &operand_data[1361],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+134",
    "ldr%?\t%3, [%0, %2]!\t%@ loadhi",
    0,
    &operand_data[1379],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+135",
    "ldr%?\t%3, [%0, -%2]!\t%@ loadhi",
    0,
    &operand_data[1385],
    6,
    2,
    1,
    1
  },
  {
    "ffssi2+136",
    "str%?b\t%5, [%0, %3%S2]!",
    0,
    &operand_data[1391],
    8,
    4,
    1,
    1
  },
  {
    "ffssi2+137",
    "str%?b\t%5, [%0, -%3%S2]!",
    0,
    &operand_data[1391],
    8,
    4,
    1,
    1
  },
  {
    "ffssi2+138",
    "ldr%?b\t%5, [%0, %3%S2]!",
    0,
    &operand_data[1399],
    8,
    4,
    1,
    1
  },
  {
    "ffssi2+139",
    "ldr%?b\t%5, [%0, -%3%S2]!",
    0,
    &operand_data[1399],
    8,
    4,
    1,
    1
  },
  {
    "ffssi2+140",
    "str%?\t%5, [%0, %3%S2]!",
    0,
    &operand_data[1407],
    8,
    4,
    1,
    1
  },
  {
    "ffssi2+141",
    "str%?\t%5, [%0, -%3%S2]!",
    0,
    &operand_data[1407],
    8,
    4,
    1,
    1
  },
  {
    "ffssi2+142",
    "ldr%?\t%5, [%0, %3%S2]!",
    0,
    &operand_data[1415],
    8,
    4,
    1,
    1
  },
  {
    "ffssi2+143",
    "ldr%?\t%5, [%0, -%3%S2]!",
    0,
    &operand_data[1415],
    8,
    4,
    1,
    1
  },
  {
    "ffssi2+144",
    "ldr%?\t%5, [%0, %3%S2]!\t%@ loadhi",
    0,
    &operand_data[1423],
    8,
    4,
    1,
    1
  },
  {
    "ffssi2+145",
    "ldr%?\t%5, [%0, -%3%S2]!\t%@ loadhi",
    0,
    &operand_data[1423],
    8,
    4,
    1,
    1
  },
};


const char *
get_insn_name (code)
     int code;
{
  return insn_data[code].name;
}
