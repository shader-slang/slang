//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK-GLSL): -stage compute -entry computeMain -allow-glsl -target glsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK-SPV): -stage compute -entry computeMain -allow-glsl -target spirv -emit-spirv-directly

volatile buffer MyBlockName1
{
    uint data1;
} inputBuffer1;
restrict buffer MyBlockName2
{
    uint data1;
} inputBuffer2;
readonly writeonly buffer MyBlockName3
{
    uint data1;
} inputBuffer3;
writeonly buffer MyBlockName4
{
    uint data1;
} inputBuffer4;
readonly buffer MyBlockName5
{
    uint data1;
} inputBuffer5;
coherent buffer MyBlockName6
{
    uint data1;
} inputBuffer6;

// CHECK-GLSL: main
// CHECK-SPV: OpEntryPoint
layout(local_size_x = 1) in;
void computeMain()
{
    inputBuffer1.data1 = 1;
    inputBuffer2.data1 = 1;
    inputBuffer4.data1 = 1;
    int v = inputBuffer5.data1;
    inputBuffer6.data1 = 1;
}