//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target glsl
//TEST:REFLECTION: -stage compute -entry computeMain -target glsl

RWByteAddressBuffer outputBuffer;

// CHECK-DAG: layout(binding = 0, set = 1)
// CHECK-DAG: texture2D g_TextureHeap_1[]

// CHECK-DAG: layout(binding = 0, set = 1)
// CHECK-DAG: utexture2D g_TextureHeap_0[]

[vk::binding(0, 1)]
__DynamicResource<__DynamicResourceKind.General> g_TextureHeap[];

// CHECK-DAG: layout(binding = 2, set = 1)
// CHECK-DAG: sampler g_SamplerHeap_0[]
[vk::binding(2, 1)]
__DynamicResource<__DynamicResourceKind.Sampler> g_SamplerHeap[];

// CHECK-DAG: layout(binding = 0, set = 2)
// CHECK-DAG: texture2D g_SingleDynamicResource_0;
[vk::binding(0, 2)]
__DynamicResource g_SingleDynamicResource;


// CHECK: IndirectAccess_0
// CHECK: g_TextureHeap_0[_S1]
[noinline]
float4 IndirectAccess(__DynamicResource resource, int2 pos)
{
    let texture = resource.as<Texture2D<uint4>>();
    return texture[pos] / 255.0;
}

// CHECK: main
// CHECK-DAG: g_TextureHeap_1[3]
// CHECK-DAG: g_SamplerHeap_0[2]
[numthreads(16)]
void computeMain(int2 tid: SV_DispatchThreadID)
{
    Texture2D texture = g_TextureHeap[3];
    SamplerState sampler = g_SamplerHeap[2].as<SamplerState>();
    float4 value = texture.SampleLevel(sampler, tid / 64.0, 0.0);

    value += IndirectAccess(g_TextureHeap[1], tid);

    value += g_SingleDynamicResource.as<Texture2D>()[tid];

    outputBuffer.Store(tid.x + tid.y * 64, value);
}