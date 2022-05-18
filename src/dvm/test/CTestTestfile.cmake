# CMake generated Testfile for 
# Source directory: /home/ioc/rem/CC/src/dvm/test
# Build directory: /home/ioc/rem/CC/src/dvm/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(dvmone/dvmc-vmtester "/home/ioc/rem/CC/src/dvm/bin/dvmc-vmtester" "/home/ioc/rem/CC/src/dvm/lib/libdvmone.so.0.9.0-dev")
set_tests_properties(dvmone/dvmc-vmtester PROPERTIES  _BACKTRACE_TRIPLES "/home/ioc/rem/CC/src/dvm/dvmc/cmake/DVMC.cmake;17;add_test;/home/ioc/rem/CC/src/dvm/test/CMakeLists.txt;35;dvmc_add_vm_test;/home/ioc/rem/CC/src/dvm/test/CMakeLists.txt;0;")
subdirs("utils")
subdirs("bench")
subdirs("integration")
subdirs("internal_benchmarks")
subdirs("unittests")
