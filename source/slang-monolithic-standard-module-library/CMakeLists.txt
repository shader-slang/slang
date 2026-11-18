#
# Optional (-DSLANG_BUILD_MONOLITHIC_STANDARD_MODULE_LIBRARY=ON): build slang-standard-modules,
# a single shared library embedding all serialized standard modules. When
# enabled, this library is installed in place of the loose
# slang-standard-module-<version>/ tree; that tree is still built (it is this
# library's generator input) but its install() rules in source/standard-modules
# are skipped (guarded on NOT SLANG_BUILD_MONOLITHIC_STANDARD_MODULE_LIBRARY).
#
# This is a runtime-loaded MODULE library, loaded on demand by slang.dll rather
# than linked into it.
#
# Inputs are taken explicitly here rather than inherited from the standard-modules
# directory scope:
#   SLANG_STANDARD_MODULE_BUILD_SUBDIR  cache: build-tree subdir the producers write
#   SLANG_STANDARD_MODULE_FILES         cache: the built .slang-module files (manifest)
#   SLANG_STANDARD_MODULE_TARGETS       cache: the producer targets (manifest)
#   SLANG_VERSION_NUMERIC               project version
#   slang_BINARY_DIR                    project binary dir

# The built versioned module tree. $<CONFIG> is resolved when the custom command
# runs. This uses the build-tree subdir the producers actually write to -- not
# SLANG_STANDARD_MODULE_INSTALL_DIR, which is the (overridable) install
# destination and may point elsewhere.
set(embed_module_root
    "${slang_BINARY_DIR}/$<CONFIG>/${SLANG_STANDARD_MODULE_BUILD_SUBDIR}"
)

# Generate the payload source per-configuration so multi-config builds (e.g.
# Debug and Release, whose module roots differ by $<CONFIG>) do not race on, or
# reuse, one another's generated source. The include directory carries $<CONFIG>
# to match, so the wrapper's #include resolves to this configuration's payload.
set(embed_generated_dir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
set(embed_generated_source "${embed_generated_dir}/slang-monolithic-standard-module-library-generated.cpp")

# Scan the built tree, LZ4-compress each module, and emit the embedded byte
# arrays + lookup table. The generator links core for LZ4CompressionSystem; the
# compiler decompresses with the same system at load.
#
# DEPENDS lists both the producer targets and their output files:
#   - the module targets enforce build ordering (under Visual Studio, file-level
#     dependencies across directories do not order the generating projects, so
#     without these the embedding can start before every .slang-module exists);
#   - the SLANG_STANDARD_MODULE_FILES manifest tracks the actual outputs, so
#     editing a module regenerates the payload instead of leaving it stale.
add_custom_command(
    OUTPUT ${embed_generated_source}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${embed_generated_dir}
    COMMAND
        slang-generate-monolithic-standard-module-library
        ${embed_module_root}
        ${embed_generated_source}
    DEPENDS
        slang-generate-monolithic-standard-module-library
        ${SLANG_STANDARD_MODULE_TARGETS}
        ${SLANG_STANDARD_MODULE_FILES}
    COMMENT "Embedding standard modules (LZ4) into ${embed_generated_source}"
    VERBATIM
)
add_custom_target(
    generate_standard_modules_embed_source
    DEPENDS ${embed_generated_source}
)
set_target_properties(
    generate_standard_modules_embed_source
    PROPERTIES FOLDER generated
)

# Build the monolithic library itself: a MODULE (runtime-loaded) library that
# links core privately and hides all symbols except the two C entry points. The
# generated payload is #included by the wrapper (not compiled as its own
# translation unit), so it is reached through INCLUDE_DIRECTORIES_PRIVATE rather
# than listed as a source.
slang_add_target(
    .
    MODULE
    TARGET_NAME slang-standard-modules
    OUTPUT_NAME slang-standard-modules
    FOLDER generated
    LINK_WITH_PRIVATE core
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE ./slang-monolithic-standard-module-library.cpp
    REQUIRES generate_standard_modules_embed_source
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
    EXPORT_SET_NAME SlangTargets
    INCLUDE_DIRECTORIES_PRIVATE
        ${embed_generated_dir}
        ${CMAKE_CURRENT_BINARY_DIR}/../slang/slang-version-header
    INSTALL
)
set_target_properties(
    slang-standard-modules
    PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
)
if(NOT WIN32)
    # Versioned filename on Mac/Linux: these libraries are deployed in a directory
    # that ends up in the library search path, so the version suffix avoids
    # collisions. Windows stays unversioned and relies on the build-tag check in
    # the loader.
    set_target_properties(
        slang-standard-modules
        PROPERTIES OUTPUT_NAME slang-standard-modules-${SLANG_VERSION_NUMERIC}
    )
endif()
add_supported_cxx_linker_flags(
    slang-standard-modules
    PRIVATE
    "-Wl,--exclude-libs,ALL"
)
