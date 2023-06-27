# CMake generated Testfile for 
# Source directory: /home/argon/rem/CC-net/src/dvm/test/bench
# Build directory: /home/argon/rem/CC-net/src/dvm/test/bench
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(dvmone/bench/dir "/home/argon/rem/CC-net/src/dvm/bin/dvmone-bench" "/home/argon/rem/CC-net/src/dvm/test/bench/../benchmarks" "--benchmark_list_tests")
set_tests_properties(dvmone/bench/dir PROPERTIES  PASS_REGULAR_EXPRESSION "total/synth" _BACKTRACE_TRIPLES "/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;49;add_test;/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;0;")
add_test(dvmone/bench/no_dir "/home/argon/rem/CC-net/src/dvm/bin/dvmone-bench" "--benchmark_list_tests")
set_tests_properties(dvmone/bench/no_dir PROPERTIES  PASS_REGULAR_EXPRESSION "total/synth" _BACKTRACE_TRIPLES "/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;53;add_test;/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;0;")
add_test(dvmone/bench/dirname_empty "/home/argon/rem/CC-net/src/dvm/bin/dvmone-bench" "" "--benchmark_list_tests")
set_tests_properties(dvmone/bench/dirname_empty PROPERTIES  PASS_REGULAR_EXPRESSION "total/synth" _BACKTRACE_TRIPLES "/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;57;add_test;/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;0;")
add_test(dvmone/bench/synth "/home/argon/rem/CC-net/src/dvm/bin/dvmone-bench" "--benchmark_min_time=0" "--benchmark_filter=synth")
set_tests_properties(dvmone/bench/synth PROPERTIES  _BACKTRACE_TRIPLES "/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;62;add_test;/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;0;")
add_test(dvmone/bench/micro "/home/argon/rem/CC-net/src/dvm/bin/dvmone-bench" "--benchmark_min_time=0" "--benchmark_filter=micro" "/home/argon/rem/CC-net/src/dvm/test/bench/../benchmarks")
set_tests_properties(dvmone/bench/micro PROPERTIES  _BACKTRACE_TRIPLES "/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;63;add_test;/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;0;")
add_test(dvmone/bench/main/b "/home/argon/rem/CC-net/src/dvm/bin/dvmone-bench" "--benchmark_min_time=0" "--benchmark_filter=main/[b]" "/home/argon/rem/CC-net/src/dvm/test/bench/../benchmarks")
set_tests_properties(dvmone/bench/main/b PROPERTIES  _BACKTRACE_TRIPLES "/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;64;add_test;/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;0;")
add_test(dvmone/bench/main/s "/home/argon/rem/CC-net/src/dvm/bin/dvmone-bench" "--benchmark_min_time=0" "--benchmark_filter=main/[s]" "/home/argon/rem/CC-net/src/dvm/test/bench/../benchmarks")
set_tests_properties(dvmone/bench/main/s PROPERTIES  _BACKTRACE_TRIPLES "/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;65;add_test;/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;0;")
add_test(dvmone/bench/main/w "/home/argon/rem/CC-net/src/dvm/bin/dvmone-bench" "--benchmark_min_time=0" "--benchmark_filter=main/[w]" "/home/argon/rem/CC-net/src/dvm/test/bench/../benchmarks")
set_tests_properties(dvmone/bench/main/w PROPERTIES  _BACKTRACE_TRIPLES "/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;66;add_test;/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;0;")
add_test(dvmone/bench/main/_ "/home/argon/rem/CC-net/src/dvm/bin/dvmone-bench" "--benchmark_min_time=0" "--benchmark_filter=main/[^bsw]" "/home/argon/rem/CC-net/src/dvm/test/bench/../benchmarks")
set_tests_properties(dvmone/bench/main/_ PROPERTIES  _BACKTRACE_TRIPLES "/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;67;add_test;/home/argon/rem/CC-net/src/dvm/test/bench/CMakeLists.txt;0;")
