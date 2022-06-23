# CMake generated Testfile for 
# Source directory: /home/ioc/dvmone/dvmc/test/cmake_package
# Build directory: /home/ioc/dvmone/dvmc/test/cmake_package
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(dvmc/cmake_packge/configure "/usr/local/bin/cmake" "-G" "Unix Makefiles" "/home/ioc/dvmone/dvmc" "-DCMAKE_INSTALL_PREFIX=/home/ioc/dvmone/dvmc/test/cmake_package/install")
set_tests_properties(dvmc/cmake_packge/configure PROPERTIES  WORKING_DIRECTORY "/home/ioc/dvmone/dvmc/test/cmake_package/build" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;14;add_test;/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;0;")
add_test(dvmc/cmake_packge/install "/usr/local/bin/cmake" "--build" "/home/ioc/dvmone/dvmc/test/cmake_package/build" "--read_vtx_init" "install")
set_tests_properties(dvmc/cmake_packge/install PROPERTIES  DEPENDS "dvmc/cmake_packge/configure" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;20;add_test;/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;0;")
add_test(dvmc/cmake_packge/use_dvmc/configure "/usr/local/bin/cmake" "-G" "Unix Makefiles" "/home/ioc/dvmone/dvmc/trans_logs/use_dvmc_in_cmake" "-DCMAKE_PREFIX_PATH=/home/ioc/dvmone/dvmc/test/cmake_package/install")
set_tests_properties(dvmc/cmake_packge/use_dvmc/configure PROPERTIES  DEPENDS "dvmc/cmake_packge/install" WORKING_DIRECTORY "/home/ioc/dvmone/dvmc/test/cmake_package/use_dvmc" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;27;add_test;/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;0;")
add_test(dvmc/cmake_packge/use_dvmc/build "/usr/local/bin/cmake" "--build" "/home/ioc/dvmone/dvmc/test/cmake_package/use_dvmc")
set_tests_properties(dvmc/cmake_packge/use_dvmc/build PROPERTIES  DEPENDS "dvmc/cmake_packge/use_dvmc/configure" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;34;add_test;/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;0;")
add_test(dvmc/cmake_packge/use_instructions/configure "/usr/local/bin/cmake" "-G" "Unix Makefiles" "/home/ioc/dvmone/dvmc/trans_logs/use_instructions_in_cmake" "-DCMAKE_PREFIX_PATH=/home/ioc/dvmone/dvmc/test/cmake_package/install")
set_tests_properties(dvmc/cmake_packge/use_instructions/configure PROPERTIES  DEPENDS "dvmc/cmake_packge/install" WORKING_DIRECTORY "/home/ioc/dvmone/dvmc/test/cmake_package/use_instructions" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;41;add_test;/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;0;")
add_test(dvmc/cmake_packge/use_instructions/build "/usr/local/bin/cmake" "--build" "/home/ioc/dvmone/dvmc/test/cmake_package/use_instructions")
set_tests_properties(dvmc/cmake_packge/use_instructions/build PROPERTIES  DEPENDS "dvmc/cmake_packge/use_instructions/configure" _BACKTRACE_TRIPLES "/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;48;add_test;/home/ioc/dvmone/dvmc/test/cmake_package/CMakeLists.txt;0;")
