include_directories(
  ${PATHSCALE_SOURCE_DIR}/common/util
  ${PATHSCALE_SOURCE_DIR}/common/com
  ${PATHSCALE_SOURCE_DIR}/common/com/${BUILD_TARGET_DIR}
  )

IF(NOT PATH64_ENABLE_VALGRIND)
add_definitions(
  -DNO_VALGRIND
  ${PATH64_STDC_FORMAT_MACROS}
 )
ENDIF(NOT PATH64_ENABLE_VALGRIND)

add_library(comutil STATIC
  cxx_memory.cxx
  errors.cxx
  bitset.c
  c_q_rel.c
  file_util.c
  flags.c
  linklist.c
  memory.c
  priority_queue.c
  quadsim.c
  resource.c
  mstack.c
  tracing.c
  util.c
  vstring.c
  c_a_to_q.c
  c_q_to_a.c
  c_qtenscale.c
  ${BUILD_TARGET_DIR}/c_qwmultu.c
  )
