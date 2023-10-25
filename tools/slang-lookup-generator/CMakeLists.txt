add_executable(slang-lookup-generator EXCLUDE_FROM_ALL
               ./lookup-generator-main.cpp)
set_default_compile_options(slang-lookup-generator)
target_link_libraries(slang-lookup-generator PRIVATE core compiler-core)
