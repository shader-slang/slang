add_executable(test-server EXCLUDE_FROM_ALL test-server-diagnostics.cpp
                                            test-server-main.cpp)
set_default_compile_options(test-server)
target_link_libraries(test-server core compiler-core slang)
