function(get_best_slang_binary_release_url out_var)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
        set(arch "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
        set(arch "aarch64")
    else()
        message(
            WARNING
            "Unsupported architecture for slang binary releases: ${CMAKE_SYSTEM_PROCESSOR}"
        )
        return()
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(os "windows")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(os "macos")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(os "linux")
    else()
        message(
            WARNING
            "Unsupported operating system for slang binary releases: ${CMAKE_SYSTEM_NAME}"
        )
        return()
    endif()

    set(owner "shader-slang")
    set(repo "slang")

    if(${SLANG_VERSION_NUMERIC} VERSION_EQUAL "0.0.0")
        message(
            WARNING
            "Could not determine Slang version from git tags — unable to select a prebuilt slang-llvm binary.\n"
            "Git tags are required to download the correct prebuilt library.\n"
            "If you cloned from https://github.com/shader-slang/slang.git, fetch tags:\n"
            "    git fetch --tags\n"
            "If you cloned from a mirror or fork, fetch tags from the official repository:\n"
            "    git fetch https://github.com/shader-slang/slang.git 'refs/tags/*:refs/tags/*'\n"
            "Alternatively, set SLANG_SLANG_LLVM_BINARY_URL manually or use a different SLANG_SLANG_LLVM_FLAVOR."
        )
        return()
    endif()

    # This is the first version which distributed libslang-llvm.so
    if(${SLANG_VERSION_NUMERIC} VERSION_LESS "2024.1.27")
        message(
            WARNING
            "Slang version ${SLANG_VERSION_NUMERIC} predates prebuilt slang-llvm binary releases (first available in 2024.1.27).\n"
            "If your git tags appear stale, fetch them:\n"
            "    git fetch --tags\n"
            "Alternatively, set SLANG_SLANG_LLVM_BINARY_URL manually or use a different SLANG_SLANG_LLVM_FLAVOR."
        )
        return()
    endif()

    set(${out_var}
        "https://github.com/${owner}/${repo}/releases/download/v${SLANG_VERSION_NUMERIC}/slang-${SLANG_VERSION_NUMERIC}-${os}-${arch}.zip"
        PARENT_SCOPE
    )
endfunction()
