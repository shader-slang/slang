# Miniz
set(MINIZ_STANDALONE_PROJECT OFF)
add_subdirectory(miniz EXCLUDE_FROM_ALL)

# LZ4
set(LZ4_BUNDLED_MODE ON)
add_subdirectory(lz4/build/cmake EXCLUDE_FROM_ALL)

# Vulkan headers
add_subdirectory(vulkan EXCLUDE_FROM_ALL)

# SPIRV-Headers
add_subdirectory(spirv-headers EXCLUDE_FROM_ALL)

# SPIRV-Tools
set(SPIRV_TOOLS_BUILD_STATIC ON)
set(SPIRV_WERROR OFF)
set(SPIRV_HEADER_DIR "${CMAKE_CURRENT_LIST_DIR}/spirv-headers/")
add_subdirectory(spirv-tools EXCLUDE_FROM_ALL)

# glslang
set(SKIP_GLSLANG_INSTALL ON)
set(ENABLE_OPT ON)
add_subdirectory(glslang EXCLUDE_FROM_ALL)
