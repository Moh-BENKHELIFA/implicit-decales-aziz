project(PSE-Libraries)
cmake_minimum_required (VERSION 3.9)
include(CMakePackageConfigHelpers)

get_filename_component(PSE_SRC_ROOT_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/../../src"
  REALPATH
)

set(PSE_COLORSPACE_SRC_DIR "${PSE_SRC_ROOT_DIR}/ColorSpace")
set(PSE_KDTREE_SRC_DIR "${PSE_SRC_ROOT_DIR}/KdTree")
set(PSE_SRC_DIR "${PSE_SRC_ROOT_DIR}/pse")
set(PSE_SERIAL_SRC_DIR "${PSE_SRC_DIR}/serialization")
set(PSE_COLOR_SRC_DIR "${PSE_SRC_DIR}/color")
set(PSE_COLOR_SERIAL_SRC_DIR "${PSE_COLOR_SRC_DIR}/serialization")

set(OpenMP_C_DEPENDENCY)
if(OpenMP_C_FOUND)
  set(OpenMP_C_DEPENDENCY OpenMP::OpenMP_C)
endif()

set(OpenMP_CXX_DEPENDENCY)
if(OpenMP_CXX_FOUND)
  set(OpenMP_CXX_DEPENDENCY OpenMP::OpenMP_CXX)
endif()

# =============================================================================
# Function use to add and configure a PSE library
# =============================================================================

function(pse_add_library NAME)
  cmake_parse_arguments(PSE
    "SHARED;STATIC"
    "NAMESPACE;VERSION;INSTALL_SUBDIR;SOURCE_SUBDIR;C_STANDARD;CXX_STANDARD;BUILD_MACRO"
    "PUBLIC_C_SOURCES;PRIVATE_C_SOURCES;PUBLIC_CXX_SOURCES;PRIVATE_CXX_SOURCES;PRIVATE_C_DEPENDENCIES;PUBLIC_C_DEPENDENCIES;PRIVATE_CXX_DEPENDENCIES;COMPILE_DEFINITIONS"
    ${ARGN}
  )

  # Check that we have all the information needed
  if(NOT PSE_PUBLIC_C_SOURCES AND NOT PSE_PUBLIC_CXX_SOURCES AND NOT PSE_PRIVATE_C_SOURCES AND NOT PSE_PRIVATE_CXX_SOURCES)
    message(FATAL_ERROR "No sources files provided for ${NAME}.")
  endif()
  if(NOT PSE_SHARED AND NOT PSE_STATIC)
    message(FATAL_ERROR "No library type provided for ${NAME}. Choose either SHARED or STATIC")
  endif()
  if(NOT PSE_VERSION)
    message(FATAL_ERROR "No version provided for ${NAME}.")
  endif()
  if(NOT PSE_BUILD_MACRO)
    message(FATAL_ERROR "No build macro defined for ${NAME}.")
  endif()

  set(PSE_ROOT_SOURCE_DIR "${PSE_SRC_ROOT_DIR}")
  set(PSE_SOURCE_DIR "${PSE_ROOT_SOURCE_DIR}")
  if(PSE_SOURCE_SUBDIR)
    set(PSE_SOURCE_DIR "${PSE_SOURCE_DIR}/${PSE_SOURCE_SUBDIR}")
  endif()

  set(PSE_ROOT_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
  set(PSE_INSTALL_DIR "${PSE_ROOT_INSTALL_DIR}")
  if(NOT PSE_INSTALL_SUBDIR)
    set(PSE_INSTALL_SUBDIR ${PSE_SOURCE_SUBDIR}) # use the same hierarchy
  endif()
  if(PSE_INSTALL_SUBDIR)
    set(PSE_INSTALL_DIR "${PSE_INSTALL_DIR}/${PSE_INSTALL_SUBDIR}")
  endif()

  set(PSE_LIB_TYPE SHARED)
  if(PSE_STATIC)
    set(PSE_LIB_TYPE STATIC)
  endif()
  if(NOT PSE_NAMESPACE)
    set(PSE_NAMESPACE PSE::)
  endif()
  if(NOT PSE_C_STANDARD)
    set(PSE_C_STANDARD 90)
  endif()
  if(NOT PSE_CXX_STANDARD)
    set(PSE_CXX_STANDARD 11)
  endif()

  set(PUBLIC_FILES_C__)
  foreach(F IN LISTS PSE_PUBLIC_C_SOURCES)
    list(APPEND PUBLIC_FILES_C__ "${PSE_SOURCE_DIR}/${F}")
  endforeach()
  set(PRIVATE_FILES_C__)
  foreach(F IN LISTS PSE_PRIVATE_C_SOURCES)
    list(APPEND PRIVATE_FILES_C__ "${PSE_SOURCE_DIR}/${F}")
  endforeach()
  set(PUBLIC_FILES_CXX__)
  foreach(F IN LISTS PSE_PUBLIC_CXX_SOURCES)
    list(APPEND PUBLIC_FILES_CXX__ "${PSE_SOURCE_DIR}/${F}")
  endforeach()
  set(PRIVATE_FILES_CXX__)
  foreach(F IN LISTS PSE_PRIVATE_CXX_SOURCES)
    list(APPEND PRIVATE_FILES_CXX__ "${PSE_SOURCE_DIR}/${F}")
  endforeach()
  set(FILES_C__ ${PUBLIC_FILES_C__} ${PRIVATE_FILES_C__})
  set(FILES_CXX__ ${PUBLIC_FILES_CXX__} ${PRIVATE_FILES_CXX__})

  add_library(${NAME} ${PSE_LIB_TYPE} ${FILES_C__})
  target_compile_definitions(${NAME} PRIVATE
    ${PSE_BUILD_MACRO}_${PSE_LIB_TYPE}
    ${PSE_COMPILE_DEFINITIONS}
  )
  set_property(TARGET ${NAME} PROPERTY C_STANDARD ${PSE_C_STANDARD})
  if(PSE_PUBLIC_C_DEPENDENCIES)
    target_link_libraries(${NAME} PUBLIC ${PSE_PUBLIC_C_DEPENDENCIES})
  endif()
  if(PSE_PRIVATE_C_DEPENDENCIES)
    target_link_libraries(${NAME} PRIVATE ${PSE_PRIVATE_C_DEPENDENCIES})
  endif()
  target_link_libraries(${NAME} PUBLIC ${OpenMP_C_DEPENDENCY})
  target_include_directories(${NAME} PUBLIC
    $<BUILD_INTERFACE:${PSE_ROOT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${PSE_ROOT_INSTALL_DIR}>
  )
  if( PSE_ENABLE_INTRINSICS_OPTIM )
    enable_avx(${NAME})
  endif( PSE_ENABLE_INTRINSICS_OPTIM )

  if(FILES_CXX__)
    set(NAME_CXX ${NAME}-cpp)
    add_library(${NAME_CXX} STATIC ${FILES_CXX__})
    target_compile_definitions(${NAME_CXX} PRIVATE
      ${PSE_BUILD_MACRO}_STATIC
      ${PSE_COMPILE_DEFINITIONS}
    )
    set_property(TARGET ${NAME_CXX} PROPERTY CXX_STANDARD ${PSE_CXX_STANDARD})
    if(PSE_PRIVATE_CXX_DEPENDENCIES)
      target_link_libraries(${NAME_CXX} PRIVATE ${PSE_PRIVATE_CXX_DEPENDENCIES})
    endif()
    if( PSE_ENABLE_INTRINSICS_OPTIM )
      enable_avx(${NAME_CXX})
    endif( PSE_ENABLE_INTRINSICS_OPTIM )
    target_link_libraries(${NAME} PRIVATE ${NAME_CXX} ${OpenMP_CXX_DEPENDENCY})
  endif()

  add_library(${PSE_NAMESPACE}${NAME} ALIAS ${NAME})

  install(
    TARGETS ${NAME}
    EXPORT ${NAME}_Targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION "${PSE_INSTALL_DIR}"
  )
  write_basic_package_version_file(
    "${NAME}ConfigVersion.cmake"
    VERSION ${PSE_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/../TemplateConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake"
  )
  install(
    EXPORT ${NAME}_Targets
    NAMESPACE ${PSE_NAMESPACE}
    FILE "${NAME}Targets.cmake"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake"
  )
  install(
    FILES
      "${CMAKE_CURRENT_BINARY_DIR}/${NAME}Config.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/${NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake"
  )
  foreach(F IN LISTS PSE_PUBLIC_C_SOURCES PSE_PUBLIC_CXX_SOURCES)
    get_filename_component(FILE_SUBDIR "${F}" DIRECTORY)
    install(
      FILES "${PSE_SOURCE_DIR}/${F}"
      DESTINATION "${PSE_INSTALL_DIR}/${FILE_SUBDIR}"
    )
  endforeach()
endfunction()

function(add_source_library NAME SOURCE_DIR)
  set(FILES__)
  foreach(F IN LISTS ARGN)
    list(APPEND FILES__ "${SOURCE_DIR}/${F}")
  endforeach()
  add_custom_target(SOURCES_${NAME} SOURCES ${FILES__})
endfunction()

# =============================================================================
# TO REMOVE: ColorSpace library
# =============================================================================

add_source_library(ColorSpace "${PSE_COLORSPACE_SRC_DIR}"
  "colorspace.h"
  "colorspace.cpp"
)

# =============================================================================
# TO REMOVE: KdTree library
# =============================================================================

add_source_library(KdTree "${PSE_KDTREE_SRC_DIR}"
  "kdtree.h"
  "bbox.h"
  "utils/disablewarnings.h"
)

# =============================================================================
# PSE Common library
# =============================================================================

add_source_library(pse-cmn "${PSE_SRC_DIR}"
  "pse_common.hpp"
  "pse_cost_generic.hpp"
  "pse_cost_generic.inl"
  "pse_cost.hpp"
  "pse_cps.hpp"
  "pse_cps.inl"
  "pse_cps_types.hpp"
  "pse_factory.hpp"
  "pse_graph_types.hpp"
  "pse_graph_utils.hpp"
  "pse_graph_utils.inl"
  "pse_levenberg_utils.hpp"
  "pse_literal_string.hpp"
  "pse_solver_exploration.hpp"
  "pse_solver_exploration.inl"
  "pse_solver.hpp"
  "pse_solver_interpolation.hpp"
  "pse_solver_interpolation.inl"
  "pse_types.hpp"
  "pse_uniform_grid.hpp"
)

# =============================================================================
# PSE Serializer library
# =============================================================================

add_source_library(pse-slz "${PSE_SERIAL_SRC_DIR}"
  "pse_serializer.hpp"
  "pse_serializer.inl"
  "pse_slz_cost_generic.hpp"
  "pse_slz_cost.hpp"
  "pse_slz_cps.hpp"
  "pse_slz_types.hpp"
  "pse_slz_utils.hpp"
)

# =============================================================================
# PSE Serializer library - Binary driver
# =============================================================================

# =============================================================================
# PSE Serializer library - YAML driver
# =============================================================================

add_source_library(pse-slz-drv-yaml "${PSE_SERIAL_SRC_DIR}"
  "pse_serializer_yaml.hpp"
  "pse_serializer_yaml.inl"
)

if(PSE_BUILD_CLT_SPACE_COLOR)
  # ===========================================================================
  # PSE Utils library - Color
  # ===========================================================================
  add_source_library(pse-color "${PSE_COLOR_SRC_DIR}"
    "pse_color_conversion.hpp"
    "pse_color_gamut.hpp"
    "pse_color.hpp"
    "pse_color_palette_processing.hpp"
    "pse_color_palettized_raster.hpp"
    "pse_color_palettizer.hpp"
    "pse_color_spaces.hpp"
    "pse_color_unscaling.hpp"
    "pse_color_vision_deficiencies.hpp"
    "pse_cost_color_energy.hpp"
    "pse_cost_color_energy.inl"
    "pse_cost_color.hpp"
    "pse_cost_color.inl"
    "pse_cost_color_label_map.hpp"
  )

  # ===========================================================================
  # PSE Utils library - Color Serialization
  # ===========================================================================

  add_source_library(pse-slz-color "${PSE_COLOR_SERIAL_SRC_DIR}"
    "pse_slz_color.hpp"
    "pse_slz_cost_color_energy.hpp"
    "pse_slz_cost_color.hpp"
  )
endif()

# =============================================================================
# PSE library
# =============================================================================

if(PSE_BUILD_API)
  set(PSE_API_PRIVATE_C_DEPENDENCIES)
  if(CMAKE_COMPILER_IS_GNUCC AND NOT WIN32)
    set(PSE_API_PRIVATE_C_DEPENDENCIES dl)
  endif()
  pse_add_library(pse SHARED
    VERSION "0.8.0"
    BUILD_MACRO PSE_BUILD
    PUBLIC_C_SOURCES
      "pse_api.h"
      "pse.h"
      "pse_allocator.h"
      "pse_capacities.h"
      "pse_drv.h"
      "pse_logger.h"
      "pse_platform.h"
      "pse_ref_count.h"
      "pse_types.h"
    PRIVATE_C_SOURCES
      "pse_allocator.c"
      "pse_cpspace_p.h"
      "pse_cpspace.c"
      "pse_cpspace_exploration_p.h"
      "pse_cpspace_exploration.c"
      "pse_cpspace_values_p.h"
      "pse_cpspace_values.c"
      "pse_device_p.h"
      "pse_device.c"
      "pse_drv.c"
      "pse_logger.c"
    PRIVATE_C_DEPENDENCIES
      ${PSE_API_PRIVATE_C_DEPENDENCIES}
  )
endif()

if(PSE_BUILD_SLZ)
  pse_add_library(pse-slz SHARED
    VERSION "0.0.0"
    BUILD_MACRO PSE_SLZ_BUILD
    PUBLIC_C_SOURCES
      "pse_slz_api.h"
      "pse_slz.h"
      "pse_slz_buffer.h"
      "pse_slz_buffer_file.h"
      "pse_slz_buffer_memory.h"
      "pse_slz_types.h"
    PRIVATE_C_SOURCES
      "pse_slz.c"
      "pse_slz_buffer_file.c"
      "pse_slz_buffer_memory.c"
    PRIVATE_C_DEPENDENCIES
      PSE::pse
  )
endif()

if(PSE_BUILD_DRV_EIGEN)
  pse_add_library(pse-drv-eigen SHARED
    VERSION "0.0.1"
    BUILD_MACRO PSE_EIGEN_BUILD
    SOURCE_SUBDIR "drv/eigen"
    PRIVATE_C_SOURCES
      "pse_eigen_api.h"
      "pse_eigen_drv.h"
      "pse_eigen_drv.c"
      "pse_eigen_values.h"
    PRIVATE_C_DEPENDENCIES
      PSE::pse
    PRIVATE_CXX_SOURCES
      "pse_eigen_exploration.h"
      "pse_eigen_exploration.cpp"
    PRIVATE_CXX_DEPENDENCIES
      PSE::pse
      Eigen::Eigen3
  )
endif()

if(PSE_BUILD_DRV_EIGEN_REF)
  pse_add_library(pse-drv-eigen-ref SHARED
    VERSION "0.0.1"
    BUILD_MACRO PSE_EIGEN_BUILD
    SOURCE_SUBDIR "drv/eigen"
    PRIVATE_C_SOURCES
      "pse_eigen_api.h"
      "pse_eigen_drv.h"
      "pse_eigen_drv.c"
      "pse_eigen_values.h"
    PRIVATE_C_DEPENDENCIES
      PSE::pse
    PRIVATE_CXX_SOURCES
      "pse_eigen_exploration.h"
      "pse_eigen_exploration.cpp"
    PRIVATE_CXX_DEPENDENCIES
      PSE::pse
      Eigen::Eigen3
    COMPILE_DEFINITIONS
      PSE_EIGEN_REF
  )
endif()

if(PSE_BUILD_CLT)
  pse_add_library(pse-clt SHARED
    VERSION "0.2.0"
    BUILD_MACRO PSE_CLT_BUILD
    SOURCE_SUBDIR "clt"
    PUBLIC_C_SOURCES
      "pse_clt_api.h"
      "pse_clt.h"
      "pse_clt_cost.h"
      "pse_clt_cost_L1.h"
      "pse_clt_cost3.h"
      "pse_clt_space_utils.h"
    PRIVATE_C_SOURCES
      "pse_clt.c"
      "pse_clt_p.h"
      "pse_clt_space_utils.c"
    PRIVATE_C_DEPENDENCIES
      PSE::pse
  )
endif()

if(PSE_BUILD_CLT_SPACE_COLOR)
  pse_add_library(pse-clt-space-color SHARED
    VERSION "0.3.0"
    BUILD_MACRO PSE_COLOR_BUILD
    SOURCE_SUBDIR "clt/space/color"
    PUBLIC_C_SOURCES
      "pse_color_api.h"
      "pse_color.h"
      "pse_color_cost.h"
      "pse_color_cost_generic.h"
      "pse_color_cost_Cat02_LMS.h"
      "pse_color_cost_HSV.h"
      "pse_color_cost_Lab.h"
      "pse_color_cost_RGB.h"
      "pse_color_cost_XYZ.h"
      "pse_color_palette.h"
      "pse_color_palette_constraints.h"
      "pse_color_palette_exploration.h"
      "pse_color_space_Cat02_LMS.h"
      "pse_color_space_HSV.h"
      "pse_color_space_Lab.h"
      "pse_color_space_RGB.h"
      "pse_color_space_XYZ.h"
      "pse_color_types.h"
      "pse_color_vision_deficiencies.h"
    PRIVATE_C_SOURCES
      "pse_color_cost_p.h"
      "pse_color_cost.c"
      "pse_color_cost_distance_tmpl.h"
      "pse_color_cost_distance_utils.h"
      "pse_color_cost_generic_p.h"
      "pse_color_cost_generic.c"
      "pse_color_cost_Cat02_LMS_p.h"
      "pse_color_cost_Cat02_LMS.c"
      "pse_color_cost_HSV_p.h"
      "pse_color_cost_HSV.c"
      "pse_color_cost_Lab_p.h"
      "pse_color_cost_Lab.c"
      "pse_color_cost_RGB_p.h"
      "pse_color_cost_RGB.c"
      "pse_color_cost_XYZ_p.h"
      "pse_color_cost_XYZ.c"
      "pse_color.c"
      "pse_color_conversion_p.h"
      "pse_color_conversion_tmpl.h"
      "pse_color_palette.c"
      "pse_color_palette_p.h"
      "pse_color_palette_constraints.c"
      "pse_color_palette_constraints_p.h"
      "pse_color_palette_exploration.c"
      "pse_color_palette_variation_p.h"
      "pse_color_space_Cat02_LMS.c"
      "pse_color_space_HSV.c"
      "pse_color_space_Lab.c"
      "pse_color_space_RGB.c"
      "pse_color_space_XYZ.c"
      "pse_color_values.c"
      "pse_color_values_p.h"
      "pse_color_vision_deficiencies.c"
      "../../../ColorSpace/colorspace.h"
      "../../../ColorSpace/colorspace.cpp"
      "../../../stb_ds.h"
      "../../../stretchy_buffer.h"
    PUBLIC_C_DEPENDENCIES
      PSE::pse
      PSE::pse-clt
  )
endif()
