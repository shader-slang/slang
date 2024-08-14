function(check_release_and_get_latest owner repo version os arch out_var)
    # Construct the URL for the specified version's release API endpoint
    set(version_url "https://api.github.com/repos/${owner}/${repo}/releases/tags/v${version}")

    # Path to save the JSON response
    set(json_output_file "${CMAKE_BINARY_DIR}/release_info.json")

    # Function to check if the file exists in the assets
    function(check_assets_for_file json_content expected_zip found_var)
        # Parse the JSON array of assets
        string(JSON asset_count LENGTH "${json_content}" "assets")
        set(found "FALSE")
        foreach(i RANGE 0 ${asset_count})
            string(JSON asset_name GET "${json_content}" "assets" ${i} "name")
            if("${asset_name}" STREQUAL "${expected_zip}")
                set(found "TRUE")
                break()
            endif()
        endforeach()
        set(${found_var} "${found}" PARENT_SCOPE)
    endfunction()

    # Download the specified release info from GitHub
    file(DOWNLOAD "${version_url}" "${json_output_file}" STATUS download_status)

    # Check if the download was successful
    list(GET download_status 0 status_code)
    if(status_code EQUAL 0)
        # Read the downloaded JSON file
        file(READ "${json_output_file}" json_content)

        # Construct the expected zip file name
        set(expected_zip "slang-${version}-${os}-${arch}.zip")

        # Check if the specified version contains the expected ZIP file
        check_assets_for_file("${json_content}" "${expected_zip}" file_found)

        if(file_found)
            set(${out_var} "${version}" PARENT_SCOPE)
        endif()
        return()
    endif()

    message(WARNING "Failed to download release info for version ${version} from ${version_url}.\nFalling back to latest version if it differs")

    # If not found, get the latest release
    set(latest_release_url "https://api.github.com/repos/${owner}/${repo}/releases/latest")

    # Download the latest release info
    file(DOWNLOAD "${latest_release_url}" "${json_output_file}" STATUS download_status)

    # Check if the download was successful
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
        message(WARNING "Failed to download latest release info from ${latest_release_url}")
        return()
    endif()

    # Read the latest release JSON
    file(READ "${json_output_file}" latest_json_content)

    string(JSON latest_release_tag GET "${latest_json_content}" "tag_name")
    string(REGEX REPLACE "^v" "" latest_version "${latest_release_tag}")

    if(latest_version EQUAL version)
        # The versions are the same
        message(WARNING "No release binary for ${os}-${arch} exists for ${version}")
        return()
    endif()

    # Construct the expected zip file name
    set(expected_zip "slang-${latest_version}-${os}-${arch}.zip")

    # Check if the expected ZIP file is in the latest release
    check_assets_for_file("${latest_json_content}" "${expected_zip}" file_found_latest)

    if(file_found_latest)
        # Extract the latest release tag name and set it as the output variable
        set(${out_var} "${latest_version}" PARENT_SCOPE)
    else()
        message(WARNING "No release binary for ${os}-${arch} exists for ${version} or the latest version ${latest_version}")
        return()
    endif()
endfunction()

function(get_best_slang_binary_release_url out_var)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
        set(arch "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(arch "aarch64")
    else()
        message(WARNING "Unsupported architecture for slang binary releases: ${CMAKE_SYSTEM_PROCESSOR}")
        return()
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(os "windows")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(os "macos")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(os "linux")
    else()
        message(WARNING "Unsupported operating system for slang binary releases: ${CMAKE_SYSTEM_NAME}")
        return()
    endif()

    set(owner "shader-slang")
    set(repo "slang")

    check_release_and_get_latest(${owner} ${repo} ${SLANG_VERSION_NUMERIC} ${os} ${arch} release_version)

    set(${out_var} "https://github.com/${owner}/${repo}/releases/download/v${release_version}/slang-${release_version}-${os}-${arch}.zip" PARENT_SCOPE)
endfunction()
