#
# Our wrapper for glslang
#
if(SLANG_ENABLE_SLANG_GLSLANG)
    if(SLANG_LIB_TYPE STREQUAL "STATIC" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
        #
        # Static-link path (Linux): instead of a runtime-loaded MODULE, build the
        # wrapper as a symbol-localized static object that core `slang` links at
        # compile time, so a STATIC slangc works with no libslang-glslang*.so.
        #
        # The bundled SPIRV-Tools/glslang symbols are default-global in their .a
        # archives, and the MODULE's `--exclude-libs,ALL`/hidden-visibility action
        # only applies to a shared link, so it does not survive into a static
        # archive. To avoid ODR collisions when a consumer app links its own
        # SPIRV-Tools, we partial-link (`ld -r`) the wrapper object plus the
        # `--whole-archive` set of bundled archives into one relocatable object,
        # then localize the bundled strong global symbols (see
        # localize-static-symbols.sh) so only our 7 entry points stay global.
        # Weak/COMDAT symbols are intentionally left global so the final link's
        # COMDAT de-duplication keeps working.
        #

        # Build the wrapper translation unit as an OBJECT library so we can grab
        # its object(s) cleanly. Reuse the same private deps / includes / defs as
        # the MODULE path so the compiled code is identical.
        slang_add_target(
            .
            OBJECT
            USE_FEWER_WARNINGS
            SKIP_ASAN
            TARGET_NAME slang-glslang-obj
            LINK_WITH_PRIVATE glslang SPIRV SPIRV-Tools-opt SPIRV-Tools-link
            INCLUDE_DIRECTORIES_PRIVATE
                ${slang_SOURCE_DIR}/include
                ${CMAKE_CURRENT_BINARY_DIR}/../slang/slang-version-header
        )
        target_compile_definitions(
            slang-glslang-obj
            PRIVATE
                SLANG_ENABLE_SPIRV_OPT_MERGE_RETURN=$<BOOL:${SLANG_ENABLE_SPIRV_OPT_MERGE_RETURN}>
        )
        add_supported_cxx_flags(slang-glslang-obj PRIVATE -fno-rtti)

        # The bundled archive targets to fold in. opt/link transitively depend on
        # the SPIRV-Tools core; include it exactly once. Different SPIRV-Tools
        # versions expose the static core as both `SPIRV-Tools` and
        # `SPIRV-Tools-static`, which resolve to the same libSPIRV-Tools.a;
        # whole-archiving both would cause multiple-definition errors, so add only
        # the first that exists.
        set(slang_glslang_bundled_archives
            glslang
            SPIRV
            SPIRV-Tools-opt
            SPIRV-Tools-link
        )
        foreach(core_candidate SPIRV-Tools-static SPIRV-Tools)
            if(TARGET ${core_candidate})
                list(APPEND slang_glslang_bundled_archives ${core_candidate})
                break()
            endif()
        endforeach()
        if(TARGET SPIRV-Tools-reduce)
            list(APPEND slang_glslang_bundled_archives SPIRV-Tools-reduce)
        endif()

        set(slang_glslang_localized_obj
            ${CMAKE_CURRENT_BINARY_DIR}/slang-glslang-localized${CMAKE_CXX_OUTPUT_EXTENSION}
        )
        set(slang_glslang_combined_obj
            ${CMAKE_CURRENT_BINARY_DIR}/slang-glslang-combined${CMAKE_CXX_OUTPUT_EXTENSION}
        )

        # The 7 extern "C" entry points that must stay GLOBAL; mirror the bindings
        # in source/compiler-core/slang-glslang-compiler.cpp.
        set(slang_glslang_entry_points
            glslang_compile
            glslang_compile_1_1
            glslang_compile_1_2
            glslang_validateSPIRV
            glslang_disassembleSPIRV
            glslang_disassembleSPIRVWithResult
            glslang_linkSPIRV
        )
        # nm is used by the localization step to enumerate strong global symbols.
        set(slang_glslang_nm ${CMAKE_NM})
        if(NOT slang_glslang_nm)
            set(slang_glslang_nm nm)
        endif()

        # Per-archive whole-archive flags so every member is pulled into the
        # relocatable object.
        set(slang_glslang_archive_files "")
        foreach(archive ${slang_glslang_bundled_archives})
            list(APPEND slang_glslang_archive_files $<TARGET_FILE:${archive}>)
        endforeach()

        add_custom_command(
            OUTPUT ${slang_glslang_localized_obj}
            BYPRODUCTS ${slang_glslang_combined_obj}
            # 1. Partial-link the wrapper object(s) + all bundled archive members
            #    into a single relocatable object.
            COMMAND
                ${CMAKE_LINKER} -r -o ${slang_glslang_combined_obj}
                $<TARGET_OBJECTS:slang-glslang-obj> --whole-archive
                ${slang_glslang_archive_files} --no-whole-archive
            # 2. Localize the bundled strong global symbols, keeping the 7 entry
            #    points global and leaving weak/COMDAT symbols intact. See
            #    localize-static-symbols.sh for why only strong globals are
            #    localized (localizing weak/COMDAT symbols breaks the final link).
            COMMAND
                bash ${CMAKE_CURRENT_SOURCE_DIR}/localize-static-symbols.sh
                ${slang_glslang_nm} ${CMAKE_OBJCOPY}
                ${slang_glslang_combined_obj} ${slang_glslang_localized_obj}
                ${slang_glslang_entry_points}
            DEPENDS
                slang-glslang-obj
                $<TARGET_OBJECTS:slang-glslang-obj>
                ${slang_glslang_bundled_archives}
                ${CMAKE_CURRENT_SOURCE_DIR}/localize-static-symbols.sh
            COMMAND_EXPAND_LISTS
            VERBATIM
            COMMENT
                "Localizing slang-glslang into a self-contained static object"
        )

        # Wrap the localized object as a STATIC library target. compiler-core
        # links it PUBLICly (see source/compiler-core/CMakeLists.txt), so the
        # dependency propagates transitively to every compiler-core consumer.
        add_custom_target(
            slang-glslang-localized-obj
            DEPENDS ${slang_glslang_localized_obj}
        )
        add_library(slang-glslang-static STATIC ${slang_glslang_localized_obj})
        add_dependencies(slang-glslang-static slang-glslang-localized-obj)
        set_target_properties(
            slang-glslang-static
            PROPERTIES LINKER_LANGUAGE CXX
        )
        # compiler-core links this PUBLICly (see source/compiler-core), so it is
        # referenced by slang's exported interface; an installed STATIC slang
        # consumed via find_package(slang) therefore needs the localized archive
        # too. Add it to the export set and install it with the other archives.
        install(
            TARGETS slang-glslang-static
            EXPORT SlangTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    else()
        slang_add_target(
            .
            MODULE
            USE_FEWER_WARNINGS
            SKIP_ASAN
            LINK_WITH_PRIVATE glslang SPIRV SPIRV-Tools-opt SPIRV-Tools-link
            INCLUDE_DIRECTORIES_PRIVATE
                ${slang_SOURCE_DIR}/include
                ${CMAKE_CURRENT_BINARY_DIR}/../slang/slang-version-header
            INSTALL
            EXPORT_SET_NAME SlangTargets
        )
        target_compile_definitions(
            slang-glslang
            PRIVATE
                SLANG_ENABLE_SPIRV_OPT_MERGE_RETURN=$<BOOL:${SLANG_ENABLE_SPIRV_OPT_MERGE_RETURN}>
        )
        # Our only interface is through what we define in source/slang-glslang, in the
        # interests of hygiene, hide anything else we link in.
        set_target_properties(
            slang-glslang
            PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
        )
        if(NOT WIN32)
            # The module name is manually versioned on Mac and Linux platforms, but not on Windows.
            # Modules are runtime-loaded libraries. We need to embed a version string in the
            # filename on Mac (and Linux) platforms because these files are deployed in a directory
            # that ends up in the library path. (This can cause conflicts with other versions.)
            # See also locateGlslangSpirvDownstreamCompiler() in slang-glslang-compiler.cpp
            set_target_properties(
                slang-glslang
                PROPERTIES OUTPUT_NAME slang-glslang-${SLANG_VERSION_NUMERIC}
            )
        endif()
        add_supported_cxx_flags(slang-glslang PRIVATE -fno-rtti)
        add_supported_cxx_linker_flags(
            slang-glslang
            PRIVATE
            "-Wl,--exclude-libs,ALL"
        )
    endif()
endif()
