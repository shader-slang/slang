#
# Our wrapper for glslang
#
if(SLANG_ENABLE_SLANG_GLSLANG)
    slang_add_target(
        .
        MODULE
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE glslang SPIRV SPIRV-Tools-opt SPIRV-Tools-link
        INCLUDE_DIRECTORIES_PRIVATE
            ${slang_SOURCE_DIR}/include
            ${CMAKE_CURRENT_BINARY_DIR}/../slang/slang-version-header
        INSTALL
        EXPORT_SET_NAME SlangTargets
        DEBUG_INFO_INSTALL_COMPONENT slang-glslang-debug-info
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
    add_supported_cxx_linker_flags(
        slang-glslang
        PRIVATE
        "-Wl,--exclude-libs,ALL"
    )
endif()
