#include "defs.h"
#include "config.h"
#include "erglob.h"
#include "ercg.h"
#include "glob.h"
#include "tracing.h"
#include "util.h"

#include "symtab.h"
#include "opcode.h"
#include "intrn_info.h"
#include "const.h" /* needed to manipulate target/host consts */
#include "targ_const.h" /* needed to manipulate target/host consts */
#include "cgir.h"

#include "topcode.h"
#include "targ_isa_lits.h"
#include "targ_isa_properties.h"

/*
 * Expansion of absl based on validated and scheduled basic assembly source.
*/
static void
Expand__absl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0 = Gen_Literal_TN(0, 4) ;
  TN *c_1 = Gen_Literal_TN(-1, 4) ;
  Build_OP (	TOP_cmpgtu_i_b,	b0_0_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_0,	ih0,	c0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_21_0,	c0,	ih0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_22_0,	c_1,	ih0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_1,	b0_0_0,	r0_22_0,	r0_21_0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_20_1,	c0,	il0,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_1_0,	r0_20_1,	il0,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_1_0,	r0_21_1,	ih0,	ops) ;
} /* Expand__absl */

/*
@@@  case INTRN_ABSL:
@@@    Expand__absl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of addl based on validated and scheduled basic assembly source.
*/
static void
Expand__addl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  Build_OP (	TOP_mtb,	b0_0_0,	Zero_TN,	ops) ;
  Build_OP (	TOP_addcg,	ol0,	b0_0_1,	il0,	il1,	b0_0_0,	ops) ;
  Build_OP (	TOP_addcg,	oh0,	b0_0_2,	ih0,	ih1,	b0_0_1,	ops) ;
} /* Expand__addl */

/*
@@@  case INTRN_ADDL:
@@@    Expand__addl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of addul based on validated and scheduled basic assembly source.
*/
static void
Expand__addul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  Build_OP (	TOP_mtb,	b0_0_0,	Zero_TN,	ops) ;
  Build_OP (	TOP_addcg,	ol0,	b0_0_1,	il0,	il1,	b0_0_0,	ops) ;
  Build_OP (	TOP_addcg,	oh0,	b0_0_2,	ih0,	ih1,	b0_0_1,	ops) ;
} /* Expand__addul */

/*
@@@  case INTRN_ADDUL:
@@@    Expand__addul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of eql based on validated and scheduled basic assembly source.
*/
static void
Expand__eql(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_andl_r_r,	o0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__eql */

/*
@@@  case INTRN_EQL:
@@@    Expand__eql(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of equl based on validated and scheduled basic assembly source.
*/
static void
Expand__equl(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_andl_r_r,	o0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__equl */

/*
@@@  case INTRN_EQUL:
@@@    Expand__equl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of gel based on validated and scheduled basic assembly source.
*/
static void
Expand__gel(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgeu_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpge_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__gel */

/*
@@@  case INTRN_GEL:
@@@    Expand__gel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of geul based on validated and scheduled basic assembly source.
*/
static void
Expand__geul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgeu_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpgeu_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__geul */

/*
@@@  case INTRN_GEUL:
@@@    Expand__geul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of gtl based on validated and scheduled basic assembly source.
*/
static void
Expand__gtl(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgtu_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpgt_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__gtl */

/*
@@@  case INTRN_GTL:
@@@    Expand__gtl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of gtul based on validated and scheduled basic assembly source.
*/
static void
Expand__gtul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgtu_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpgtu_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__gtul */

/*
@@@  case INTRN_GTUL:
@@@    Expand__gtul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of lel based on validated and scheduled basic assembly source.
*/
static void
Expand__lel(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpleu_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmple_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__lel */

/*
@@@  case INTRN_LEL:
@@@    Expand__lel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of leul based on validated and scheduled basic assembly source.
*/
static void
Expand__leul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpleu_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpleu_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__leul */

/*
@@@  case INTRN_LEUL:
@@@    Expand__leul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of ltl based on validated and scheduled basic assembly source.
*/
static void
Expand__ltl(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmplt_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__ltl */

/*
@@@  case INTRN_LTL:
@@@    Expand__ltl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of ltul based on validated and scheduled basic assembly source.
*/
static void
Expand__ltul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpltu_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__ltul */

/*
@@@  case INTRN_LTUL:
@@@    Expand__ltul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of maxl based on validated and scheduled basic assembly source.
*/
static void
Expand__maxl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgt_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_1_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_maxu_r,	r0_20_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_1,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_1_0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__maxl */

/*
@@@  case INTRN_MAXL:
@@@    Expand__maxl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of maxul based on validated and scheduled basic assembly source.
*/
static void
Expand__maxul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgtu_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_1_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_maxu_r,	r0_20_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_1,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_1_0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__maxul */

/*
@@@  case INTRN_MAXUL:
@@@    Expand__maxul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of minl based on validated and scheduled basic assembly source.
*/
static void
Expand__minl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmplt_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_1_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_minu_r,	r0_20_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_1,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_1_0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__minl */

/*
@@@  case INTRN_MINL:
@@@    Expand__minl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of minul based on validated and scheduled basic assembly source.
*/
static void
Expand__minul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_b,	b0_1_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_minu_r,	r0_20_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_1,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_1_0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__minul */

/*
@@@  case INTRN_MINUL:
@@@    Expand__minul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of mull based on validated and scheduled basic assembly source.
*/
static void
Expand__mull(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_29_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_30_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_30_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_31_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_32_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_32_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_33_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0 = Gen_Literal_TN(0, 4) ;
  Build_OP (	TOP_mullu_r,	r0_24_2,	il0,	il1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_25_2,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_0,	il1,	c0,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_26_3,	il0,	il1,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_27_3,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_i,	r0_28_1,	b0_1_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_slct_i,	r0_29_1,	b0_0_0,	il1,	c0,	ops) ;
  Build_OP (	TOP_mullu_r,	r0_30_4,	ih1,	il0,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_31_4,	ih1,	il0,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_add_r,	r0_28_2,	r0_28_1,	r0_29_1,	ops) ;
  Build_OP (	TOP_mullu_r,	r0_32_5,	ih0,	il1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_33_5,	ih0,	il1,	ops) ;
  Build_OP (	TOP_addcg,	ol0,	b0_3_3,	r0_24_2,	r0_25_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_addcg,	r0_17_4,	b0_3_4,	r0_26_3,	r0_27_3,	b0_3_3,	ops) ;
  Build_OP (	TOP_add_r,	r0_30_5,	r0_30_4,	r0_31_4,	ops) ;
  Build_OP (	TOP_add_r,	r0_17_5,	r0_17_4,	r0_28_2,	ops) ;
  Build_OP (	TOP_add_r,	r0_32_6,	r0_32_5,	r0_33_5,	ops) ;
  Build_OP (	TOP_add_r,	r0_17_6,	r0_17_5,	r0_30_5,	ops) ;
  Build_OP (	TOP_add_r,	oh0,	r0_17_6,	r0_32_6,	ops) ;
} /* Expand__mull */

/*
@@@  case INTRN_MULL:
@@@    Expand__mull(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulul based on validated and scheduled basic assembly source.
*/
static void
Expand__mulul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_29_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_30_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_30_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_31_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_32_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_32_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_33_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0 = Gen_Literal_TN(0, 4) ;
  Build_OP (	TOP_mullu_r,	r0_24_2,	il0,	il1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_25_2,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_0,	il1,	c0,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_26_3,	il0,	il1,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_27_3,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_i,	r0_28_1,	b0_1_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_slct_i,	r0_29_1,	b0_0_0,	il1,	c0,	ops) ;
  Build_OP (	TOP_mullu_r,	r0_30_4,	ih1,	il0,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_31_4,	ih1,	il0,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_add_r,	r0_28_2,	r0_28_1,	r0_29_1,	ops) ;
  Build_OP (	TOP_mullu_r,	r0_32_5,	ih0,	il1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_33_5,	ih0,	il1,	ops) ;
  Build_OP (	TOP_addcg,	ol0,	b0_3_3,	r0_24_2,	r0_25_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_addcg,	r0_17_4,	b0_3_4,	r0_26_3,	r0_27_3,	b0_3_3,	ops) ;
  Build_OP (	TOP_add_r,	r0_30_5,	r0_30_4,	r0_31_4,	ops) ;
  Build_OP (	TOP_add_r,	r0_17_5,	r0_17_4,	r0_28_2,	ops) ;
  Build_OP (	TOP_add_r,	r0_32_6,	r0_32_5,	r0_33_5,	ops) ;
  Build_OP (	TOP_add_r,	r0_17_6,	r0_17_5,	r0_30_5,	ops) ;
  Build_OP (	TOP_add_r,	oh0,	r0_17_6,	r0_32_6,	ops) ;
} /* Expand__mulul */

/*
@@@  case INTRN_MULUL:
@@@    Expand__mulul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of muluw based on validated and scheduled basic assembly source.
*/
static void
Expand__muluw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_mullu_r,	r0_24_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_25_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_add_r,	o0,	r0_24_2,	r0_25_2,	ops) ;
} /* Expand__muluw */

/*
@@@  case INTRN_MULUW:
@@@    Expand__muluw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulw based on validated and scheduled basic assembly source.
*/
static void
Expand__mulw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_mullu_r,	r0_24_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_25_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_add_r,	o0,	r0_24_2,	r0_25_2,	ops) ;
} /* Expand__mulw */

/*
@@@  case INTRN_MULW:
@@@    Expand__mulw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of negl based on validated and scheduled basic assembly source.
*/
static void
Expand__negl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0 = Gen_Literal_TN(0, 4) ;
  TN *c_1 = Gen_Literal_TN(-1, 4) ;
  Build_OP (	TOP_cmpgtu_i_b,	b0_0_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_sub_i,	ol0,	c0,	il0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_17_0,	c0,	ih0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_18_0,	c_1,	ih0,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_0,	r0_18_0,	r0_17_0,	ops) ;
} /* Expand__negl */

/*
@@@  case INTRN_NEGL:
@@@    Expand__negl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of negul based on validated and scheduled basic assembly source.
*/
static void
Expand__negul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0 = Gen_Literal_TN(0, 4) ;
  TN *c_1 = Gen_Literal_TN(-1, 4) ;
  Build_OP (	TOP_cmpgtu_i_b,	b0_0_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_sub_i,	ol0,	c0,	il0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_17_0,	c0,	ih0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_18_0,	c_1,	ih0,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_0,	r0_18_0,	r0_17_0,	ops) ;
} /* Expand__negul */

/*
@@@  case INTRN_NEGUL:
@@@    Expand__negul(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of nel based on validated and scheduled basic assembly source.
*/
static void
Expand__nel(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_nandl_r_r,	o0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__nel */

/*
@@@  case INTRN_NEL:
@@@    Expand__nel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of neul based on validated and scheduled basic assembly source.
*/
static void
Expand__neul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_nandl_r_r,	o0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__neul */

/*
@@@  case INTRN_NEUL:
@@@    Expand__neul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of shll based on validated and scheduled basic assembly source.
*/
static void
Expand__shll(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c32 = Gen_Literal_TN(32, 4) ;
  Build_OP (	TOP_cmpgeu_i_b,	b0_0_0,	i1,	c32,	ops) ;
  Build_OP (	TOP_sub_i,	r0_19_0,	c32,	i1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_17_0,	ih0,	i1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_20_0,	il0,	i1,	ops) ;
  Build_OP (	TOP_shru_r,	r0_21_1,	il0,	r0_19_0,	ops) ;
  Build_OP (	TOP_sub_r,	r0_23_1,	Zero_TN,	r0_19_0,	ops) ;
  Build_OP (	TOP_or_r,	r0_17_2,	r0_17_0,	r0_21_1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_19_2,	il0,	r0_23_1,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_0,	r0_19_2,	r0_17_2,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_0_0,	Zero_TN,	r0_20_0,	ops) ;
} /* Expand__shll */

/*
@@@  case INTRN_SHLL:
@@@    Expand__shll(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of shrl based on validated and scheduled basic assembly source.
*/
static void
Expand__shrl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c31 = Gen_Literal_TN(31, 4) ;
  TN *c32 = Gen_Literal_TN(32, 4) ;
  Build_OP (	TOP_cmpgeu_i_b,	b0_0_0,	i1,	c32,	ops) ;
  Build_OP (	TOP_sub_i,	r0_19_0,	c32,	i1,	ops) ;
  Build_OP (	TOP_shru_r,	r0_16_0,	il0,	i1,	ops) ;
  Build_OP (	TOP_shr_r,	r0_20_0,	ih0,	i1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_21_1,	ih0,	r0_19_0,	ops) ;
  Build_OP (	TOP_shr_i,	r0_22_1,	ih0,	c31,	ops) ;
  Build_OP (	TOP_sub_r,	r0_23_1,	Zero_TN,	r0_19_0,	ops) ;
  Build_OP (	TOP_or_r,	r0_16_2,	r0_16_0,	r0_21_1,	ops) ;
  Build_OP (	TOP_shr_r,	r0_19_2,	ih0,	r0_23_1,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_0_0,	r0_19_2,	r0_16_2,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_0,	r0_22_1,	r0_20_0,	ops) ;
} /* Expand__shrl */

/*
@@@  case INTRN_SHRL:
@@@    Expand__shrl(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of shrul based on validated and scheduled basic assembly source.
*/
static void
Expand__shrul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c32 = Gen_Literal_TN(32, 4) ;
  Build_OP (	TOP_cmpgeu_i_b,	b0_0_0,	i1,	c32,	ops) ;
  Build_OP (	TOP_sub_i,	r0_19_0,	c32,	i1,	ops) ;
  Build_OP (	TOP_shru_r,	r0_16_0,	il0,	i1,	ops) ;
  Build_OP (	TOP_shru_r,	r0_20_0,	ih0,	i1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_21_1,	ih0,	r0_19_0,	ops) ;
  Build_OP (	TOP_sub_r,	r0_23_1,	Zero_TN,	r0_19_0,	ops) ;
  Build_OP (	TOP_or_r,	r0_16_2,	r0_16_0,	r0_21_1,	ops) ;
  Build_OP (	TOP_shru_r,	r0_19_2,	ih0,	r0_23_1,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_0_0,	r0_19_2,	r0_16_2,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_0,	Zero_TN,	r0_20_0,	ops) ;
} /* Expand__shrul */

/*
@@@  case INTRN_SHRUL:
@@@    Expand__shrul(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of subl based on validated and scheduled basic assembly source.
*/
static void
Expand__subl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_19_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_b,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_addcg,	r0_19_1,	b0_0_1,	ih1,	Zero_TN,	b0_0_0,	ops) ;
  Build_OP (	TOP_sub_r,	ol0,	il0,	il1,	ops) ;
  Build_OP (	TOP_sub_r,	oh0,	ih0,	r0_19_1,	ops) ;
} /* Expand__subl */

/*
@@@  case INTRN_SUBL:
@@@    Expand__subl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of subul based on validated and scheduled basic assembly source.
*/
static void
Expand__subul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_19_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_b,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_addcg,	r0_19_1,	b0_0_1,	ih1,	Zero_TN,	b0_0_0,	ops) ;
  Build_OP (	TOP_sub_r,	ol0,	il0,	il1,	ops) ;
  Build_OP (	TOP_sub_r,	oh0,	ih0,	r0_19_1,	ops) ;
} /* Expand__subul */

/*
@@@  case INTRN_SUBUL:
@@@    Expand__subul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

void
Exp_Intrinsic_Op (
  INTRINSIC id,
  INT num_results,
  INT num_opnds,
  TN *result[],
  TN *opnd[],
  OPS *ops
)
{
  switch (id) {
    case INTRN_ABSL:
      Expand__absl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ADDL:
      Expand__addl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_ADDUL:
      Expand__addul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_EQL:
      Expand__eql(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_EQUL:
      Expand__equl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_GEL:
      Expand__gel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_GEUL:
      Expand__geul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_GTL:
      Expand__gtl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_GTUL:
      Expand__gtul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_LEL:
      Expand__lel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_LEUL:
      Expand__leul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_LTL:
      Expand__ltl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_LTUL:
      Expand__ltul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MAXL:
      Expand__maxl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MAXUL:
      Expand__maxul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MINL:
      Expand__minl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MINUL:
      Expand__minul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MULL:
      Expand__mull(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MULUL:
      Expand__mulul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MULUW:
      Expand__muluw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULW:
      Expand__mulw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEGL:
      Expand__negl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEGUL:
      Expand__negul(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEL:
      Expand__nel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_NEUL:
      Expand__neul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_SHLL:
      Expand__shll(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_SHRL:
      Expand__shrl(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_SHRUL:
      Expand__shrul(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_SUBL:
      Expand__subl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_SUBUL:
      Expand__subul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    default:
      FmtAssert (FALSE, ("Exp_Intrinsic_Op: unknown intrinsic op %s", INTRN_c_name(id)));
  } /* switch*/
}
