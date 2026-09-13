# Don't let all these packages spam up with status messages, filter to only the
# important stuff.
# Don't change things however if the user has explicitly set
# CMAKE_MESSAGE_LOG_LEVEL
set(set_cmake_log_level FALSE)
if(NOT CMAKE_MESSAGE_LOG_LEVEL)
    set(set_cmake_log_level TRUE)
    set(CMAKE_MESSAGE_LOG_LEVEL NOTICE)
endif()

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.25")
    set(system SYSTEM)
else()
    message(
        STATUS
        "CMake 3.25 is required to suppress warnings originating in headers in external/ but you are using ${CMAKE_VERSION}, be prepared for some warnings"
    )
    set(system)
endif()

# Similarly, disable warnings for external projects
if(NOT SLANG_ENABLE_EXTERNAL_COMPILER_WARNINGS)
    if(NOT MSVC)
        add_compile_options(-w)
    endif()
endif()

# On MSVC with ASAN, /fsanitize=address embeds annotate_string /
# annotate_vector markers per object file. The linker rejects a mix of
# instrumented and non-instrumented objects with LNK2038, so every external
# subproject must be compiled with the same flag.
#
# BUILDSYSTEM_TARGETS only lists targets defined directly in a directory, not
# in subdirectories. slang_apply_asan_to_dir() recurses into SUBDIRECTORIES
# so every library target (e.g. SPIRV-Tools.lib under spirv-tools/source/) is
# covered. Call it after each add_subdirectory() that needs ASAN propagation.
if(SLANG_ENABLE_ASAN AND MSVC)
    function(slang_get_all_targets result_var dir)
        get_directory_property(_targets DIRECTORY "${dir}" BUILDSYSTEM_TARGETS)
        get_directory_property(_subdirs DIRECTORY "${dir}" SUBDIRECTORIES)
        set(_all ${_targets})
        foreach(_subdir ${_subdirs})
            slang_get_all_targets(_sub "${_subdir}")
            list(APPEND _all ${_sub})
        endforeach()
        set(${result_var} ${_all} PARENT_SCOPE)
    endfunction()

    function(slang_apply_asan_to_dir dir)
        slang_get_all_targets(_targets "${dir}")
        foreach(_t ${_targets})
            get_target_property(_type ${_t} TYPE)
            if(
                _type STREQUAL "STATIC_LIBRARY"
                OR _type STREQUAL "SHARED_LIBRARY"
                OR _type STREQUAL "MODULE_LIBRARY"
                OR _type STREQUAL "OBJECT_LIBRARY"
                OR _type STREQUAL "EXECUTABLE"
            )
                # /wd5072 suppresses "ASAN enabled without debug information
                # emission" — external Release builds have no /Zi, and their
                # /WX would turn C5072 into a fatal error otherwise.
                target_compile_options(${_t} PRIVATE /fsanitize=address /wd5072)
            endif()
        endforeach()
    endfunction()
endif()

# Unordered dense
if(NOT ${SLANG_USE_SYSTEM_UNORDERED_DENSE})
    if(NOT SLANG_OVERRIDE_UNORDERED_DENSE_PATH)
        add_subdirectory(unordered_dense EXCLUDE_FROM_ALL ${system})
    else()
        add_subdirectory(
            ${SLANG_OVERRIDE_UNORDERED_DENSE_PATH}/unordered_dense
            unordered_dense
            EXCLUDE_FROM_ALL
            ${system}
        )
    endif()
endif()

# Hash map and hash function backing Slang::Dictionary and Slang::Hash.
#
# Both are chosen at build time (SLANG_HASHMAP, SLANG_HASH) so that they can be
# compared against each other on real compiler workloads, and they are
# independent of each other, so any of the 12x7 combinations is valid. The
# `slang-hashmap` interface target below is the single place that gathers
# whatever the selected pair needs -- headers, link libraries and the two
# preprocessor definitions that source/core/slang-hashmap-impl.h and
# source/core/slang-hash-impl.h switch on -- and `core` links it publicly, so
# that every translation unit sees the same Dictionary layout.
add_library(slang-hashmap INTERFACE)

# Builds the vendored library in external/<dir> and links the CMake target it
# defines into slang-hashmap. Most of these libraries are only checked out when
# the configuration actually asks for them, so report a missing submodule with
# the command that fixes it rather than letting add_subdirectory fail.
function(slang_hashmap_use_submodule dir target)
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/${dir}/CMakeLists.txt")
        message(
            FATAL_ERROR
            "external/${dir} is missing. Run: git submodule update --init external/${dir}"
        )
    endif()
    add_subdirectory(${dir} EXCLUDE_FROM_ALL ${system})
    target_link_libraries(slang-hashmap INTERFACE ${target})
endfunction()

# Puts the vendored single-header library in external/<dir> on slang-hashmap's
# include path.
#
# This is separate from slang_hashmap_use_submodule because the three hash
# functions ship a bare header and no CMake package to consume: rapidhash and
# komihash have no CMakeLists at all, and xxHash's (under cmake_unofficial/)
# builds a library we do not want, since XXH_INLINE_ALL below makes it
# header-only.
function(slang_hashmap_use_header_only dir header)
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/${dir}/${header}")
        message(
            FATAL_ERROR
            "external/${dir}/${header} is missing. Run: git submodule update --init external/${dir}"
        )
    endif()
    target_include_directories(
        slang-hashmap
        ${system}
        INTERFACE "${CMAKE_CURRENT_LIST_DIR}/${dir}"
    )
endfunction()

# unordered_dense is a dependency of every configuration, not just
# SLANG_HASHMAP=UNORDERED_DENSE: slang-hash-impl.h uses its `is_avalanching`
# marker as the (implementation-independent) definition of which types get a
# library-provided hash rather than a Slang-defined getHashCode().
target_link_libraries(slang-hashmap INTERFACE unordered_dense::unordered_dense)

if(SLANG_HASHMAP MATCHES "^BOOST_" OR SLANG_HASH STREQUAL "BOOST")
    # Boost.Unordered and Boost.ContainerHash are header-only, so we add the
    # modular boostorg repositories' include directories directly instead of
    # using their CMakeLists, which are written to be driven by the Boost
    # superproject and would pull in its whole build machinery.
    add_library(slang-boost-headers INTERFACE)
    foreach(
        boost_module
        IN
        ITEMS
            assert
            config
            container_hash
            core
            describe
            mp11
            predef
            throw_exception
            unordered
    )
        set(boost_module_include
            "${CMAKE_CURRENT_LIST_DIR}/boost/${boost_module}/include"
        )
        if(NOT EXISTS "${boost_module_include}")
            message(
                FATAL_ERROR
                "${boost_module_include} is missing. Run: git submodule update --init external/boost/${boost_module}"
            )
        endif()
        target_include_directories(
            slang-boost-headers
            ${system}
            INTERFACE "${boost_module_include}"
        )
    endforeach()
    target_link_libraries(slang-hashmap INTERFACE slang-boost-headers)
endif()

if(SLANG_HASHMAP MATCHES "^ABSL_" OR SLANG_HASH STREQUAL "ABSL")
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/abseil-cpp/CMakeLists.txt")
        message(
            FATAL_ERROR
            "external/abseil-cpp is missing. Run: git submodule update --init external/abseil-cpp"
        )
    endif()
    set(ABSL_PROPAGATE_CXX_STD ON)
    set(ABSL_ENABLE_INSTALL OFF)
    set(ABSL_BUILD_TESTING OFF)
    # `core` is a static library that ends up inside the shared slang-compiler
    # library, so anything it links has to be position independent too.
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_subdirectory(abseil-cpp EXCLUDE_FROM_ALL ${system})
endif()

if(SLANG_HASHMAP STREQUAL "ABSL_FLAT")
    target_link_libraries(slang-hashmap INTERFACE absl::flat_hash_map)
elseif(SLANG_HASHMAP STREQUAL "ABSL_NODE")
    target_link_libraries(slang-hashmap INTERFACE absl::node_hash_map)
elseif(SLANG_HASHMAP STREQUAL "GTL_FLAT")
    slang_hashmap_use_submodule(gtl gtl)
elseif(SLANG_HASHMAP STREQUAL "TSL_ROBIN")
    slang_hashmap_use_submodule(robin-map tsl::robin_map)
elseif(SLANG_HASHMAP STREQUAL "TSL_HOPSCOTCH")
    slang_hashmap_use_submodule(hopscotch-map tsl::hopscotch_map)
elseif(SLANG_HASHMAP STREQUAL "TSL_SPARSE")
    slang_hashmap_use_submodule(sparse-map tsl::sparse_map)
endif()

# UNORDERED_DENSE_SEGMENTED needs nothing here: segmented_map lives in the same
# header as the default map, which is already linked above.

if(SLANG_HASH STREQUAL "ABSL")
    target_link_libraries(slang-hashmap INTERFACE absl::hash)
elseif(SLANG_HASH STREQUAL "RAPIDHASH")
    slang_hashmap_use_header_only(rapidhash rapidhash.h)
elseif(SLANG_HASH STREQUAL "KOMIHASH")
    slang_hashmap_use_header_only(komihash komihash.h)
elseif(SLANG_HASH STREQUAL "XXH3")
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/xxHash/xxhash.h")
        message(
            FATAL_ERROR
            "external/xxHash/xxhash.h is missing. Run: git submodule update --init external/xxHash"
        )
    endif()

    # xxHash cannot simply be put on the include path under its own name.
    #
    # lz4 vendors its own copy of xxhash.h, from a version predating XXH3, and
    # exports external/lz4/lib as a usage requirement. `core` links lz4, so
    # that directory lands on its -isystem path ahead of ours and
    # `#include <xxhash.h>` quietly resolves to the copy without XXH3 in it.
    # Adding ours BEFORE would not help: the two directories are usage
    # requirements of two different targets, so their relative order comes from
    # link order rather than from either target's own property.
    #
    # So give xxHash a name nothing else can claim: copy the header next to a
    # wrapper, and include the wrapper.
    #
    # The wrapper reaches the copy as a plain sibling, with no absolute path and
    # no "..", because slang-embed also walks these include directories. It
    # inlines the #includes it finds while embedding the C++ preludes, which
    # reach core/slang-hash.h and therefore this header, and it resolves a
    # quoted include by gluing it onto the parent directory of the file it
    # appears in. Handed an absolute path it builds a nonsensical one, fails to
    # read it, and aborts on the SLANG_ASSERT in tools/slang-embed.
    #
    # xxHash is compiled once, into its own translation unit, rather than pulled
    # into every translation unit with XXH_INLINE_ALL. Inlining it looked
    # attractive -- Slang only calls XXH3_64bits, on keys of a handful of bytes,
    # where call overhead is a real fraction of the cost -- but XXH_INLINE_ALL
    # makes every xxHash function `static`, and
    # Slang::HashImpl::hashBytes is an `inline` function with external linkage.
    # An inline function whose body names an internal-linkage entity is
    # ill-formed, no diagnostic required: each translation unit defines the same
    # COMDAT symbol with a different callee, and the linker keeps one of them
    # arbitrarily.
    #
    # That is not theoretical. At -Og the copies behaved alike and it went
    # unnoticed; at -O3, with xxHash's always_inline SIMD kernels folded in, the
    # surviving copy disagreed with the one that had been used to insert, and
    # slang-spirv-embed-generator aborted with "The key does not exist in
    # dictionary" looking up an SpvOp it had just added
    # (tools/slang-spirv-embed-generator/spirv-embed-generator-main.cpp).
    # Defining XXH_NO_INLINE_HINTS made it go away, which is a workaround for a
    # miscompilation and not a fix for the linkage.
    #
    # Neither generated file is rewritten when its content is unchanged --
    # configure_file and file(GENERATE) both compare first -- so reconfiguring
    # does not rebuild all of core.
    set(xxhash_shim_dir "${CMAKE_CURRENT_BINARY_DIR}/slang-xxhash-shim")
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/xxHash/xxhash.h"
        "${xxhash_shim_dir}/slang-xxhash-impl.h"
        COPYONLY
    )
    # Declarations only: without XXH_IMPLEMENTATION or XXH_INLINE_ALL, xxhash.h
    # declares the API with external linkage and defines nothing.
    file(
        GENERATE
        OUTPUT "${xxhash_shim_dir}/slang-xxhash.h"
        CONTENT "#pragma once\n#include \"slang-xxhash-impl.h\"\n"
    )
    # The one translation unit that holds the implementation.
    #
    # XXH_STATIC_LINKING_ONLY as well as XXH_IMPLEMENTATION: the implementation
    # needs the state structs, and xxhash.h only defines them behind that macro.
    # Without it the build fails on "invalid application of sizeof to incomplete
    # type XXH32_state_t".
    file(
        GENERATE
        OUTPUT "${xxhash_shim_dir}/slang-xxhash-impl.c"
        CONTENT
            "#define XXH_STATIC_LINKING_ONLY\n#define XXH_IMPLEMENTATION\n#include \"slang-xxhash-impl.h\"\n"
    )

    add_library(slang-xxhash STATIC "${xxhash_shim_dir}/slang-xxhash-impl.c")
    target_include_directories(slang-xxhash PUBLIC "${xxhash_shim_dir}")
    # `core` is a static library that ends up inside the shared slang-compiler
    # library, so anything it links has to be position independent too.
    set_target_properties(slang-xxhash PROPERTIES POSITION_INDEPENDENT_CODE ON)

    # XXH3's SSE2 kernels are marked always_inline, and GCC refuses to inline a
    # function carrying __target__("sse2") into a caller without it unless it is
    # optimizing properly -- "inlining failed in call to always_inline
    # XXH3_accumulate_sse2", which is an error, not a warning. xxHash guards
    # against this itself, but only by looking for __NO_INLINE__, which GCC
    # defines at -O0 and not at -Og, and Slang's Debug configuration is -Og (see
    # the top of the root CMakeLists.txt).
    #
    # PRIVATE, and only for the configuration that needs it: this is now the
    # sole translation unit compiling xxHash, so the hint cannot differ between
    # translation units the way it did when every one of them compiled its own
    # copy.
    target_compile_definitions(
        slang-xxhash
        PRIVATE $<$<CONFIG:Debug>:XXH_NO_INLINE_HINTS=1>
    )

    target_link_libraries(slang-hashmap INTERFACE slang-xxhash)
    target_include_directories(
        slang-hashmap
        ${system}
        INTERFACE "${xxhash_shim_dir}"
    )
endif()

# enum_option has already checked that these name one of the implementations
# enumerated in slang-hashmap-impl.h / slang-hash-impl.h.
target_compile_definitions(
    slang-hashmap
    INTERFACE
        SLANG_HASHMAP_IMPL=SLANG_HASHMAP_${SLANG_HASHMAP}
        SLANG_HASH_IMPL=SLANG_HASH_${SLANG_HASH}
)

message(
    STATUS
    "Slang::Dictionary will use SLANG_HASHMAP=${SLANG_HASHMAP} with SLANG_HASH=${SLANG_HASH}"
)

# Miniz
if(NOT ${SLANG_USE_SYSTEM_MINIZ})
    if(NOT SLANG_OVERRIDE_MINIZ_PATH)
        add_subdirectory(miniz EXCLUDE_FROM_ALL ${system})
    else()
        add_subdirectory(
            ${SLANG_OVERRIDE_MINIZ_PATH}/miniz
            miniz
            EXCLUDE_FROM_ALL
            ${system}
        )
    endif()
    set_property(TARGET miniz PROPERTY POSITION_INDEPENDENT_CODE ON)
    # Work around https://github.com/richgel999/miniz/pull/292
    get_target_property(miniz_c_launcher miniz C_COMPILER_LAUNCHER)
    if(MSVC AND miniz_c_launcher MATCHES "ccache")
        set_property(TARGET miniz PROPERTY C_COMPILER_LAUNCHER)
        set_property(TARGET miniz PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "")
    endif()
endif()

# LZ4
if(NOT ${SLANG_USE_SYSTEM_LZ4})
    set(LZ4_BUNDLED_MODE ON)
    if(NOT SLANG_OVERRIDE_LZ4_PATH)
        add_subdirectory(lz4/build/cmake EXCLUDE_FROM_ALL ${system})
    else()
        add_subdirectory(
            ${SLANG_OVERRIDE_LZ4_PATH}/lz4/build/cmake
            lz4/build/cmake
            EXCLUDE_FROM_ALL
            ${system}
        )
    endif()
    if(MSVC)
        target_compile_options(
            lz4_static
            PRIVATE /wd5045 /wd4820 /wd4711 /wd6385 /wd6262
        )
    endif()
endif()

# cmark-gfm (CommonMark / GFM Markdown parser, swiftlang fork)
set(CMARK_LIB_FUZZER OFF)
set(CMARK_FUZZ_QUADRATIC OFF)
set(_prev_BUILD_TESTING ${BUILD_TESTING})
set(_prev_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(BUILD_TESTING OFF)
set(BUILD_SHARED_LIBS OFF)
if(NOT SLANG_OVERRIDE_CMARK_PATH)
    add_subdirectory(cmark EXCLUDE_FROM_ALL ${system})
else()
    add_subdirectory(
        ${SLANG_OVERRIDE_CMARK_PATH}/cmark
        cmark
        EXCLUDE_FROM_ALL
        ${system}
    )
endif()
set(BUILD_TESTING ${_prev_BUILD_TESTING})
set(BUILD_SHARED_LIBS ${_prev_BUILD_SHARED_LIBS})

# Vulkan headers
if(NOT TARGET Vulkan::Headers)
    if(NOT ${SLANG_USE_SYSTEM_VULKAN_HEADERS})
        if(NOT SLANG_OVERRIDE_VULKAN_HEADERS_PATH)
            add_subdirectory(vulkan EXCLUDE_FROM_ALL ${system})
        else()
            add_subdirectory(
                ${SLANG_OVERRIDE_VULKAN_HEADERS_PATH}/vulkan
                vulkan
                EXCLUDE_FROM_ALL
                ${system}
            )
        endif()
    endif()
endif()

# Reuse Slang's Vulkan-Headers checkout when slang-rhi requests the same dependency through
# FetchContent. Callers can still provide their own FetchContent source directory explicitly.
if(
    NOT DEFINED FETCHCONTENT_SOURCE_DIR_VULKAN_HEADERS
    AND NOT SLANG_USE_SYSTEM_VULKAN_HEADERS
)
    if(NOT SLANG_OVERRIDE_VULKAN_HEADERS_PATH)
        set(FETCHCONTENT_SOURCE_DIR_VULKAN_HEADERS
            "${CMAKE_CURRENT_SOURCE_DIR}/vulkan"
        )
    else()
        set(FETCHCONTENT_SOURCE_DIR_VULKAN_HEADERS
            "${SLANG_OVERRIDE_VULKAN_HEADERS_PATH}/vulkan"
        )
    endif()
endif()

# metal-cpp headers
add_library(metal-cpp INTERFACE)
target_include_directories(
    metal-cpp
    INTERFACE "${CMAKE_CURRENT_LIST_DIR}/metal-cpp"
)

# fast_float headers (header-only). Mark as a system include so the bundled
# (or override-supplied) headers don't trip Slang's extra warnings / -Werror.
add_library(fast_float INTERFACE)
if(NOT SLANG_OVERRIDE_FAST_FLOAT_PATH)
    set(_fast_float_include "${CMAKE_CURRENT_LIST_DIR}/fast_float/include")
else()
    set(_fast_float_include
        "${SLANG_OVERRIDE_FAST_FLOAT_PATH}/fast_float/include"
    )
    # The add_subdirectory-based overrides fail at configure time on a bad path
    # because CMake errors on a missing source directory. A header-only override
    # is just an include-dir string, so validate the user-supplied path here to
    # fail loudly at configure time rather than as an opaque "header not found"
    # compile error later.
    if(NOT EXISTS "${_fast_float_include}")
        message(
            FATAL_ERROR
            "SLANG_OVERRIDE_FAST_FLOAT_PATH is set to '${SLANG_OVERRIDE_FAST_FLOAT_PATH}' but '${_fast_float_include}' does not exist"
        )
    endif()
endif()
target_include_directories(
    fast_float
    ${system}
    INTERFACE "${_fast_float_include}"
)

# Slang and SPIRV-Tools share one mimalloc target. Resolve the checkout before either consumer is
# configured so slang-compiler can use mimalloc even when SPIRV-Tools is disabled or provided by
# the system.
set(SLANG_BUILD_MIMALLOC OFF)
if(SLANG_ENABLE_MIMALLOC)
    set(SLANG_BUILD_MIMALLOC ON)
elseif(
    SLANG_ENABLE_SLANG_GLSLANG
    AND NOT SLANG_USE_SYSTEM_SPIRV_TOOLS
    AND SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC
)
    set(SLANG_BUILD_MIMALLOC ON)
endif()

set(SLANG_MIMALLOC_AVAILABLE OFF)
if(SLANG_BUILD_MIMALLOC)
    if(NOT SLANG_OVERRIDE_MIMALLOC_PATH)
        # Place mimalloc in slang/external instead of spirv-tools/external for better organization.
        set(MIMALLOC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mimalloc")
    else()
        set(MIMALLOC_PATH "${SLANG_OVERRIDE_MIMALLOC_PATH}/mimalloc")
    endif()

    # mimalloc is vendored as a submodule under external/mimalloc. A missing
    # checkout means submodules were not initialized, so fail loudly at configure
    # time, mirroring the fast_float override validation earlier in this file.
    if(NOT EXISTS "${MIMALLOC_PATH}/CMakeLists.txt")
        message(
            FATAL_ERROR
            "mimalloc source not found at ${MIMALLOC_PATH}. Run 'git submodule update --init --recursive', or set SLANG_OVERRIDE_MIMALLOC_PATH to a directory containing a 'mimalloc' checkout."
        )
    endif()

    set(mimalloc_SOURCE_DIR "${MIMALLOC_PATH}")
    set(SLANG_MIMALLOC_AVAILABLE ON)
endif()

# SPIRV-Headers
# Avoid using ${system} intentionally in `add_subdirectory`, because if we do, it will end up using
# spirv.h installed in the system directory on MacOS.
if(NOT TARGET SPIRV-Headers)
    if(SLANG_USE_SYSTEM_SPIRV_HEADERS)
        if(SLANG_OVERRIDE_SPIRV_HEADERS_PATH)
            message(
                WARNING
                "SLANG_OVERRIDE_SPIRV_HEADERS_PATH does nothing when SLANG_USE_SPIRV_HEADERS is set"
            )
        endif()
    elseif(SLANG_OVERRIDE_SPIRV_HEADERS_PATH)
        add_subdirectory(
            ${SLANG_OVERRIDE_SPIRV_HEADERS_PATH}/spirv-headers
            spirv-headers
            EXCLUDE_FROM_ALL
        )
    else()
        add_subdirectory(spirv-headers EXCLUDE_FROM_ALL)
    endif()
endif()

if(SLANG_ENABLE_SLANG_GLSLANG)
    # When using spirv headers via find_package, SPIRV-Headers_SOURCE_DIR is not set
    # SPIRV-Tools requires that variable, as it uses it to detect if SPIRV-Headers is provided by the consumer.
    # Fake a source build by setting it here. Ideally SPIRV-Tools should not depend on _SOURCE_DIR
    if(${SLANG_USE_SYSTEM_SPIRV_HEADERS})
        get_target_property(
            SPIRV-Headers_SOURCE_DIR
            SPIRV-Headers::SPIRV-Headers
            INTERFACE_INCLUDE_DIRECTORIES
        )
        cmake_path(
            GET
            SPIRV-Headers_SOURCE_DIR
            PARENT_PATH SPIRV-Headers_SOURCE_DIR
        )
    endif()

    if(NOT ${SLANG_USE_SYSTEM_SPIRV_TOOLS})
        # SPIRV-Tools
        set(SPIRV_TOOLS_BUILD_STATIC ON)
        set(SPIRV_WERROR OFF)
        set(SPIRV_SKIP_TESTS ON)

        # Enable mimalloc for SPIRV-Tools to improve compilation performance
        # Based on SPIRV-Tools PR #6267: https://github.com/KhronosGroup/SPIRV-Tools/pull/6267
        if(SLANG_MIMALLOC_AVAILABLE)
            set(SPIRV_TOOLS_USE_MIMALLOC
                ${SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC}
                CACHE BOOL
                "Executables and shared libraries use mimalloc instead of the default allocator"
                FORCE
            )
            set(SPIRV_TOOLS_USE_MIMALLOC_IN_STATIC_BUILD
                ${SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC}
                CACHE BOOL
                "Static SPIRV-Tools libraries use mimalloc instead of the default allocator"
                FORCE
            )
            if(SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC)
                message(
                    STATUS
                    "Enabling mimalloc for SPIRV-Tools (including static builds) from ${MIMALLOC_PATH}"
                )
            endif()
        endif()

        # Tools
        if(NOT SLANG_OVERRIDE_SPIRV_TOOLS_PATH)
            add_subdirectory(spirv-tools EXCLUDE_FROM_ALL)
        else()
            add_subdirectory(
                ${SLANG_OVERRIDE_SPIRV_TOOLS_PATH}/spirv-tools
                spirv-tools
                EXCLUDE_FROM_ALL
            )
        endif()
    endif()

    if(NOT ${SLANG_USE_SYSTEM_GLSLANG})
        # glslang
        set(SKIP_GLSLANG_INSTALL ON)
        set(ENABLE_OPT ON)
        set(ENABLE_PCH OFF)
        set(ALLOW_EXTERNAL_SPIRV_TOOLS ${SLANG_USE_SYSTEM_SPIRV_TOOLS})
        if(NOT SLANG_OVERRIDE_GLSLANG_PATH)
            add_subdirectory(glslang EXCLUDE_FROM_ALL ${system})
        else()
            add_subdirectory(
                ${SLANG_OVERRIDE_GLSLANG_PATH}/glslang
                glslang
                EXCLUDE_FROM_ALL
                ${system}
            )
        endif()
    endif()
endif()

# SPIRV-Tools normally creates mimalloc-static. Create it here when slang-compiler needs it and
# SPIRV-Tools did not, while keeping the global allocation override in Slang-owned source code.
if(SLANG_ENABLE_MIMALLOC AND NOT TARGET mimalloc-static)
    function(_slang_add_mimalloc_target)
        set(MI_OVERRIDE OFF)
        set(MI_BUILD_SHARED OFF)
        set(MI_BUILD_STATIC ON)
        set(MI_BUILD_OBJECT OFF)
        set(MI_BUILD_TESTS OFF)
        set(MI_INSTALL_TOPLEVEL OFF)
        add_subdirectory("${mimalloc_SOURCE_DIR}" mimalloc EXCLUDE_FROM_ALL)
    endfunction()
    _slang_add_mimalloc_target()
endif()

# Mimalloc defaults to level 2 in Debug builds, which enables additional internal assertions.
# Reduce this to level 1 to retain basic heap diagnostics while improving Debug execution time.
if(SLANG_ENABLE_MIMALLOC AND TARGET mimalloc-static AND NOT MI_DEBUG_FULL)
    target_compile_definitions(
        mimalloc-static
        PRIVATE $<$<CONFIG:Debug>:MI_DEBUG=1>
    )
endif()

if(
    SLANG_ENABLE_GFX
    OR (SLANG_ENABLE_SLANG_RHI AND (SLANG_ENABLE_TESTS OR SLANG_ENABLE_EXAMPLES))
)
    # imgui
    add_library(imgui INTERFACE)
    if(NOT SLANG_OVERRIDE_IMGUI_PATH)
        target_include_directories(
            imgui
            ${system}
            INTERFACE "${CMAKE_CURRENT_LIST_DIR}/imgui"
        )
    else()
        target_include_directories(
            imgui
            ${system}
            INTERFACE "${SLANG_OVERRIDE_IMGUI_PATH}/imgui"
        )
    endif()
    add_supported_cxx_flags(imgui INTERFACE -Wno-nontrivial-memcall)

    # stb
    add_library(stb INTERFACE)
    target_include_directories(
        stb
        ${system}
        INTERFACE "${CMAKE_CURRENT_LIST_DIR}/stb"
    )
endif()

# slang-rhi
if(SLANG_ENABLE_SLANG_RHI)
    set(SLANG_RHI_BUILD_FROM_SLANG_REPO ON)
    set(SLANG_RHI_BUILD_TESTS ON)
    set(SLANG_RHI_BUILD_TESTS_WITH_GLFW OFF)
    set(SLANG_RHI_INSTALL OFF)
    set(SLANG_RHI_BINARY_DIR ${slang_BINARY_DIR}/$<CONFIG>/bin)
    set(SLANG_RHI_FETCH_SLANG OFF)
    set(SLANG_RHI_SLANG_INCLUDE_DIR ${slang_SOURCE_DIR}/include)
    set(SLANG_RHI_SLANG_BINARY_DIR ${slang_BINARY_DIR})
    set(SLANG_RHI_ENABLE_NVAPI ${SLANG_ENABLE_NVAPI})
    set(SLANG_RHI_ENABLE_WGPU ${SLANG_HAS_WEBGPU_SUPPORT})
    if(SLANG_ENABLE_DXIL AND NOT DEFINED SLANG_RHI_FETCH_DXC)
        # Slang's FetchDXC.cmake owns DXC download/build/deployment in this
        # configuration, so prevent slang-rhi from fetching its own copy.
        set(SLANG_RHI_FETCH_DXC OFF)
    endif()

    # CoopVec requires the preview Agility SDK that supports SM 6.10
    #set(SLANG_RHI_AGILITY_SDK_VERSION D3D12_PREVIEW_SDK_VERSION)
    #set(SLANG_RHI_AGILITY_SDK_URL
    #"https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.719.1-preview"
    #)

    if(SLANG_ENABLE_DX_ON_VK)
        set(SLANG_RHI_HAS_D3D12 ON)
    endif()
    if(MINGW)
        set(SLANG_RHI_ENABLE_D3D11 OFF)
        set(SLANG_RHI_ENABLE_D3D12 OFF)
    endif()
    if(NOT SLANG_OVERRIDE_SLANG_RHI_PATH)
        add_subdirectory(slang-rhi)
    else()
        add_subdirectory(
            ${SLANG_OVERRIDE_SLANG_RHI_PATH}/slang-rhi
            slang-rhi
            EXCLUDE_FROM_ALL
            ${system}
        )
    endif()

    if(SLANG_ENABLE_ASAN AND MSVC)
        slang_apply_asan_to_dir("${CMAKE_CURRENT_SOURCE_DIR}/slang-rhi")
    endif()

    # Output slang-rhi-tests to the bin directory so we can run it easily.
    set_target_properties(
        slang-rhi-tests
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${slang_BINARY_DIR}/$<CONFIG>/bin
    )
endif()

# Tidy things up:

# Restore log level if we set it
if(set_cmake_log_level)
    unset(CMAKE_MESSAGE_LOG_LEVEL)
endif()

# for this directory and all subdirectories, prepend
# `external/` to the IDE FOLDER property to every target
function(make_external dir)
    get_property(external_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(external_target ${external_targets})
        get_property(folder TARGET ${external_target} PROPERTY FOLDER)
        set_property(
            TARGET ${external_target}
            PROPERTY FOLDER "external/${folder}"
        )
    endforeach()

    get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirs})
        make_external(${subdir})
    endforeach()
endfunction()
make_external(.)
