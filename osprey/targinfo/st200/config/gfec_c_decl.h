#if defined(TARG_ST200)
builtin_function ("__absch",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node, endlink)),
                   BUILT_IN_ABSCH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__abscl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_ABSCL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__abscw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_ABSCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__absh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node, endlink)),
                   BUILT_IN_ABSH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__absl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_ABSL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__absw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_ABSW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__addch",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_ADDCH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__addcl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_ADDCL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__addcw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_ADDCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__addd",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_ADDD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__addl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_ADDL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__adds",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_ADDS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__addul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_ADDUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitclrh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_BITCLRH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitclrw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_BITCLRW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitcnth",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink)),
                   BUILT_IN_BITCNTH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitcntw",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)),
                   BUILT_IN_BITCNTW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitnoth",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_BITNOTH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitnotw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_BITNOTW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitrevw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_BITREVW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitseth",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_BITSETH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitsetw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_BITSETW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitvalh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_BITVALH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__bitvalw",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_BITVALW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__clamplw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_CLAMPLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__clampwh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_CLAMPWH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__disth",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_DISTH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__distuh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_DISTUH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__distuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_DISTUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__distw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_DISTW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divd",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_DIVD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divfch",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_DIVFCH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divfcm",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_DIVFCM, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divfcw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_DIVFCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_DIVH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_DIVL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divm",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_DIVM, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divs",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_DIVS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divuh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_DIVUH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_DIVUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divum",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_DIVUM, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_DIVUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__divw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_DIVW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__dtol",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, double_type_node, endlink)),
                   BUILT_IN_DTOL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__dtos",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, double_type_node, endlink)),
                   BUILT_IN_DTOS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__dtoul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, double_type_node, endlink)),
                   BUILT_IN_DTOUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__dtouw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, double_type_node, endlink)),
                   BUILT_IN_DTOUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__dtow",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, double_type_node, endlink)),
                   BUILT_IN_DTOW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__edgesh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_EDGESH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__edgesw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_EDGESW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__eqd",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_EQD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__eql",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_EQL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__eqs",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_EQS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__equl",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_EQUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ged",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_GED, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__gel",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_GEL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ges",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_GES, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__gethh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_GETHH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__gethw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_GETHW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__getlh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_GETLH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__getlw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_GETLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__geul",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_GEUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__gtd",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_GTD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__gtl",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_GTL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__gts",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_GTS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__gtul",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_GTUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insequw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)))),
                   BUILT_IN_INSEQUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__inseqw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink)))),
                   BUILT_IN_INSEQW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insgeuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)))),
                   BUILT_IN_INSGEUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insgew",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink)))),
                   BUILT_IN_INSGEW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insgtuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)))),
                   BUILT_IN_INSGTUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insgtw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink)))),
                   BUILT_IN_INSGTW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insleuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)))),
                   BUILT_IN_INSLEUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__inslew",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink)))),
                   BUILT_IN_INSLEW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insltuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)))),
                   BUILT_IN_INSLTUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insltw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink)))),
                   BUILT_IN_INSLTW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insneuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)))),
                   BUILT_IN_INSNEUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__insnew",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink)))),
                   BUILT_IN_INSNEW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__led",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_LED, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__lel",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_LEL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__les",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_LES, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__leul",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_LEUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ltd",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_LTD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ltl",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_LTL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ltod",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_LTOD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ltos",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_LTOS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__lts",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_LTS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ltul",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_LTUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__lzcnth",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node, endlink)),
                   BUILT_IN_LZCNTH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__lzcntl",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_LZCNTL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__lzcntw",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_LZCNTW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__madds",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink)))),
                   BUILT_IN_MADDS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mafcw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink)))),
                   BUILT_IN_MAFCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__maxd",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_MAXD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__maxh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MAXH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__maxl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_MAXL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__maxs",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_MAXS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__maxuh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_MAXUH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__maxul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_MAXUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__maxuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_MAXUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__maxw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_MAXW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mind",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_MIND, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__minh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MINH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__minl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_MINL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mins",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_MINS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__minuh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_MINUH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__minul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_MINUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__minuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_MINUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__minw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_MINW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__modfch",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MODFCH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__modfcm",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MODFCM, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__modfcw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_MODFCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__modh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MODH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__modl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_MODL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__modm",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MODM, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__moduh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_MODUH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__modul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_MODUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__modum",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_MODUM, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__moduw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_MODUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__modw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_MODW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mpfcw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MPFCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mpfcwl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_MPFCWL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mpfml",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MPFML, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mpfrch",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MPFRCH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mpml",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MPML, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mpuml",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_MPUML, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__msubs",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink)))),
                   BUILT_IN_MSUBS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__muld",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_MULD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulfch",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MULFCH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulfcm",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MULFCM, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulfcw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_MULFCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MULH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulhh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MULHH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulhuh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_MULHUH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulhuw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_MULHUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulhw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_MULHW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mull",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_MULL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulm",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_MULM, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__muln",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_MULN, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__muls",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_MULS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__muluh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_MULUH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_MULUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulum",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_MULUM, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulun",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_MULUN, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__muluw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_MULUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__mulw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_MULW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__nearclw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_NEARCLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__nearcwh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_NEARCWH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__nearlw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_NEARLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__nearwh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_NEARWH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ned",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_NED, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__negch",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node, endlink)),
                   BUILT_IN_NEGCH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__negcl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_NEGCL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__negcw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_NEGCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__negl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_NEGL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__negul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink)),
                   BUILT_IN_NEGUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__nel",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_NEL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__nes",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_NES, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__neul",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_NEUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__normh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node, endlink)),
                   BUILT_IN_NORMH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__norml",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_NORML, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__normw",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_NORMW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__priorh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node, endlink)),
                   BUILT_IN_PRIORH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__priorl",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_PRIORL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__priorw",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_PRIORW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__puthl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_PUTHL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__puthw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node, endlink)),
                   BUILT_IN_PUTHW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__putll",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_PUTLL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__putlw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node, endlink)),
                   BUILT_IN_PUTLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__rotlh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_ROTLH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__rotlw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_ROTLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__roundclw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_ROUNDCLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__roundcwh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_ROUNDCWH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__roundlw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_ROUNDLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__roundwh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_ROUNDWH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shlch",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHLCH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shlcw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHLCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shll",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHLL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shlul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHLUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shlw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shrl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHRL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shrrh",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHRRH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shrrw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHRRW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shrul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHRUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shruw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHRUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__shrw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink))),
                   BUILT_IN_SHRW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__sqrtd",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, double_type_node, endlink)),
                   BUILT_IN_SQRTD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__sqrts",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, float_type_node, endlink)),
                   BUILT_IN_SQRTS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220addcg",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)))),
                   BUILT_IN_ST220ADDCG, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220divs",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)))),
                   BUILT_IN_ST220DIVS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220mulhhs",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_ST220MULHHS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220mulhs",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_ST220MULHS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220mullhus",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_ST220MULLHUS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220mullu",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node, endlink))),
                   BUILT_IN_ST220MULLU, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220pft",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_ST220PFT, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220prgadd",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_ST220PRGADD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220prgins",
                   build_function_type( void_type_node, 
                     tree_cons(NULL_TREE, void_type_node, endlink)),
                   BUILT_IN_ST220PRGINS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220prgset",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_ST220PRGSET, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220sync",
                   build_function_type( void_type_node, 
                     tree_cons(NULL_TREE, void_type_node, endlink)),
                   BUILT_IN_ST220SYNC, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220syncins",
                   build_function_type( void_type_node, 
                     tree_cons(NULL_TREE, void_type_node, endlink)),
                   BUILT_IN_ST220SYNCINS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__st220syscall",
                   build_function_type( void_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_ST220SYSCALL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__stod",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, float_type_node, endlink)),
                   BUILT_IN_STOD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__stol",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, float_type_node, endlink)),
                   BUILT_IN_STOL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__stoul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, float_type_node, endlink)),
                   BUILT_IN_STOUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__stouw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, float_type_node, endlink)),
                   BUILT_IN_STOUW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__stow",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, float_type_node, endlink)),
                   BUILT_IN_STOW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__subch",
                   build_function_type( short_integer_type_node, 
                     tree_cons(NULL_TREE, short_integer_type_node,
                     tree_cons(NULL_TREE, short_integer_type_node, endlink))),
                   BUILT_IN_SUBCH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__subcl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_SUBCL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__subcw",
                   build_function_type( integer_type_node, 
                     tree_cons(NULL_TREE, integer_type_node,
                     tree_cons(NULL_TREE, integer_type_node, endlink))),
                   BUILT_IN_SUBCW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__subd",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, double_type_node,
                     tree_cons(NULL_TREE, double_type_node, endlink))),
                   BUILT_IN_SUBD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__subl",
                   build_function_type( long_long_integer_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node,
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink))),
                   BUILT_IN_SUBL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__subs",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, float_type_node,
                     tree_cons(NULL_TREE, float_type_node, endlink))),
                   BUILT_IN_SUBS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__subul",
                   build_function_type( long_long_unsigned_type_node, 
                     tree_cons(NULL_TREE, long_long_unsigned_type_node,
                     tree_cons(NULL_TREE, long_long_unsigned_type_node, endlink))),
                   BUILT_IN_SUBUL, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__swapbh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink)),
                   BUILT_IN_SWAPBH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__swapbw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)),
                   BUILT_IN_SWAPBW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__swaphw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)),
                   BUILT_IN_SWAPHW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ultod",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_ULTOD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__ultos",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, long_long_integer_type_node, endlink)),
                   BUILT_IN_ULTOS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__uwtod",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)),
                   BUILT_IN_UWTOD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__uwtos",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node, endlink)),
                   BUILT_IN_UWTOS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__wtod",
                   build_function_type( double_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_WTOD, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__wtos",
                   build_function_type( float_type_node, 
                     tree_cons(NULL_TREE, integer_type_node, endlink)),
                   BUILT_IN_WTOS, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__xshlh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink)))),
                   BUILT_IN_XSHLH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__xshlw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink)))),
                   BUILT_IN_XSHLW, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__xshrh",
                   build_function_type( short_unsigned_type_node, 
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink)))),
                   BUILT_IN_XSHRH, BUILT_IN_NORMAL, NULL_PTR);

builtin_function ("__xshrw",
                   build_function_type( unsigned_type_node, 
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, unsigned_type_node,
                     tree_cons(NULL_TREE, short_unsigned_type_node, endlink)))),
                   BUILT_IN_XSHRW, BUILT_IN_NORMAL, NULL_PTR);

#endif /* defined(TARG_ST200) */
