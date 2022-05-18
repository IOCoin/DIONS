# CMake generated Testfile for 
# Source directory: /home/ioc/dvmone/dvmc/test/vmtester
# Build directory: /home/ioc/dvmone/dvmc/test/vmtester
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(dvmc/vmtester/exampldvm "/home/ioc/dvmone/bin/dvmc-vmtester" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so")
set_tests_properties(dvmc/vmtester/exampldvm PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/vmtester-%m-%p.profraw" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/cmake/DVMC.cmake;17;add_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;9;dvmc_add_vm_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;0;")
add_test(dvmc/vmtester/trans_log_precompiles_vm "/home/ioc/dvmone/bin/dvmc-vmtester" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_precompiles_vm/libtrans_log-precompiles-vm.so")
set_tests_properties(dvmc/vmtester/trans_log_precompiles_vm PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/vmtester-%m-%p.profraw" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/cmake/DVMC.cmake;17;add_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;10;dvmc_add_vm_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;0;")
add_test(dvmc/vmtester/help "/home/ioc/dvmone/bin/dvmc-vmtester" "--version" "--help")
set_tests_properties(dvmc/vmtester/help PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/vmtester-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Usage:" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;12;add_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;0;")
add_test(dvmc/vmtester/nonexistingvm "/home/ioc/dvmone/bin/dvmc-vmtester" "nonexistingvm")
set_tests_properties(dvmc/vmtester/nonexistingvm PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/vmtester-%m-%p.profraw" PASS_REGULAR_EXPRESSION "[Cc]annot open" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;15;add_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;0;")
add_test(dvmc/vmtester/noarg "/home/ioc/dvmone/bin/dvmc-vmtester")
set_tests_properties(dvmc/vmtester/noarg PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/vmtester-%m-%p.profraw" PASS_REGULAR_EXPRESSION "is required" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;18;add_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;0;")
add_test(dvmc/vmtester/too-many-args "/home/ioc/dvmone/bin/dvmc-vmtester" "a" "b")
set_tests_properties(dvmc/vmtester/too-many-args PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/vmtester-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Unexpected" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;21;add_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;0;")
add_test(dvmc/vmtester/version "/home/ioc/dvmone/bin/dvmc-vmtester" "--version")
set_tests_properties(dvmc/vmtester/version PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/vmtester-%m-%p.profraw" PASS_REGULAR_EXPRESSION "10.0.0-alpha.5" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;24;add_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;0;")
add_test(dvmc/vmtester/unknown-option "/home/ioc/dvmone/bin/dvmc-vmtester" "--verbose")
set_tests_properties(dvmc/vmtester/unknown-option PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/vmtester-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Unknown" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;27;add_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;0;")
add_test(dvmc/vmtester/option-long-prefix "/home/ioc/dvmone/bin/dvmc-vmtester" "---")
set_tests_properties(dvmc/vmtester/option-long-prefix PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/vmtester-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Unknown" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;30;add_test;/home/ioc/dvmone/dvmc/test/vmtester/CMakeLists.txt;0;")
