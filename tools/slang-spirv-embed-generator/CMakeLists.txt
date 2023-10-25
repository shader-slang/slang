add_executable(slang-spirv-embed-generator EXCLUDE_FROM_ALL
               spirv-embed-generator-main.cpp)
set_default_compile_options(slang-spirv-embed-generator)
target_link_libraries(slang-spirv-embed-generator PRIVATE core compiler-core)
