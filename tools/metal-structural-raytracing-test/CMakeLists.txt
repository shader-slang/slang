if(NOT APPLE)
    message(
        FATAL_ERROR
        "metal-structural-raytracing-test is available only on macOS"
    )
endif()

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

# The test loads `slang.raytracing` at runtime, so building this target alone must also produce the
# separately shipped standard module. `slang` itself intentionally has no dependency on that
# module because ordinary compiler users should not pay its build or load cost.
add_dependencies(metal-structural-raytracing-test slang-raytracing-module)

set_target_properties(
    metal-structural-raytracing-test
    PROPERTIES FOLDER test/tools
)
