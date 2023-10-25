add_library(
  slang-unit-test SHARED EXCLUDE_FROM_ALL
  unit-test-byte-encode.cpp
  unit-test-chunked-list.cpp
  unit-test-com-host-callable.cpp
  unit-test-command-line-args.cpp
  unit-test-compression.cpp
  unit-test-crypto.cpp
  unit-test-file-system.cpp
  unit-test-find-type-by-name.cpp
  unit-test-free-list.cpp
  unit-test-io.cpp
  unit-test-json-native.cpp
  unit-test-json.cpp
  unit-test-lock-file.cpp
  unit-test-memory-arena.cpp
  unit-test-offset-container.cpp
  unit-test-path.cpp
  unit-test-persistent-cache.cpp
  unit-test-process.cpp
  unit-test-riff.cpp
  unit-test-rtti.cpp
  unit-test-short-list.cpp
  unit-test-source-map.cpp
  unit-test-string-escape.cpp
  unit-test-string.cpp
  unit-test-translation-unit-import.cpp)
set_default_compile_options(slang-unit-test)
target_link_libraries(slang-unit-test PRIVATE core compiler-core unit-test
                                              slang Threads::Threads)
target_compile_definitions(slang-unit-test PRIVATE SLANG_SHARED_LIBRARY_TOOL)
set_target_properties(slang-unit-test PROPERTIES OUTPUT_NAME
                                                 slang-unit-test-tool)
