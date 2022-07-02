# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

include(UseJava)

find_package(Java REQUIRED COMPONENTS Development)
find_package(JNI REQUIRED)

add_jar(dvmc-jar
    OUTPUT_NAME dvmc-java
    VERSION ${PROJECT_VERSION}
    SOURCES
    java/src/main/java/org/blastdoor7/dvmc/EvmcLoaderException.java
    java/src/main/java/org/blastdoor7/dvmc/EvmcVm.java
    java/src/main/java/org/blastdoor7/dvmc/Host.java
    java/src/main/java/org/blastdoor7/dvmc/HostContext.java
    GENERATE_NATIVE_HEADERS dvmc-java-native-headers)

add_library(dvmc-java SHARED)
target_sources(dvmc-java PRIVATE
    c/dvmc-vm.c
    c/host.c
    c/host.h)
target_include_directories(dvmc-java PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries(dvmc-java PRIVATE dvmc-java-native-headers dvmc::dvmc dvmc::loader ${JNI_LIBRARIES})

if(DVMC_INSTALL)
    install(TARGETS dvmc-java EXPORT dvmcTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    install_jar(dvmc-jar DESTINATION ${CMAKE_INSTALL_DATADIR}/java)
endif()