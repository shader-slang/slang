//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -lang hlsl -no-codegen

// Regression test for https://github.com/shader-slang/slang/issues/10306
// `typedef precise float` triggers ICE 99999 in `-lang hlsl` mode.

typedef precise float pfloat;
//CHECK: 99999
pfloat g_val;
