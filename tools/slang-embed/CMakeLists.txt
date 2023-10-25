add_executable(slang-embed EXCLUDE_FROM_ALL slang-embed.cpp)
set_default_compile_options(slang-embed)
target_link_libraries(slang-embed PRIVATE core)
