//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -lang hlsl -no-codegen

// Regression test for https://github.com/shader-slang/slang/issues/10306
// `typedef volatile float` triggers ICE 99999 in `-lang hlsl` mode.

typedef volatile float vfloat;
//CHECK: 99999
vfloat g_val;
