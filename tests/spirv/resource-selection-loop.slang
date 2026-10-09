//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -validate-ir

// This test checks that a conditionally updated sampler is carried through a loop as a heap index,
// and that a texture select also selects a heap index.

// CHECK: %[[S:[^ ]+]] = OpVariable %_ptr_Function_uint Function
// CHECK: OpStore %[[S]] %uint_0
// CHECK: OpLoopMerge
// CHECK: %[[INDEX:[^ ]+]] = OpLoad %uint %[[S]]
// CHECK: OpUntypedAccessChainKHR {{.*}} %slang_samplerHeap %[[INDEX]]
// CHECK: %[[NEXT:[^ ]+]] = OpIAdd %uint
// CHECK: OpStore %[[S]] %[[NEXT]]
// CHECK: %[[TEXTURE:[^ ]+]] = OpSelect %uint
// CHECK: OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap %[[TEXTURE]]
// CHECK: OpImageSampleImplicitLod

cbuffer Ubo { uint x; uint i; uint j; };

float4 main(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState s = SamplerDescriptorHeap[0];
    for (uint k = 0; k < x; k++)
    {
        if (k % 2 == 0)
            s = SamplerDescriptorHeap[i + k];
    }
    Texture2D<float4> t = select(x == 1,
        (Texture2D<float4>)ResourceDescriptorHeap[i],
        (Texture2D<float4>)ResourceDescriptorHeap[j]);
    return t.Sample(s, uv);
}
