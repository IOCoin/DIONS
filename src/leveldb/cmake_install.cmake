# Install script for directory: /home/ioc/REPO/DIONS/src/leveldb

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/ioc/REPO/DIONS/src/leveldb/libleveldb.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/leveldb" TYPE FILE FILES
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/c.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/cache.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/comparator.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/db.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/dumpfile.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/env.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/export.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/filter_policy.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/iterator.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/options.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/slice.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/status.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/table_builder.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/table.h"
    "/home/ioc/REPO/DIONS/src/leveldb/include/leveldb/write_batch.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb/leveldbTargets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb/leveldbTargets.cmake"
         "/home/ioc/REPO/DIONS/src/leveldb/CMakeFiles/Export/lib/cmake/leveldb/leveldbTargets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb/leveldbTargets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb/leveldbTargets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb" TYPE FILE FILES "/home/ioc/REPO/DIONS/src/leveldb/CMakeFiles/Export/lib/cmake/leveldb/leveldbTargets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb" TYPE FILE FILES "/home/ioc/REPO/DIONS/src/leveldb/CMakeFiles/Export/lib/cmake/leveldb/leveldbTargets-noconfig.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb" TYPE FILE FILES
    "/home/ioc/REPO/DIONS/src/leveldb/cmake/leveldbConfig.cmake"
    "/home/ioc/REPO/DIONS/src/leveldb/cmake/leveldbConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/ioc/REPO/DIONS/src/leveldb/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
