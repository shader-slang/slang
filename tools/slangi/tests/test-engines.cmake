# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

function(check_run expected_status expected_output expected_error)
    execute_process(
        COMMAND "${SLANGI}" ${ARGN}
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        RESULT_VARIABLE result
        TIMEOUT 30
    )
    if(NOT result MATCHES "^-?[0-9]+$")
        message(FATAL_ERROR "Interpreter did not exit normally: ${result}\n${output}\n${error}")
    endif()
    if(expected_status STREQUAL "success")
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Interpreter failed: ${ARGN}\n${output}\n${error}")
        endif()
    elseif(result EQUAL 0)
        message(FATAL_ERROR "Interpreter unexpectedly succeeded: ${ARGN}\n${output}\n${error}")
    endif()
    if(NOT output MATCHES "${expected_output}" OR NOT error MATCHES "${expected_error}")
        message(FATAL_ERROR "Unexpected interpreter output: ${ARGN}\n${output}\n${error}")
    endif()
endfunction()

check_run(success "-legacy" "" -help)
check_run(success "^42[\r\n]*$" "" "${SOURCE}")
check_run(success "^42[\r\n]*$" "" -hgvm "${SOURCE}")
check_run(success "% HGBC Disassembly" "" -disasm "${SOURCE}")
# Arguments after the filename belong to the legacy program, even when they look like options.
check_run(success "^legacy 3[\r\n]*$" ""
    -legacy -entry legacyMain "${SOURCE}" payload -hgvm)
check_run(success "func legacyMain" "" -legacy -entry legacyMain -disasm "${SOURCE}" payload)
check_run(failure "" "not marked .host.*Use -legacy" -entry ordinaryMain "${SOURCE}")
check_run(failure "" "omit -legacy" -legacy "${SOURCE}")
check_run(failure "" "cannot execute .hgbc files" -legacy unused.hgbc)

set(hgvm_options "-hgvm" "-debug" "-gpu-execution" "-report-checkpoint-intermediates")
if(TRACKING)
    list(APPEND hgvm_options "-stats" "-trace-all" "-trace-n")
endif()
foreach(option IN LISTS hgvm_options)
    set(arguments "${option}")
    if(option STREQUAL "-gpu-execution")
        list(APPEND arguments "default")
    elseif(option STREQUAL "-trace-n")
        list(APPEND arguments "0")
    endif()
    check_run(failure "" "-legacy cannot be combined" -legacy ${arguments} "${SOURCE}")
    check_run(failure "" "-legacy cannot be combined" ${arguments} -legacy "${SOURCE}")
endforeach()
