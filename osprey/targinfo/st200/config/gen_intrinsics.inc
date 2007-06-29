#ifdef INTRN_EXPAND

/* Check_Opd_RClass
 *   Expand a copy if source tn is not of given register class.
 */
static inline TN*
Check_Opd_RClass(TN *opd, ISA_REGISTER_CLASS rc, OPS *ops) {
  if(TN_is_register(opd) && TN_register_class(opd) != rc) {
    TN *tn_rc = Build_RCLASS_TN(rc);
    Expand_Copy(tn_rc,True_TN,opd,ops);
    return tn_rc;
  }
  else {
    return opd;
  }
}

/* Check_Opd_Literal
 *  Check the availability of TN value and if it fit in
 *  corresponding literal class range.
 *  Return literal TN.
 */
static inline TN*
Check_Opd_Literal( TN *tn_opnd, ISA_LIT_CLASS lc, INT32 arg_idx, const char *intrn_name, SRCPOS srcpos  ) {
  char error_msg[256];

  TN *current_value = Gen_Literal_TN(-1,4);

  if (TN_is_rematerializable(tn_opnd)) {
    WN *wn = TN_home(tn_opnd);
    if (WN_operator_is(wn, OPR_INTCONST)) {
      TN *cunknown = Gen_Literal_TN(WN_const_val(wn), 4);
      current_value = cunknown;
    } else {
      DevWarn("%s::Expand_Extension_intrinsic:"
	" TN_is_rematerializable BUT WN_operator_is *not* OPR_INTCONST.", __FILE__);
    }
  }
  else if (TN_is_zero(tn_opnd)) {
    current_value = Gen_Literal_TN(0,4);
  }
  else if (TN_has_value(tn_opnd)) {
   current_value = tn_opnd;
  }
  else {
    sprintf(error_msg, "Immediate value expected for arg %d of intrinsic call '%s'",
                       arg_idx, intrn_name);
    if (SRCPOS_linenum(srcpos)>0) {
      ErrMsgLine(EC_CG_Generic_Fatal, SRCPOS_linenum(srcpos), error_msg);
    }
    else {
      ErrMsg(EC_CG_Generic_Fatal, error_msg);
    }
  }

  // Check immediate value is in range
  if (!ISA_LC_Value_In_Class(TN_value(current_value), lc)) {
    sprintf(error_msg, "Immediate value '%lld' of arg %d out of bounds"
          " for intrinsic call '%s'.",
          TN_value(current_value), arg_idx, intrn_name);
    if (SRCPOS_linenum(srcpos)>0) {
      ErrMsgLine(EC_CG_Generic_Fatal, SRCPOS_linenum(srcpos), error_msg);
    }
    else {
      ErrMsg(EC_CG_Generic_Fatal, error_msg);
    }
  }
  return current_value;
}

/* Check_Opd_Literal_For_Dedicated
 *  Check the availability of TN value and if it fit in
 *  given register idx range [0..reg_file_size[.
 *  Return literal TN.
 */
static inline TN*
Check_Opd_Literal_For_Dedicated( TN *tn_opnd, INT32 reg_file_size, INT32 arg_idx, const char *intrn_name, SRCPOS srcpos  ) {
  char error_msg[256];

  TN *current_value = Gen_Literal_TN(-1,4);

  if (TN_is_rematerializable(tn_opnd)) {
    WN *wn = TN_home(tn_opnd);
    if (WN_operator_is(wn, OPR_INTCONST)) {
      INT64 value = WN_const_val(wn);
        current_value = Gen_Literal_TN(WN_const_val(wn), 4);
    } else {
      DevWarn("%s::Expand_Extension_intrinsic:"
	" TN_is_rematerializable BUT WN_operator_is *not* OPR_INTCONST.", __FILE__);
    }
  }
  else if (TN_is_zero(tn_opnd)) {
    current_value = Gen_Literal_TN(0,4);
  }
  else if (TN_has_value(tn_opnd)) {
   current_value = tn_opnd;
  }
  else {
    sprintf(error_msg, "Immediate value expected for arg %d of intrinsic call '%s'",
                       arg_idx, intrn_name);
    if (SRCPOS_linenum(srcpos)>0) {
      ErrMsgLine(EC_CG_Generic_Fatal, SRCPOS_linenum(srcpos), error_msg);
    }
    else {
      ErrMsg(EC_CG_Generic_Fatal, error_msg);
    }
  }

  // Check immediate value is in range
  if ( ! (0 <= TN_value(current_value) && TN_value(current_value) < reg_file_size) ) {
    sprintf(error_msg, "Immediate value '%lld' of arg %d out of bounds"
          " for intrinsic call '%s'.",
          TN_value(current_value), arg_idx, intrn_name);
    if (SRCPOS_linenum(srcpos)>0) {
      ErrMsgLine(EC_CG_Generic_Fatal, SRCPOS_linenum(srcpos), error_msg);
    }
    else {
      ErrMsg(EC_CG_Generic_Fatal, error_msg);
    }
  }
  return current_value;
}

/* Expansion of __st200abss_ph from MDS */
static void
Expand___st200abss_ph(
 TN *o0,
 TN *i0,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_abss_ph_r_r)) {
    Build_OP( TOP_abss_ph_r_r,	o0,	i0,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_abss_ph_r_r, ops);
  }
} /* Expand___st200abss_ph */

/* Expansion of __st200absubu_pb from MDS */
static void
Expand___st200absubu_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_absubu_pb_r_r_r)) {
    Build_OP( TOP_absubu_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_absubu_pb_r_r_r, ops);
  }
} /* Expand___st200absubu_pb */

/* Expansion of __st200addso from MDS */
static void
Expand___st200addso(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_addso_r_r_r)) {
    Build_OP( TOP_addso_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_addso_r_r_r, ops);
  }
} /* Expand___st200addso */

/* Expansion of __st200adds_ph from MDS */
static void
Expand___st200adds_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_adds_ph_r_r_r)) {
    Build_OP( TOP_adds_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_adds_ph_r_r_r, ops);
  }
} /* Expand___st200adds_ph */

/* Expansion of __st200add_ph from MDS */
static void
Expand___st200add_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_add_ph_r_r_r)) {
    Build_OP( TOP_add_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_add_ph_r_r_r, ops);
  }
} /* Expand___st200add_ph */

/* Expansion of __st200avg4u_pb from MDS */
static void
Expand___st200avg4u_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 TN *i2,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_avg4u_pb_r_r_b_r)) {
    Build_OP( TOP_avg4u_pb_r_r_b_r,	o0,	i0,	i1,	i2,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_avg4u_pb_r_r_b_r, ops);
  }
} /* Expand___st200avg4u_pb */

/* Expansion of __st200avgu_pb from MDS */
static void
Expand___st200avgu_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 TN *i2,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_avgu_pb_r_r_b_r)) {
    Build_OP( TOP_avgu_pb_r_r_b_r,	o0,	i0,	i1,	i2,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_avgu_pb_r_r_b_r, ops);
  }
} /* Expand___st200avgu_pb */

/* Expansion of __st200cmpeq_pb from MDS */
static void
Expand___st200cmpeq_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_cmpeq_pb_r_r_r)) {
    Build_OP( TOP_cmpeq_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_cmpeq_pb_r_r_r, ops);
  }
} /* Expand___st200cmpeq_pb */

/* Expansion of __st200cmpeq_pb_bdest from MDS */
static void
Expand___st200cmpeq_pb_bdest(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_cmpeq_pb_r_r_b)) {
    Build_OP( TOP_cmpeq_pb_r_r_b,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_cmpeq_pb_r_r_b, ops);
  }
} /* Expand___st200cmpeq_pb_bdest */

/* Expansion of __st200cmpeq_ph from MDS */
static void
Expand___st200cmpeq_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_cmpeq_ph_r_r_r)) {
    Build_OP( TOP_cmpeq_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_cmpeq_ph_r_r_r, ops);
  }
} /* Expand___st200cmpeq_ph */

/* Expansion of __st200cmpeq_ph_bdest from MDS */
static void
Expand___st200cmpeq_ph_bdest(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_cmpeq_ph_r_r_b)) {
    Build_OP( TOP_cmpeq_ph_r_r_b,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_cmpeq_ph_r_r_b, ops);
  }
} /* Expand___st200cmpeq_ph_bdest */

/* Expansion of __st200cmpgtu_pb from MDS */
static void
Expand___st200cmpgtu_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_cmpgtu_pb_r_r_r)) {
    Build_OP( TOP_cmpgtu_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_cmpgtu_pb_r_r_r, ops);
  }
} /* Expand___st200cmpgtu_pb */

/* Expansion of __st200cmpgtu_pb_bdest from MDS */
static void
Expand___st200cmpgtu_pb_bdest(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_cmpgtu_pb_r_r_b)) {
    Build_OP( TOP_cmpgtu_pb_r_r_b,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_cmpgtu_pb_r_r_b, ops);
  }
} /* Expand___st200cmpgtu_pb_bdest */

/* Expansion of __st200cmpgt_ph from MDS */
static void
Expand___st200cmpgt_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_cmpgt_ph_r_r_r)) {
    Build_OP( TOP_cmpgt_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_cmpgt_ph_r_r_r, ops);
  }
} /* Expand___st200cmpgt_ph */

/* Expansion of __st200cmpgt_ph_bdest from MDS */
static void
Expand___st200cmpgt_ph_bdest(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_cmpgt_ph_r_r_b)) {
    Build_OP( TOP_cmpgt_ph_r_r_b,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_cmpgt_ph_r_r_b, ops);
  }
} /* Expand___st200cmpgt_ph_bdest */

/* Expansion of __st200ext1_pb from MDS */
static void
Expand___st200ext1_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_ext1_pb_r_r_r)) {
    Build_OP( TOP_ext1_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_ext1_pb_r_r_r, ops);
  }
} /* Expand___st200ext1_pb */

/* Expansion of __st200ext2_pb from MDS */
static void
Expand___st200ext2_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_ext2_pb_r_r_r)) {
    Build_OP( TOP_ext2_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_ext2_pb_r_r_r, ops);
  }
} /* Expand___st200ext2_pb */

/* Expansion of __st200ext3_pb from MDS */
static void
Expand___st200ext3_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_ext3_pb_r_r_r)) {
    Build_OP( TOP_ext3_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_ext3_pb_r_r_r, ops);
  }
} /* Expand___st200ext3_pb */

/* Expansion of __st200extl_pb from MDS */
static void
Expand___st200extl_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 TN *i2,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_extl_pb_r_r_b_r)) {
    Build_OP( TOP_extl_pb_r_r_b_r,	o0,	i0,	i1,	i2,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_extl_pb_r_r_b_r, ops);
  }
} /* Expand___st200extl_pb */

/* Expansion of __st200extr_pb from MDS */
static void
Expand___st200extr_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 TN *i2,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_extr_pb_r_r_b_r)) {
    Build_OP( TOP_extr_pb_r_r_b_r,	o0,	i0,	i1,	i2,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_extr_pb_r_r_b_r, ops);
  }
} /* Expand___st200extr_pb */

/* Expansion of __st200flushadd from MDS */
static void
Expand___st200flushadd(
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_flushadd_r_ii)) {
    Build_OP( TOP_flushadd_r_ii,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_flushadd_r_ii, ops);
  }
} /* Expand___st200flushadd */

/* Expansion of __st200flushadd_l1 from MDS */
static void
Expand___st200flushadd_l1(
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_flushadd_l1_r_ii)) {
    Build_OP( TOP_flushadd_l1_r_ii,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_flushadd_l1_r_ii, ops);
  }
} /* Expand___st200flushadd_l1 */

/* Expansion of __st200invadd from MDS */
static void
Expand___st200invadd(
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_invadd_r_ii)) {
    Build_OP( TOP_invadd_r_ii,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_invadd_r_ii, ops);
  }
} /* Expand___st200invadd */

/* Expansion of __st200invadd_l1 from MDS */
static void
Expand___st200invadd_l1(
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_invadd_l1_r_ii)) {
    Build_OP( TOP_invadd_l1_r_ii,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_invadd_l1_r_ii, ops);
  }
} /* Expand___st200invadd_l1 */

/* Expansion of __maxuw from MDS */
static void
Expand___maxuw(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_maxu_r_r_r)) {
    Build_OP( TOP_maxu_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_maxu_r_r_r, ops);
  }
} /* Expand___maxuw */

/* Expansion of __maxw from MDS */
static void
Expand___maxw(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_max_r_r_r)) {
    Build_OP( TOP_max_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_max_r_r_r, ops);
  }
} /* Expand___maxw */

/* Expansion of __st200max_ph from MDS */
static void
Expand___st200max_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_max_ph_r_r_r)) {
    Build_OP( TOP_max_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_max_ph_r_r_r, ops);
  }
} /* Expand___st200max_ph */

/* Expansion of __minuw from MDS */
static void
Expand___minuw(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_minu_r_r_r)) {
    Build_OP( TOP_minu_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_minu_r_r_r, ops);
  }
} /* Expand___minuw */

/* Expansion of __minw from MDS */
static void
Expand___minw(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_min_r_r_r)) {
    Build_OP( TOP_min_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_min_r_r_r, ops);
  }
} /* Expand___minw */

/* Expansion of __st200min_ph from MDS */
static void
Expand___st200min_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_min_ph_r_r_r)) {
    Build_OP( TOP_min_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_min_ph_r_r_r, ops);
  }
} /* Expand___st200min_ph */

/* Expansion of __st200muladdus_pb from MDS */
static void
Expand___st200muladdus_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_muladdus_pb_r_r_r)) {
    Build_OP( TOP_muladdus_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_muladdus_pb_r_r_r, ops);
  }
} /* Expand___st200muladdus_pb */

/* Expansion of __st200muladd_ph from MDS */
static void
Expand___st200muladd_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_muladd_ph_r_r_r)) {
    Build_OP( TOP_muladd_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_muladd_ph_r_r_r, ops);
  }
} /* Expand___st200muladd_ph */

/* Expansion of __st200mulfracadds_ph from MDS */
static void
Expand___st200mulfracadds_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_mulfracadds_ph_r_r_r)) {
    Build_OP( TOP_mulfracadds_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_mulfracadds_ph_r_r_r, ops);
  }
} /* Expand___st200mulfracadds_ph */

/* Expansion of __st200mulfracrm_ph from MDS */
static void
Expand___st200mulfracrm_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_mulfracrm_ph_r_r_r)) {
    Build_OP( TOP_mulfracrm_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_mulfracrm_ph_r_r_r, ops);
  }
} /* Expand___st200mulfracrm_ph */

/* Expansion of __st200mulfracrne_ph from MDS */
static void
Expand___st200mulfracrne_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_mulfracrne_ph_r_r_r)) {
    Build_OP( TOP_mulfracrne_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_mulfracrne_ph_r_r_r, ops);
  }
} /* Expand___st200mulfracrne_ph */

/* Expansion of __st200mul_ph from MDS */
static void
Expand___st200mul_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_mul_ph_r_r_r)) {
    Build_OP( TOP_mul_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_mul_ph_r_r_r, ops);
  }
} /* Expand___st200mul_ph */

/* Expansion of __st200packrnp_phh from MDS */
static void
Expand___st200packrnp_phh(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_packrnp_phh_r_r_r)) {
    Build_OP( TOP_packrnp_phh_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_packrnp_phh_r_r_r, ops);
  }
} /* Expand___st200packrnp_phh */

/* Expansion of __st200packsu_pb from MDS */
static void
Expand___st200packsu_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_packsu_pb_r_r_r)) {
    Build_OP( TOP_packsu_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_packsu_pb_r_r_r, ops);
  }
} /* Expand___st200packsu_pb */

/* Expansion of __st200packs_ph from MDS */
static void
Expand___st200packs_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_packs_ph_r_r_r)) {
    Build_OP( TOP_packs_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_packs_ph_r_r_r, ops);
  }
} /* Expand___st200packs_ph */

/* Expansion of __st200pack_pb from MDS */
static void
Expand___st200pack_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_pack_pb_r_r_r)) {
    Build_OP( TOP_pack_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_pack_pb_r_r_r, ops);
  }
} /* Expand___st200pack_pb */

/* Expansion of __st200pack_ph from MDS */
static void
Expand___st200pack_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_pack_ph_r_r_r)) {
    Build_OP( TOP_pack_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_pack_ph_r_r_r, ops);
  }
} /* Expand___st200pack_ph */

/* Expansion of __st200perm_pb from MDS */
static void
Expand___st200perm_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_perm_pb_r_r_r)) {
    Build_OP( TOP_perm_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_perm_pb_r_r_r, ops);
  }
} /* Expand___st200perm_pb */

/* Expansion of __st220prgadd_l1 from MDS */
static void
Expand___st220prgadd_l1(
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_prgadd_l1_r_ii)) {
    Build_OP( TOP_prgadd_l1_r_ii,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_prgadd_l1_r_ii, ops);
  }
} /* Expand___st220prgadd_l1 */

/* Expansion of __st220prgins from MDS */
static void
Expand___st220prgins(
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_prgins)) {
    Build_OP( TOP_prgins,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_prgins, ops);
  }
} /* Expand___st220prgins */

/* Expansion of __st200prginsadd from MDS */
static void
Expand___st200prginsadd(
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_prginsadd_r_ii)) {
    Build_OP( TOP_prginsadd_r_ii,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_prginsadd_r_ii, ops);
  }
} /* Expand___st200prginsadd */

/* Expansion of __st200prginsadd_l1 from MDS */
static void
Expand___st200prginsadd_l1(
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_prginsadd_l1_r_ii)) {
    Build_OP( TOP_prginsadd_l1_r_ii,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_prginsadd_l1_r_ii, ops);
  }
} /* Expand___st200prginsadd_l1 */

/* Expansion of __st200prginsset_l1 from MDS */
static void
Expand___st200prginsset_l1(
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_prginsset_l1_r_ii)) {
    Build_OP( TOP_prginsset_l1_r_ii,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_prginsset_l1_r_ii, ops);
  }
} /* Expand___st200prginsset_l1 */

/* Expansion of __st200pswclr from MDS */
static void
Expand___st200pswclr(
 TN *i0,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_pswclr_r)) {
    Build_OP( TOP_pswclr_r,	i0,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_pswclr_r, ops);
  }
} /* Expand___st200pswclr */

/* Expansion of __st200pswmask from MDS */
static void
Expand___st200pswmask(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_pswmask_ii_r_r)) {
    Build_OP( TOP_pswmask_ii_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_pswmask_ii_r_r, ops);
  }
} /* Expand___st200pswmask */

/* Expansion of __st200pswset from MDS */
static void
Expand___st200pswset(
 TN *i0,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_pswset_r)) {
    Build_OP( TOP_pswset_r,	i0,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_pswset_r, ops);
  }
} /* Expand___st200pswset */

/* Expansion of __st200retention from MDS */
static void
Expand___st200retention(
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_retention)) {
    Build_OP( TOP_retention,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_retention, ops);
  }
} /* Expand___st200retention */

/* Expansion of __st200sadu_pb from MDS */
static void
Expand___st200sadu_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_sadu_pb_r_r_r)) {
    Build_OP( TOP_sadu_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_sadu_pb_r_r_r, ops);
  }
} /* Expand___st200sadu_pb */

/* Expansion of __st200satso from MDS */
static void
Expand___st200satso(
 TN *o0,
 TN *i0,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_satso_r_r)) {
    Build_OP( TOP_satso_r_r,	o0,	i0,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_satso_r_r, ops);
  }
} /* Expand___st200satso */

/* Expansion of __st200sh1addso from MDS */
static void
Expand___st200sh1addso(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_sh1addso_r_r_r)) {
    Build_OP( TOP_sh1addso_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_sh1addso_r_r_r, ops);
  }
} /* Expand___st200sh1addso */

/* Expansion of __st200sh1subso from MDS */
static void
Expand___st200sh1subso(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_sh1subso_r_r_r)) {
    Build_OP( TOP_sh1subso_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_sh1subso_r_r_r, ops);
  }
} /* Expand___st200sh1subso */

/* Expansion of __st200shlso from MDS */
static void
Expand___st200shlso(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shlso_r_r_r)) {
    Build_OP( TOP_shlso_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shlso_r_r_r, ops);
  }
} /* Expand___st200shlso */

/* Expansion of __st200shls from MDS */
static void
Expand___st200shls(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shls_r_r_r)) {
    Build_OP( TOP_shls_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shls_r_r_r, ops);
  }
} /* Expand___st200shls */

/* Expansion of __st200shls_ph from MDS */
static void
Expand___st200shls_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shls_ph_r_r_r)) {
    Build_OP( TOP_shls_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shls_ph_r_r_r, ops);
  }
} /* Expand___st200shls_ph */

/* Expansion of __st200shl_ph from MDS */
static void
Expand___st200shl_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shl_ph_r_r_r)) {
    Build_OP( TOP_shl_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shl_ph_r_r_r, ops);
  }
} /* Expand___st200shl_ph */

/* Expansion of __st200shrrne_ph from MDS */
static void
Expand___st200shrrne_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shrrne_ph_r_r_r)) {
    Build_OP( TOP_shrrne_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shrrne_ph_r_r_r, ops);
  }
} /* Expand___st200shrrne_ph */

/* Expansion of __st200shrrnp from MDS */
static void
Expand___st200shrrnp(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shrrnp_ii_r_r)) {
    Build_OP( TOP_shrrnp_ii_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shrrnp_ii_r_r, ops);
  }
} /* Expand___st200shrrnp */

/* Expansion of __st200shrrnp_ph from MDS */
static void
Expand___st200shrrnp_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shrrnp_ph_r_r_r)) {
    Build_OP( TOP_shrrnp_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shrrnp_ph_r_r_r, ops);
  }
} /* Expand___st200shrrnp_ph */

/* Expansion of __st200shr_ph from MDS */
static void
Expand___st200shr_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shr_ph_r_r_r)) {
    Build_OP( TOP_shr_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shr_ph_r_r_r, ops);
  }
} /* Expand___st200shr_ph */

/* Expansion of __st200shuffeve_pb from MDS */
static void
Expand___st200shuffeve_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shuffeve_pb_r_r_r)) {
    Build_OP( TOP_shuffeve_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shuffeve_pb_r_r_r, ops);
  }
} /* Expand___st200shuffeve_pb */

/* Expansion of __st200shuffodd_pb from MDS */
static void
Expand___st200shuffodd_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shuffodd_pb_r_r_r)) {
    Build_OP( TOP_shuffodd_pb_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shuffodd_pb_r_r_r, ops);
  }
} /* Expand___st200shuffodd_pb */

/* Expansion of __st200shuff_pbh from MDS */
static void
Expand___st200shuff_pbh(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shuff_pbh_r_r_r)) {
    Build_OP( TOP_shuff_pbh_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shuff_pbh_r_r_r, ops);
  }
} /* Expand___st200shuff_pbh */

/* Expansion of __st200shuff_pbl from MDS */
static void
Expand___st200shuff_pbl(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shuff_pbl_r_r_r)) {
    Build_OP( TOP_shuff_pbl_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shuff_pbl_r_r_r, ops);
  }
} /* Expand___st200shuff_pbl */

/* Expansion of __st200shuff_phh from MDS */
static void
Expand___st200shuff_phh(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shuff_phh_r_r_r)) {
    Build_OP( TOP_shuff_phh_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shuff_phh_r_r_r, ops);
  }
} /* Expand___st200shuff_phh */

/* Expansion of __st200shuff_phl from MDS */
static void
Expand___st200shuff_phl(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_shuff_phl_r_r_r)) {
    Build_OP( TOP_shuff_phl_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_shuff_phl_r_r_r, ops);
  }
} /* Expand___st200shuff_phl */

/* Expansion of __st200slct_pb from MDS */
static void
Expand___st200slct_pb(
 TN *o0,
 TN *i0,
 TN *i1,
 TN *i2,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_slct_pb_r_r_b_r)) {
    Build_OP( TOP_slct_pb_r_r_b_r,	o0,	i0,	i1,	i2,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_slct_pb_r_r_b_r, ops);
  }
} /* Expand___st200slct_pb */

/* Expansion of __st200subso from MDS */
static void
Expand___st200subso(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_subso_r_r_r)) {
    Build_OP( TOP_subso_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_subso_r_r_r, ops);
  }
} /* Expand___st200subso */

/* Expansion of __st200subs_ph from MDS */
static void
Expand___st200subs_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_subs_ph_r_r_r)) {
    Build_OP( TOP_subs_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_subs_ph_r_r_r, ops);
  }
} /* Expand___st200subs_ph */

/* Expansion of __st200sub_ph from MDS */
static void
Expand___st200sub_ph(
 TN *o0,
 TN *i0,
 TN *i1,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_sub_ph_r_r_r)) {
    Build_OP( TOP_sub_ph_r_r_r,	o0,	i0,	i1,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_sub_ph_r_r_r, ops);
  }
} /* Expand___st200sub_ph */

/* Expansion of __st220sync from MDS */
static void
Expand___st220sync(
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_sync)) {
    Build_OP( TOP_sync,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_sync, ops);
  }
} /* Expand___st220sync */

/* Expansion of __st220syncins from MDS */
static void
Expand___st220syncins(
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_syncins)) {
    Build_OP( TOP_syncins,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_syncins, ops);
  }
} /* Expand___st220syncins */

/* Expansion of __st200unpacku_pbh from MDS */
static void
Expand___st200unpacku_pbh(
 TN *o0,
 TN *i0,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_unpacku_pbh_r_r)) {
    Build_OP( TOP_unpacku_pbh_r_r,	o0,	i0,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_unpacku_pbh_r_r, ops);
  }
} /* Expand___st200unpacku_pbh */

/* Expansion of __st200unpacku_pbl from MDS */
static void
Expand___st200unpacku_pbl(
 TN *o0,
 TN *i0,
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_unpacku_pbl_r_r)) {
    Build_OP( TOP_unpacku_pbl_r_r,	o0,	i0,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_unpacku_pbl_r_r, ops);
  }
} /* Expand___st200unpacku_pbl */

/* Expansion of __st200waitl from MDS */
static void
Expand___st200waitl(
 OPS *ops
)
{
  if (ISA_SUBSET_Member (ISA_SUBSET_Value, TOP_waitl)) {
    Build_OP( TOP_waitl,	ops);
  } else {
    Expand_Unimplemented_Intrinsic ( TOP_waitl, ops);
  }
} /* Expand___st200waitl */

#endif /* INTRN_EXPAND */

#ifdef INTRN_SWITCH

    case INTRN___ST200ABSS_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 1,("INTRN___ST200ABSS_PH: there is %d operands instead of 1\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200ABSS_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200abss_ph(res_0 == NULL ? result[0] : res_0,opnd[0],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200ABSUBU_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200ABSUBU_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200ABSUBU_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200absubu_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200ADDSO: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200ADDSO: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200ADDSO: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200addso(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200ADDS_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200ADDS_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200ADDS_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200adds_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200ADD_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200ADD_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200ADD_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200add_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200AVG4U_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 3,("INTRN___ST200AVG4U_PB: there is %d operands instead of 3\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200AVG4U_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_branch, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        opnd[2] = Check_Opd_RClass(opnd[2], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200avg4u_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],opnd[2],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200AVGU_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 3,("INTRN___ST200AVGU_PB: there is %d operands instead of 3\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200AVGU_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_branch, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        opnd[2] = Check_Opd_RClass(opnd[2], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200avgu_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],opnd[2],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200CMPEQ_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200CMPEQ_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200CMPEQ_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200cmpeq_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200CMPEQ_PB_BDEST: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_branch) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200CMPEQ_PB_BDEST: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200CMPEQ_PB_BDEST: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200cmpeq_pb_bdest(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200CMPEQ_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200CMPEQ_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200CMPEQ_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200cmpeq_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200CMPEQ_PH_BDEST: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_branch) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200CMPEQ_PH_BDEST: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200CMPEQ_PH_BDEST: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200cmpeq_ph_bdest(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200CMPGTU_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200CMPGTU_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200CMPGTU_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200cmpgtu_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200CMPGTU_PB_BDEST: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_branch) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200CMPGTU_PB_BDEST: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200CMPGTU_PB_BDEST: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200cmpgtu_pb_bdest(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200CMPGT_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200CMPGT_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200CMPGT_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200cmpgt_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200CMPGT_PH_BDEST: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_branch) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200CMPGT_PH_BDEST: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200CMPGT_PH_BDEST: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200cmpgt_ph_bdest(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200EXT1_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200EXT1_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200EXT1_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200ext1_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200EXT2_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200EXT2_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200EXT2_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200ext2_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200EXT3_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200EXT3_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200EXT3_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200ext3_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200EXTL_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 3,("INTRN___ST200EXTL_PB: there is %d operands instead of 3\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200EXTL_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_branch, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        opnd[2] = Check_Opd_RClass(opnd[2], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200extl_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],opnd[2],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200EXTR_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 3,("INTRN___ST200EXTR_PB: there is %d operands instead of 3\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200EXTR_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_branch, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        opnd[2] = Check_Opd_RClass(opnd[2], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200extr_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],opnd[2],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200FLUSHADD: {
        FmtAssert(num_opnds == 2,("INTRN___ST200FLUSHADD: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200FLUSHADD: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_Literal(opnd[0], LC_xsrc2, 0, "__builtin___st200flushadd", srcpos);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200flushadd(opnd[0],opnd[1],ops);
        break;
      }
    case INTRN___ST200FLUSHADD_L1: {
        FmtAssert(num_opnds == 2,("INTRN___ST200FLUSHADD_L1: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200FLUSHADD_L1: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_Literal(opnd[0], LC_xsrc2, 0, "__builtin___st200flushadd_l1", srcpos);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200flushadd_l1(opnd[0],opnd[1],ops);
        break;
      }
    case INTRN___ST200INVADD: {
        FmtAssert(num_opnds == 2,("INTRN___ST200INVADD: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200INVADD: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_Literal(opnd[0], LC_xsrc2, 0, "__builtin___st200invadd", srcpos);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200invadd(opnd[0],opnd[1],ops);
        break;
      }
    case INTRN___ST200INVADD_L1: {
        FmtAssert(num_opnds == 2,("INTRN___ST200INVADD_L1: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200INVADD_L1: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_Literal(opnd[0], LC_xsrc2, 0, "__builtin___st200invadd_l1", srcpos);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200invadd_l1(opnd[0],opnd[1],ops);
        break;
      }
    case INTRN___MAXUW: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___MAXUW: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___MAXUW: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___maxuw(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___MAXW: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___MAXW: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___MAXW: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___maxw(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200MAX_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200MAX_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200MAX_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200max_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___MINUW: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___MINUW: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___MINUW: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___minuw(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___MINW: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___MINW: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___MINW: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___minw(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200MIN_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200MIN_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200MIN_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200min_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200MULADDUS_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200MULADDUS_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200MULADDUS_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200muladdus_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200MULADD_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200MULADD_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200MULADD_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200muladd_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200MULFRACADDS_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200MULFRACADDS_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200MULFRACADDS_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200mulfracadds_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200MULFRACRM_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200MULFRACRM_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200MULFRACRM_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200mulfracrm_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200MULFRACRNE_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200MULFRACRNE_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200MULFRACRNE_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200mulfracrne_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200MUL_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200MUL_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200MUL_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200mul_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200PACKRNP_PHH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200PACKRNP_PHH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200PACKRNP_PHH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200packrnp_phh(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200PACKSU_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200PACKSU_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200PACKSU_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200packsu_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200PACKS_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200PACKS_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200PACKS_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200packs_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200PACK_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200PACK_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200PACK_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200pack_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200PACK_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200PACK_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200PACK_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200pack_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200PERM_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200PERM_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200PERM_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200perm_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST220PRGADD_L1: {
        FmtAssert(num_opnds == 2,("INTRN___ST220PRGADD_L1: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST220PRGADD_L1: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_Literal(opnd[0], LC_xsrc2, 0, "__builtin___st220prgadd_l1", srcpos);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st220prgadd_l1(opnd[0],opnd[1],ops);
        break;
      }
    case INTRN___ST220PRGINS: {
        FmtAssert(num_opnds == 0,("INTRN___ST220PRGINS: there is %d operands instead of 0\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST220PRGINS: there is %d results instead of 0\n",num_results));
        Expand___st220prgins(ops);
        break;
      }
    case INTRN___ST200PRGINSADD: {
        FmtAssert(num_opnds == 2,("INTRN___ST200PRGINSADD: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200PRGINSADD: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_Literal(opnd[0], LC_xsrc2, 0, "__builtin___st200prginsadd", srcpos);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200prginsadd(opnd[0],opnd[1],ops);
        break;
      }
    case INTRN___ST200PRGINSADD_L1: {
        FmtAssert(num_opnds == 2,("INTRN___ST200PRGINSADD_L1: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200PRGINSADD_L1: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_Literal(opnd[0], LC_xsrc2, 0, "__builtin___st200prginsadd_l1", srcpos);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200prginsadd_l1(opnd[0],opnd[1],ops);
        break;
      }
    case INTRN___ST200PRGINSSET_L1: {
        FmtAssert(num_opnds == 2,("INTRN___ST200PRGINSSET_L1: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200PRGINSSET_L1: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_Literal(opnd[0], LC_xsrc2, 0, "__builtin___st200prginsset_l1", srcpos);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200prginsset_l1(opnd[0],opnd[1],ops);
        break;
      }
    case INTRN___ST200PSWCLR: {
        FmtAssert(num_opnds == 1,("INTRN___ST200PSWCLR: there is %d operands instead of 1\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200PSWCLR: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200pswclr(opnd[0],ops);
        break;
      }
    case INTRN___ST200PSWMASK: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200PSWMASK: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200PSWMASK: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_Literal(opnd[1], LC_xsrc2, 1, "__builtin___st200pswmask", srcpos);
        Expand___st200pswmask(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200PSWSET: {
        FmtAssert(num_opnds == 1,("INTRN___ST200PSWSET: there is %d operands instead of 1\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200PSWSET: there is %d results instead of 0\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200pswset(opnd[0],ops);
        break;
      }
    case INTRN___ST200RETENTION: {
        FmtAssert(num_opnds == 0,("INTRN___ST200RETENTION: there is %d operands instead of 0\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200RETENTION: there is %d results instead of 0\n",num_results));
        Expand___st200retention(ops);
        break;
      }
    case INTRN___ST200SADU_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SADU_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SADU_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200sadu_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SATSO: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 1,("INTRN___ST200SATSO: there is %d operands instead of 1\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SATSO: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200satso(res_0 == NULL ? result[0] : res_0,opnd[0],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SH1ADDSO: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SH1ADDSO: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SH1ADDSO: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200sh1addso(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SH1SUBSO: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SH1SUBSO: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SH1SUBSO: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200sh1subso(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHLSO: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHLSO: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHLSO: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shlso(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHLS: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHLS: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHLS: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shls(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHLS_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHLS_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHLS_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shls_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHL_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHL_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHL_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shl_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHRRNE_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHRRNE_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHRRNE_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shrrne_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHRRNP: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHRRNP: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHRRNP: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_Literal(opnd[1], LC_xsrc2, 1, "__builtin___st200shrrnp", srcpos);
        Expand___st200shrrnp(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHRRNP_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHRRNP_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHRRNP_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shrrnp_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHR_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHR_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHR_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shr_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHUFFEVE_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHUFFEVE_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHUFFEVE_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shuffeve_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHUFFODD_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHUFFODD_PB: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHUFFODD_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shuffodd_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHUFF_PBH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHUFF_PBH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHUFF_PBH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shuff_pbh(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHUFF_PBL: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHUFF_PBL: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHUFF_PBL: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shuff_pbl(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHUFF_PHH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHUFF_PHH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHUFF_PHH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shuff_phh(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SHUFF_PHL: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SHUFF_PHL: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SHUFF_PHL: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200shuff_phl(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SLCT_PB: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 3,("INTRN___ST200SLCT_PB: there is %d operands instead of 3\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SLCT_PB: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_branch, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        opnd[2] = Check_Opd_RClass(opnd[2], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200slct_pb(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],opnd[2],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SUBSO: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SUBSO: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SUBSO: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200subso(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SUBS_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SUBS_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SUBS_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200subs_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200SUB_PH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 2,("INTRN___ST200SUB_PH: there is %d operands instead of 2\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200SUB_PH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        opnd[1] = Check_Opd_RClass(opnd[1], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200sub_ph(res_0 == NULL ? result[0] : res_0,opnd[0],opnd[1],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST220SYNC: {
        FmtAssert(num_opnds == 0,("INTRN___ST220SYNC: there is %d operands instead of 0\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST220SYNC: there is %d results instead of 0\n",num_results));
        Expand___st220sync(ops);
        break;
      }
    case INTRN___ST220SYNCINS: {
        FmtAssert(num_opnds == 0,("INTRN___ST220SYNCINS: there is %d operands instead of 0\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST220SYNCINS: there is %d results instead of 0\n",num_results));
        Expand___st220syncins(ops);
        break;
      }
    case INTRN___ST200UNPACKU_PBH: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 1,("INTRN___ST200UNPACKU_PBH: there is %d operands instead of 1\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200UNPACKU_PBH: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200unpacku_pbh(res_0 == NULL ? result[0] : res_0,opnd[0],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200UNPACKU_PBL: {
        TN *res_0 = (TN_register_class(result[0]) != ISA_REGISTER_CLASS_integer) ?
                           Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) : NULL;
        FmtAssert(num_opnds == 1,("INTRN___ST200UNPACKU_PBL: there is %d operands instead of 1\n",num_opnds));
        FmtAssert(num_results == 1,("INTRN___ST200UNPACKU_PBL: there is %d results instead of 1\n",num_results));
        opnd[0] = Check_Opd_RClass(opnd[0], ISA_REGISTER_CLASS_integer, ops);
        Expand___st200unpacku_pbl(res_0 == NULL ? result[0] : res_0,opnd[0],ops);
        if(res_0 != NULL) {
          Expand_Copy(result[0],True_TN,res_0,ops);
        }
        break;
      }
    case INTRN___ST200WAITL: {
        FmtAssert(num_opnds == 0,("INTRN___ST200WAITL: there is %d operands instead of 0\n",num_opnds));
        FmtAssert(num_results == 0,("INTRN___ST200WAITL: there is %d results instead of 0\n",num_results));
        Expand___st200waitl(ops);
        break;
      }
#endif /* INTRN_SWITCH */

#ifdef INTRN_DEFINES

INTRN___ST200ABSS_PH	=	INTRINSIC_GENERIC_LAST+1,
INTRN___ST200ABSUBU_PB	=	INTRINSIC_GENERIC_LAST+2,
INTRN___ST200ADDSO	=	INTRINSIC_GENERIC_LAST+3,
INTRN___ST200ADDS_PH	=	INTRINSIC_GENERIC_LAST+4,
INTRN___ST200ADD_PH	=	INTRINSIC_GENERIC_LAST+5,
INTRN___ST200AVG4U_PB	=	INTRINSIC_GENERIC_LAST+6,
INTRN___ST200AVGU_PB	=	INTRINSIC_GENERIC_LAST+7,
INTRN___ST200CMPEQ_PB	=	INTRINSIC_GENERIC_LAST+8,
INTRN___ST200CMPEQ_PB_BDEST	=	INTRINSIC_GENERIC_LAST+9,
INTRN___ST200CMPEQ_PH	=	INTRINSIC_GENERIC_LAST+10,
INTRN___ST200CMPEQ_PH_BDEST	=	INTRINSIC_GENERIC_LAST+11,
INTRN___ST200CMPGTU_PB	=	INTRINSIC_GENERIC_LAST+12,
INTRN___ST200CMPGTU_PB_BDEST	=	INTRINSIC_GENERIC_LAST+13,
INTRN___ST200CMPGT_PH	=	INTRINSIC_GENERIC_LAST+14,
INTRN___ST200CMPGT_PH_BDEST	=	INTRINSIC_GENERIC_LAST+15,
INTRN___ST200EXT1_PB	=	INTRINSIC_GENERIC_LAST+16,
INTRN___ST200EXT2_PB	=	INTRINSIC_GENERIC_LAST+17,
INTRN___ST200EXT3_PB	=	INTRINSIC_GENERIC_LAST+18,
INTRN___ST200EXTL_PB	=	INTRINSIC_GENERIC_LAST+19,
INTRN___ST200EXTR_PB	=	INTRINSIC_GENERIC_LAST+20,
INTRN___ST200FLUSHADD	=	INTRINSIC_GENERIC_LAST+21,
INTRN___ST200FLUSHADD_L1	=	INTRINSIC_GENERIC_LAST+22,
INTRN___ST200INVADD	=	INTRINSIC_GENERIC_LAST+23,
INTRN___ST200INVADD_L1	=	INTRINSIC_GENERIC_LAST+24,
INTRN___MAXUW	=	INTRINSIC_GENERIC_LAST+25,
INTRN___MAXW	=	INTRINSIC_GENERIC_LAST+26,
INTRN___ST200MAX_PH	=	INTRINSIC_GENERIC_LAST+27,
INTRN___MINUW	=	INTRINSIC_GENERIC_LAST+28,
INTRN___MINW	=	INTRINSIC_GENERIC_LAST+29,
INTRN___ST200MIN_PH	=	INTRINSIC_GENERIC_LAST+30,
INTRN___ST200MULADDUS_PB	=	INTRINSIC_GENERIC_LAST+31,
INTRN___ST200MULADD_PH	=	INTRINSIC_GENERIC_LAST+32,
INTRN___ST200MULFRACADDS_PH	=	INTRINSIC_GENERIC_LAST+33,
INTRN___ST200MULFRACRM_PH	=	INTRINSIC_GENERIC_LAST+34,
INTRN___ST200MULFRACRNE_PH	=	INTRINSIC_GENERIC_LAST+35,
INTRN___ST200MUL_PH	=	INTRINSIC_GENERIC_LAST+36,
INTRN___ST200PACKRNP_PHH	=	INTRINSIC_GENERIC_LAST+37,
INTRN___ST200PACKSU_PB	=	INTRINSIC_GENERIC_LAST+38,
INTRN___ST200PACKS_PH	=	INTRINSIC_GENERIC_LAST+39,
INTRN___ST200PACK_PB	=	INTRINSIC_GENERIC_LAST+40,
INTRN___ST200PACK_PH	=	INTRINSIC_GENERIC_LAST+41,
INTRN___ST200PERM_PB	=	INTRINSIC_GENERIC_LAST+42,
INTRN___ST220PRGADD_L1	=	INTRINSIC_GENERIC_LAST+43,
INTRN___ST220PRGINS	=	INTRINSIC_GENERIC_LAST+44,
INTRN___ST200PRGINSADD	=	INTRINSIC_GENERIC_LAST+45,
INTRN___ST200PRGINSADD_L1	=	INTRINSIC_GENERIC_LAST+46,
INTRN___ST200PRGINSSET_L1	=	INTRINSIC_GENERIC_LAST+47,
INTRN___ST200PSWCLR	=	INTRINSIC_GENERIC_LAST+48,
INTRN___ST200PSWMASK	=	INTRINSIC_GENERIC_LAST+49,
INTRN___ST200PSWSET	=	INTRINSIC_GENERIC_LAST+50,
INTRN___ST200RETENTION	=	INTRINSIC_GENERIC_LAST+51,
INTRN___ST200SADU_PB	=	INTRINSIC_GENERIC_LAST+52,
INTRN___ST200SATSO	=	INTRINSIC_GENERIC_LAST+53,
INTRN___ST200SH1ADDSO	=	INTRINSIC_GENERIC_LAST+54,
INTRN___ST200SH1SUBSO	=	INTRINSIC_GENERIC_LAST+55,
INTRN___ST200SHLSO	=	INTRINSIC_GENERIC_LAST+56,
INTRN___ST200SHLS	=	INTRINSIC_GENERIC_LAST+57,
INTRN___ST200SHLS_PH	=	INTRINSIC_GENERIC_LAST+58,
INTRN___ST200SHL_PH	=	INTRINSIC_GENERIC_LAST+59,
INTRN___ST200SHRRNE_PH	=	INTRINSIC_GENERIC_LAST+60,
INTRN___ST200SHRRNP	=	INTRINSIC_GENERIC_LAST+61,
INTRN___ST200SHRRNP_PH	=	INTRINSIC_GENERIC_LAST+62,
INTRN___ST200SHR_PH	=	INTRINSIC_GENERIC_LAST+63,
INTRN___ST200SHUFFEVE_PB	=	INTRINSIC_GENERIC_LAST+64,
INTRN___ST200SHUFFODD_PB	=	INTRINSIC_GENERIC_LAST+65,
INTRN___ST200SHUFF_PBH	=	INTRINSIC_GENERIC_LAST+66,
INTRN___ST200SHUFF_PBL	=	INTRINSIC_GENERIC_LAST+67,
INTRN___ST200SHUFF_PHH	=	INTRINSIC_GENERIC_LAST+68,
INTRN___ST200SHUFF_PHL	=	INTRINSIC_GENERIC_LAST+69,
INTRN___ST200SLCT_PB	=	INTRINSIC_GENERIC_LAST+70,
INTRN___ST200SUBSO	=	INTRINSIC_GENERIC_LAST+71,
INTRN___ST200SUBS_PH	=	INTRINSIC_GENERIC_LAST+72,
INTRN___ST200SUB_PH	=	INTRINSIC_GENERIC_LAST+73,
INTRN___ST220SYNC	=	INTRINSIC_GENERIC_LAST+74,
INTRN___ST220SYNCINS	=	INTRINSIC_GENERIC_LAST+75,
INTRN___ST200UNPACKU_PBH	=	INTRINSIC_GENERIC_LAST+76,
INTRN___ST200UNPACKU_PBL	=	INTRINSIC_GENERIC_LAST+77,
INTRN___ST200WAITL	=	INTRINSIC_GENERIC_LAST+78,
INTRINSIC_TARG_LAST	=	INTRINSIC_GENERIC_LAST+78,
#endif /* INTRN_DEFINES */

#ifdef INTRN_INFO

{ /* __st200abss_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200abss_ph", NULL, "__st200abss_ph"},
{ /* __st200absubu_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200absubu_pb", NULL, "__st200absubu_pb"},
{ /* __st200addso */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200addso", NULL, "__st200addso"},
{ /* __st200adds_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200adds_ph", NULL, "__st200adds_ph"},
{ /* __st200add_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200add_ph", NULL, "__st200add_ph"},
{ /* __st200avg4u_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200avg4u_pb", NULL, "__st200avg4u_pb"},
{ /* __st200avgu_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200avgu_pb", NULL, "__st200avgu_pb"},
{ /* __st200cmpeq_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200cmpeq_pb", NULL, "__st200cmpeq_pb"},
{ /* __st200cmpeq_pb_bdest */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_U4, "__st200cmpeq_pb_bdest", NULL, "__st200cmpeq_pb_bdest"},
{ /* __st200cmpeq_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200cmpeq_ph", NULL, "__st200cmpeq_ph"},
{ /* __st200cmpeq_ph_bdest */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_U4, "__st200cmpeq_ph_bdest", NULL, "__st200cmpeq_ph_bdest"},
{ /* __st200cmpgtu_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200cmpgtu_pb", NULL, "__st200cmpgtu_pb"},
{ /* __st200cmpgtu_pb_bdest */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_U4, "__st200cmpgtu_pb_bdest", NULL, "__st200cmpgtu_pb_bdest"},
{ /* __st200cmpgt_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200cmpgt_ph", NULL, "__st200cmpgt_ph"},
{ /* __st200cmpgt_ph_bdest */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_U4, "__st200cmpgt_ph_bdest", NULL, "__st200cmpgt_ph_bdest"},
{ /* __st200ext1_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200ext1_pb", NULL, "__st200ext1_pb"},
{ /* __st200ext2_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200ext2_pb", NULL, "__st200ext2_pb"},
{ /* __st200ext3_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200ext3_pb", NULL, "__st200ext3_pb"},
{ /* __st200extl_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200extl_pb", NULL, "__st200extl_pb"},
{ /* __st200extr_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200extr_pb", NULL, "__st200extr_pb"},
{ /* __st200flushadd */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200flushadd", NULL, "__st200flushadd"},
{ /* __st200flushadd_l1 */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200flushadd_l1", NULL, "__st200flushadd_l1"},
{ /* __st200invadd */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200invadd", NULL, "__st200invadd"},
{ /* __st200invadd_l1 */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200invadd_l1", NULL, "__st200invadd_l1"},
{ /* __maxuw */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_U4, "__maxuw", NULL, "__maxuw"},
{ /* __maxw */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__maxw", NULL, "__maxw"},
{ /* __st200max_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200max_ph", NULL, "__st200max_ph"},
{ /* __minuw */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_U4, "__minuw", NULL, "__minuw"},
{ /* __minw */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__minw", NULL, "__minw"},
{ /* __st200min_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200min_ph", NULL, "__st200min_ph"},
{ /* __st200muladdus_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200muladdus_pb", NULL, "__st200muladdus_pb"},
{ /* __st200muladd_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200muladd_ph", NULL, "__st200muladd_ph"},
{ /* __st200mulfracadds_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200mulfracadds_ph", NULL, "__st200mulfracadds_ph"},
{ /* __st200mulfracrm_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200mulfracrm_ph", NULL, "__st200mulfracrm_ph"},
{ /* __st200mulfracrne_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200mulfracrne_ph", NULL, "__st200mulfracrne_ph"},
{ /* __st200mul_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200mul_ph", NULL, "__st200mul_ph"},
{ /* __st200packrnp_phh */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200packrnp_phh", NULL, "__st200packrnp_phh"},
{ /* __st200packsu_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200packsu_pb", NULL, "__st200packsu_pb"},
{ /* __st200packs_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200packs_ph", NULL, "__st200packs_ph"},
{ /* __st200pack_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200pack_pb", NULL, "__st200pack_pb"},
{ /* __st200pack_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200pack_ph", NULL, "__st200pack_ph"},
{ /* __st200perm_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200perm_pb", NULL, "__st200perm_pb"},
{ /* __st220prgadd_l1 */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st220prgadd_l1", NULL, "__st220prgadd_l1"},
{ /* __st220prgins */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st220prgins", NULL, "__st220prgins"},
{ /* __st200prginsadd */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200prginsadd", NULL, "__st200prginsadd"},
{ /* __st200prginsadd_l1 */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200prginsadd_l1", NULL, "__st200prginsadd_l1"},
{ /* __st200prginsset_l1 */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200prginsset_l1", NULL, "__st200prginsset_l1"},
{ /* __st200pswclr */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200pswclr", NULL, "__st200pswclr"},
{ /* __st200pswmask */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200pswmask", NULL, "__st200pswmask"},
{ /* __st200pswset */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200pswset", NULL, "__st200pswset"},
{ /* __st200retention */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200retention", NULL, "__st200retention"},
{ /* __st200sadu_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200sadu_pb", NULL, "__st200sadu_pb"},
{ /* __st200satso */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200satso", NULL, "__st200satso"},
{ /* __st200sh1addso */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200sh1addso", NULL, "__st200sh1addso"},
{ /* __st200sh1subso */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200sh1subso", NULL, "__st200sh1subso"},
{ /* __st200shlso */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shlso", NULL, "__st200shlso"},
{ /* __st200shls */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shls", NULL, "__st200shls"},
{ /* __st200shls_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shls_ph", NULL, "__st200shls_ph"},
{ /* __st200shl_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shl_ph", NULL, "__st200shl_ph"},
{ /* __st200shrrne_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shrrne_ph", NULL, "__st200shrrne_ph"},
{ /* __st200shrrnp */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shrrnp", NULL, "__st200shrrnp"},
{ /* __st200shrrnp_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shrrnp_ph", NULL, "__st200shrrnp_ph"},
{ /* __st200shr_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shr_ph", NULL, "__st200shr_ph"},
{ /* __st200shuffeve_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shuffeve_pb", NULL, "__st200shuffeve_pb"},
{ /* __st200shuffodd_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shuffodd_pb", NULL, "__st200shuffodd_pb"},
{ /* __st200shuff_pbh */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shuff_pbh", NULL, "__st200shuff_pbh"},
{ /* __st200shuff_pbl */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shuff_pbl", NULL, "__st200shuff_pbl"},
{ /* __st200shuff_phh */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shuff_phh", NULL, "__st200shuff_phh"},
{ /* __st200shuff_phl */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200shuff_phl", NULL, "__st200shuff_phl"},
{ /* __st200slct_pb */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200slct_pb", NULL, "__st200slct_pb"},
{ /* __st200subso */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200subso", NULL, "__st200subso"},
{ /* __st200subs_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200subs_ph", NULL, "__st200subs_ph"},
{ /* __st200sub_ph */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200sub_ph", NULL, "__st200sub_ph"},
{ /* __st220sync */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st220sync", NULL, "__st220sync"},
{ /* __st220syncins */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st220syncins", NULL, "__st220syncins"},
{ /* __st200unpacku_pbh */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200unpacku_pbh", NULL, "__st200unpacku_pbh"},
{ /* __st200unpacku_pbl */
  BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_I4, "__st200unpacku_pbl", NULL, "__st200unpacku_pbl"},
{ /* __st200waitl */
  BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,
  IRETURN_V, "__st200waitl", NULL, "__st200waitl"},
#endif /* INTRN_INFO */

#ifdef INTRN_WUTIL

INTRN___ST200ABSS_PH,	"__ST200ABSS_PH",
INTRN___ST200ABSUBU_PB,	"__ST200ABSUBU_PB",
INTRN___ST200ADDSO,	"__ST200ADDSO",
INTRN___ST200ADDS_PH,	"__ST200ADDS_PH",
INTRN___ST200ADD_PH,	"__ST200ADD_PH",
INTRN___ST200AVG4U_PB,	"__ST200AVG4U_PB",
INTRN___ST200AVGU_PB,	"__ST200AVGU_PB",
INTRN___ST200CMPEQ_PB,	"__ST200CMPEQ_PB",
INTRN___ST200CMPEQ_PB_BDEST,	"__ST200CMPEQ_PB_BDEST",
INTRN___ST200CMPEQ_PH,	"__ST200CMPEQ_PH",
INTRN___ST200CMPEQ_PH_BDEST,	"__ST200CMPEQ_PH_BDEST",
INTRN___ST200CMPGTU_PB,	"__ST200CMPGTU_PB",
INTRN___ST200CMPGTU_PB_BDEST,	"__ST200CMPGTU_PB_BDEST",
INTRN___ST200CMPGT_PH,	"__ST200CMPGT_PH",
INTRN___ST200CMPGT_PH_BDEST,	"__ST200CMPGT_PH_BDEST",
INTRN___ST200EXT1_PB,	"__ST200EXT1_PB",
INTRN___ST200EXT2_PB,	"__ST200EXT2_PB",
INTRN___ST200EXT3_PB,	"__ST200EXT3_PB",
INTRN___ST200EXTL_PB,	"__ST200EXTL_PB",
INTRN___ST200EXTR_PB,	"__ST200EXTR_PB",
INTRN___ST200FLUSHADD,	"__ST200FLUSHADD",
INTRN___ST200FLUSHADD_L1,	"__ST200FLUSHADD_L1",
INTRN___ST200INVADD,	"__ST200INVADD",
INTRN___ST200INVADD_L1,	"__ST200INVADD_L1",
INTRN___MAXUW,	"__MAXUW",
INTRN___MAXW,	"__MAXW",
INTRN___ST200MAX_PH,	"__ST200MAX_PH",
INTRN___MINUW,	"__MINUW",
INTRN___MINW,	"__MINW",
INTRN___ST200MIN_PH,	"__ST200MIN_PH",
INTRN___ST200MULADDUS_PB,	"__ST200MULADDUS_PB",
INTRN___ST200MULADD_PH,	"__ST200MULADD_PH",
INTRN___ST200MULFRACADDS_PH,	"__ST200MULFRACADDS_PH",
INTRN___ST200MULFRACRM_PH,	"__ST200MULFRACRM_PH",
INTRN___ST200MULFRACRNE_PH,	"__ST200MULFRACRNE_PH",
INTRN___ST200MUL_PH,	"__ST200MUL_PH",
INTRN___ST200PACKRNP_PHH,	"__ST200PACKRNP_PHH",
INTRN___ST200PACKSU_PB,	"__ST200PACKSU_PB",
INTRN___ST200PACKS_PH,	"__ST200PACKS_PH",
INTRN___ST200PACK_PB,	"__ST200PACK_PB",
INTRN___ST200PACK_PH,	"__ST200PACK_PH",
INTRN___ST200PERM_PB,	"__ST200PERM_PB",
INTRN___ST220PRGADD_L1,	"__ST220PRGADD_L1",
INTRN___ST220PRGINS,	"__ST220PRGINS",
INTRN___ST200PRGINSADD,	"__ST200PRGINSADD",
INTRN___ST200PRGINSADD_L1,	"__ST200PRGINSADD_L1",
INTRN___ST200PRGINSSET_L1,	"__ST200PRGINSSET_L1",
INTRN___ST200PSWCLR,	"__ST200PSWCLR",
INTRN___ST200PSWMASK,	"__ST200PSWMASK",
INTRN___ST200PSWSET,	"__ST200PSWSET",
INTRN___ST200RETENTION,	"__ST200RETENTION",
INTRN___ST200SADU_PB,	"__ST200SADU_PB",
INTRN___ST200SATSO,	"__ST200SATSO",
INTRN___ST200SH1ADDSO,	"__ST200SH1ADDSO",
INTRN___ST200SH1SUBSO,	"__ST200SH1SUBSO",
INTRN___ST200SHLSO,	"__ST200SHLSO",
INTRN___ST200SHLS,	"__ST200SHLS",
INTRN___ST200SHLS_PH,	"__ST200SHLS_PH",
INTRN___ST200SHL_PH,	"__ST200SHL_PH",
INTRN___ST200SHRRNE_PH,	"__ST200SHRRNE_PH",
INTRN___ST200SHRRNP,	"__ST200SHRRNP",
INTRN___ST200SHRRNP_PH,	"__ST200SHRRNP_PH",
INTRN___ST200SHR_PH,	"__ST200SHR_PH",
INTRN___ST200SHUFFEVE_PB,	"__ST200SHUFFEVE_PB",
INTRN___ST200SHUFFODD_PB,	"__ST200SHUFFODD_PB",
INTRN___ST200SHUFF_PBH,	"__ST200SHUFF_PBH",
INTRN___ST200SHUFF_PBL,	"__ST200SHUFF_PBL",
INTRN___ST200SHUFF_PHH,	"__ST200SHUFF_PHH",
INTRN___ST200SHUFF_PHL,	"__ST200SHUFF_PHL",
INTRN___ST200SLCT_PB,	"__ST200SLCT_PB",
INTRN___ST200SUBSO,	"__ST200SUBSO",
INTRN___ST200SUBS_PH,	"__ST200SUBS_PH",
INTRN___ST200SUB_PH,	"__ST200SUB_PH",
INTRN___ST220SYNC,	"__ST220SYNC",
INTRN___ST220SYNCINS,	"__ST220SYNCINS",
INTRN___ST200UNPACKU_PBH,	"__ST200UNPACKU_PBH",
INTRN___ST200UNPACKU_PBL,	"__ST200UNPACKU_PBL",
INTRN___ST200WAITL,	"__ST200WAITL",
#endif /* INTRN_WUTIL */

#ifdef INTRN_GFEC_WFE_EXPR

    case BUILT_IN___ST200ABSS_PH:
      iopc = INTRN___ST200ABSS_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200ABSUBU_PB:
      iopc = INTRN___ST200ABSUBU_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200ADDSO:
      iopc = INTRN___ST200ADDSO;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200ADDS_PH:
      iopc = INTRN___ST200ADDS_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200ADD_PH:
      iopc = INTRN___ST200ADD_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200AVG4U_PB:
      iopc = INTRN___ST200AVG4U_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200AVGU_PB:
      iopc = INTRN___ST200AVGU_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200CMPEQ_PB:
      iopc = INTRN___ST200CMPEQ_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200CMPEQ_PB_BDEST:
      iopc = INTRN___ST200CMPEQ_PB_BDEST;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200CMPEQ_PH:
      iopc = INTRN___ST200CMPEQ_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200CMPEQ_PH_BDEST:
      iopc = INTRN___ST200CMPEQ_PH_BDEST;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200CMPGTU_PB:
      iopc = INTRN___ST200CMPGTU_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200CMPGTU_PB_BDEST:
      iopc = INTRN___ST200CMPGTU_PB_BDEST;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200CMPGT_PH:
      iopc = INTRN___ST200CMPGT_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200CMPGT_PH_BDEST:
      iopc = INTRN___ST200CMPGT_PH_BDEST;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200EXT1_PB:
      iopc = INTRN___ST200EXT1_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200EXT2_PB:
      iopc = INTRN___ST200EXT2_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200EXT3_PB:
      iopc = INTRN___ST200EXT3_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200EXTL_PB:
      iopc = INTRN___ST200EXTL_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200EXTR_PB:
      iopc = INTRN___ST200EXTR_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200FLUSHADD:
      iopc = INTRN___ST200FLUSHADD;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200FLUSHADD_L1:
      iopc = INTRN___ST200FLUSHADD_L1;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200INVADD:
      iopc = INTRN___ST200INVADD;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200INVADD_L1:
      iopc = INTRN___ST200INVADD_L1;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___MAXUW:
      iopc = INTRN___MAXUW;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___MAXW:
      iopc = INTRN___MAXW;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200MAX_PH:
      iopc = INTRN___ST200MAX_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___MINUW:
      iopc = INTRN___MINUW;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___MINW:
      iopc = INTRN___MINW;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200MIN_PH:
      iopc = INTRN___ST200MIN_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200MULADDUS_PB:
      iopc = INTRN___ST200MULADDUS_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200MULADD_PH:
      iopc = INTRN___ST200MULADD_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200MULFRACADDS_PH:
      iopc = INTRN___ST200MULFRACADDS_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200MULFRACRM_PH:
      iopc = INTRN___ST200MULFRACRM_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200MULFRACRNE_PH:
      iopc = INTRN___ST200MULFRACRNE_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200MUL_PH:
      iopc = INTRN___ST200MUL_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200PACKRNP_PHH:
      iopc = INTRN___ST200PACKRNP_PHH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200PACKSU_PB:
      iopc = INTRN___ST200PACKSU_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200PACKS_PH:
      iopc = INTRN___ST200PACKS_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200PACK_PB:
      iopc = INTRN___ST200PACK_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200PACK_PH:
      iopc = INTRN___ST200PACK_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200PERM_PB:
      iopc = INTRN___ST200PERM_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST220PRGADD_L1:
      iopc = INTRN___ST220PRGADD_L1;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST220PRGINS:
      iopc = INTRN___ST220PRGINS;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200PRGINSADD:
      iopc = INTRN___ST200PRGINSADD;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200PRGINSADD_L1:
      iopc = INTRN___ST200PRGINSADD_L1;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200PRGINSSET_L1:
      iopc = INTRN___ST200PRGINSSET_L1;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200PSWCLR:
      iopc = INTRN___ST200PSWCLR;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200PSWMASK:
      iopc = INTRN___ST200PSWMASK;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200PSWSET:
      iopc = INTRN___ST200PSWSET;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200RETENTION:
      iopc = INTRN___ST200RETENTION;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200SADU_PB:
      iopc = INTRN___ST200SADU_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SATSO:
      iopc = INTRN___ST200SATSO;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SH1ADDSO:
      iopc = INTRN___ST200SH1ADDSO;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SH1SUBSO:
      iopc = INTRN___ST200SH1SUBSO;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHLSO:
      iopc = INTRN___ST200SHLSO;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHLS:
      iopc = INTRN___ST200SHLS;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHLS_PH:
      iopc = INTRN___ST200SHLS_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHL_PH:
      iopc = INTRN___ST200SHL_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHRRNE_PH:
      iopc = INTRN___ST200SHRRNE_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHRRNP:
      iopc = INTRN___ST200SHRRNP;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHRRNP_PH:
      iopc = INTRN___ST200SHRRNP_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHR_PH:
      iopc = INTRN___ST200SHR_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHUFFEVE_PB:
      iopc = INTRN___ST200SHUFFEVE_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHUFFODD_PB:
      iopc = INTRN___ST200SHUFFODD_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHUFF_PBH:
      iopc = INTRN___ST200SHUFF_PBH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHUFF_PBL:
      iopc = INTRN___ST200SHUFF_PBL;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHUFF_PHH:
      iopc = INTRN___ST200SHUFF_PHH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SHUFF_PHL:
      iopc = INTRN___ST200SHUFF_PHL;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SLCT_PB:
      iopc = INTRN___ST200SLCT_PB;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SUBSO:
      iopc = INTRN___ST200SUBSO;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SUBS_PH:
      iopc = INTRN___ST200SUBS_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200SUB_PH:
      iopc = INTRN___ST200SUB_PH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST220SYNC:
      iopc = INTRN___ST220SYNC;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST220SYNCINS:
      iopc = INTRN___ST220SYNCINS;
      intrinsic_op = FALSE;
    break;
    case BUILT_IN___ST200UNPACKU_PBH:
      iopc = INTRN___ST200UNPACKU_PBH;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200UNPACKU_PBL:
      iopc = INTRN___ST200UNPACKU_PBL;
      intrinsic_op = TRUE;
    break;
    case BUILT_IN___ST200WAITL:
      iopc = INTRN___ST200WAITL;
      intrinsic_op = FALSE;
    break;
#endif /* INTRN_GFEC_WFE_EXPR */

#ifdef INTRN_GFECC_BUILTINS_DEF

DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200ABSS_PH,
            "__builtin___st200abss_ph",
             BT_FN_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200ABSUBU_PB,
            "__builtin___st200absubu_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200ADDSO,
            "__builtin___st200addso",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200ADDS_PH,
            "__builtin___st200adds_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200ADD_PH,
            "__builtin___st200add_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200AVG4U_PB,
            "__builtin___st200avg4u_pb",
             BT_FN_INT_UNSIGNED_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200AVGU_PB,
            "__builtin___st200avgu_pb",
             BT_FN_INT_UNSIGNED_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200CMPEQ_PB,
            "__builtin___st200cmpeq_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200CMPEQ_PB_BDEST,
            "__builtin___st200cmpeq_pb_bdest",
             BT_FN_UNSIGNED_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200CMPEQ_PH,
            "__builtin___st200cmpeq_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200CMPEQ_PH_BDEST,
            "__builtin___st200cmpeq_ph_bdest",
             BT_FN_UNSIGNED_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200CMPGTU_PB,
            "__builtin___st200cmpgtu_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200CMPGTU_PB_BDEST,
            "__builtin___st200cmpgtu_pb_bdest",
             BT_FN_UNSIGNED_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200CMPGT_PH,
            "__builtin___st200cmpgt_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200CMPGT_PH_BDEST,
            "__builtin___st200cmpgt_ph_bdest",
             BT_FN_UNSIGNED_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200EXT1_PB,
            "__builtin___st200ext1_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200EXT2_PB,
            "__builtin___st200ext2_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200EXT3_PB,
            "__builtin___st200ext3_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200EXTL_PB,
            "__builtin___st200extl_pb",
             BT_FN_INT_UNSIGNED_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200EXTR_PB,
            "__builtin___st200extr_pb",
             BT_FN_INT_UNSIGNED_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200FLUSHADD,
            "__builtin___st200flushadd",
             BT_FN_VOID_INT_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200FLUSHADD_L1,
            "__builtin___st200flushadd_l1",
             BT_FN_VOID_INT_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200INVADD,
            "__builtin___st200invadd",
             BT_FN_VOID_INT_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200INVADD_L1,
            "__builtin___st200invadd_l1",
             BT_FN_VOID_INT_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___MAXUW,
            "__builtin___maxuw",
             BT_FN_UNSIGNED_UNSIGNED_UNSIGNED,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___MAXW,
            "__builtin___maxw",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200MAX_PH,
            "__builtin___st200max_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___MINUW,
            "__builtin___minuw",
             BT_FN_UNSIGNED_UNSIGNED_UNSIGNED,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___MINW,
            "__builtin___minw",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200MIN_PH,
            "__builtin___st200min_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200MULADDUS_PB,
            "__builtin___st200muladdus_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200MULADD_PH,
            "__builtin___st200muladd_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200MULFRACADDS_PH,
            "__builtin___st200mulfracadds_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200MULFRACRM_PH,
            "__builtin___st200mulfracrm_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200MULFRACRNE_PH,
            "__builtin___st200mulfracrne_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200MUL_PH,
            "__builtin___st200mul_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PACKRNP_PHH,
            "__builtin___st200packrnp_phh",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PACKSU_PB,
            "__builtin___st200packsu_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PACKS_PH,
            "__builtin___st200packs_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PACK_PB,
            "__builtin___st200pack_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PACK_PH,
            "__builtin___st200pack_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PERM_PB,
            "__builtin___st200perm_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST220PRGADD_L1,
            "__builtin___st220prgadd_l1",
             BT_FN_VOID_INT_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST220PRGINS,
            "__builtin___st220prgins",
             BT_FN_VOID,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PRGINSADD,
            "__builtin___st200prginsadd",
             BT_FN_VOID_INT_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PRGINSADD_L1,
            "__builtin___st200prginsadd_l1",
             BT_FN_VOID_INT_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PRGINSSET_L1,
            "__builtin___st200prginsset_l1",
             BT_FN_VOID_INT_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PSWCLR,
            "__builtin___st200pswclr",
             BT_FN_VOID_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PSWMASK,
            "__builtin___st200pswmask",
             BT_FN_INT_INT_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200PSWSET,
            "__builtin___st200pswset",
             BT_FN_VOID_INT,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200RETENTION,
            "__builtin___st200retention",
             BT_FN_VOID,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SADU_PB,
            "__builtin___st200sadu_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SATSO,
            "__builtin___st200satso",
             BT_FN_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SH1ADDSO,
            "__builtin___st200sh1addso",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SH1SUBSO,
            "__builtin___st200sh1subso",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHLSO,
            "__builtin___st200shlso",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHLS,
            "__builtin___st200shls",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHLS_PH,
            "__builtin___st200shls_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHL_PH,
            "__builtin___st200shl_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHRRNE_PH,
            "__builtin___st200shrrne_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHRRNP,
            "__builtin___st200shrrnp",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHRRNP_PH,
            "__builtin___st200shrrnp_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHR_PH,
            "__builtin___st200shr_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHUFFEVE_PB,
            "__builtin___st200shuffeve_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHUFFODD_PB,
            "__builtin___st200shuffodd_pb",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHUFF_PBH,
            "__builtin___st200shuff_pbh",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHUFF_PBL,
            "__builtin___st200shuff_pbl",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHUFF_PHH,
            "__builtin___st200shuff_phh",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SHUFF_PHL,
            "__builtin___st200shuff_phl",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SLCT_PB,
            "__builtin___st200slct_pb",
             BT_FN_INT_UNSIGNED_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SUBSO,
            "__builtin___st200subso",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SUBS_PH,
            "__builtin___st200subs_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200SUB_PH,
            "__builtin___st200sub_ph",
             BT_FN_INT_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST220SYNC,
            "__builtin___st220sync",
             BT_FN_VOID,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST220SYNCINS,
            "__builtin___st220syncins",
             BT_FN_VOID,
             ATTR_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200UNPACKU_PBH,
            "__builtin___st200unpacku_pbh",
             BT_FN_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200UNPACKU_PBL,
            "__builtin___st200unpacku_pbl",
             BT_FN_INT_INT,
             ATTR_PURE_NOTHROW_LIST)
DEF_EXT_LIB_BUILTIN (BUILT_IN___ST200WAITL,
            "__builtin___st200waitl",
             BT_FN_VOID,
             ATTR_NOTHROW_LIST)
#endif /* INTRN_GFECC_BUILTINS_DEF */

#ifdef INTRN_GFEC_BUILTINS_DEF

DEF_BUILTIN (BUILT_IN___ST200ABSS_PH)
DEF_BUILTIN (BUILT_IN___ST200ABSUBU_PB)
DEF_BUILTIN (BUILT_IN___ST200ADDSO)
DEF_BUILTIN (BUILT_IN___ST200ADDS_PH)
DEF_BUILTIN (BUILT_IN___ST200ADD_PH)
DEF_BUILTIN (BUILT_IN___ST200AVG4U_PB)
DEF_BUILTIN (BUILT_IN___ST200AVGU_PB)
DEF_BUILTIN (BUILT_IN___ST200CMPEQ_PB)
DEF_BUILTIN (BUILT_IN___ST200CMPEQ_PB_BDEST)
DEF_BUILTIN (BUILT_IN___ST200CMPEQ_PH)
DEF_BUILTIN (BUILT_IN___ST200CMPEQ_PH_BDEST)
DEF_BUILTIN (BUILT_IN___ST200CMPGTU_PB)
DEF_BUILTIN (BUILT_IN___ST200CMPGTU_PB_BDEST)
DEF_BUILTIN (BUILT_IN___ST200CMPGT_PH)
DEF_BUILTIN (BUILT_IN___ST200CMPGT_PH_BDEST)
DEF_BUILTIN (BUILT_IN___ST200EXT1_PB)
DEF_BUILTIN (BUILT_IN___ST200EXT2_PB)
DEF_BUILTIN (BUILT_IN___ST200EXT3_PB)
DEF_BUILTIN (BUILT_IN___ST200EXTL_PB)
DEF_BUILTIN (BUILT_IN___ST200EXTR_PB)
DEF_BUILTIN (BUILT_IN___ST200FLUSHADD)
DEF_BUILTIN (BUILT_IN___ST200FLUSHADD_L1)
DEF_BUILTIN (BUILT_IN___ST200INVADD)
DEF_BUILTIN (BUILT_IN___ST200INVADD_L1)
DEF_BUILTIN (BUILT_IN___MAXUW)
DEF_BUILTIN (BUILT_IN___MAXW)
DEF_BUILTIN (BUILT_IN___ST200MAX_PH)
DEF_BUILTIN (BUILT_IN___MINUW)
DEF_BUILTIN (BUILT_IN___MINW)
DEF_BUILTIN (BUILT_IN___ST200MIN_PH)
DEF_BUILTIN (BUILT_IN___ST200MULADDUS_PB)
DEF_BUILTIN (BUILT_IN___ST200MULADD_PH)
DEF_BUILTIN (BUILT_IN___ST200MULFRACADDS_PH)
DEF_BUILTIN (BUILT_IN___ST200MULFRACRM_PH)
DEF_BUILTIN (BUILT_IN___ST200MULFRACRNE_PH)
DEF_BUILTIN (BUILT_IN___ST200MUL_PH)
DEF_BUILTIN (BUILT_IN___ST200PACKRNP_PHH)
DEF_BUILTIN (BUILT_IN___ST200PACKSU_PB)
DEF_BUILTIN (BUILT_IN___ST200PACKS_PH)
DEF_BUILTIN (BUILT_IN___ST200PACK_PB)
DEF_BUILTIN (BUILT_IN___ST200PACK_PH)
DEF_BUILTIN (BUILT_IN___ST200PERM_PB)
DEF_BUILTIN (BUILT_IN___ST220PRGADD_L1)
DEF_BUILTIN (BUILT_IN___ST220PRGINS)
DEF_BUILTIN (BUILT_IN___ST200PRGINSADD)
DEF_BUILTIN (BUILT_IN___ST200PRGINSADD_L1)
DEF_BUILTIN (BUILT_IN___ST200PRGINSSET_L1)
DEF_BUILTIN (BUILT_IN___ST200PSWCLR)
DEF_BUILTIN (BUILT_IN___ST200PSWMASK)
DEF_BUILTIN (BUILT_IN___ST200PSWSET)
DEF_BUILTIN (BUILT_IN___ST200RETENTION)
DEF_BUILTIN (BUILT_IN___ST200SADU_PB)
DEF_BUILTIN (BUILT_IN___ST200SATSO)
DEF_BUILTIN (BUILT_IN___ST200SH1ADDSO)
DEF_BUILTIN (BUILT_IN___ST200SH1SUBSO)
DEF_BUILTIN (BUILT_IN___ST200SHLSO)
DEF_BUILTIN (BUILT_IN___ST200SHLS)
DEF_BUILTIN (BUILT_IN___ST200SHLS_PH)
DEF_BUILTIN (BUILT_IN___ST200SHL_PH)
DEF_BUILTIN (BUILT_IN___ST200SHRRNE_PH)
DEF_BUILTIN (BUILT_IN___ST200SHRRNP)
DEF_BUILTIN (BUILT_IN___ST200SHRRNP_PH)
DEF_BUILTIN (BUILT_IN___ST200SHR_PH)
DEF_BUILTIN (BUILT_IN___ST200SHUFFEVE_PB)
DEF_BUILTIN (BUILT_IN___ST200SHUFFODD_PB)
DEF_BUILTIN (BUILT_IN___ST200SHUFF_PBH)
DEF_BUILTIN (BUILT_IN___ST200SHUFF_PBL)
DEF_BUILTIN (BUILT_IN___ST200SHUFF_PHH)
DEF_BUILTIN (BUILT_IN___ST200SHUFF_PHL)
DEF_BUILTIN (BUILT_IN___ST200SLCT_PB)
DEF_BUILTIN (BUILT_IN___ST200SUBSO)
DEF_BUILTIN (BUILT_IN___ST200SUBS_PH)
DEF_BUILTIN (BUILT_IN___ST200SUB_PH)
DEF_BUILTIN (BUILT_IN___ST220SYNC)
DEF_BUILTIN (BUILT_IN___ST220SYNCINS)
DEF_BUILTIN (BUILT_IN___ST200UNPACKU_PBH)
DEF_BUILTIN (BUILT_IN___ST200UNPACKU_PBL)
DEF_BUILTIN (BUILT_IN___ST200WAITL)
#endif /* INTRN_GFEC_BUILTINS_DEF */

#ifdef INTRN_GFEC_BUILTINS_H

    case BUILT_IN___ST200ABSS_PH:
    case BUILT_IN___ST200ABSUBU_PB:
    case BUILT_IN___ST200ADDSO:
    case BUILT_IN___ST200ADDS_PH:
    case BUILT_IN___ST200ADD_PH:
    case BUILT_IN___ST200AVG4U_PB:
    case BUILT_IN___ST200AVGU_PB:
    case BUILT_IN___ST200CMPEQ_PB:
    case BUILT_IN___ST200CMPEQ_PB_BDEST:
    case BUILT_IN___ST200CMPEQ_PH:
    case BUILT_IN___ST200CMPEQ_PH_BDEST:
    case BUILT_IN___ST200CMPGTU_PB:
    case BUILT_IN___ST200CMPGTU_PB_BDEST:
    case BUILT_IN___ST200CMPGT_PH:
    case BUILT_IN___ST200CMPGT_PH_BDEST:
    case BUILT_IN___ST200EXT1_PB:
    case BUILT_IN___ST200EXT2_PB:
    case BUILT_IN___ST200EXT3_PB:
    case BUILT_IN___ST200EXTL_PB:
    case BUILT_IN___ST200EXTR_PB:
    case BUILT_IN___ST200FLUSHADD:
    case BUILT_IN___ST200FLUSHADD_L1:
    case BUILT_IN___ST200INVADD:
    case BUILT_IN___ST200INVADD_L1:
    case BUILT_IN___MAXUW:
    case BUILT_IN___MAXW:
    case BUILT_IN___ST200MAX_PH:
    case BUILT_IN___MINUW:
    case BUILT_IN___MINW:
    case BUILT_IN___ST200MIN_PH:
    case BUILT_IN___ST200MULADDUS_PB:
    case BUILT_IN___ST200MULADD_PH:
    case BUILT_IN___ST200MULFRACADDS_PH:
    case BUILT_IN___ST200MULFRACRM_PH:
    case BUILT_IN___ST200MULFRACRNE_PH:
    case BUILT_IN___ST200MUL_PH:
    case BUILT_IN___ST200PACKRNP_PHH:
    case BUILT_IN___ST200PACKSU_PB:
    case BUILT_IN___ST200PACKS_PH:
    case BUILT_IN___ST200PACK_PB:
    case BUILT_IN___ST200PACK_PH:
    case BUILT_IN___ST200PERM_PB:
    case BUILT_IN___ST220PRGADD_L1:
    case BUILT_IN___ST220PRGINS:
    case BUILT_IN___ST200PRGINSADD:
    case BUILT_IN___ST200PRGINSADD_L1:
    case BUILT_IN___ST200PRGINSSET_L1:
    case BUILT_IN___ST200PSWCLR:
    case BUILT_IN___ST200PSWMASK:
    case BUILT_IN___ST200PSWSET:
    case BUILT_IN___ST200RETENTION:
    case BUILT_IN___ST200SADU_PB:
    case BUILT_IN___ST200SATSO:
    case BUILT_IN___ST200SH1ADDSO:
    case BUILT_IN___ST200SH1SUBSO:
    case BUILT_IN___ST200SHLSO:
    case BUILT_IN___ST200SHLS:
    case BUILT_IN___ST200SHLS_PH:
    case BUILT_IN___ST200SHL_PH:
    case BUILT_IN___ST200SHRRNE_PH:
    case BUILT_IN___ST200SHRRNP:
    case BUILT_IN___ST200SHRRNP_PH:
    case BUILT_IN___ST200SHR_PH:
    case BUILT_IN___ST200SHUFFEVE_PB:
    case BUILT_IN___ST200SHUFFODD_PB:
    case BUILT_IN___ST200SHUFF_PBH:
    case BUILT_IN___ST200SHUFF_PBL:
    case BUILT_IN___ST200SHUFF_PHH:
    case BUILT_IN___ST200SHUFF_PHL:
    case BUILT_IN___ST200SLCT_PB:
    case BUILT_IN___ST200SUBSO:
    case BUILT_IN___ST200SUBS_PH:
    case BUILT_IN___ST200SUB_PH:
    case BUILT_IN___ST220SYNC:
    case BUILT_IN___ST220SYNCINS:
    case BUILT_IN___ST200UNPACKU_PBH:
    case BUILT_IN___ST200UNPACKU_PBL:
    case BUILT_IN___ST200WAITL:
#endif /* INTRN_GFEC_BUILTINS_H */

