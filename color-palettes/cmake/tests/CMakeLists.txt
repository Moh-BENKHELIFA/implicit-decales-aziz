project(PSE-Tests CXX)
cmake_minimum_required(VERSION 3.9)

set(OpenMP_C_DEPENDENCY)
if(OpenMP_C_FOUND)
  set(OpenMP_C_DEPENDENCY OpenMP::OpenMP_C)
endif()

set(OpenMP_CXX_DEPENDENCY)
if(OpenMP_CXX_FOUND)
  set(OpenMP_CXX_DEPENDENCY OpenMP::OpenMP_CXX)
endif()

get_filename_component(PSE_TESTS_ROOT_SRC_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/../../src/test"
  REALPATH
)

function(deprecated_test NAME)
  add_custom_target(DISABLED_${NAME}_files SOURCES "${ARGN}")
  if(FALSE) # disabled
    add_executable(${NAME} "${ARGN}")
    target_compile_features(${NAME} INTERFACE cxx_std_14)
    target_link_libraries(${NAME} PUBLIC PSE::pse-cmn PRIVATE Eigen::Eigen3)
    if(PSE_ENABLE_INTRINSICS_OPTIM)
      enable_avx(${NAME})
    endif()
  endif()
endfunction()

function(pse_add_test_executable NAME)
  add_executable(${NAME} "${ARGN}")
  target_link_libraries(${NAME} PUBLIC pse ${OpenMP_C_DEPENDENCY})
endfunction()
function(pse_add_test)
  cmake_parse_arguments(PSE "" "NAME" "COMMAND" ${ARGN})
  add_test(NAME ${PSE_NAME} COMMAND ${PSE_COMMAND})
  set_tests_properties(${PSE_NAME} PROPERTIES
    ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/libs
  )
endfunction()

if(PSE_BUILD_API)
  pse_add_test_executable(test_api
    "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_api.c"
  )
  set_property(TARGET test_api PROPERTY C_STANDARD 90)
  pse_add_test(NAME test_api COMMAND test_api)

  pse_add_test_executable(test_api_exploration
    "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_api_exploration.c"
  )
  set_property(TARGET test_api_exploration PROPERTY C_STANDARD 90)
  if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(test_api_exploration PRIVATE m)
  endif()
  pse_add_test(NAME test_api_exploration COMMAND test_api_exploration)
endif()

if(PSE_BUILD_CLT)
  pse_add_test_executable(test_clt_api
    "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_clt_api.c"
  )
  set_property(TARGET test_clt_api PROPERTY C_STANDARD 90)
  target_link_libraries(test_clt_api PRIVATE PSE::pse-clt)
  pse_add_test(NAME test_clt_api COMMAND test_clt_api)
endif()

if(PSE_BUILD_CLT_SPACE_COLOR)
  if(CMAKE_COMPILER_IS_GNUCC)
    pse_add_test_executable(test_clt_space_color_api
      "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_clt_space_color_api.c"
    )
    set_property(TARGET test_clt_space_color_api PROPERTY C_STANDARD 90)
    target_link_libraries(test_clt_space_color_api PRIVATE PSE::pse-clt-space-color)
    if(CMAKE_COMPILER_IS_GNUCC)
      target_link_libraries(test_clt_space_color_api PRIVATE m)
    endif()
  endif()
endif()


deprecated_test(test_levenberg
  "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_levenberg.cpp"
)

deprecated_test(test_bezier
  "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_bezier.cpp"
)

deprecated_test(test_literal_string
  "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_literal_string.cpp"
)

deprecated_test(test_cps
  "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_cps.cpp"
)

if(PSE_BUILD_CLT_SPACE_COLOR)
  deprecated_test(test_color_solvers_perfs
    "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_color_solvers_perfs.cpp"
  )

  deprecated_test(test_color_poly_interpolation
    "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_color_poly_interpolation.cpp"
    "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_color_problem.hpp"
    "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_color_problem.inl"
  )

  if(YAML_FOUND)
    deprecated_test(test_color_serialization
      "${PSE_TESTS_ROOT_SRC_DIR}/test_pse_color_serialization.cpp"
    )
  endif()
endif()
