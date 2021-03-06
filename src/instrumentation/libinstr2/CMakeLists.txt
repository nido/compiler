project(libinstr2)

SET(PATHSCALE_INSTR2_SRCS       "dump.cxx"
				"profile.cxx"
				"../../common/instrument/profile_errors.cxx"
				"profile_interface.cxx")

INCLUDE_DIRECTORIES(
	.
	../../common/instrument
	../../common/com
	../include)

add_definitions(${PATH64_STDC_FORMAT_MACROS})

add_library(instr-static STATIC ${PATHSCALE_INSTR2_SRCS})

add_library(instr-shared SHARED ${PATHSCALE_INSTR2_SRCS})

set_property(TARGET instr-shared PROPERTY SOVERSION 1)
IF(NOT "${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
set_property(TARGET instr-shared PROPERTY LINK_FLAGS -Wl,--version-script="${libinstr2_SOURCE_DIR}/libinstr.vs")
ENDIF()
SET_TARGET_PROPERTIES(instr-shared PROPERTIES OUTPUT_NAME "instr2")
SET_TARGET_PROPERTIES(instr-static PROPERTIES OUTPUT_NAME "instr2")

IF(PATH64_ENABLE_PROFILING)
add_library(instr_p STATIC ${PATHSCALE_INSTR2_SRCS})
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "SunOS")   # temporary workaround for bug in gcc on osol
set_property(TARGET instr_p PROPERTY COMPILE_FLAGS -pg)
endif()
SET_TARGET_PROPERTIES(instr_p PROPERTIES OUTPUT_NAME "instr2_p")
install(TARGETS instr_p DESTINATION ${PATH64_LIB_PATH})
ENDIF()

install(TARGETS instr-shared instr-static DESTINATION ${PATH64_LIB_PATH})
