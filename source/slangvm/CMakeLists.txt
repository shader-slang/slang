find_package(Python 3.9 COMPONENTS Interpreter Development.Module REQUIRED)
# Use an installed nanobind package, or an explicitly supplied source checkout. Neither requires
# building or importing SlangPy. No dependency is downloaded during configuration.
set(SLANG_NANOBIND_SOURCE_DIR "" CACHE PATH "Optional nanobind source checkout")
if(SLANG_NANOBIND_SOURCE_DIR)
    add_subdirectory("${SLANG_NANOBIND_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/nanobind")
else()
    find_package(nanobind CONFIG REQUIRED)
endif()

nanobind_add_module(_slangvm slangvm.cpp)
target_compile_features(_slangvm PRIVATE cxx_std_20)
target_link_libraries(_slangvm PRIVATE slang-hgvm slang-hgvm-compiler core)
set(_slangvm_package "${CMAKE_BINARY_DIR}/$<CONFIG>/python/slangvm")
set_target_properties(_slangvm PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${_slangvm_package}"
    RUNTIME_OUTPUT_DIRECTORY "${_slangvm_package}"
)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${slang_SOURCE_DIR}/python/slangvm/__init__.py")
file(GENERATE OUTPUT "${_slangvm_package}/__init__.py"
    INPUT "${slang_SOURCE_DIR}/python/slangvm/__init__.py")
if(WIN32)
    add_custom_command(TARGET _slangvm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:_slangvm> "${_slangvm_package}"
        COMMAND_EXPAND_LISTS
    )
endif()
if(SLANG_ENABLE_TESTS)
    add_test(NAME slangvm-python
        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_BINARY_DIR}/$<CONFIG>/python"
            "${Python_EXECUTABLE}" "${slang_SOURCE_DIR}/python/tests/test_slangvm.py"
    )
endif()
