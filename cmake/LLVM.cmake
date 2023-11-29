# A convenience on top of the llvm package's cmake files, this creates a target
# to pass to target_link_libraries which correctly pulls in the llvm include
# dir and other compile dependencies
function(llvm_target_from_components target_name)
    set(components ${ARGN})
    llvm_map_components_to_libnames(llvm_libs
        ${components}
    )
    add_library(${target_name} INTERFACE)
    target_link_libraries(${target_name} INTERFACE ${llvm_libs})
    target_include_directories(
        ${target_name}
        SYSTEM
        INTERFACE ${LLVM_INCLUDE_DIRS}
    )
    target_compile_definitions(${target_name} INTERFACE ${LLVM_DEFINITIONS})
    if(NOT LLVM_ENABLE_RTTI)
        # Make sure that we don't disable rtti if this library wasn't compiled with
        # support
        add_supported_cxx_flags(${target_name} INTERFACE -fno-rtti /GR-)
    endif()
endfunction()

# The same for clang
function(clang_target_from_libs target_name)
    set(clang_libs ${ARGN})
    add_library(${target_name} INTERFACE)
    target_link_libraries(${target_name} INTERFACE ${clang_libs})
    target_include_directories(
        ${target_name}
        SYSTEM
        INTERFACE ${CLANG_INCLUDE_DIRS}
    )
    target_compile_definitions(${target_name} INTERFACE ${CLANG_DEFINITIONS})
    if(NOT LLVM_ENABLE_RTTI)
        # Make sure that we don't disable rtti if this library wasn't compiled with
        # support
        add_supported_cxx_flags(${target_name} INTERFACE -fno-rtti /GR-)
    endif()
endfunction()
