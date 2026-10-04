cmake_minimum_required(VERSION 3.20)
project(cpu-ray-query LANGUAGES CXX)

find_program(SLANGC_EXECUTABLE NAMES slangc REQUIRED)

# Generate ordinary C++ and link it directly into the host. No RHI or runtime library is needed.
set(shader_cpp ${CMAKE_CURRENT_BINARY_DIR}/shader.cpp)
add_custom_command(
    OUTPUT ${shader_cpp}
    COMMAND
        ${SLANGC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/shader.slang -target
        cpp -entry computeMain -o ${shader_cpp}
    DEPENDS ${SLANGC_EXECUTABLE} shader.slang
    VERBATIM
)
add_executable(cpu-ray-query main.cpp ${shader_cpp})
target_compile_features(cpu-ray-query PRIVATE cxx_std_17)
target_include_directories(
    cpu-ray-query
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../prelude
)

enable_testing()
add_test(NAME cpu-ray-query-standalone COMMAND cpu-ray-query --test)
