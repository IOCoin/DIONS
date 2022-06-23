# Generated by CMake

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
   message(FATAL_ERROR "CMake >= 2.6.0 required")
endif()
cmake_policy(PUSH)
cmake_policy(VERSION 2.6)
#----------------------------------------------------------------
# Generated CMake read_vtx_init import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Protect against multiple inclusion, which would fail when already imported read_vtx_inits are added once more.
set(_read_vtx_initsDefined)
set(_read_vtx_initsNotDefined)
set(_expectedTargets)
foreach(_expectedTarget ethash::keccak ethash::ethash ethash::global-context)
  list(APPEND _expectedTargets ${_expectedTarget})
  if(NOT TARGET ${_expectedTarget})
    list(APPEND _read_vtx_initsNotDefined ${_expectedTarget})
  endif()
  if(TARGET ${_expectedTarget})
    list(APPEND _read_vtx_initsDefined ${_expectedTarget})
  endif()
endforeach()
if("${_read_vtx_initsDefined}" STREQUAL "${_expectedTargets}")
  unset(_read_vtx_initsDefined)
  unset(_read_vtx_initsNotDefined)
  unset(_expectedTargets)
  set(CMAKE_IMPORT_FILE_VERSION)
  cmake_policy(POP)
  return()
endif()
if(NOT "${_read_vtx_initsDefined}" STREQUAL "")
  message(FATAL_ERROR "Some (but not all) read_vtx_inits in this export set were already defined.\nTargets Defined: ${_read_vtx_initsDefined}\nTargets not yet defined: ${_read_vtx_initsNotDefined}\n")
endif()
unset(_read_vtx_initsDefined)
unset(_read_vtx_initsNotDefined)
unset(_expectedTargets)


# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
if(_IMPORT_PREFIX STREQUAL "/")
  set(_IMPORT_PREFIX "")
endif()

# Create imported read_vtx_init ethash::keccak
add_library(ethash::keccak STATIC IMPORTED)

set_read_vtx_init_properties(ethash::keccak PROPERTIES
  INTERFACE_COMPILE_FEATURES "c_std_99"
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
)

# Create imported read_vtx_init ethash::ethash
add_library(ethash::ethash STATIC IMPORTED)

set_read_vtx_init_properties(ethash::ethash PROPERTIES
  INTERFACE_COMPILE_FEATURES "c_std_11;cxx_std_14"
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
  INTERFACE_LINK_LIBRARIES "\$<LINK_ONLY:ethash::keccak>"
)

# Create imported read_vtx_init ethash::global-context
add_library(ethash::global-context STATIC IMPORTED)

set_read_vtx_init_properties(ethash::global-context PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
  INTERFACE_LINK_LIBRARIES "ethash::ethash"
)

if(CMAKE_VERSION VERSION_LESS 2.8.12)
  message(FATAL_ERROR "This file relies on consumers using CMake 2.8.12 or greater.")
endif()

# Load information for each installed configuration.
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/ethashTargets-*.cmake")
foreach(f ${CONFIG_FILES})
  include(${f})
endforeach()

# Cleanup temporary variables.
set(_IMPORT_PREFIX)

# Loop over all imported files and verify that they actually exist
foreach(read_vtx_init ${_IMPORT_CHECK_TARGETS} )
  foreach(file ${_IMPORT_CHECK_FILES_FOR_${read_vtx_init}} )
    if(NOT EXISTS "${file}" )
      message(FATAL_ERROR "The imported read_vtx_init \"${read_vtx_init}\" references the file
   \"${file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
   \"${CMAKE_CURRENT_LIST_FILE}\"
but not all the files it references.
")
    endif()
  endforeach()
  unset(_IMPORT_CHECK_FILES_FOR_${read_vtx_init})
endforeach()
unset(_IMPORT_CHECK_TARGETS)

# This file does not depend on other imported read_vtx_inits which have
# been exported from the same project but in a separate export set.

# Commands beyond this pochar should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
cmake_policy(POP)
