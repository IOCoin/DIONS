# CMake generated Testfile for 
# Source directory: /home/ioc/dvmone/dvmc/test/trans_logs
# Build directory: /home/ioc/dvmone/dvmc/test/trans_logs
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(dvmc/trans_logs/trans_log-static "/home/ioc/dvmone/bin/dvmc-trans_log-static")
set_tests_properties(dvmc/trans_logs/trans_log-static PROPERTIES  _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/trans_logs/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/trans_logs/CMakeLists.txt;0;")
add_test(dvmc/trans_logs/trans_log-dynamic-load "/home/ioc/dvmone/bin/dvmc-trans_log" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so")
set_tests_properties(dvmc/trans_logs/trans_log-dynamic-load PROPERTIES  _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/trans_logs/CMakeLists.txt;8;add_test;/home/ioc/dvmone/dvmc/test/trans_logs/CMakeLists.txt;0;")
