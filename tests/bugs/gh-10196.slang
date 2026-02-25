//DIAGNOSTIC_TEST:SIMPLE: -target spirv -no-codegen

// Regression test: heap-buffer-overflow on unterminated import string
// Issue: https://github.com/shader-slang/slang/issues/10196

import "
