add_executable(test-process test-process-main.cpp)
set_default_compile_options(test-process)
target_link_libraries(test-process PRIVATE core compiler-core)
