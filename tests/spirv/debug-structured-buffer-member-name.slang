//TEST(compute, vulkan):SIMPLE(filecheck=SPV): -stage compute -entry computeMain -target spirv -emit-spirv-directly -g2

// This test verifies that StructuredBuffer debug info does not name its array member, which is
// what's expected by debuggers when resolving StructuredBuffer accesses e.g.,
// ssbo[n] -> ssbo.unnamed[n] .

struct SSBOData
{
    float3 positionShift;
    float3 colorShift;
};

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=32):name=ssbo
StructuredBuffer<SSBOData> ssbo;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// SPV: OpString "unnamed"
// SPV-NOT: OpMemberName %RWStructuredBuffer 0
// SPV-NOT: OpMemberName %StructuredBuffer 0

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = ssbo[0].positionShift.x;
}
