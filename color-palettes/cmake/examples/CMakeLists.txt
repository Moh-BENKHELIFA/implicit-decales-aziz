project(PSE-Examples CXX)
cmake_minimum_required(VERSION 3.9)

set(OpenMP_C_DEPENDENCY)
if(OpenMP_C_FOUND)
  set(OpenMP_C_DEPENDENCY OpenMP::OpenMP_C)
endif()

set(OpenMP_CXX_DEPENDENCY)
if(OpenMP_CXX_FOUND)
  set(OpenMP_CXX_DEPENDENCY OpenMP::OpenMP_CXX)
endif()

get_filename_component(PSE_EXAMPLES_ROOT_SRC_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/../../src/examples"
  REALPATH
)

function(pse_add_example_executable NAME)
  add_executable(${NAME} "${ARGN}")
  target_link_libraries(${NAME} PUBLIC pse ${OpenMP_C_DEPENDENCY})
endfunction()

if(PSE_BUILD_API)
  pse_add_example_executable(example1_basics
    "${PSE_EXAMPLES_ROOT_SRC_DIR}/example1_basics.c"
  )
  set_property(TARGET example1_basics PROPERTY C_STANDARD 90)
  if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(example1_basics PRIVATE m)
  endif()
endif()
