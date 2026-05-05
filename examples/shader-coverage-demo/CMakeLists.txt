if(TARGET slang-coverage-rt)
    example(shader-coverage-demo)
    target_link_libraries(shader-coverage-demo PRIVATE slang-coverage-rt)
    target_include_directories(
        shader-coverage-demo
        PRIVATE ${slang_SOURCE_DIR}/source/slang-coverage-rt/include
    )
endif()
