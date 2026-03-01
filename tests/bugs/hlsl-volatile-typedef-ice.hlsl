//TEST_IGNORE_FILE:

// Regression test for https://github.com/shader-slang/slang/issues/10306
// `typedef volatile float` triggers ICE 99999 in `-lang hlsl` mode.
// Run: slangc -lang hlsl tests/bugs/hlsl-volatile-typedef-ice.hlsl -no-codegen
// When fixed, change to: //DIAGNOSTIC_TEST:SIMPLE: -lang hlsl -no-codegen

typedef volatile float vfloat;
vfloat g_val;
