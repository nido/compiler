# this should build coco
SET(PATHSCALE_COCO_SRCS "coco.F90" )

set_source_files_properties("coco.F90" PROPERTIES LANGUAGE Fortran)
if(PATH64_ENABLE_FORTRAN)
add_executable (coco ${PATHSCALE_COCO_SRCS})
add_dependencies(coco pathfortran pathfstart mv mpath)
install(TARGETS coco
    RUNTIME DESTINATION ${PATH64_LIB_PATH})

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/lib/coco
    DEPENDS coco
    COMMAND ln -s ../bin/coco ${CMAKE_BINARY_DIR}/lib
)

add_custom_target(
    stageit-coco ALL
    DEPENDS ${CMAKE_BINARY_DIR}/lib/coco
)

endif()
