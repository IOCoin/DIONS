file(REMOVE_RECURSE
  "../libdvmone-standalone.a"
  "../libdvmone-standalone.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/dvmone-standalone.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
