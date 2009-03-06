/* Generated automatically by the program `genflags'
   from the machine description file `md'.  */

#ifndef GCC_INSN_FLAGS_H
#define GCC_INSN_FLAGS_H

#define HAVE_incscc (TARGET_ARM)
#define HAVE_addsf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_adddf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_addxf3 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_decscc (TARGET_ARM)
#define HAVE_subsf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_subdf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_subxf3 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_mulsidi3 (TARGET_ARM && arm_fast_multiply)
#define HAVE_umulsidi3 (TARGET_ARM && arm_fast_multiply)
#define HAVE_smulsi3_highpart (TARGET_ARM && arm_fast_multiply)
#define HAVE_umulsi3_highpart (TARGET_ARM && arm_fast_multiply)
#define HAVE_mulhisi3 (TARGET_ARM && arm_is_xscale)
#define HAVE_mulsf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_muldf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_mulxf3 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_divsf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_divdf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_divxf3 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_modsf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_moddf3 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_modxf3 (ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_anddi3 (TARGET_ARM)
#define HAVE_andsi_notsi_si (TARGET_ARM)
#define HAVE_bicsi3 (TARGET_THUMB)
#define HAVE_andsi_not_shiftsi_si (TARGET_ARM)
#define HAVE_iordi3 (TARGET_ARM)
#define HAVE_xordi3 (TARGET_ARM)
#define HAVE_smaxsi3 (TARGET_ARM)
#define HAVE_sminsi3 (TARGET_ARM)
#define HAVE_umaxsi3 (TARGET_ARM)
#define HAVE_uminsi3 (TARGET_ARM)
#define HAVE_negsf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_negdf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_negxf2 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_abssi2 (TARGET_ARM)
#define HAVE_abssf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_absdf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_absxf2 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_sqrtsf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_sqrtdf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_sqrtxf2 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_one_cmpldi2 (TARGET_ARM)
#define HAVE_floatsisf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_floatsidf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_floatsixf2 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_fix_truncsfsi2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_fix_truncdfsi2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_fix_truncxfsi2 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_truncdfsf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_truncxfsf2 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_truncxfdf2 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_zero_extendsidi2 (TARGET_ARM)
#define HAVE_zero_extendqidi2 (TARGET_ARM)
#define HAVE_extendsidi2 (TARGET_ARM)
#define HAVE_extendsfdf2 (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_extendsfxf2 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_extenddfxf2 (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_pic_load_addr_arm (TARGET_ARM && flag_pic)
#define HAVE_pic_load_addr_thumb (TARGET_THUMB && flag_pic)
#define HAVE_pic_add_dot_plus_four (TARGET_THUMB && flag_pic)
#define HAVE_pic_add_dot_plus_eight (TARGET_ARM && flag_pic)
#define HAVE_rotated_loadsi (TARGET_ARM && (!TARGET_MMU_TRAPS))
#define HAVE_thumb_movhi_clobber (TARGET_THUMB)
#define HAVE_movmem12b (TARGET_THUMB)
#define HAVE_movmem8b (TARGET_THUMB)
#define HAVE_cbranchsi4 (TARGET_THUMB)
#define HAVE_return (TARGET_ARM && USE_RETURN_INSN (FALSE))
#define HAVE_blockage (TARGET_EITHER)
#define HAVE_casesi_internal (TARGET_ARM)
#define HAVE_nop (TARGET_EITHER)
#define HAVE_movcond (TARGET_ARM)
#define HAVE_sibcall_epilogue (TARGET_ARM)
#define HAVE_stack_tie 1
#define HAVE_align_4 (TARGET_EITHER)
#define HAVE_consttable_end (TARGET_EITHER)
#define HAVE_consttable_1 (TARGET_THUMB)
#define HAVE_consttable_2 (TARGET_THUMB)
#define HAVE_consttable_4 (TARGET_EITHER)
#define HAVE_consttable_8 (TARGET_EITHER)
#define HAVE_clz (TARGET_ARM && arm_arch5)
#define HAVE_prefetch (TARGET_ARM && arm_arch5e)
#define HAVE_prologue_use 1
#define HAVE_adddi3 (TARGET_EITHER)
#define HAVE_addsi3 (TARGET_EITHER)
#define HAVE_subdi3 (TARGET_EITHER)
#define HAVE_subsi3 (TARGET_EITHER)
#define HAVE_mulsi3 (TARGET_EITHER)
#define HAVE_andsi3 (TARGET_EITHER)
#define HAVE_insv (TARGET_ARM)
#define HAVE_iorsi3 (TARGET_EITHER)
#define HAVE_xorsi3 (TARGET_EITHER)
#define HAVE_ashlsi3 (TARGET_EITHER)
#define HAVE_ashrsi3 (TARGET_EITHER)
#define HAVE_lshrsi3 (TARGET_EITHER)
#define HAVE_rotlsi3 (TARGET_ARM)
#define HAVE_rotrsi3 (TARGET_EITHER)
#define HAVE_extzv (TARGET_THUMB)
#define HAVE_negdi2 (TARGET_EITHER)
#define HAVE_negsi2 (TARGET_EITHER)
#define HAVE_one_cmplsi2 (TARGET_EITHER)
#define HAVE_zero_extendhisi2 (TARGET_EITHER)
#define HAVE_zero_extendqisi2 (TARGET_EITHER)
#define HAVE_extendhisi2 (TARGET_EITHER)
#define HAVE_extendhisi2_mem (TARGET_ARM)
#define HAVE_extendqihi2 (TARGET_ARM)
#define HAVE_extendqisi2 (TARGET_EITHER)
#define HAVE_movdi (TARGET_EITHER)
#define HAVE_movsi (TARGET_EITHER)
#define HAVE_pic_load_addr_based (TARGET_ARM && flag_pic)
#define HAVE_builtin_setjmp_receiver (flag_pic)
#define HAVE_storehi (TARGET_ARM)
#define HAVE_storehi_bigend (TARGET_ARM)
#define HAVE_storeinthi (TARGET_ARM)
#define HAVE_storehi_single_op (TARGET_ARM && arm_arch4)
#define HAVE_movhi (TARGET_EITHER)
#define HAVE_movhi_bytes (TARGET_ARM)
#define HAVE_movhi_bigend (TARGET_ARM)
#define HAVE_reload_outhi (TARGET_EITHER)
#define HAVE_reload_inhi (TARGET_THUMB || (TARGET_ARM && TARGET_MMU_TRAPS))
#define HAVE_movqi (TARGET_EITHER)
#define HAVE_movsf (TARGET_EITHER)
#define HAVE_movdf (TARGET_EITHER)
#define HAVE_reload_outdf (TARGET_ARM)
#define HAVE_movxf (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_load_multiple (TARGET_ARM)
#define HAVE_store_multiple (TARGET_ARM)
#define HAVE_movstrqi (TARGET_EITHER)
#define HAVE_cmpsi (TARGET_ARM)
#define HAVE_cmpsf (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_cmpdf (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_cmpxf (TARGET_ARM && ENABLE_XF_PATTERNS && TARGET_HARD_FLOAT)
#define HAVE_beq (TARGET_ARM)
#define HAVE_bne (TARGET_ARM)
#define HAVE_bgt (TARGET_ARM)
#define HAVE_ble (TARGET_ARM)
#define HAVE_bge (TARGET_ARM)
#define HAVE_blt (TARGET_ARM)
#define HAVE_bgtu (TARGET_ARM)
#define HAVE_bleu (TARGET_ARM)
#define HAVE_bgeu (TARGET_ARM)
#define HAVE_bltu (TARGET_ARM)
#define HAVE_bunordered (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_bordered (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_bungt (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_bunlt (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_bunge (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_bunle (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_buneq (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_bltgt (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_seq (TARGET_ARM)
#define HAVE_sne (TARGET_ARM)
#define HAVE_sgt (TARGET_ARM)
#define HAVE_sle (TARGET_ARM)
#define HAVE_sge (TARGET_ARM)
#define HAVE_slt (TARGET_ARM)
#define HAVE_sgtu (TARGET_ARM)
#define HAVE_sleu (TARGET_ARM)
#define HAVE_sgeu (TARGET_ARM)
#define HAVE_sltu (TARGET_ARM)
#define HAVE_sunordered (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_sordered (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_sungt (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_sunge (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_sunlt (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_sunle (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_movsicc (TARGET_ARM)
#define HAVE_movsfcc (TARGET_ARM)
#define HAVE_movdfcc (TARGET_ARM && TARGET_HARD_FLOAT)
#define HAVE_jump (TARGET_EITHER)
#define HAVE_call (TARGET_EITHER)
#define HAVE_call_value (TARGET_EITHER)
#define HAVE_sibcall (TARGET_ARM)
#define HAVE_sibcall_value (TARGET_ARM)
#define HAVE_return_addr_mask (TARGET_ARM)
#define HAVE_untyped_call (TARGET_ARM)
#define HAVE_casesi (TARGET_ARM)
#define HAVE_indirect_jump (TARGET_EITHER)
#define HAVE_prologue (TARGET_EITHER)
#define HAVE_epilogue (TARGET_EITHER)
#define HAVE_eh_epilogue (TARGET_EITHER)
#define HAVE_tablejump (TARGET_THUMB)
#define HAVE_ffssi2 (TARGET_ARM && arm_arch5)
extern rtx        gen_incscc                  PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_addsf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_adddf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_addxf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_decscc                  PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_subsf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_subdf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_subxf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_mulsidi3                PARAMS ((rtx, rtx, rtx));
extern rtx        gen_umulsidi3               PARAMS ((rtx, rtx, rtx));
extern rtx        gen_smulsi3_highpart        PARAMS ((rtx, rtx, rtx));
extern rtx        gen_umulsi3_highpart        PARAMS ((rtx, rtx, rtx));
extern rtx        gen_mulhisi3                PARAMS ((rtx, rtx, rtx));
extern rtx        gen_mulsf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_muldf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_mulxf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_divsf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_divdf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_divxf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_modsf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_moddf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_modxf3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_anddi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_andsi_notsi_si          PARAMS ((rtx, rtx, rtx));
extern rtx        gen_bicsi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_andsi_not_shiftsi_si    PARAMS ((rtx, rtx, rtx, rtx, rtx));
extern rtx        gen_iordi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_xordi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_smaxsi3                 PARAMS ((rtx, rtx, rtx));
extern rtx        gen_sminsi3                 PARAMS ((rtx, rtx, rtx));
extern rtx        gen_umaxsi3                 PARAMS ((rtx, rtx, rtx));
extern rtx        gen_uminsi3                 PARAMS ((rtx, rtx, rtx));
extern rtx        gen_negsf2                  PARAMS ((rtx, rtx));
extern rtx        gen_negdf2                  PARAMS ((rtx, rtx));
extern rtx        gen_negxf2                  PARAMS ((rtx, rtx));
extern rtx        gen_abssi2                  PARAMS ((rtx, rtx));
extern rtx        gen_abssf2                  PARAMS ((rtx, rtx));
extern rtx        gen_absdf2                  PARAMS ((rtx, rtx));
extern rtx        gen_absxf2                  PARAMS ((rtx, rtx));
extern rtx        gen_sqrtsf2                 PARAMS ((rtx, rtx));
extern rtx        gen_sqrtdf2                 PARAMS ((rtx, rtx));
extern rtx        gen_sqrtxf2                 PARAMS ((rtx, rtx));
extern rtx        gen_one_cmpldi2             PARAMS ((rtx, rtx));
extern rtx        gen_floatsisf2              PARAMS ((rtx, rtx));
extern rtx        gen_floatsidf2              PARAMS ((rtx, rtx));
extern rtx        gen_floatsixf2              PARAMS ((rtx, rtx));
extern rtx        gen_fix_truncsfsi2          PARAMS ((rtx, rtx));
extern rtx        gen_fix_truncdfsi2          PARAMS ((rtx, rtx));
extern rtx        gen_fix_truncxfsi2          PARAMS ((rtx, rtx));
extern rtx        gen_truncdfsf2              PARAMS ((rtx, rtx));
extern rtx        gen_truncxfsf2              PARAMS ((rtx, rtx));
extern rtx        gen_truncxfdf2              PARAMS ((rtx, rtx));
extern rtx        gen_zero_extendsidi2        PARAMS ((rtx, rtx));
extern rtx        gen_zero_extendqidi2        PARAMS ((rtx, rtx));
extern rtx        gen_extendsidi2             PARAMS ((rtx, rtx));
extern rtx        gen_extendsfdf2             PARAMS ((rtx, rtx));
extern rtx        gen_extendsfxf2             PARAMS ((rtx, rtx));
extern rtx        gen_extenddfxf2             PARAMS ((rtx, rtx));
extern rtx        gen_pic_load_addr_arm       PARAMS ((rtx, rtx));
extern rtx        gen_pic_load_addr_thumb     PARAMS ((rtx, rtx));
extern rtx        gen_pic_add_dot_plus_four   PARAMS ((rtx, rtx));
extern rtx        gen_pic_add_dot_plus_eight  PARAMS ((rtx, rtx));
extern rtx        gen_rotated_loadsi          PARAMS ((rtx, rtx));
extern rtx        gen_thumb_movhi_clobber     PARAMS ((rtx, rtx, rtx));
extern rtx        gen_movmem12b               PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_movmem8b                PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_cbranchsi4              PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_return                  PARAMS ((void));
extern rtx        gen_blockage                PARAMS ((void));
extern rtx        gen_casesi_internal         PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_nop                     PARAMS ((void));
extern rtx        gen_movcond                 PARAMS ((rtx, rtx, rtx, rtx, rtx, rtx));
extern rtx        gen_sibcall_epilogue        PARAMS ((void));
extern rtx        gen_stack_tie               PARAMS ((rtx, rtx));
extern rtx        gen_align_4                 PARAMS ((void));
extern rtx        gen_consttable_end          PARAMS ((void));
extern rtx        gen_consttable_1            PARAMS ((rtx));
extern rtx        gen_consttable_2            PARAMS ((rtx));
extern rtx        gen_consttable_4            PARAMS ((rtx));
extern rtx        gen_consttable_8            PARAMS ((rtx));
extern rtx        gen_clz                     PARAMS ((rtx, rtx));
extern rtx        gen_prefetch                PARAMS ((rtx, rtx, rtx));
extern rtx        gen_prologue_use            PARAMS ((rtx));
extern rtx        gen_adddi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_addsi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_subdi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_subsi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_mulsi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_andsi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_insv                    PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_iorsi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_xorsi3                  PARAMS ((rtx, rtx, rtx));
extern rtx        gen_ashlsi3                 PARAMS ((rtx, rtx, rtx));
extern rtx        gen_ashrsi3                 PARAMS ((rtx, rtx, rtx));
extern rtx        gen_lshrsi3                 PARAMS ((rtx, rtx, rtx));
extern rtx        gen_rotlsi3                 PARAMS ((rtx, rtx, rtx));
extern rtx        gen_rotrsi3                 PARAMS ((rtx, rtx, rtx));
extern rtx        gen_extzv                   PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_negdi2                  PARAMS ((rtx, rtx));
extern rtx        gen_negsi2                  PARAMS ((rtx, rtx));
extern rtx        gen_one_cmplsi2             PARAMS ((rtx, rtx));
extern rtx        gen_zero_extendhisi2        PARAMS ((rtx, rtx));
extern rtx        gen_zero_extendqisi2        PARAMS ((rtx, rtx));
extern rtx        gen_extendhisi2             PARAMS ((rtx, rtx));
extern rtx        gen_extendhisi2_mem         PARAMS ((rtx, rtx));
extern rtx        gen_extendqihi2             PARAMS ((rtx, rtx));
extern rtx        gen_extendqisi2             PARAMS ((rtx, rtx));
extern rtx        gen_movdi                   PARAMS ((rtx, rtx));
extern rtx        gen_movsi                   PARAMS ((rtx, rtx));
extern rtx        gen_pic_load_addr_based     PARAMS ((rtx, rtx));
extern rtx        gen_builtin_setjmp_receiver PARAMS ((rtx));
extern rtx        gen_storehi                 PARAMS ((rtx, rtx));
extern rtx        gen_storehi_bigend          PARAMS ((rtx, rtx));
extern rtx        gen_storeinthi              PARAMS ((rtx, rtx));
extern rtx        gen_storehi_single_op       PARAMS ((rtx, rtx));
extern rtx        gen_movhi                   PARAMS ((rtx, rtx));
extern rtx        gen_movhi_bytes             PARAMS ((rtx, rtx));
extern rtx        gen_movhi_bigend            PARAMS ((rtx, rtx));
extern rtx        gen_reload_outhi            PARAMS ((rtx, rtx, rtx));
extern rtx        gen_reload_inhi             PARAMS ((rtx, rtx, rtx));
extern rtx        gen_movqi                   PARAMS ((rtx, rtx));
extern rtx        gen_movsf                   PARAMS ((rtx, rtx));
extern rtx        gen_movdf                   PARAMS ((rtx, rtx));
extern rtx        gen_reload_outdf            PARAMS ((rtx, rtx, rtx));
extern rtx        gen_movxf                   PARAMS ((rtx, rtx));
extern rtx        gen_load_multiple           PARAMS ((rtx, rtx, rtx));
extern rtx        gen_store_multiple          PARAMS ((rtx, rtx, rtx));
extern rtx        gen_movstrqi                PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_cmpsi                   PARAMS ((rtx, rtx));
extern rtx        gen_cmpsf                   PARAMS ((rtx, rtx));
extern rtx        gen_cmpdf                   PARAMS ((rtx, rtx));
extern rtx        gen_cmpxf                   PARAMS ((rtx, rtx));
extern rtx        gen_beq                     PARAMS ((rtx));
extern rtx        gen_bne                     PARAMS ((rtx));
extern rtx        gen_bgt                     PARAMS ((rtx));
extern rtx        gen_ble                     PARAMS ((rtx));
extern rtx        gen_bge                     PARAMS ((rtx));
extern rtx        gen_blt                     PARAMS ((rtx));
extern rtx        gen_bgtu                    PARAMS ((rtx));
extern rtx        gen_bleu                    PARAMS ((rtx));
extern rtx        gen_bgeu                    PARAMS ((rtx));
extern rtx        gen_bltu                    PARAMS ((rtx));
extern rtx        gen_bunordered              PARAMS ((rtx));
extern rtx        gen_bordered                PARAMS ((rtx));
extern rtx        gen_bungt                   PARAMS ((rtx));
extern rtx        gen_bunlt                   PARAMS ((rtx));
extern rtx        gen_bunge                   PARAMS ((rtx));
extern rtx        gen_bunle                   PARAMS ((rtx));
extern rtx        gen_buneq                   PARAMS ((rtx));
extern rtx        gen_bltgt                   PARAMS ((rtx));
extern rtx        gen_seq                     PARAMS ((rtx));
extern rtx        gen_sne                     PARAMS ((rtx));
extern rtx        gen_sgt                     PARAMS ((rtx));
extern rtx        gen_sle                     PARAMS ((rtx));
extern rtx        gen_sge                     PARAMS ((rtx));
extern rtx        gen_slt                     PARAMS ((rtx));
extern rtx        gen_sgtu                    PARAMS ((rtx));
extern rtx        gen_sleu                    PARAMS ((rtx));
extern rtx        gen_sgeu                    PARAMS ((rtx));
extern rtx        gen_sltu                    PARAMS ((rtx));
extern rtx        gen_sunordered              PARAMS ((rtx));
extern rtx        gen_sordered                PARAMS ((rtx));
extern rtx        gen_sungt                   PARAMS ((rtx));
extern rtx        gen_sunge                   PARAMS ((rtx));
extern rtx        gen_sunlt                   PARAMS ((rtx));
extern rtx        gen_sunle                   PARAMS ((rtx));
extern rtx        gen_movsicc                 PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_movsfcc                 PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_movdfcc                 PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_jump                    PARAMS ((rtx));
#define GEN_CALL(A, B, C, D) gen_call ((A), (B), (C))
extern rtx        gen_call                    PARAMS ((rtx, rtx, rtx));
#define GEN_CALL_VALUE(A, B, C, D, E) gen_call_value ((A), (B), (C), (D))
extern rtx        gen_call_value              PARAMS ((rtx, rtx, rtx, rtx));
#define GEN_SIBCALL(A, B, C, D) gen_sibcall ((A), (B), (C))
extern rtx        gen_sibcall                 PARAMS ((rtx, rtx, rtx));
#define GEN_SIBCALL_VALUE(A, B, C, D, E) gen_sibcall_value ((A), (B), (C), (D))
extern rtx        gen_sibcall_value           PARAMS ((rtx, rtx, rtx, rtx));
extern rtx        gen_return_addr_mask        PARAMS ((rtx));
extern rtx        gen_untyped_call            PARAMS ((rtx, rtx, rtx));
extern rtx        gen_casesi                  PARAMS ((rtx, rtx, rtx, rtx, rtx));
extern rtx        gen_indirect_jump           PARAMS ((rtx));
extern rtx        gen_prologue                PARAMS ((void));
extern rtx        gen_epilogue                PARAMS ((void));
extern rtx        gen_eh_epilogue             PARAMS ((rtx, rtx, rtx));
extern rtx        gen_tablejump               PARAMS ((rtx, rtx));
extern rtx        gen_ffssi2                  PARAMS ((rtx, rtx));

#endif /* GCC_INSN_FLAGS_H */
