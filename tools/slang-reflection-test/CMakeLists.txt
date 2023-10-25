add_library(slang-reflection-test SHARED EXCLUDE_FROM_ALL
            slang-reflection-test-main.cpp)
set_default_compile_options(slang-reflection-test)
target_link_libraries(slang-reflection-test PRIVATE core slang)
target_compile_definitions(slang-reflection-test
                           PRIVATE SLANG_SHARED_LIBRARY_TOOL)
set_target_properties(slang-reflection-test
                      PROPERTIES OUTPUT_NAME slang-reflection-test-tool)
