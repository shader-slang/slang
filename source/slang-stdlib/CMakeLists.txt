#
# Generate an embeddable stdlib
#

set(stdlib_generated_header_dir ${CMAKE_CURRENT_BINARY_DIR})
set(stdlib_generated_header
    ${stdlib_generated_header_dir}/slang-stdlib-generated.h
)
add_custom_command(
    OUTPUT ${stdlib_generated_header}
    COMMAND
        slang-bootstrap -archive-type riff-lz4 -save-stdlib-bin-source
        ${stdlib_generated_header}
    DEPENDS slang-bootstrap
    VERBATIM
)

slang_add_target(
  .
  OBJECT
  EXCLUDE_FROM_ALL
  USE_EXTRA_WARNINGS
  TARGET_NAME slang-embedded-stdlib
  LINK_WITH_PRIVATE core
  FOLDER generated
  EXPLICIT_SOURCE ./slang-embedded-stdlib.cpp ${stdlib_generated_header}
  EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_EMBED_STDLIB
  INCLUDE_DIRECTORIES_PRIVATE ${stdlib_generated_header_dir}
)

slang_add_target(
  .
  OBJECT
  EXCLUDE_FROM_ALL
  USE_EXTRA_WARNINGS
  TARGET_NAME slang-no-embedded-stdlib
  LINK_WITH_PRIVATE core
  FOLDER generated
  EXPLICIT_SOURCE ./slang-embedded-stdlib.cpp
)

#
# Generate embedded stdlib source
#

slang_add_target(
  .
  OBJECT
  EXCLUDE_FROM_ALL
  USE_EXTRA_WARNINGS
  TARGET_NAME slang-embedded-stdlib-source
  LINK_WITH_PRIVATE core slang-capability-defs slang-reflect-headers SPIRV-Headers slang-meta-headers
  INCLUDE_DIRECTORIES_PRIVATE ../slang
  FOLDER generated
  EXPLICIT_SOURCE ./slang-embedded-stdlib-source.cpp
  EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_EMBED_STDLIB_SOURCE
  INCLUDE_DIRECTORIES_PRIVATE ${stdlib_generated_header_dir}
)

slang_add_target(
  .
  OBJECT
  EXCLUDE_FROM_ALL
  USE_EXTRA_WARNINGS
  TARGET_NAME slang-no-embedded-stdlib-source
  LINK_WITH_PRIVATE core slang-capability-defs slang-reflect-headers SPIRV-Headers slang-meta-headers
  INCLUDE_DIRECTORIES_PRIVATE ../slang
  FOLDER generated
  EXPLICIT_SOURCE ./slang-embedded-stdlib-source.cpp
)
