project(PSE-CLIs CXX)
cmake_minimum_required(VERSION 3.9)

set(OpenMP_C_DEPENDENCY)
if(OpenMP_C_FOUND)
  set(OpenMP_C_DEPENDENCY OpenMP::OpenMP_C)
endif()

set(OpenMP_CXX_DEPENDENCY)
if(OpenMP_CXX_FOUND)
  set(OpenMP_CXX_DEPENDENCY OpenMP::OpenMP_CXX)
endif()

get_filename_component(PSE_CLIS_ROOT_SRC_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/../../src/cli"
  REALPATH
)

include_directories(${EIGEN3_INCLUDE_DIR})

function(add_source_cli NAME)
  add_custom_target(SOURCES_${NAME} SOURCES "${ARGN}")
endfunction()

function(pse_add_cli NAME)
  cmake_parse_arguments(PSE "" "" "SOURCES;DEPENDENCIES" ${ARGN})

  add_executable(${NAME} "${PSE_SOURCES}")
  set_property(TARGET ${NAME} PROPERTY C_STANDARD 90)
  target_link_libraries(${NAME} PRIVATE ${PSE_DEPENDENCIES})
  if(PSE_ENABLE_INTRINSICS_OPTIM)
    enable_avx(${NAME})
  endif()
endfunction()

add_source_cli(pse_color_explore
  "${PSE_CLIS_ROOT_SRC_DIR}/pse_color_explore.cpp"
)

if(PSE_BUILD_CLT_SPACE_COLOR)
  pse_add_cli(pse_space_color_exploration
    SOURCES
      "${PSE_CLIS_ROOT_SRC_DIR}/pse_cli_space_color_exploration.c"
    DEPENDENCIES
      PSE::pse-clt-space-color
  )
  if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(pse_space_color_exploration PRIVATE m)
  endif()
endif()

