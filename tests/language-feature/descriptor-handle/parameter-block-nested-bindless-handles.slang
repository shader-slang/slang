// Test: Nested ParameterBlock containing bindless resource handles
// GitHub issue #8952: https://github.com/shader-slang/slang/issues/8952
//
// When ParameterBlocks are nested, each must be in its own descriptor set,
// and the bindless resource heap must be in a separate descriptor set from all of them
// to avoid descriptor type conflicts.
//

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly

// Struct containing only resource handles (no uniform data)
struct Texture2DHandleBuffer
{
    StructuredBuffer<Texture2D.Handle> texture2DbufferOfHandles;
};

// Nested ParameterBlock structure
struct NestedParameterBlock
{
    ParameterBlock<Texture2DHandleBuffer> BLOCK;
    ParameterBlock<Texture2DHandleBuffer> BLOCK2;
};

ParameterBlock<NestedParameterBlock> BLOCK_nested;

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Access through the nested ParameterBlock
    Texture2D tex_nested = BLOCK_nested.BLOCK.texture2DbufferOfHandles[0];
    Texture2D tex_nested2 = BLOCK_nested.BLOCK2.texture2DbufferOfHandles[0];

    output[0] = tex_nested.Load(int3(0, 0, 0));
    output[1] = tex_nested2.Load(int3(0, 0, 0));
}

// Verify that the nested ParameterBlock's resource is in Set 1
// CHECK-DAG: OpDecorate %BLOCK_nested_BLOCK_texture2DbufferOfHandles{{.*}}DescriptorSet 1

// Verify that the nested ParameterBlock's resource is in Set 2
// CHECK-DAG: OpDecorate %BLOCK_nested_BLOCK2_texture2DbufferOfHandles{{.*}}DescriptorSet 2

// Verify that the resource heap is in Set 3 (NOT Set 1 or Set 2)
// CHECK-DAG: OpDecorate %__slang_resource_heap{{.*}}DescriptorSet 3

