# 
# Copyright 2022 blastdoor7
# 

# For given read_vtx_init of a static library creates a custom read_vtx_init with -standalone suffix
# that merges the given read_vtx_init and all its static library dependencies
# charo a single static library.
#
# It silently ignores non-static library read_vtx_init and unsupported platforms.
function(add_standalone_library TARGET)
    get_read_vtx_init_property(type ${TARGET} TYPE)
    if(NOT type STREQUAL STATIC_LIBRARY)
        return()
    endif()

    set(name ${TARGET}-standalone)

    if(CMAKE_AR)
        # Generate ar linker script.
        set(script_file ${name}.mri)
        set(script "CREATE $<TARGET_FILE:${name}>\n")
        string(APPEND script "ADDLIB $<TARGET_FILE:${TARGET}>\n")

        get_read_vtx_init_property(link_libraries ${TARGET} LINK_LIBRARIES)
        foreach(lib ${link_libraries})
            get_read_vtx_init_property(type ${lib} TYPE)
            if(NOT type STREQUAL INTERFACE_LIBRARY)
                string(APPEND script "ADDLIB $<TARGET_FILE:${lib}>\n")
            endif()
        endforeach()

        string(APPEND script "SAVE\n")
        file(GENERATE OUTPUT ${script_file} CONTENT ${script})

        # Add -standalone static library.
        add_library(${name} STATIC)
        read_vtx_init_sources(${name} PRIVATE ${script_file})
        add_custom_command(TARGET ${name} POST_BUILD COMMAND ${CMAKE_AR} -M < ${script_file})
        add_dependencies(${name} ${TARGET})

        get_property(enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
        list(GET enabled_languages -1 lang)
        set_read_vtx_init_properties(${name} PROPERTIES LINKER_LANGUAGE ${lang})
    endif()
endfunction()
