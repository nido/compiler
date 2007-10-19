/* Test of intrinsics generated from MDS. */
/* This is not a functional test (auto check)
 * but just to test the compiler configuration
 * i.e. is the front-end recognized intrinsics, ...
 */

/* Note on dedicated registers: as these registers are not allocated by the compiler,
 * it is necessary to pass the index of such register. When it is passed as builtin
 * operand, an immediate of the form 'u[0..n]' is expected in place of dedicated register.
 * 'n' is the size of the register file minus one.
 * When dedicated register is the result, one additional operand of the same form 'u[0..n]'
 * is placed as first parameter to select the result register index.
 *
 * Note on immediates: some builtins may accept constant value as parameter. Programmer must
 * respect the immediate range given by the bits number and signess.
 * Immediates are given under the form '[su][0-9]+'
 * - [su] for signed and unsigned respectively.
 * - [0-9]+ the bits number associated to the immediate.
 */


#include "builtins_header.h"

/* abss.ph integer = integer
 * integer abss.ph( integer);
 */
int
test___st200abss_ph(int a) {
  return __builtin___st200abss_ph(a);
}

/* absubu.pb integer = integer, integer
 * integer absubu.pb( integer, integer);
 */
int
test___st200absubu_pb(int a, int b) {
  return __builtin___st200absubu_pb(a, b);
}

/* addso integer = integer, integer
 * integer addso( integer, integer);
 */
int
test___st200addso(int a, int b) {
  return __builtin___st200addso(a, b);
}

/* adds.ph integer = integer, integer
 * integer adds.ph( integer, integer);
 */
int
test___st200adds_ph(int a, int b) {
  return __builtin___st200adds_ph(a, b);
}

/* add.ph integer = integer, integer
 * integer add.ph( integer, integer);
 */
int
test___st200add_ph(int a, int b) {
  return __builtin___st200add_ph(a, b);
}

/* avg4u.pb ISA_REGISTER_SUBCLASS_nolink = branch, integer, integer
 * ISA_REGISTER_SUBCLASS_nolink avg4u.pb( branch, integer, integer);
 */
int
test___st200avg4u_pb(unsigned a, int b, int c) {
  return __builtin___st200avg4u_pb(a, b, c);
}

/* avgu.pb integer = branch, integer, integer
 * integer avgu.pb( branch, integer, integer);
 */
int
test___st200avgu_pb(unsigned a, int b, int c) {
  return __builtin___st200avgu_pb(a, b, c);
}

/* cmpeq.pb integer = integer, integer
 * integer cmpeq.pb( integer, integer);
 */
int
test___st200cmpeq_pb(int a, int b) {
  return __builtin___st200cmpeq_pb(a, b);
}

/* cmpeq.pb branch = integer, integer
 * branch cmpeq.pb( integer, integer);
 */
unsigned
test___st200cmpeq_pb_bdest(int a, int b) {
  return __builtin___st200cmpeq_pb_bdest(a, b);
}

/* cmpeq.ph integer = integer, integer
 * integer cmpeq.ph( integer, integer);
 */
int
test___st200cmpeq_ph(int a, int b) {
  return __builtin___st200cmpeq_ph(a, b);
}

/* cmpeq.ph branch = integer, integer
 * branch cmpeq.ph( integer, integer);
 */
unsigned
test___st200cmpeq_ph_bdest(int a, int b) {
  return __builtin___st200cmpeq_ph_bdest(a, b);
}

/* cmpgtu.pb integer = integer, integer
 * integer cmpgtu.pb( integer, integer);
 */
int
test___st200cmpgtu_pb(int a, int b) {
  return __builtin___st200cmpgtu_pb(a, b);
}

/* cmpgtu.pb branch = integer, integer
 * branch cmpgtu.pb( integer, integer);
 */
unsigned
test___st200cmpgtu_pb_bdest(int a, int b) {
  return __builtin___st200cmpgtu_pb_bdest(a, b);
}

/* cmpgt.ph integer = integer, integer
 * integer cmpgt.ph( integer, integer);
 */
int
test___st200cmpgt_ph(int a, int b) {
  return __builtin___st200cmpgt_ph(a, b);
}

/* cmpgt.ph branch = integer, integer
 * branch cmpgt.ph( integer, integer);
 */
unsigned
test___st200cmpgt_ph_bdest(int a, int b) {
  return __builtin___st200cmpgt_ph_bdest(a, b);
}

/* dib
 * dib( );
 */
void
test___st200dib(void) {
  __builtin___st200dib();
}

/* ext1.pb integer = integer, integer
 * integer ext1.pb( integer, integer);
 */
int
test___st200ext1_pb(int a, int b) {
  return __builtin___st200ext1_pb(a, b);
}

/* ext2.pb integer = integer, integer
 * integer ext2.pb( integer, integer);
 */
int
test___st200ext2_pb(int a, int b) {
  return __builtin___st200ext2_pb(a, b);
}

/* ext3.pb integer = integer, integer
 * integer ext3.pb( integer, integer);
 */
int
test___st200ext3_pb(int a, int b) {
  return __builtin___st200ext3_pb(a, b);
}

/* extl.pb integer = branch, integer, integer
 * integer extl.pb( branch, integer, integer);
 */
int
test___st200extl_pb(unsigned a, int b, int c) {
  return __builtin___st200extl_pb(a, b, c);
}

/* extr.pb integer = branch, integer, integer
 * integer extr.pb( branch, integer, integer);
 */
int
test___st200extr_pb(unsigned a, int b, int c) {
  return __builtin___st200extr_pb(a, b, c);
}

/* flushadd <s32>[integer]
 * flushadd( <s32>, integer);
 */
void
test___st200flushadd(int a) {
  __builtin___st200flushadd(0, a);
}

/* flushadd.l1 <s32>[integer]
 * flushadd.l1( <s32>, integer);
 */
void
test___st200flushadd_l1(int a) {
  __builtin___st200flushadd_l1(0, a);
}

/* invadd <s32>[integer]
 * invadd( <s32>, integer);
 */
void
test___st200invadd(int a) {
  __builtin___st200invadd(0, a);
}

/* invadd.l1 <s32>[integer]
 * invadd.l1( <s32>, integer);
 */
void
test___st200invadd_l1(int a) {
  __builtin___st200invadd_l1(0, a);
}

/* ldwl integer = [integer]
 * integer ldwl( integer);
 */
int
test___st200ldwl(int a) {
  return __builtin___st200ldwl(a);
}

/* maxu integer = integer, integer
 * integer maxu( integer, integer);
 */
unsigned
test___maxuw(unsigned a, unsigned b) {
  return __builtin___maxuw(a, b);
}

/* max integer = integer, integer
 * integer max( integer, integer);
 */
int
test___maxw(int a, int b) {
  return __builtin___maxw(a, b);
}

/* max.ph integer = integer, integer
 * integer max.ph( integer, integer);
 */
int
test___st200max_ph(int a, int b) {
  return __builtin___st200max_ph(a, b);
}

/* minu integer = integer, integer
 * integer minu( integer, integer);
 */
unsigned
test___minuw(unsigned a, unsigned b) {
  return __builtin___minuw(a, b);
}

/* min integer = integer, integer
 * integer min( integer, integer);
 */
int
test___minw(int a, int b) {
  return __builtin___minw(a, b);
}

/* min.ph integer = integer, integer
 * integer min.ph( integer, integer);
 */
int
test___st200min_ph(int a, int b) {
  return __builtin___st200min_ph(a, b);
}

/* muladdus.pb ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink muladdus.pb( integer, integer);
 */
int
test___st200muladdus_pb(int a, int b) {
  return __builtin___st200muladdus_pb(a, b);
}

/* muladd.ph ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink muladd.ph( integer, integer);
 */
int
test___st200muladd_ph(int a, int b) {
  return __builtin___st200muladd_ph(a, b);
}

/* mulfracadds.ph ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink mulfracadds.ph( integer, integer);
 */
int
test___st200mulfracadds_ph(int a, int b) {
  return __builtin___st200mulfracadds_ph(a, b);
}

/* mulfracrm.ph ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink mulfracrm.ph( integer, integer);
 */
int
test___st200mulfracrm_ph(int a, int b) {
  return __builtin___st200mulfracrm_ph(a, b);
}

/* mulfracrne.ph ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink mulfracrne.ph( integer, integer);
 */
int
test___st200mulfracrne_ph(int a, int b) {
  return __builtin___st200mulfracrne_ph(a, b);
}

/* mul.ph ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink mul.ph( integer, integer);
 */
int
test___st200mul_ph(int a, int b) {
  return __builtin___st200mul_ph(a, b);
}

/* packrnp.phh integer = integer, integer
 * integer packrnp.phh( integer, integer);
 */
int
test___st200packrnp_phh(int a, int b) {
  return __builtin___st200packrnp_phh(a, b);
}

/* packsu.pb integer = integer, integer
 * integer packsu.pb( integer, integer);
 */
int
test___st200packsu_pb(int a, int b) {
  return __builtin___st200packsu_pb(a, b);
}

/* packs.ph integer = integer, integer
 * integer packs.ph( integer, integer);
 */
int
test___st200packs_ph(int a, int b) {
  return __builtin___st200packs_ph(a, b);
}

/* pack.pb integer = integer, integer
 * integer pack.pb( integer, integer);
 */
int
test___st200pack_pb(int a, int b) {
  return __builtin___st200pack_pb(a, b);
}

/* pack.ph integer = integer, integer
 * integer pack.ph( integer, integer);
 */
int
test___st200pack_ph(int a, int b) {
  return __builtin___st200pack_ph(a, b);
}

/* perm.pb integer = integer, integer
 * integer perm.pb( integer, integer);
 */
int
test___st200perm_pb(int a, int b) {
  return __builtin___st200perm_pb(a, b);
}

/* prgadd.l1 <s32>[integer]
 * prgadd.l1( <s32>, integer);
 */
void
test___st220prgadd_l1(int a) {
  __builtin___st220prgadd_l1(0, a);
}

/* prgadd.l1 <s32>[integer]
 * prgadd.l1( <s32>, integer);
 */
void
test___st200prgadd_l1(int a) {
  __builtin___st200prgadd_l1(0, a);
}

/* prgins
 * prgins( );
 */
void
test___st220prgins(void) {
  __builtin___st220prgins();
}

/* prgins
 * prgins( );
 */
void
test___st200prgins(void) {
  __builtin___st200prgins();
}

/* prginsadd <s32>[integer]
 * prginsadd( <s32>, integer);
 */
void
test___st200prginsadd(int a) {
  __builtin___st200prginsadd(0, a);
}

/* prginsadd.l1 <s32>[integer]
 * prginsadd.l1( <s32>, integer);
 */
void
test___st200prginsadd_l1(int a) {
  __builtin___st200prginsadd_l1(0, a);
}

/* prginsset <s32>[integer]
 * prginsset( <s32>, integer);
 */
void
test___st200prginsset(int a) {
  __builtin___st200prginsset(0, a);
}

/* prginsset.l1 <s32>[integer]
 * prginsset.l1( <s32>, integer);
 */
void
test___st200prginsset_l1(int a) {
  __builtin___st200prginsset_l1(0, a);
}

/* prgset.l1 <s32>[integer]
 * prgset.l1( <s32>, integer);
 */
void
test___st200prgset_l1(int a) {
  __builtin___st200prgset_l1(0, a);
}

/* pswclr integer
 * pswclr( integer);
 */
void
test___st200pswclr(int a) {
  __builtin___st200pswclr(a);
}

/* pswmask ISA_REGISTER_SUBCLASS_nolink = integer, <s32>
 * ISA_REGISTER_SUBCLASS_nolink pswmask( integer, <s32>);
 */
int
test___st200pswmask(int a) {
  return __builtin___st200pswmask(a, 0);
}

/* pswset integer
 * pswset( integer);
 */
void
test___st200pswset(int a) {
  __builtin___st200pswset(a);
}

/* retention
 * retention( );
 */
void
test___st200retention(void) {
  __builtin___st200retention();
}

/* sadu.pb ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink sadu.pb( integer, integer);
 */
int
test___st200sadu_pb(int a, int b) {
  return __builtin___st200sadu_pb(a, b);
}

/* satso integer = integer
 * integer satso( integer);
 */
int
test___st200satso(int a) {
  return __builtin___st200satso(a);
}

/* sh1addso integer = integer, integer
 * integer sh1addso( integer, integer);
 */
int
test___st200sh1addso(int a, int b) {
  return __builtin___st200sh1addso(a, b);
}

/* sh1subso integer = integer, integer
 * integer sh1subso( integer, integer);
 */
int
test___st200sh1subso(int a, int b) {
  return __builtin___st200sh1subso(a, b);
}

/* shlso ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink shlso( integer, integer);
 */
int
test___st200shlso(int a, int b) {
  return __builtin___st200shlso(a, b);
}

/* shls ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink shls( integer, integer);
 */
int
test___st200shls(int a, int b) {
  return __builtin___st200shls(a, b);
}

/* shls.ph ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink shls.ph( integer, integer);
 */
int
test___st200shls_ph(int a, int b) {
  return __builtin___st200shls_ph(a, b);
}

/* shl.ph integer = integer, integer
 * integer shl.ph( integer, integer);
 */
int
test___st200shl_ph(int a, int b) {
  return __builtin___st200shl_ph(a, b);
}

/* shrrne.ph ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink shrrne.ph( integer, integer);
 */
int
test___st200shrrne_ph(int a, int b) {
  return __builtin___st200shrrne_ph(a, b);
}

/* shrrnp ISA_REGISTER_SUBCLASS_nolink = integer, <s32>
 * ISA_REGISTER_SUBCLASS_nolink shrrnp( integer, <s32>);
 */
int
test___st200shrrnp(int a) {
  return __builtin___st200shrrnp(a, 0);
}

/* shrrnp.ph ISA_REGISTER_SUBCLASS_nolink = integer, integer
 * ISA_REGISTER_SUBCLASS_nolink shrrnp.ph( integer, integer);
 */
int
test___st200shrrnp_ph(int a, int b) {
  return __builtin___st200shrrnp_ph(a, b);
}

/* shr.ph integer = integer, integer
 * integer shr.ph( integer, integer);
 */
int
test___st200shr_ph(int a, int b) {
  return __builtin___st200shr_ph(a, b);
}

/* shuffeve.pb integer = integer, integer
 * integer shuffeve.pb( integer, integer);
 */
int
test___st200shuffeve_pb(int a, int b) {
  return __builtin___st200shuffeve_pb(a, b);
}

/* shuffodd.pb integer = integer, integer
 * integer shuffodd.pb( integer, integer);
 */
int
test___st200shuffodd_pb(int a, int b) {
  return __builtin___st200shuffodd_pb(a, b);
}

/* shuff.pbh integer = integer, integer
 * integer shuff.pbh( integer, integer);
 */
int
test___st200shuff_pbh(int a, int b) {
  return __builtin___st200shuff_pbh(a, b);
}

/* shuff.pbl integer = integer, integer
 * integer shuff.pbl( integer, integer);
 */
int
test___st200shuff_pbl(int a, int b) {
  return __builtin___st200shuff_pbl(a, b);
}

/* shuff.phh integer = integer, integer
 * integer shuff.phh( integer, integer);
 */
int
test___st200shuff_phh(int a, int b) {
  return __builtin___st200shuff_phh(a, b);
}

/* shuff.phl integer = integer, integer
 * integer shuff.phl( integer, integer);
 */
int
test___st200shuff_phl(int a, int b) {
  return __builtin___st200shuff_phl(a, b);
}

/* slct.pb integer = branch, integer, integer
 * integer slct.pb( branch, integer, integer);
 */
int
test___st200slct_pb(unsigned a, int b, int c) {
  return __builtin___st200slct_pb(a, b, c);
}

/* stwl branch, [integer] = integer
 * branch stwl( integer, integer);
 */
unsigned
test___st200stwl(int a, int b) {
  return __builtin___st200stwl(a, b);
}

/* subso integer = integer, integer
 * integer subso( integer, integer);
 */
int
test___st200subso(int a, int b) {
  return __builtin___st200subso(a, b);
}

/* subs.ph integer = integer, integer
 * integer subs.ph( integer, integer);
 */
int
test___st200subs_ph(int a, int b) {
  return __builtin___st200subs_ph(a, b);
}

/* sub.ph integer = integer, integer
 * integer sub.ph( integer, integer);
 */
int
test___st200sub_ph(int a, int b) {
  return __builtin___st200sub_ph(a, b);
}

/* sync
 * sync( );
 */
void
test___st220sync(void) {
  __builtin___st220sync();
}

/* sync
 * sync( );
 */
void
test___st200sync(void) {
  __builtin___st200sync();
}

/* syncins
 * syncins( );
 */
void
test___st220syncins(void) {
  __builtin___st220syncins();
}

/* syncins
 * syncins( );
 */
void
test___st200syncins(void) {
  __builtin___st200syncins();
}

/* unpacku.pbh integer = integer
 * integer unpacku.pbh( integer);
 */
int
test___st200unpacku_pbh(int a) {
  return __builtin___st200unpacku_pbh(a);
}

/* unpacku.pbl integer = integer
 * integer unpacku.pbl( integer);
 */
int
test___st200unpacku_pbl(int a) {
  return __builtin___st200unpacku_pbl(a);
}

/* waitl
 * waitl( );
 */
void
test___st200waitl(void) {
  __builtin___st200waitl();
}

/* wmb
 * wmb( );
 */
void
test___st200wmb(void) {
  __builtin___st200wmb();
}

