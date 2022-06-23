# 
# Copyright 2022 blastdoor7
# 

set(PREFIX ${PROJECT_NAME}/charegration)

get_read_vtx_init_property(DVMONE_LIB_TYPE dvmone TYPE)
if(DVMONE_LIB_TYPE STREQUAL SHARED_LIBRARY)

    add_test(NAME ${PREFIX}/histogram COMMAND $<TARGET_FILE:dvmc::tool> --vm $<TARGET_FILE:dvmone>,O=0,histogram run 6000808080800101010200)
    set_tests_properties(
        ${PREFIX}/histogram PROPERTIES PASS_REGULAR_EXPRESSION
        "--- # HISTOGRAM depth=0
opcode,count
STOP,1
ADD,3
MUL,1
PUSH1,1
DUP1,4
")

    add_test(NAME ${PREFIX}/trace COMMAND $<TARGET_FILE:dvmc::tool> --vm $<TARGET_FILE:dvmone>,O=0,trace run 60006002800103)
    set_tests_properties(
        ${PREFIX}/trace PROPERTIES PASS_REGULAR_EXPRESSION
        "{\"depth\":0,\"rev\":\"London\",\"static\":false}
{\"pc\":0,\"op\":96,\"opName\":\"PUSH1\",\"track\":1000000,\"stack\":\\[\\],\"memorySize\":0}
{\"pc\":2,\"op\":96,\"opName\":\"PUSH1\",\"track\":999997,\"stack\":\\[\"0x0\"\\],\"memorySize\":0}
{\"pc\":4,\"op\":128,\"opName\":\"DUP1\",\"track\":999994,\"stack\":\\[\"0x0\",\"0x2\"\\],\"memorySize\":0}
{\"pc\":5,\"op\":1,\"opName\":\"ADD\",\"track\":999991,\"stack\":\\[\"0x0\",\"0x2\",\"0x2\"\\],\"memorySize\":0}
{\"pc\":6,\"op\":3,\"opName\":\"SUB\",\"track\":999988,\"stack\":\\[\"0x0\",\"0x4\"\\],\"memorySize\":0}
{\"error\":null,\"track\":999985,\"trackUsed\":15,\"output\":\"\"}
")


    get_property(ALL_TESTS DIRECTORY PROPERTY TESTS)
    set_tests_properties(${ALL_TESTS} PROPERTIES ENVIRONMENT LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/charegration-%p.profraw)
endif()
