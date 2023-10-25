set(SLANG_CORE_SOURCE
    slang-archive-file-system.cpp
    slang-blob.cpp
    slang-byte-encode-util.cpp
    slang-castable.cpp
    slang-char-encode.cpp
    slang-char-util.cpp
    slang-command-line.cpp
    slang-command-options-writer.cpp
    slang-command-options.cpp
    slang-crypto.cpp
    slang-deflate-compression-system.cpp
    slang-file-system.cpp
    slang-free-list.cpp
    slang-hex-dump-util.cpp
    slang-http.cpp
    slang-implicit-directory-collector.cpp
    slang-io.cpp
    slang-lz4-compression-system.cpp
    slang-memory-arena.cpp
    slang-memory-file-system.cpp
    slang-name-value.cpp
    slang-offset-container.cpp
    slang-performance-profiler.cpp
    slang-persistent-cache.cpp
    slang-platform.cpp
    slang-process-util.cpp
    slang-random-generator.cpp
    slang-render-api-util.cpp
    slang-riff-file-system.cpp
    slang-riff.cpp
    slang-rtti-info.cpp
    slang-rtti-util.cpp
    slang-semantic-version.cpp
    slang-shared-library.cpp
    slang-signal.cpp
    slang-std-writers.cpp
    slang-stream.cpp
    slang-string-escape-util.cpp
    slang-string-slice-index-map.cpp
    slang-string-slice-pool.cpp
    slang-string-util.cpp
    slang-string.cpp
    slang-test-tool-util.cpp
    slang-text-io.cpp
    slang-token-reader.cpp
    slang-type-convert-util.cpp
    slang-type-text-util.cpp
    slang-uint-set.cpp
    slang-writer.cpp
    slang-zip-file-system.cpp)

if(WIN32)
  list(APPEND SLANG_CORE_SOURCE windows/slang-win-process.cpp)
endif()

if(UNIX)
  list(APPEND SLANG_CORE_SOURCE unix/slang-unix-process.cpp)
endif()

add_library(core STATIC EXCLUDE_FROM_ALL ${SLANG_CORE_SOURCE})
set_default_compile_options(core USE_EXTRA_WARNINGS)
target_link_libraries(core PRIVATE miniz lz4_static)

# When things link with core (mainly gfx), they often reference files as
# "core/slang-blah.h", so add the parent directory as an interface for this
# library
target_include_directories(
  core INTERFACE $<BUILD_INTERFACE:${SLANG_ROOT_DIR}/source>)
