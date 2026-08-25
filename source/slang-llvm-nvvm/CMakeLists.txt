cmake_minimum_required(VERSION 3.20)

project(slang-llvm-nvvm LANGUAGES C CXX)

get_filename_component(_default_slang_source_dir "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(
    SLANG_SOURCE_DIR
    "${_default_slang_source_dir}"
    CACHE PATH
    "Path to the Slang source tree"
)
set(
    SLANG_NVVM_LLVM_DIR
    ""
    CACHE PATH
    "Path to an isolated LLVM 14.0.6 CMake package"
)

if(NOT SLANG_NVVM_LLVM_DIR)
    message(FATAL_ERROR "SLANG_NVVM_LLVM_DIR must name the isolated LLVM 14.0.6 CMake package")
endif()

find_package(
    LLVM 14.0.6 EXACT REQUIRED CONFIG
    PATHS "${SLANG_NVVM_LLVM_DIR}"
    NO_DEFAULT_PATH
)

if(LLVM_LINK_LLVM_DYLIB)
    message(FATAL_ERROR "slang-llvm-nvvm requires LLVM 14 static component libraries")
endif()
if(LLVM_ENABLE_EH)
    message(FATAL_ERROR "slang-llvm-nvvm requires LLVM 14 built without C++ exceptions")
endif()

llvm_map_components_to_libnames(slang_nvvm_llvm_libraries core bitwriter support)
foreach(slang_nvvm_llvm_library IN LISTS slang_nvvm_llvm_libraries)
    if(NOT TARGET ${slang_nvvm_llvm_library})
        message(FATAL_ERROR "LLVM component target ${slang_nvvm_llvm_library} is unavailable")
    endif()
    get_target_property(slang_nvvm_llvm_library_type ${slang_nvvm_llvm_library} TYPE)
    if(NOT slang_nvvm_llvm_library_type STREQUAL "STATIC_LIBRARY")
        message(
            FATAL_ERROR
            "${slang_nvvm_llvm_library} is not a static LLVM component library"
        )
    endif()
endforeach()
separate_arguments(slang_nvvm_llvm_definitions NATIVE_COMMAND "${LLVM_DEFINITIONS}")

if(MSVC)
    set(slang_nvvm_c_api_check_options /W4 /WX)
else()
    set(slang_nvvm_c_api_check_options -Wall -Wextra -Werror)
endif()
set_source_files_properties(
    slang-nvvm-ir-builder-api-c.c
    PROPERTIES COMPILE_OPTIONS "${slang_nvvm_c_api_check_options}"
)

add_library(
    slang-llvm-nvvm
    MODULE
    slang-nvvm-ir-builder-api-c.c
    slang-llvm-nvvm.cpp
)

target_compile_features(slang-llvm-nvvm PRIVATE cxx_std_17)
target_compile_definitions(
    slang-llvm-nvvm
    PRIVATE
        SLANG_NVVM_BUILDER_EXPORTS
)
target_compile_options(slang-llvm-nvvm PRIVATE ${slang_nvvm_llvm_definitions})
target_include_directories(
    slang-llvm-nvvm
    SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS}
)
target_include_directories(
    slang-llvm-nvvm
    PRIVATE
        "${SLANG_SOURCE_DIR}/include"
        "${SLANG_SOURCE_DIR}/source"
)
target_link_libraries(slang-llvm-nvvm PRIVATE ${slang_nvvm_llvm_libraries})

set_target_properties(
    slang-llvm-nvvm
    PROPERTIES
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
)

if(NOT LLVM_ENABLE_RTTI)
    if(MSVC)
        target_compile_options(slang-llvm-nvvm PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/GR->")
    else()
        target_compile_options(slang-llvm-nvvm PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>")
    endif()
endif()

if(NOT MSVC)
    target_compile_options(slang-llvm-nvvm PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>")
endif()

if(MSVC)
    target_sources(slang-llvm-nvvm PRIVATE slang-llvm-nvvm.def)
    target_compile_definitions(slang-llvm-nvvm PRIVATE _HAS_EXCEPTIONS=0)
    target_compile_options(
        slang-llvm-nvvm
        PRIVATE
            -wd4244
            -wd4267
            -wd4530
            "$<$<COMPILE_LANGUAGE:CXX>:/EHs-c->"
            /Zc:preprocessor
    )
    set_property(
        TARGET slang-llvm-nvvm
        PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
elseif(APPLE)
    set_property(TARGET slang-llvm-nvvm PROPERTY SUFFIX ".dylib")
    target_link_options(
        slang-llvm-nvvm
        PRIVATE
            "-Wl,-exported_symbols_list,${CMAKE_CURRENT_LIST_DIR}/slang-llvm-nvvm.exports"
    )
    set_property(
        TARGET slang-llvm-nvvm
        APPEND
        PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/slang-llvm-nvvm.exports"
    )
else()
    target_link_options(
        slang-llvm-nvvm
        PRIVATE
            "-Wl,--exclude-libs,ALL"
            "-Wl,--version-script=${CMAKE_CURRENT_LIST_DIR}/slang-llvm-nvvm.version-script"
    )
    set_property(
        TARGET slang-llvm-nvvm
        APPEND
        PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/slang-llvm-nvvm.version-script"
    )
endif()
