add_executable(
  slang-cpp-extractor EXCLUDE_FROM_ALL
  cpp-extractor-main.cpp
  diagnostics.cpp
  file-util.cpp
  identifier-lookup.cpp
  macro-writer.cpp
  node-tree.cpp
  node.cpp
  options.cpp
  parser.cpp
  unit-test.cpp)
set_default_compile_options(slang-cpp-extractor)
target_link_libraries(slang-cpp-extractor PRIVATE core compiler-core)
