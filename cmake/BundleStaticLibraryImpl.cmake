#
# Merge the archives listed in a response file into one archive. Run via `cmake -P` from
# the custom command created by slang_bundle_static_library(); see BundleStaticLibrary.cmake
# for why the work lives in a script rather than directly in the custom command.
#
# Expected definitions:
#   SLANG_BUNDLE_OUTPUT    path of the archive to produce
#   SLANG_BUNDLE_RESPONSE  file listing one input archive path per line
#   SLANG_BUNDLE_TOOL      one of `ar`, `libtool`, `lib`
#   SLANG_BUNDLE_PROGRAM   the executable implementing SLANG_BUNDLE_TOOL
#

foreach(
    required
    SLANG_BUNDLE_OUTPUT
    SLANG_BUNDLE_RESPONSE
    SLANG_BUNDLE_TOOL
    SLANG_BUNDLE_PROGRAM
)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "BundleStaticLibraryImpl: ${required} is not set")
    endif()
endforeach()

file(STRINGS "${SLANG_BUNDLE_RESPONSE}" libs)
if(NOT libs)
    message(
        FATAL_ERROR
        "BundleStaticLibraryImpl: '${SLANG_BUNDLE_RESPONSE}' lists no archives"
    )
endif()

foreach(lib ${libs})
    if(NOT EXISTS "${lib}")
        message(
            FATAL_ERROR
            "BundleStaticLibraryImpl: input archive '${lib}' is missing"
        )
    endif()
endforeach()

# Always start from nothing. `ar -M`'s `create` truncates, but lib.exe and libtool would
# happily merge into a stale output and silently keep objects from a previous build.
file(REMOVE "${SLANG_BUNDLE_OUTPUT}")
get_filename_component(output_dir "${SLANG_BUNDLE_OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${output_dir}")

if(SLANG_BUNDLE_TOOL STREQUAL "ar")
    # GNU ar and llvm-ar merge archives through an MRI script, which has to arrive on
    # stdin. `addlib` splices in the members of an existing archive rather than nesting it.
    #
    # The MRI format has no quoting or escaping, so a path containing a space cannot be
    # represented at all -- ar would read it as two tokens and fail somewhere mid-merge
    # with an unrelated-looking error. Reject such paths up front with a real message.
    foreach(path "${SLANG_BUNDLE_OUTPUT}" ${libs})
        if(path MATCHES " ")
            message(
                FATAL_ERROR
                "BundleStaticLibraryImpl: '${path}' contains a space, which ar's MRI script format cannot represent. Build in a directory whose path has no spaces."
            )
        endif()
    endforeach()
    set(mri "${SLANG_BUNDLE_OUTPUT}.mri")
    set(mri_content "create ${SLANG_BUNDLE_OUTPUT}\n")
    foreach(lib ${libs})
        string(APPEND mri_content "addlib ${lib}\n")
    endforeach()
    string(APPEND mri_content "save\nend\n")
    file(WRITE "${mri}" "${mri_content}")

    execute_process(
        COMMAND "${SLANG_BUNDLE_PROGRAM}" -M
        INPUT_FILE "${mri}"
        COMMAND_ERROR_IS_FATAL ANY
    )
elseif(SLANG_BUNDLE_TOOL STREQUAL "libtool")
    # BSD ar has no MRI mode; Apple's libtool is the supported way to merge archives.
    execute_process(
        COMMAND
            "${SLANG_BUNDLE_PROGRAM}" -static -o "${SLANG_BUNDLE_OUTPUT}"
            ${libs}
        COMMAND_ERROR_IS_FATAL ANY
    )
elseif(SLANG_BUNDLE_TOOL STREQUAL "lib")
    execute_process(
        COMMAND
            "${SLANG_BUNDLE_PROGRAM}" /NOLOGO "/OUT:${SLANG_BUNDLE_OUTPUT}"
            ${libs}
        COMMAND_ERROR_IS_FATAL ANY
    )
else()
    message(
        FATAL_ERROR
        "BundleStaticLibraryImpl: unknown SLANG_BUNDLE_TOOL '${SLANG_BUNDLE_TOOL}'"
    )
endif()

if(NOT EXISTS "${SLANG_BUNDLE_OUTPUT}")
    message(
        FATAL_ERROR
        "BundleStaticLibraryImpl: ${SLANG_BUNDLE_TOOL} reported success but '${SLANG_BUNDLE_OUTPUT}' was not produced"
    )
endif()
