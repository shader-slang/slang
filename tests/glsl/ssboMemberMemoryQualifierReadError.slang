//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry computeMain -allow-glsl -target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry computeMain -allow-glsl -target spirv -emit-spirv-directly

volatile buffer MyBlockName1
{
    readonly uint data2;
    readonly writeonly uint data3;
} inputBuffer1;

layout(local_size_x = 1) in;
void computeMain()
{
    inputBuffer1.data2 = 1;
//CHECK:               ^ left of '=' is not an l-value
//CHECK:               ^ left of '=' is not an l-value.
    inputBuffer1.data3 = 1;
//CHECK:               ^ left of '=' is not an l-value
//CHECK:               ^ left of '=' is not an l-value.
}