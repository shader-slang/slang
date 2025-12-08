// GitHub issue #8952: Incorrect resource type of buffer of handles in a struct in a parameterblock
// https://github.com/shader-slang/slang/issues/8952
//
// When a StructuredBuffer<Texture2D.Handle> is inside a struct inside a ParameterBlock
// (with NO uniform data), the bindless resource heap was incorrectly placed in the same
// descriptor set as the ParameterBlock, causing descriptor type conflicts.
//
// The fix ensures that ParameterBlock's SubElementRegisterSpace is tracked even when
// there is no container constant buffer (no uniform data).

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly

// Case: Inside struct inside ParameterBlock with NO uniform data
// This is the failing case - the struct has only resource handles, no uniform data
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
    
    output[0] = tex.Load(int3(0,0,0));
}

// Verify that the ParameterBlock's resource is in Set 1
// CHECK: OpDecorate %BLOCK_texture2DbufferOfHandles{{.*}}DescriptorSet 1

// Verify that the resource heap is in Set 2 (NOT Set 1)
// CHECK: OpDecorate %__slang_resource_heap{{.*}}DescriptorSet 2
