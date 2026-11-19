// WGSL has no bindless resource types, so a `DescriptorHandle<T>` is a `vec2<u32>` heap index
// there, as layout and AnyValue marshalling already assume (shader-slang/slang#13633). Every
// place the handle's type is spelled must emit it as `vec2<u32>`: function parameters and return
// types, struct fields in storage buffers, uniforms, and the AnyValue unpack functions that
// `reinterpret` generates.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NO-RESOURCE):-target wgsl -entry computeMain -stage compute

struct Material
{
    DescriptorHandle<Texture2D<float4>> texture;
    float scale;
};

uniform DescriptorHandle<Texture2D<float4>> gTexture;
uniform RWStructuredBuffer<Material> materials;
uniform StructuredBuffer<uint2> handleBits;
uniform RWStructuredBuffer<float4> outputBuffer;

[noinline]
float4 loadTexel(DescriptorHandle<Texture2D<float4>> texture)
{
    return texture.Load(int3(0));
}

[noinline]
DescriptorHandle<Texture2D<float4>> makeHandle(uint2 bits)
{
    return DescriptorHandle<Texture2D<float4>>(bits);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Material material = materials[tid.x];
    let sampledTexture = reinterpret<DescriptorHandle<Texture2D<float4>>>(handleBits[tid.x]);
    let sampler = reinterpret<DescriptorHandle<SamplerState>>(handleBits[tid.x + 1]);
    outputBuffer[tid.x] = loadTexel(makeHandle(handleBits[tid.y])) + loadTexel(gTexture) +
                          material.texture.Load(int3(0)) * material.scale +
                          sampledTexture.SampleLevel(sampler, float2(0), 0);
}

// CHECK: struct Material
// CHECK-NEXT: {
// CHECK-NEXT: texture_0 : vec2<u32>,
// CHECK-DAG: gTexture_0 : vec2<u32>
// CHECK-DAG: fn makeHandle_0({{.*}}: vec2<u32>) -> vec2<u32>
// CHECK-DAG: fn loadTexel_0({{.*}}: vec2<u32>) -> vec4<f32>
// CHECK-DAG: fn unpackAnyValue8_0({{.*}}) -> vec2<u32>
// CHECK-DAG: fn unpackAnyValue8_1({{.*}}) -> vec2<u32>

// No function, parameter, or field has a texture or sampler type; those types appear only as the
// element types of the heap bindings.
// NO-RESOURCE-NOT: -> texture_2d
// NO-RESOURCE-NOT: -> sampler
// NO-RESOURCE-NOT: : texture_2d<f32>,
// NO-RESOURCE-NOT: : texture_2d<f32>)
// NO-RESOURCE-NOT: : sampler,
// NO-RESOURCE-NOT: : sampler)
