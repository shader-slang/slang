add_executable(slangd main.cpp)
set_default_compile_options(slangd)
target_link_libraries(slangd PRIVATE core compiler-core slang)
