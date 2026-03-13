//TEST_IGNORE_FILE:

// Regression test for https://github.com/shader-slang/slang/issues/10305
// HLSL `class` instantiation causes SIGSEGV when compiled with `-lang hlsl`.
// Defining a class alone compiles OK; declaring a variable of class type crashes.
// Run: slangc -lang hlsl tests/bugs/hlsl-class-instantiation-segfault.hlsl -no-codegen
// Expected: SIGSEGV (exit 139)
// When fixed, change to: //TEST:SIMPLE: -lang hlsl -no-codegen

class X {};
X x;
