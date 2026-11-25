//TEST_IGNORE_FILE:

// `transitive-attribute-extensions-serialized.slang` compiles this module to a `.slang-module`
// before importing it. We give the precompiled library its own module name so that its
// `.slang-module` never shadows `transitive-attribute-extensions-lib` for the tests that import
// the library from source.
#include "transitive-attribute-extensions-lib.slang"
