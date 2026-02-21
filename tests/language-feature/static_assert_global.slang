//TEST:SIMPLE(filecheck=CHK):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPV):-target spirv -stage compute -entry computeMain

// Test global static assert functionality.
static_assert(true, "PASS_global");
static_assert(false, "FAIL_global");

// CHK: error 41400: static assertion failed, FAIL_global
// HLSL: error 41400: static assertion failed, FAIL_global
// GLSL: error 41400: static assertion failed, FAIL_global
// SPV: error 41400: static assertion failed, FAIL_global

[numthreads(1,1,1)]
void computeMain()
{
}
