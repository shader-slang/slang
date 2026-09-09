cmake_minimum_required(VERSION 3.20)
project(spvdb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for clang-tidy and other tooling.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect whether we are the top-level project or included via add_subdirectory.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(SPVDB_IS_TOPLEVEL TRUE)
else()
    set(SPVDB_IS_TOPLEVEL FALSE)
endif()

# ---- Third-party dependencies -----------------------------------------------
#
# spvdb is vendored inside the Slang repository at tests/spvdb, two levels
# below the repository root. All of spvdb's own third-party dependencies
# (SPIRV-Headers, stb, nlohmann_json, Catch2, replxx) live as submodules
# under <slang root>/external instead of being duplicated in a nested
# third_party/ directory: SPIRV-Headers and stb are dependencies Slang
# already vendors there; nlohmann_json, Catch2, and replxx are specific to
# spvdb and were added as new Slang submodules (external/nlohmann-json,
# external/catch2, external/replxx) as part of this vendoring.
#
# spvdb's CMakeLists.txt can still be configured on its own (e.g. from
# spvdb's upstream repository, or by pointing cmake directly at this
# directory) — in that case <slang root>/external won't resolve to anything
# meaningful, and every dependency below falls back to FetchContent.

include(FetchContent)

# tests/spvdb/CMakeLists.txt -> tests/spvdb -> tests -> <slang root>/external
get_filename_component(
    SPVDB_SLANG_EXTERNAL_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../external"
    ABSOLUTE
)
if(NOT IS_DIRECTORY "${SPVDB_SLANG_EXTERNAL_DIR}")
    set(SPVDB_SLANG_EXTERNAL_DIR "")
endif()

# SPIRV-Headers (header-only; provides spirv/unified1/spirv.h)
if(NOT TARGET spirv-headers)
    add_library(spirv-headers INTERFACE)
    if(SPVDB_SLANG_EXTERNAL_DIR AND EXISTS "${SPVDB_SLANG_EXTERNAL_DIR}/spirv-headers/include")
        target_include_directories(spirv-headers INTERFACE
            "${SPVDB_SLANG_EXTERNAL_DIR}/spirv-headers/include")
    else()
        FetchContent_Declare(spirv_headers_src
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
            GIT_TAG        main)
        FetchContent_Populate(spirv_headers_src)
        target_include_directories(spirv-headers INTERFACE
            ${spirv_headers_src_SOURCE_DIR}/include)
    endif()
endif()

# stb (single-file headers; image loading via stb_image.h)
# Guard: the parent project (e.g. slang) may already define a 'stb' target.
if(NOT TARGET stb)
    add_library(stb INTERFACE)
    if(SPVDB_SLANG_EXTERNAL_DIR AND EXISTS "${SPVDB_SLANG_EXTERNAL_DIR}/stb")
        target_include_directories(stb INTERFACE "${SPVDB_SLANG_EXTERNAL_DIR}/stb")
    else()
        FetchContent_Declare(stb_src
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG        master)
        FetchContent_Populate(stb_src)
        target_include_directories(stb INTERFACE ${stb_src_SOURCE_DIR})
    endif()
endif()

# nlohmann/json (header-only) — reuse Slang's submodule if present, else fetch.
if(NOT TARGET nlohmann_json)
    if(SPVDB_SLANG_EXTERNAL_DIR AND
       EXISTS "${SPVDB_SLANG_EXTERNAL_DIR}/nlohmann-json/include/nlohmann/json.hpp")
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json INTERFACE
            "${SPVDB_SLANG_EXTERNAL_DIR}/nlohmann-json/include")
    else()
        FetchContent_Declare(nlohmann_json_src
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG        d96329f8d6dee5da67c5ebd861acab25445d0ddb)
        FetchContent_Populate(nlohmann_json_src)
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json INTERFACE
            ${nlohmann_json_src_SOURCE_DIR}/include)
    endif()
endif()

# Catch2, replxx, CLI, tools, and tests are only needed for the standalone build.
if(SPVDB_IS_TOPLEVEL)
    # Catch2 (for tests) — reuse Slang's submodule if present, else fetch.
    set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
    set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
    if(SPVDB_SLANG_EXTERNAL_DIR AND EXISTS "${SPVDB_SLANG_EXTERNAL_DIR}/catch2/CMakeLists.txt")
        add_subdirectory("${SPVDB_SLANG_EXTERNAL_DIR}/catch2" "${CMAKE_CURRENT_BINARY_DIR}/catch2" EXCLUDE_FROM_ALL)
    else()
        FetchContent_Declare(Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        69e0473f6e98d47c93518424c08ee69ee632c0f0)
        FetchContent_MakeAvailable(Catch2)
    endif()

    # replxx (for CLI) — reuse Slang's submodule if present, else fetch.
    set(REPLXX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(REPLXX_BUILD_PACKAGE  OFF CACHE BOOL "" FORCE)
    if(SPVDB_SLANG_EXTERNAL_DIR AND EXISTS "${SPVDB_SLANG_EXTERNAL_DIR}/replxx/CMakeLists.txt")
        add_subdirectory("${SPVDB_SLANG_EXTERNAL_DIR}/replxx" "${CMAKE_CURRENT_BINARY_DIR}/replxx" EXCLUDE_FROM_ALL)
    else()
        FetchContent_Declare(replxx
            GIT_REPOSITORY https://github.com/AmokHuginnsson/replxx.git
            GIT_TAG        1f149bfe20bf6e49c1afd4154eaf0032c8c2fda2)
        FetchContent_MakeAvailable(replxx)
    endif()
endif()

# ---- libspvdb ---------------------------------------------------------------

add_library(spvdb STATIC
    lib/parser/parser.cpp
    lib/module/module.cpp
    lib/interpreter/interpreter.cpp
    lib/interpreter/glsl_std_450.cpp
    lib/interpreter/image_sampler.cpp
    lib/api/spvdb.cpp
)

target_include_directories(spvdb PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(spvdb PUBLIC
    spirv-headers
    nlohmann_json
    stb
)

# When embedded in another project, skip the stb_image implementation to avoid
# duplicate symbol errors if the parent already compiles it.
if(NOT SPVDB_IS_TOPLEVEL)
    target_compile_definitions(spvdb PRIVATE SPVDB_SKIP_STB_IMAGE_IMPL=1)
endif()

target_compile_options(spvdb PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wno-unused-parameter>
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /wd4100>
)

# ---- spvdb CLI, tools, and tests (standalone build only) --------------------

if(SPVDB_IS_TOPLEVEL)
    add_subdirectory(cli)
    enable_testing()
    add_subdirectory(tests)

    # ---- Formatting (clang-format) ------------------------------------------
    find_program(CLANG_FORMAT_EXE NAMES clang-format)
    if(CLANG_FORMAT_EXE)
        file(GLOB_RECURSE SPVDB_FORMAT_SOURCES
            LIST_DIRECTORIES false
            "${CMAKE_CURRENT_SOURCE_DIR}/cli/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/cli/*.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
        )
        # 'format'       — reformat all sources in-place.
        # 'format-check' — exit non-zero if any source is not formatted.
        add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${SPVDB_FORMAT_SOURCES}
            COMMENT "clang-format: reformatting sources"
            VERBATIM
        )
        add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${SPVDB_FORMAT_SOURCES}
            COMMENT "clang-format: checking sources"
            VERBATIM
        )
    endif()

    # ---- Static analysis (clang-tidy) ---------------------------------------
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        add_custom_target(tidy
            COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_CURRENT_BINARY_DIR} ${SPVDB_FORMAT_SOURCES}
            COMMENT "clang-tidy: running static analysis"
            VERBATIM
        )
    endif()
endif()
