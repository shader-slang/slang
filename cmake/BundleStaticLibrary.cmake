#
# Merge a static library and every static library it links into one self-contained archive.
#
# Linking against a static Slang otherwise means naming a dozen archives in the right
# order: slang-compiler, compiler-core, core, slang-glslang-static, then glslang, the three
# SPIRV-Tools archives, miniz, lz4 and cmark-gfm. That list is an internal implementation
# detail, it changes between releases, and build systems that are not CMake (Cargo, in
# particular) have no way to discover it. Merging everything into a single archive lets a
# consumer link with one `-lslang-static` and nothing else, which is what makes a prebuilt
# static Slang distributable.
#
# Merging archives is not the same as nesting them: the result is a flat archive holding
# every member object, so the linker resolves symbols across the whole set exactly as it
# would if all the individual archives had been named on the command line. Member names are
# allowed to collide -- SPIRV-Tools and SPIRV-Tools-opt both contain a `basic_block.cpp.o`,
# for instance -- and both `ld` and `lib.exe` cope with that. Only naive extraction with
# `ar x` would clobber one with the other.
#

# Return every distinct target name mentioned in a link-libraries entry, which may be
# wrapped in generator expressions.
#
# slang_add_target wraps its private dependencies as `$<BUILD_LOCAL_INTERFACE:core>` so they
# stay out of the install interface, CMake itself adds `$<LINK_ONLY:...>` around private
# dependencies of static libraries, and a few entries select between targets with
# `$<IF:$<BOOL:ON>,slang-embedded-core-module,slang-no-embedded-core-module>`. Rather than
# evaluate those expressions at configure time, pull out every identifier-shaped token and
# let the caller discard the ones that are not targets. Over-collecting is safe here: the
# caller keeps only STATIC_LIBRARY targets, and where an `$<IF:...>` really does select
# between two targets, both branches are OBJECT libraries whose members already live inside
# the archive of whatever links them.
function(_slang_link_entry_target_names entry out_var)
    # Identifiers may contain `::` (e.g. `SPIRV-Headers::SPIRV-Headers`), so match that too
    # rather than splitting on `:`, which is also generator-expression syntax.
    string(
        REGEX MATCHALL
        "[A-Za-z0-9_.+-]+(::[A-Za-z0-9_.+-]+)*"
        names
        "${entry}"
    )
    set(${out_var} ${names} PARENT_SCOPE)
endfunction()

# Collect the static archives reachable from `root` through its link graph, `root` first.
#
# OBJECT libraries are skipped deliberately: CMake places their members directly into the
# archive of the target that links them, so adding them again would duplicate every object.
# INTERFACE targets carry no archive of their own but are still traversed, because they can
# forward real dependencies.
function(slang_collect_static_link_archives root out_var)
    set(worklist ${root})
    set(seen "")
    set(archives "")

    while(worklist)
        list(POP_FRONT worklist item)
        _slang_link_entry_target_names("${item}" names)

        foreach(name ${names})
            if(NOT TARGET ${name})
                continue()
            endif()
            if(${name} IN_LIST seen)
                continue()
            endif()
            list(APPEND seen ${name})

            get_target_property(type ${name} TYPE)
            if(type STREQUAL "STATIC_LIBRARY")
                list(APPEND archives ${name})
            endif()

            # INTERFACE libraries have no LINK_LIBRARIES property; reading it is an error
            # rather than a no-op, so consult only the interface list for them.
            set(deps "")
            if(NOT type STREQUAL "INTERFACE_LIBRARY")
                get_target_property(link_libs ${name} LINK_LIBRARIES)
                if(link_libs)
                    list(APPEND deps ${link_libs})
                endif()
            endif()
            get_target_property(interface_libs ${name} INTERFACE_LINK_LIBRARIES)
            if(interface_libs)
                list(APPEND deps ${interface_libs})
            endif()

            if(deps)
                list(APPEND worklist ${deps})
            endif()
        endforeach()
    endwhile()

    set(${out_var} ${archives} PARENT_SCOPE)
endfunction()

# Create a target named `output_name` that merges `root` and its static dependencies into a
# single archive, and return that archive's path in `out_var`.
#
# The archive paths are only known per configuration, so they are written to a generated
# response file with file(GENERATE) and the merge itself runs as a `cmake -P` step. Using a
# response file rather than passing the list on the command line keeps paths with spaces
# and CMake's list separator from needing another layer of escaping.
function(slang_bundle_static_library root output_name out_var)
    slang_collect_static_link_archives(${root} archives)

    if(NOT archives)
        message(
            FATAL_ERROR
            "slang_bundle_static_library: no static archives are reachable from '${root}'. Bundling requires SLANG_LIB_TYPE=STATIC."
        )
    endif()

    list(JOIN archives ", " archives_pretty)
    message(STATUS "Bundling ${output_name} from: ${archives_pretty}")

    set(bundled
        "${CMAKE_BINARY_DIR}/$<CONFIG>/${library_subdir}/${CMAKE_STATIC_LIBRARY_PREFIX}${output_name}${CMAKE_STATIC_LIBRARY_SUFFIX}"
    )

    set(archive_files "")
    foreach(archive ${archives})
        list(APPEND archive_files "$<TARGET_FILE:${archive}>")
    endforeach()

    # Pick the merge tool. None of the three take the same arguments, and BSD ar has no MRI
    # mode at all, so the choice is made here and passed to the script.
    if(MSVC)
        set(merge_tool "lib")
        set(merge_program "${CMAKE_AR}")
    elseif(APPLE)
        find_program(SLANG_LIBTOOL NAMES libtool REQUIRED)
        set(merge_tool "libtool")
        set(merge_program "${SLANG_LIBTOOL}")
    elseif(MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
        set(merge_tool "ar")
        set(merge_program "${CMAKE_AR}")
    else()
        message(
            FATAL_ERROR
            "SLANG_BUNDLE_STATIC_LIB does not know how to merge archives for compiler '${CMAKE_CXX_COMPILER_ID}' on '${CMAKE_SYSTEM_NAME}'. Supported: MSVC (lib.exe), Apple (libtool), GNU/Clang ar (MRI script)."
        )
    endif()

    set(response "${CMAKE_CURRENT_BINARY_DIR}/${output_name}-$<CONFIG>.rsp")
    file(GENERATE OUTPUT "${response}" CONTENT "$<JOIN:${archive_files},\n>\n")

    # Depend on the archive files so the bundle is rebuilt when any input changes, and on
    # the targets so build ordering is right. Imported targets have a file but cannot be
    # named in a dependency edge.
    set(depend_targets "")
    foreach(archive ${archives})
        get_target_property(is_imported ${archive} IMPORTED)
        if(NOT is_imported)
            list(APPEND depend_targets ${archive})
        endif()
    endforeach()

    add_custom_command(
        OUTPUT "${bundled}"
        COMMAND
            ${CMAKE_COMMAND} "-DSLANG_BUNDLE_OUTPUT=${bundled}"
            "-DSLANG_BUNDLE_RESPONSE=${response}"
            "-DSLANG_BUNDLE_TOOL=${merge_tool}"
            "-DSLANG_BUNDLE_PROGRAM=${merge_program}" -P
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/BundleStaticLibraryImpl.cmake"
        DEPENDS ${archive_files} ${depend_targets} "${response}"
        COMMENT "Bundling static library ${output_name}"
        VERBATIM
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(${output_name} ALL DEPENDS "${bundled}")

    set(${out_var} "${bundled}" PARENT_SCOPE)
endfunction()
