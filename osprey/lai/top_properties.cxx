

#include "top_properties.h"
#include "variants.h"
#include "op.h"

/*
 * Definition for the largest host int type.
 */
#define INTM_BITS 64
typedef INT64 intm_t;
typedef UINT64 uintm_t;

/*
 * Definition of the open64 interface.
 */
typedef TOP opc_t;
typedef TN *opnd_t;

#define TN_Value(tn) ((tn == Zero_TN) ? 0 : TN_value(tn))
static intm_t opnd_value(opnd_t opnd) { return TN_Value(opnd); }
static int opc_opnd_signed(opc_t opc, int opnd) { return TOP_opnd_use_signed(opc, opnd); }
static int opc_opnd_bits(opc_t opc, int opnd) { return TOP_opnd_use_bits(opc, opnd); }
static int opc_ou_opnd_idx(opc_t opc, int ou) { return TOP_Find_Operand_Use(opc, ou); }
static int opc_cond_variant(opc_t opc) { return TOP_cond_variant(opc); }
static int opc_cmp_variant(opc_t opc) { return TOP_cmp_variant(opc); }

#define opc_is(opc, prop) TOP_is_##prop(opc)

/*
 * Utility functions.
 */

/* Zero extension. 
 * 0 <= bits <= INTM_BITS
 * Returns (value mod 2^bits)
 */
static intm_t zext(intm_t value, int bits) {
  if (bits == 0) return 0;
  return ((uintm_t)value << INTM_BITS-bits) >> INTM_BITS-bits;
}

/* Sign extension. 
 * 0 <= bits <= INTM_BITS
 * Return (value cmod 2^bits) where
 * cmod is the rest of the centered division.
 */
static intm_t sext(intm_t value, int bits) {
  if (bits == 0) return 0;
  return (value << INTM_BITS-bits) >> INTM_BITS-bits;
}

/* Operand fetch. */
static intm_t fetch(opc_t opc, opnd_t *opnds, int opnd) {
  intm_t value = opnd_value(opnds[opnd]);
  if (opc_opnd_signed(opc, opnd))
    value = sext(value, opc_opnd_bits(opc, opnd));
  else
    value = zext(value, opc_opnd_bits(opc, opnd));
  return value;
}

/*
 * Fetching an operand.
 */
intm_t TOP_fetch_opnd(TOP opc, TN **opnds, int opnd) { return fetch(opc, opnds, opnd); }

/*
 * Constant folding of opcode properties.
 */
intm_t TOP_fold_iadd(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  result = op1 + op2;
  return result;
}

intm_t TOP_fold_isub(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  result = op1 - op2;
  return result;
}

intm_t TOP_fold_imul(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  result = op1 * op2;
  return result;
}

intm_t TOP_fold_not(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  intm_t op1 = fetch(opc, opnds, opnd1);
  result = ~op1;
  return result;
}

intm_t TOP_fold_and(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  result = op1 & op2;
  return result;
}

intm_t TOP_fold_or(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  result = op1 | op2;
  return result;
}

intm_t TOP_fold_xor(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  result = op1 ^ op2;
  return result;
}

intm_t TOP_fold_shl(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  if (op2 < opc_opnd_bits(opc, opnd1))
    result = op1 << op2;
  else
    result = 0;
  return result;
}

intm_t TOP_fold_shr(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  if (op2 < opc_opnd_bits(opc, opnd1))
    result = op1 >> op2;
  else
    result = op1 < 0 ? -1: 0;
  return result;
}

intm_t TOP_fold_shru(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  if (op2 < opc_opnd_bits(opc, opnd1))
    result = (uintm_t)op1 >> op2;
  else
    result = op1 < 0 ? -1: 0;
  return result;
}

intm_t TOP_fold_move(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  result = fetch(opc, opnds, opnd1);
  return result;
}


intm_t TOP_fold_zext(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  result = fetch(opc, opnds, opnd1);
  return result;
}

intm_t TOP_fold_sext(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  result = fetch(opc, opnds, opnd1);
  return result;
}

intm_t TOP_fold_select(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int condition = opc_ou_opnd_idx(opc, OU_condition);
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t cond = fetch(opc, opnds, condition);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  VARIANT variant = opc_cond_variant(opc);
  if ((variant == V_COND_TRUE && cond != 0) ||
      (variant == V_COND_FALSE && cond == 0))
    result = op1;
  else
    result = op2;
  return result;
}

intm_t TOP_fold_icmp(opc_t opc, opnd_t *opnds) {
  intm_t result;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);
  intm_t op1 = fetch(opc, opnds, opnd1);
  intm_t op2 = fetch(opc, opnds, opnd2);
  VARIANT variant = opc_cmp_variant(opc);
  switch (variant) {
  case V_CMP_NE: result = (op1 != op2); break;
  case V_CMP_EQ: result = (op1 == op2); break;
  case V_CMP_LE: result = (op1 <= op2); break;
  case V_CMP_LEU: result = ((uintm_t)op1 <= op2); break;
  case V_CMP_LT: result = (op1 < op2); break;
  case V_CMP_LTU: result = ((uintm_t)op1 < op2); break;
  case V_CMP_GT: result = (op1 > op2); break;
  case V_CMP_GTU: result = ((uintm_t)op1 > op2); break;
  case V_CMP_GE: result = (op1 >= op2); break;
  case V_CMP_GEU: result = ((uintm_t)op1 >= op2); break;
  case V_CMP_ANDL: result = (op1 != 0) & (op2 != 0); break;
  case V_CMP_NANDL: result = (op1 = 0) | (op2 = 0); break;
  case V_CMP_ORL: result = (op1 != 0) | (op2 != 0); break;
  case V_CMP_NORL: result = (op1 = 0) & (op2 = 0); break;
  }
  return result;
}

#ifdef TARG_ST200
// [CG]: Just to check some ST200 properties
// Should be moved elsewhere or discarded.
#define assert_equals(x,y,s) { FmtAssert((INT32)(x) == (INT32)(y), ("unexpect result: %s", s)); }
static int check_st200_opcodes(void)
{
  TN *opnds[3];
  TN *tn_0 = Gen_Literal_TN(0, 4);
  TN *tn_1 = Gen_Literal_TN(1, 4);
  TN *tn_m1 = Gen_Literal_TN(-1, 4);
  TN *tn_31 = Gen_Literal_TN(31, 4);
  TN *tn_32 = Gen_Literal_TN(32, 4);
  TN *tn_255 = Gen_Literal_TN(255, 4);
  TN *tn_256 = Gen_Literal_TN(256, 4);
  TN *tn_imin = Gen_Literal_TN(0x80000000, 4);
  TN *tn_imax = Gen_Literal_TN(0x7fffffff, 4);
  INT64 res;

  opnds[0] = tn_0; opnds[1] = tn_1;
  assert_equals(TOP_fold_iadd(TOP_add_r, opnds), 1, "add_r 0 1 != 1"); 
  opnds[0] = tn_0; opnds[1] = tn_m1;
  assert_equals(TOP_fold_iadd(TOP_add_r, opnds), -1, "add_r 0 -1 != 1"); 
  opnds[0] = tn_1; opnds[1] = tn_imax;
  assert_equals(TOP_fold_iadd(TOP_add_r, opnds), 0x80000000, "add_r 1 imax != imin"); 
  opnds[0] = tn_0; opnds[1] = tn_1;

  assert_equals(TOP_fold_isub(TOP_sub_r, opnds), -1, "sub_r 0 1 != -1"); 
  opnds[0] = tn_0; opnds[1] = tn_m1;
  assert_equals(TOP_fold_isub(TOP_sub_r, opnds), 1, "sub_r 0 -1 != 1"); 
  opnds[0] = tn_0; opnds[1] = tn_imin;
  assert_equals(TOP_fold_isub(TOP_sub_r, opnds), 0x80000000, "sub_r 0 imin != imin"); 
  opnds[0] = tn_0; opnds[1] = tn_imax;
  assert_equals(TOP_fold_isub(TOP_sub_r, opnds), 0x80000001, "sub_r 0 imax != imin+1"); 

  opnds[0] = tn_imin; opnds[1] = tn_0;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0x80000000, "shr_r imin 0 != imin"); 
  opnds[0] = tn_imin; opnds[1] = tn_1;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0xc0000000, "shr_r imin 1 != 0xc0000000"); 
  opnds[0] = tn_imin; opnds[1] = tn_31;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), -1, "shr_r imin 31 != -1"); 
  opnds[0] = tn_imin; opnds[1] = tn_32;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), -1, "shr_r imin 32 != -1"); 
  opnds[0] = tn_imin; opnds[1] = tn_255;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), -1, "shr_r imin 255 != -1"); 
  opnds[0] = tn_imin; opnds[1] = tn_256;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0x80000000, "shr_r imin 256 != imin"); 
  opnds[0] = tn_imin; opnds[1] = tn_m1;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), -1, "shr_r imin -1 != -1"); 

  opnds[0] = tn_imax; opnds[1] = tn_0;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0x7fffffff, "shr_r imax 0 != imax"); 
  opnds[0] = tn_imax; opnds[1] = tn_1;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0x3fffffff, "shr_r imax 1 != 0x3fffffff"); 
  opnds[0] = tn_imax; opnds[1] = tn_31;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0, "shr_r imax 31 != 0"); 
  opnds[0] = tn_imax; opnds[1] = tn_32;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0, "shr_r imax 32 != 0"); 
  opnds[0] = tn_imax; opnds[1] = tn_255;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0, "shr_r imax 255 != 0"); 
  opnds[0] = tn_imax; opnds[1] = tn_256;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0x7fffffff, "shr_r imax 256 != imax"); 
  opnds[0] = tn_imax; opnds[1] = tn_m1;
  assert_equals(TOP_fold_shr(TOP_shr_r, opnds), 0, "shr_r imax -1 != 0"); 

  opnds[0] = tn_imin; opnds[1] = tn_0;
  assert_equals(TOP_fold_shru(TOP_shru_r, opnds), 0x80000000, "shru_r imin 1 != imin"); 
  opnds[0] = tn_imin; opnds[1] = tn_1;
  assert_equals(TOP_fold_shru(TOP_shru_r, opnds), 0x40000000, "shru_r imin 1 != 0x40000000"); 
  opnds[0] = tn_m1; opnds[1] = tn_31;
  assert_equals(TOP_fold_shru(TOP_shru_r, opnds), 1, "shru_r -1 31 != 1"); 
  opnds[0] = tn_m1; opnds[1] = tn_32;
  assert_equals(TOP_fold_shru(TOP_shru_r, opnds), 0, "shru_r -1 32 != 0"); 
  opnds[0] = tn_m1; opnds[1] = tn_255;
  assert_equals(TOP_fold_shru(TOP_shru_r, opnds), 0, "shru_r -1 255 != 0"); 
  opnds[0] = tn_m1; opnds[1] = tn_256;
  assert_equals(TOP_fold_shru(TOP_shru_r, opnds), -1, "shru_r -1 256 != -1"); 
  opnds[0] = tn_m1; opnds[1] = tn_m1;
  assert_equals(TOP_fold_shru(TOP_shru_r, opnds), 0, "shru_r -1 -1 != 0"); 

  opnds[0] = tn_1; opnds[1] = tn_0;
  assert_equals(TOP_fold_shl(TOP_shl_r, opnds), 1, "shrl_r 1 0 != 0"); 
  opnds[0] = tn_1; opnds[1] = tn_31;
  assert_equals(TOP_fold_shl(TOP_shl_r, opnds), 0x80000000, "shrl_r 1 31 != 0x80000000"); 
  opnds[0] = tn_1; opnds[1] = tn_32;
  assert_equals(TOP_fold_shl(TOP_shl_r, opnds), 0, "shrl_r 1 32 != 0"); 
  opnds[0] = tn_1; opnds[1] = tn_255;
  assert_equals(TOP_fold_shl(TOP_shl_r, opnds), 0, "shrl_r 1 255 != 0"); 
  opnds[0] = tn_1; opnds[1] = tn_256;
  assert_equals(TOP_fold_shl(TOP_shl_r, opnds), 1, "shrl_r 1 256 != 1"); 
  opnds[0] = tn_1; opnds[1] = tn_m1;
  assert_equals(TOP_fold_shl(TOP_shl_r, opnds), 0, "shrl_r 1 -1 != 0"); 

  opnds[0] = tn_m1; opnds[1] = tn_255;
  assert_equals(TOP_fold_and(TOP_and_r, opnds), 255, "and_r -1 255 != 255"); 
  opnds[0] = tn_m1; opnds[1] = tn_255;
  assert_equals(TOP_fold_or(TOP_and_r, opnds), -1, "or_r -1 255 != -1"); 
  opnds[0] = tn_m1; opnds[1] = tn_255;
  assert_equals(TOP_fold_xor(TOP_xor_r, opnds), 0xffffff00, "xor_r -1 255 != 0xffffff00"); 
  return 0;
}
#endif

/*
 * Constraints on properties.
 */
int TOP_check_properties(opc_t opc)
{
  int ok = 1;
  int opnd1 = opc_ou_opnd_idx(opc, OU_opnd1);
  int opnd2 = opc_ou_opnd_idx(opc, OU_opnd2);

  /* Check that host sign extends. */
  if ((intm_t)-1 >> 1 != -1) return 0;
  /* Check that host is 2 complement. */
  if ((intm_t)-1 != ~(uintm_t)1 + 1) return 0;

  if (opc_is(opc, add) ||
      opc_is(opc, sub) ||
      opc_is(opc, and) ||
      opc_is(opc, or) ||
      opc_is(opc, xor) ||
      opc_is(opc, select) ||
      opc_is(opc, cmp)) {
    if (opnd1 < 0 || opnd2 < 0) return 0;
    if (opc_opnd_bits(opc,opnd1) != opc_opnd_bits(opc,opnd2)) return 0;
    if (opc_opnd_signed(opc,opnd1) != opc_opnd_signed(opc,opnd2)) return 0;
    if (opc_opnd_bits(opc,opnd1) <= 0) return 0;
  }
  if (opc_is(opc, mul)) {
    if (opnd1 < 0 || opnd2 < 0) return 0;
    if (opc_opnd_bits(opc,opnd1) <= 0) return 0;
    if (opc_opnd_bits(opc,opnd2) <= 0) return 0;
  }
  //if (opc_is(opc, not)) {
  //if (opnd1 < 0) return 0;
  //if (opc_opnd_bits(opc,opnd1) <= 0) return 0;
  //}
  if (opc_is(opc, move)) {
    if (opnd1 < 0) return 0;
    if (opc_opnd_bits(opc,opnd1) <= 0) return 0;
  }
  if (opc_is(opc, sext)) {
    if (opnd1 < 0) return 0;
    if (opc_opnd_bits(opc,opnd1) <= 0) return 0;
    if (!opc_opnd_signed(opc,opnd1)) return 0;
  }
  if (opc_is(opc, zext)) {
    if (opnd1 < 0) return 0;
    if (opc_opnd_bits(opc,opnd1) <= 0) return 0;
    if (opc_opnd_signed(opc,opnd1)) return 0;
  }
  if (opc_is(opc, select)) {
    int condition = opc_ou_opnd_idx(opc, OU_condition);
    if (condition < 0) return 0;
    if (opnd1 < 0 || opnd2 < 0) return 0;
    if (opc_opnd_bits(opc,opnd1) != opc_opnd_bits(opc,opnd2)) return 0;
    if (opc_opnd_signed(opc,opnd1) != opc_opnd_signed(opc,opnd2)) return 0;
  }
  if (opc_is(opc, load)) {
    int base = opc_ou_opnd_idx(opc, OU_base);
    int offset = opc_ou_opnd_idx(opc, OU_offset);
    if (base < 0 || offset < 0) return 0;
    if (opc_opnd_bits(opc,base) != opc_opnd_bits(opc,offset)) return 0;
    if (opc_opnd_signed(opc,base) != opc_opnd_signed(opc,offset)) return 0;
  }
  if (opc_is(opc, store)) {
    int base = opc_ou_opnd_idx(opc, OU_base);
    int offset = opc_ou_opnd_idx(opc, OU_offset);
    int storeval = opc_ou_opnd_idx(opc, OU_storeval);
    if (base < 0 || offset < 0 || storeval < 0) return 0;
    if (opc_opnd_bits(opc,base) != opc_opnd_bits(opc,offset)) return 0;
    if (opc_opnd_signed(opc,base) != opc_opnd_signed(opc,offset)) return 0;
  }

#ifdef TARG_ST200
  check_st200_opcodes();
#endif

  return 1;
}

