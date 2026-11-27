//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -validate-ir
//TEST:SIMPLE(filecheck=ARRAY): -target spirv-asm -stage fragment -entry main -validate-ir

// Check that selecting a heap sampler stored in a struct field compiles with native
// descriptor heaps and with the descriptor-array fallback.

// CHECK: OpTypeSampler
// CHECK: OpBranchConditional {{[^ ]+}} %[[TRUE:[^ ]+]] %[[FALSE:[^ ]+]]
// CHECK: %[[INDEX:[^ ]+]] = OpPhi %uint %uint_0 %[[TRUE]] %uint_1 %[[FALSE]]
// CHECK: OpUntypedAccessChainKHR {{.*}} %slang_samplerHeap %[[INDEX]]
// CHECK: OpImageSampleImplicitLod
// ARRAY: OpCapability SampledImageArrayNonUniformIndexing
// ARRAY: OpImageSampleImplicitLod

cbuffer Ubo { int x; };
struct Foo { SamplerState s; };

float4 main(float2 fragUv : TEXCOORD) : SV_TARGET
{
    Texture2D tex = ResourceDescriptorHeap[0];
    Foo foo;
    foo.s = (x == 0) ? (SamplerState)SamplerDescriptorHeap[0] : (SamplerState)SamplerDescriptorHeap[1];
    return tex.Sample(foo.s, fragUv);
}
