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
 * Expansion of absch based on validated and scheduled basic assembly source.
*/
static void
Expand__absch(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x7fff = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpeq_i_b,	b0_0_0,	i0,	c0xffff8000,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_1,	i0,	c0,	ops) ;
  Build_OP (	TOP_slctf_i,	r0_20_1,	b0_0_0,	r0_20_0,	c0x7fff,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_1,	r0_20_1,	i0,	ops) ;
} /* Expand__absch */

/*
@@@  case INTRN_ABSCH:
@@@    Expand__absch(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of abscl based on validated and scheduled basic assembly source.
*/
static void
Expand__abscl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpgtu_i_b,	b0_0_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_20_0,	c0,	il0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_21_0,	c0,	ih0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_22_0,	c_1,	ih0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_1,	b0_0_0,	r0_22_0,	r0_21_0,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_22_1,	ih0,	c0x80000000,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_23_1,	il0,	c0,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_2,	r0_22_1,	r0_23_1,	ops) ;
  Build_OP (	TOP_mov_i,	r0_22_2,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_2,	ih0,	c0,	ops) ;
  Build_OP (	TOP_slctf_i,	r0_20_3,	b0_0_2,	r0_20_0,	c_1,	ops) ;
  Build_OP (	TOP_slctf_r,	r0_21_3,	b0_0_2,	r0_21_1,	r0_22_2,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_1_2,	r0_20_3,	il0,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_1_2,	r0_21_3,	ih0,	ops) ;
} /* Expand__abscl */

/*
@@@  case INTRN_ABSCL:
@@@    Expand__abscl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of abscw based on validated and scheduled basic assembly source.
*/
static void
Expand__abscw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpeq_i_b,	b0_0_0,	i0,	c0x80000000,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
  Build_OP (	TOP_slctf_i,	r0_20_1,	b0_0_0,	r0_20_0,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_1_0,	r0_20_1,	i0,	ops) ;
} /* Expand__abscw */

/*
@@@  case INTRN_ABSCW:
@@@    Expand__abscw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of absh based on validated and scheduled basic assembly source.
*/
static void
Expand__absh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_20_0,	i0,	ops) ;
} /* Expand__absh */

/*
@@@  case INTRN_ABSH:
@@@    Expand__absh(result[0],opnd[0],ops) ;
@@@  break ;
*/

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
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
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
 * Expansion of absw based on validated and scheduled basic assembly source.
*/
static void
Expand__absw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_20_0,	i0,	ops) ;
} /* Expand__absw */

/*
@@@  case INTRN_ABSW:
@@@    Expand__absw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of addch based on validated and scheduled basic assembly source.
*/
static void
Expand__addch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fff = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_r,	r0_16_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_max_i,	r0_16_1,	r0_16_0,	c0xffff8000,	ops) ;
  Build_OP (	TOP_min_i,	o0,	r0_16_1,	c0x7fff,	ops) ;
} /* Expand__addch */

/*
@@@  case INTRN_ADDCH:
@@@    Expand__addch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of addcl based on validated and scheduled basic assembly source.
*/
static void
Expand__addcl(
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
  TN *b0_0_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_xor_r,	r0_20_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_mtb,	b0_0_0,	Zero_TN,	ops) ;
  Build_OP (	TOP_mov_i,	r0_21_0,	c0x80000000,	ops) ;
  Build_OP (	TOP_addcg,	r0_22_1,	b0_0_1,	il0,	il1,	b0_0_0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_20_1,	r0_20_0,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_1,	ih0,	c0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_23_1,	r0_21_0,	c0,	ops) ;
  Build_OP (	TOP_addcg,	r0_24_2,	b0_0_2,	ih0,	ih1,	b0_0_1,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_2,	b0_1_1,	r0_21_0,	r0_23_1,	ops) ;
  Build_OP (	TOP_shru_i,	r0_20_2,	r0_20_1,	c31,	ops) ;
  Build_OP (	TOP_slct_i,	r0_23_3,	b0_1_1,	Zero_TN,	c_1,	ops) ;
  Build_OP (	TOP_xor_r,	r0_25_3,	r0_24_2,	ih0,	ops) ;
  Build_OP (	TOP_shru_i,	r0_25_4,	r0_25_3,	c31,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_5,	r0_20_2,	r0_25_4,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_0_5,	r0_23_3,	r0_22_1,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_5,	r0_21_2,	r0_24_2,	ops) ;
} /* Expand__addcl */

/*
@@@  case INTRN_ADDCL:
@@@    Expand__addcl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of addcw based on validated and scheduled basic assembly source.
*/
static void
Expand__addcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_xor_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_mov_i,	r0_21_0,	c0x80000000,	ops) ;
  Build_OP (	TOP_add_r,	r0_22_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_orc_i,	r0_20_1,	r0_20_0,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_1,	i0,	c0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_23_1,	r0_21_0,	c0,	ops) ;
  Build_OP (	TOP_xor_r,	r0_24_1,	r0_22_0,	i0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_2,	b0_1_1,	r0_21_0,	r0_23_1,	ops) ;
  Build_OP (	TOP_and_r,	r0_20_2,	r0_20_1,	r0_21_0,	ops) ;
  Build_OP (	TOP_and_r,	r0_24_2,	r0_24_1,	r0_21_0,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_3,	r0_20_2,	r0_24_2,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_3,	r0_21_2,	r0_22_0,	ops) ;
} /* Expand__addcw */

/*
@@@  case INTRN_ADDCW:
@@@    Expand__addcw(result[0],opnd[0],opnd[1],ops) ;
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
 * Expansion of bitclrh based on validated and scheduled basic assembly source.
*/
static void
Expand__bitclrh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_andc_r,	o0,	r0_20_1,	i0,	ops) ;
} /* Expand__bitclrh */

/*
@@@  case INTRN_BITCLRH:
@@@    Expand__bitclrh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitclrw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitclrw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_andc_r,	o0,	r0_20_1,	i0,	ops) ;
} /* Expand__bitclrw */

/*
@@@  case INTRN_BITCLRW:
@@@    Expand__bitclrw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitcnth based on validated and scheduled basic assembly source.
*/
static void
Expand__bitcnth(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x0f0f = Gen_Literal_TN(__EXTS32TOS64(0x0f0f), 4) ;
  TN *c0x1f = Gen_Literal_TN(__EXTS32TOS64(0x1f), 4) ;
  TN *c0x3333 = Gen_Literal_TN(__EXTS32TOS64(0x3333), 4) ;
  TN *c0x5555 = Gen_Literal_TN(__EXTS32TOS64(0x5555), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c2 = Gen_Literal_TN(__EXTS32TOS64(2), 4) ;
  TN *c4 = Gen_Literal_TN(__EXTS32TOS64(4), 4) ;
  TN *c8 = Gen_Literal_TN(__EXTS32TOS64(8), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_1,	r0_20_0,	c0x5555,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_2,	i0,	r0_20_1,	ops) ;
  Build_OP (	TOP_shru_i,	r0_21_3,	r0_20_2,	c2,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_3,	r0_20_2,	c0x3333,	ops) ;
  Build_OP (	TOP_and_i,	r0_21_4,	r0_21_3,	c0x3333,	ops) ;
  Build_OP (	TOP_add_r,	r0_20_5,	r0_20_3,	r0_21_4,	ops) ;
  Build_OP (	TOP_shru_i,	r0_21_6,	r0_20_5,	c4,	ops) ;
  Build_OP (	TOP_add_r,	r0_20_7,	r0_20_5,	r0_21_6,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_8,	r0_20_7,	c0x0f0f,	ops) ;
  Build_OP (	TOP_shru_i,	r0_21_9,	r0_20_8,	c8,	ops) ;
  Build_OP (	TOP_add_r,	r0_20_10,	r0_20_8,	r0_21_9,	ops) ;
  Build_OP (	TOP_and_i,	o0,	r0_20_10,	c0x1f,	ops) ;
} /* Expand__bitcnth */

/*
@@@  case INTRN_BITCNTH:
@@@    Expand__bitcnth(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitcntw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitcntw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x0f0f0f0f = Gen_Literal_TN(__EXTS32TOS64(0x0f0f0f0f), 4) ;
  TN *c0x33333333 = Gen_Literal_TN(__EXTS32TOS64(0x33333333), 4) ;
  TN *c0x3f = Gen_Literal_TN(__EXTS32TOS64(0x3f), 4) ;
  TN *c0x55555555 = Gen_Literal_TN(__EXTS32TOS64(0x55555555), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
  TN *c2 = Gen_Literal_TN(__EXTS32TOS64(2), 4) ;
  TN *c4 = Gen_Literal_TN(__EXTS32TOS64(4), 4) ;
  TN *c8 = Gen_Literal_TN(__EXTS32TOS64(8), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_1,	r0_20_0,	c0x55555555,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_2,	i0,	r0_20_1,	ops) ;
  Build_OP (	TOP_shru_i,	r0_21_3,	r0_20_2,	c2,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_3,	r0_20_2,	c0x33333333,	ops) ;
  Build_OP (	TOP_and_i,	r0_21_4,	r0_21_3,	c0x33333333,	ops) ;
  Build_OP (	TOP_add_r,	r0_20_5,	r0_20_3,	r0_21_4,	ops) ;
  Build_OP (	TOP_shru_i,	r0_21_6,	r0_20_5,	c4,	ops) ;
  Build_OP (	TOP_add_r,	r0_20_7,	r0_20_5,	r0_21_6,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_8,	r0_20_7,	c0x0f0f0f0f,	ops) ;
  Build_OP (	TOP_shru_i,	r0_21_9,	r0_20_8,	c8,	ops) ;
  Build_OP (	TOP_add_r,	r0_20_10,	r0_20_8,	r0_21_9,	ops) ;
  Build_OP (	TOP_shru_i,	r0_21_11,	r0_20_10,	c16,	ops) ;
  Build_OP (	TOP_add_r,	r0_20_12,	r0_20_10,	r0_21_11,	ops) ;
  Build_OP (	TOP_and_i,	o0,	r0_20_12,	c0x3f,	ops) ;
} /* Expand__bitcntw */

/*
@@@  case INTRN_BITCNTW:
@@@    Expand__bitcntw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitnoth based on validated and scheduled basic assembly source.
*/
static void
Expand__bitnoth(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_xor_r,	o0,	i0,	r0_20_1,	ops) ;
} /* Expand__bitnoth */

/*
@@@  case INTRN_BITNOTH:
@@@    Expand__bitnoth(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitnotw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitnotw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_xor_r,	o0,	i0,	r0_20_1,	ops) ;
} /* Expand__bitnotw */

/*
@@@  case INTRN_BITNOTW:
@@@    Expand__bitnotw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitrevw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitrevw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x00FF00FF = Gen_Literal_TN(__EXTS32TOS64(0x00FF00FF), 4) ;
  TN *c0x0F0F0F0F = Gen_Literal_TN(__EXTS32TOS64(0x0F0F0F0F), 4) ;
  TN *c0x33333333 = Gen_Literal_TN(__EXTS32TOS64(0x33333333), 4) ;
  TN *c0x55555555 = Gen_Literal_TN(__EXTS32TOS64(0x55555555), 4) ;
  TN *c0xAAAAAAAA = Gen_Literal_TN(__EXTS32TOS64(0xAAAAAAAA), 4) ;
  TN *c0xCCCCCCCC = Gen_Literal_TN(__EXTS32TOS64(0xCCCCCCCC), 4) ;
  TN *c0xF0F0F0F0 = Gen_Literal_TN(__EXTS32TOS64(0xF0F0F0F0), 4) ;
  TN *c0xFF00FF00 = Gen_Literal_TN(__EXTS32TOS64(0xFF00FF00), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
  TN *c2 = Gen_Literal_TN(__EXTS32TOS64(2), 4) ;
  TN *c4 = Gen_Literal_TN(__EXTS32TOS64(4), 4) ;
  TN *c8 = Gen_Literal_TN(__EXTS32TOS64(8), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_and_i,	r0_20_0,	i1,	c0xAAAAAAAA,	ops) ;
  Build_OP (	TOP_and_i,	r0_21_0,	i1,	c0x55555555,	ops) ;
  Build_OP (	TOP_shru_i,	r0_20_1,	r0_20_0,	c1,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_1,	r0_21_0,	c1,	ops) ;
  Build_OP (	TOP_or_r,	r0_20_2,	r0_20_1,	r0_21_1,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_3,	r0_20_2,	c0xCCCCCCCC,	ops) ;
  Build_OP (	TOP_and_i,	r0_21_3,	r0_20_2,	c0x33333333,	ops) ;
  Build_OP (	TOP_shru_i,	r0_20_4,	r0_20_3,	c2,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_4,	r0_21_3,	c2,	ops) ;
  Build_OP (	TOP_or_r,	r0_20_5,	r0_20_4,	r0_21_4,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_6,	r0_20_5,	c0xF0F0F0F0,	ops) ;
  Build_OP (	TOP_and_i,	r0_21_6,	r0_20_5,	c0x0F0F0F0F,	ops) ;
  Build_OP (	TOP_shru_i,	r0_20_7,	r0_20_6,	c4,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_7,	r0_21_6,	c4,	ops) ;
  Build_OP (	TOP_or_r,	r0_20_8,	r0_20_7,	r0_21_7,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_9,	r0_20_8,	c0xFF00FF00,	ops) ;
  Build_OP (	TOP_and_i,	r0_21_9,	r0_20_8,	c0x00FF00FF,	ops) ;
  Build_OP (	TOP_shru_i,	r0_20_10,	r0_20_9,	c8,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_10,	r0_21_9,	c8,	ops) ;
  Build_OP (	TOP_or_r,	r0_20_11,	r0_20_10,	r0_21_10,	ops) ;
  Build_OP (	TOP_shru_i,	r0_20_12,	r0_20_11,	c16,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_12,	r0_20_11,	c16,	ops) ;
  Build_OP (	TOP_or_r,	r0_20_13,	r0_20_12,	r0_21_12,	ops) ;
  Build_OP (	TOP_shru_r,	o0,	r0_20_13,	i0,	ops) ;
} /* Expand__bitrevw */

/*
@@@  case INTRN_BITREVW:
@@@    Expand__bitrevw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitseth based on validated and scheduled basic assembly source.
*/
static void
Expand__bitseth(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_or_r,	o0,	i0,	r0_20_1,	ops) ;
} /* Expand__bitseth */

/*
@@@  case INTRN_BITSETH:
@@@    Expand__bitseth(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitsetw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitsetw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_or_r,	o0,	i0,	r0_20_1,	ops) ;
} /* Expand__bitsetw */

/*
@@@  case INTRN_BITSETW:
@@@    Expand__bitsetw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitvalh based on validated and scheduled basic assembly source.
*/
static void
Expand__bitvalh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_and_i,	o0,	r0_20_0,	c1,	ops) ;
} /* Expand__bitvalh */

/*
@@@  case INTRN_BITVALH:
@@@    Expand__bitvalh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitvalw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitvalw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_and_i,	o0,	r0_20_0,	c1,	ops) ;
} /* Expand__bitvalw */

/*
@@@  case INTRN_BITVALW:
@@@    Expand__bitvalw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of clamplw based on validated and scheduled basic assembly source.
*/
static void
Expand__clamplw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	ih0,	c0,	ops) ;
  Build_OP (	TOP_add_i,	r0_20_0,	Zero_TN,	c0x80000000,	ops) ;
  Build_OP (	TOP_shr_i,	r0_21_0,	il0,	c31,	ops) ;
  Build_OP (	TOP_orc_i,	r0_22_1,	r0_20_0,	c0,	ops) ;
  Build_OP (	TOP_cmpne_r_b,	b0_1_1,	r0_21_0,	ih0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_20_2,	b0_0_0,	r0_20_0,	r0_22_1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_1_1,	r0_20_2,	il0,	ops) ;
} /* Expand__clamplw */

/*
@@@  case INTRN_CLAMPLW:
@@@    Expand__clamplw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of clampwh based on validated and scheduled basic assembly source.
*/
static void
Expand__clampwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x7fff = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_add_i,	r0_20_0,	Zero_TN,	c0xffff8000,	ops) ;
  Build_OP (	TOP_orc_i,	r0_22_1,	r0_20_0,	c0,	ops) ;
  Build_OP (	TOP_cmpgt_i_r,	r0_21_1,	i0,	c0x7fff,	ops) ;
  Build_OP (	TOP_cmplt_r_r,	r0_23_1,	i0,	r0_20_0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_20_2,	b0_0_0,	r0_20_0,	r0_22_1,	ops) ;
  Build_OP (	TOP_orl_r_b,	b0_1_2,	r0_21_1,	r0_23_1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_1_2,	r0_20_2,	i0,	ops) ;
} /* Expand__clampwh */

/*
@@@  case INTRN_CLAMPWH:
@@@    Expand__clampwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of disth based on validated and scheduled basic assembly source.
*/
static void
Expand__disth(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmple_r_b,	b0_0_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r,	r0_21_0,	i1,	i0,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_21_0,	r0_20_0,	ops) ;
} /* Expand__disth */

/*
@@@  case INTRN_DISTH:
@@@    Expand__disth(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of distuh based on validated and scheduled basic assembly source.
*/
static void
Expand__distuh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpleu_r_b,	b0_0_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r,	r0_21_0,	i1,	i0,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_21_0,	r0_20_0,	ops) ;
} /* Expand__distuh */

/*
@@@  case INTRN_DISTUH:
@@@    Expand__distuh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of distuw based on validated and scheduled basic assembly source.
*/
static void
Expand__distuw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpleu_r_b,	b0_0_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r,	r0_21_0,	i1,	i0,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_21_0,	r0_20_0,	ops) ;
} /* Expand__distuw */

/*
@@@  case INTRN_DISTUW:
@@@    Expand__distuw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of distw based on validated and scheduled basic assembly source.
*/
static void
Expand__distw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmple_r_b,	b0_0_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r,	r0_21_0,	i1,	i0,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_21_0,	r0_20_0,	ops) ;
} /* Expand__distw */

/*
@@@  case INTRN_DISTW:
@@@    Expand__distw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of edgesh based on validated and scheduled basic assembly source.
*/
static void
Expand__edgesh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fff = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
  TN *c0xffff = Gen_Literal_TN(__EXTS32TOS64(0xffff), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i,	r0_20_0,	i1,	c1,	ops) ;
  Build_OP (	TOP_xor_r,	r0_20_1,	i1,	r0_20_0,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_2,	r0_20_1,	c0x7fff,	ops) ;
  Build_OP (	TOP_and_i,	r0_21_2,	i0,	c0xffff,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_21_2,	r0_20_2,	ops) ;
} /* Expand__edgesh */

/*
@@@  case INTRN_EDGESH:
@@@    Expand__edgesh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of edgesw based on validated and scheduled basic assembly source.
*/
static void
Expand__edgesw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i,	r0_20_0,	i1,	c1,	ops) ;
  Build_OP (	TOP_xor_r,	r0_20_1,	i1,	r0_20_0,	ops) ;
  Build_OP (	TOP_and_i,	r0_20_2,	r0_20_1,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_or_r,	o0,	i0,	r0_20_2,	ops) ;
} /* Expand__edgesw */

/*
@@@  case INTRN_EDGESW:
@@@    Expand__edgesw(result[0],opnd[0],opnd[1],ops) ;
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
 * Expansion of gethh based on validated and scheduled basic assembly source.
*/
static void
Expand__gethh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shr_i,	o0,	i0,	c16,	ops) ;
} /* Expand__gethh */

/*
@@@  case INTRN_GETHH:
@@@    Expand__gethh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of gethw based on validated and scheduled basic assembly source.
*/
static void
Expand__gethw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	o0,	ih0,	c0,	ops) ;
} /* Expand__gethw */

/*
@@@  case INTRN_GETHW:
@@@    Expand__gethw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of getlh based on validated and scheduled basic assembly source.
*/
static void
Expand__getlh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  Build_OP (	TOP_sxth_r,	o0,	i0,	ops) ;
} /* Expand__getlh */

/*
@@@  case INTRN_GETLH:
@@@    Expand__getlh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of getlw based on validated and scheduled basic assembly source.
*/
static void
Expand__getlw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
} /* Expand__getlw */

/*
@@@  case INTRN_GETLW:
@@@    Expand__getlw(result[0],opnd[0],opnd[1],ops) ;
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
 * Expansion of insequw based on validated and scheduled basic assembly source.
*/
static void
Expand__insequw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insequw */

/*
@@@  case INTRN_INSEQUW:
@@@    Expand__insequw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of inseqw based on validated and scheduled basic assembly source.
*/
static void
Expand__inseqw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__inseqw */

/*
@@@  case INTRN_INSEQW:
@@@    Expand__inseqw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insgeuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insgeuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpgeu_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insgeuw */

/*
@@@  case INTRN_INSGEUW:
@@@    Expand__insgeuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insgew based on validated and scheduled basic assembly source.
*/
static void
Expand__insgew(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpge_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insgew */

/*
@@@  case INTRN_INSGEW:
@@@    Expand__insgew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insgtuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insgtuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpgtu_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insgtuw */

/*
@@@  case INTRN_INSGTUW:
@@@    Expand__insgtuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insgtw based on validated and scheduled basic assembly source.
*/
static void
Expand__insgtw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpgt_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insgtw */

/*
@@@  case INTRN_INSGTW:
@@@    Expand__insgtw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insleuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insleuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpleu_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insleuw */

/*
@@@  case INTRN_INSLEUW:
@@@    Expand__insleuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of inslew based on validated and scheduled basic assembly source.
*/
static void
Expand__inslew(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmple_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__inslew */

/*
@@@  case INTRN_INSLEW:
@@@    Expand__inslew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insltuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insltuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpltu_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insltuw */

/*
@@@  case INTRN_INSLTUW:
@@@    Expand__insltuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insltw based on validated and scheduled basic assembly source.
*/
static void
Expand__insltw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmplt_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insltw */

/*
@@@  case INTRN_INSLTW:
@@@    Expand__insltw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insneuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insneuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpne_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insneuw */

/*
@@@  case INTRN_INSNEUW:
@@@    Expand__insneuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insnew based on validated and scheduled basic assembly source.
*/
static void
Expand__insnew(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpne_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insnew */

/*
@@@  case INTRN_INSNEW:
@@@    Expand__insnew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
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
 * Expansion of lzcnth based on validated and scheduled basic assembly source.
*/
static void
Expand__lzcnth(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c_16 = Gen_Literal_TN(__EXTS32TOS64(-16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_clz,	r0_16_0,	i0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_add_i,	r0_16_1,	r0_16_0,	c_16,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	Zero_TN,	r0_16_1,	ops) ;
} /* Expand__lzcnth */

/*
@@@  case INTRN_LZCNTH:
@@@    Expand__lzcnth(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of lzcntl based on validated and scheduled basic assembly source.
*/
static void
Expand__lzcntl(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_clz,	r0_20_0,	ih0,	ops) ;
  Build_OP (	TOP_clz,	r0_21_0,	il0,	ops) ;
  Build_OP (	TOP_cmpeq_i_b,	b0_0_0,	ih0,	c0,	ops) ;
  Build_OP (	TOP_add_i,	r0_21_1,	r0_21_0,	c32,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_21_1,	r0_20_0,	ops) ;
} /* Expand__lzcntl */

/*
@@@  case INTRN_LZCNTL:
@@@    Expand__lzcntl(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of lzcntw based on validated and scheduled basic assembly source.
*/
static void
Expand__lzcntw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  Build_OP (	TOP_clz,	o0,	i0,	ops) ;
} /* Expand__lzcntw */

/*
@@@  case INTRN_LZCNTW:
@@@    Expand__lzcntw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of mafcw based on validated and scheduled basic assembly source.
*/
static void
Expand__mafcw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *b0_0_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_2_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mulll_r,	r0_26_2,	i1,	i2,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_27_0,	i1,	c0xffff8000,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_28_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_2_3,	r0_27_0,	r0_28_0,	ops) ;
  Build_OP (	TOP_shl_i,	r0_26_3,	r0_26_2,	c1,	ops) ;
  Build_OP (	TOP_slctf_i,	r0_26_4,	b0_2_3,	r0_26_3,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_xor_r,	r0_20_5,	i0,	r0_26_4,	ops) ;
  Build_OP (	TOP_mov_i,	r0_21_5,	c0x80000000,	ops) ;
  Build_OP (	TOP_add_r,	r0_22_5,	i0,	r0_26_4,	ops) ;
  Build_OP (	TOP_orc_i,	r0_20_6,	r0_20_5,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_6,	i0,	c0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_23_6,	r0_21_5,	c0,	ops) ;
  Build_OP (	TOP_xor_r,	r0_24_6,	r0_22_5,	i0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_7,	b0_1_6,	r0_21_5,	r0_23_6,	ops) ;
  Build_OP (	TOP_and_r,	r0_20_7,	r0_20_6,	r0_21_5,	ops) ;
  Build_OP (	TOP_and_r,	r0_24_7,	r0_24_6,	r0_21_5,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_8,	r0_20_7,	r0_24_7,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_8,	r0_21_7,	r0_22_5,	ops) ;
} /* Expand__mafcw */

/*
@@@  case INTRN_MAFCW:
@@@    Expand__mafcw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of maxh based on validated and scheduled basic assembly source.
*/
static void
Expand__maxh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_max_r,	o0,	i0,	i1,	ops) ;
} /* Expand__maxh */

/*
@@@  case INTRN_MAXH:
@@@    Expand__maxh(result[0],opnd[0],opnd[1],ops) ;
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
 * Expansion of maxuh based on validated and scheduled basic assembly source.
*/
static void
Expand__maxuh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_maxu_r,	o0,	i0,	i1,	ops) ;
} /* Expand__maxuh */

/*
@@@  case INTRN_MAXUH:
@@@    Expand__maxuh(result[0],opnd[0],opnd[1],ops) ;
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
 * Expansion of maxuw based on validated and scheduled basic assembly source.
*/
static void
Expand__maxuw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_maxu_r,	o0,	i0,	i1,	ops) ;
} /* Expand__maxuw */

/*
@@@  case INTRN_MAXUW:
@@@    Expand__maxuw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of maxw based on validated and scheduled basic assembly source.
*/
static void
Expand__maxw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_max_r,	o0,	i0,	i1,	ops) ;
} /* Expand__maxw */

/*
@@@  case INTRN_MAXW:
@@@    Expand__maxw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of minh based on validated and scheduled basic assembly source.
*/
static void
Expand__minh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_min_r,	o0,	i0,	i1,	ops) ;
} /* Expand__minh */

/*
@@@  case INTRN_MINH:
@@@    Expand__minh(result[0],opnd[0],opnd[1],ops) ;
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
 * Expansion of minuh based on validated and scheduled basic assembly source.
*/
static void
Expand__minuh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_minu_r,	o0,	i0,	i1,	ops) ;
} /* Expand__minuh */

/*
@@@  case INTRN_MINUH:
@@@    Expand__minuh(result[0],opnd[0],opnd[1],ops) ;
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
 * Expansion of minuw based on validated and scheduled basic assembly source.
*/
static void
Expand__minuw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_minu_r,	o0,	i0,	i1,	ops) ;
} /* Expand__minuw */

/*
@@@  case INTRN_MINUW:
@@@    Expand__minuw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of minw based on validated and scheduled basic assembly source.
*/
static void
Expand__minw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_min_r,	o0,	i0,	i1,	ops) ;
} /* Expand__minw */

/*
@@@  case INTRN_MINW:
@@@    Expand__minw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpfcw based on validated and scheduled basic assembly source.
*/
static void
Expand__mpfcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mulll_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_20_0,	i0,	c0xffff8000,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_21_1,	i0,	i1,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_3,	r0_20_0,	r0_21_1,	ops) ;
  Build_OP (	TOP_shl_i,	r0_16_3,	r0_26_2,	c1,	ops) ;
  Build_OP (	TOP_slctf_i,	o0,	b0_0_3,	r0_16_3,	c0x7fffffff,	ops) ;
} /* Expand__mpfcw */

/*
@@@  case INTRN_MPFCW:
@@@    Expand__mpfcw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpfcwl based on validated and scheduled basic assembly source.
*/
static void
Expand__mpfcwl(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mullu_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_27_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_24_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_25_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_21_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_20_2,	i0,	c0x80000000,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_3,	r0_20_2,	r0_21_2,	ops) ;
  Build_OP (	TOP_addcg,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_addcg,	r0_21_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
  Build_OP (	TOP_shru_i,	r0_17_4,	r0_16_3,	c31,	ops) ;
  Build_OP (	TOP_shl_i,	r0_16_4,	r0_16_3,	c1,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_5,	r0_21_4,	c1,	ops) ;
  Build_OP (	TOP_add_i,	r0_20_5,	Zero_TN,	c_1,	ops) ;
  Build_OP (	TOP_or_r,	r0_17_6,	r0_17_4,	r0_21_5,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_0_3,	r0_20_5,	r0_16_4,	ops) ;
  Build_OP (	TOP_slctf_i,	oh0,	b0_0_3,	r0_17_6,	c0x7fffffff,	ops) ;
} /* Expand__mpfcwl */

/*
@@@  case INTRN_MPFCWL:
@@@    Expand__mpfcwl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpfml based on validated and scheduled basic assembly source.
*/
static void
Expand__mpfml(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c15 = Gen_Literal_TN(__EXTS32TOS64(15), 4) ;
  TN *c17 = Gen_Literal_TN(__EXTS32TOS64(17), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mullu_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_27_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_24_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_25_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_addcg,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_shl_i,	ol0,	r0_16_3,	c17,	ops) ;
  Build_OP (	TOP_shru_i,	r0_20_4,	r0_16_3,	c15,	ops) ;
  Build_OP (	TOP_addcg,	r0_17_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_5,	r0_17_4,	c17,	ops) ;
  Build_OP (	TOP_or_r,	oh0,	r0_20_4,	r0_21_5,	ops) ;
} /* Expand__mpfml */

/*
@@@  case INTRN_MPFML:
@@@    Expand__mpfml(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpfrch based on validated and scheduled basic assembly source.
*/
static void
Expand__mpfrch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x4000 = Gen_Literal_TN(__EXTS32TOS64(0x4000), 4) ;
  TN *c0x7fff = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
  TN *c15 = Gen_Literal_TN(__EXTS32TOS64(15), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mulll_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_20_0,	i0,	c0xffff8000,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_21_1,	i0,	i1,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_3,	r0_20_0,	r0_21_1,	ops) ;
  Build_OP (	TOP_add_i,	r0_21_3,	r0_26_2,	c0x4000,	ops) ;
  Build_OP (	TOP_shr_i,	r0_16_4,	r0_21_3,	c15,	ops) ;
  Build_OP (	TOP_slctf_i,	o0,	b0_0_3,	r0_16_4,	c0x7fff,	ops) ;
} /* Expand__mpfrch */

/*
@@@  case INTRN_MPFRCH:
@@@    Expand__mpfrch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpml based on validated and scheduled basic assembly source.
*/
static void
Expand__mpml(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_mullu_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_27_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_24_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_25_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_addcg,	ol0,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_addcg,	oh0,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
} /* Expand__mpml */

/*
@@@  case INTRN_MPML:
@@@    Expand__mpml(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpuml based on validated and scheduled basic assembly source.
*/
static void
Expand__mpuml(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mullu_r,	ol0,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_20_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_slct_i,	r0_28_1,	b0_0_0,	i1,	c0,	ops) ;
  Build_OP (	TOP_add_r,	oh0,	r0_20_2,	r0_28_1,	ops) ;
} /* Expand__mpuml */

/*
@@@  case INTRN_MPUML:
@@@    Expand__mpuml(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulfch based on validated and scheduled basic assembly source.
*/
static void
Expand__mulfch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fff = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
  TN *c15 = Gen_Literal_TN(__EXTS32TOS64(15), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mulll_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_20_0,	i0,	c0xffff8000,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_21_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_3,	r0_20_0,	r0_21_0,	ops) ;
  Build_OP (	TOP_shr_i,	r0_16_3,	r0_26_2,	c15,	ops) ;
  Build_OP (	TOP_slctf_i,	o0,	b0_0_3,	r0_16_3,	c0x7fff,	ops) ;
} /* Expand__mulfch */

/*
@@@  case INTRN_MULFCH:
@@@    Expand__mulfch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulfcm based on validated and scheduled basic assembly source.
*/
static void
Expand__mulfcm(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	r0_17_0,	i1,	c16,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_22_3,	i0,	r0_17_0,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_23_3,	i0,	r0_17_0,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_20_2,	i0,	c0x80000000,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_21_2,	i0,	r0_17_0,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_3,	r0_20_2,	r0_21_2,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_4,	r0_23_3,	c1,	ops) ;
  Build_OP (	TOP_shru_i,	r0_16_4,	r0_22_3,	c31,	ops) ;
  Build_OP (	TOP_or_r,	r0_16_5,	r0_16_4,	r0_21_4,	ops) ;
  Build_OP (	TOP_slctf_i,	o0,	b0_0_3,	r0_16_5,	c0x7fffffff,	ops) ;
} /* Expand__mulfcm */

/*
@@@  case INTRN_MULFCM:
@@@    Expand__mulfcm(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulfcw based on validated and scheduled basic assembly source.
*/
static void
Expand__mulfcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mullu_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_27_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_24_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_25_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_20_2,	i0,	c0x80000000,	ops) ;
  Build_OP (	TOP_cmpeq_r_r,	r0_21_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_3,	r0_20_2,	r0_21_2,	ops) ;
  Build_OP (	TOP_addcg,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_addcg,	r0_21_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
  Build_OP (	TOP_shru_i,	r0_16_4,	r0_16_3,	c31,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_5,	r0_21_4,	c1,	ops) ;
  Build_OP (	TOP_or_r,	r0_16_6,	r0_16_4,	r0_21_5,	ops) ;
  Build_OP (	TOP_slctf_i,	o0,	b0_0_3,	r0_16_6,	c0x7fffffff,	ops) ;
} /* Expand__mulfcw */

/*
@@@  case INTRN_MULFCW:
@@@    Expand__mulfcw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulh based on validated and scheduled basic assembly source.
*/
static void
Expand__mulh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_mulll_r,	o0,	i0,	i1,	ops) ;
} /* Expand__mulh */

/*
@@@  case INTRN_MULH:
@@@    Expand__mulh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulhh based on validated and scheduled basic assembly source.
*/
static void
Expand__mulhh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mulll_r,	r0_16_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_shr_i,	o0,	r0_16_2,	c16,	ops) ;
} /* Expand__mulhh */

/*
@@@  case INTRN_MULHH:
@@@    Expand__mulhh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulhuh based on validated and scheduled basic assembly source.
*/
static void
Expand__mulhuh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mulllu_r,	r0_16_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_shr_i,	o0,	r0_16_2,	c16,	ops) ;
} /* Expand__mulhuh */

/*
@@@  case INTRN_MULHUH:
@@@    Expand__mulhuh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulhuw based on validated and scheduled basic assembly source.
*/
static void
Expand__mulhuw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_29_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mullu_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_27_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_0,	i1,	c0,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_24_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_25_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_slct_i,	r0_28_1,	b0_1_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_slct_i,	r0_29_1,	b0_0_0,	i1,	c0,	ops) ;
  Build_OP (	TOP_add_r,	r0_28_2,	r0_28_1,	r0_29_1,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_addcg,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_addcg,	r0_17_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
  Build_OP (	TOP_add_r,	o0,	r0_17_4,	r0_28_2,	ops) ;
} /* Expand__mulhuw */

/*
@@@  case INTRN_MULHUW:
@@@    Expand__mulhuw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulhw based on validated and scheduled basic assembly source.
*/
static void
Expand__mulhw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_mullu_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_27_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_24_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_25_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_addcg,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_addcg,	o0,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
} /* Expand__mulhw */

/*
@@@  case INTRN_MULHW:
@@@    Expand__mulhw(result[0],opnd[0],opnd[1],ops) ;
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
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
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
 * Expansion of mulm based on validated and scheduled basic assembly source.
*/
static void
Expand__mulm(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_mull_r,	o0,	i0,	i1,	ops) ;
} /* Expand__mulm */

/*
@@@  case INTRN_MULM:
@@@    Expand__mulm(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of muln based on validated and scheduled basic assembly source.
*/
static void
Expand__muln(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_mullu_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_27_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_24_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_25_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_addcg,	ol0,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_addcg,	oh0,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
} /* Expand__muln */

/*
@@@  case INTRN_MULN:
@@@    Expand__muln(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of muluh based on validated and scheduled basic assembly source.
*/
static void
Expand__muluh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_mulllu_r,	o0,	i0,	i1,	ops) ;
} /* Expand__muluh */

/*
@@@  case INTRN_MULUH:
@@@    Expand__muluh(result[0],opnd[0],opnd[1],ops) ;
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
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
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
 * Expansion of mulum based on validated and scheduled basic assembly source.
*/
static void
Expand__mulum(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_mullu_r,	o0,	i0,	i1,	ops) ;
} /* Expand__mulum */

/*
@@@  case INTRN_MULUM:
@@@    Expand__mulum(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulun based on validated and scheduled basic assembly source.
*/
static void
Expand__mulun(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_29_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mullu_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r,	r0_27_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_0,	i1,	c0,	ops) ;
  Build_OP (	TOP_mulhhs_r,	r0_24_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r,	r0_25_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_slct_i,	r0_28_1,	b0_1_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_slct_i,	r0_29_1,	b0_0_0,	i1,	c0,	ops) ;
  Build_OP (	TOP_add_r,	r0_28_2,	r0_28_1,	r0_29_1,	ops) ;
  Build_OP (	TOP_mtb,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_addcg,	ol0,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_addcg,	r0_17_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
  Build_OP (	TOP_add_r,	oh0,	r0_17_4,	r0_28_2,	ops) ;
} /* Expand__mulun */

/*
@@@  case INTRN_MULUN:
@@@    Expand__mulun(result[0],result[1],opnd[0],opnd[1],ops) ;
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
 * Expansion of nearclw based on validated and scheduled basic assembly source.
*/
static void
Expand__nearclw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpeq_i_b,	b0_0_0,	ih0,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_shru_i,	r0_21_0,	il0,	c31,	ops) ;
  Build_OP (	TOP_add_r,	r0_21_1,	ih0,	r0_21_0,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_20_1,	il0,	c0x80000000,	ops) ;
  Build_OP (	TOP_andc_r,	r0_21_2,	r0_20_1,	r0_21_1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	ih0,	r0_21_2,	ops) ;
} /* Expand__nearclw */

/*
@@@  case INTRN_NEARCLW:
@@@    Expand__nearclw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of nearcwh based on validated and scheduled basic assembly source.
*/
static void
Expand__nearcwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fff0000 = Gen_Literal_TN(__EXTS32TOS64(0x7fff0000), 4) ;
  TN *c0x8000 = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
  TN *c0xffff = Gen_Literal_TN(__EXTS32TOS64(0xffff), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_20_0,	i0,	c0x8000,	ops) ;
  Build_OP (	TOP_and_i,	r0_22_0,	i0,	c0xffff,	ops) ;
  Build_OP (	TOP_shr_i,	r0_20_1,	r0_20_0,	c16,	ops) ;
  Build_OP (	TOP_shr_i,	r0_21_1,	i0,	c16,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_22_1,	r0_22_0,	c0x8000,	ops) ;
  Build_OP (	TOP_cmpge_i_b,	b0_0_2,	i0,	c0x7fff0000,	ops) ;
  Build_OP (	TOP_andc_r,	r0_20_2,	r0_22_1,	r0_20_1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_2,	r0_21_1,	r0_20_2,	ops) ;
} /* Expand__nearcwh */

/*
@@@  case INTRN_NEARCWH:
@@@    Expand__nearcwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of nearlw based on validated and scheduled basic assembly source.
*/
static void
Expand__nearlw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i,	r0_21_0,	il0,	c31,	ops) ;
  Build_OP (	TOP_add_r,	r0_16_1,	ih0,	r0_21_0,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_20_1,	il0,	c0x80000000,	ops) ;
  Build_OP (	TOP_andc_r,	o0,	r0_20_1,	r0_16_1,	ops) ;
} /* Expand__nearlw */

/*
@@@  case INTRN_NEARLW:
@@@    Expand__nearlw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of nearwh based on validated and scheduled basic assembly source.
*/
static void
Expand__nearwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x8000 = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
  TN *c0xffff = Gen_Literal_TN(__EXTS32TOS64(0xffff), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_21_0,	i0,	c0x8000,	ops) ;
  Build_OP (	TOP_and_i,	r0_22_0,	i0,	c0xffff,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_22_1,	r0_22_0,	c0x8000,	ops) ;
  Build_OP (	TOP_shr_i,	r0_21_1,	r0_21_0,	c16,	ops) ;
  Build_OP (	TOP_andc_r,	o0,	r0_22_1,	r0_21_1,	ops) ;
} /* Expand__nearwh */

/*
@@@  case INTRN_NEARWH:
@@@    Expand__nearwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of negch based on validated and scheduled basic assembly source.
*/
static void
Expand__negch(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fff = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpeq_i_b,	b0_0_0,	i0,	c0xffff8000,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
  Build_OP (	TOP_slctf_i,	o0,	b0_0_0,	r0_20_0,	c0x7fff,	ops) ;
} /* Expand__negch */

/*
@@@  case INTRN_NEGCH:
@@@    Expand__negch(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of negcl based on validated and scheduled basic assembly source.
*/
static void
Expand__negcl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpgtu_i_b,	b0_0_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_20_0,	c0,	il0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_21_0,	c0,	ih0,	ops) ;
  Build_OP (	TOP_sub_i,	r0_22_0,	c_1,	ih0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_1,	b0_0_0,	r0_22_0,	r0_21_0,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_22_1,	ih0,	c0x80000000,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_23_1,	il0,	c0,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_2,	r0_22_1,	r0_23_1,	ops) ;
  Build_OP (	TOP_mov_i,	r0_22_2,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_slctf_i,	ol0,	b0_0_2,	r0_20_0,	c_1,	ops) ;
  Build_OP (	TOP_slctf_r,	oh0,	b0_0_2,	r0_21_1,	r0_22_2,	ops) ;
} /* Expand__negcl */

/*
@@@  case INTRN_NEGCL:
@@@    Expand__negcl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of negcw based on validated and scheduled basic assembly source.
*/
static void
Expand__negcw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpeq_i_b,	b0_0_0,	i0,	c0x80000000,	ops) ;
  Build_OP (	TOP_sub_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
  Build_OP (	TOP_slctf_i,	o0,	b0_0_0,	r0_20_0,	c0x7fffffff,	ops) ;
} /* Expand__negcw */

/*
@@@  case INTRN_NEGCW:
@@@    Expand__negcw(result[0],opnd[0],ops) ;
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
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
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
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
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
 * Expansion of normh based on validated and scheduled basic assembly source.
*/
static void
Expand__normh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c_17 = Gen_Literal_TN(__EXTS32TOS64(-17), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_i,	r0_20_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_cmpeq_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_max_r,	r0_20_1,	i0,	r0_20_0,	ops) ;
  Build_OP (	TOP_clz,	r0_16_2,	r0_20_1,	ops) ;
  Build_OP (	TOP_add_i,	r0_16_3,	r0_16_2,	c_17,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	Zero_TN,	r0_16_3,	ops) ;
} /* Expand__normh */

/*
@@@  case INTRN_NORMH:
@@@    Expand__normh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of norml based on validated and scheduled basic assembly source.
*/
static void
Expand__norml(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_i,	r0_22_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_23_0,	ih0,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	ih0,	c0,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_24_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_cmpeq_i_r,	r0_25_1,	ih0,	c0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_22_1,	b0_0_0,	r0_22_0,	il0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_23_1,	b0_0_0,	r0_23_0,	ih0,	ops) ;
  Build_OP (	TOP_clz,	r0_20_2,	r0_23_1,	ops) ;
  Build_OP (	TOP_clz,	r0_21_2,	r0_22_1,	ops) ;
  Build_OP (	TOP_cmpeq_i_b,	b0_0_2,	r0_23_1,	c0,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_1_3,	r0_24_0,	r0_25_1,	ops) ;
  Build_OP (	TOP_add_i,	r0_21_3,	r0_21_2,	c32,	ops) ;
  Build_OP (	TOP_slct_r,	r0_16_4,	b0_0_2,	r0_21_3,	r0_20_2,	ops) ;
  Build_OP (	TOP_add_i,	r0_16_5,	r0_16_4,	c_1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_1_3,	Zero_TN,	r0_16_5,	ops) ;
} /* Expand__norml */

/*
@@@  case INTRN_NORML:
@@@    Expand__norml(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of normw based on validated and scheduled basic assembly source.
*/
static void
Expand__normw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_i,	r0_20_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_cmpeq_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_max_r,	r0_20_1,	i0,	r0_20_0,	ops) ;
  Build_OP (	TOP_clz,	r0_16_2,	r0_20_1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	Zero_TN,	r0_16_2,	ops) ;
} /* Expand__normw */

/*
@@@  case INTRN_NORMW:
@@@    Expand__normw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of priorh based on validated and scheduled basic assembly source.
*/
static void
Expand__priorh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c_17 = Gen_Literal_TN(__EXTS32TOS64(-17), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_i,	r0_20_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_max_r,	r0_20_1,	i0,	r0_20_0,	ops) ;
  Build_OP (	TOP_clz,	r0_16_2,	r0_20_1,	ops) ;
  Build_OP (	TOP_add_i,	o0,	r0_16_2,	c_17,	ops) ;
} /* Expand__priorh */

/*
@@@  case INTRN_PRIORH:
@@@    Expand__priorh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of priorl based on validated and scheduled basic assembly source.
*/
static void
Expand__priorl(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_i,	r0_22_0,	il0,	c0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_23_0,	ih0,	c0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	ih0,	c0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_22_1,	b0_0_0,	r0_22_0,	il0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_23_1,	b0_0_0,	r0_23_0,	ih0,	ops) ;
  Build_OP (	TOP_clz,	r0_20_2,	r0_23_1,	ops) ;
  Build_OP (	TOP_clz,	r0_21_2,	r0_22_1,	ops) ;
  Build_OP (	TOP_cmpeq_i_b,	b0_0_2,	r0_23_1,	c0,	ops) ;
  Build_OP (	TOP_add_i,	r0_21_3,	r0_21_2,	c32,	ops) ;
  Build_OP (	TOP_slct_r,	r0_16_4,	b0_0_2,	r0_21_3,	r0_20_2,	ops) ;
  Build_OP (	TOP_add_i,	o0,	r0_16_4,	c_1,	ops) ;
} /* Expand__priorl */

/*
@@@  case INTRN_PRIORL:
@@@    Expand__priorl(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of priorw based on validated and scheduled basic assembly source.
*/
static void
Expand__priorw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_i,	r0_20_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_max_r,	r0_20_1,	i0,	r0_20_0,	ops) ;
  Build_OP (	TOP_clz,	r0_16_2,	r0_20_1,	ops) ;
  Build_OP (	TOP_add_i,	o0,	r0_16_2,	c_1,	ops) ;
} /* Expand__priorw */

/*
@@@  case INTRN_PRIORW:
@@@    Expand__priorw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of puthl based on validated and scheduled basic assembly source.
*/
static void
Expand__puthl(
 TN* ol0,
 TN* oh0,
 TN* i0,
 OPS* ops
)
{
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	oh0,	i0,	c0,	ops) ;
  Build_OP (	TOP_add_i,	ol0,	Zero_TN,	c0,	ops) ;
} /* Expand__puthl */

/*
@@@  case INTRN_PUTHL:
@@@    Expand__puthl(result[0],result[1],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of puthw based on validated and scheduled basic assembly source.
*/
static void
Expand__puthw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i,	o0,	i0,	c16,	ops) ;
} /* Expand__puthw */

/*
@@@  case INTRN_PUTHW:
@@@    Expand__puthw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of putll based on validated and scheduled basic assembly source.
*/
static void
Expand__putll(
 TN* ol0,
 TN* oh0,
 TN* i0,
 OPS* ops
)
{
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shr_i,	oh0,	ol0,	c31,	ops) ;
} /* Expand__putll */

/*
@@@  case INTRN_PUTLL:
@@@    Expand__putll(result[0],result[1],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of putlw based on validated and scheduled basic assembly source.
*/
static void
Expand__putlw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
} /* Expand__putlw */

/*
@@@  case INTRN_PUTLW:
@@@    Expand__putlw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of rotlh based on validated and scheduled basic assembly source.
*/
static void
Expand__rotlh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_i,	r0_21_0,	c16,	i1,	ops) ;
  Build_OP (	TOP_shru_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__rotlh */

/*
@@@  case INTRN_ROTLH:
@@@    Expand__rotlh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of rotlw based on validated and scheduled basic assembly source.
*/
static void
Expand__rotlw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_i,	r0_21_0,	c32,	i1,	ops) ;
  Build_OP (	TOP_shru_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__rotlw */

/*
@@@  case INTRN_ROTLW:
@@@    Expand__rotlw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of roundclw based on validated and scheduled basic assembly source.
*/
static void
Expand__roundclw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpeq_i_b,	b0_0_0,	ih0,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_shru_i,	r0_21_0,	il0,	c31,	ops) ;
  Build_OP (	TOP_add_r,	r0_21_1,	ih0,	r0_21_0,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	ih0,	r0_21_1,	ops) ;
} /* Expand__roundclw */

/*
@@@  case INTRN_ROUNDCLW:
@@@    Expand__roundclw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of roundcwh based on validated and scheduled basic assembly source.
*/
static void
Expand__roundcwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fff0000 = Gen_Literal_TN(__EXTS32TOS64(0x7fff0000), 4) ;
  TN *c0x8000 = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_20_0,	i0,	c0x8000,	ops) ;
  Build_OP (	TOP_cmpge_i_b,	b0_0_0,	i0,	c0x7fff0000,	ops) ;
  Build_OP (	TOP_shr_i,	r0_20_1,	r0_20_0,	c16,	ops) ;
  Build_OP (	TOP_shr_i,	r0_16_1,	i0,	c16,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_0,	r0_16_1,	r0_20_1,	ops) ;
} /* Expand__roundcwh */

/*
@@@  case INTRN_ROUNDCWH:
@@@    Expand__roundcwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of roundlw based on validated and scheduled basic assembly source.
*/
static void
Expand__roundlw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i,	r0_21_0,	il0,	c31,	ops) ;
  Build_OP (	TOP_add_r,	o0,	ih0,	r0_21_0,	ops) ;
} /* Expand__roundlw */

/*
@@@  case INTRN_ROUNDLW:
@@@    Expand__roundlw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of roundwh based on validated and scheduled basic assembly source.
*/
static void
Expand__roundwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x8000 = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_21_0,	i0,	c0x8000,	ops) ;
  Build_OP (	TOP_shr_i,	o0,	r0_21_0,	c16,	ops) ;
} /* Expand__roundwh */

/*
@@@  case INTRN_ROUNDWH:
@@@    Expand__roundwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of shlch based on validated and scheduled basic assembly source.
*/
static void
Expand__shlch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mov_i,	r0_20_0,	c0xffff8000,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_21_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_22_1,	b0_0_0,	r0_21_0,	i0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_23_1,	r0_20_0,	c0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_20_2,	b0_0_0,	r0_20_0,	r0_23_1,	ops) ;
  Build_OP (	TOP_clz,	r0_21_2,	r0_22_1,	ops) ;
  Build_OP (	TOP_add_i,	r0_24_2,	i1,	c16,	ops) ;
  Build_OP (	TOP_shl_r,	r0_22_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmple_r_b,	b0_0_3,	r0_21_2,	r0_24_2,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_3,	r0_20_2,	r0_22_3,	ops) ;
} /* Expand__shlch */

/*
@@@  case INTRN_SHLCH:
@@@    Expand__shlch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of shlcw based on validated and scheduled basic assembly source.
*/
static void
Expand__shlcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mov_i,	r0_20_0,	c0x80000000,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_0_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_21_0,	i0,	c0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_22_1,	b0_0_0,	r0_21_0,	i0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_23_1,	r0_20_0,	c0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_20_2,	b0_0_0,	r0_20_0,	r0_23_1,	ops) ;
  Build_OP (	TOP_clz,	r0_21_2,	r0_22_1,	ops) ;
  Build_OP (	TOP_shl_r,	r0_22_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmple_r_b,	b0_0_3,	r0_21_2,	i1,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_3,	r0_20_2,	r0_22_3,	ops) ;
} /* Expand__shlcw */

/*
@@@  case INTRN_SHLCW:
@@@    Expand__shlcw(result[0],opnd[0],opnd[1],ops) ;
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
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
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
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
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
 * Expansion of shrrh based on validated and scheduled basic assembly source.
*/
static void
Expand__shrrh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i,	r0_21_0,	i1,	c_1,	ops) ;
  Build_OP (	TOP_shr_r,	r0_20_1,	i0,	i1,	ops) ;
  Build_OP (	TOP_shr_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_and_i,	r0_21_2,	r0_21_1,	c1,	ops) ;
  Build_OP (	TOP_add_r,	o0,	r0_20_1,	r0_21_2,	ops) ;
} /* Expand__shrrh */

/*
@@@  case INTRN_SHRRH:
@@@    Expand__shrrh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of shrrw based on validated and scheduled basic assembly source.
*/
static void
Expand__shrrw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shr_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_add_i,	r0_21_0,	i1,	c_1,	ops) ;
  Build_OP (	TOP_shr_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_and_i,	r0_21_2,	r0_21_1,	c1,	ops) ;
  Build_OP (	TOP_add_r,	o0,	r0_20_0,	r0_21_2,	ops) ;
} /* Expand__shrrw */

/*
@@@  case INTRN_SHRRW:
@@@    Expand__shrrw(result[0],opnd[0],opnd[1],ops) ;
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
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
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
 * Expansion of subch based on validated and scheduled basic assembly source.
*/
static void
Expand__subch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fff = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
  TN *c0xffff8000 = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_sub_r,	r0_16_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_max_i,	r0_16_1,	r0_16_0,	c0xffff8000,	ops) ;
  Build_OP (	TOP_min_i,	o0,	r0_16_1,	c0x7fff,	ops) ;
} /* Expand__subch */

/*
@@@  case INTRN_SUBCH:
@@@    Expand__subch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of subcl based on validated and scheduled basic assembly source.
*/
static void
Expand__subcl(
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
  TN *b0_0_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_19_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_xor_r,	r0_20_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpltu_r_b,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_mov_i,	r0_21_0,	c0x80000000,	ops) ;
  Build_OP (	TOP_addcg,	r0_19_1,	b0_0_1,	ih1,	Zero_TN,	b0_0_0,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_1,	ih0,	c0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_23_1,	r0_21_0,	c0,	ops) ;
  Build_OP (	TOP_sub_r,	r0_22_2,	il0,	il1,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_2,	b0_1_1,	r0_21_0,	r0_23_1,	ops) ;
  Build_OP (	TOP_sub_r,	r0_24_2,	ih0,	r0_19_1,	ops) ;
  Build_OP (	TOP_shru_i,	r0_20_2,	r0_20_0,	c31,	ops) ;
  Build_OP (	TOP_slct_i,	r0_23_3,	b0_1_1,	Zero_TN,	c_1,	ops) ;
  Build_OP (	TOP_xor_r,	r0_25_3,	r0_24_2,	ih0,	ops) ;
  Build_OP (	TOP_shru_i,	r0_25_4,	r0_25_3,	c31,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_5,	r0_20_2,	r0_25_4,	ops) ;
  Build_OP (	TOP_slct_r,	ol0,	b0_0_5,	r0_23_3,	r0_22_2,	ops) ;
  Build_OP (	TOP_slct_r,	oh0,	b0_0_5,	r0_21_2,	r0_24_2,	ops) ;
} /* Expand__subcl */

/*
@@@  case INTRN_SUBCL:
@@@    Expand__subcl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of subcw based on validated and scheduled basic assembly source.
*/
static void
Expand__subcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c0x80000000 = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_xor_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_mov_i,	r0_21_0,	c0x80000000,	ops) ;
  Build_OP (	TOP_sub_r,	r0_22_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmplt_i_b,	b0_1_1,	i0,	c0,	ops) ;
  Build_OP (	TOP_orc_i,	r0_23_1,	r0_21_0,	c0,	ops) ;
  Build_OP (	TOP_xor_r,	r0_24_1,	r0_22_0,	i0,	ops) ;
  Build_OP (	TOP_slct_r,	r0_21_2,	b0_1_1,	r0_21_0,	r0_23_1,	ops) ;
  Build_OP (	TOP_and_r,	r0_20_2,	r0_20_0,	r0_21_0,	ops) ;
  Build_OP (	TOP_and_r,	r0_24_2,	r0_24_1,	r0_21_0,	ops) ;
  Build_OP (	TOP_andl_r_b,	b0_0_3,	r0_20_2,	r0_24_2,	ops) ;
  Build_OP (	TOP_slct_r,	o0,	b0_0_3,	r0_21_2,	r0_22_0,	ops) ;
} /* Expand__subcw */

/*
@@@  case INTRN_SUBCW:
@@@    Expand__subcw(result[0],opnd[0],opnd[1],ops) ;
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

/*
 * Expansion of swapbh based on validated and scheduled basic assembly source.
*/
static void
Expand__swapbh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c8 = Gen_Literal_TN(__EXTS32TOS64(8), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i,	r0_20_0,	i0,	c8,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_0,	i0,	c8,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__swapbh */

/*
@@@  case INTRN_SWAPBH:
@@@    Expand__swapbh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of swapbw based on validated and scheduled basic assembly source.
*/
static void
Expand__swapbw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
} /* Expand__swapbw */

/*
@@@  case INTRN_SWAPBW:
@@@    Expand__swapbw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of swaphw based on validated and scheduled basic assembly source.
*/
static void
Expand__swaphw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i,	r0_20_0,	i0,	c16,	ops) ;
  Build_OP (	TOP_shl_i,	r0_21_0,	i0,	c16,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__swaphw */

/*
@@@  case INTRN_SWAPHW:
@@@    Expand__swaphw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of xshlh based on validated and scheduled basic assembly source.
*/
static void
Expand__xshlh(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_r,	r0_20_0,	i0,	i2,	ops) ;
  Build_OP (	TOP_sub_i,	r0_21_0,	c16,	i2,	ops) ;
  Build_OP (	TOP_shru_r,	r0_21_1,	i1,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__xshlh */

/*
@@@  case INTRN_XSHLH:
@@@    Expand__xshlh(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of xshlw based on validated and scheduled basic assembly source.
*/
static void
Expand__xshlw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_r,	r0_20_0,	i0,	i2,	ops) ;
  Build_OP (	TOP_sub_i,	r0_21_0,	c32,	i2,	ops) ;
  Build_OP (	TOP_shru_r,	r0_21_1,	i1,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__xshlw */

/*
@@@  case INTRN_XSHLW:
@@@    Expand__xshlw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of xshrh based on validated and scheduled basic assembly source.
*/
static void
Expand__xshrh(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_r,	r0_20_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_sub_i,	r0_21_0,	c16,	i2,	ops) ;
  Build_OP (	TOP_shl_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__xshrh */

/*
@@@  case INTRN_XSHRH:
@@@    Expand__xshrh(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of xshrw based on validated and scheduled basic assembly source.
*/
static void
Expand__xshrw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_r,	r0_20_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_sub_i,	r0_21_0,	c32,	i2,	ops) ;
  Build_OP (	TOP_shl_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__xshrw */

/*
@@@  case INTRN_XSHRW:
@@@    Expand__xshrw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
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
    case INTRN_ABSCH:
      Expand__absch(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ABSCL:
      Expand__abscl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ABSCW:
      Expand__abscw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ABSH:
      Expand__absh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ABSL:
      Expand__absl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ABSW:
      Expand__absw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ADDCH:
      Expand__addch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ADDCL:
      Expand__addcl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_ADDCW:
      Expand__addcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ADDL:
      Expand__addl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_ADDUL:
      Expand__addul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_BITCLRH:
      Expand__bitclrh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITCLRW:
      Expand__bitclrw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITCNTH:
      Expand__bitcnth(result[0],opnd[0],ops) ;
    break ;
    case INTRN_BITCNTW:
      Expand__bitcntw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_BITNOTH:
      Expand__bitnoth(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITNOTW:
      Expand__bitnotw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITREVW:
      Expand__bitrevw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITSETH:
      Expand__bitseth(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITSETW:
      Expand__bitsetw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITVALH:
      Expand__bitvalh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITVALW:
      Expand__bitvalw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_CLAMPLW:
      Expand__clamplw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_CLAMPWH:
      Expand__clampwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_DISTH:
      Expand__disth(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_DISTUH:
      Expand__distuh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_DISTUW:
      Expand__distuw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_DISTW:
      Expand__distw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_EDGESH:
      Expand__edgesh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_EDGESW:
      Expand__edgesw(result[0],opnd[0],opnd[1],ops) ;
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
    case INTRN_GETHH:
      Expand__gethh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_GETHW:
      Expand__gethw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_GETLH:
      Expand__getlh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_GETLW:
      Expand__getlw(result[0],opnd[0],opnd[1],ops) ;
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
    case INTRN_INSEQUW:
      Expand__insequw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSEQW:
      Expand__inseqw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSGEUW:
      Expand__insgeuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSGEW:
      Expand__insgew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSGTUW:
      Expand__insgtuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSGTW:
      Expand__insgtw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSLEUW:
      Expand__insleuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSLEW:
      Expand__inslew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSLTUW:
      Expand__insltuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSLTW:
      Expand__insltw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSNEUW:
      Expand__insneuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSNEW:
      Expand__insnew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
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
    case INTRN_LZCNTH:
      Expand__lzcnth(result[0],opnd[0],ops) ;
    break ;
    case INTRN_LZCNTL:
      Expand__lzcntl(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_LZCNTW:
      Expand__lzcntw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_MAFCW:
      Expand__mafcw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_MAXH:
      Expand__maxh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MAXL:
      Expand__maxl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MAXUH:
      Expand__maxuh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MAXUL:
      Expand__maxul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MAXUW:
      Expand__maxuw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MAXW:
      Expand__maxw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MINH:
      Expand__minh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MINL:
      Expand__minl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MINUH:
      Expand__minuh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MINUL:
      Expand__minul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MINUW:
      Expand__minuw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MINW:
      Expand__minw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPFCW:
      Expand__mpfcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPFCWL:
      Expand__mpfcwl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPFML:
      Expand__mpfml(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPFRCH:
      Expand__mpfrch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPML:
      Expand__mpml(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPUML:
      Expand__mpuml(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULFCH:
      Expand__mulfch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULFCM:
      Expand__mulfcm(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULFCW:
      Expand__mulfcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULH:
      Expand__mulh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULHH:
      Expand__mulhh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULHUH:
      Expand__mulhuh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULHUW:
      Expand__mulhuw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULHW:
      Expand__mulhw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULL:
      Expand__mull(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MULM:
      Expand__mulm(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULN:
      Expand__muln(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULUH:
      Expand__muluh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULUL:
      Expand__mulul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MULUM:
      Expand__mulum(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULUN:
      Expand__mulun(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULUW:
      Expand__muluw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULW:
      Expand__mulw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEARCLW:
      Expand__nearclw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEARCWH:
      Expand__nearcwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_NEARLW:
      Expand__nearlw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEARWH:
      Expand__nearwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_NEGCH:
      Expand__negch(result[0],opnd[0],ops) ;
    break ;
    case INTRN_NEGCL:
      Expand__negcl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEGCW:
      Expand__negcw(result[0],opnd[0],ops) ;
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
    case INTRN_NORMH:
      Expand__normh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_NORML:
      Expand__norml(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NORMW:
      Expand__normw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_PRIORH:
      Expand__priorh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_PRIORL:
      Expand__priorl(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_PRIORW:
      Expand__priorw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_PUTHL:
      Expand__puthl(result[0],result[1],opnd[0],ops) ;
    break ;
    case INTRN_PUTHW:
      Expand__puthw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_PUTLL:
      Expand__putll(result[0],result[1],opnd[0],ops) ;
    break ;
    case INTRN_PUTLW:
      Expand__putlw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ROTLH:
      Expand__rotlh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ROTLW:
      Expand__rotlw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ROUNDCLW:
      Expand__roundclw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ROUNDCWH:
      Expand__roundcwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ROUNDLW:
      Expand__roundlw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ROUNDWH:
      Expand__roundwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_SHLCH:
      Expand__shlch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHLCW:
      Expand__shlcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHLL:
      Expand__shll(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_SHRL:
      Expand__shrl(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_SHRRH:
      Expand__shrrh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHRRW:
      Expand__shrrw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHRUL:
      Expand__shrul(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_SUBCH:
      Expand__subch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SUBCL:
      Expand__subcl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_SUBCW:
      Expand__subcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SUBL:
      Expand__subl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_SUBUL:
      Expand__subul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_SWAPBH:
      Expand__swapbh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_SWAPBW:
      Expand__swapbw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_SWAPHW:
      Expand__swaphw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_XSHLH:
      Expand__xshlh(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_XSHLW:
      Expand__xshlw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_XSHRH:
      Expand__xshrh(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_XSHRW:
      Expand__xshrw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    default:
      FmtAssert (FALSE, ("Exp_Intrinsic_Op: unknown intrinsic op %s", INTRN_c_name(id)));
  } /* switch*/
}
