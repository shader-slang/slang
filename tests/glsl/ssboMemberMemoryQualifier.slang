//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK-GLSL): -stage compute -entry computeMain -allow-glsl -target glsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK-SPV): -stage compute -entry computeMain -allow-glsl -target spirv -emit-spirv-directly

volatile buffer MyBlockName1
{
    restrict uint data1;
    readonly uint data2;
    readonly writeonly uint data3;
    writeonly uint data4;
    coherent uint data5;
    volatile uint data6;
} inputBuffer1;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockNameOutput
{
    uint data[];
} outputBuffer;

// CHECK-GLSL: main
// CHECK-SPV: OpEntryPoint
layout(local_size_x = 1) in;
void computeMain()
{
    int v = inputBuffer1.data1;
    v = inputBuffer1.data2;
    inputBuffer1.data4 = 1;
    inputBuffer1.data5 = 1;
    inputBuffer1.data6 = 1;
}
