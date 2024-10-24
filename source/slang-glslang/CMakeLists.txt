#
# Our wrapper for glslang 
#
if(SLANG_ENABLE_SLANG_GLSLANG)
    slang_add_target(
        .
        MODULE
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE glslang SPIRV SPIRV-Tools-opt
        INCLUDE_DIRECTORIES_PRIVATE ${slang_SOURCE_DIR}/include
        INSTALL
    )
    # Our only interface is through what we define in source/slang-glslang, in the
    # interests of hygiene, hide anything else we link in.
    add_supported_cxx_linker_flags(
        slang-glslang
        PRIVATE
        "-Wl,--exclude-libs,ALL"
    )
endif()
