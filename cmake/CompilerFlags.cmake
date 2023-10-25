#
# Given a list of flags, add those which the C++ compiler supports to the target
#
function(add_supported_cxx_flags target)
  set(flags ${ARGN})
  foreach(flag ${flags})
    # remove the `no-` prefix from warnings because gcc doesn't treat it as an
    # error on its own
    string(REGEX REPLACE "\\-Wno\\-(.+)" "-W\\1" flag_to_test "${flag}")
    string(REGEX REPLACE "[^a-zA-Z0-9]+" "_" test_name
                         "CXXFLAG_${flag_to_test}")
    check_cxx_compiler_flag("${flag_to_test}" ${test_name})
    if(${test_name})
      target_compile_options(${target} PRIVATE ${flag})
    endif()
  endforeach()
endfunction()

#
# Add our default compiler flags to a target
#
# Pass USE_EXTRA_WARNINGS to enable -WExtra or /W3
#
function(set_default_compile_options target)
  # Check if the use_extra_warnings argument is provided
  if(ARGN)
    list(FIND ARGV "USE_EXTRA_WARNINGS" use_extra_index)
    if(use_extra_index GREATER -1)
      set(warning_flags -Wall -Wextra /W3)
    else()
      set(warning_flags -Wall /W2)
    endif()
  endif()

  add_supported_cxx_flags(${target} ${warning_flags})
  add_supported_cxx_flags(
    ${target}
    # Disabled warnings:
    -Wno-switch
    -Wno-parentheses
    -Wno-unused-local-typedefs
    -Wno-class-memaccess
    -Wno-assume
    -Wno-reorder
    -Wno-invalid-offsetof
    # Enabled warnings: If a function returns an address/reference to a local,
    # we want it to produce an error, because it probably means something very
    # bad.
    -Werror=return-local-addr
    # This approximates the default in MSVC
    -Wnarrowing
    # Flags: Makes all symbols hidden by default unless explicitly 'exported'
    -fvisibility=hidden)

  # TODO: use check_link_flag if we move to cmake 3.18
  if(NOT MSVC)
    # Don't assume that symbols will be resolved at runtime
    target_link_options(${target} PRIVATE "-Wl,--no-undefined")
  endif()

  # C++ standard
  set_target_properties(${target} PROPERTIES CXX_STANDARD 17)

  # PIC
  set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)

  # Add _DEBUG depending on the build type
  target_compile_definitions(${target} PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

  # For including windows.h in a way that minimized namespace pollution.
  # Although we define these here, we still set them manually in any header
  # files which may be included by another project
  target_compile_definitions(${target} PRIVATE WIN32_LEAN_AND_MEAN VC_EXTRALEAN
                                               NOMINMAX _ITERATOR_DEBUG_LEVEL=0)

  if(SLANG_ENABLE_DX_ON_VK)
    target_compile_definitions(${target} PRIVATE SLANG_CONFIG_DX_ON_VK)
  endif()

  if(SLANG_ENABLE_ASAN)
    add_supported_cxx_flags(${target} /fsanitize=address -fsanitize=address)
    if(MSVC)
      target_link_options(${target} BEFORE PUBLIC /INCREMENTAL:NO)
    else()
      target_link_options(${target} BEFORE PUBLIC -fsanitize=address)
    endif()
  endif()
endfunction()
