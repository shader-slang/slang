cmake_minimum_required(VERSION 3.20)

project(metal-structural-raytracing-test LANGUAGES CXX OBJCXX)

if(NOT APPLE)
    message(FATAL_ERROR "metal-structural-raytracing-test is available only on macOS")
endif()

add_executable(
    metal-structural-raytracing-test
    main.mm
    metal-test-host.mm
    metal-test-scenes.mm
)

target_compile_features(metal-structural-raytracing-test PRIVATE cxx_std_17)
target_compile_options(metal-structural-raytracing-test PRIVATE -fobjc-arc)
target_link_libraries(
    metal-structural-raytracing-test
    PRIVATE
        "-framework Foundation"
        "-framework Metal"
)
