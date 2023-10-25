include(ExternalProject)

# Miniz
set(MINIZ_STANDALONE_PROJECT OFF)
add_subdirectory(miniz EXCLUDE_FROM_ALL)

# LZ4
set(LZ4_BUNDLED_MODE ON)
add_subdirectory(lz4/build/cmake EXCLUDE_FROM_ALL)

# SPIR-V
add_subdirectory(spirv-headers EXCLUDE_FROM_ALL)

# Vulkan
add_subdirectory(vulkan EXCLUDE_FROM_ALL)
