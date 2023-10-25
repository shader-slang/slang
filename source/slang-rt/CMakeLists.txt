set(SLANG_RT_SOURCE ${core_source})
list(APPEND SLANG_RT_SOURCE slang-rt.cpp)
add_library(slang-rt ${SLANG_RT_SOURCE})
set_default_compile_options(slang-rt)
target_compile_definitions(slang-rt PRIVATE SLANG_RT_DYNAMIC
                                            SLANG_RT_DYNAMIC_EXPORT)
target_link_libraries(slang-rt PRIVATE miniz lz4 Threads::Threads)
