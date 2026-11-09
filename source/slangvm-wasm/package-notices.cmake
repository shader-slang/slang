# Include each dependency's copyright-bearing notice, as in the main Slang
# package. Overrides and system libraries supply their own distribution notices.
function(slangvm_add_notice source name)
    list(
        APPEND slangvm_notice_copy_commands
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${source}" "${slangvm_package_dir}/third-party-notices/${name}"
    )
    set(slangvm_notice_copy_commands "${slangvm_notice_copy_commands}" PARENT_SCOPE)
    install(
        FILES "${source}"
        DESTINATION javascript/slangvm/third-party-notices
        RENAME "${name}"
        COMPONENT slangvm-javascript
        EXCLUDE_FROM_ALL
    )
endfunction()

if(NOT SLANG_OVERRIDE_CMARK_PATH)
    slangvm_add_notice("${slang_SOURCE_DIR}/external/cmark/COPYING" cmark-COPYING)
endif()
if(NOT SLANG_USE_SYSTEM_LZ4 AND NOT SLANG_OVERRIDE_LZ4_PATH)
    slangvm_add_notice("${slang_SOURCE_DIR}/external/lz4/lib/LICENSE" lz4-LICENSE)
endif()
if(NOT SLANG_USE_SYSTEM_MINIZ AND NOT SLANG_OVERRIDE_MINIZ_PATH)
    slangvm_add_notice("${slang_SOURCE_DIR}/external/miniz/LICENSE" miniz-LICENSE)
endif()
if(NOT SLANG_USE_SYSTEM_UNORDERED_DENSE AND NOT SLANG_OVERRIDE_UNORDERED_DENSE_PATH)
    slangvm_add_notice(
        "${slang_SOURCE_DIR}/external/unordered_dense/LICENSE" unordered_dense-LICENSE
    )
endif()
if(NOT SLANG_OVERRIDE_FAST_FLOAT_PATH)
    slangvm_add_notice(
        "${slang_SOURCE_DIR}/external/fast_float/LICENSE-MIT" fast_float-LICENSE-MIT
    )
endif()
if(NOT SLANG_USE_SYSTEM_SPIRV_HEADERS AND NOT SLANG_OVERRIDE_SPIRV_HEADERS_PATH)
    slangvm_add_notice(
        "${slang_SOURCE_DIR}/external/spirv-headers/LICENSE" spirv-headers-LICENSE
    )
endif()
if(NOT SLANG_OVERRIDE_SLANG_RHI_PATH)
    slangvm_add_notice("${slang_SOURCE_DIR}/external/slang-rhi/LICENSE" slang-rhi-LICENSE)
endif()
slangvm_add_notice("${slang_SOURCE_DIR}/external/tinyexr/LICENSE" tinyexr-LICENSE)

# STB keeps its notices in its headers rather than a separate license file.
foreach(header stb_image stb_image_write)
    set(source "${slang_SOURCE_DIR}/external/stb/${header}.h")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${source}")
    file(READ "${source}" contents)
    string(FIND "${contents}" "This software is available under 2 licenses" start REVERSE)
    if(start EQUAL -1)
        message(FATAL_ERROR "Cannot find the license notice in ${source}")
    endif()
    string(SUBSTRING "${contents}" ${start} -1 notice)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${header}-LICENSE" "${notice}")
    slangvm_add_notice("${CMAKE_CURRENT_BINARY_DIR}/${header}-LICENSE" "${header}-LICENSE")
endforeach()

# Emdawn is downloaded by Emscripten while building. Query that SDK's configured
# port cache, then copy its notices after linking has fetched the port. This also
# respects EM_PORTS/EM_CACHE and custom SDK configurations.
get_filename_component(slangvm_emscripten_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
find_program(
    slangvm_em_config
    NAMES em-config
    HINTS "${slangvm_emscripten_dir}"
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
    REQUIRED
)
execute_process(
    COMMAND "${slangvm_em_config}" PORTS
    OUTPUT_VARIABLE slangvm_ports_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
file(TO_CMAKE_PATH "${slangvm_ports_dir}" slangvm_ports_dir)
set(slangvm_emdawn_dir "${slangvm_ports_dir}/emdawnwebgpu/emdawnwebgpu_pkg")
slangvm_add_notice("${slangvm_emdawn_dir}/webgpu/src/LICENSE" emdawnwebgpu-LICENSE)
slangvm_add_notice("${slangvm_emdawn_dir}/webgpu_cpp/LICENSE" dawn-LICENSE)
