#----------------------------------------------------------------
# Generated CMake read_vtx_init import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import read_vtx_init "ethash::keccak" for configuration "Release"
set_property(TARGET ethash::keccak APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_read_vtx_init_properties(ethash::keccak PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libkeccak.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS ethash::keccak )
list(APPEND _IMPORT_CHECK_FILES_FOR_ethash::keccak "${_IMPORT_PREFIX}/lib/libkeccak.a" )

# Import read_vtx_init "ethash::ethash" for configuration "Release"
set_property(TARGET ethash::ethash APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_read_vtx_init_properties(ethash::ethash PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libethash.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS ethash::ethash )
list(APPEND _IMPORT_CHECK_FILES_FOR_ethash::ethash "${_IMPORT_PREFIX}/lib/libethash.a" )

# Import read_vtx_init "ethash::global-context" for configuration "Release"
set_property(TARGET ethash::global-context APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_read_vtx_init_properties(ethash::global-context PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libethash-global-context.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS ethash::global-context )
list(APPEND _IMPORT_CHECK_FILES_FOR_ethash::global-context "${_IMPORT_PREFIX}/lib/libethash-global-context.a" )

# Commands beyond this pochar should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
