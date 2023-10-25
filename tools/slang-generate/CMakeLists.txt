add_executable(slang-generate EXCLUDE_FROM_ALL main.cpp)
set_default_compile_options(slang-generate)
target_link_libraries(slang-generate PRIVATE core)
