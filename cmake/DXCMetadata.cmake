# DXC release metadata, shared by FetchDXC.cmake (download / source build),
# FindDXC.cmake (system-DXC version check), and StageDXCForCI.cmake.
#
# To upgrade DXC:
#   1. Pick a release tag from
#      https://github.com/microsoft/DirectXShaderCompiler/releases.
#   2. Set SLANG_DXC_VERSION_TAG to that tag.
#   3. Set SLANG_DXC_RELEASE_DATE to the date used in the asset names
#      (dxc_<YYYY_MM_DD>.zip and linux_dxc_<YYYY_MM_DD>.x86_64.tar.gz).
#   4. Resolve the tag to the source commit (peeled hash for annotated tags):
#      git ls-remote https://github.com/microsoft/DirectXShaderCompiler.git \
#          "refs/tags/<tag>" "refs/tags/<tag>^{}"
#   5. Update the hashes:
#      sha256sum dxc_<YYYY_MM_DD>.zip linux_dxc_<YYYY_MM_DD>.x86_64.tar.gz
#   6. Keep external/slang-rhi/CMakeLists.txt's SLANG_RHI_DXC_URL in sync
#      with SLANG_DXC_WINDOWS_URL below.

set(SLANG_DXC_VERSION_TAG "v1.9.2602")
set(SLANG_DXC_EXPECTED_GIT_COMMIT "21d28f727ad395b59394815ef76012e432f7e4e5")
set(SLANG_DXC_RELEASE_DATE "2026_02_20")
set(SLANG_DXC_WINDOWS_SHA256
    "a1e89031421cf3c1fca6627766ab3020ca4f962ac7e2caa7fab2b33a8436151e"
)
set(SLANG_DXC_LINUX_SHA256
    "a1d3e3b5e1c5685b3eb27d5e8890e41d87df45def05112a2d6f1a63a931f7d60"
)

set(SLANG_DXC_WINDOWS_URL
    "https://github.com/microsoft/DirectXShaderCompiler/releases/download/${SLANG_DXC_VERSION_TAG}/dxc_${SLANG_DXC_RELEASE_DATE}.zip"
)
set(SLANG_DXC_LINUX_URL
    "https://github.com/microsoft/DirectXShaderCompiler/releases/download/${SLANG_DXC_VERSION_TAG}/linux_dxc_${SLANG_DXC_RELEASE_DATE}.x86_64.tar.gz"
)
