// Test: ParameterBlock containing only bindless resource handles (no uniform data)
// GitHub issue #8952: https://github.com/shader-slang/slang/issues/8952
//
// When a StructuredBuffer<Texture2D.Handle> is inside a struct inside a ParameterBlock
// with NO uniform data, the bindless resource heap must be placed in a separate
// descriptor set to avoid descriptor type conflicts.
//

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly

// ParameterBlock containing only resource handles (no uniform data)
struct Texture2DHandleBuffer
{
    StructuredBuffer<Texture2D.Handle> texture2DbufferOfHandles;
};

ParameterBlock<Texture2DHandleBuffer> BLOCK;

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Use the buffer inside the ParameterBlock
    Texture2D tex = BLOCK.texture2DbufferOfHandles[0];
    output[0] = tex.Load(int3(0, 0, 0));
}

// Verify that the ParameterBlock's resource is in Set 1
// CHECK: OpDecorate %BLOCK_texture2DbufferOfHandles{{.*}}DescriptorSet 1

// Verify that the resource heap is in Set 2 (NOT Set 1)
// This ensures the bindless heap doesn't conflict with the ParameterBlock's descriptor set
// CHECK: OpDecorate %__slang_resource_heap{{.*}}DescriptorSet 2

