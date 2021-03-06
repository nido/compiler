project(libmpath)

IF(PATH64_BUILD_32)

SET(PATHSCALE_MPATH_SRCS dummy.c)
SET(PATHSCALE_MPATH_S_SRCS )

ELSE()

SET(PATHSCALE_MPATH_SRCS        "amd/pow_errorl.c"
				"amd/remainder_piby2d2f.c"
				"amd/s_atan2.c"
				"amd/s_atan2f.c"
				"amd/s_atan.c"
				"amd/s_atanf.c"
				"amd/s_ceil.c"
				"amd/s_ceilf.c"
				"amd/s_fdim.c"
				"amd/s_fdimf.c"
				"amd/s_floor.c"
				"amd/s_floorf.c"
				"amd/s_fma.c"
				"amd/s_fmaf.c"
				"amd/s_logb.c"
				"amd/s_logbf.c"
				"amd/s_modf.c"
				"amd/s_modff.c"
				"amd/s_tan.c"
				"amd/s_tanf.c"
				"amd/s_trunc.c"
				"amd/s_truncf.c"
				"amd/w_acos.c"
				"amd/w_acosf.c"
				"amd/w_asin.c"
				"amd/w_asinf.c"
				"amd/w_cosh.c"
				"amd/w_coshf.c"
				"amd/w_exp10.c"
				"amd/w_exp10f.c"
				"amd/w_exp2.c"
				"amd/w_exp2f.c"
				"amd/w_exp.c"
				"amd/w_expf.c"
				"amd/w_hypot.c"
				"amd/w_hypotf.c"
				"amd/w_log2.c"
				"amd/w_log2f.c"
				"amd/w_log10.c"
				"amd/w_log10f.c"
				"amd/w_powf.c"
				"amd/w_remainder.c"
				"amd/w_remainderf.c"
				"amd/w_remainder_piby2.c"
				"amd/w_remainder_piby2f.c"
				"amd/w_sincos.c"
				"amd/w_sincosf.c"
				"amd/w_sinh.c"
				"amd/w_sinhf.c"
				"amd/w_tanh.c"
				"amd/w_tanhf.c"
				"cisf.c"
				"cisf.c")

SET(PATHSCALE_MPATH_S_SRCS      "amd/w_copysignf.s"	
				"amd/w_copysign.s"
				"amd/w_finitef.s"
				"amd/w_finite.s"
				"amd/w_fmaxf.s"
				"amd/w_fmax.s"
				"amd/w_fminf.s"
				"amd/w_fmin.s"
				"amd/w_fastcos.S"
				"amd/w_fastexp.s"
				"amd/w_fastexpf.s"
				"amd/w_fastlog.s"
				"amd/w_fastlogf.s"
				"amd/w_fastlog10.s"
				"amd/w_fastpow.s"
				"amd/w_fastpowf.s"
				"amd/w_fastsin.S"
				"amd/w_fastsincos.S"
				"amd/w_pow.S")

ENDIF()


INCLUDE_DIRECTORIES(
    ../libmpath/amd
    ../libm
)

ADD_DEFINITIONS(
	-D_IP_NAN_SETS_ERRNO
	-D_HDW_SQRT
	-D_libm_weak=1)

IF(PATH64_ENABLE_MATHLIBS)

set_source_files_properties("amd/w_log2.c" "amd/w_log2f.c" PROPERTIES COMPILE_DEFINITIONS "COMPILING_LOG2")
set_source_files_properties("amd/w_log10.c" "amd/w_log10f.c" PROPERTIES COMPILE_DEFINITIONS "COMPILING_LOG10")

set_source_files_properties(${PATHSCALE_MPATH_S_SRCS} PROPERTIES LANGUAGE C)
add_library(mpath-static STATIC ${PATHSCALE_MPATH_SRCS} ${PATHSCALE_MPATH_S_SRCS})
SET_TARGET_PROPERTIES(mpath-static PROPERTIES OUTPUT_NAME "mpath")

add_library(mpath SHARED ${PATHSCALE_MPATH_SRCS} ${PATHSCALE_MPATH_S_SRCS})
set_property(TARGET mpath PROPERTY COMPILE_DEFINITIONS PIC)
set_property(TARGET mpath PROPERTY SOVERSION 1)
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
set_property(TARGET mpath PROPERTY LINK_FLAGS "-Wl,-x -Wl,--version-script=${libmpath_SOURCE_DIR}/libmpath.vs")
endif()

target_link_libraries(mpath m)

install(TARGETS mpath-static mpath
        LIBRARY DESTINATION ${PATH64_LIB_PATH}
        ARCHIVE DESTINATION ${PATH64_LIB_PATH})


IF(PATH64_ENABLE_PROFILING)
add_library(mpath_p STATIC ${PATHSCALE_MPATH_SRCS} ${PATHSCALE_MPATH_S_SRCS})
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
set_property(TARGET mpath_p PROPERTY COMPILE_FLAGS -pg)
endif()
install(TARGETS mpath_p
        LIBRARY DESTINATION ${PATH64_LIB_PATH}
        ARCHIVE DESTINATION ${PATH64_LIB_PATH})
ENDIF(PATH64_ENABLE_PROFILING)

ENDIF(PATH64_ENABLE_MATHLIBS)
