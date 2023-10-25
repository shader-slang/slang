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
                                         Threads::Threads)
