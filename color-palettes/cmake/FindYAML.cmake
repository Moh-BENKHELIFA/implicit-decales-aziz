# Input:
#  - YAML_ROOT_DIR (optional): for local installation containing 'include' and
#    'lib' directories.
#
# Output:     
#  - YAML_FOUND: says if the library has been found
#  - YAML_INCLUDES: on success, contains all the YAML include files
#  - YAML_LIBRARIES: on success, contains all the YAML libraries
#


if(NOT YAML_FOUND)
  
  include(FindPackageHandleStandardArgs)
  include(GNUInstallDirs)

  find_path(YAML_INCLUDE_PATH yaml.h
    HINTS
      ${YAML_ROOT_DIR}
      ${CMAKE_INSTALL_INCLUDES}
    PATH_SUFFIXES
      include
  )

  find_library(YAML_LIBRARY_PATH yaml
    HINTS
      ${YAML_ROOT_DIR}
      ${CMAKE_INSTALL_LIBDIR}
    PATH_SUFFIXES
      lib
  )

  find_package_handle_standard_args(YAML DEFAULT_MSG
    YAML_INCLUDE_PATH
    YAML_LIBRARY_PATH
  )

  if(YAML_INCLUDE_PATH AND YAML_LIBRARY_PATH)
    add_library(YAML UNKNOWN IMPORTED)
    set_target_properties(YAML PROPERTIES
      IMPORTED_LOCATION ${YAML_LIBRARY_PATH}
      INTERFACE_INCLUDE_DIRECTORIES ${YAML_INCLUDE_PATH}
    )
  endif()

  mark_as_advanced(YAML_ROOT_DIR YAML_INCLUDE_PATH YAML_LIBRARY_PATH)
endif()
