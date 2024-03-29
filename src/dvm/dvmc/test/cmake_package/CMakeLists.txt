# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

set(install_dir ${CMAKE_CURRENT_BINARY_DIR}/install)
set(build_dir ${CMAKE_CURRENT_BINARY_DIR}/build)
set(use_dvmc_build_dir ${CMAKE_CURRENT_BINARY_DIR}/use_dvmc)
set(use_instructions_build_dir ${CMAKE_CURRENT_BINARY_DIR}/use_instructions)

file(MAKE_DIRECTORY ${build_dir} ${use_dvmc_build_dir} ${use_instructions_build_dir})

set(prefix ${PROJECT_NAME}/cmake_packge)

add_test(
    NAME ${prefix}/configure
    COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} ${PROJECT_SOURCE_DIR} -DCMAKE_INSTALL_PREFIX=${install_dir}
    WORKING_DIRECTORY ${build_dir}
)

add_test(
    NAME ${prefix}/install
    COMMAND ${CMAKE_COMMAND} --build ${build_dir} --target install
)
set_tests_properties(${prefix}/install PROPERTIES DEPENDS ${prefix}/configure)


add_test(
    NAME ${prefix}/use_dvmc/configure
    COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} ${PROJECT_SOURCE_DIR}/trans_logs/use_dvmc_in_cmake -DCMAKE_PREFIX_PATH=${install_dir}
    WORKING_DIRECTORY ${use_dvmc_build_dir}
)
set_tests_properties(${prefix}/use_dvmc/configure PROPERTIES DEPENDS ${prefix}/install)

add_test(
    NAME ${prefix}/use_dvmc/build
    COMMAND ${CMAKE_COMMAND} --build ${use_dvmc_build_dir}
)
set_tests_properties(${prefix}/use_dvmc/build PROPERTIES DEPENDS ${prefix}/use_dvmc/configure)


add_test(
    NAME ${prefix}/use_instructions/configure
    COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} ${PROJECT_SOURCE_DIR}/trans_logs/use_instructions_in_cmake -DCMAKE_PREFIX_PATH=${install_dir}
    WORKING_DIRECTORY ${use_instructions_build_dir}
)
set_tests_properties(${prefix}/use_instructions/configure PROPERTIES DEPENDS ${prefix}/install)

add_test(
    NAME ${prefix}/use_instructions/build
    COMMAND ${CMAKE_COMMAND} --build ${use_instructions_build_dir}
)
set_tests_properties(${prefix}/use_instructions/build PROPERTIES DEPENDS ${prefix}/use_instructions/configure)
