//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -validate-ir
//TEST:SIMPLE(filecheck=ARRAY): -target spirv-asm -stage fragment -entry main -validate-ir

// Check that selecting a heap sampler stored in a struct field compiles with native
// descriptor heaps and with the descriptor-array fallback.

// CHECK: OpTypeSampler
// CHECK: %[[S:[^ ]+]] = OpVariable %_ptr_Function_uint Function
// CHECK: OpStore %[[S]] %uint_0
// CHECK: OpStore %[[S]] %uint_1
// CHECK: %[[INDEX:[^ ]+]] = OpLoad %uint %[[S]]
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
