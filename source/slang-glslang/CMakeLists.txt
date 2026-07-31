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

#
# The same wrapper, built as a static library to be linked directly into the compiler
#
# This is the piece that makes a fully static Slang possible. Slang emits SPIR-V natively,
# but the SPIRV-Tools optimizer, linker, validator and disassembler all live behind the
# slang-glslang ABI, so an optimized (or validated, or multi-module) SPIR-V compile has to
# reach them somehow. In the default build that means dlopen()ing the slang-glslang module at
# runtime; here we compile the exact same sources into an archive and let
# GlslangDownstreamCompiler bind to the symbols directly (see the static path in
# source/compiler-core/slang-glslang-compiler.cpp).
#
# The definition is PUBLIC because slang-glslang-compiler.cpp, which lives in compiler-core,
# needs SLANG_GLSLANG_STATIC to be set as well: it selects the undecorated prototypes in
# slang-glslang.h over the __declspec(dllexport) declarations used by the module build.
#
if(SLANG_EMBED_SLANG_GLSLANG)
    slang_add_target(
        .
        STATIC
        TARGET_NAME slang-glslang-static
        USE_FEWER_WARNINGS
        SKIP_ASAN
        EXCLUDE_FROM_ALL
        EXTRA_COMPILE_DEFINITIONS_PUBLIC SLANG_GLSLANG_STATIC
        LINK_WITH_PRIVATE glslang SPIRV SPIRV-Tools-opt SPIRV-Tools-link
        INCLUDE_DIRECTORIES_PRIVATE
            ${slang_SOURCE_DIR}/include
            ${CMAKE_CURRENT_BINARY_DIR}/../slang/slang-version-header
    )
    # Keep glslang's and SPIRV-Tools' symbols out of whatever ends up linking this. It only
    # covers this target's own objects; the dependency archives are governed by their own
    # visibility settings, so a SHARED slang built with SLANG_EMBED_SLANG_GLSLANG can still
    # re-export some of them.
    set_target_properties(
        slang-glslang-static
        PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
    )
    add_supported_cxx_flags(slang-glslang-static PRIVATE -fno-rtti)

    # slang-glslang-compiler.cpp, which references these entry points, is part of
    # compiler-core, so that is where the archive has to be attached.
    target_link_libraries(compiler-core PRIVATE slang-glslang-static)

    if(SLANG_ENABLE_SLANG_GLSLANG)
        message(
            STATUS
            "SLANG_EMBED_SLANG_GLSLANG is ON, so slang-compiler resolves glslang and SPIRV-Tools statically and never loads the slang-glslang module at runtime. The module is still being built because SLANG_ENABLE_SLANG_GLSLANG is also ON; set it to OFF for a static-only build."
        )
    endif()
endif()
