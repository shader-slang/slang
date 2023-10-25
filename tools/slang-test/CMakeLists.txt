add_executable(
  slang-test
  directory-util.cpp
  options.cpp
  parse-diagnostic-util.cpp
  slang-test-main.cpp
  slangc-tool.cpp
  test-context.cpp
  test-reporter.cpp)
set_default_compile_options(slang-test)
target_link_libraries(slang-test PRIVATE core compiler-core slang
                                         ${CMAKE_DL_LIBS} Threads::Threads)
add_dependencies(slang-test slang-reflection-test render-test slang-unit-test
                 gfx-unit-test)
set_property(
  TARGET slang-test
  APPEND
  PROPERTY BUILD_RPATH
           "$<TARGET_FILE_DIR:slang-reflection-test>"
           "$<TARGET_FILE_DIR:render-test>"
           "$<TARGET_FILE_DIR:slang-unit-test>"
           "$<TARGET_FILE_DIR:gfx-unit-test>")
