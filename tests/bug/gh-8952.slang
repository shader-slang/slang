// GitHub issue #8952: Incorrect resource type of buffer of handles in a struct in a parameterblock
// https://github.com/shader-slang/slang/issues/8952
//
// When a StructuredBuffer<Texture2D.Handle> is inside a struct inside a ParameterBlock
// (with NO uniform data), the bindless resource heap was incorrectly placed in the same
// descriptor set as the ParameterBlock, causing descriptor type conflicts.
//

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly

// Case: Inside struct inside ParameterBlock with NO uniform data
// This is the failing case - the struct has only resource handles, no uniform data
struct Texture2DHandleBuffer
{
    StructuredBuffer<Texture2D.Handle> texture2DbufferOfHandles;
};

struct NestedParameterBlock
{
    ParameterBlock<Texture2DHandleBuffer> BLOCK;
};

ParameterBlock<Texture2DHandleBuffer> BLOCK;
ParameterBlock<NestedParameterBlock> BLOCK_nested;

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Use the buffer inside the ParameterBlock
    Texture2D tex = BLOCK.texture2DbufferOfHandles[0];
    Texture2D tex_nested = BLOCK_nested.BLOCK.texture2DbufferOfHandles[0];

    output[0] = tex.Load(int3(0, 0, 0));
    output[0] = tex_nested.Load(int3(0, 0, 0));
}

// Verify that the ParameterBlock's resource is in Set 1
// CHECK-DAG: OpDecorate %BLOCK_texture2DbufferOfHandles{{.*}}DescriptorSet 1

// Verify that the nested ParameterBlock's resource is in Set 2
// CHECK-DAG: OpDecorate %BLOCK_nested_BLOCK_texture2DbufferOfHandles{{.*}}DescriptorSet 2

// Verify that the resource heap is in Set 3 (NOT Set 1 or Set 2)
// CHECK-DAG: OpDecorate %__slang_resource_heap{{.*}}DescriptorSet 3
