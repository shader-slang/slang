# The same for clang
function(clang_target_from_libs target_name)
    set(clang_libs ${ARGN})
    add_library(${target_name} INTERFACE)
    # Check if we have the individual modules or not.
    if(TARGET clangBasic)
        target_link_libraries(${target_name} INTERFACE ${clang_libs})
    else()
        # If not, we can still link to the catch-all clang-cpp.
        target_link_libraries(${target_name} INTERFACE clang-cpp)
    endif()
    target_include_directories(
        ${target_name}
        SYSTEM
        INTERFACE ${CLANG_INCLUDE_DIRS}
    )
    target_compile_definitions(${target_name} INTERFACE ${CLANG_DEFINITIONS})
endfunction()

function(fetch_or_build_slang_llvm)
    if(SLANG_SLANG_LLVM_FLAVOR STREQUAL "FETCH_BINARY")
        install_fetched_shared_library(
            "slang-llvm"
            "${SLANG_SLANG_LLVM_BINARY_URL}"
        )
    elseif(SLANG_SLANG_LLVM_FLAVOR STREQUAL "FETCH_BINARY_IF_POSSIBLE")
        if(SLANG_SLANG_LLVM_BINARY_URL)
            install_fetched_shared_library(
                "slang-llvm"
                "${SLANG_SLANG_LLVM_BINARY_URL}"
                IGNORE_FAILURE
            )
            if(NOT TARGET slang-llvm)
                message(
                    WARNING
                    "Unable to fetch slang-llvm prebuilt binary, configuring without LLVM support"
                )
            endif()
        endif()
    elseif(SLANG_SLANG_LLVM_FLAVOR STREQUAL "USE_SYSTEM_LLVM")
        find_package(LLVM 21.1 REQUIRED CONFIG)
        find_package(Clang REQUIRED CONFIG)

        if(LLVM_LINK_LLVM_DYLIB)
            set(LLVM_LINK_TYPE USE_SHARED)
        endif()

        clang_target_from_libs(
            clang-dep
            clangBasic
            clangCodeGen
            clangDriver
            clangLex
            clangFrontend
            clangFrontendTool
        )
        slang_add_target(
            source/slang-llvm
            MODULE
            LINK_WITH_PRIVATE core compiler-core clang-dep
            # We include slang.h, but don't need to link with it
            INCLUDE_FROM_PRIVATE slang
            # We include tools/slang-test/filecheck.h, but don't need to link
            # with it and it might not be a target if SLANG_ENABLE_TESTS is
            # false, so just include the directory manually here
            INCLUDE_DIRECTORIES_PRIVATE ${slang_SOURCE_DIR}/tools
            # This uses the SLANG_DLL_EXPORT macro from slang.h, so make sure to set
            # SLANG_DYNAMIC and SLANG_DYNAMIC_EXPORT
            EXPORT_MACRO_PREFIX SLANG
            INSTALL
            INSTALL_COMPONENT slang-llvm
            EXPORT_SET_NAME SlangTargets
        )

        llvm_config(slang-llvm ${LLVM_LINK_TYPE} filecheck native orcjit)

        # If we don't include this, then the symbols in the LLVM linked here may
        # conflict with those of other LLVMs linked at runtime, for instance in mesa.
        set_target_properties(
            slang-llvm
            PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
        )
        add_supported_cxx_linker_flags(
            slang-llvm
            PRIVATE
            "-Wl,--exclude-libs,ALL"
        )

        # The LLVM headers need a warning disabling, which somehow slips through \external
        if(MSVC)
            target_compile_options(slang-llvm PRIVATE -wd4244 /Zc:preprocessor)
        endif()

        if(NOT LLVM_ENABLE_RTTI)
            # Make sure that we don't disable rtti if this library wasn't compiled with
            # support
            add_supported_cxx_flags(slang-llvm PRIVATE -fno-rtti /GR-)
        endif()

        # TODO: Put a check here that libslang-llvm.so doesn't have a 'NEEDED'
        # directive for libLLVM-21.so, it's almost certainly going to break at
        # runtime in surprising ways when linked alongside Mesa (or anything else
        # pulling in libLLVM.so)
    endif()

    if(SLANG_ENABLE_PREBUILT_BINARIES)
        if(CMAKE_SYSTEM_NAME MATCHES "Windows")
            file(
                GLOB prebuilt_binaries
                "${slang_SOURCE_DIR}/external/slang-binaries/bin/windows-x64/*"
            )
            list(REMOVE_ITEM prebuilt_binaries ${prebuilt_d3d12_binaries})
            add_custom_target(
                copy-prebuilt-binaries
                ALL
                COMMAND
                    ${CMAKE_COMMAND} -E make_directory
                    ${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}
                COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different ${prebuilt_binaries}
                    ${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}
                VERBATIM
            )
            set_target_properties(
                copy-prebuilt-binaries
                PROPERTIES FOLDER external
            )
        endif()
    endif()
endfunction()
