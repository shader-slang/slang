//TEST_IGNORE_FILE:

// Regression test for https://github.com/shader-slang/slang/issues/10306
// `typedef precise float` triggers ICE 99999 in `-lang hlsl` mode.
// Run: slangc -lang hlsl tests/bugs/hlsl-precise-typedef-ice.hlsl -no-codegen
// When fixed, change to: //DIAGNOSTIC_TEST:SIMPLE: -lang hlsl -no-codegen

typedef precise float pfloat;
pfloat g_val;
