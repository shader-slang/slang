#
# Our wrapper for glslang
#
if(SLANG_ENABLE_SLANG_GLSLANG)
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
    # Our only interface is through what we define in source/slang-glslang, in the
    # interests of hygiene, hide anything else we link in. This hides our own non-exported symbols;
    # the version script below is what bounds the final export list.
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
    # Bounds the export list irrespective of where a symbol came from, which the visibility preset
    # and `--exclude-libs` above cannot do on their own.
    #
    # `slang-glslang.version-script` is the single source of truth for the exported names. ELF
    # consumes it directly. Mach-O has no version-script concept and instead takes an
    # `-exported_symbols_list`: a bare list of C symbol names, one per line, each with the leading
    # underscore ld64 prefixes to C symbols (`glslang_compile` -> `_glslang_compile`). On Apple we
    # derive that list from the same file at configure time so the two formats cannot drift -- only
    # the version-script is ever hand-maintained. See #12380.
    set(slang_glslang_version_script
        "${CMAKE_CURRENT_SOURCE_DIR}/slang-glslang.version-script"
    )
    if(NOT WIN32 AND NOT APPLE)
        add_supported_cxx_linker_flags(
            slang-glslang
            PRIVATE
            "-Wl,--version-script=${slang_glslang_version_script}"
        )
        set_property(
            TARGET slang-glslang
            APPEND
            PROPERTY LINK_DEPENDS "${slang_glslang_version_script}"
        )
    elseif(APPLE)
        file(
            READ
            "${slang_glslang_version_script}"
            _slang_glslang_version_script_content
        )
        # Isolate the `global:` allow-list block; CMake regex `.` matches newlines, so `(.*)` spans
        # the whole multi-line block up to `local:`. The captured group is read from CMAKE_MATCH_1.
        string(
            REGEX MATCH
            "global:(.*)local:"
            _slang_glslang_ignored
            "${_slang_glslang_version_script_content}"
        )
        set(_slang_glslang_global_block "${CMAKE_MATCH_1}")
        # Drop `#` comment lines so a commented-out name is not exported.
        string(
            REGEX REPLACE
            "#[^\n]*"
            ""
            _slang_glslang_global_block
            "${_slang_glslang_global_block}"
        )
        # Each exported entry is `name;`; pull the names, then prepend `_` for ld64's C-symbol spelling.
        string(
            REGEX MATCHALL
            "[A-Za-z_][A-Za-z0-9_]*[ \t]*;"
            _slang_glslang_exported_names
            "${_slang_glslang_global_block}"
        )
        set(_slang_glslang_exported_symbols_list "")
        foreach(_entry ${_slang_glslang_exported_names})
            string(REGEX REPLACE "[ \t]*;" "" _name "${_entry}")
            string(APPEND _slang_glslang_exported_symbols_list "_${_name}\n")
        endforeach()
        if(_slang_glslang_exported_symbols_list STREQUAL "")
            message(
                FATAL_ERROR
                "slang-glslang: extracted no exported names from ${slang_glslang_version_script}; its `global:` block format may have changed."
            )
        endif()
        # Catch a partial mis-parse (some but not all names): everything except the `name;` entries
        # must be whitespace. Anything left means an entry was not in the expected form and its export
        # would be silently dropped -- fail loud instead of shipping an under-bounded list.
        string(
            REGEX REPLACE
            "[A-Za-z_][A-Za-z0-9_]*[ \t]*;"
            ""
            _slang_glslang_global_block_residual
            "${_slang_glslang_global_block}"
        )
        string(
            REGEX REPLACE
            "[ \t\r\n]"
            ""
            _slang_glslang_global_block_residual
            "${_slang_glslang_global_block_residual}"
        )
        if(NOT _slang_glslang_global_block_residual STREQUAL "")
            message(
                FATAL_ERROR
                "slang-glslang: unparsed content in the `global:` block of ${slang_glslang_version_script} (`${_slang_glslang_global_block_residual}`); an export may be silently dropped."
            )
        endif()

        set(_slang_glslang_exported_symbols_file
            "${CMAKE_CURRENT_BINARY_DIR}/slang-glslang.exported-symbols.txt"
        )
        # Write only when the content changes so a reconfigure does not force a needless relink.
        set(_slang_glslang_exported_symbols_existing "")
        if(EXISTS "${_slang_glslang_exported_symbols_file}")
            file(
                READ
                "${_slang_glslang_exported_symbols_file}"
                _slang_glslang_exported_symbols_existing
            )
        endif()
        if(
            NOT _slang_glslang_exported_symbols_existing
                STREQUAL
                _slang_glslang_exported_symbols_list
        )
            file(
                WRITE
                "${_slang_glslang_exported_symbols_file}"
                "${_slang_glslang_exported_symbols_list}"
            )
        endif()
        # Re-run configure (regenerating the list above) whenever the canonical script is edited.
        set_property(
            DIRECTORY
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS "${slang_glslang_version_script}"
        )

        # Applied directly, not via add_supported_cxx_linker_flags: check_linker_flag cannot validate
        # this option because its probe defines none of the listed symbols and ld64 errors on them.
        target_link_options(
            slang-glslang
            PRIVATE
                "-Wl,-exported_symbols_list,${_slang_glslang_exported_symbols_file}"
        )
        set_property(
            TARGET slang-glslang
            APPEND
            PROPERTY LINK_DEPENDS "${_slang_glslang_exported_symbols_file}"
        )
    endif()
endif()
