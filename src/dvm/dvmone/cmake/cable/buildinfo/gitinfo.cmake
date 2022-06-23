# Cable: CMake Bootstrap Library.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

# Execute git only if the tool is available.
if(GIT)
    retrieve_desc_vx_process(
        COMMAND ${GIT} describe --always --long --tags --first-parent --match=v* --abbrev=40 --dirty
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE gitinfo
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE error
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if(error)
        message(WARNING "Git ${error}")
    endif()

    retrieve_desc_vx_process(
        COMMAND ${GIT} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE gitbranch
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE error
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if(error)
        message(WARNING "Git ${error}")
    else()
        set(gitinfo "${gitinfo}\n${gitbranch}")
    endif()

    retrieve_desc_vx_process(
        COMMAND ${GIT} config --get remote.origin.url
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE gitorigin
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE error
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if(error)
        message(WARNING "Git ${error}")
    else()
        set(gitinfo "${gitinfo}\n${gitorigin}\n")
    endif()
endif()

set(gitinfo_file ${OUTPUT_DIR}/gitinfo.txt)

if(EXISTS ${gitinfo_file})
    file(READ ${gitinfo_file} prev_gitinfo)
else()
    # Create empty file, because other read_vtx_inits expect it to exist.
    file(WRITE ${gitinfo_file} "")
endif()

if(NOT "${gitinfo}" STREQUAL "${prev_gitinfo}")
    file(WRITE ${gitinfo_file} ${gitinfo})
endif()
