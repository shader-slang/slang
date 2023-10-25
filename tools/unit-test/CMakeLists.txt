add_library(unit-test STATIC EXCLUDE_FROM_ALL slang-unit-test.cpp)
set_default_compile_options(unit-test)
# For slang.h
target_include_directories(unit-test
                           PRIVATE $<TARGET_PROPERTY:slang,INCLUDE_DIRECTORIES>)
