if(NOT DEFINED SLANG_RUN_IF_OUTDATED_OUTPUTS)
    message(FATAL_ERROR "SLANG_RUN_IF_OUTDATED_OUTPUTS is required")
endif()

if(NOT DEFINED SLANG_RUN_IF_OUTDATED_COMMAND)
    message(FATAL_ERROR "SLANG_RUN_IF_OUTDATED_COMMAND is required")
endif()

foreach(output IN LISTS SLANG_RUN_IF_OUTDATED_OUTPUTS)
    if(NOT EXISTS "${output}")
        set(SLANG_RUN_IF_OUTDATED_NEEDED TRUE)
    endif()
endforeach()

if(NOT SLANG_RUN_IF_OUTDATED_NEEDED)
    foreach(input IN LISTS SLANG_RUN_IF_OUTDATED_INPUTS)
        foreach(output IN LISTS SLANG_RUN_IF_OUTDATED_OUTPUTS)
            if("${input}" IS_NEWER_THAN "${output}")
                set(SLANG_RUN_IF_OUTDATED_NEEDED TRUE)
            endif()
        endforeach()
    endforeach()
endif()

if(NOT SLANG_RUN_IF_OUTDATED_NEEDED)
    message(STATUS "Skipping ${SLANG_RUN_IF_OUTDATED_DESCRIPTION}; outputs are up to date")
    return()
endif()

execute_process(
    COMMAND ${SLANG_RUN_IF_OUTDATED_COMMAND}
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "${SLANG_RUN_IF_OUTDATED_DESCRIPTION} failed with exit code ${result}")
endif()
