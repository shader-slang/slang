# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

execute_process(
    COMMAND "${SLANGI}" -help
    OUTPUT_VARIABLE help
    RESULT_VARIABLE result
)
if(
    NOT result EQUAL 0
    OR NOT help MATCHES "-trace-all"
    OR NOT help MATCHES "-trace-n <num-events>"
)
    message(FATAL_ERROR "Trace option missing from help: ${help}")
endif()

execute_process(
    COMMAND "${SLANGI}" "${SOURCE}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
string(STRIP "${output}" output)
if(NOT result EQUAL 0 OR NOT output STREQUAL "42")
    message(FATAL_ERROR "Untraced output changed: ${output}\n${error}")
endif()

execute_process(
    COMMAND "${SLANGI}" -trace-n
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
if(result EQUAL 0 OR NOT error MATCHES "Missing value for -trace-n")
    message(FATAL_ERROR "Missing trace-limit diagnostic: ${output}\n${error}")
endif()
foreach(
    value
    IN
    ITEMS -1 abc 1.5 1tail 18446744073709551616
)
    execute_process(
        COMMAND "${SLANGI}" -trace-n "${value}" "${SOURCE}"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    if(result EQUAL 0 OR NOT error MATCHES "Invalid -trace-n value")
        message(
            FATAL_ERROR
            "Invalid trace limit accepted (${value}): ${output}\n${error}"
        )
    endif()
endforeach()
execute_process(
    COMMAND "${SLANGI}" -trace "${SOURCE}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
if(result EQUAL 0 OR NOT error MATCHES "Unknown option: -trace")
    message(FATAL_ERROR "Old trace option still accepted: ${output}\n${error}")
endif()

if(NOT TRACKING)
    foreach(option IN ITEMS "-trace-all" "-trace-n;5")
        execute_process(
            COMMAND "${SLANGI}" ${option} "${SOURCE}"
            OUTPUT_VARIABLE output
            ERROR_VARIABLE error
            RESULT_VARIABLE result
        )
        if(
            result EQUAL 0
            OR NOT error MATCHES "SLANG_ENABLE_HGVM_EVENT_TRACKING=ON"
            OR NOT output STREQUAL ""
        )
            message(
                FATAL_ERROR
                "Missing unavailable-tracing diagnostic: ${output}\n${error}"
            )
        endif()
    endforeach()
    return()
endif()

execute_process(
    COMMAND "${SLANGC}" "${SOURCE}" -target hgbc -g1 -o "${BINARY}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "HGBC compilation failed: ${output}\n${error}")
endif()

# Counts only event lines and checks that execution still produces the complete program output.
function(check_trace_count program expected_count)
    execute_process(
        COMMAND "${SLANGI}" ${ARGN} "${program}"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    string(REGEX MATCHALL "\\[hgvm\\] [^\n]*\n" events "${output}")
    list(LENGTH events event_count)
    string(REGEX REPLACE "\\[hgvm\\] [^\n]*\n" "" program_output "${output}")
    string(STRIP "${program_output}" program_output)
    if(
        NOT result EQUAL 0
        OR NOT event_count EQUAL expected_count
        OR NOT program_output STREQUAL "42"
        OR error MATCHES "\\[hgvm\\]"
    )
        message(
            FATAL_ERROR
            "Expected ${expected_count} trace events for ${ARGN} (${program}): ${output}\n${error}"
        )
    endif()
endfunction()

foreach(program IN ITEMS "${SOURCE}" "${BINARY}")
    execute_process(
        COMMAND "${SLANGI}" -trace-all "${program}"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    if(
        NOT result EQUAL 0
        OR NOT output MATCHES "\n42\r?\n"
        OR error MATCHES "\\[hgvm\\]"
    )
        message(FATAL_ERROR "Trace execution failed: ${output}\n${error}")
    endif()
    if(
        output MATCHES " (result|program|type-id)="
        OR output MATCHES " usage=0x"
    )
        message(
            FATAL_ERROR
            "Trace contains internal IDs, result codes, or numeric buffer usage: ${output}"
        )
    endif()
    if(
        NOT output
            MATCHES
            "buffer-create [^\n]*purpose=program usage=[A-Za-z|]*ShaderResource\\|UnorderedAccess\\|CopySource\\|CopyDestination"
    )
        message(FATAL_ERROR "Buffer usage flag names missing: ${output}")
    endif()
    foreach(
        event
        IN
        ITEMS
            heap-alloc
            heap-free
            module-load
            pipeline-create
            buffer-create
            batch-begin
            dispatch
            command-submit
            pipeline-complete
            buffer-recycle
            buffer-evict
    )
        if(NOT output MATCHES "\\[hgvm\\] ${event} ")
            message(
                FATAL_ERROR
                "Missing ${event} trace for ${program}: ${output}"
            )
        endif()
    endforeach()
    foreach(kind IN ITEMS ray-tracing-pipeline ray-acceleration-structure)
        foreach(event IN ITEMS heap-alloc heap-free)
            if(NOT output MATCHES "${event} [^\n]*kind=${kind} ")
                message(
                    FATAL_ERROR
                    "Missing ${kind} ${event} trace for ${program}: ${output}"
                )
            endif()
        endforeach()
    endforeach()
    string(
        REGEX MATCH
        "dispatch batch=[0-9]+ execution=([0-9]+)"
        dispatch
        "${output}"
    )
    set(execution_id "${CMAKE_MATCH_1}")
    if(
        NOT dispatch
        OR NOT output
            MATCHES
            "pipeline-complete batch=[0-9]+ execution=${execution_id} "
    )
        message(
            FATAL_ERROR
            "Dispatch and completion identities do not match: ${output}"
        )
    endif()
    foreach(event IN ITEMS module-load buffer-create pipeline-create)
        if(
            NOT output
                MATCHES
                "\\[hgvm\\] [^\n]*${event} [^\n]* duration-ms=[0-9]+\\.[0-9]+"
        )
            message(
                FATAL_ERROR
                "Missing ${event} duration for ${program}: ${output}"
            )
        endif()
    endforeach()
    foreach(event IN ITEMS pipeline-create dispatch pipeline-complete)
        if(NOT output MATCHES "${event} [^\n]* name=store<uint>([ \r\n]|$)")
            message(
                FATAL_ERROR
                "Missing specialized pipeline name for ${program}: ${output}"
            )
        endif()
    endforeach()
    if(output MATCHES "\\[hgvm\\] [^\n]*:[0-9]+:[0-9]+ ")
        message(
            FATAL_ERROR
            "Unexpected leading trace source location for ${program}: ${output}"
        )
    endif()
    string(REGEX MATCHALL " heap-alloc " allocations "${output}")
    string(REGEX MATCHALL " heap-free " frees "${output}")
    list(LENGTH allocations allocation_count)
    list(LENGTH frees free_count)
    if(NOT allocation_count EQUAL free_count)
        message(FATAL_ERROR "Cleanup events missing: ${output}")
    endif()

    string(REGEX MATCHALL "\\[hgvm\\] [^\n]*\n" events "${output}")
    list(LENGTH events event_count)
    foreach(limit IN ITEMS 0 1 5 ${event_count})
        check_trace_count("${program}" ${limit} -trace-n ${limit})
    endforeach()
    math(EXPR above_count "${event_count} + 1")
    check_trace_count("${program}" ${event_count} -trace-n ${above_count})
    check_trace_count("${program}" ${event_count} -trace-n 18446744073709551615)
    check_trace_count("${program}" ${event_count} -trace-n 1 -trace-all)
    check_trace_count("${program}" 1 -trace-all -trace-n 1)
endforeach()

execute_process(
    COMMAND "${SLANGC}" "${SOURCE}" -target hgbc -g0 -o "${BINARY}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Stripped HGBC compilation failed: ${output}\n${error}")
endif()
execute_process(
    COMMAND "${SLANGI}" -trace-all "${BINARY}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
if(
    NOT result EQUAL 0
    OR NOT output MATCHES "\\[hgvm\\] dispatch "
    OR output MATCHES "\\[hgvm\\] [^\n]*:[0-9]+:[0-9]+ "
    OR NOT output MATCHES "dispatch [^\n]* name=store<uint>([ \r\n]|$)"
)
    message(FATAL_ERROR "Tracing stripped HGBC failed: ${output}\n${error}")
endif()

get_filename_component(source_directory "${SOURCE}" DIRECTORY)
foreach(debug_level IN ITEMS 0 1)
    execute_process(
        COMMAND
            "${SLANGC}" "${source_directory}/trace-lambdas.slang" -target hgbc
            -g${debug_level} -o "${BINARY}.lambdas.hgbc"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(
            FATAL_ERROR
            "Lambda HGBC compilation failed: ${output}\n${error}"
        )
    endif()
    execute_process(
        COMMAND "${SLANGI}" -trace-all "${BINARY}.lambdas.hgbc"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0 OR NOT output MATCHES "\n42\r?\n")
        message(
            FATAL_ERROR
            "Lambda trace execution failed: ${output}\n${error}"
        )
    endif()
    if(debug_level EQUAL 1)
        foreach(event IN ITEMS pipeline-create dispatch pipeline-complete)
            if(
                NOT output
                    MATCHES
                    "${event} [^\n]*name=[^\n]*lambda@file\\([^\n]*trace-lambdas.slang:7:16\\)"
                OR NOT output
                    MATCHES
                    "${event} [^\n]*name=[^\n]*lambda@file\\([^\n]*trace-lambdas.slang:7:35\\)"
                OR NOT output
                    MATCHES
                    "${event} [^\n]*name=[^\n]*lambda@file\\([^\n]*trace_lambda_helpers.slang:16:16\\)"
            )
                message(
                    FATAL_ERROR
                    "Lambda definition locations missing: ${output}"
                )
            endif()
        endforeach()
        if(output MATCHES "name=[^\n]*_slang_Lambda_")
            message(
                FATAL_ERROR
                "Known lambda name was not substituted: ${output}"
            )
        endif()
    elseif(output MATCHES "lambda@" OR NOT output MATCHES "_slang_Lambda_")
        message(FATAL_ERROR "Stripped lambda names changed: ${output}")
    endif()
endforeach()
execute_process(
    COMMAND "${SLANGI}" -trace-all "${source_directory}/trace-metadata.slang"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
if(
    NOT result EQUAL 0
    OR NOT output MATCHES "\n42\r?\n"
    OR NOT output MATCHES "type=Tracing.Box<int>"
)
    message(FATAL_ERROR "Imported type metadata missing: ${output}\n${error}")
endif()

# Minimal debug info preserves signed loop indices and generic method scopes. Verify both
# compiler lowering and HGBC validation/execution, not just the presence of debug records.
execute_process(
    COMMAND
        "${SLANGI}" -trace-all
        "${source_directory}/../../../tests/compute/hgvm/class-generic.slang"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
if(
    NOT result EQUAL 0
    OR NOT output MATCHES "\n7\r?\n2\r?\n5\r?\n3\r?\n"
    OR NOT output MATCHES "type=Box<int, 2>"
    OR NOT output MATCHES "type=Box<float, 3>"
)
    message(FATAL_ERROR "Generic class tracing failed: ${output}\n${error}")
endif()

# Event formatting is independent of diagnostics, program output, and cursor control.
string(ASCII 27 escape)
foreach(value IN ITEMS invalid ALWAYS 1)
    execute_process(
        COMMAND "${SLANGI}" -color "${value}" "${SOURCE}"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    if(result EQUAL 0 OR NOT error MATCHES "Invalid -color value")
        message(FATAL_ERROR "Invalid color mode accepted: ${output}\n${error}")
    endif()
endforeach()
execute_process(
    COMMAND "${SLANGI}" -color
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
if(result EQUAL 0 OR NOT error MATCHES "Missing value for -color")
    message(FATAL_ERROR "Missing color mode diagnostic: ${output}\n${error}")
endif()

foreach(program IN ITEMS "${SOURCE}" "${BINARY}")
    foreach(mode IN ITEMS auto never always)
        execute_process(
            COMMAND "${SLANGI}" -trace-all -color ${mode} "${program}"
            OUTPUT_VARIABLE output
            ERROR_VARIABLE error
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Color trace failed: ${output}\n${error}")
        endif()
        if(mode STREQUAL "always")
            foreach(
                style
                IN
                ITEMS 1 2 32 33 35 36
            )
                string(FIND "${output}" "${escape}[${style}m" position)
                if(position LESS 0)
                    message(FATAL_ERROR "Missing style ${style}: ${output}")
                endif()
            endforeach()
        elseif(output MATCHES "${escape}")
            message(
                FATAL_ERROR
                "Redirected auto/never trace contains ANSI: ${output}"
            )
        endif()
        string(REGEX REPLACE "${escape}\\[[0-9;]*m" "" plain "${output}")
        if(
            plain MATCHES "${escape}"
            OR NOT plain MATCHES "name=store<uint>"
            OR plain MATCHES "name=graph-trace::"
            OR NOT output MATCHES "\n42\r?\n"
        )
            message(
                FATAL_ERROR
                "Compact trace or unstyled program output changed: ${output}"
            )
        endif()
    endforeach()
    execute_process(
        COMMAND "${SLANGI}" -trace-all -color never -full-names "${program}"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0 OR NOT output MATCHES "name=graph-trace::store<uint>")
        message(FATAL_ERROR "Full pipeline name missing: ${output}\n${error}")
    endif()
endforeach()

foreach(mode IN ITEMS auto never always)
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E env "NO_COLOR=1" "TERM=dumb" "${SLANGI}"
            -trace-n 1 -color ${mode} "${SOURCE}"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    string(FIND "${output}" "${escape}[1m" position)
    if(
        NOT result EQUAL 0
        OR (mode STREQUAL "always" AND position LESS 0)
        OR (NOT mode STREQUAL "always" AND output MATCHES "${escape}")
    )
        message(
            FATAL_ERROR
            "Color override/environment policy failed: ${output}\n${error}"
        )
    endif()
endforeach()

foreach(full IN ITEMS FALSE TRUE)
    set(name_option)
    if(full)
        set(name_option -full-names)
    endif()
    execute_process(
        COMMAND
            "${SLANGI}" -trace-all -color never ${name_option}
            "${source_directory}/trace-metadata.slang"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    if(
        NOT result EQUAL 0
        OR NOT output MATCHES "type=Tracing.Box<int>"
        OR (
            full
            AND NOT output MATCHES "type=Tracing.Box<int> module=trace_metadata_types"
        )
        OR (NOT full AND output MATCHES "type=Tracing.Box<int> module=")
    )
        message(
            FATAL_ERROR
            "Module display changed namespace/type identity: ${output}\n${error}"
        )
    endif()
    foreach(mode IN ITEMS auto never always)
        execute_process(
            COMMAND "${SLANGI}" -stats -color ${mode} ${name_option} "${SOURCE}"
            OUTPUT_VARIABLE output
            ERROR_VARIABLE error
            RESULT_VARIABLE result
        )
        string(STRIP "${output}" output)
        string(FIND "${error}" "${escape}[33m" duration_style)
        if(
            NOT result EQUAL 0
            OR NOT output STREQUAL "42"
            OR (mode STREQUAL "always" AND duration_style LESS 0)
            OR (NOT mode STREQUAL "always" AND error MATCHES "${escape}")
        )
            message(
                FATAL_ERROR
                "Statistics coloring changed stream behavior: ${output}\n${error}"
            )
        endif()
        string(REGEX REPLACE "${escape}\\[[0-9;]*m" "" plain "${error}")
        if(
            plain MATCHES "${escape}"
            OR NOT plain MATCHES "HGVM Live Statistics.*final"
            OR (full AND NOT plain MATCHES "graph-trace::store<uint>")
            OR (
                NOT full
                AND (
                    plain MATCHES "graph-trace::"
                    OR NOT plain MATCHES "compute store<uint>"
                )
            )
        )
            message(
                FATAL_ERROR
                "Statistics names or redirected cursor control incorrect: ${error}"
            )
        endif()
    endforeach()
endforeach()
