# Used when SLANG_USE_SYSTEM_DXC is ON: find an installed DXC and re-emit
# the copy-dxcompiler / copy-dxil / stage-dxc-headers targets that
# FetchDXC.cmake exposes, so the rest of the build is oblivious to which
# acquisition path was taken.
#
# Requires runtime_subdir and library_subdir from SlangTarget.cmake.

find_package(DXC REQUIRED)

# Stage DXC HLSL headers (dx/linalg.h) at the same build-tree location
# FetchDXC uses so `slangc -Xdxc -Ibuild/dxc/include` and test directives
# resolve identically. A system DXC usually lays them out under
# <prefix>/include/dxc.
if(EXISTS "${DXC_INCLUDE_DIRS}/dx/linalg.h")
    set(_dxc_hlsl_include_dir "${DXC_INCLUDE_DIRS}")
elseif(EXISTS "${DXC_INCLUDE_DIRS}/hlsl/dx/linalg.h")
    set(_dxc_hlsl_include_dir "${DXC_INCLUDE_DIRS}/hlsl")
else()
    set(_dxc_hlsl_include_dir "")
endif()

if(_dxc_hlsl_include_dir)
    set(_dxc_inc_dst "${CMAKE_BINARY_DIR}/dxc/include/dx/linalg.h")
    add_custom_command(
        OUTPUT "${_dxc_inc_dst}"
        COMMAND
            ${CMAKE_COMMAND} -E copy_directory "${_dxc_hlsl_include_dir}"
            "${CMAKE_BINARY_DIR}/dxc/include"
        DEPENDS "${_dxc_hlsl_include_dir}/dx/linalg.h"
        VERBATIM
    )
    add_custom_target(stage-dxc-headers DEPENDS "${_dxc_inc_dst}")
else()
    # Same FATAL/WARNING split as FetchDXC's _dxc_stage_hlsl_headers.
    if(SLANG_ENABLE_TESTS)
        message(
            FATAL_ERROR
            "System DXC at ${DXC_INCLUDE_DIRS} is missing dx/linalg.h, "
            "which the cooperative-{vector,matrix} tests rely on. Install "
            "a DXC build that ships the public HLSL headers, or disable "
            "SLANG_ENABLE_TESTS."
        )
    endif()
    message(
        WARNING
        "System DXC at ${DXC_INCLUDE_DIRS} is missing dx/linalg.h; "
        "DXIL cooperative-{vector,matrix} local builds that pass "
        "-Xdxc -Ibuild/dxc/include will need an external DXC include path."
    )
    add_custom_target(stage-dxc-headers)
endif()
set_target_properties(stage-dxc-headers PROPERTIES FOLDER generated)

# Stage runtime libraries next to the binaries with the same destination
# layout as FetchDXC. dxil is enforced by FindDXC on Windows; elsewhere it
# is optional, so copy-dxil is only emitted when it was actually found.
set(_dxc_names dxcompiler)
set(_dxc_srcs "${DXC_DXCOMPILER_RUNTIME}")
if(WIN32)
    set(_dxc_subdir "${runtime_subdir}")
    list(APPEND _dxc_names dxil)
    list(APPEND _dxc_srcs "${DXC_DXIL_RUNTIME}")
else()
    set(_dxc_subdir "${library_subdir}")
    if(DXC_DXIL_RUNTIME)
        list(APPEND _dxc_names dxil)
        list(APPEND _dxc_srcs "${DXC_DXIL_RUNTIME}")
    endif()
endif()

foreach(_dxc_lib _dxc_src IN ZIP_LISTS _dxc_names _dxc_srcs)
    get_filename_component(_dxc_filename "${_dxc_src}" NAME)
    set(_dxc_dst
        "${CMAKE_BINARY_DIR}/$<CONFIG>/${_dxc_subdir}/${_dxc_filename}"
    )
    add_custom_command(
        OUTPUT "${_dxc_dst}"
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "${_dxc_src}" "${_dxc_dst}"
        DEPENDS "${_dxc_src}"
        VERBATIM
    )
    add_custom_target(copy-${_dxc_lib} DEPENDS "${_dxc_dst}")
    set_target_properties(copy-${_dxc_lib} PROPERTIES FOLDER generated)
endforeach()
