//TEST(compute, vulkan):SIMPLE(filecheck=SPV): -stage compute -entry computeMain -target spirv -emit-spirv-directly -g2

// This test checks that StructuredBuffer debug info uses "__member0" naming
// for the array member instead of "unamed", making it easier for debuggers to parse.

struct SSBOData
{
    float3 positionShift;
    float3 colorShift;
};

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=32):name=ssbo
StructuredBuffer<SSBOData> ssbo;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// SPV: OpString "__member0"
// SPV: OpMemberName %RWStructuredBuffer 0 "__member0"
// SPV: OpMemberName %StructuredBuffer 0 "__member0"
// SPV: DebugTypeMember %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}}

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = ssbo[0].positionShift.x;
}
