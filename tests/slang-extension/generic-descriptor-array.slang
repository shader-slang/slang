//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -O0 -target glsl

RWByteAddressBuffer outputBuffer;

// CHECK: layout(binding = 0, set = 1)
// CHECK-NEXT: utexture2D g_TextureHeap_0[]
[vk::binding(0, 1)]
__ResourceArray<__IndexableResourceKind.Resource> g_TextureHeap;

// CHECK: layout(binding = 2, set = 1)
// CHECK-NEXT: sampler g_SamplerHeap_0[]
[vk::binding(2, 1)]
__ResourceArray<__IndexableResourceKind.Sampler> g_SamplerHeap;

// CHECK: layout(binding = 0, set = 1)
// CHECK-NEXT: texture2D g_TextureHeap_1[]

// CHECK: IndirectAccess_0
// CHECK: g_TextureHeap_0[1]
[noinline]
float4 IndirectAccess(__ResourceArray<__IndexableResourceKind.Resource> array, int2 pos)
{
    return array.get<Texture2D<uint4>>(1)[pos] / 255.0;
}

// CHECK: main
// CHECK: g_TextureHeap_1[3]
// CHECK: g_SamplerHeap_0[2]
[numthreads(16)]
void computeMain(int2 tid: SV_DispatchThreadID)
{
    let texture = g_TextureHeap.get<Texture2D>(3);
    let sampler = g_SamplerHeap.get<SamplerState>(2);
    float4 value = texture.SampleLevel(sampler, tid / 64.0, 0.0);

    value += IndirectAccess(g_TextureHeap, tid);

    outputBuffer.Store(tid.x + tid.y * 64, value);
}