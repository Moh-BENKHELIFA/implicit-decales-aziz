################################################################################
# This code has been tested with
#
#    gcc version 5.2.1 20151003 (Debian 5.2.1-21)
#    Apple LLVM version 7.0.0 (clang-700.1.76)
#
#
# Authors:
#   Clement Rodrigues-Viguier (crv.public@free.fr)
#   Nicolas Mellado (nmellado0@gmail.com)
#   David Vanderheaghe (vdh@irit.fr)
#
# Contributors:
#    Mathias Paulin (Mathias.Paulin@irit.fr) MacOSX cross-compilation
################################################################################

project (STORM-ParameterSpaceOptimization C CXX)
cmake_minimum_required(VERSION 3.9)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CTest)

################################################################################
# Configuration
################################################################################

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/../local"
    CACHE PATH
    "Install prefix prepended to all installed files and directories"
    FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(PSE_ENABLE_TRACE FALSE
  CACHE BOOL "Enable some trace logs put on standard output"
)
set(PSE_ENABLE_INTRINSICS_OPTIM FALSE
  CACHE BOOL "Enable intrinsics optimizations (automatically forced to FALSE in debug)"
)
set(PSE_ENABLE_USE_OF_OPENMP FALSE
  CACHE BOOL "Use OpenMP where possible"
)
set(PSE_BUILD_API TRUE
  CACHE BOOL "Build the core API library"
)
set(PSE_BUILD_DRV_EIGEN TRUE
  CACHE BOOL "Build the Eigen driver"
)
set(PSE_BUILD_DRV_EIGEN_REF TRUE
  CACHE BOOL "Build the Eigen reference driver"
)
set(PSE_BUILD_CLT TRUE
  CACHE BOOL "Build the generic client library"
)
set(PSE_BUILD_CLT_SPACE_COLOR TRUE
  CACHE BOOL "Build the color space client library"
)
set(PSE_BUILD_SLZ TRUE
  CACHE BOOL "Build the serialization library"
)
set(PSE_BUILD_SLZ_CLT_SPACE_COLOR TRUE
  CACHE BOOL "Build the serialization of the color space client library"
)
set(PSE_BUILD_TESTS TRUE
  CACHE BOOL "Build tests"
)
set(PSE_BUILD_EXAMPLES FALSE
  CACHE BOOL "Build examples"
)
set(PSE_BUILD_CLIS TRUE
  CACHE BOOL "Build Command Line Instructions"
)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EIGEN3_INCLUDE_DIR "C:/Users/aniyazov/Downloads/eigen-master/eigen-master")
set(Functionnal_INCLUDE_DIRS "C:/Users/aniyazov/Downloads/Functionnal-master/Functionnal-master")

################################################################################
# Include helpers and find packages
################################################################################

find_package(Eigen3 REQUIRED)
#find_package(Functionnal REQUIRED)
find_package(YAML)
if(PSE_ENABLE_USE_OF_OPENMP)
  find_package(OpenMP)
endif()

if(PSE_BUILD_CLT_SPACE_COLOR)
# TODO: these libraries should be manage as externals or fully integrated!
#  find_package(ColorSpace REQUIRED)
#  find_package(KdTree REQUIRED)
endif()

include(GNUInstallDirs)
include(CheckSSE2)

################################################################################
# Check validity of the project
################################################################################

# guard against in-source builds (source: Eigen)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

# guard against bad build-type strings (source: Eigen)
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_tolower)
if(    NOT cmake_build_type_tolower STREQUAL "debug"
   AND NOT cmake_build_type_tolower STREQUAL "release"
   AND NOT cmake_build_type_tolower STREQUAL "relwithdebinfo")
  message(FATAL_ERROR "Unknown build type \"${CMAKE_BUILD_TYPE}\". Allowed values are Debug, Release, RelWithDebInfo (case-insensitive).")
endif()

if( cmake_build_type_tolower STREQUAL "debug")
  if( PSE_ENABLE_INTRINSICS_OPTIM )
    set(PSE_ENABLE_INTRINSICS_OPTIM FALSE)
    message("Disable intrinsics optimisation in Debug mode")
  endif()
endif()

################################################################################
# Configure global compilation flags
################################################################################

# Force some options for all targets
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-fPIC -Wall -Wextra -pedantic)
endif()

if(PSE_ENABLE_TRACE)
  add_definitions(-DTRACE_ENABLED)
endif()

if(cmake_build_type_tolower STREQUAL "debug")
  add_definitions(-D__DEBUG__)
endif()

################################################################################
# Add a target to generate API documentation with Doxygen                      #
################################################################################

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../doc/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
  )
  add_custom_target(doc
    "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../doc"
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
else()
  add_custom_target(doc COMMENT "API documentation disabled, Doxygen not found")
endif()

################################################################################
# Subdirectories
################################################################################

add_subdirectory(libs)
if(PSE_BUILD_CLIS)
  add_subdirectory(clis)
endif()
if(BUILD_TESTING AND PSE_BUILD_TESTS)
  add_subdirectory(tests)
endif()
if(PSE_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

################################################################################
# Package configuration
################################################################################

# Get the current Git HASH
execute_process(
  COMMAND
    "${GITCOMMAND}" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_SHA1
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CPACK_PACKAGE_NAME "pso")
set(CPACK_PACKAGE_VERSION "${GIT_SHA1}")
set(CPACK_PACKAGE_VENDOR "IRIT")
set(CPACK_PACKAGE_CHECKSUM "SHA512")
set(CPACK_INSTALLED_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/html" "share/doc/html")
set(CPACK_GENERATOR "ZIP")
include(CPack)
