if(NOT APPLE)
    message(
        FATAL_ERROR
        "metal-structural-raytracing-test is available only on macOS"
    )
endif()

enable_language(OBJCXX)

add_executable(
    metal-structural-raytracing-test
    EXCLUDE_FROM_ALL
    main.mm
    metal-test-host.mm
    metal-test-scenes.mm
)

target_compile_features(metal-structural-raytracing-test PRIVATE cxx_std_17)
target_compile_options(metal-structural-raytracing-test PRIVATE -fobjc-arc)
target_link_libraries(
    metal-structural-raytracing-test
    PRIVATE slang "-framework Foundation" "-framework Metal"
)

set_target_properties(
    metal-structural-raytracing-test
    PROPERTIES FOLDER test/tools
)
