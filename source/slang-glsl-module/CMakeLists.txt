#
# Generate an embeddable glsl module
#

# This header is generated in the same call to slang-bootstrap that generates
# the core module embed, which can be found in
# ../slang-core-module/CMakeLists.txt
# The target to look for is generate_core_module_headers

# CMake doesn't allow creating an implicit dependency across directory scopes,
# see https://cmake.org/cmake/help/v3.31/prop_sf/GENERATED.html. Because of
# this we add the explicit REQUIRES
slang_add_target(
    .
    MODULE
    FOLDER generated
    LINK_WITH_PRIVATE core
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE
        ./slang-embedded-glsl-module.cpp
        ${glsl_module_generated_header}
    REQUIRES generate_core_module_headers
    EXCLUDE_FROM_ALL
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
    EXPORT_SET_NAME SlangTargets
    INCLUDE_DIRECTORIES_PRIVATE
        ${glsl_module_generated_header_dir}
        ${CMAKE_CURRENT_BINARY_DIR}/../slang/slang-version-header
    INSTALL
)
set_target_properties(
    slang-glsl-module
    PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
)
if(NOT WIN32)
    # The module name is manually versioned on Mac and Linux platforms, but not on Windows.
    # Modules are runtime-loaded libraries. The Mac toolchain
    # doesn't allow the versioning parameters to be used on modules, but we
    # still need to embed a version string in the filename on Mac (and Linux) platforms
    # because these files are deployed in a directory that ends up in the library path.
    # (This can cause conflicts with other versions of the same module.)
    # See also tryLoadBuiltinModuleFromDLL in slang-api.cpp
    set_target_properties(
        slang-glsl-module
        PROPERTIES OUTPUT_NAME slang-glsl-module-${SLANG_VERSION_NUMERIC}
    )
endif()
add_supported_cxx_linker_flags(
    slang-glsl-module
    PRIVATE
    "-Wl,--exclude-libs,ALL"
)
