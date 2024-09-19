function(check_release_and_get_latest owner repo version os arch github_token out_var)
    # Construct the URL for the specified version's release API endpoint
    set(version_url "https://api.github.com/repos/${owner}/${repo}/releases/tags/v${version}")

    set(json_output_file "${CMAKE_CURRENT_BINARY_DIR}/${owner}_${repo}_release_info.json")

    function(check_assets_for_file json_content filename found_var)
        string(JSON asset_count LENGTH "${json_content}" "assets")
        set(found "FALSE")
        foreach(i RANGE 0 ${asset_count})
            string(JSON asset_name GET "${json_content}" "assets" ${i} "name")
            if("${asset_name}" STREQUAL "${filename}")
                set(found "TRUE")
                break()
            endif()
        endforeach()
        set(${found_var} "${found}" PARENT_SCOPE)
    endfunction()

    # Prepare download arguments
    set(download_args
        "${version_url}"
        "${json_output_file}"
        STATUS download_statuses
    )

    if(github_token)
        # Add authorization header if token is provided
        list(APPEND download_args HTTPHEADER "Authorization: token ${github_token}")
    endif()

    # Perform the download
    file(DOWNLOAD ${download_args})

    # Check if the downloading was successful
    list(GET download_statuses 0 status_code)
    if(status_code EQUAL 0)
        file(READ "${json_output_file}" json_content)

        # Check if the specified version contains the expected ZIP file
        set(desired_zip "${repo}-${version}-${os}-${arch}.zip")
        check_assets_for_file("${json_content}" "${desired_zip}" file_found)

        if(file_found)
            set(${out_var} "${version}" PARENT_SCOPE)
            return()
        endif()
        message(WARNING "Failed to find ${desired_zip} in release assets for ${version} from ${version_url}\nFalling back to latest version if it differs")
    else()
        message(WARNING "Failed to download release info for version ${version} from ${version_url}\nFalling back to latest version if it differs")

        if(status_code EQUAL 22)
            message(WARNING "If API rate limit is exceeded, Github allows a higher limit when you use token. Try a cmake option -DSLANG_GITHUB_TOKEN=your_token_here")
	endif()
    endif()


    # If not found, get the latest release tag
    set(latest_release_url "https://api.github.com/repos/${owner}/${repo}/releases/latest")
    file(DOWNLOAD "${latest_release_url}" "${json_output_file}" STATUS download_status)
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
        message(WARNING "Failed to download latest release info from ${latest_release_url}")
        return()
    endif()

    # Get the tag from this release json file
    file(READ "${json_output_file}" latest_json_content)
    string(JSON latest_release_tag GET "${latest_json_content}" "tag_name")
    string(REGEX REPLACE "^v" "" latest_version "${latest_release_tag}")

    if(latest_version EQUAL version)
        # The versions are the same
        message(WARNING "No release binary for ${os}-${arch} exists for ${version}")
        return()
    endif()

    # Check if the expected ZIP file is in the latest release
    set(desired_zip "${repo}-${latest_version}-${os}-${arch}.zip")
    check_assets_for_file("${latest_json_content}" "${desired_zip}" file_found_latest)

    if(file_found_latest)
        # If we got it, we found a good version
        set(${out_var} "${latest_version}" PARENT_SCOPE)
    else()
        message(WARNING "No release binary for ${os}-${arch} exists for ${version} or the latest version ${latest_version}")
    endif()
endfunction()

function(get_best_slang_binary_release_url github_token out_var)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
        set(arch "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
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

    check_release_and_get_latest(${owner} ${repo} ${SLANG_VERSION_NUMERIC} ${os} ${arch} "${github_token}" release_version)
    if(DEFINED release_version)
      set(${out_var} "https://github.com/${owner}/${repo}/releases/download/v${release_version}/slang-${release_version}-${os}-${arch}.zip" PARENT_SCOPE)
    endif()
endfunction()
