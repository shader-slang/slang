add_library(slang-reflection-test EXCLUDE_FROM_ALL
            slang-reflection-test-main.cpp)
set_default_compile_options(slang-reflection-test)
target_link_libraries(slang-reflection-test PRIVATE core slang)
