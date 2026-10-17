//TEST:REFLECTION(filecheck=HLSL): -target hlsl -stage compute -entry computeMain
//TEST:REFLECTION(filecheck=VK): -target spirv -stage compute -entry computeMain
//TEST:REFLECTION(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

// Only Metal gives a parameter block field an argument buffer slot, so no other
// target reports one.

struct PB
{
    float4 v;
    float s;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = block.v.x + block.s;
}

// HLSL: "name": "v"
// HLSL-NOT: metalArgumentBufferElement
// VK: "name": "v"
// VK-NOT: metalArgumentBufferElement
// WGSL: "name": "v"
// WGSL-NOT: metalArgumentBufferElement
