# Installs the user-facing agent skills pinned by the Slang superproject.
function(slang_install_user_skills)
    if(SLANG_INSTALL_USER_SKILLS STREQUAL "OFF")
        message(
            STATUS
            "Slang user skills will not be installed (SLANG_INSTALL_USER_SKILLS=OFF)"
        )
        return()
    endif()

    set(_source_dir "${slang_SOURCE_DIR}/external/slang-user-skills")
    set(_submodule_path "external/slang-user-skills")
    set(_problems)

    if(NOT IS_DIRECTORY "${_source_dir}/skills")
        list(APPEND _problems "${_submodule_path}/skills is missing")
    endif()
    if(NOT EXISTS "${_source_dir}/README.md")
        list(APPEND _problems "${_submodule_path}/README.md is missing")
    endif()
    if(NOT EXISTS "${_source_dir}/LICENSE")
        list(APPEND _problems "${_submodule_path}/LICENSE is missing")
    endif()

    if(NOT GIT_EXECUTABLE)
        list(
            APPEND
            _problems
            "Git is unavailable, so the submodule pin cannot be verified"
        )
    elseif(NOT _problems)
        execute_process(
            COMMAND
                "${GIT_EXECUTABLE}" -C "${slang_SOURCE_DIR}" rev-parse
                "HEAD:${_submodule_path}"
            RESULT_VARIABLE _gitlink_result
            OUTPUT_VARIABLE _gitlink_commit
            ERROR_VARIABLE _gitlink_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
        if(NOT _gitlink_result EQUAL 0)
            list(
                APPEND
                _problems
                "the committed submodule pin could not be read: ${_gitlink_error}"
            )
        endif()

        execute_process(
            COMMAND "${GIT_EXECUTABLE}" -C "${_source_dir}" rev-parse HEAD
            RESULT_VARIABLE _checkout_result
            OUTPUT_VARIABLE _checkout_commit
            ERROR_VARIABLE _checkout_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
        if(NOT _checkout_result EQUAL 0)
            list(
                APPEND
                _problems
                "the checked-out skills commit could not be read: ${_checkout_error}"
            )
        endif()

        if(_gitlink_result EQUAL 0 AND _checkout_result EQUAL 0)
            string(TOLOWER "${_gitlink_commit}" _gitlink_commit)
            string(TOLOWER "${_checkout_commit}" _checkout_commit)
            string(LENGTH "${_gitlink_commit}" _gitlink_length)
            if(
                NOT _gitlink_length EQUAL 40
                OR NOT _gitlink_commit MATCHES "^[0-9a-f]+$"
            )
                list(
                    APPEND
                    _problems
                    "the committed submodule pin is not a full Git commit SHA: ${_gitlink_commit}"
                )
            elseif(NOT _checkout_commit STREQUAL _gitlink_commit)
                list(
                    APPEND
                    _problems
                    "the skills checkout ${_checkout_commit} does not match the committed pin ${_gitlink_commit}"
                )
            endif()
        endif()

        execute_process(
            COMMAND
                "${GIT_EXECUTABLE}" -C "${_source_dir}" status --porcelain
                --untracked-files=all
            RESULT_VARIABLE _status_result
            OUTPUT_VARIABLE _status_output
            ERROR_VARIABLE _status_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
        if(NOT _status_result EQUAL 0)
            list(
                APPEND
                _problems
                "the skills checkout status could not be read: ${_status_error}"
            )
        elseif(_status_output)
            list(APPEND _problems "the skills checkout contains local changes")
        endif()
    endif()

    if(_problems)
        list(JOIN _problems "\n  - " _problem_text)
        string(
            CONCAT
            _message
            "Cannot install Slang user skills:\n  - ${_problem_text}\n"
            "Initialize the pinned checkout with:\n"
            "  git submodule update --init ${_submodule_path}"
        )
        if(SLANG_INSTALL_USER_SKILLS STREQUAL "ON")
            message(FATAL_ERROR "${_message}")
        endif()
        message(
            STATUS
            "${_message}\nContinuing because SLANG_INSTALL_USER_SKILLS=AUTO."
        )
        return()
    endif()

    set(SLANG_USER_SKILLS_SOURCE_COMMIT "${_gitlink_commit}")
    set(_generated_dir "${slang_BINARY_DIR}/user-skills")
    set(_install_dir "share/slang/agent-skills")
    file(MAKE_DIRECTORY "${_generated_dir}")
    configure_file(
        "${slang_SOURCE_DIR}/cmake/SlangUserSkillsProvenance.json.in"
        "${_generated_dir}/PROVENANCE.json"
        @ONLY
    )

    install(
        DIRECTORY "${_source_dir}/skills/"
        DESTINATION "${_install_dir}/skills"
        COMPONENT user-skills
        PATTERN ".*" EXCLUDE
    )
    install(
        FILES "${_source_dir}/README.md" "${_source_dir}/LICENSE"
        DESTINATION "${_install_dir}"
        COMPONENT user-skills
    )
    install(
        FILES "${_generated_dir}/PROVENANCE.json"
        DESTINATION "${_install_dir}"
        COMPONENT user-skills
    )
    message(
        STATUS
        "Slang user skills will be installed from ${_gitlink_commit}"
    )
endfunction()
