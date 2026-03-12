//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry computeMain -allow-glsl -target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry computeMain -allow-glsl -target spirv -emit-spirv-directly

readonly writeonly buffer MyBlockName3
{
    uint data1;
} inputBuffer1;

readonly buffer MyBlockName5
{
    uint data1;
} inputBuffer2;

buffer dontOptimizeOutBlock
{
    uint data1;
    uint data2;
} optimizeBlock;

layout(local_size_x = 1) in;
void computeMain()
{
    inputBuffer1.data1 = optimizeBlock.data1;
//CHECK:               ^ left of '=' is not an l-value
//CHECK:               ^ left of '=' is not an l-value.
    inputBuffer2.data1 = optimizeBlock.data2;
//CHECK:               ^ left of '=' is not an l-value
//CHECK:               ^ left of '=' is not an l-value.
}