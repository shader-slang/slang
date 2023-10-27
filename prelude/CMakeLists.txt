glob_append(SLANG_PRELUDE_HEADERS "*-prelude.h")

set(SLANG_PRELUDE_SOURCE)
foreach(input ${SLANG_PRELUDE_HEADERS})
    get_filename_component(input_name ${input} NAME)
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${input_name}.cpp")
    add_custom_command(
        OUTPUT ${output}
        COMMAND slang-embed "${input}" ${output}
        DEPENDS ${input}
        VERBATIM
    )
    list(APPEND SLANG_PRELUDE_SOURCE ${output})
endforeach()

add_library(prelude OBJECT EXCLUDE_FROM_ALL ${SLANG_PRELUDE_SOURCE})
set_default_compile_options(prelude)
target_include_directories(prelude PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
