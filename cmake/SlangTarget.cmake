function(slang_add_target dir type)
  cmake_parse_arguments(
    ARG
    "EXCLUDE_FROM_ALL;SHARED_LIBRARY_TOOL;USE_EXTRA_WARNINGS;USE_FEWER_WARNINGS"
    "TARGET_NAME;OUTPUT_NAME;EXPORT_MACRO_PREFIX"
    "EXTRA_SOURCE_DIRS;LINK_WITH_PRIVATE;INCLUDE_FROM_PRIVATE;MODULE_DEPENDS;INCLUDE_DIRECTORIES_PUBLIC"
    ${ARGV})
  get_filename_component(dir_absolute ${dir} REALPATH)
  if(DEFINED ARG_TARGET_NAME)
    set(target ${ARG_TARGET_NAME})
  else()
    get_filename_component(target ${dir_absolute} NAME)
  endif()

  slang_glob_sources(source "${dir}/*.cpp")
  foreach(extra_dir ${ARG_EXTRA_SOURCE_DIRS})
    slang_glob_sources(source "${extra_dir}/*.cpp")
  endforeach()

  set(library_types STATIC SHARED OBJECT MODULE ALIAS)
  if(type STREQUAL "EXECUTABLE")
    add_executable(${target} ${source})
  elseif(type STREQUAL "LIBRARY")
    add_library(${target} ${source})
  elseif(type IN_LIST library_types)
    add_library(${target} ${type} ${source})
  else()
    message(SEND_ERROR "Unsupported target type ${type} in slang_add_target")
    return()
  endif()

  if(ARG_USE_EXTRA_WARNINGS)
    set_default_compile_options(${target} USE_EXTRA_WARNINGS)
  elseif(ARG_USE_FEWER_WARNINGS)
    set_default_compile_options(${target} USE_FEWER_WARNINGS)
  else()
    set_default_compile_options(${target})
  endif()

  set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL
                                                  ${ARG_EXCLUDE_FROM_ALL})

  target_link_libraries(${target} PRIVATE ${ARG_LINK_WITH_PRIVATE})

  foreach(include_only_target ${ARG_INCLUDE_FROM_PRIVATE})
    target_include_directories(
      ${target}
      PRIVATE
        $<TARGET_PROPERTY:${include_only_target},INTERFACE_INCLUDE_DIRECTORIES>)
  endforeach()

  if(DEFINED ARG_OUTPUT_NAME)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME
                                                    ${ARG_OUTPUT_NAME})
  endif()

  #
  # Include directories
  #
  foreach(inc ${ARG_INCLUDE_DIRECTORIES_PUBLIC})
    get_filename_component(inc_abs ${inc} ABSOLUTE)
    target_include_directories(${target}
                               PUBLIC "$<BUILD_INTERFACE:${inc_abs}>")
  endforeach()

  #
  # Runtime dependencies on other targets
  #
  if(DEFINED ARG_MODULE_DEPENDS)
    add_dependencies(${target} ${ARG_MODULE_DEPENDS})
    list(TRANSFORM ARG_MODULE_DEPENDS REPLACE "(.+)" "$<TARGET_FILE_DIR:\\1>"
                                              OUTPUT_VARIABLE build_rpaths)
    set_property(
      TARGET ${target}
      APPEND
      PROPERTY BUILD_RPATH ${build_rpaths})
    get_target_property(x ${target} BUILD_RPATH)
  endif()

  #
  # Macros for declaring exports
  #

  if(ARG_SHARED_LIBRARY_TOOL)
    target_compile_definitions(${target} PRIVATE SLANG_SHARED_LIBRARY_TOOL)
  endif()

  get_target_property(target_type ${target} TYPE)
  if(target_type STREQUAL SHARED_LIBRARY OR target_type STREQUAL MODULE_LIBRARY)
    if(DEFINED ARG_EXPORT_MACRO_PREFIX)
      target_compile_definitions(
        ${target}
        PUBLIC "${ARG_EXPORT_MACRO_PREFIX}_DYNAMIC"
        PRIVATE "${ARG_EXPORT_MACRO_PREFIX}_DYNAMIC_EXPORT")
    endif()
  endif()
endfunction()
