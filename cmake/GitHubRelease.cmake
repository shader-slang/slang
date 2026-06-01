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

    # The prebuilt libslang-llvm library is published as a release asset whose
    # version is selected purely from the git-tag version (SLANG_VERSION_NUMERIC).
    # The release-asset download below is a direct github.com URL and is NOT
    # subject to the GitHub REST API rate limit, so we deliberately avoid querying
    # api.github.com here (and therefore need no access token).
    #
    # v2024.1.27 is the first version that distributed libslang-llvm. An older or
    # missing version (0.0.0) means git tags were not fetched, in which case a
    # prebuilt cannot be resolved; warn and let the caller decide whether that is
    # fatal.
    if(SLANG_VERSION_NUMERIC VERSION_LESS "2024.1.27")
        message(
            WARNING
            "The detected slang version is ${SLANG_VERSION_NUMERIC}, which predates "
            "the first release that distributed a prebuilt libslang-llvm (v2024.1.27). "
            "This usually means git tags were not fetched, so a prebuilt slang-llvm "
            "cannot be resolved for this version.\n"
            "Fetch tags from the official repository:\n"
            "    git fetch https://github.com/shader-slang/slang.git 'refs/tags/*:refs/tags/*'\n"
            "or build slang-llvm from source with -DSLANG_SLANG_LLVM_FLAVOR=USE_SYSTEM_LLVM."
        )
        return()
    endif()

    set(${out_var}
        "https://github.com/${owner}/${repo}/releases/download/v${SLANG_VERSION_NUMERIC}/slang-${SLANG_VERSION_NUMERIC}-${os}-${arch}.zip"
        PARENT_SCOPE
    )
endfunction()
