# CMake generated Testfile for 
# Source directory: /home/ioc/rem/CC/src/dvm/test/integration
# Build directory: /home/ioc/rem/CC/src/dvm/test/integration
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(dvmone/integration/histogram "/home/ioc/rem/CC/src/dvm/bin/dvmc" "--vm" "/home/ioc/rem/CC/src/dvm/lib/libdvmone.so.0.9.0-dev,O=0,histogram" "run" "6000808080800101010200")
set_tests_properties(dvmone/integration/histogram PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/rem/CC/src/dvm/integration-%p.profraw" PASS_REGULAR_EXPRESSION "--- # HISTOGRAM depth=0
opcode,count
STOP,1
ADD,3
MUL,1
PUSH1,1
DUP1,4
" _BACKTRACE_TRIPLES "/home/ioc/rem/CC/src/dvm/test/integration/CMakeLists.txt;10;add_test;/home/ioc/rem/CC/src/dvm/test/integration/CMakeLists.txt;0;")
add_test(dvmone/integration/trace "/home/ioc/rem/CC/src/dvm/bin/dvmc" "--vm" "/home/ioc/rem/CC/src/dvm/lib/libdvmone.so.0.9.0-dev,O=0,trace" "run" "60006002800103")
set_tests_properties(dvmone/integration/trace PROPERTIES  ENVIRONMENT "LLVM_PROFILE_FILE=/home/ioc/rem/CC/src/dvm/integration-%p.profraw" PASS_REGULAR_EXPRESSION "{\"depth\":0,\"rev\":\"London\",\"static\":false}
{\"pc\":0,\"op\":96,\"opName\":\"PUSH1\",\"track\":1000000,\"stack\":\\[\\],\"memorySize\":0}
{\"pc\":2,\"op\":96,\"opName\":\"PUSH1\",\"track\":999997,\"stack\":\\[\"0x0\"\\],\"memorySize\":0}
{\"pc\":4,\"op\":128,\"opName\":\"DUP1\",\"track\":999994,\"stack\":\\[\"0x0\",\"0x2\"\\],\"memorySize\":0}
{\"pc\":5,\"op\":1,\"opName\":\"ADD\",\"track\":999991,\"stack\":\\[\"0x0\",\"0x2\",\"0x2\"\\],\"memorySize\":0}
{\"pc\":6,\"op\":3,\"opName\":\"SUB\",\"track\":999988,\"stack\":\\[\"0x0\",\"0x4\"\\],\"memorySize\":0}
{\"error\":null,\"track\":999985,\"trackUsed\":15,\"output\":\"\"}
" _BACKTRACE_TRIPLES "/home/ioc/rem/CC/src/dvm/test/integration/CMakeLists.txt;22;add_test;/home/ioc/rem/CC/src/dvm/test/integration/CMakeLists.txt;0;")
