# CMake generated Testfile for 
# Source directory: /home/ioc/dvmone/dvmc/test/tools
# Build directory: /home/ioc/dvmone/dvmc/test/tools
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(dvmc/dvmc-tool/trans_log1 "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "30600052596000f3" "--track" "99")
set_tests_properties(dvmc/dvmc-tool/trans_log1 PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Result: +success[
]+Gas used: +6[
]+Output: +0000000000000000000000000000000000000000000000000000000000000000[
]" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;12;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/version "/home/ioc/dvmone/bin/dvmc" "--version")
set_tests_properties(dvmc/dvmc-tool/version PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "DVMC 10.0.0-alpha.5 \\(/home/ioc/dvmone/bin/dvmc\\)" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;18;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/version_vm "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "--version")
set_tests_properties(dvmc/dvmc-tool/version_vm PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "trans_log_vm 10.0.0-alpha.5 \\(/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so\\)[
]DVMC 10.0.0-alpha.5 \\(/home/ioc/dvmone/bin/dvmc\\)" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;24;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/copy_input "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "600035600052596000f3" "--input" "0xaabbccdd")
set_tests_properties(dvmc/dvmc-tool/copy_input PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Result: +success[
]+Gas used: +7[
]+Output: +aabbccdd00000000000000000000000000000000000000000000000000000000[
]" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;30;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/default_revision "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "00")
set_tests_properties(dvmc/dvmc-tool/default_revision PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Executing on London" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;36;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/create_return_2 "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "--create" "6960026000526001601ff3600052600a6016f3")
set_tests_properties(dvmc/dvmc-tool/create_return_2 PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Result: +success[
]+Gas used: +6[
]+Output: +02[
]" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;42;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/empty_code "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "")
set_tests_properties(dvmc/dvmc-tool/empty_code PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Result: +success[
]+Gas used: +0[
]+Output: +[
]" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;48;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/explicit_empty_input "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "0x6000" "--input" "")
set_tests_properties(dvmc/dvmc-tool/explicit_empty_input PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Result: +success[
]+Gas used: +1[
]+Output: +[
]" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;51;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/invalid_hex_code "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "0x600")
set_tests_properties(dvmc/dvmc-tool/invalid_hex_code PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "code: \\(incomplete hex byte pair\\) OR \\(File does not exist: 0x600\\)" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;54;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/invalid_hex_input "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "0x" "--input" "aa0y")
set_tests_properties(dvmc/dvmc-tool/invalid_hex_input PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "--input: \\(invalid hex digit\\) OR \\(File does not exist: aa0y\\)" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;60;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/code_from_file "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "/home/ioc/dvmone/dvmc/test/tools/code.hex" "--input" "0xaabbccdd")
set_tests_properties(dvmc/dvmc-tool/code_from_file PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Result: +success[
]+Gas used: +7[
]+Output: +aabbccdd00000000000000000000000000000000000000000000000000000000[
]" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;66;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/input_from_file "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "600035600052596000f3" "--input" "/home/ioc/dvmone/dvmc/test/tools/input.hex")
set_tests_properties(dvmc/dvmc-tool/input_from_file PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Result: +success[
]+Gas used: +7[
]+Output: +aabbccdd00000000000000000000000000000000000000000000000000000000[
]" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;72;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/invalid_code_file "/home/ioc/dvmone/bin/dvmc" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "run" "/home/ioc/dvmone/dvmc/test/tools/invalid_code.dvm")
set_tests_properties(dvmc/dvmc-tool/invalid_code_file PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Error: invalid hex digit" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;78;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
add_test(dvmc/dvmc-tool/vm_option_fallthrough "/home/ioc/dvmone/bin/dvmc" "run" "--vm" "/home/ioc/dvmone/dvmc/trans_logs/trans_log_vm/libtrans_log-vm.so" "0x600030")
set_tests_properties(dvmc/dvmc-tool/vm_option_fallthrough PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/dvmone/tools-%m-%p.profraw" PASS_REGULAR_EXPRESSION "Result: +success[
]+Gas used: +2[
]+Output: +[
]" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;7;add_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;84;add_dvmc_tool_test;/home/ioc/dvmone/dvmc/test/tools/CMakeLists.txt;0;")
