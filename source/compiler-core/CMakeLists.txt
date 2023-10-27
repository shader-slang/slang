glob_append(SLANG_COMPILER_CORE_SOURCE "*.cpp")

add_library(compiler-core STATIC EXCLUDE_FROM_ALL ${SLANG_COMPILER_CORE_SOURCE})
set_default_compile_options(compiler-core USE_EXTRA_WARNINGS)
target_link_libraries(compiler-core PRIVATE core)
