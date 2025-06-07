//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NOT_SUPP): -entry computeMain -stage compute -target spirv
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NOT_SUPP): -entry computeMain -stage compute -target glsl
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NOT_SUPP): -entry computeMain -stage compute -target wgsl

//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SUPP):
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SUPP): -entry computeMain -stage compute -target hlsl
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SUPP): -entry computeMain -stage compute -target metal
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SUPP): -entry computeMain -stage compute -target cpp
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SUPP): -entry computeMain -stage compute -target cuda

// Some compilation targets allow missing returns while some do not.
// This test ensures that either errors and warnings are emitted appropriately.

RWStructuredBuffer<uint> out;

// CHECK_NOT_SUPP: warning 41010: non-void function
// CHECK_NOT_SUPP: error 41009: non-void function

// CHECK_SUPP: warning 41010: non-void function

uint func()
{
}

[shader("compute")]
void computeMain()
{
    out[0] = func();
}
