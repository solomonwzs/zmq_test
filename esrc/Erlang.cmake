find_program(ERLC erlc
    /usr/bin
    /usr/local/bin)

macro(add_erlang_beam)
    set(ERL_SOURCES ${ARGV})
    get_target_property(BUILD_ERLANG_TARGET build_erlang TYPE)
    if (NOT BUILD_ERLANG_TARGET)
        add_custom_target(build_erlang ALL)
    endif()

    add_dependencies(build_erlang ${ERL_SOURCES})

    foreach(ERL_SOURCE ${ERL_SOURCES})
        find_file(SOURCE_FILE ${ERL_SOURCE}.erl ${CMAKE_CURRENT_SOURCE_DIR})
        add_custom_target(${ERL_SOURCE}
            COMMAND ${ERLC} -Wall -o ${CMAKE_CURRENT_BINARY_DIR} ${SOURCE_FILE}
            COMMENT "Building .beam ${ERL_SOURCE}.beam")
        set_property(DIRECTORY APPEND PROPERTY
            ADDITIONAL_MAKE_CLEAN_FILES "${ERL_SOURCE}.beam")
    endforeach()
endmacro(add_erlang_beam)
