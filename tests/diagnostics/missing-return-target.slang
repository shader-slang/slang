//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP): -entry computeMain -stage compute -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP): -entry computeMain -stage compute -target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NOT_SUPP): -entry computeMain -stage compute -target wgsl

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP):
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -entry computeMain -stage compute -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -entry computeMain -stage compute -target metal
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -entry computeMain -stage compute -target cpp
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SUPP): -entry computeMain -stage compute -target cuda

// Some compilation targets allow missing returns while some do not.
// This test ensures that either errors and warnings are emitted appropriately.

RWStructuredBuffer<uint> out;

//CHECK_SUPP: non-void function does not return in all cases
//CHECK_SUPP: non-void function does not return in all cases
uint func()
/*CHECK_NOT_SUPP:
     ^^^^ non-void function does not return in all cases
     ^^^^ non-void function does not return in all cases
     ^^^^ non-void function must return
     ^^^^ non-void function must return in all cases for target
*/
{
}

[shader("compute")]
void computeMain()
{
    out[0] = func();
}
