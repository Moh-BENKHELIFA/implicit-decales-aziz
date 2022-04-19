# FindFunctionnal
# ---------------
#
# Find the Functionnal library header-only files.
#
# This code defines the following variables:
#
#   Functionnal_FOUND                  - TRUE if library has been found.
#   Functionnal_INCLUDE_DIRS           - Full paths to all include dirs.
#

set(_Functionnal_REQUIRED_VARS Functionnal_INCLUDE_DIRS)
find_path(Functionnal_INCLUDE_DIRS Functionnal/functionnal.h)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Functionnal
  REQUIRED_VARS ${_Functionnal_REQUIRED_VARS}
)
unset(_Functionnal_REQUIRED_VARS)

set(Functionnal_FOUND ${Functionnal_FOUND})

if(Functionnal_FOUND)
  if(Functionnal_INCLUDE_DIRS AND NOT TARGET Functionnal::Functionnal)
    add_library(Functionnal::Functionnal INTERFACE IMPORTED)
    set_target_properties(Functionnal::Functionnal PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${Functionnal_INCLUDE_DIRS}"
    )
  endif()
endif()
