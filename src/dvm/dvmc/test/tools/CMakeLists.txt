# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

function(add_dvmc_tool_test NAME ARGUMENTS EXPECTED_OUTPUT)
    separate_arguments(ARGUMENTS)
    add_test(NAME ${PROJECT_NAME}/dvmc-tool/${NAME} COMMAND dvmc::tool ${ARGUMENTS})
    set_tests_properties(${PROJECT_NAME}/dvmc-tool/${NAME} PROPERTIES PASS_REGULAR_EXPRESSION ${EXPECTED_OUTPUT})
endfunction()


add_dvmc_tool_test(
    trans_log1
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> run 30600052596000f3 --track 99"
    "Result: +success[\r\n]+Gas used: +6[\r\n]+Output: +0000000000000000000000000000000000000000000000000000000000000000[\r\n]"
)

add_dvmc_tool_test(
    version
    "--version"
    "DVMC ${PROJECT_VERSION} \\($<TARGET_FILE:dvmc::tool>\\)"
)

add_dvmc_tool_test(
    version_vm
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> --version"
    "trans_log_vm ${PROJECT_VERSION} \\($<TARGET_FILE:dvmc::trans_log-vm>\\)[\r\n]DVMC ${PROJECT_VERSION} \\($<TARGET_FILE:dvmc::tool>\\)"
)

add_dvmc_tool_test(
    copy_input
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> run 600035600052596000f3 --input 0xaabbccdd"
    "Result: +success[\r\n]+Gas used: +7[\r\n]+Output: +aabbccdd00000000000000000000000000000000000000000000000000000000[\r\n]"
)

add_dvmc_tool_test(
    default_revision
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> run 00"
    "Executing on London"
)

add_dvmc_tool_test(
    create_return_2
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> run --create 6960026000526001601ff3600052600a6016f3"
    "Result: +success[\r\n]+Gas used: +6[\r\n]+Output: +02[\r\n]"
)

add_test(NAME ${PROJECT_NAME}/dvmc-tool/empty_code COMMAND dvmc::tool --vm $<TARGET_FILE:dvmc::trans_log-vm> run "")
set_tests_properties(${PROJECT_NAME}/dvmc-tool/empty_code PROPERTIES PASS_REGULAR_EXPRESSION "Result: +success[\r\n]+Gas used: +0[\r\n]+Output: +[\r\n]")

add_test(NAME ${PROJECT_NAME}/dvmc-tool/explicit_empty_input COMMAND dvmc::tool --vm $<TARGET_FILE:dvmc::trans_log-vm> run 0x6000 --input "")
set_tests_properties(${PROJECT_NAME}/dvmc-tool/explicit_empty_input PROPERTIES PASS_REGULAR_EXPRESSION "Result: +success[\r\n]+Gas used: +1[\r\n]+Output: +[\r\n]")

add_dvmc_tool_test(
    invalid_hex_code
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> run 0x600"
    "code: \\(incomplete hex byte pair\\) OR \\(File does not exist: 0x600\\)"
)

add_dvmc_tool_test(
    invalid_hex_input
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> run 0x --input aa0y"
    "--input: \\(invalid hex digit\\) OR \\(File does not exist: aa0y\\)"
)

add_dvmc_tool_test(
    code_from_file
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> run ${CMAKE_CURRENT_SOURCE_DIR}/code.hex --input 0xaabbccdd"
    "Result: +success[\r\n]+Gas used: +7[\r\n]+Output: +aabbccdd00000000000000000000000000000000000000000000000000000000[\r\n]"
)

add_dvmc_tool_test(
    input_from_file
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> run 600035600052596000f3 --input ${CMAKE_CURRENT_SOURCE_DIR}/input.hex"
    "Result: +success[\r\n]+Gas used: +7[\r\n]+Output: +aabbccdd00000000000000000000000000000000000000000000000000000000[\r\n]"
)

add_dvmc_tool_test(
    invalid_code_file
    "--vm $<TARGET_FILE:dvmc::trans_log-vm> run ${CMAKE_CURRENT_SOURCE_DIR}/invalid_code.dvm"
    "Error: invalid hex digit"
)

add_dvmc_tool_test(
    vm_option_fallthrough
    "run --vm $<TARGET_FILE:dvmc::trans_log-vm> 0x600030"
    "Result: +success[\r\n]+Gas used: +2[\r\n]+Output: +[\r\n]"
)

get_property(TOOLS_TESTS DIRECTORY PROPERTY TESTS)
set_tests_properties(${TOOLS_TESTS} PROPERTIES ENVIRONMENT LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/tools-%m-%p.profraw)
