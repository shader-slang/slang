//DIAGNOSTIC_TEST:SIMPLE: -target spirv -no-codegen

// Regression test: SIGSEGV on __generic> (missing opening <) after error recovery
// Issue: https://github.com/shader-slang/slang/issues/10249
// Found by AFL++ fuzzing

__generic>t;
