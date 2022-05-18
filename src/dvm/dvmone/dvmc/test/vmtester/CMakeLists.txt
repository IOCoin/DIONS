# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

include(DVMC)

set(prefix ${PROJECT_NAME}/vmtester)

dvmc_add_vm_test(NAME ${prefix}/exampldvm TARGET trans_log-vm)
dvmc_add_vm_test(NAME ${prefix}/trans_log_precompiles_vm TARGET trans_log-precompiles-vm)

add_test(NAME ${prefix}/help COMMAND dvmc::dvmc-vmtester --version --help)
set_tests_properties(${prefix}/help PROPERTIES PASS_REGULAR_EXPRESSION "Usage:")

add_test(NAME ${prefix}/nonexistingvm COMMAND dvmc::dvmc-vmtester nonexistingvm)
set_tests_properties(${prefix}/nonexistingvm PROPERTIES PASS_REGULAR_EXPRESSION "[Cc]annot open")

add_test(NAME ${prefix}/noarg COMMAND dvmc::dvmc-vmtester)
set_tests_properties(${prefix}/noarg PROPERTIES PASS_REGULAR_EXPRESSION "is required")

add_test(NAME ${prefix}/too-many-args COMMAND dvmc::dvmc-vmtester a b)
set_tests_properties(${prefix}/too-many-args PROPERTIES PASS_REGULAR_EXPRESSION "Unexpected")

add_test(NAME ${prefix}/version COMMAND dvmc::dvmc-vmtester --version)
set_tests_properties(${prefix}/version PROPERTIES PASS_REGULAR_EXPRESSION ${PROJECT_VERSION})

add_test(NAME ${prefix}/unknown-option COMMAND dvmc::dvmc-vmtester --verbose)
set_tests_properties(${prefix}/unknown-option PROPERTIES PASS_REGULAR_EXPRESSION "Unknown")

add_test(NAME ${prefix}/option-long-prefix COMMAND dvmc::dvmc-vmtester ---)
set_tests_properties(${prefix}/option-long-prefix PROPERTIES PASS_REGULAR_EXPRESSION "Unknown")

get_property(vmtester_tests DIRECTORY PROPERTY TESTS)
set_tests_properties(${vmtester_tests} PROPERTIES ENVIRONMENT LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/vmtester-%m-%p.profraw)
