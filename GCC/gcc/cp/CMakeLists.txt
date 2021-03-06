# Restore the real executable output path.  We want to put our executable
# there.
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GCC_EXECUTABLE_OUTPUT_PATH})

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

#-----------------------------------------------------------------------------
# C++ Compiler Executable

IF(CMAKE_SYSTEM MATCHES "HP-UX")
  IF(CMAKE_C_COMPILER MATCHES "(^cc$|/cc$)")
    SET_SOURCE_FILES_PROPERTIES(../c-opts.c PROPERTIES COMPILE_FLAGS "-Wp,-H,20000")
  ENDIF(CMAKE_C_COMPILER MATCHES "(^cc$|/cc$)")
ENDIF(CMAKE_SYSTEM MATCHES "HP-UX")

#-----------------------------------------------------------------------------
# Special flags for some source files.

IF(CYGWIN)
  SET_SOURCE_FILES_PROPERTIES(../cppdefault.c PROPERTIES
    COMPILE_FLAGS "-DTOOL_INCLUDE_DIR=/usr/include")
ENDIF(CYGWIN)

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
CONFIGURE_FILE(${GCC_SOURCE_DIR}/gcc/path64_prefix.h.in
               ${GCC_BINARY_DIR}/gcc/cp/path64_prefix.h @ONLY IMMEDIATE)

ADD_EXECUTABLE(cc1plus42
     ${cxx_target_srcs}         ../attribs.c        ../c-common.c    ../c-cppbuiltin.c
            ../c-dump.c        ../c-format.c      ../c-gimplify.c       ../c-incpath.c
             ../c-lex.c           ../c-omp.c          ../c-opts.c           ../c-pch.c
        ../c-ppoutput.c        ../c-pragma.c  ../c-pretty-print.c     ../c-semantics.c
        ../cppdefault.c  ../dummy-checksum.c            ../main.c          ../prefix.c
         ../stub-objc.c     ../tree-inline.c    ../tree-mudflap.c               call.c
                class.c        cp-gimplify.c            cp-lang.c    cp-objcp-common.c
                  cvt.c   cxx-pretty-print.c               decl.c              decl2.c
                 dump.c              error.c             except.c               expr.c
               friend.c               init.c                lex.c             mangle.c
               method.c        name-lookup.c           optimize.c             parser.c
                   pt.c              ptree.c               repo.c               rtti.c
               search.c          semantics.c               tree.c             typeck.c
              typeck2.c


   # source file count: 53
  )


TARGET_LINK_LIBRARIES(cc1plus42 backend cpp decnumber iberty spin)



# Use the "-lx" option to disable map file generation on Borland.
# This avoids the "Fatal: Access violation.  Link terminated." error.
IF(BORLAND)
  SET_TARGET_PROPERTIES(cc1plus42 PROPERTIES LINK_FLAGS "-lx")
ENDIF(BORLAND)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/lib/cc1plus42
    DEPENDS cc1plus42
    COMMAND ln -s ../bin/cc1plus42 ${CMAKE_BINARY_DIR}/lib
)

add_custom_target(
    stageit-cc1plus42 ALL
    DEPENDS ${CMAKE_BINARY_DIR}/lib/cc1plus42
)

# Install cc1plus42 next to the gcc executable.
INSTALL(TARGETS cc1plus42
  RUNTIME DESTINATION ${PATH64_GCC_INSTALL_PATH}
  ${PATH64_INSTALL_COMPONENT_RUNTIME_EXECUTABLE})
