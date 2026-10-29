// A groupshared array sized by an enum-typed specialization constant on GLSL and WGSL (#14030).

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-via-glsl

// GLSL: shared float {{ *}}byEnum_0[kSize_0];
// WGSL: var<workgroup> byEnum_0 : array<f32, kSize_0>;

enum Size : int
{
    Small = 4,
    Large = 8,
}

[SpecializationConstant]
const Size kSize = Size.Small;

groupshared float byEnum[int(kSize)];

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint gi: SV_GroupIndex)
{
    byEnum[gi] = gi;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[gi] = byEnum[3 - gi];
}

// BUF: 3.0
// BUF-NEXT: 2.0
// BUF-NEXT: 1.0
// BUF-NEXT: 0.0
