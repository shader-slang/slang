# Host runtime and resource algorithms are shipped as ordinary importable modules.
# Keep the source hierarchy identical to the public import hierarchy beneath slang/.
set(host_module_root
    "${slang_BINARY_DIR}/$<CONFIG>/${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}"
)
set(host_source_root "${CMAKE_CURRENT_BINARY_DIR}/source")

set(host_modules
    runtime
    runtime/core/simple_assert
    runtime/core/simple_string
    runtime/core/simple_time
    runtime/io/byte_stream
    runtime/io/exr_header
    runtime/io/image
    runtime/io/image_header
    runtime/io/simple_file
    runtime/resource/buffer_view
    runtime/resource/simple_texture
    runtime/resource/texture_clear
    runtime/resource/texture_format
    runtime/resource/upload_buffer
    runtime/compute/grid
    runtime/raster/draw_parameters
    runtime/raster/primitive_stream
    runtime/raster/raster_state
    runtime/raster/render_targets
    pipeline_rt
    runtime/window/simple_input
    runtime/window/simple_window
    resource/resizable_buffer
    compute/buffer_utils
    compute/texture_utils
    tensor
    tensor/differentiable
)
set(host_sources)
foreach(module ${host_modules})
    list(APPEND host_sources "${module}.slang")
endforeach()

# Like numerics, build dependencies from staged sources: a bootstrap invocation that
# loads the core archive cannot also deserialize another module referencing that core.
# Installed clients load the compiled artifacts, with no source checkout or -I needed.
set(host_staged_sources)
foreach(source ${host_sources})
    set(staged "${host_source_root}/slang/${source}")
    get_filename_component(staged_dir "${staged}" DIRECTORY)
    add_custom_command(
        OUTPUT "${staged}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${staged_dir}"
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${source}" "${staged}"
        DEPENDS "${source}"
        VERBATIM
    )
    list(APPEND host_staged_sources "${staged}")
endforeach()
get_directory_property(
    numerics_source_stage_root
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../numerics"
    DEFINITION numerics_source_stage_root
)
get_directory_property(
    numerics_staged_sources
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../numerics"
    DEFINITION numerics_staged_sources
)

add_custom_target(
    slang-host-module-sources
    DEPENDS ${host_staged_sources} ${numerics_staged_sources}
)
set_target_properties(slang-host-module-sources PROPERTIES FOLDER generated)
if(TARGET slangvm-javascript)
    add_dependencies(slangvm-javascript slang-host-module-sources)
endif()

if(SLANG_GENERATORS_PATH)
    if(CMAKE_HOST_WIN32)
        set(host_compiler_suffix ".exe")
    else()
        set(host_compiler_suffix "")
    endif()
    set(host_compiler
        "${SLANG_GENERATORS_PATH}/slang-bootstrap${host_compiler_suffix}"
    )
    set(host_compiler_dependencies "${host_compiler}")
else()
    set(host_compiler slang-bootstrap)
    set(host_compiler_dependencies
        slang-bootstrap
        slang-without-embedded-core-module
    )
endif()

set(host_module_files)
# Serialize library IR without trying to specialize generic shader entry points.
# The canonical import name must also be the serialized module name: HGBC dispatch
# references use it to find the module containing the shader implementation.
# Omit library debug IR: HGBC cannot yet consume all generic DebugFunction records
# from precompiled dependencies when the application itself enables debugging.
foreach(module ${host_modules})
    set(output "${host_module_root}/slang/${module}.slang-module")
    get_filename_component(output_dir "${output}" DIRECTORY)
    add_custom_command(
        OUTPUT "${output}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}"
        COMMAND
            ${host_compiler} -load-core-module
            ${core_module_archive_without_timestamp} -experimental-feature
            -emit-ir -no-codegen -g0 -module-name "slang/${module}" -I
            "${host_source_root}" -I "${numerics_source_stage_root}"
            "${host_source_root}/slang/${module}.slang" -o "${output}"
        DEPENDS
            ${host_staged_sources}
            ${numerics_staged_sources}
            ${host_compiler_dependencies}
            generate_core_module
            slang-numerics-modules
        VERBATIM
    )
    list(APPEND host_module_files "${output}")
    get_filename_component(module_dir "${module}" DIRECTORY)
    install(
        FILES "${output}"
        DESTINATION "${SLANG_STANDARD_MODULE_INSTALL_DIR}/slang/${module_dir}"
    )
endforeach()

install(
    DIRECTORY "${host_source_root}/" "${numerics_source_stage_root}/"
    DESTINATION javascript/slangvm/slang
    COMPONENT slangvm-javascript
    EXCLUDE_FROM_ALL
    FILES_MATCHING
    PATTERN "*.slang"
)

add_custom_target(slang-host-modules ALL DEPENDS ${host_module_files})
set_target_properties(slang-host-modules PROPERTIES FOLDER generated)

if(TARGET slangvm-javascript)
    # Ship the serialized modules beside their sources. Use this build's core archive
    # and generators, just like the native installation, so the WASM compiler can
    # load the binaries while slangd still has the sources it prefers.
    set(browser_module_files ${host_module_files})
    foreach(
        module_variable
        numerics_module_file
        numerics_builtin_module_file
        numerics_differentiable_builtin_module_file
    )
        get_directory_property(
            module_file
            DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../numerics"
            DEFINITION ${module_variable}
        )
        list(APPEND browser_module_files "${module_file}")
    endforeach()
    set(browser_module_root "$<TARGET_FILE_DIR:slangvm-wasm>/../slang")
    set(browser_module_commands)
    set(browser_module_inventory)
    foreach(module_file ${browser_module_files})
        string(REPLACE "${host_module_root}/" "" module_path "${module_file}")
        get_filename_component(module_dir "${module_path}" DIRECTORY)
        list(
            APPEND
            browser_module_commands
            COMMAND
            ${CMAKE_COMMAND}
            -E
            make_directory
            "${browser_module_root}/${module_dir}"
            COMMAND
            ${CMAKE_COMMAND}
            -E
            copy_if_different
            "${module_file}"
            "${browser_module_root}/${module_path}"
        )
        list(APPEND browser_module_inventory "  \"${module_path}\"")
        install(
            FILES "${module_file}"
            DESTINATION "javascript/slangvm/slang/${module_dir}"
            COMPONENT slangvm-javascript
            EXCLUDE_FROM_ALL
        )
    endforeach()
    list(JOIN browser_module_inventory ",\n" browser_module_inventory_json)
    set(browser_module_manifest
        "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/standard-modules.json"
    )
    file(
        GENERATE OUTPUT
        "${browser_module_manifest}"
        CONTENT "[\n${browser_module_inventory_json}\n]\n"
        NEWLINE_STYLE LF
    )
    add_custom_target(
        slangvm-browser-standard-modules
        ${browser_module_commands}
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "${browser_module_manifest}"
            "${browser_module_root}/standard-modules.json"
        DEPENDS slang-host-modules slang-numerics-modules
        VERBATIM
    )
    set_target_properties(
        slangvm-browser-standard-modules
        PROPERTIES FOLDER generated
    )
    add_dependencies(slangvm-javascript slangvm-browser-standard-modules)
    install(
        FILES "${browser_module_manifest}"
        DESTINATION javascript/slangvm/slang
        COMPONENT slangvm-javascript
        EXCLUDE_FROM_ALL
    )
endif()

foreach(
    consumer
    slangc
    slangi
    slangd
    slang-test
    _slangvm
)
    if(TARGET ${consumer})
        add_dependencies(${consumer} slang-host-modules)
    endif()
endforeach()

# The Windows Python package carries its own compiler DLL. Standard-module discovery
# is relative to that DLL, so keep its module directory in sync even without a relink.
if(WIN32 AND TARGET _slangvm)
    add_custom_target(
        slangvm-standard-modules
        COMMAND
            ${CMAKE_COMMAND} -E copy_directory "${host_module_root}"
            "$<TARGET_FILE_DIR:_slangvm>/${SLANG_STANDARD_MODULE_DIR_NAME}"
        DEPENDS slang-host-modules
        VERBATIM
    )
    set_target_properties(slangvm-standard-modules PROPERTIES FOLDER generated)
    add_dependencies(_slangvm slangvm-standard-modules)
endif()

# Sources support tooling and the browser package, which resolves imports via its host filesystem.
foreach(source ${host_sources})
    get_filename_component(source_dir "${source}" DIRECTORY)
    install(
        FILES "${source}"
        DESTINATION "${SLANG_STANDARD_MODULE_INSTALL_DIR}/slang/${source_dir}"
    )
endforeach()
