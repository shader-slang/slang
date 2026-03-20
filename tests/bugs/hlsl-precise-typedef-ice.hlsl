//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -lang hlsl -no-codegen

// Regression test for https://github.com/shader-slang/slang/issues/10306
// `typedef precise float` should not trigger ICE 99999 in `-lang hlsl` mode.

typedef precise float pfloat;

// CHECK-NOT: 99999

pfloat g_val;
