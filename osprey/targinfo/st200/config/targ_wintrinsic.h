
  /* Integer Arithmetic Support */

  INTRN_ADDL       = INTRINSIC_GENERAL_LAST+1,
  INTRN_ADDUL      = INTRINSIC_GENERAL_LAST+2,
  INTRN_SUBL       = INTRINSIC_GENERAL_LAST+3,
  INTRN_SUBUL      = INTRINSIC_GENERAL_LAST+4,
  INTRN_MULW       = INTRINSIC_GENERAL_LAST+5,
  INTRN_MULUW      = INTRINSIC_GENERAL_LAST+6,
  INTRN_DIVW       = INTRINSIC_GENERAL_LAST+7,
  INTRN_DIVUW      = INTRINSIC_GENERAL_LAST+8,
  INTRN_MULL       = INTRINSIC_GENERAL_LAST+9,
  INTRN_MULUL      = INTRINSIC_GENERAL_LAST+10,
  INTRN_DIVL       = INTRINSIC_GENERAL_LAST+11,
  INTRN_DIVUL      = INTRINSIC_GENERAL_LAST+12,
  INTRN_MODW       = INTRINSIC_GENERAL_LAST+13,
  INTRN_MODUW      = INTRINSIC_GENERAL_LAST+14,
  INTRN_MODL       = INTRINSIC_GENERAL_LAST+15,
  INTRN_MODUL      = INTRINSIC_GENERAL_LAST+16,
  INTRN_SHLL       = INTRINSIC_GENERAL_LAST+17,
  INTRN_SHRL       = INTRINSIC_GENERAL_LAST+18,
  INTRN_SHRUL      = INTRINSIC_GENERAL_LAST+19,
  INTRN_NEGL       = INTRINSIC_GENERAL_LAST+20,
  INTRN_NEGUL      = INTRINSIC_GENERAL_LAST+21,
  INTRN_EQL        = INTRINSIC_GENERAL_LAST+22,
  INTRN_EQUL       = INTRINSIC_GENERAL_LAST+23,
  INTRN_NEL        = INTRINSIC_GENERAL_LAST+24,
  INTRN_NEUL       = INTRINSIC_GENERAL_LAST+25,
  INTRN_LTL        = INTRINSIC_GENERAL_LAST+26,
  INTRN_LTUL       = INTRINSIC_GENERAL_LAST+27,
  INTRN_LEL        = INTRINSIC_GENERAL_LAST+28,
  INTRN_LEUL       = INTRINSIC_GENERAL_LAST+29,
  INTRN_GTL        = INTRINSIC_GENERAL_LAST+30,
  INTRN_GTUL       = INTRINSIC_GENERAL_LAST+31,
  INTRN_GEL        = INTRINSIC_GENERAL_LAST+32,
  INTRN_GEUL       = INTRINSIC_GENERAL_LAST+33,

  /* Floating-point Arithmetic Support */
  INTRN_ADDS       = INTRINSIC_GENERAL_LAST+34,
  INTRN_ADDD       = INTRINSIC_GENERAL_LAST+35,
  INTRN_SUBS       = INTRINSIC_GENERAL_LAST+36,
  INTRN_SUBD       = INTRINSIC_GENERAL_LAST+37,
  INTRN_MULS       = INTRINSIC_GENERAL_LAST+38,
  INTRN_MULD       = INTRINSIC_GENERAL_LAST+39,
  INTRN_DIVS       = INTRINSIC_GENERAL_LAST+40,
  INTRN_DIVD       = INTRINSIC_GENERAL_LAST+41,
  INTRN_EQS        = INTRINSIC_GENERAL_LAST+42,
  INTRN_EQD        = INTRINSIC_GENERAL_LAST+43,
  INTRN_NES        = INTRINSIC_GENERAL_LAST+44,
  INTRN_NED        = INTRINSIC_GENERAL_LAST+45,
  INTRN_LTS        = INTRINSIC_GENERAL_LAST+46,
  INTRN_LTD        = INTRINSIC_GENERAL_LAST+47,
  INTRN_LES        = INTRINSIC_GENERAL_LAST+48,
  INTRN_LED        = INTRINSIC_GENERAL_LAST+49,
  INTRN_GTS        = INTRINSIC_GENERAL_LAST+50,
  INTRN_GTD        = INTRINSIC_GENERAL_LAST+51,
  INTRN_GES        = INTRINSIC_GENERAL_LAST+52,
  INTRN_GED        = INTRINSIC_GENERAL_LAST+53,

  INTRN_DTOS       = INTRINSIC_GENERAL_LAST+54,
  INTRN_STOD       = INTRINSIC_GENERAL_LAST+55,

  INTRN_WTOS       = INTRINSIC_GENERAL_LAST+56,
  INTRN_UWTOS      = INTRINSIC_GENERAL_LAST+57,
  INTRN_LTOS       = INTRINSIC_GENERAL_LAST+58,
  INTRN_ULTOS      = INTRINSIC_GENERAL_LAST+59,
  INTRN_STOW       = INTRINSIC_GENERAL_LAST+60,
  INTRN_STOUW      = INTRINSIC_GENERAL_LAST+61,
  INTRN_STOL       = INTRINSIC_GENERAL_LAST+62,
  INTRN_STOUL      = INTRINSIC_GENERAL_LAST+63,

  INTRN_WTOD       = INTRINSIC_GENERAL_LAST+64,
  INTRN_UWTOD      = INTRINSIC_GENERAL_LAST+65,
  INTRN_LTOD       = INTRINSIC_GENERAL_LAST+66,
  INTRN_ULTOD      = INTRINSIC_GENERAL_LAST+67,
  INTRN_DTOW       = INTRINSIC_GENERAL_LAST+68,
  INTRN_DTOUW      = INTRINSIC_GENERAL_LAST+69,
  INTRN_DTOL       = INTRINSIC_GENERAL_LAST+70,
  INTRN_DTOUL      = INTRINSIC_GENERAL_LAST+71,

  /* more run-time */
  INTRN_MINL       = INTRINSIC_GENERAL_LAST+72,
  INTRN_MINUL      = INTRINSIC_GENERAL_LAST+73,
  INTRN_MAXL       = INTRINSIC_GENERAL_LAST+74,
  INTRN_MAXUL      = INTRINSIC_GENERAL_LAST+75,

  INTRN_ABSL       = INTRINSIC_GENERAL_LAST+76,

  /* These are part of run-time support provided on the ST200
   * targets. At the run-time lowering stage the OPR_SQRT
   * WHIRL operators become these, so we can properly generate
   * code for them */
  INTRN_SQRTS       = INTRINSIC_GENERAL_LAST+77,
  INTRN_SQRTD       = INTRINSIC_GENERAL_LAST+78,

  /* INTRN_LAST_ST200_INTRINSIC */
  INTRINSIC_TARG_LAST  = INTRINSIC_GENERAL_LAST+78,



