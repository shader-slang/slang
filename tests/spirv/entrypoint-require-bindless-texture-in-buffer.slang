//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -profile spirv_1_6 -entry computeMain -stage compute

// Coherence regression pin for shader-slang/slang#11631.
//
// A `DescriptorHandle` stored in a buffer must lay out *and* load as the same representation. A
// source-level `[require(spvBindlessTextureNV)]` does not select the bindless descriptor path (the
// accessor is chosen from the global capabilities), so this handle uses the default `uint2`
// descriptor-heap representation for both its buffer layout and its access. A `uint64` layout with a
// `uint2` access (or vice versa) would emit inconsistent SPIR-V.

// CHECK: %v2uint = OpTypeVector %uint 2
// CHECK: %Handles_std430 = OpTypeStruct %v2uint
// CHECK: %__slang_resource_heap

struct Handles
{
    Texture2D<float4>.Handle tex;
};

[[vk::binding(0)]]
StructuredBuffer<Handles> handleBuffer;

[[vk::binding(1)]]
RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
[require(spvBindlessTextureNV)]
void computeMain()
{
    Texture2D<float4>.Handle h = handleBuffer[0].tex;
    outputBuffer[0] = h.Load(int3(0, 0, 0));
}
