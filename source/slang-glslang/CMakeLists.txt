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
    # and `--exclude-libs` above cannot do on their own. macOS is not yet covered -- Mach-O needs
    # `-exported_symbols_list` with its own file format: see #12380.
    if(NOT WIN32 AND NOT APPLE)
        set(slang_glslang_version_script
            "${CMAKE_CURRENT_SOURCE_DIR}/slang-glslang.map"
        )
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
    endif()
endif()
