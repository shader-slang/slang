//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly

// A physical (`PhysicalStorageBuffer`) pointer may be a struct member under SPIR-V logical
// addressing, so type legalization leaves a struct holding one intact, including as the element
// type of a structured buffer.

struct View
{
    Ptr<float> data;
    float tag;
}

RWStructuredBuffer<View> views;
RWStructuredBuffer<float> outputBuffer;

// CHECK: OpTypeStruct %_ptr_PhysicalStorageBuffer_float %float

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    View v = views[0];
    outputBuffer[0] = v.data[0] + v.tag;
}
