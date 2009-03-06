/* Generated automatically by the program `genpreds'.  */

#ifndef GCC_TM_PREDS_H
#define GCC_TM_PREDS_H

#ifdef RTX_CODE

extern int s_register_operand PARAMS ((rtx, machine_mode_t));
extern int arm_hard_register_operand PARAMS ((rtx, machine_mode_t));
extern int f_register_operand PARAMS ((rtx, machine_mode_t));
extern int arm_add_operand PARAMS ((rtx, machine_mode_t));
extern int fpu_add_operand PARAMS ((rtx, machine_mode_t));
extern int fpu_rhs_operand PARAMS ((rtx, machine_mode_t));
extern int arm_rhs_operand PARAMS ((rtx, machine_mode_t));
extern int arm_not_operand PARAMS ((rtx, machine_mode_t));
extern int reg_or_int_operand PARAMS ((rtx, machine_mode_t));
extern int index_operand PARAMS ((rtx, machine_mode_t));
extern int thumb_cmp_operand PARAMS ((rtx, machine_mode_t));
extern int offsettable_memory_operand PARAMS ((rtx, machine_mode_t));
extern int bad_signed_byte_operand PARAMS ((rtx, machine_mode_t));
extern int alignable_memory_operand PARAMS ((rtx, machine_mode_t));
extern int shiftable_operator PARAMS ((rtx, machine_mode_t));
extern int minmax_operator PARAMS ((rtx, machine_mode_t));
extern int shift_operator PARAMS ((rtx, machine_mode_t));
extern int di_operand PARAMS ((rtx, machine_mode_t));
extern int nonimmediate_di_operand PARAMS ((rtx, machine_mode_t));
extern int soft_df_operand PARAMS ((rtx, machine_mode_t));
extern int nonimmediate_soft_df_operand PARAMS ((rtx, machine_mode_t));
extern int load_multiple_operation PARAMS ((rtx, machine_mode_t));
extern int store_multiple_operation PARAMS ((rtx, machine_mode_t));
extern int equality_operator PARAMS ((rtx, machine_mode_t));
extern int arm_comparison_operator PARAMS ((rtx, machine_mode_t));
extern int arm_rhsm_operand PARAMS ((rtx, machine_mode_t));
extern int const_shift_operand PARAMS ((rtx, machine_mode_t));
extern int multi_register_push PARAMS ((rtx, machine_mode_t));
extern int cc_register PARAMS ((rtx, machine_mode_t));
extern int logical_binary_operator PARAMS ((rtx, machine_mode_t));
extern int dominant_cc_register PARAMS ((rtx, machine_mode_t));

#endif /* RTX_CODE */

#endif /* GCC_TM_PREDS_H */
